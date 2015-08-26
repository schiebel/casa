//
// C++ Interface: Scantable
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSCANTABLE_H
#define ASAPSCANTABLE_H

// STL
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
// AIPS++
#include <casa/aips.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/CountedPtr.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <scimath/Mathematics/FFTServer.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/Table.h>

#include "MathUtils.h"
#include "STBaselineEnum.h"
#include "STFit.h"
#include "STFitEntry.h"
#include "STFocus.h"
#include "STFrequencies.h"
#include "STHeader.h"
#include "STHistory.h"
#include "STMolecules.h"
#include "STPol.h"
#include "STSelector.h"
#include "STTcal.h"
#include "STWeather.h"

namespace asap {

class Fitter;
class STLineFinder;
class STBaselineTable;

/**
  * This class contains and wraps a casa::Table, which is used to store
  * all the information. This can be either a MemoryTable or a
  * disk based Table.
  * It provides access functions to the underlying table
  * It contains n subtables:
  * @li weather
  * @li frequencies
  * @li molecules
  * @li tcal
  * @li focus
  * @li fits
  *
  * @brief The main ASAP data container
  * @author Malte Marquarding
  * @date
  * @version
*/
class Scantable
{

friend class STMath;

public:
  /**
   * Default constructor
   */
  explicit Scantable(casa::Table::TableType ttype = casa::Table::Memory);

  /**
   * Create a Scantable object from an existing table on disk
   * @param[in] name the name of the existing Scantable
   */
  explicit Scantable(const std::string& name, 
		     casa::Table::TableType ttype = casa::Table::Memory);

  /// @fixme this is only sensible for MemoryTables....
  Scantable(const Scantable& other, bool clear=true);

  /**
   * Destructor
   */
  virtual ~Scantable();

  /**
   * get a const reference to the underlying casa::Table
   * @return const \ref casa::Table reference
   */
  const casa::Table& table() const;

  /**
   * get a reference to the underlying casa::Table with the Selection
   * object applied if set
   * @return casa::Table reference
   */
  casa::Table& table();


  /**
   * Get a handle to the selection object
   * @return constant STSelector reference
   */
  const STSelector& getSelection() const { return selector_; }

  /**
   * Set the data to be a subset as defined by the STSelector
   * @param selection a STSelector object
   */
  void setSelection(const STSelector& selection);

  /**
   * unset the selection of the data
   */
  void unsetSelection();
  /**
   * set the header
   * @param[in] sth an STHeader object
   */
  void setHeader( const STHeader& sth );

  /**
   * get the header information
   * @return an STHeader object
   */
  STHeader getHeader( ) const;

  /**
   * Checks if the "other" Scantable is conformant with this,
   * i.e. if  header values are the same.
   * @param[in] other another Scantable
   * @return true or false
   */
  bool conformant( const Scantable& other);

  /**
   *
   * @param stype The type of the source, 0 = on, 1 = off
   */
  void setSourceType(int stype);

  /**
   *
   * @param stype The name of the source
   */
  void setSourceName(const std::string& name);


  /**
   * The number of scans in the table
   * @return number of scans in the table
   */
  int nscan() const;

  casa::MEpoch::Types getTimeReference() const;


  casa::MEpoch getEpoch(int whichrow) const;

  /**
   * Get global antenna position
   * @return casa::MPosition
   */
  casa::MPosition getAntennaPosition() const;

  /**
   * the @ref casa::MDirection for a specific row
   * @param[in] whichrow the row number
   * return casa::MDirection
   */
  casa::MDirection getDirection( int whichrow ) const;

  /**
   * get the direction type as a string, e.g. "J2000"
   * @param[in] whichrow the row number
   * return the direction string
   */
  std::string getDirectionString( int whichrow ) const;

  /**
   * set the direction type as a string, e.g. "J2000"
   * @param[in] refstr the direction type
   */
  void setDirectionRefString(const std::string& refstr="");

  /**
   * get the direction reference string
   * @return a string describing the direction reference
   */
  std::string getDirectionRefString() const;

  /**
   *  Return the Flux unit of the data, e.g. "Jy" or "K"
   * @return string
   */
  std::string getFluxUnit() const;

  /**
   * Set the Flux unit of the data
   * @param unit a string representing the unit, e.g "Jy" or "K"
   */
  void setFluxUnit( const std::string& unit );

  /**
   * Set the Stokes type of the data
   * @param feedtype a string representing the type, e.g "circular" or "linear"
   */
  void setFeedType( const std::string& feedtype );

  /**
   *
   * @param instrument a string representing an insturment. see xxx
   */
  void setInstrument( const std::string& instrument );

  /**
   * (Re)calculate the azimuth and elevationnfor all rows
   */
  void calculateAZEL();

  /**
   * "hard" flag the data, this flags everything selected in setSelection()
   * param[in] msk a boolean mask of length nchan describing the points to
   * to be flagged
   */
  //void flag( const std::vector<bool>& msk = std::vector<bool>());
  //void flag( const std::vector<bool>& msk = std::vector<bool>(), bool unflag=false);

  void flag( int whichrow = -1, const std::vector<bool>& msk = std::vector<bool>(), bool unflag=false);

  /**
   * Flag the data in a row-based manner. (CAS-1433 Wataru Kawasaki)
   * param[in] rows    list of row numbers to be flagged
   */
  void flagRow( const std::vector<casa::uInt>& rows = std::vector<casa::uInt>(), bool unflag=false);

  /**
   * Get flagRow info at the specified row. If true, the whole data
   * at the row should be flagged.
   */
  bool getFlagRow(int whichrow) const
    { return (flagrowCol_(whichrow) > 0); }

  /**
   * Flag the data outside a specified range (in a channel-based manner).
   * (CAS-1807 Wataru Kawasaki)
   */
  void clip(const casa::Float uthres, const casa::Float dthres, bool clipoutside, bool unflag);

  /**
   * Return a list of booleans with the size of nchan for a specified row, to get info
   * about which channel is clipped.
   */
  std::vector<bool> getClipMask(int whichrow, const casa::Float uthres, const casa::Float dthres, bool clipoutside, bool unflag);
  void srchChannelsToClip(casa::uInt whichrow, const casa::Float uthres, const casa::Float dthres, bool clipoutside, bool unflag,
			  casa::Vector<casa::uChar> flgs);

  /**
   * Return a list of row numbers with respect to the original table.
   * @return a list of unsigned ints
   */
  std::vector<unsigned int> rownumbers() const;


  /**
   * Get the number of beams in the data or a specific scan
   * @param scanno the scan number to get the number of beams for.
   * If scanno<0 the number is retrieved from the header.
   * @return an integer number
   */
  int nbeam(int scanno=-1) const;
  /**
   * Get the number of IFs in the data or a specific scan
   * @param scanno the scan number to get the number of IFs for.
   * If scanno<0 the number is retrieved from the header.
   * @return an integer number
   */
  int nif(int scanno=-1) const;
  /**
   * Get the number of polarizations in the data or a specific scan
   * @param scanno the scan number to get the number of polarizations for.
   * If scanno<0 the number is retrieved from the header.
   * @return an integer number
   */
  int npol(int scanno=-1) const;

  std::string getPolType() const;


  /**
   * Get the number of integartion cycles
   * @param scanno the scan number to get the number of rows for.
   * If scanno<0 the number is retrieved from the header.
   * @return the number of rows (for the specified scanno)
   */
  int nrow(int scanno=-1) const;

  int getBeam(int whichrow) const;
  std::vector<uint> getBeamNos() const { return getNumbers(beamCol_); }

  int getIF(int whichrow) const;
  std::vector<uint> getIFNos() const { return getNumbers(ifCol_); }

  int getPol(int whichrow) const;
  std::vector<uint> getPolNos() const { return getNumbers(polCol_); }

  std::vector<uint> getScanNos() const { return getNumbers(scanCol_); }
  int getScan(int whichrow) const { return scanCol_(whichrow); }

  std::vector<uint> getCycleNos() const { return getNumbers(cycleCol_); }
  int getCycle(int whichrow) const { return cycleCol_(whichrow); }

  //TT addition
  std::vector<uint> getMolNos() {return getNumbers(mmolidCol_); }

  /**
   * Get the number of channels in the data or a specific IF. This currently
   * varies only with IF number
   * @param ifno the IF number to get the number of channels for.
   * If ifno<0 the number is retrieved from the header.
   * @return an integer number
   */
  int nchan(int ifno=-1) const;
  int getChannels(int whichrow) const;

  int ncycle(int scanno=-1) const;


  double getInterval(int whichrow) const
    { return integrCol_(whichrow); }

  float getTsys(int whichrow) const
    { return casa::Vector<casa::Float>(tsysCol_(whichrow))(0); }
  std::vector<float> getTsysSpectrum(int whichrow) const ;
  
  void setTsys(const std::vector<float>& newvals, int whichrow);

  float getElevation(int whichrow) const
    { return elCol_(whichrow); }
  float getAzimuth(int whichrow) const
    { return azCol_(whichrow); }
  float getParAngle(int whichrow) const
    { return focus().getParAngle(mfocusidCol_(whichrow)); }
  int getTcalId(int whichrow) const
    { return mtcalidCol_(whichrow); }

  std::string getSourceName(int whichrow) const
    { return srcnCol_(whichrow); }

  std::vector<bool> getMask(int whichrow) const;
  std::vector<float> getSpectrum(int whichrow,
                                 const std::string& poltype = "" ) const;

  void setSpectrum(const std::vector<float>& spec, int whichrow);

  std::string getPolarizationLabel(int index, const std::string& ptype) const
    { return STPol::getPolLabel(index, ptype ); }

  /**
   * Write the Scantable to disk
   * @param filename the output file name
   */
  void makePersistent(const std::string& filename);

  std::vector<std::string> getHistory(int nrow=-1, int start=0) const
    { return historyTable_.getHistory(nrow, start); }

  uint historyLength()
  { return historyTable_.nrow(); }
  
  void dropHistory() { historyTable_.drop(); }

  void addHistory(const std::string& hist) { historyTable_.addEntry(hist); }

  void appendToHistoryTable(const STHistory& otherhist)
    { historyTable_.append(otherhist); }

  std::string headerSummary();
  void summary(const std::string& filename="");
  std::string oldheaderSummary();
  //  std::string summary();
  void oldsummary(const std::string& filename="");

  //std::string getTime(int whichrow=-1, bool showdate=true) const;
  std::string getTime(int whichrow=-1, bool showdate=true, 
		      casa::uInt prec=0) const;
  double getIntTime(int whichrow) const { return integrCol_(whichrow); }

  // returns unit, conversion frame, doppler, base-frame

  /**
   * Get the frequency set up
   * This is forwarded to the STFrequencies subtable
   * @return unit, frame, doppler
   */
  std::vector<std::string> getCoordInfo() const
    { return freqTable_.getInfo(); };

  void setCoordInfo(std::vector<string> theinfo)
    { return freqTable_.setInfo(theinfo); };


  std::vector<double> getAbcissa(int whichrow) const;

  std::vector<float> getWeather(int whichrow) const;

  std::string getAbcissaLabel(int whichrow) const;
  std::vector<double> getRestFrequencies() const
    { return moleculeTable_.getRestFrequencies(); }
  std::vector<double> getRestFrequency(int id) const
    { return moleculeTable_.getRestFrequency(id); }

  /**
  void setRestFrequencies(double rf, const std::string& name = "",
                          const std::string& = "Hz");
  **/
  // Modified by Takeshi Nakazato 05/09/2008
  /***
  void setRestFrequencies(vector<double> rf, const vector<std::string>& name = "",
                          const std::string& = "Hz");
  ***/
  void setRestFrequencies(vector<double> rf,
                          const vector<std::string>& name = vector<std::string>(1,""),
                          const std::string& = "Hz");

  //void setRestFrequencies(const std::string& name);
  void setRestFrequencies(const vector<std::string>& name);

  void shift(int npix);

  casa::SpectralCoordinate getSpectralCoordinate(int whichrow) const;

  void convertDirection(const std::string& newframe);

  STFrequencies& frequencies() { return freqTable_; }
  const STFrequencies& frequencies() const { return freqTable_; }
  STWeather& weather() { return weatherTable_; }
  const STWeather& weather() const { return weatherTable_; }
  STFocus& focus() { return focusTable_; }
  const STFocus& focus() const { return focusTable_; }
  STTcal& tcal() { return tcalTable_; }
  const STTcal& tcal() const { return tcalTable_; }
  STMolecules& molecules() { return moleculeTable_; }
  const STMolecules& molecules() const { return moleculeTable_; }
  STHistory& history() { return historyTable_; }
  const STHistory& history() const { return historyTable_; }
  STFit& fit() { return fitTable_; }
  const STFit& fit() const { return fitTable_; }

  std::vector<std::string> columnNames() const;

  void addFit(const STFitEntry& fit, int row);
  STFitEntry getFit(int row) const
    { STFitEntry fe; fitTable_.getEntry(fe, mfitidCol_(row)); return fe; }

  //Added by TT
  /**
   * Get the antenna name
   * @return antenna name string
   */
  casa::String getAntennaName() const;

  /**
   * For GBT MS data only. check a scan list
   * against the information found in GBT_GO table for
   * scan number orders to get correct pairs.
   * @param[in] scan list
   * @return status
   */
  int checkScanInfo(const std::vector<int>& scanlist) const;

  /**
   * Get the direction as a vector, for a specific row
   * @param[in] whichrow the row numbyyer
   * @return the direction in a vector
   */
  std::vector<double> getDirectionVector(int whichrow) const;

  /**
   * Set a flag indicating whether the data was parallactified
   * @param[in] flag true or false
   */
  void parallactify(bool flag)
    { focusTable_.setParallactify(flag); }

  /**
   * Reshape spectrum
   * @param[in] nmin, nmax minimum and maximum channel
   * @param[in] irow       row number
   *
   * 30/07/2008 Takeshi Nakazato
   **/
  void reshapeSpectrum( int nmin, int nmax ) throw( casa::AipsError );
  void reshapeSpectrum( int nmin, int nmax, int irow ) ;

  /**
   * Change channel number under fixed bandwidth
   * @param[in] nchan, dnu new channel number and spectral resolution
   * @param[in] irow       row number
   *
   * 27/08/2008 Takeshi Nakazato
   **/
  void regridChannel( int nchan, double dnu ) ;
  void regridChannel( int nchan, double dnu, int irow ) ;
  void regridChannel( int nchan, double dnu, double fmin, int irow ) ;
  void regridSpecChannel( double dnu, int nchan=-1 ) ;

  bool isAllChannelsFlagged(casa::uInt whichrow);

  std::vector<std::string> applyBaselineTable(const std::string& bltable, 
					      const bool returnfitresult, 
					      const std::string& outbltable, 
					      const bool outbltableexists, 
					      const bool overwrite);
  std::vector<std::string> subBaseline(const std::vector<std::string>& blInfoList, 
				       const bool returnfitresult, 
				       const std::string& outbltable, 
				       const bool outbltableexists, 
				       const bool overwrite);
  void polyBaseline(const std::vector<bool>& mask,
		    int order,
		    float thresClip, 
		    int nIterClip,
		    bool getResidual=true,
		    const std::string& progressInfo="true,1000", 
		    const bool outLogger=false,
		    const std::string& blfile="",
		    const std::string& bltable="");
  void autoPolyBaseline(const std::vector<bool>& mask,
		        int order,
		        float thresClip, 
		        int nIterClip,
	                const std::vector<int>& edge, 
	                float threshold=3.0, 
	                int chanAvgLimit=1, 
	                bool getResidual=true,
	                const std::string& progressInfo="true,1000", 
	                const bool outLogger=false, 
	                const std::string& blfile="",
			const std::string& bltable="");
  void chebyshevBaseline(const std::vector<bool>& mask,
			 int order,
			 float thresClip, 
		         int nIterClip,
		         bool getResidual=true,
		         const std::string& progressInfo="true,1000", 
		         const bool outLogger=false,
		         const std::string& blfile="",
			 const std::string& bltable="");
  void autoChebyshevBaseline(const std::vector<bool>& mask,
		             int order,
		             float thresClip, 
		             int nIterClip,
		             const std::vector<int>& edge, 
		             float threshold=3.0, 
		             int chanAvgLimit=1, 
		             bool getResidual=true,
		             const std::string& progressInfo="true,1000", 
		             const bool outLogger=false, 
		             const std::string& blfile="",
			     const std::string& bltable="");
  void cubicSplineBaseline(const std::vector<bool>& mask,
			   int nPiece,
			   float thresClip, 
			   int nIterClip,
			   bool getResidual=true,
			   const std::string& progressInfo="true,1000", 
			   const bool outLogger=false,
			   const std::string& blfile="",
			   const std::string& bltable="");
  void autoCubicSplineBaseline(const std::vector<bool>& mask,
			       int nPiece,
			       float thresClip, 
			       int nIterClip,
			       const std::vector<int>& edge, 
			       float threshold=3.0, 
			       int chanAvgLimit=1, 
			       bool getResidual=true,
			       const std::string& progressInfo="true,1000", 
			       const bool outLogger=false, 
			       const std::string& blfile="",
			       const std::string& bltable="");
  void sinusoidBaseline(const std::vector<bool>& mask,
			const std::string& fftInfo, 
			const std::vector<int>& addNWaves, 
			const std::vector<int>& rejectNWaves, 
			float thresClip, 
		        int nIterClip,
			bool getResidual=true,
			const std::string& progressInfo="true,1000", 
		        const bool outLogger=false,
		        const std::string& blfile="",
			const std::string& bltable="");
  void autoSinusoidBaseline(const std::vector<bool>& mask,
			    const std::string& fftInfo, 
			    const std::vector<int>& addNWaves, 
			    const std::vector<int>& rejectNWaves, 
		            float thresClip, 
		            int nIterClip,
		            const std::vector<int>& edge, 
		            float threshold=3.0, 
		            int chanAvgLimit=1, 
			    bool getResidual=true,
			    const std::string& progressInfo="true,1000", 
		            const bool outLogger=false, 
		            const std::string& blfile="",
			    const std::string& bltable="");
  std::vector<float> execFFT(const int whichrow, 
			     const std::vector<bool>& inMask,
			     bool getRealImag=false,
			     bool getAmplitudeOnly=false);
  float getRms(const std::vector<bool>& mask, int whichrow);
  std::string formatBaselineParams(const std::vector<float>& params, 
			           const std::vector<bool>& fixed, 
			           float rms, 
				   int nClipped,
		   	           const std::string& masklist, 
			           int whichrow, 
			           bool verbose=false,
				   bool csvformat=false,
				   int start=-1,
				   int count=-1,
				   bool resetparamid=false) const;
  std::string formatPiecewiseBaselineParams(const std::vector<int>& ranges, 
					    const std::vector<float>& params, 
					    const std::vector<bool>& fixed, 
					    float rms, 
					    int nClipped,
					    const std::string& masklist, 
					    int whichrow, 
					    bool verbose=false,
					    bool csvformat=false) const;
  std::vector<uint> getMoleculeIdColumnData() const;
  void setMoleculeIdColumnData(const std::vector<uint>& molids);

  /**
   * Get row idx of root table
   **/
  std::vector<uint> getRootTableRowNumbers() const;

  double calculateModelSelectionCriteria(const std::string& valname,
					 const std::string& blfunc,
					 int order,
					 const std::vector<bool>& inMask,
					 int whichrow,
					 bool useLineFinder,
					 const std::vector<int>& edge,
					 float threshold,
					 int chanAvgLimit);
  static std::vector<bool> getMaskFromMaskList(const int nchan,
					  const std::vector<int>& masklist);
  static casa::Vector<casa::uInt> getMaskListFromMask(
					   const std::vector<bool>& mask);
  static std::vector<int> splitToIntList(const std::string& str, 
					 const char delim);
  static std::vector<string> splitToStringList(const std::string& str, 
					       const char delim);

  void dropXPol();

private:

  casa::Matrix<casa::Float> getPolMatrix( casa::uInt whichrow ) const;

  /**
   * Turns a time value into a formatted string
   * @param x
   * @return
   */
  std::string formatSec(casa::Double x) const;

  std::string formatTime(const casa::MEpoch& me, bool showdate)const;
  std::string formatTime(const casa::MEpoch& me, bool showdate, 
			 casa::uInt prec)const;

  /**
   *  Turns a casa::MDirection into a nicely formatted string
   * @param md an casa::MDirection
   * @param prec output precision of direction
   * @return
   */
  std::string formatDirection(const casa::MDirection& md, casa::Int prec=7) const;

  /**
   * Create a unique file name for the paged (temporary) table
   * @return just the name
   */
  static casa::String generateName();

  /**
   * attach to cached columns
   */
  void attach();

  /**
   * Set up the main casa::Table
   */
  void setupMainTable();

  void attachSubtables();
  void copySubtables(const Scantable& other);

  /**
   * Convert an "old" asap1 style row index into a new index
   * @param[in] therow
   * @return and index into @table_
   */
  int rowToScanIndex(int therow);

  std::vector<uint> getNumbers(const casa::ScalarColumn<casa::uInt>& col) const;

  /**
   * Returns a number of elements with "true" in a bool vector. 
   * @param[in] mask (boolean vector)
   * @return the numerb of elements in true
   */
  std::size_t nValidMask(const std::vector<bool>& mask);

  static const casa::uInt version_ = 4;

  STSelector selector_;

  casa::Table::TableType type_;

  // the actual data
  casa::Table table_;
  casa::Table originalTable_;

  STTcal tcalTable_;
  STFrequencies freqTable_;
  STWeather weatherTable_;
  STFocus focusTable_;
  STMolecules moleculeTable_;
  STHistory historyTable_;
  STFit fitTable_;

  // Cached Columns to avoid reconstructing them for each row get/put
  casa::ScalarColumn<casa::Double> integrCol_;
  casa::MDirection::ScalarColumn dirCol_;
  casa::MEpoch::ScalarColumn timeCol_;
  casa::ScalarColumn<casa::Float> azCol_;
  casa::ScalarColumn<casa::Float> elCol_;
  casa::ScalarColumn<casa::String> srcnCol_, fldnCol_;
  casa::ScalarColumn<casa::uInt> scanCol_, beamCol_, ifCol_, polCol_, cycleCol_, flagrowCol_;
  casa::ScalarColumn<casa::Int> rbeamCol_, srctCol_;
  casa::ArrayColumn<casa::Float> specCol_, tsysCol_;
  casa::ArrayColumn<casa::uChar> flagsCol_;

  // id in frequencies table
  casa::ScalarColumn<casa::uInt> mfreqidCol_;
  // id in tcal table
  casa::ScalarColumn<casa::uInt> mtcalidCol_;

  casa::ArrayColumn<casa::String> histitemCol_;
  casa::ScalarColumn<casa::Int> mfitidCol_;
  casa::ScalarColumn<casa::uInt> mweatheridCol_;

  casa::ScalarColumn<casa::uInt> mfocusidCol_;

  casa::ScalarColumn<casa::uInt> mmolidCol_;

  static std::map<std::string, STPol::STPolFactory *> factories_;
  void initFactories();

  /**
   * Add an auxiliary column to the main table and attach it to a
   * cached column. Use for adding new columns that the original asap2
   * tables do not have.
   * @param[in] col      reference to the cached column to be attached
   * @param[in] colName  column name in asap table
   * @param[in] defValue default value to fill in the column
   *
   * 25/10/2009 Wataru Kawasaki
   */
  template<class T, class T2> void attachAuxColumnDef(casa::ScalarColumn<T>&,
						       const casa::String&,
						       const T2&);
  template<class T, class T2> void attachAuxColumnDef(casa::ArrayColumn<T>&,
						      const casa::String&,
						      const casa::Array<T2>&);

  double getNormalPolynomial(int n, double x);
  double getChebyshevPolynomial(int n, double x);
  std::vector<std::vector<double> > getPolynomialModel(int order, 
						       int nchan, 
						       double (Scantable::*pfunc)(int, double));
  std::vector<std::vector<std::vector<double> > > getPolynomialModelReservoir(int order,
									      double (Scantable::*pfunc)(int, double),
									      std::vector<int>& nChanNos);
  std::vector<float> doPolynomialFitting(const std::vector<float>& data, 
					 const std::vector<bool>& mask,
					 int order,
					 std::vector<float>& params,
					 float& rms, 
					 std::vector<bool>& finalMask, 
					 float clipth, 
					 int clipn);
  std::vector<float> doPolynomialFitting(const std::vector<float>& data, 
					 const std::vector<bool>& mask,
					 int order,
					 std::vector<float>& params,
					 float& rms, 
					 std::vector<bool>& finalMask, 
					 int& nClipped,
					 float thresClip=3.0, 
					 int nIterClip=0,
					 bool getResidual=true);
  std::vector<float> doChebyshevFitting(const std::vector<float>& data, 
					const std::vector<bool>& mask,
					int order,
					std::vector<float>& params,
					float& rms, 
					std::vector<bool>& finalMask, 
					float clipth, 
					int clipn);
  std::vector<float> doChebyshevFitting(const std::vector<float>& data, 
					const std::vector<bool>& mask,
				        int order,
				        std::vector<float>& params,
				        float& rms, 
				        std::vector<bool>& finalMask, 
				        int& nClipped,
				        float thresClip=3.0, 
				        int nIterClip=0,
				        bool getResidual=true);
  std::vector<float> doLeastSquareFitting(const std::vector<float>& data, 
					  const std::vector<bool>& mask, 
					  const std::vector<std::vector<double> >& model, 
					  std::vector<float>& params, 
					  float& rms, 
					  std::vector<bool>& finalMask, 
					  int& nClipped, 
					  float thresClip=3.0, 
					  int nIterClip=0, 
					  bool getResidual=true);
  std::vector<float> doCubicSplineFitting(const std::vector<float>& data, 
					  const std::vector<bool>& mask,
					  std::vector<int>& idxEdge,
					  std::vector<float>& params,
					  float& rms, 
					  std::vector<bool>& finalMask, 
					  float clipth, 
					  int clipn);
  std::vector<float> doCubicSplineFitting(const std::vector<float>& data, 
					  const std::vector<bool>& mask,
					  int nPiece,
					  std::vector<int>& idxEdge,
					  std::vector<float>& params,
					  float& rms, 
					  std::vector<bool>& finalMask, 
					  float clipth, 
					  int clipn);
  std::vector<float> doCubicSplineFitting(const std::vector<float>& data, 
					  const std::vector<bool>& mask,
					  int nPiece,
					  bool useGivenPieceBoundary, 
					  std::vector<int>& idxEdge,
					  std::vector<float>& params,
					  float& rms, 
					  std::vector<bool>& finalMask, 
					  int& nClipped,
					  float thresClip=3.0, 
					  int nIterClip=0,
					  bool getResidual=true);
  std::vector<float> doCubicSplineLeastSquareFitting(const std::vector<float>& data, 
						     const std::vector<bool>& mask, 
						     const std::vector<std::vector<double> >& model, 
						     int nPiece, 
						     bool useGivenPieceBoundary, 
						     std::vector<int>& idxEdge, 
						     std::vector<float>& params, 
						     float& rms, 
						     std::vector<bool>& finalMask, 
						     int& nClipped, 
						     float thresClip=3.0, 
						     int nIterClip=0, 
						     bool getResidual=true);
  std::vector<float> doSinusoidFitting(const std::vector<float>& data, 
				       const std::vector<bool>& mask,
				       const std::vector<int>& waveNumbers,
				       std::vector<float>& params,
				       float& rms, 
				       std::vector<bool>& finalMask, 
				       float clipth, 
				       int clipn);
  std::vector<float> doSinusoidFitting(const std::vector<float>& data, 
				       const std::vector<bool>& mask,
				       const std::vector<int>& waveNumbers,
				       std::vector<float>& params,
				       float& rms, 
				       std::vector<bool>& finalMask, 
				       int& nClipped,
				       float thresClip=3.0, 
				       int nIterClip=0,
				       bool getResidual=true);
  std::vector<std::vector<double> > getSinusoidModel(const std::vector<int>& waveNumbers, int nchan);
  std::vector<std::vector<std::vector<double> > > getSinusoidModelReservoir(const std::vector<int>& waveNumbers,
									    std::vector<int>& nChanNos);
  std::vector<int> selectWaveNumbers(const std::vector<int>& addNWaves, 
				     const std::vector<int>& rejectNWaves);
  std::vector<int> selectWaveNumbers(const int whichrow, 
				     const std::vector<bool>& chanMask, 
				     //const std::string& fftInfo, 
				     const bool applyFFT, 
				     const std::string& fftMethod, 
				     const std::string& fftThresh, 
				     const std::vector<int>& addNWaves, 
				     const std::vector<int>& rejectNWaves);
  int getIdxOfNchan(const int nChan, const std::vector<int>& nChanNos);
  void parseFFTInfo(const std::string& fftInfo, 
		    bool& applyFFT, 
		    std::string& fftMethod, 
		    std::string& fftThresh);
  void parseFFTThresholdInfo(const std::string& fftThresh,
			     std::string& fftThAttr,
			     float& fftThSigma,
			     int& fftThTop);
  void doSelectWaveNumbers(const int whichrow,
			   const std::vector<bool>& chanMask, 
			   const std::string& fftMethod, 
			   const float fftThSigma, 
			   const int fftThTop, 
			   const std::string& fftThAttr, 
			   std::vector<int>& nWaves);
  void addAuxWaveNumbers(const int whichrow,
			 const std::vector<int>& addNWaves, 
			 const std::vector<int>& rejectNWaves, 
			 std::vector<int>& nWaves);
  void setWaveNumberListUptoNyquistFreq(const int whichrow,
					std::vector<int>& nWaves);
  void initialiseBaselining(const std::string& blfile, 
			    std::ofstream& ofs, 
			    const bool outLogger, 
			    bool& outTextFile, 
			    bool& csvFormat, 
			    casa::String& coordInfo, 
			    bool& hasSameNchan, 
			    const std::string& progressInfo, 
			    bool& showProgress, 
			    int& minNRow, 
			    casa::Vector<casa::Double>& timeSecCol);
  void finaliseBaselining(const bool outBaselineTable, 
			  STBaselineTable* pbt, 
			  const string& bltable, 
			  const bool outTextFile, 
			  std::ofstream& ofs);
  void initLineFinder(const std::vector<int>& edge, 
		      const float threshold, 
		      const int chanAvgLimit,
		      STLineFinder& lineFinder);
  bool hasSameNchanOverIFs();
  std::string getMaskRangeList(const std::vector<bool>& mask, 
				int whichrow, 
				const casa::String& coordInfo, 
				bool hasSameNchan,
				bool verbose=false);
  std::vector<int> getMaskEdgeIndices(const std::vector<bool>& mask);
  std::string formatBaselineParamsHeader(int whichrow, const std::string& masklist, bool verbose, bool csvformat) const;
  std::string formatBaselineParamsFooter(float rms, int nClipped, bool verbose, bool csvformat) const;
  std::vector<bool> getCompositeChanMask(int whichrow, const std::vector<bool>& inMask);
  std::vector<bool> getCompositeChanMask(int whichrow, const std::vector<bool>& inMask, const std::vector<int>& edge, std::vector<int>& currEdge, STLineFinder& lineFinder);
  void outputFittingResult(bool outLogger, bool outTextFile, bool csvFormat, const std::vector<bool>& chanMask, int whichrow, const casa::String& coordInfo, bool hasSameNchan, std::ofstream& ofs, const casa::String& funcName, const std::vector<int>& edge, const std::vector<float>& params, const int nClipped);
  void outputFittingResult(bool outLogger, bool outTextFile, bool csvFormat, const std::vector<bool>& chanMask, int whichrow, const casa::String& coordInfo, bool hasSameNchan, std::ofstream& ofs, const casa::String& funcName, const std::vector<float>& params, const int nClipped);
  void parseProgressInfo(const std::string& progressInfo, bool& showProgress, int& minNRow);
  void showProgressOnTerminal(const int nProcessed, const int nTotal, const bool showProgress=true, const int nTotalThreshold=1000);

  void applyChanFlag( casa::uInt whichrow, const std::vector<bool>& msk, casa::uChar flagval);

  double doCalculateModelSelectionCriteria(const std::string& valname, 
					   const std::vector<float>& spec, 
					   const std::vector<bool>& mask, 
					   const std::string& blfunc, 
					   int order);
  double doGetRms(const std::vector<bool>& mask, const casa::Vector<casa::Float>& spec);
  std::string packFittingResults(const int irow, const std::vector<float>& params, const float rms);
  void parseBlInfo(const std::string& blInfo, int& whichrow, STBaselineFunc::FuncName& ftype, std::vector<int>& fpar, std::vector<bool>& mask, float& thresClip, int& nIterClip, bool& useLineFinder, float& thresLF, std::vector<int>& edgeLF, int& avgLF);
  std::vector<float> doApplyBaselineTable(std::vector<float>& spec, std::vector<bool>& mask, const STBaselineFunc::FuncName ftype, std::vector<int>& fpar, std::vector<float>& params, float&rms, int irow);
 std::vector<float> doSubtractBaseline(std::vector<float>& spec, std::vector<bool>& mask, const STBaselineFunc::FuncName ftype, std::vector<int>& fpar, std::vector<float>& params, float&rms, std::vector<bool>& finalmask, float clipth, int clipn, bool uself, int irow, float lfth, std::vector<int>& lfedge, int lfavg);

 // storage of cubic spline model for various number of channels
 map<size_t, vector< vector<double> > > cubicSplineModelPool_;

};
} // namespace

#endif
