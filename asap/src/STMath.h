//
// C++ Interface: STMath
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTMATH_H
#define ASAPSTMATH_H

#include <map>
#include <string>
#include <iostream>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

#include "Scantable.h"
#include "STDefs.h"
#include "STPol.h"

namespace asap {

/**
	* Mathmatical operations on Scantable objects
	* @author Malte Marquarding
*/
class STMath {
public:
	// typedef for long method name
  typedef casa::InterpolateArray1D<casa::Double,
                                   casa::Float>::InterpolationMethod imethod;

  // typedef for std::map
  typedef std::map<std::string, imethod> imap;

/**
  * whether to operate on the given Scantable or return a new one
  * @param insitu the toggle for this behaviour
  */
  explicit STMath(bool insitu=true);

  virtual ~STMath();

  /**
   * get the currnt @attr inistu_ state
   */
  bool insitu() const { return insitu_;};

  /**
   * set the currnt @attr inistu state
   * @param b the new state
   */
  void setInsitu(bool b) { insitu_ = b; };


  /**
    * average a vector of Scantables
    * @param in the vector of Scantables to average
    * @param mask an optional mask to apply on specific weights
    * @param weight weighting scheme
    * @param avmode the mode ov averaging. Per "SCAN" or "ALL".
    * @return a casa::CountedPtr<Scantable> which either holds a new Scantable
    * or returns the imput pointer.
    */
  casa::CountedPtr<Scantable>
    average( const std::vector<casa::CountedPtr<Scantable> >& in,
             const std::vector<bool>& mask = std::vector<bool>(),
             const std::string& weight = "NONE",
             const std::string& avmode = "SCAN");

  /**
    * median average a vector of Scantables. See also STMath::average
    * @param in the Scantable to average
    * @param mode the averaging mode. Currently only "MEDIAN"
    * @param avmode the mode ov averaging. Per "SCAN" or "ALL".
    * @return a casa::CountedPtr<Scantable> which either holds a new Scantable
    * or returns the imput pointer.
    */
  casa::CountedPtr<Scantable>
    averageChannel( const casa::CountedPtr<Scantable> & in,
                    const std::string& mode = "MEDIAN",
                    const std::string& avmode = "SCAN");

  /**
    * Average polarisations together. really only useful if only linears are
    * available.
    * @param in the input Scantable
    * @param mask an optional mask if weight allows one
    * @param weight weighting scheme
    * @return
    */
  casa::CountedPtr< Scantable >
    averagePolarisations( const casa::CountedPtr< Scantable > & in,
                          const std::vector<bool>& mask,
                          const std::string& weight );

  /**
    * Average beams together.
    * @param in the input Scantable
    * @param mask an optional mask if weight allows one
    * @param weight weighting scheme
    * @return
    */
  casa::CountedPtr< Scantable >
    averageBeams( const casa::CountedPtr< Scantable > & in,
                   const std::vector<bool>& mask,
                   const std::string& weight );

  casa::CountedPtr<Scantable>
    unaryOperate( const casa::CountedPtr<Scantable>& in, float val,
                  const std::string& mode, bool tsys=false,
                  bool skip_flaggedrow=false );

  // array operation
  casa::CountedPtr<Scantable>
    arrayOperate( const casa::CountedPtr<Scantable>& in,
                  const std::vector<float> val,
                  const std::string& mode,
                  const std::string& opmode="channel",  
                  bool tsys=false,
                  bool skip_flaggedrow=false );

  // channel operation
  casa::CountedPtr<Scantable>
    arrayOperateChannel( const casa::CountedPtr<Scantable>& in, 
                         const std::vector<float> val,
                         const std::string& mode, bool tsys=false,
                         bool skip_flaggedrow=false );

  // row operation
  casa::CountedPtr<Scantable>
    arrayOperateRow( const casa::CountedPtr<Scantable>& in, 
                     const std::vector<float> val,
                     const std::string& mode, bool tsys=false,
                     bool skip_flaggedrow=false );

  // 2d array operation
  casa::CountedPtr<Scantable>
    array2dOperate( const casa::CountedPtr<Scantable>& in,
                  const std::vector< std::vector<float> > val,
                  const std::string& mode, bool tsys=false );

  casa::CountedPtr<Scantable>
    binaryOperate( const casa::CountedPtr<Scantable>& left,
		   const casa::CountedPtr<Scantable>& right,
		   const std::string& mode);

  casa::CountedPtr<Scantable> autoQuotient(const casa::CountedPtr<Scantable>& in,
                                           const std::string& mode = "NEAREST",
                                           bool preserve = true);

  casa::CountedPtr<Scantable> quotient( const casa::CountedPtr<Scantable>& on,
                                        const casa::CountedPtr<Scantable>& off,
                                        bool preserve = true );

  /**
    * Calibrate total power scans (translated from GBTIDL)
    * @param calon uncalibrated Scantable with CAL noise signal 
    * @param caloff uncalibrated Scantable with no CAL signal
    * @param tcal optional scalar Tcal, CAL temperature (K)
    * @return casa::CountedPtr<Scantable> which holds a calibrated Scantable 
    * (spectrum - average of the two CAL on and off spectra;
    * tsys - mean Tsys = <caloff>*Tcal/<calon-caloff> + Tcal/2)
    */  	    
  casa::CountedPtr<Scantable> dototalpower( const casa::CountedPtr<Scantable>& calon,
                                            const casa::CountedPtr<Scantable>& caloff,
                                            casa::Float tcal=1.0 );

  /**
    * Combine signal and reference scans (translated from GBTIDL)
    * @param sig Scantable which contains signal scans
    * @param ref Scantable which contains reference scans
    * @param smoothref optional Boxcar smooth width of the reference scans
    * default: no smoothing (=1)
    * @param tsysv optional scalar Tsys value at the zenith, required to 
    * set tau, as well 
    * @param tau optional scalar Tau value
    * @return casa::CountedPtr<Scantable> which holds combined scans
    * (spectrum = (sig-ref)/ref * Tsys )
    */
  casa::CountedPtr<Scantable> dosigref( const casa::CountedPtr<Scantable>& sig,
                                        const casa::CountedPtr<Scantable>& ref,
                                        int smoothref=1,
                                        casa::Float tsysv=0.0,
                                        casa::Float tau=0.0 );

  /**
    * Calibrate GBT Nod scan pairs (translated from GBTIDL)
    * @param s Scantable which contains Nod scans
    * @param scans Vector of scan numbers
    * @param smoothref optional Boxcar smooth width of the reference scans
    * @param tsysv optional scalar Tsys value at the zenith, required to
    * set tau, as well
    * @param tau optional scalar Tau value 
    * @param tcal optional scalar Tcal, CAL temperature (K)
    * @return casa::CountedPtr<Scantable> which holds calibrated scans
    */
  casa::CountedPtr<Scantable> donod( const casa::CountedPtr<Scantable>& s,
                                     const std::vector<int>& scans,
                                     int smoothref=1,
                                     casa::Float tsysv=0.0,
                                     casa::Float tau=0.0,
                                     casa::Float tcal=0.0 );

  /**
    * Calibrate frequency switched scans (translated from GBTIDL)
    * @param s Scantable which contains frequency switched  scans
    * @param scans Vector of scan numbers
    * @param smoothref optional Boxcar smooth width of the reference scans
    * @param tsysv optional scalar Tsys value at the zenith, required to
    * set tau, as well
    * @param tau optional scalar Tau value
    * @param tcal optional scalar Tcal, CAL temperature (K)
    * @return casa::CountedPtr<Scantable> which holds calibrated scans
    */
  casa::CountedPtr<Scantable> dofs( const casa::CountedPtr<Scantable>& s,
                                    const std::vector<int>& scans,
                                    int smoothref=1,
                                    casa::Float tsysv=0.0,
                                    casa::Float tau=0.0,
                                    casa::Float tcal=0.0 );

  /**
   * Calibrate data with Chopper-Wheel like calibration method 
   * which adopts position switching by antenna motion, 
   * wobbler (nutator) switching and On-The-Fly observation.
   * 
   * The method is applicable to APEX, and other telescopes other than GBT.
   *
   * @param a Scantable which contains ON and OFF scans
   * @param a string that indicates calibration mode 
   * @param a string that indicates antenna name
   **/
  casa::CountedPtr<Scantable> cwcal( const casa::CountedPtr<Scantable>& s,
                                       const casa::String calmode, 
                                       const casa::String antname );

  /**
   * Calibrate frequency switched scans with Chopper-Wheel like 
   * calibration method.
   *
   * The method is applicable to APEX, and other telescopes other than GBT.
   * 
   * @param a Scantable which contains ON and OFF scans
   * @param a string that indicates antenna name
   **/
  casa::CountedPtr<Scantable> cwcalfs( const casa::CountedPtr<Scantable>& s,
                                       const casa::String antname );


  /**
   * Folding frequency-switch data
   * @param sig
   * @param ref
   * @param choffset
   **/
  casa::CountedPtr<Scantable> dofold( const casa::CountedPtr<Scantable> &sig,
                                      const casa::CountedPtr<Scantable> &ref,
                                      casa::Double choffset,
                                      casa::Double choffset2 = 0.0 );

  /**
   * ALMA calibration
   **/
  casa::CountedPtr<Scantable> almacal( const casa::CountedPtr<Scantable>& s, 
                                       const casa::String calmode ) ;
  casa::CountedPtr<Scantable> almacalfs( const casa::CountedPtr<Scantable>& s ) ;

  casa::CountedPtr<Scantable>
    freqSwitch( const casa::CountedPtr<Scantable>& in );

  std::vector<float> statistic(const casa::CountedPtr<Scantable>& in,
                               const std::vector<bool>& mask,
                               const std::string& which);

  std::vector<float> statisticRow(const casa::CountedPtr<Scantable>& in,
                               const std::vector<bool>& mask,
			       const std::string& which,
			       int row);

  std::vector< int > minMaxChan(const casa::CountedPtr<Scantable>& in,
                                const std::vector<bool>& mask,
                                const std::string& which);

  casa::CountedPtr<Scantable> bin( const casa::CountedPtr<Scantable>& in,
                                   int width=5);
  casa::CountedPtr<Scantable>
    resample(const casa::CountedPtr<Scantable>& in,
             const std::string& method, float width);

  casa::CountedPtr<Scantable>
    smooth(const casa::CountedPtr<Scantable>& in, const std::string& kernel,
                      float width, int order=2);

  casa::CountedPtr<Scantable>
    gainElevation(const casa::CountedPtr<Scantable>& in,
                  const std::vector<float>& coeff,
                  const std::string& fileName,
                  const std::string& method);
  casa::CountedPtr<Scantable>
    convertFlux(const casa::CountedPtr<Scantable>& in, float d,
                float etaap, float jyperk);

  casa::CountedPtr<Scantable> opacity(const casa::CountedPtr<Scantable>& in,
                                      const std::vector<float>& tau);

  casa::CountedPtr<Scantable>
    merge(const std::vector<casa::CountedPtr<Scantable> >& in,
	  const std::string &freqTol = "");

  casa::CountedPtr<Scantable>
    invertPhase( const casa::CountedPtr<Scantable>& in);

  casa::CountedPtr<Scantable>
    rotateXYPhase( const casa::CountedPtr<Scantable>& in, float phase);

  casa::CountedPtr<Scantable>
    rotateLinPolPhase( const casa::CountedPtr<Scantable>& in, float phase);

  casa::CountedPtr<Scantable>
    swapPolarisations(const casa::CountedPtr<Scantable>& in);

  casa::CountedPtr<Scantable>
    frequencyAlign( const casa::CountedPtr<Scantable>& in,
                    const std::string& refTime = "",
                    const std::string& method = "cubic" );

  casa::CountedPtr<Scantable>
    convertPolarisation( const casa::CountedPtr<Scantable>& in,
                         const std::string& newtype);

  casa::CountedPtr<Scantable>
    mxExtract( const casa::CountedPtr<Scantable>& in,
               const std::string& srctype = "on");

  /**
   * "hard" flag the data, this flags everything selected in setSelection()
   * @param frequency the frequency to remove
   * @param width the number of lags to flag left to the side of the frequency
   */
  casa::CountedPtr<Scantable>
    lagFlag( const casa::CountedPtr<Scantable>& in, double start,
             double end, const std::string& mode="frequency");

  std::vector<float>
    fft( const casa::CountedPtr<Scantable>& in,
	 const std::vector<int>& whichrow, 
	 bool getRealImag=false );

  // test for average spectra with different channel/resolution
  casa::CountedPtr<Scantable>
    new_average( const std::vector<casa::CountedPtr<Scantable> >& in,
		 const bool& compel, 
		 const std::vector<bool>& mask = std::vector<bool>(),
		 const std::string& weight = "NONE",
		 const std::string& avmode = "SCAN" )
    throw (casa::AipsError) ;

private:
  casa::CountedPtr<Scantable>  applyToPol( const casa::CountedPtr<Scantable>& in,
                                           STPol::polOperation fptr,
                                           casa::Float phase);

  static imethod stringToIMethod(const std::string& in);
  static WeightType stringToWeight(const std::string& in);

  void scaleByVector(casa::Table& in,
                     const casa::Vector<casa::Float>& factor,
                     bool dotsys);

  void scaleFromAsciiTable(casa::Table& in, const std::string& filename,
                           const std::string& method,
                           const casa::Vector<casa::Float>& xout,
                           bool dotsys);

  void scaleFromTable(casa::Table& in, const casa::Table& table,
                      const std::string& method,
                      const casa::Vector<casa::Float>& xout, bool dotsys);

  void convertBrightnessUnits(casa::CountedPtr<Scantable>& in,
                              bool tokelvin, float cfac);

  casa::CountedPtr< Scantable >
    smoothOther( const casa::CountedPtr< Scantable >& in,
                 const std::string& kernel,
                 float width, int order=2 );

  casa::CountedPtr< Scantable >
    getScantable(const casa::CountedPtr< Scantable >& in, bool droprows);

  casa::MaskedArray<casa::Float>
    maskedArray( const casa::Vector<casa::Float>& s,
                 const casa::Vector<casa::uChar>& f );
  casa::MaskedArray<casa::Double>
    maskedArray( const casa::Vector<casa::Double>& s,
                 const casa::Vector<casa::uChar>& f );
  casa::Vector<casa::uChar>
    flagsFromMA(const casa::MaskedArray<casa::Float>& ma);

  // Frequency switching
  void calibrateFS( casa::CountedPtr<Scantable> &sig,
                    casa::CountedPtr<Scantable> &ref,
                    const casa::CountedPtr<Scantable> &rsig,
                    const casa::CountedPtr<Scantable> &rref,
                    const casa::CountedPtr<Scantable> &sky,
                    const casa::CountedPtr<Scantable> &hot,
                    const casa::CountedPtr<Scantable> &cold,
                    const casa::Vector<casa::uInt> &rows ) ;
  void calibrateAPEXFS( casa::CountedPtr<Scantable> &sig,
                        casa::CountedPtr<Scantable> &ref,
                        const vector< casa::CountedPtr<Scantable> > &on,
                        const vector< casa::CountedPtr<Scantable> > &sky,
                        const vector< casa::CountedPtr<Scantable> > &hot,
                        const vector< casa::CountedPtr<Scantable> > &cold,
                        const casa::Vector<casa::uInt> &rows ) ;
  void copyRows( casa::Table &out, 
                 const casa::Table &in, 
                 casa::uInt startout, 
                 casa::uInt startin, 
                 casa::uInt nrow, 
                 casa::Bool copySpectra=true,
                 casa::Bool copyFlagtra=true, 
                 casa::Bool copyTsys=true ) ; 
  casa::CountedPtr<Scantable> averageWithinSession( casa::CountedPtr<Scantable> &s,
                                                    vector<bool> &mask,
                                                    string weight ) ;

  bool insitu_;
};

}
#endif
