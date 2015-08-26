//#---------------------------------------------------------------------------
//# PKSMS2reader.h: Class to read Parkes Multibeam data from a v2 MS.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2000-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: PKSMS2reader.h,v 19.18 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2000/08/03, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSMS2READER_H
#define ATNF_PKSMS2READER_H

#include <atnf/PKSIO/PKSreader.h>
#include <atnf/PKSIO/PKSrecord.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <casa/namespace.h>

// <summary>
// Class to read Parkes Multibeam data from a v2 MS.
// </summary>

class PKSMS2reader : public PKSreader
{
  public:
    // Default constructor.
    PKSMS2reader();

    // Destructor.
    virtual ~PKSMS2reader();

    //  Open the MS for reading.
    virtual Int open(
        const String msName,
        const String antenna,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra);

    // Get parameters describing the data.
    virtual Int getHeader(
        String &observer,
        String &project,
        String &antName,
        Vector<Double> &antPosition,
        String &obsMode,
        String &bunit,
        Float  &equinox,
        //String &freqRef,
        String &dopplerFrame,
        Double &mjd,
        Double &refFreq,
        Double &bandwidth);

    // Get frequency parameters for each IF.
    virtual Int getFreqInfo(
        Vector<Double> &startFreq,
        Vector<Double> &endFreq);

    // Set data selection criteria.  Channel numbering is 1-relative, zero or
    // negative channel numbers are taken to be offsets from the last channel.
    virtual uInt select(
        const Vector<Bool> beamSel,
        const Vector<Bool> IFsel,
        const Vector<Int>  startChan,
        const Vector<Int>  endChan,
        const Vector<Int>  refChan,
        const Bool getSpectra = True,
        const Bool getXPol    = False,
        const Bool getFeedPos = False,
        const Bool getPointing = False,
        const Int  coordSys   = 0);


    // Find the range of the data selected in time and position.
    virtual Int findRange(
        Int    &nRow,
        Int    &nSel,
        Vector<Double> &timeSpan,
        Matrix<Double> &positions);

    // Read the next data record.
/**
    virtual Int read(
        Int             &scanNo,
        Int             &cycleNo,
        Double          &mjd,
        Double          &interval,
        String          &fieldName,
        String          &srcName,
        Vector<Double>  &srcDir,
        Vector<Double>  &srcPM,
        Double          &srcVel,
        String          &obsMode,
        Int             &IFno,
        Double          &refFreq,
        Double          &bandwidth,
        Double          &freqInc,
        Vector<Double>  &restFreq,
        Vector<Float>   &tcal,
        String          &tcalTime,
        Float           &azimuth,
        Float           &elevation,
        Float           &parAngle,
        Float           &focusAxi,
        Float           &focusTan,
        Float           &focusRot,
        Float           &temperature,
        Float           &pressure,
        Float           &humidity,
        Float           &windSpeed,
        Float           &windAz,
        Int             &refBeam,
        Int             &beamNo,
        Vector<Double>  &direction,
        Vector<Double>  &scanRate,
        Vector<Float>   &tsys,
        Vector<Float>   &sigma,
        Vector<Float>   &calFctr,
        Matrix<Float>   &baseLin,
        Matrix<Float>   &baseSub,
        Matrix<Float>   &spectra,
        Matrix<uChar>   &flagged,
        Complex         &xCalFctr,
        Vector<Complex> &xPol);
**/
    virtual Int read(PKSrecord &pksrec);


    // Read the next data record, just the basics.
    virtual Int read(
        Int           &IFno,
        Vector<Float> &tsys,
        Vector<Float> &calFctr,
        Matrix<Float> &baseLin,
        Matrix<Float> &baseSub,
        Matrix<Float> &spectra,
        Matrix<uChar> &flagged);

    // Close the MS.
    virtual void close(void);

  private:
    Vector<String> splitAntennaSelectionString( const String s );
    void setupAntennaList( const String s ) ;

    Bool   cHaveBaseLin, cHaveCalFctr, cHaveSrcVel, cHaveTsys, cHaveXCalFctr,
           cMSopen, cHaveTcal, cHaveDataCol, cALMA, cHaveSysCal, cHaveCorrectedDataCol;
    Int    cCycleNo, cIdx, cNRow, cScanNo;
    Double cTime, lastmjd;
    Vector<Int>    cEndChan, cRefChan, cStartChan;
    Vector<Bool>   cBeams, cIFs;
    Vector<Slicer> cDataSel;
    String         cDirRef, cTelName;
    MeasurementSet cPKSMS;
    Table          cSysCalTab, tmptab, tmptab2;

    //Vector<String> cAntenna;
    Vector<Int> cAntId;

    ROScalarColumn<Int>     cScanNoCol;
    ROScalarColumn<Double>  cTimeCol;
    ROScalarColumn<Double>  cIntervalCol;
    ROScalarColumn<Int>     cFieldIdCol;
    ROScalarColumn<String>  cFieldNameCol;
    ROArrayColumn<Double>   cFieldDelayDirCol;
    ROScalarColumn<Int>     cSrcIdCol;
    ROScalarColumn<Int>     cSrcId2Col;
    ROScalarColumn<String>  cSrcNameCol;
    ROArrayColumn<Double>   cSrcDirCol;
    ROArrayColumn<Double>   cSrcPMCol;
    ROArrayColumn<Double>   cSrcVelCol;
    ROScalarColumn<Int>     cStateIdCol;
    ROScalarColumn<Double>  cCalCol;   
    ROScalarColumn<String>  cObsModeCol;
    ROArrayColumn<Double>   cSrcRestFrqCol;
    ROScalarColumn<Int>     cDataDescIdCol;
    ROScalarColumn<Int>     cSpWinIdCol;
    ROArrayColumn<Double>   cChanFreqCol;
    ROScalarColumn<Double>   cTotBWCol;
    ROScalarColumn<Double>  cWeatherTimeCol;
    ROScalarColumn<Float>   cTemperatureCol;
    ROScalarColumn<Float>   cPressureCol;
    ROScalarColumn<Float>   cHumidityCol;
    ROArrayColumn<Float>    cTcalCol;
    ROScalarColumn<Int>     cBeamNoCol;
    ROArrayColumn<Double>   cPointingCol;
    ROScalarColumn<Double>  cPointingTimeCol;
    ROArrayColumn<Float>    cTsysCol;
    ROArrayColumn<Float>    cSigmaCol;
    ROArrayColumn<Float>    cCalFctrCol;
    ROArrayColumn<Float>    cBaseLinCol;
    ROArrayColumn<Float>    cBaseSubCol;
    ROArrayColumn<Float>    cFloatDataCol;
    ROArrayColumn<Bool>     cFlagCol;
    ROScalarColumn<Bool>    cFlagRowCol;
    ROScalarColumn<Complex> cXCalFctrCol;
    ROArrayColumn<Complex>  cDataCol;
    ROArrayColumn<Complex>  cCorrectedDataCol;
    ROScalarColumn<Int>     cNumReceptorCol;
    ROScalarColumn<Bool>    cSigStateCol;
    ROScalarColumn<Bool>    cRefStateCol;
    
};

#endif
