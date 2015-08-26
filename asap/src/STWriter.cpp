//#---------------------------------------------------------------------------
//# STWriter.cc: ASAP class to write out single dish spectra.
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: STWriter.cpp 2658 2012-10-10 03:29:17Z MalteMarquarding $
//#---------------------------------------------------------------------------

#include <string>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Utilities/Assert.h>
#include <casa/Logging/LogIO.h>

#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/PKSSDwriter.h>
#include <atnf/PKSIO/SrcType.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ArrayColumn.h>

#include <fits/FITS/FITSSpectralUtil.h>

#include "STFITSImageWriter.h"
#include "STAsciiWriter.h"
#include "STHeader.h"
#include "STMath.h"


#include "STWriter.h"

using namespace casa;
namespace asap {

STWriter::STWriter(const std::string &format)
{
  format_ = format;
  String t(format_);
  t.upcase();
  if (t == "MS2") {
    throw (AipsError("MS2 OUTPUT FORMAT IS NO LONGER SUPPORTED"));
  } else if (t == "SDFITS") {
    writer_ = new PKSSDwriter();
  } else if (t == "ASCII" || t == "FITS" || t == "CLASS") {
    writer_ = 0;
  } else {
    throw (AipsError("Unrecognized export format"));
  }
}

STWriter::~STWriter()
{
   if (writer_) {
     delete writer_;
   }
}

Int STWriter::setFormat(const std::string &format)
{
  if (format != format_) {
    if (writer_) delete writer_;
  }

  format_ = format;
  String t(format_);
  t.upcase();
  if (t== "MS2") {
    throw (AipsError("MS2 OUTPUT FORMAT IS NO LONGER SUPPORTED"));
  } else if (t== "SDFITS") {
    writer_ = new PKSSDwriter();
  } else if (t == "ASCII" || t == "FITS" || t == "CLASS") {
    writer_ = 0;
  } else {
    throw (AipsError("Unrecognized Format"));
  }
  return 0;
}

Int STWriter::write(const CountedPtr<Scantable> in,
                    const std::string &filename)
{
  // If we write out foreign formats we have to convert the frequency system
  // into the output frame, as we do everything related to SPectarlCoordinates
  // in asap on-the-fly.

  String freqframe;
  FITSSpectralUtil::specsysFromFrame(freqframe, 
				     in->frequencies().getFrame(true));
  CountedPtr<Scantable> inst = in;
  if (in->frequencies().getFrame(true) != in->frequencies().getFrame(false)) {
    FITSSpectralUtil::specsysFromFrame(freqframe, 
				     in->frequencies().getFrame(false));
  
    STMath stm(false);
    inst = stm.frequencyAlign(in);
  }

  if (format_=="ASCII") {
    STAsciiWriter iw;
    // ASCII calls SpectralCoordinate::toWorld so no freqAlign use 'in'
    if (iw.write(*in, filename)) {
      return 0;
    } else {
      return 1;
    }
  } else if ( format_ == "FITS" || format_ == "CLASS") {
    STFITSImageWriter iw;
    if (format_ == "CLASS") {
      iw.setClass(True);
    }
    if (iw.write(*inst, filename)) {
      return 0;
    }
  }

  // MS or SDFITS

  // Extract the header from the table.
  // this is a little different from what I have done
  // before. Need to check with the Offline User Test data
  STHeader hdr = inst->getHeader();
  std::vector<uint> ifs = inst->getIFNos();
  int nIF = inst->nif();//ifs.size();
  Vector<uInt> nPol(nIF),nChan(nIF);
  Vector<Bool> havexpol(nIF);
  String fluxUnit = hdr.fluxunit;
  fluxUnit.upcase();
  nPol = 0;nChan = 0; havexpol = False;
  for (uint i=0;i<ifs.size();++i) {
    if ( inst->npol() > 2 ) {
      havexpol(ifs[i]) = True;      
      nPol(ifs[i]) = 2;
    } else {
      nPol(ifs[i]) = inst->npol();
    }
    nChan(ifs[i]) = inst->nchan(ifs[i]);
  }
//   Vector<String> obstypes(2);
//   obstypes(0) = "TR";//on
//   obstypes(1) = "RF TR";//off
  const Table table = inst->table();


  // Create the output file and write static data.
  Int status;
  status = writer_->create(String(filename), hdr.observer, hdr.project,
                           inst->getAntennaName(), hdr.antennaposition,
                           hdr.obstype, hdr.fluxunit,
                           hdr.equinox, freqframe,
                           nChan, nPol, havexpol, False);
  if ( status ) {
    throw(AipsError("Failed to create output file"));
  }

  Int count = 0;
  PKSrecord pksrec;
  pksrec.scanNo = 1;
  // use spearate iterators to ensure renumbering of all numbers
  TableIterator scanit(table, "SCANNO");
  while (!scanit.pastEnd() ) {
    Table stable = scanit.table();
    TableIterator beamit(stable, "BEAMNO");
    pksrec.beamNo = 1;
    while (!beamit.pastEnd() ) {
      Table btable = beamit.table();
      //MDirection::ScalarColumn dirCol(btable, "DIRECTION");
      //pksrec.direction = dirCol(0).getAngle("rad").getValue();
      TableIterator cycit(btable, "CYCLENO");
      ROArrayColumn<Double> srateCol(btable, "SCANRATE");
      Vector<Double> sratedbl;
      srateCol.get(0, sratedbl);
      Vector<Float> srateflt(sratedbl.nelements());
      convertArray(srateflt, sratedbl);
      //pksrec.scanRate = srateflt;
      pksrec.scanRate = sratedbl;
      ROArrayColumn<Double> spmCol(btable, "SRCPROPERMOTION");
      spmCol.get(0, pksrec.srcPM);
      ROArrayColumn <Double> sdirCol(btable, "SRCDIRECTION");
      sdirCol.get(0, pksrec.srcDir);
      ROScalarColumn<Double> svelCol(btable, "SRCVELOCITY");
      svelCol.get(0, pksrec.srcVel);
      ROScalarColumn<uInt> bCol(btable, "BEAMNO");
      pksrec.beamNo = bCol(0)+1;
      pksrec.cycleNo = 1;
      while (!cycit.pastEnd() ) {
        Table ctable = cycit.table();
        TableIterator typeit( ctable, "SRCTYPE" ) ;
        while(!typeit.pastEnd() ) {
          Table ttable = typeit.table() ;
          TableIterator ifit(ttable, "IFNO", 
			     TableIterator::Ascending, TableIterator::HeapSort);
          MDirection::ScalarColumn dirCol(ctable, "DIRECTION");
          pksrec.direction = dirCol(0).getAngle("rad").getValue();
          pksrec.IFno = 1;
          while (!ifit.pastEnd() ) {
            Table itable = ifit.table();
            TableRow row(itable);
            // use the first row to fill in all the "metadata"
            const TableRecord& rec = row.get(0);
            ROArrayColumn<Float> specCol(itable, "SPECTRA");
            pksrec.IFno = rec.asuInt("IFNO")+1;
            uInt nchan = specCol(0).nelements();
            Double crval,crpix;
            //Vector<Double> restfreq;
            Float tmp0,tmp1,tmp2,tmp3,tmp4;
            String tcalt;
            Vector<String> stmp0, stmp1;
            inst->frequencies().getEntry(crpix,crval, pksrec.freqInc,
                                         rec.asuInt("FREQ_ID"));
            inst->focus().getEntry(pksrec.parAngle, pksrec.focusAxi, 
				   pksrec.focusTan,
                                   pksrec.focusRot, tmp0,tmp1,tmp2,tmp3,tmp4,
                                   rec.asuInt("FOCUS_ID"));
            inst->molecules().getEntry(pksrec.restFreq,stmp0,stmp1,
                                       rec.asuInt("MOLECULE_ID"));
//             inst->tcal().getEntry(pksrec.tcalTime, pksrec.tcal,
//                                   rec.asuInt("TCAL_ID"));
            inst->weather().getEntry(pksrec.temperature, pksrec.pressure,
                                     pksrec.humidity, pksrec.windSpeed,
                                     pksrec.windAz, rec.asuInt("WEATHER_ID"));
            Double pixel = Double(nchan/2);
            pksrec.refFreq = (pixel-crpix)*pksrec.freqInc + crval;
            // ok, now we have nrows for the n polarizations in this table
            polConversion(pksrec.spectra, pksrec.flagged, pksrec.xPol, itable);
            pksrec.tsys = tsysFromTable(itable);
            // dummy data
            uInt npol = pksrec.spectra.ncolumn();
            
            // TCAL
            inst->tcal().getEntry( pksrec.tcalTime, pksrec.tcal,
                                   rec.asuInt("TCAL_ID") ) ;
            if ( pksrec.tcal.nelements() == 1 ) {
              ROScalarColumn<uInt> uintCol( itable, "TCAL_ID" ) ;
              Vector<uInt> tcalids = uintCol.getColumn() ;
              pksrec.tcal.resize( npol ) ;
              Vector<Float> dummyA ;
              String dummyS ;
              for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
                inst->tcal().getEntry( dummyS, dummyA, tcalids[ipol] ) ;
                pksrec.tcal[ipol] = dummyA[0] ;
              }
            }
            else if ( pksrec.tcal.nelements() == nchan ) {
              ROScalarColumn<uInt> uintCol( itable, "TCAL_ID" ) ;
              Vector<uInt> tcalids = uintCol.getColumn() ;
              pksrec.tcal.resize( npol ) ;
              Vector<Float> dummyA ;
              String dummyS ;
              for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
                inst->tcal().getEntry( dummyS, dummyA, tcalids[ipol] ) ;
                pksrec.tcal[ipol] = mean( dummyA ) ;
              }
            }
            pksrec.mjd       = rec.asDouble("TIME");
            pksrec.interval  = rec.asDouble("INTERVAL");
            pksrec.fieldName = rec.asString("FIELDNAME");
            pksrec.srcName   = rec.asString("SRCNAME");
            //pksrec.obsType   = obstypes[rec.asInt("SRCTYPE")];
            pksrec.obsType = getObsTypes( rec.asInt("SRCTYPE") ) ;
            pksrec.bandwidth = nchan * abs(pksrec.freqInc);
            pksrec.azimuth   = rec.asFloat("AZIMUTH");
            pksrec.elevation = rec.asFloat("ELEVATION");
            pksrec.refBeam   = rec.asInt("REFBEAMNO") + 1;
            pksrec.sigma.resize(npol);
            pksrec.sigma     = 0.0f;
            pksrec.calFctr.resize(npol);
            pksrec.calFctr   = 0.0f;
            pksrec.baseLin.resize(npol,2);
            pksrec.baseLin   = 0.0f;
            pksrec.baseSub.resize(npol,9);
            pksrec.baseSub   = 0.0f;
            pksrec.xCalFctr  = 0.0;
            pksrec.flagrow = rec.asuInt("FLAGROW");
            
            status = writer_->write(pksrec);
            if ( status ) {
              writer_->close();
              throw(AipsError("STWriter: Failed to export Scantable."));
            }
            ++count;
            //++pksrec.IFno;
            ++ifit;
          }
          ++typeit ;
        }
        ++pksrec.cycleNo;
        ++cycit;
      }
      //++pksrec.beamNo;
      ++beamit;
    }
    ++pksrec.scanNo;
    ++scanit;
  }
  LogIO os( casa::LogOrigin("STWriter"));
  os << "STWriter: wrote " << count << " rows to " << filename
     << casa::LogIO::POST;

  writer_->close();

  return 0;
}

Vector<Float> STWriter::tsysFromTable(const Table& tab)
{
  ROArrayColumn<Float> tsysCol(tab, "TSYS");
  Vector<Float> out(tab.nrow());
  Vector<Float> tmp;
  for (uInt i=0; i<tab.nrow(); ++i) {
    tmp.resize();
    tmp = tsysCol(i);
    out[i] = tmp[0];
  }
  return out;
}

void STWriter::polConversion( Matrix< Float >& specs, Matrix< uChar >& flags,
                              Vector< Complex > & xpol, const Table & tab )
{
  String poltype = tab.keywordSet().asString("POLTYPE");
// Full stokes is not supported. Just allow stokes I
  if ( poltype == "stokes" && tab.nrow() != 1) {
    String msg = "poltype = " + poltype + " not yet supported in output.";
    throw(AipsError(msg));
  }
  ROArrayColumn<Float> specCol(tab, "SPECTRA");
  ROArrayColumn<uChar> flagCol(tab, "FLAGTRA");
  uInt nchan = specCol(0).nelements();
  uInt ncol = (tab.nrow()==1 ? 1: 2 );
  specs.resize(nchan, ncol);
  flags.resize(nchan, ncol);
  // the linears
  for (uInt i=0; i<ncol; ++i) {
    specs.column(i) = specCol(i);
    flags.column(i) = flagCol(i);
  }
  // now the complex if exists
  Bool hasxpol = False;
  xpol.resize();
  if ( tab.nrow() == 4 ) {
    hasxpol = True;
    xpol.resize(nchan);
    Vector<Float> reals, imags;
    reals = specCol(2); imags = specCol(3);
    for (uInt k=0; k < nchan; ++k) {
      xpol[k] = Complex(reals[k], imags[k]);
    }
  }
}


// get obsType string from SRCTYPE value
String STWriter::getObsTypes( Int srctype )
{
  return SrcType::getName(srctype) ;
}

}
