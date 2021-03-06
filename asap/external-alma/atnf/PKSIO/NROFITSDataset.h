//#---------------------------------------------------------------------------
//# NROFITSDataset.h: Class for NRO 45m FITS dataset.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//#---------------------------------------------------------------------------
//# Original: 2009/02/27, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO_FITS_DATASET_H
#define NRO_FITS_DATASET_H

#define NRO_FITS_ARYMAX 75

#include <atnf/PKSIO/NRODataset.h>

#include <string>
#include <map>

using namespace std ;

// <summary>
// Accessor class for NRO 45m FITS data.
// </summary>
//
// <prerequisite>
//   <li> <linkto class=NRO45FITSReader>NRO45FITSReader</linkto>
//   <li> <linkto class=NRODataset>NRODataset</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// This class actually accesses data from NRO telescopes. This is specialized class 
// for NRO 45m telescope with non-OTF observing mode. In contrast to other concrete classes, 
// both fillHeader and fillRecord methods are implemented here. 
// This is because that the output of non-OTF observing mode is in FITS format and is 
// quite different format from that of OTF observing mode.
// </etymology>
//
// <note>
// Although the input data is FITS format, the class does not depend on cfitsio library.
// </note>
//
// <synopsis>
// Accessor class for NRO 45m FITS data.
// </synopsis>
//

class NROFITSDataset : public NRODataset
{
 public:
  // constructor
  NROFITSDataset( string name ) ;

  // destructor
  virtual ~NROFITSDataset() ;

  // data initialization 
  virtual void initialize() ;

  // fill data record
  virtual  int fillRecord( int i ) ;

  // get various parameters
  virtual vector< vector<double> > getSpectrum() ;
  virtual vector<double> getSpectrum( int i ) ;
  virtual int getIndex( int irow ) ;
  virtual int getPolarizationNum() ;
  virtual uInt getArrayId( string type ) ;
  virtual double getStartIntTime( int i ) ;
  virtual double getScanTime( int i ) ;
  virtual uInt getPolNo( int irow ) ;

 protected:
  // stracture representing property of fields
  struct FieldProperty {
    //string name;    // field name
    int size;         // data size [byte]
    //string unit;    // unit of the field
    long offset;      // offset from the head of scan record [byte]
  };

  // fill header information
  int fillHeader( int sameEndian ) ;

  // Read char data
  int readHeader( string &v, const char *name ) ;
  int readTable( char *v, const char *name, int clen, int idx=0 ) ;
  int readTable( vector<char *> &v, const char *name, int idx=0 ) ;
  int readColumn( vector<string> &v, const char *name, int idx=0 ) ;

  // Read int data
  int readHeader( int &v, const char *name) ;
  int readTable( int &v, const char *name, int b, int idx=0 ) ;
  int readTable( vector<int> &v, const char *name, int b, int idx=0 ) ;
  int readColumn( vector<int> &v, const char *name, int b, int idx=0 ) ;

  // Read float data
  int readHeader( float &v, const char *name) ;
  int readTable( float &v, const char *name, int b, int idx=0 ) ;
  int readTable( vector<float> &v, const char *name, int b, int idx=0 ) ;
  int readColumn( vector<float> &v, const char *name, int b, int idx=0 ) ;

  // Read double data
  int readHeader( double &v, const char *name) ;
  int readTable( double &v, const char *name, int b, int idx=0 ) ;
  int readTable( vector<double> &v, const char *name, int b, int idx=0 ) ;
  int readColumn( vector<double> &v, const char *name, int b, int idx=0 ) ;

  // read ARRY
  int readARRY() ;

  // Convert RA character representation to radian
  double radRA( string ra ) ;
  
  // Convert DEC character representation to radian
  double radDEC( string dec ) ;

  // get field parameters for scan header
  void getField() ;

  // fill array type
  void fillARYTP() ;

  // find data for each ARYTP
  void findData() ;

  // get offset bytes for attributes
  long getOffset( const char *name ) ;

  // move pointer to target position
  int movePointer( const char *name, int idx=0 ) ;

  // number of column for scan header
  int numField_ ;

  // number of HDU
  int numHdu_ ;

  // array type
  vector<string> ARYTP ;

  // reference index
  vector<int> arrayid_ ;

  // field properties
  // Key   = field name
  // Value = field properties 
  map<string, FieldProperty> properties_ ;

  // spectral data
  vector<int> JDATA ;
} ;


#endif /* NRO_FITS_DATASET_H */
