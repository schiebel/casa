#include <image_cmpt.h>

#include <iostream>
#include <sys/wait.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/fstream.h>
#include <casa/BasicSL/STLIO.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/Assert.h>
#include <components/ComponentModels/SkyCompRep.h>

#include <images/Images/ImageExpr.h>
#include <images/Images/ImageExprParse.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/LELImageCoord.h>
#include <images/Images/PagedImage.h>
#include <images/Images/RebinImage.h>
#include <images/Images/TempImage.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCLELMask.h>
#include <lattices/LatticeMath/Fit2D.h>
#include <lattices/LatticeMath/LatticeFit.h>
// #include <lattices/LatticeMath/LatticeAddNoise.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/LRegions/LatticeRegion.h>
#include <lattices/LatticeMath/LatticeSlice1D.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/LRegions/LCBox.h>
#include <lattices/LRegions/LCSlicer.h>
#include <lattices/Lattices/MaskedLatticeIterator.h>
#include <lattices/Lattices/PixelCurve1D.h>
#include <lattices/LRegions/RegionType.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <measures/Measures/Stokes.h>
#include <../casacore/measures/Measures/MeasIERS.h>
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <tables/LogTables/NewFile.h>

#include <components/ComponentModels/GaussianDeconvolver.h>
#include <components/SpectralComponents/SpectralListFactory.h>

#include <imageanalysis/ImageAnalysis/Image2DConvolver.h>
#include <imageanalysis/ImageAnalysis/BeamManipulator.h>
#include <imageanalysis/ImageAnalysis/CasaImageBeamSet.h>
#include <imageanalysis/ImageAnalysis/ComplexImageRegridder.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/ImageBoxcarSmoother.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/ImageConcatenator.h>
#include <imageanalysis/ImageAnalysis/ImageCropper.h>
#include <imageanalysis/ImageAnalysis/ImageDecimator.h>
#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/ImageFFTer.h>
#include <imageanalysis/ImageAnalysis/ImageFitter.h>
#include <imageanalysis/ImageAnalysis/ImageHanningSmoother.h>
#include <imageanalysis/ImageAnalysis/ImageHistory.h>
#include <imageanalysis/ImageAnalysis/ImageMaskedPixelReplacer.h>
#include <imageanalysis/ImageAnalysis/ImagePadder.h>
#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>
#include <imageanalysis/ImageAnalysis/ImagePrimaryBeamCorrector.h>
#include <imageanalysis/ImageAnalysis/ImageRebinner.h>
#include <imageanalysis/ImageAnalysis/ImageRegridder.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <imageanalysis/ImageAnalysis/ImageTransposer.h>
#include <imageanalysis/ImageAnalysis/PeakIntensityFluxDensityConverter.h>
#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>
#include <imageanalysis/ImageAnalysis/PVGenerator.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

#include <stdcasa/version.h>

#include <casa/namespace.h>

#include <stdcasa/cboost_foreach.h>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;
using namespace std;

#define _ORIGIN LogOrigin(_class, __func__, WHERE)

namespace casac {

const String image::_class = "image";

image::image() :
_log(), _image(new ImageAnalysis()) {}

// private ImageInterface constructor for on the fly components
// The constructed object will take over management of the provided pointer
// using a shared_ptr

image::image(casa::ImageInterface<casa::Float> *inImage) :
	_log(), _image(new ImageAnalysis(SHARED_PTR<ImageInterface<Float> >(inImage))) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(ImageInterface<Complex> *inImage) :
	_log(), _image(new ImageAnalysis(SHARED_PTR<ImageInterface<Complex> >(inImage))) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(SPIIF inImage) :
	_log(), _image(new ImageAnalysis(inImage)) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(SPIIC inImage) :
	_log(), _image(new ImageAnalysis(inImage)) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(SHARED_PTR<ImageAnalysis> ia) :
	_log(), _image(ia) {
	try {
		_log << _ORIGIN;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::~image() {}

Bool isunset(const ::casac::variant &theVar) {
	Bool rstat(False);
	if ((theVar.type() == ::casac::variant::BOOLVEC) && (theVar.size() == 0))
		rstat = True;
	return rstat;
}

::casac::record*
image::torecord() {
	::casac::record *rstat = new ::casac::record();
	_log << LogOrigin("image", "torecord");
	if (detached())
		return rstat;
	try {
		Record rec;
		if (!_image->toRecord(rec)) {
			_log << "Could not convert to record " << LogIO::EXCEPTION;
		}

		// Put it in a ::casac::record
		delete rstat;
		rstat = 0;
		rstat = fromRecord(rec);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;

}
bool image::fromrecord(const record& imrecord, const string& outfile) {
	try {
		_log << _ORIGIN;
		std::unique_ptr<Record> tmpRecord(toRecord(imrecord));
		if (_image.get() == 0) {
			_image.reset(new ImageAnalysis());
		}
		if (
			! _image->fromRecord(*tmpRecord, casa::String(outfile))
		) {
			_log << "Failed to create a valid image from this record"
					<< LogIO::EXCEPTION;
		}
		return True;
	} catch (const AipsError& x) {
		RETHROW(x);
	}
}

bool image::addnoise(
	const std::string& type, const std::vector<double>& pars,
	const variant& region, const bool zeroIt, const vector<int>& seeds
) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return False;
		}
		SHARED_PTR<Record> pRegion = _getRegion(region, False);
		SHARED_PTR<std::pair<Int, Int> > seedPair(new std::pair<Int, Int>(0, 0));
		//Int seed1, seed2;
		if (seeds.size() >= 2) {
			seedPair->first = seeds[0];
			seedPair->second = seeds[1];
		}
		else {
			Time now;
			Double seedBase = 1e7*now.modifiedJulianDay();
			seedPair->second = (Int)((uInt)seedBase);
			seedPair->first = seeds.size() == 1
				? seeds[0]
				: (Int)((uInt)(1e7*(seedBase - seedPair->second)));
		}
		_image->addnoise(type, pars, *pRegion, zeroIt, seedPair.get());

		_stats.reset(0);
		return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

// FIXME need to support region records as input
image * image::collapse(
	const string& function, const variant& axes,
	const string& outfile, const variant& region, const string& box,
	const string& chans, const string& stokes, const string& mask,
	const bool overwrite, const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return 0;
	}
	try {
		IPosition myAxes;
		if (axes.type() == variant::INT) {
			myAxes = IPosition(1, axes.toInt());
		}
		else if (axes.type() == variant::INTVEC) {
			myAxes = IPosition(axes.getIntVec());
		}
		else if (
			axes.type() == variant::STRINGVEC
			|| axes.type() == variant::STRING
		) {
			Vector<String> axVec = (axes.type() == variant::STRING)
				? Vector<String> (1, axes.getString())
				: toVectorString(axes.toStringVec());
			myAxes = IPosition(
				_image->isFloat()
				? _image->getImage()->coordinates().getWorldAxesOrder(
					axVec, False
				)
				: _image->getComplexImage()->coordinates().getWorldAxesOrder(
					axVec, False
				)
			);
			for (
				IPosition::iterator iter = myAxes.begin();
				iter != myAxes.end(); iter++
			) {
				ThrowIf(
					*iter < 0,
					"At least one specified axis does not exist"
				);
			}
		}
		else {
			ThrowCc("Unsupported type for parameter axes");
		}
		SHARED_PTR<Record> regRec = _getRegion(region, True);
		String aggString = function;
		aggString.trim();
		aggString.downcase();
		if (aggString == "avdev") {
			_log << "avdev currently not supported. Let us know if you have a need for it"
				<< LogIO::EXCEPTION;
		}
		String myStokes = stokes;
		if (_image->isFloat()) {
			SPCIIF myimage = _image->getImage();
			CasacRegionManager rm(myimage->coordinates());
			String diagnostics;
			uInt nSelectedChannels;
			Record myreg = rm.fromBCS(
				diagnostics, nSelectedChannels, myStokes, regRec.get(),
				"", chans, CasacRegionManager::USE_ALL_STOKES, box,
				myimage->shape(), mask, False
			);
			ImageCollapser<Float> collapser(
				aggString, myimage, &myreg,
				mask, myAxes, False, outfile, overwrite
			);
			collapser.setStretch(stretch);
			return new image(collapser.collapse());
		}
		else {
			SPCIIC myimage = _image->getComplexImage();
			CasacRegionManager rm(myimage->coordinates());
			String diagnostics;
			uInt nSelectedChannels;
			Record myreg = rm.fromBCS(
				diagnostics, nSelectedChannels, myStokes, regRec.get(),
				"", chans, CasacRegionManager::USE_ALL_STOKES, box,
				myimage->shape(), mask, False
			);
			ImageCollapser<Complex> collapser(
				aggString, myimage, &myreg,
				mask, myAxes, False, outfile, overwrite
			);
			collapser.setStretch(stretch);
			return new image(collapser.collapse());
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::imagecalc(
	const string& outfile, const string& pixels,
	const bool overwrite
) {
	try {
		SHARED_PTR<ImageAnalysis> ia(new ImageAnalysis());
        ia->imagecalc(outfile, pixels, overwrite);
        return  new image(ia);
	}
	catch (const AipsError& x) {
		RETHROW(x);
	}
}

image* image::imageconcat(
	const string& outfile, const variant& infiles,
	int axis, bool relax, bool tempclose,
	bool overwrite, bool reorder
) {
	try {
		Vector<String> inFiles;
		if (infiles.type() == variant::BOOLVEC) {
			inFiles.resize(0); // unset
		}
		else if (infiles.type() == variant::STRING) {
			sepCommaEmptyToVectorStrings(inFiles, infiles.toString());
		}
		else if (infiles.type() == variant::STRINGVEC) {
			inFiles = toVectorString(infiles.toStringVec());
		}
		else {
			ThrowCc("Unrecognized infiles datatype");
		}
		vector<String> imageNames = Directory::shellExpand(inFiles, False).tovector();
		ThrowIf(
			imageNames.size() < 2,
			"You must provide at least two images to concatentate"
		);
		String first = imageNames[0];
		imageNames.erase(imageNames.begin());

		SPtrHolder<LatticeBase> latt(ImageOpener::openImage(first));
		ThrowIf (! latt.ptr(), "Unable to open image " + first);
		DataType dataType = latt->dataType();
		if (dataType == TpComplex) {
			SPIIC c(dynamic_cast<ImageInterface<Complex> *>(latt.transfer()));
			ImageConcatenator<Complex> concat(c, outfile, overwrite);
			concat.setAxis(axis);
			concat.setRelax(relax);
			concat.setReorder(reorder);
			concat.setTempClose(tempclose);
			return new image(concat.concatenate(imageNames));
		}
		else if (dataType == TpFloat) {
			SPIIF f(dynamic_cast<ImageInterface<Float> *>(latt.transfer()));
			ImageConcatenator<Float> concat(f, outfile, overwrite);
			concat.setAxis(axis);
			concat.setRelax(relax);
			concat.setReorder(reorder);
			concat.setTempClose(tempclose);
			return new image(concat.concatenate(imageNames));
		}
		else {
			ostringstream x;
			x << dataType;
			ThrowCc("Unsupported data type " + x.str());
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromarray(const std::string& outfile,
		const ::casac::variant& pixels, const ::casac::record& csys,
		const bool linear, const bool overwrite, const bool log) {
	try {
		Vector<Int> shape = pixels.arrayshape();
		ThrowIf(
			shape.ndim() == 0,
			"The pixels array cannot be empty"
		);
		Array<Float> floatArray;
		Array<Complex> complexArray;
		if (pixels.type() == variant::DOUBLEVEC) {
			std::vector<double> pixelVector = pixels.getDoubleVec();
			floatArray.resize(IPosition(shape));
			Vector<Double> localpix(pixelVector);
			convertArray(floatArray, localpix.reform(IPosition(shape)));
		}
		else if (pixels.type() == ::casac::variant::INTVEC) {
			vector<int> pixelVector = pixels.getIntVec();
			floatArray.resize(IPosition(shape));
			Vector<Int> localpix(pixelVector);
			convertArray(floatArray, localpix.reform(IPosition(shape)));
		}
		else if (pixels.type() == ::casac::variant::COMPLEXVEC) {
			vector<std::complex<double> > pixelVector = pixels.getComplexVec();
			complexArray.resize(IPosition(shape));
			Vector<DComplex> localpix(pixelVector);
			convertArray(complexArray, localpix.reform(IPosition(shape)));
		}
		else {
			ThrowCc(
				"pixels is not understood, try using an array "
			);
		}
		LogOrigin lor("image", __func__);
		_log << lor;
		_reset();
		std::unique_ptr<Record> coordinates(toRecord(csys));
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(lor, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("csys", csys));
			inputs.push_back(make_pair("linear", linear));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("log", log));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(lor, os.str()));
		}
		if (floatArray.ndim() > 0) {
			SPIIF myfloat = ImageFactory::imageFromArray(
				outfile, floatArray, *coordinates,
				linear, overwrite, log, &msgs
			);
			_image.reset(new ImageAnalysis(myfloat));
		}
		else {
			SPIIC mycomplex = ImageFactory::imageFromArray(
				outfile, complexArray, *coordinates,
				linear, overwrite, log, &msgs
			);
			_image.reset(new ImageAnalysis(mycomplex));
		}
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

void image::_reset() {
	_image.reset(new ImageAnalysis());
	//_imageFloat.reset();
	//_imageComplex.reset();
	_stats.reset(0);
}

bool image::fromascii(const string& outfile, const string& infile,
		const vector<int>& shape, const string& sep, const record& csys,
		const bool linear, const bool overwrite) {
	try {
		_log << _ORIGIN;

		if (infile == "") {
			_log << LogIO::SEVERE << "infile must be specified" << LogIO::POST;
			return false;
		}
		if (shape.size() == 1 && shape[0] == -1) {
			_log << LogIO::SEVERE << "Image shape must be specified"
					<< LogIO::POST;
			return false;
		}

		_reset();
		std::unique_ptr<Record> coordsys(toRecord(csys));
		if (
			_image->imagefromascii(
				outfile, infile, Vector<Int> (shape),
				sep, *coordsys, linear, overwrite
			)
		) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error creating image from ascii.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromfits(const std::string& outfile, const std::string& fitsfile,
		const int whichrep, const int whichhdu, const bool zeroBlanks,
		const bool overwrite) {
	try {
		_reset();
		_log << _ORIGIN;
		return _image->imagefromfits(outfile, fitsfile, whichrep, whichhdu,
				zeroBlanks, overwrite);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromimage(const string& outfile, const string& infile,
		const variant& region, const variant& mask, const bool dropdeg,
		const bool overwrite) {
	try {
		_reset();

		_log << _ORIGIN;
		String theMask;
		if (mask.type() == variant::BOOLVEC) {
			theMask = "";
		} else if (mask.type() == variant::STRING) {
			theMask = mask.toString();
		} else if (mask.type() == variant::RECORD) {
			_log << LogIO::SEVERE
					<< "Don't support region masking yet, only valid LEL "
					<< LogIO::POST;
			return False;
		} else {
			_log << LogIO::SEVERE
					<< "Mask is not understood, try a valid LEL string "
					<< LogIO::POST;
			return False;
		}

		SHARED_PTR<Record> regionPtr(_getRegion(region, False));
		return _image->imagefromimage(outfile, infile, *regionPtr, theMask,
				dropdeg, overwrite);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromshape(
	const string& outfile, const vector<int>& shape,
	const record& csys, const bool linear, const bool overwrite,
	const bool log, const string& type
) {
	try {
		LogOrigin lor("image", __func__);
		_log << lor;
		_reset();
		std::unique_ptr<Record> coordinates(toRecord(csys));
        String mytype = type;
        mytype.downcase();
        ThrowIf(
            ! mytype.startsWith("f") && ! mytype.startsWith("c"),
            "Input parm type must start with either 'f' or 'c'"
        );
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(lor, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("shape", shape));
			inputs.push_back(make_pair("csys", csys));
			inputs.push_back(make_pair("linear", linear));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("log", log));
			inputs.push_back(make_pair("type", type));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(lor, os.str()));
		}
        if (mytype.startsWith("f")) {
            SPIIF myfloat = ImageFactory::floatImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            _image.reset(new ImageAnalysis(myfloat));
        }
        else {
            SPIIC mycomplex = ImageFactory::complexImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            _image.reset(new ImageAnalysis(mycomplex));
        }
       	return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image *
image::adddegaxes(
	const std::string& outfile, bool direction,
	bool spectral, const std::string& stokes, bool linear,
	bool tabular, bool overwrite, bool silent
) {
	try {
        _log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		if (_image->isFloat()) {
			SPCIIF image = _image->getImage();
			return _adddegaxes(
				image,
				outfile, direction, spectral, stokes, linear,
				tabular, overwrite, silent
			);
		}
		else {
			SPCIIC image = _image->getComplexImage();
			return _adddegaxes(
				image,
				outfile, direction, spectral, stokes,
				linear, tabular, overwrite, silent
			);
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

template<class T> image* image::_adddegaxes(
	SPCIIT inImage,
	const std::string& outfile, bool direction,
		bool spectral, const std::string& stokes, bool linear,
		bool tabular, bool overwrite, bool silent
) {
	_log << _ORIGIN;
	PtrHolder<ImageInterface<T> > outimage;
	ImageUtilities::addDegenerateAxes(
		_log, outimage, *inImage, outfile,
		direction, spectral, stokes, linear, tabular, overwrite, silent
	);
	ImageInterface<T> *outPtr = outimage.ptr();
	outimage.clear(False);
	return new image(outPtr);
}

::casac::image* image::convolve(
	const string& outFile, const variant& kernel,
	const double in_scale, const variant& region,
	const variant& vmask, const bool overwrite,
	const bool stretch, const bool async
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return nullptr;
		}
		Array<Float> kernelArray;
		casa::String kernelFileName = "";
		if (kernel.type() == variant::DOUBLEVEC) {
			const auto kernelVector = kernel.toDoubleVec();
			const auto shape = kernel.arrayshape();
			kernelArray.resize(IPosition(shape));
			Vector<Double> localkern(kernelVector);
			convertArray(kernelArray, localkern.reform(IPosition(shape)));
		}
		else if (kernel.type() == variant::INTVEC) {
			const auto kernelVector = kernel.toIntVec();
			const auto shape = kernel.arrayshape();
			kernelArray.resize(IPosition(shape));
			Vector<Int> localkern(kernelVector);
			convertArray(kernelArray, localkern.reform(IPosition(shape)));
		}
		else if (
			kernel.type() == variant::STRING
			|| kernel.type() == variant::STRINGVEC
		) {
			kernelFileName = kernel.toString();
		}
		else {
			ThrowCc("kernel is not understood, try using an array or an image");
		}

		String theMask;
		//Record *theMaskRegion;
		if (vmask.type() == variant::BOOLVEC) {
			theMask = "";
		}
		else if (
			vmask.type() == variant::STRING
			|| vmask.type() == variant::STRINGVEC
		) {
			theMask = vmask.toString();
		}
		else if (vmask.type() == variant::RECORD) {
			/*
			variant localvar(vmask);
			theMaskRegion = toRecord(localvar.asRecord());
			*/
			ThrowCc("Don't support region masking yet, only valid LEL");
		}
		else {
			_log << "Mask is not understood, try a valid LEL string "
				<< LogIO::EXCEPTION;
		}

		SHARED_PTR<Record> Region(_getRegion(region, False));
		return new image(
			_image->convolve(
				outFile, kernelArray,
				kernelFileName, in_scale, *Region,
				theMask, overwrite, async, stretch
			)
		);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
	return nullptr;
}

::casac::record* image::boundingbox(const variant& region) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return nullptr;
		}
		SHARED_PTR<Record> Region(_getRegion(region, False));
		return fromRecord(*_image->boundingbox(*Region));
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
	return nullptr;
}

image* image::boxcar(
	const string& outfile, const variant& region,
	const variant& vmask, int axis, int width, bool drop,
	const string& dmethod,
	bool overwrite, bool stretch
) {
	LogOrigin lor(_class, __func__);
	_log << lor;
	if (detached()) {
		throw AipsError("Unable to create image");
	}
	try {
		SHARED_PTR<const Record> myregion = _getRegion(
			region, True
		);
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (axis < 0) {
			const CoordinateSystem csys = _image->isFloat()
				? _image->getImage()->coordinates()
				: _image->getComplexImage()->coordinates();
			ThrowIf(
				! csys.hasSpectralAxis(),
				"Axis not specified and image has no spectral coordinate"
			);
			axis = csys.spectralAxisNumber(False);
		}
        ImageDecimatorData::Function dFunction = ImageDecimatorData::NFUNCS;
        if (drop) {
            String mymethod = dmethod;
            mymethod.downcase();
            if (mymethod.startsWith("m")) {
                dFunction = ImageDecimatorData::MEAN;
            }
            else if (mymethod.startsWith("c")) {
                dFunction = ImageDecimatorData::COPY;
            }
            else {
                ThrowCc(
                    "Value of dmethod must be "
                    "either 'm'(ean) or 'c'(opy)"
                );
            }
        }
		vector<String> msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__ << "() on image " << _name();
			msgs.push_back(os.str());
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", vmask));
			inputs.push_back(make_pair("axis", axis));
			inputs.push_back(make_pair("width", width));
			inputs.push_back(make_pair("drop", drop));
			inputs.push_back(make_pair("dmethod", dmethod));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("stretch", stretch));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(os.str());
		}
		if (_image->isFloat()) {
			SPCIIF image = _image->getImage();
			return _boxcar(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, width, drop,
				dFunction, lor, msgs
			);
		}
		else {
			SPCIIC image = _image->getComplexImage();
			return _boxcar(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, width, drop,
				dFunction, lor, msgs
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

template <class T> image* image::_boxcar(
	SPCIIT myimage, SHARED_PTR<const Record> region,
	const String& mask, const string& outfile, bool overwrite,
	bool stretch, int axis, int width, bool drop,
	ImageDecimatorData::Function dFunction, const LogOrigin& lor,
	const vector<String> msgs
) {
	ImageBoxcarSmoother<T> smoother(
		myimage, region.get(), mask, outfile, overwrite
	);
	smoother.setAxis(axis);
	smoother.setDecimate(drop);
	smoother.setStretch(stretch);
	if (drop) {
		smoother.setDecimationFunction(dFunction);
	}
	smoother.setWidth(width);
	smoother.addHistory(lor, msgs);
	return new image(smoother.smooth());
}

std::string image::brightnessunit() {
	std::string rstat;
	try {
		_log << _ORIGIN;
		if (!detached()) {
			rstat = _image->brightnessunit();
		} else {
			rstat = "";
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::calc(const std::string& expr, bool verbose) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}
		_image->calc(expr, verbose);
		_stats.reset(0);
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::calcmask(
	const string& mask, const string& maskName, bool makeDefault
) {
	bool rstat(false);
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}
		Record region;
		rstat = _image->calcmask(mask, region, maskName, makeDefault);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::close() {
	try {
		_log << _ORIGIN;
		_image.reset();
		_stats.reset(0);
        MeasIERS::closeTables();
        return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::continuumsub(
	const string& outline, const string& outcont,
	const variant& region, const vector<int>& channels,
	const string& pol, const int in_fitorder, const bool overwrite
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(in_fitorder < 0, "Polynomial order cannot be negative");
		if (! pol.empty()) {
			_log << LogIO::NORMAL << "The pol parameter is no longer "
				<< "supported and will be removed in the near future. "
				<< "Please set the region parameter appropriately "
				<< "to select the polarization in which you are interested."
				<< LogIO::POST;
		}
		SHARED_PTR<Record> leRegion = _getRegion(region, False);
		vector<Int> planes = channels;
		if (planes.size() == 1 && planes[0] == -1) {
			planes.resize(0);
		}
		Int spectralAxis = _image->getImage()->coordinates().spectralAxisNumber();
		ThrowIf(spectralAxis < 0, "This image has no spectral axis");
		ImageProfileFitter fitter(
		_image->getImage(), "", leRegion.get(),
			"", "", "", "", spectralAxis,
			0, overwrite
		);
		fitter.setDoMultiFit(True);
		fitter.setPolyOrder(in_fitorder);
		fitter.setModel(outcont);
		fitter.setResidual(outline);
		fitter.setStretch(False);
		fitter.setLogResults(False);
		if (! planes.empty()) {
			std::set<int> myplanes(planes.begin(), planes.end());
			ThrowIf(*myplanes.begin() < 0, "All planes must be nonnegative");
			fitter.setGoodPlanes(std::set<uInt>(myplanes.begin(), myplanes.end()));
		}
		fitter.createResidualImage(True);
		fitter.fit(False);
		return new image(fitter.getResidual());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

/*
image* image::continuumsub(
	const string& outline, const string& outcont,
	const variant& region, const vector<int>& channels,
	const string& pol, const int in_fitorder, const bool overwrite
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		SHARED_PTR<Record> leRegion = _getRegion(region, False);
		Vector<Int> theChannels(channels);
		if (theChannels.size() == 1 && theChannels[0] == -1) {
			theChannels.resize(0);
		}
		SHARED_PTR<ImageInterface<Float> > theResid(
			_image->continuumsub(
				outline, outcont, *leRegion, theChannels,
				pol, in_fitorder, overwrite
			)
		);
		ThrowIf(
			!theResid,
			"continuumsub failed"
		);
		return new image(theResid);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}
*/

record* image::convertflux(
	const variant& qvalue, const variant& major,
	const variant& minor,  const string& /*type*/,
	const bool toPeak,
	const int channel, const int polarization
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		casa::Quantity value = casaQuantity(qvalue);
		casa::Quantity majorAxis = casaQuantity(major);
		casa::Quantity minorAxis = casaQuantity(minor);
		Bool noBeam = False;
		PeakIntensityFluxDensityConverter converter(_image->getImage());
		converter.setSize(
			Angular2DGaussian(majorAxis, minorAxis, casa::Quantity(0, "deg"))
		);
		converter.setBeam(channel, polarization);
		return recordFromQuantity(
			toPeak
			? converter.fluxDensityToPeakIntensity(noBeam, value)
			: converter.peakIntensityToFluxDensity(noBeam, value)
		);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

image* image::convolve2d(
	const string& outFile, const vector<int>& axes,
	const string& type, const variant& major, const variant& minor,
	const variant& pa, const double in_scale, const variant& region,
	const variant& vmask, const bool overwrite, const bool stretch,
	const bool targetres, const record& beam
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			throw AipsError("Unable to create image");
		}
		UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();

		if (mask == "[]") {
			mask = "";
		}
		String kernel(type);
		casa::Quantity majorKernel;
		casa::Quantity minorKernel;
		casa::Quantity paKernel;
		_log << _ORIGIN;
		if (! beam.empty()) {
			if (! String(type).startsWith("g") && ! String(type).startsWith("G")) {
				_log << "beam can only be given with a gaussian kernel" << LogIO::EXCEPTION;
			}
			if (
				! major.toString(False).empty()
				|| ! minor.toString(False).empty()
				|| ! pa.toString(False).empty()
			) {
				_log << "major, minor, and/or pa may not be specified if beam is specified"
					<< LogIO::EXCEPTION;
			}
			if (beam.size() != 3) {
				_log << "If given, beam must have exactly three fields"
					<< LogIO::EXCEPTION;
			}
			if (beam.find("major") == beam.end()) {
				_log << "Beam must have a 'major' field" << LogIO::EXCEPTION;
			}
			if (beam.find("minor") == beam.end()) {
				_log << "Beam must have a 'minor' field" << LogIO::EXCEPTION;
			}
			if (
				beam.find("positionangle") == beam.end()
				&& beam.find("pa") == beam.end()) {
				_log << "Beam must have a 'positionangle' or 'pa' field" << LogIO::EXCEPTION;
			}
			std::unique_ptr<Record> nbeam(toRecord(beam));

			for (uInt i=0; i<3; i++) {
				String key = i == 0
					? "major"
					: i == 1
					    ? "minor"
					    : beam.find("pa") == beam.end()
					        ? "positionangle"
					        : "pa";
				casa::Quantity x;
				DataType type = nbeam->dataType(nbeam->fieldNumber(key));
				String err;
				QuantumHolder z;
				Bool success;
				if (type == TpString) {
					success = z.fromString(err, nbeam->asString(key));
				}
				else if (type == TpRecord) {
					success = z.fromRecord(err, nbeam->asRecord(key));
				}
				else {
					throw AipsError("Unsupported data type for beam");
				}
				if (! success) {
					throw AipsError("Error converting beam to Quantity");
				}
				if (key == "major") {
					majorKernel = z.asQuantity();
				}
				else if (key == "minor") {
					minorKernel = z.asQuantity();
				}
				else {
					paKernel = z.asQuantity();
				}
			}
		}
		else {
			majorKernel = _casaQuantityFromVar(major);
			minorKernel = _casaQuantityFromVar(minor);
			paKernel = _casaQuantityFromVar(pa);
		}
		_log << _ORIGIN;

		Vector<Int> Axes(axes);
		if (Axes.size() == 0) {
			Axes.resize(2);
			Axes[0] = 0;
			Axes[1] = 1;
		}
		else {
			ThrowIf(
				axes.size() != 2,
				"Number of axes to convolve must be exactly 2"
			);
		}
		Image2DConvolver<Float> convolver(
			_image->getImage(), Region.get(), mask, outFile, overwrite
		);
		convolver.setAxes(std::make_pair(Axes[0], Axes[1]));
		convolver.setKernel(type, majorKernel, minorKernel, paKernel);
		convolver.setScale(in_scale);
		convolver.setStretch(stretch);
		convolver.setTargetRes(targetres);
		return new image(convolver.convolve());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

::casac::coordsys *
image::coordsys(const std::vector<int>& pixelAxes) {

	_log << _ORIGIN;

	try {
		if (detached()) {
			return 0;
		}
		std::unique_ptr<casac::coordsys> rstat;
		// Return coordsys object
		rstat.reset(new ::casac::coordsys());
		CoordinateSystem csys = _image->coordsys(Vector<Int> (pixelAxes));
		rstat->setcoordsys(csys);
		return rstat.release();
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::decimate(
	const string& outfile, int axis, int factor, const string& method,
	const variant& region, const string& mask, bool overwrite, bool stretch
) {
	ThrowIf(
		detached(),
		"Tool is not attached to an image"
	);
	try {
		ThrowIf(
			axis < 0,
			"The value of axis cannot be negative"
		);
		ThrowIf(
			factor < 0,
			"The value of factor cannot be negative"
		);
		String mymethod = method;
		mymethod.downcase();
		ImageDecimatorData::Function f;
		if (mymethod.startsWith("c")) {
			f = ImageDecimatorData::COPY;
		}
		else if (mymethod.startsWith("m")) {
			f = ImageDecimatorData::MEAN;
		}
		else {
			ThrowCc("Unsupported decimation method " + method);
		}
		SHARED_PTR<Record> regPtr(_getRegion(region, True));
		vector<String> msgs;
		{
			msgs.push_back("Ran ia.decimate() on image " + _name());
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("axis", axis));
			inputs.push_back(make_pair("factor", factor));
			inputs.push_back(make_pair("method", method));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", mask));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("stretch", stretch));
			msgs.push_back("ia.decimate" + _inputsString(inputs));
		}
		if (_image->isFloat()) {
			SPCIIF myim = _image->getImage();
			return _decimate(
				myim, outfile, axis, factor, f,
				regPtr, mask, overwrite, stretch, msgs
			);
		}
		else {
			SPCIIC myim = _image->getComplexImage();
			return _decimate(
				myim, outfile, axis, factor, f,
				regPtr, mask, overwrite, stretch, msgs
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

String image::_inputsString(const vector<std::pair<String, variant> >& inputs) {
	String out = "(";
	String quote;
	vector<std::pair<String, variant> >::const_iterator begin = inputs.begin();
	vector<std::pair<String, variant> >::const_iterator iter = begin;
	vector<std::pair<String, variant> >::const_iterator end = inputs.end();
	while (iter != end) {
		if (iter != begin) {
			out += ", ";
		}
		quote = iter->second.type() == variant::STRING ? "'" : "";
		out += iter->first + "=" + quote;
		out += iter->second.toString();
		out += quote;
		iter++;
	}
	out += ")";
	return out;
}

template <class T> image* image::_decimate(
	const SPCIIT myimage,
	const string& outfile, int axis, int factor,
	ImageDecimatorData::Function f,
	const SHARED_PTR<Record> region,
	const string& mask, bool overwrite, bool stretch,
	const vector<String>& msgs
) const {
	ImageDecimator<T> decimator(
		myimage, region.get(),
		mask, outfile, overwrite
	);
	decimator.setFunction(f);
	decimator.setAxis(axis);
	decimator.setFactor(factor);
	decimator.setStretch(stretch);
	decimator.addHistory(
		LogOrigin("image", __func__), msgs
	);
	SPIIT out = decimator.decimate();
	return new image(out);
}

::casac::record*
image::coordmeasures(const std::vector<double>&pixel) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached())
			return rstat;

		casa::Record theDir;
		casa::Record theFreq;
		casa::Record theVel;
		casa::Record* retval;
		casa::Quantity theInt;
		Vector<Double> vpixel;
		if (!(pixel.size() == 1 && pixel[0] == -1)) {
			vpixel = pixel;
		}
		retval = _image->coordmeasures(theInt, theDir, theFreq, theVel, vpixel);

		String error;
		Record R;
		if (QuantumHolder(theInt).toRecord(error, R)) {
			retval->defineRecord(RecordFieldId("intensity"), R);
		} else {
			_log << LogIO::SEVERE << "Could not convert intensity to record. "
					<< error << LogIO::POST;
		}
		rstat = fromRecord(*retval);

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

::casac::record*
image::decompose(const variant& region, const ::casac::variant& vmask,
	const bool simple, const double Threshold, const int nContour,
	const int minRange, const int nAxis, const bool fit,
	const double maxrms, const int maxRetry, const int maxIter,
	const double convCriteria, const bool stretch
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			Threshold < 0,
			"Threshold = " + String::toString(Threshold)
			+ ". You must specify a nonnegative threshold"
		);
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Matrix<Int> blcs;
		Matrix<Int> trcs;

		Matrix<Float> cl = _image->decompose(
			blcs, trcs, *Region, mask, simple, Threshold,
			nContour, minRange, nAxis, fit, maxrms,
			maxRetry, maxIter, convCriteria, stretch
		);

		casa::Record outrec1;
		outrec1.define("components", cl);
		outrec1.define("blc", blcs);
		outrec1.define("trc", trcs);
		return fromRecord(outrec1);

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::deconvolvecomponentlist(
	const record& complist, const int channel, const int polarization
) {
	_log << _ORIGIN;
	if (detached()) {
		return 0;
	}
	try {
		std::unique_ptr<Record> compList(toRecord(complist));
		return fromRecord(
			_image->deconvolvecomponentlist(
				*compList, channel, polarization
			)
		);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::deconvolvefrombeam(
	const ::casac::variant& source,
	const ::casac::variant& beam
) {

	try {
		_log << _ORIGIN;
		Vector<casa::Quantity> sourceParam, beamParam;
		Angular2DGaussian mySource;
		if (
			! toCasaVectorQuantity(source, sourceParam)
			|| (sourceParam.nelements() == 0)
			|| sourceParam.nelements() > 3
		) {
			throw(AipsError("Cannot understand source values"));
		}
		else {
			if (sourceParam.nelements() == 1) {
				sourceParam.resize(3, True);
				sourceParam[1] = sourceParam[0];
				sourceParam[2] = casa::Quantity(0, "deg");
			}
			else if (sourceParam.nelements() == 2) {
				sourceParam.resize(3, True);
				sourceParam[2] = casa::Quantity(0, "deg");
			}
			mySource = Angular2DGaussian(
				sourceParam[0], sourceParam[1], sourceParam[2]
			);
		}
		if (
			! toCasaVectorQuantity(beam, beamParam)
			|| (beamParam.nelements() == 0)) {
			throw(AipsError("Cannot understand beam values"));
		}
		else {
			if (beamParam.nelements() == 1) {
				beamParam.resize(3, True);
				beamParam[1] = beamParam[0];
				beamParam[2] = casa::Quantity(0.0, "deg");
			}
			if (beamParam.nelements() == 2) {
				beamParam.resize(3, True);
				beamParam[2] = casa::Quantity(0.0, "deg");
			}
		}
		GaussianBeam myBeam(beamParam[0], beamParam[1], beamParam[2]);
		Bool success = False;
		Angular2DGaussian decon;
		Bool retval = False;
		try {
			retval = GaussianDeconvolver::deconvolve(decon, mySource, myBeam);
			success = True;
		}
		catch (const AipsError& x) {
			retval = False;
			success = False;
		}
		Record deconval = decon.toRecord();
		deconval.defineRecord("pa", deconval.asRecord("positionangle"));
		deconval.removeField("positionangle");
		deconval.define("success", success);
		Record outrec1;
		outrec1.define("return", retval);
		outrec1.defineRecord("fit", deconval);
		return fromRecord(outrec1);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}


record* image::beamforconvolvedsize(
	const variant& source, const variant& convolved
) {

	try {
		_log << _ORIGIN;
		Vector<casa::Quantity> sourceParam, convolvedParam;
		if (
			! toCasaVectorQuantity(source, sourceParam)
			|| sourceParam.size() != 3
		) {
			throw(AipsError("Cannot understand source values"));
		}
		if (
			! toCasaVectorQuantity(convolved, convolvedParam)
			&& convolvedParam.size() != 3
		) {
			throw(AipsError("Cannot understand target values"));
		}
		Angular2DGaussian mySource(sourceParam[0], sourceParam[1], sourceParam[2]);
		GaussianBeam myConvolved(convolvedParam[0], convolvedParam[1], convolvedParam[2]);
		GaussianBeam neededBeam;
		try {
			if (GaussianDeconvolver::deconvolve(neededBeam, myConvolved, mySource)) {
				// throw without a message here, it will be caught
				// in the associated catch block and a new error will
				// be thrown with the appropriate message.
				throw AipsError();
			}
		}
		catch (const AipsError& x) {
			ostringstream os;
			os << "Unable to reach target resolution of "
				<< myConvolved << " Input source "
				<< mySource << " is probably too large.";
			throw AipsError(os.str());
		}
		Record ret;
		QuantumHolder qh(neededBeam.getMajor());
		ret.defineRecord("major", qh.toRecord());
		qh = QuantumHolder(neededBeam.getMinor());
		ret.defineRecord("minor", qh.toRecord());
		qh = QuantumHolder(neededBeam.getPA());
		ret.defineRecord("pa", qh.toRecord());
		return fromRecord(ret);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::commonbeam() {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		ImageInfo myInfo = _image->getImage()->imageInfo();
		if (! myInfo.hasBeam()) {
			throw AipsError("This image has no beam(s).");
		}
		GaussianBeam beam;
		if (myInfo.hasSingleBeam()) {
			_log << LogIO::WARN
				<< "This image only has one beam, so just returning that"
				<< LogIO::POST;
			beam = myInfo.restoringBeam();

		}
		else {
			// multiple beams in this image
			beam = CasaImageBeamSet(myInfo.getBeamSet()).getCommonBeam();
		}
		beam.setPA(casa::Quantity(beam.getPA("deg", True), "deg"));
		Record x = beam.toRecord();
		x.defineRecord("pa", x.asRecord("positionangle"));
		x.removeField("positionangle");
		return fromRecord(x);

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}

}

bool image::remove(const bool finished, const bool verbose) {
	try {
		_log << _ORIGIN;

		if (detached()) {
			return False;
		}
		_stats.reset(0);

		if (_image->remove(verbose)) {
			// Now done the image tool if desired.
			if (finished) {
				done();
			}
			return True;
		}
		throw AipsError("Error removing image.");
	} catch (const AipsError &x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::removefile(const std::string& filename) {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "removefile");

		String fileName(filename);
		if (fileName.empty()) {
			_log << LogIO::WARN << "Empty filename" << LogIO::POST;
			return rstat;
		}
		File f(fileName);
		if (!f.exists()) {
			_log << LogIO::WARN << fileName << " does not exist."
					<< LogIO::POST;
			return rstat;
		}

		// Now try and blow it away.  If it's open, tabledelete won't delete it.
		String message;
		if (Table::canDeleteTable(message, fileName, True)) {
			Table::deleteTable(fileName, True);
			rstat = true;
		} else {
			_log << LogIO::WARN << "Cannot delete file " << fileName
					<< " because " << message << LogIO::POST;
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::done(const bool remove, const bool verbose) {
	try {
		_log << _ORIGIN;
		// resetting _stats must come before the table removal or the table
		// removal will fail
		_stats.reset(0);

		if (remove && !detached()) {
			if (!_image->remove(verbose)) {
				_log << LogIO::WARN << "Failed to remove image file"
						<< LogIO::POST;
			}
		}
		_image.reset();
        MeasIERS::closeTables();
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fft(
	const string& realOut, const string& imagOut,
	const string& ampOut, const string& phaseOut,
	const std::vector<int>& axes, const variant& region,
	const variant& vmask, const bool stretch,
	const string& complexOut
) {
	try {
		_log << LogOrigin(_class, __func__);
		if (detached()) {
			return false;
		}

		SHARED_PTR<Record> myregion(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<uInt> leAxes(0);
		if (
			axes.size() > 1
			|| (axes.size() == 1 && axes[0] >= 0)
		) {
			leAxes.resize(axes.size());
			for (uInt i=0; i<axes.size(); i++) {
				ThrowIf(
					axes[i] < 0,
					"None of the elements of axes may be less than zero"
				);
				leAxes[i] = axes[i];
			}
		}
		if (_image->isFloat()) {
			ImageFFTer<Float> ffter(
				_image->getImage(),
				myregion.get(), mask, leAxes
			);
			ffter.setStretch(stretch);
			ffter.setReal(realOut);
			ffter.setImag(imagOut);
			ffter.setAmp(ampOut);
			ffter.setPhase(phaseOut);
			ffter.setComplex(complexOut);
			ffter.fft();
		}
		else {
			ImageFFTer<Complex> ffter(
				_image->getComplexImage(),
				myregion.get(), mask, leAxes
			);
			ffter.setStretch(stretch);
			ffter.setReal(realOut);
			ffter.setImag(imagOut);
			ffter.setAmp(ampOut);
			ffter.setPhase(phaseOut);
			ffter.setComplex(complexOut);
			ffter.fft();
		}
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::findsources(const int nMax, const double cutoff,
		const variant& region, const ::casac::variant& vmask,
		const bool point, const int width, const bool absFind) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}

		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]")
			mask = "";

		Record listOut = _image->findsources(nMax, cutoff, *Region, mask,
				point, width, absFind);
		rstat = fromRecord(listOut);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

record* image::fitprofile(const string& box, const variant& region,
	const string& chans, const string& stokes, const int axis,
	const variant& vmask, int ngauss, const int poly,
	const string& estimates, const int minpts, const bool multifit,
	const string& model, const string& residual, const string& amp,
	const string& amperr, const string& center, const string& centererr,
	const string& fwhm, const string& fwhmerr,
	const string& integral, const string& integralerr, const bool stretch,
	const bool logResults, const variant& pampest,
	const variant& pcenterest, const variant& pfwhmest,
	const variant& pfix, const variant& gmncomps,
    const variant& gmampcon, const variant& gmcentercon,
    const variant& gmfwhmcon, const vector<double>& gmampest,
    const vector<double>& gmcenterest, const vector<double>& gmfwhmest,
    const variant& gmfix, const string& spxtype, const vector<double>& spxest,
    const vector<bool>& spxfix, const variant& div, const string& spxsol,
    const string& spxerr, const string& logfile,
    const bool append, const variant& pfunc,
    const vector<double>& goodamprange,
    const vector<double>& goodcenterrange,
    const vector<double>& goodfwhmrange, const variant& sigma,
    const string& outsigma, const vector<int>& planes
) {
	_log << LogOrigin(_class, __func__);
	if (detached()) {
		return 0;
	}
	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		String regionName;
		SHARED_PTR<Record> regionPtr = _getRegion(region, True);
		if (ngauss < 0) {
			_log << LogIO::WARN
				<< "ngauss < 0 is meaningless. Setting ngauss = 0 "
				<< LogIO::POST;
			ngauss = 0;
		}
		vector<double> mygoodamps = toVectorDouble(goodamprange, "goodamprange");
		if (mygoodamps.size() > 2) {
			_log << "Too many elements in goodamprange" << LogIO::EXCEPTION;
		}
		vector<double> mygoodcenters = toVectorDouble(goodcenterrange, "goodcenterrange");
		if (mygoodcenters.size() > 2) {
			_log << "Too many elements in goodcenterrange" << LogIO::EXCEPTION;
		}
		vector<double> mygoodfwhms = toVectorDouble(goodfwhmrange, "goodcenterrange");
		if (mygoodfwhms.size() > 2) {
			_log << "Too many elements in goodfwhmrange" << LogIO::EXCEPTION;
		}
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		std::unique_ptr<Array<Float> > sigmaArray;
		std::unique_ptr<PagedImage<Float> > sigmaImage;
		if (sigma.type() == variant::STRING) {
			String sigmaName = sigma.toString();
			if (! sigmaName.empty()) {
				sigmaImage.reset(new PagedImage<Float>(sigmaName));
			}
		}
		else if (
			sigma.type() == variant::DOUBLEVEC
			|| sigma.type() == variant::INTVEC
		) {
			sigmaArray.reset(new Array<Float>());
			vector<double> sigmaVector = sigma.getDoubleVec();
			Vector<Int> shape = sigma.arrayshape();
			sigmaArray->resize(IPosition(shape));
			convertArray(
				*sigmaArray,
				Vector<Double>(sigmaVector).reform(IPosition(shape))
			);
		}
		else if (sigma.type() == variant::BOOLVEC) {
			// nothing to do
		}
		else {
			_log << LogIO::SEVERE
				<< "Unrecognized type for sigma. Use either a string (image name) or a numpy array"
				<< LogIO::POST;
			return 0;
		}
		String myspxtype;
		vector<double> plpest, ltpest;
		vector<bool> plpfix, ltpfix;
		if (! spxtype.empty()) {
			myspxtype = String(spxtype);
			myspxtype.downcase();
			if (myspxtype == "plp") {
				plpest = spxest;
				plpfix = spxfix;
			}
			else if (myspxtype == "ltp") {
				ltpest = spxest;
				ltpfix = spxfix;
			}
			else {
				ThrowCc("Unsupported value for spxtype");
			}
		}
		SpectralList spectralList = SpectralListFactory::create(
			_log, pampest, pcenterest, pfwhmest, pfix, gmncomps,
			gmampcon, gmcentercon, gmfwhmcon, gmampest,
			gmcenterest, gmfwhmest, gmfix, pfunc, plpest, plpfix,
			ltpest, ltpfix
		);
		ThrowIf(
			! estimates.empty() && spectralList.nelements() > 0,
			"You cannot specify both an "
			"estimates file and set estimates "
			"directly. You may only do one or "
			"the either (or neither in which "
			"case you must specify ngauss and/or poly)"
		);
		SHARED_PTR<ImageProfileFitter> fitter;
		if (spectralList.nelements() > 0) {
			fitter.reset(new ImageProfileFitter(
				_image->getImage(), regionName, regionPtr.get(),
				box, chans, stokes, mask, axis,
				spectralList
			));
		}
		else if (! estimates.empty()) {
			fitter.reset(new ImageProfileFitter(
				_image->getImage(), regionName, regionPtr.get(),
				box, chans, stokes, mask, axis,
				estimates
			));
		}
		else {
			fitter.reset(new ImageProfileFitter(
				_image->getImage(), regionName, regionPtr.get(),
				box, chans, stokes, mask, axis,
				ngauss
			));
		}
		fitter->setDoMultiFit(multifit);
		if (poly >= 0) {
			fitter->setPolyOrder(poly);
		}
		fitter->setModel(model);
		fitter->setResidual(residual);
		fitter->setAmpName(amp);
		fitter->setAmpErrName(amperr);
		fitter->setCenterName(center);
		fitter->setCenterErrName(centererr);
		fitter->setFWHMName(fwhm);
		fitter->setFWHMErrName(fwhmerr);
		fitter->setIntegralName(integral);
		fitter->setIntegralErrName(integralerr);
		fitter->setMinGoodPoints(minpts > 0 ? minpts : 0);
		fitter->setStretch(stretch);
		fitter->setLogResults(logResults);
		if (! planes.empty()) {
			std::set<int> myplanes(planes.begin(), planes.end());
			ThrowIf(*myplanes.begin() < 0, "All planes must be nonnegative");
			fitter->setGoodPlanes(std::set<uInt>(myplanes.begin(), myplanes.end()));
		}
		if (! logfile.empty()) {
			fitter->setLogfile(logfile);
			fitter->setLogfileAppend(append);
		}
		if (mygoodamps.size() == 2) {
			fitter->setGoodAmpRange(mygoodamps[0], mygoodamps[1]);
		}
		if (mygoodcenters.size() == 2) {
			fitter->setGoodCenterRange(mygoodcenters[0], mygoodcenters[1]);
		}
		if (mygoodfwhms.size() == 2) {
			fitter->setGoodFWHMRange(mygoodfwhms[0], mygoodfwhms[1]);
		}
		if (sigmaImage.get()) {
			fitter->setSigma(sigmaImage.get());
		}
		else if (sigmaArray.get()) {
			fitter->setSigma(*sigmaArray);
		}
		if (! outsigma.empty()) {
			if (sigmaImage.get() || sigmaArray.get()) {
				fitter->setOutputSigmaImage(outsigma);
			}
			else {
				_log << LogIO::WARN
					<< "outsigma specified but no sigma image "
					<< "or array specified. outsigma will be ignored"
					<< LogIO::POST;
			}
		}
		if (plpest.size() > 0 || ltpest.size() > 0) {
			variant::TYPE t = div.type();
			if (div.type() == variant::BOOLVEC) {
				fitter->setAbscissaDivisor(0);
			}
			else if (t == variant::INT || t == variant::DOUBLE) {
				fitter->setAbscissaDivisor(div.toDouble());
			}
			else if (t == variant::STRING || t == variant::RECORD) {
				fitter->setAbscissaDivisor(casaQuantity(div));
			}
			else {
				throw AipsError("Unsupported type " + div.typeString() + " for div");
			}
			if (! myspxtype.empty()) {
				if (myspxtype == "plp") {
					fitter->setPLPName(spxsol);
					fitter->setPLPErrName(spxerr);
				}
				else if (myspxtype == "ltp") {
					fitter->setLTPName(spxsol);
					fitter->setLTPErrName(spxerr);
				}
			}
		}
		return fromRecord(fitter->fit());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::transpose(
	const std::string& outfile,
	const variant& order
) {
	try {
		_log << LogOrigin("image", __func__);

		if (detached()) {
			throw AipsError("No image specified to transpose");
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		std::unique_ptr<ImageTransposer> transposer;
		switch(order.type()) {
		case variant::INT:
			transposer.reset(
				new ImageTransposer(
					_image->getImage(),
					order.toInt(), outfile
				)
			);
			break;
		case variant::STRING:
			transposer.reset(
				new ImageTransposer(
					_image->getImage(),
					order.toString(), outfile
				)
			);
			break;
		case variant::STRINGVEC:
			{
				Vector<String> orderVec = toVectorString(order.toStringVec());
				transposer.reset(
					new ImageTransposer(
						_image->getImage(), orderVec,
						outfile
					)
				);
			}
			break;
		default:
			_log << "Unsupported type for order parameter " << order.type()
					<< ". Supported types are a non-negative integer, a single "
					<< "string containing all digits or a list of strings which "
					<< "unambiguously match the image axis names."
					<< LogIO::EXCEPTION;
		}
		
		image *rstat =new image(
			transposer->transpose()
		);
		if(!rstat)
			throw AipsError("Unable to transpose image");
		return rstat;
	} catch (const AipsError& x) {
		RETHROW(x);
	}
}

::casac::record* image::fitcomponents(
		const string& box, const variant& region, const variant& chans,
		const string& stokes, const variant& vmask,
		const vector<double>& in_includepix,
		const vector<double>& in_excludepix, const string& residual,
		const string& model, const string& estimates,
		const string& logfile, const bool append,
		const string& newestimates, const string& complist,
		bool overwrite, bool dooff, double offset,
		bool fixoffset, bool stretch, const variant& rms,
		const variant& noisefwhm
) {
	if (detached()) {
		return 0;
	}
	LogOrigin lor(_class, __func__);
	_log << lor;

	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		int num = in_includepix.size();
		Vector<Float> includepix(num);
		num = in_excludepix.size();
		Vector<Float> excludepix(num);
		convertArray(includepix, Vector<Double> (in_includepix));
		convertArray(excludepix, Vector<Double> (in_excludepix));
		if (includepix.size() == 1 && includepix[0] == -1) {
			includepix.resize();
		}
		if (excludepix.size() == 1 && excludepix[0] == -1) {
			excludepix.resize();
		}
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
        SPCIIF image = _image->getImage();
		ImageFitterResults::CompListWriteControl writeControl = complist.empty()
			? ImageFitterResults::NO_WRITE
			: overwrite
				? ImageFitterResults::OVERWRITE
				: ImageFitterResults::WRITE_NO_REPLACE;
		String sChans;
		if (chans.type() == variant::BOOLVEC) {
			// for some reason which eludes me, the default variant type is boolvec
			sChans = "";
		}
		else if (chans.type() == variant::STRING) {
			sChans = chans.toString();
		}
		else if (chans.type() == variant::INT) {
			sChans = String::toString(chans.toInt());
		}
		else {
			ThrowCc(
				"Unsupported type for chans. chans must "
				"be either an integer or a string"
			);
		}
		SHARED_PTR<Record> regionRecord = _getRegion(region, True);
		vector<String> msgs;
		Bool doImages = ! residual.empty() || ! model.empty();

		ImageFitter fitter(
			image, "", regionRecord.get(), box, sChans,
			stokes, mask, estimates, newestimates, complist
		);
		if (includepix.size() == 1) {
			fitter.setIncludePixelRange(
				std::make_pair(includepix[0],includepix[0])
			);
		}
		else if (includepix.size() == 2) {
			fitter.setIncludePixelRange(
				std::make_pair(includepix[0],includepix[1])
			);
		}
		if (excludepix.size() == 1) {
			fitter.setExcludePixelRange(
				std::make_pair(excludepix[0],excludepix[0])
			);
		}
		else if (excludepix.size() == 2) {
			fitter.setExcludePixelRange(
				std::make_pair(excludepix[0],excludepix[1])
			);
		}
		fitter.setWriteControl(writeControl);
		fitter.setStretch(stretch);
		fitter.setModel(model);
		fitter.setResidual(residual);
		if (! logfile.empty()) {
			fitter.setLogfile(logfile);
			fitter.setLogfileAppend(append);
		}
		if (dooff) {
			fitter.setZeroLevelEstimate(offset, fixoffset);
		}
		casa::Quantity myrms = (rms.type() == variant::DOUBLE || rms.type() == variant::INT)
			? casa::Quantity(rms.toDouble(), _image->brightnessunit())
			: _casaQuantityFromVar(rms);
		if (myrms.getValue() > 0) {
			fitter.setRMS(myrms);
		}
		variant::TYPE noiseType = noisefwhm.type();
		if (noiseType == variant::DOUBLE || noiseType == variant::INT) {
			fitter.setNoiseFWHM(noisefwhm.toDouble());
		}
		else if (noiseType == variant::BOOLVEC) {
			fitter.clearNoiseFWHM();
		}
		else if (
			noiseType == variant::STRING || noiseType == variant::RECORD
		) {
			if (noiseType == variant::STRING && noisefwhm.toString().empty()) {
				fitter.clearNoiseFWHM();
			}
			else {
				fitter.setNoiseFWHM(_casaQuantityFromVar(noisefwhm));
			}
		}
		else {
			ThrowCc(
				"Unsupported data type for noisefwhm: " + noisefwhm.typeString()
			);
		}
		if (doImages) {
			std::vector<String> names;
			names += "box", "region", "chans", "stokes", "mask", "includepix",
				"excludepix", "residual", "model", "estimates", "logfile",
				"append", "newestimates", "complist", "dooff", "offset",
				"fixoffset", "stretch", "rms", "noisefwhm";
			std::vector<variant> values;
			values += box, region, chans, stokes, vmask, in_includepix, in_excludepix,
				residual, model, estimates, logfile, append, newestimates, complist,
				dooff, offset, fixoffset, stretch, rms, noisefwhm;
			String fname = String("ia.") + String(__func__);
			fitter.addHistory(lor, fname, names, values);
		}
		std::pair<ComponentList, ComponentList> compLists = fitter.fit();
		return fromRecord(fitter.getOutputRecord());

	}
	catch (const AipsError& x) {
		FluxRep<Double>::clearAllowedUnits();
		_log << "Exception Reported: " << x.getMesg()
			<< LogIO::EXCEPTION;
		RETHROW(x);
	}
}

variant* image::getchunk(
	const std::vector<int>& blc, const std::vector<int>& trc,
	const std::vector<int>& inc, const std::vector<int>& axes,
	bool list, bool dropdeg, bool getmask
) {
	try {

		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		Record ret;
		if (_image->isFloat()) {
			ret = _getchunk<Float>(
				_image->getImage(), blc, trc, inc,
				axes, list, dropdeg
			);
			if (! getmask) {
				Array<Float> vals = ret.asArrayFloat("values");
				vector<double> v(vals.begin(), vals.end());
				return new variant(v, vals.shape().asStdVector());
			}
		}
		else {
			ret = _getchunk<Complex> (
				_image->getComplexImage(), blc, trc, inc,
				axes, list, dropdeg
			);
			if (! getmask) {
				Array<Complex> vals = ret.asArrayComplex("values");
				vector<std::complex<double> > v(vals.begin(), vals.end());
				return new variant(v, vals.shape().asStdVector());
			}
		}
		if (getmask) {
			Array<Bool> pixelMask = ret.asArrayBool("mask");
			std::vector<bool> s_pixelmask(pixelMask.begin(), pixelMask.end());
			return new variant(s_pixelmask, pixelMask.shape().asStdVector());
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	// eliminate compiler warning, execution should never get here
	return 0;
}

template<class T> Record image::_getchunk(
	SPCIIT myimage,
	const vector<int>& blc, const vector<int>& trc,
	const vector<int>& inc, const vector<int>& axes,
	bool list, bool dropdeg
) {
	Array<T> pixels;
	Array<Bool> pixelMask;
	Vector<Int> iaxes(axes);
	// if default value change it to empty vector
	if (iaxes.size() == 1 && iaxes[0] < 0) {
		iaxes.resize();
	}
	uInt ndim = myimage->ndim();

	if (iaxes.size() == 1 && iaxes[0] < 0) {
		iaxes.resize();
	}
	// We have to support handling of sloppy inputs for backwards
	// compatibility. Ugh.
	vector<int> mblc(ndim);
	vector<int> mtrc(ndim);
	vector<int> minc(ndim);
	if (blc.size() == 1 && blc[0] < 0) {
		IPosition x(ndim, 0);
		mblc = x.asStdVector();
	}
	else {
		for (uInt i=0; i<ndim; i++) {
			mblc[i] = i < blc.size() ? blc[i] : 0;
		}
	}
	IPosition shape = myimage->shape();
	if (trc.size() == 1 && trc[0] < 0) {
		mtrc = (shape - 1).asStdVector();
	}
	else {
		for (uInt i=0; i<ndim; i++) {
			mtrc[i] = i < trc.size() ? trc[i] : shape[i] - 1;
		}
	}
	if (inc.size() == 1 && inc[0] == 1) {
		IPosition x(ndim, 1);
		minc = x.asStdVector();
	}
	else {
		for (uInt i=0; i<ndim; i++) {
			minc[i] = i < inc.size() ? inc[i] : 1;
		}
	}
	for (uInt i=0; i<ndim; i++) {
		if (mblc[i] < 0 || mblc[i] > shape[i] - 1) {
			mblc[i] = 0;
		}
		if (mtrc[i] < 0 || mtrc[i] > shape[i] - 1) {
			mtrc[i] = shape[i] - 1;
		}
		if (mblc[i] > mtrc[i]) {
			mblc[i] = 0;
			mtrc[i] = shape[i] - 1;
		}
		if (inc[i] > shape[i]) {
			minc[i] = 1;
		}
	}
	Vector<Double> vblc(mblc);
	Vector<Double> vtrc(mtrc);
	Vector<Double> vinc(minc);
	LCSlicer slicer(vblc, vtrc, vinc);
	Record rec;
	rec.assign(slicer.toRecord(""));
	PixelValueManipulator<T> pvm(myimage, &rec, "");
	if (axes.size() != 1 || axes[0] >= 0) {
		pvm.setAxes(IPosition(axes));
	}
	pvm.setVerbosity(
		list ? ImageTask<T>::DEAFENING : ImageTask<T>::QUIET
	);
	pvm.setDropDegen(dropdeg);
	return pvm.get();
}

image* image::pbcor(
	const variant& pbimage, const string& outfile,
	const bool overwrite, const string& box,
	const variant& region, const string& chans,
	const string& stokes, const string& mask,
	const string& mode, const float cutoff,
	const bool stretch
) {
	if (detached()) {
		throw AipsError("Unable to create image");
		return 0;
	}
	try {
		_log << _ORIGIN;
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		Array<Float> pbPixels;
        SPCIIF pb_ptr;
		if (pbimage.type() == variant::DOUBLEVEC) {
			Vector<Int> shape = pbimage.arrayshape();
			pbPixels.resize(IPosition(shape));
			Vector<Double> localpix(pbimage.getDoubleVec());
			casa::convertArray(pbPixels, localpix.reform(IPosition(shape)));
		}
		else if (pbimage.type() == variant::STRING) {
            ImageInterface<Float>* pb;
            ImageUtilities::openImage(pb, pbimage.getString());
			if (pb == 0) {
				_log << "Unable to open primary beam image " << pbimage.getString()
					<< LogIO::EXCEPTION;
			}
            pb_ptr.reset(pb);
		}
		else {
			_log << "Unsupported type " << pbimage.typeString()
				<< " for pbimage" << LogIO::EXCEPTION;
		}

		String regionString = "";
		std::unique_ptr<Record> regionRecord;
		if (region.type() == variant::STRING || region.size() == 0) {
			regionString = (region.size() == 0) ? "" : region.toString();
		}
		else if (region.type() == variant::RECORD) {
			regionRecord.reset(toRecord(region.clone()->asRecord()));
		}
		else {
			_log << "Unsupported type for region " << region.type()
				<< LogIO::EXCEPTION;
		}
		String modecopy = mode;
		modecopy.downcase();
		modecopy.trim();
		if (! modecopy.startsWith("d") && ! modecopy.startsWith("m")) {
			throw AipsError("Unknown mode " + mode);
		}
		ImagePrimaryBeamCorrector::Mode myMode = modecopy.startsWith("d")
			? ImagePrimaryBeamCorrector::DIVIDE
			: ImagePrimaryBeamCorrector::MULTIPLY;
		Bool useCutoff = cutoff >= 0.0;
        SPCIIF shImage = _image->getImage();
        std::unique_ptr<ImagePrimaryBeamCorrector> pbcor(
			(!pb_ptr)
			? new ImagePrimaryBeamCorrector(
				shImage, pbPixels, regionRecord.get(),
				regionString, box, chans, stokes, mask, outfile, overwrite,
				cutoff, useCutoff, myMode
			)
			: new ImagePrimaryBeamCorrector(
				shImage, pb_ptr, regionRecord.get(),
				regionString, box, chans, stokes, mask, outfile, overwrite,
				cutoff, useCutoff, myMode
			)
		);
		pbcor->setStretch(stretch);
        SHARED_PTR<ImageInterface<Float> > corrected(pbcor->correct(True));
		return new image(corrected);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::getprofile(
	int axis, const string& function, const variant& region,
	const string& mask, const string& unit,
	bool stretch, const string& spectype,
	const variant& restfreq, const string& frame
) {
	try {
		_log << _ORIGIN;
		ThrowIf(
			detached(), "Unable to create image"
		);
		SHARED_PTR<Record> myregion(_getRegion(region, False));
		SHARED_PTR<casa::Quantity> rfreq;
		if (restfreq.type() != variant::BOOLVEC) {
			String rf = restfreq.toString();
			rf.trim();
			if (! rf.empty()) {
				rfreq.reset(
					new casa::Quantity(_casaQuantityFromVar(variant(restfreq)))
				);
			}
		}
		String myframe = frame;
		myframe.trim();
		if (_image->isFloat()) {
			SPCIIF myimage = _image->getImage();
			return fromRecord(
				_getprofile(
					myimage, axis, function, unit,
					*myregion, mask, stretch,
					spectype, rfreq.get(), myframe
				)
			);
		}
		else {
			SPCIIC myimage = _image->getComplexImage();
			return fromRecord(
				_getprofile(
					myimage, axis, function, unit,
					*myregion, mask, stretch,
					spectype, rfreq.get(), myframe
				)
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
	return 0;
}
template <class T> Record image::_getprofile(
	SPCIIT myimage, int axis, const String& function,
	const String& unit, const Record& region, const String& mask,
	bool stretch, const String& spectype,
	const casa::Quantity* const &restfreq, const String& frame
) {
	PixelValueManipulatorData::SpectralType type = PixelValueManipulatorData::spectralType(spectype);
	PixelValueManipulator<T> pvm(myimage, &region, mask);
	pvm.setStretch(stretch);
	Record x = pvm.getProfile(axis, function, unit, type, restfreq, frame);
	return x;
}

variant* image::getregion(
	const variant& region, const std::vector<int>& axes,
	const ::casac::variant& mask, bool list, bool dropdeg,
	bool getmask, bool stretch
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String Mask;
		if (mask.type() == ::casac::variant::BOOLVEC) {
			Mask = "";
		}
		else if (
			mask.type() == ::casac::variant::STRING
			|| mask.type() == ::casac::variant::STRINGVEC
		) {
			Mask = mask.toString();
		}
		else {
			_log << LogIO::WARN
				<< "Only LEL string handled for mask...region is yet to come"
				<< LogIO::POST;
			Mask = "";
		}
		Vector<Int> iaxes(axes);
		// if default value change it to empty vector
		if (iaxes.size() == 1 && iaxes[0] < 0) {
			iaxes.resize();
		}
		Record ret;
		if (_image->isFloat()) {
			PixelValueManipulator<Float> pvm(
				_image->getImage(), Region.get(), Mask
			);
			pvm.setAxes(IPosition(iaxes));
			pvm.setVerbosity(
				list ? ImageTask<Float>::DEAFENING : ImageTask<Float>::QUIET
			);
			pvm.setDropDegen(dropdeg);
			pvm.setStretch(stretch);
			ret = pvm.get();
		}
		else {
			PixelValueManipulator<Complex> pvm(
				_image->getComplexImage(), Region.get(), Mask
			);
			pvm.setAxes(IPosition(iaxes));
			pvm.setVerbosity(
				list ? ImageTask<Complex>::DEAFENING : ImageTask<Complex>::QUIET
			);
			pvm.setDropDegen(dropdeg);
			pvm.setStretch(stretch);
			ret = pvm.get();
		}
		Array<Bool> pixelmask = ret.asArrayBool("mask");
		std::vector<int> s_shape = pixelmask.shape().asStdVector();
		if (getmask) {
			pixelmask.shape().asVector().tovector(s_shape);
			std::vector<bool> s_pixelmask(pixelmask.begin(), pixelmask.end());
			return new ::casac::variant(s_pixelmask, s_shape);
		}
		else if (_image->isFloat()) {
			Array<Float> pixels = ret.asArrayFloat("values");
			std::vector<double> d_pixels(pixels.begin(), pixels.end());
			return new ::casac::variant(d_pixels, s_shape);
		}
		else {
			Array<Complex> pixels = ret.asArrayComplex("values");
			std::vector<std::complex<double> > d_pixels(pixels.begin(), pixels.end());
			return new ::casac::variant(d_pixels, s_shape);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::getslice(const std::vector<double>& x, const std::vector<double>& y,
		const std::vector<int>& axes, const std::vector<int>& coord,
		const int npts, const std::string& method) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		// handle default coord
		std::vector<int> ncoord(coord);
		if (ncoord.size() == 1 && ncoord[0] == -1) {
			int n = _image->getImage()->ndim();
			ncoord.resize(n);
			for (int i = 0; i < n; i++) {
				//ncoord[i]=i;
				ncoord[i] = 0;
			}
		}

		Record *outRec = _image->getslice(Vector<Double> (x),
				Vector<Double> (y), Vector<Int> (axes), Vector<Int> (ncoord),
				npts, method);
		rstat = fromRecord(*outRec);
		delete outRec;
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

image* image::hanning(
	const string& outfile, const variant& region,
	const variant& vmask, int axis, bool drop,
	bool overwrite, bool /* async */, bool stretch,
    const string& dmethod
) {
	LogOrigin lor(_class, __func__);
	_log << lor;
	if (detached()) {
		throw AipsError("Unable to create image");
	}
	try {
		SHARED_PTR<const Record> myregion = _getRegion(
			region, True
		);
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (axis < 0) {
			const CoordinateSystem csys = _image->isFloat()
				? _image->getImage()->coordinates()
				: _image->getComplexImage()->coordinates();
			ThrowIf(
				! csys.hasSpectralAxis(),
				"Axis not specified and image has no spectral coordinate"
			);
			axis = csys.spectralAxisNumber(False);
		}
        ImageDecimatorData::Function dFunction = ImageDecimatorData::NFUNCS;
        if (drop) {
            String mymethod = dmethod;
            mymethod.downcase();
            if (mymethod.startsWith("m")) {
                dFunction = ImageDecimatorData::MEAN;
            }
            else if (mymethod.startsWith("c")) {
                dFunction = ImageDecimatorData::COPY;
            }
            else {
                ThrowCc(
                    "Value of dmethod must be "
                    "either 'm'(ean) or 'c'(opy)"
                );
            }
        }
		vector<variant> values;
		values += outfile, region, vmask, axis, drop, overwrite, stretch, dmethod;
		if (_image->isFloat()) {
			SPCIIF image = _image->getImage();
			return _hanning(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, drop,
				dFunction, values
			);
		}
		else {
			SPCIIC image = _image->getComplexImage();
			return _hanning(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, drop,
				dFunction, values
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

template <class T> image* image::_hanning(
	SPCIIT myimage, SHARED_PTR<const Record> region,
	const String& mask, const string& outfile, bool overwrite,
	bool stretch, int axis, bool drop,
	ImageDecimatorData::Function dFunction,
	const std::vector<casac::variant> values
) {
	ImageHanningSmoother<T> smoother(
		myimage, region.get(), mask, outfile, overwrite
	);
	smoother.setAxis(axis);
	smoother.setDecimate(drop);
	smoother.setStretch(stretch);
	if (drop) {
		smoother.setDecimationFunction(dFunction);
	}
	vector<String> names;
	names += "outfile", "region", "mask", "axis",
		"drop", "overwrite", "stretch", "dmethod";
	smoother.addHistory(
		LogOrigin(_class, __func__), "ia.hanning",
		names, values
	);
	return new image(smoother.smooth());
}

std::vector<bool> image::haslock() {
	std::vector<bool> rstat;
	try {
		_log << LogOrigin("image", __func__);
		if (detached())
			return rstat;

		_image->haslock().tovector(rstat);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

record* image::histograms(
	const vector<int>& axes,
	const variant& region, const variant& mask,
	const int nbins, const vector<double>& includepix,
	const bool gauss, const bool cumu, const bool log, const bool list,
	const bool force, const bool disk,
	const bool /* async */, bool stretch
) {
	_log << LogOrigin(_class, __func__);
	if (detached()) {
		return 0;
	}
	try {
		SHARED_PTR<Record> regionRec(_getRegion(region, False));
		String Mask;
		if (mask.type() == variant::BOOLVEC) {
			Mask = "";
		}
		else if (
			mask.type() == variant::STRING
			|| mask.type() == variant::STRINGVEC
		) {
			Mask = mask.toString();
		}
		else {
			_log << LogIO::WARN
					<< "Only LEL string handled for mask...region is yet to come"
					<< LogIO::POST;
			Mask = "";
		}
		Vector<Int> naxes;
		if (!(axes.size() == 1 && axes[0] == -1)) {
			naxes.resize(axes.size());
			naxes = Vector<Int> (axes);
		}
		Vector<Double> includePix;
		if (!(includepix.size() == 1 && includepix[0] == -1)) {
			includePix.resize(includepix.size());
			includePix = Vector<Double> (includepix);
		}
        return fromRecord(
        	_image->histograms(
        		naxes, *regionRec, Mask, nbins, includePix,
        		gauss, cumu, log, list, force, disk, stretch
        	)
        );
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::vector<std::string> image::history(bool list) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return vector<string>();
		}
		if (_image->isFloat()) {
			SPIIF im = _image->getImage();
			ImageHistory<Float> hist(im);
			return fromVectorString(hist.get(list));
		}
		else {
			SPIIC im = _image->getComplexImage();
			ImageHistory<Complex> hist(im);
			return fromVectorString(hist.get(list));
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::insert(
	const std::string& infile, const variant& region,
	const std::vector<double>& locate, bool verbose
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}

		Vector<Double> locatePixel(locate);
		if (locatePixel.size() == 1 && locatePixel[0] < 0) {
			locatePixel.resize(0);
		}
		SHARED_PTR<Record> Region(_getRegion(region, False));
		if (_image->insert(infile, *Region, locatePixel, verbose)) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error inserting image.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::isopen() {
	try {
		_log << _ORIGIN;

		if (_image.get() != 0 && !_image->detached()) {
			return True;
		} else {
			return False;
		}
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::ispersistent() {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "ispersistent");
		if (detached())
			return rstat;

		rstat = _image->ispersistent();
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::lock(const bool writelock, const int nattempts) {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "lock");
		if (detached())
			return rstat;
		rstat = _image->lock(writelock, nattempts);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::makecomplex(const std::string& outFile,
		const std::string& imagFile, const variant& region,
		const bool overwrite) {
	bool rstat(false);
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}
		SHARED_PTR<Record> Region(_getRegion(region, False));
		rstat = _image->makecomplex(outFile, imagFile, *Region, overwrite);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

std::vector<std::string> image::maskhandler(const std::string& op,
		const std::vector<std::string>& name) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return std::vector<string>(0);
		}

		Vector<String> namesOut;
		Vector<String> namesIn = toVectorString(name);
		namesOut = _image->maskhandler(op, namesIn);
		if (namesOut.size() == 0) {
			namesOut.resize(1);
			namesOut[0] = "T";
		}
		std::vector<string> rstat = fromVectorString(namesOut);
		_stats.reset(0);
		return rstat;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::miscinfo() {
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", "miscinfo");
		if (detached())
			return rstat;

		rstat = fromRecord(_image->miscinfo());
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::modify(
	const ::casac::record& model, const variant& region,
	const ::casac::variant& vmask, const bool subtract, const bool list,
	const bool /* async */, bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return false;
	}
	try {
		String error;
		std::unique_ptr<Record> Model(toRecord(model));
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (
			_image->modify(
				*Model, *Region, mask, subtract,
				list, stretch
			)
		) {
			_stats.reset(0);
			return True;
		}
		ThrowCc("Error modifying image.");
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::maxfit(const variant& region, const bool doPoint,
		const int width, const bool absFind, const bool list) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}
		SHARED_PTR<Record> Region(_getRegion(region, False));
		rstat = fromRecord(_image->maxfit(*Region, doPoint, width, absFind,
				list));

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

::casac::image *
image::moments(
	const std::vector<int>& moments, const int axis,
	const variant& region, const ::casac::variant& vmask,
	const std::vector<std::string>& in_method,
	const std::vector<int>& smoothaxes,
	const ::casac::variant& smoothtypes,
	const std::vector<double>& smoothwidths,
	const std::vector<double>& d_includepix,
	const std::vector<double>& d_excludepix, const double peaksnr,
	const double stddev, const std::string& velocityType,
	const std::string& out, const std::string& smoothout,
	const bool overwrite, const bool removeAxis,
	const bool stretch, const bool /* async */
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
		Vector<Int> whichmoments(moments);
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<String> method = toVectorString(in_method);

		Vector<String> kernels;
		if (smoothtypes.type() == ::casac::variant::BOOLVEC) {
			kernels.resize(0); // unset
		}
		else if (smoothtypes.type() == ::casac::variant::STRING) {
			sepCommaEmptyToVectorStrings(kernels, smoothtypes.toString());
		}
		else if (smoothtypes.type() == ::casac::variant::STRINGVEC) {
			kernels = toVectorString(smoothtypes.toStringVec());
		}
		else {
			_log << LogIO::WARN << "Unrecognized smoothtypes datatype"
				<< LogIO::POST;
		}
		int num = kernels.size();

		Vector<Quantum<Double> > kernelwidths(num);
		Unit u("pix");
		for (int i = 0; i < num; i++) {
			kernelwidths[i] = casa::Quantity(smoothwidths[i], u);
		}
		Vector<Float> includepix;
		num = d_includepix.size();
		if (!(num == 1 && d_includepix[0] == -1)) {
			includepix.resize(num);
			for (int i = 0; i < num; i++)
				includepix[i] = d_includepix[i];
		}
		Vector<Float> excludepix;
		num = d_excludepix.size();
		if (!(num == 1 && d_excludepix[0] == -1)) {
			excludepix.resize(num);
			for (int i = 0; i < num; i++)
				excludepix[i] = d_excludepix[i];
		}
		SHARED_PTR<ImageInterface<Float> > outIm(
			_image->moments(
				whichmoments, axis,
				*Region, mask, method, Vector<Int> (smoothaxes), kernels,
				kernelwidths, includepix, excludepix, peaksnr, stddev,
				velocityType, out, smoothout,
				overwrite, removeAxis, stretch
			)
		);
		return new ::casac::image(outIm);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::string image::name(const bool strippath) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return "none";
		}
		return _name(strippath);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

String image::_name(bool strippath) const {
	if (_image->isFloat()) {
		return _image->getImage()->name(strippath);
	}
	else {
		return _image->getComplexImage()->name(strippath);
	}
}

bool image::open(const std::string& infile) {
	try {
		_reset();

		_log << _ORIGIN;
        return _image->open(infile);

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::pad(
	const string& outfile, int npixels, double value, bool padmask,
	bool overwrite, const variant& region, const string& box,
	const string& chans, const string& stokes, const string& mask,
	bool  stretch, bool wantreturn
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		if (npixels <= 0) {
			_log << "Value of npixels must be greater than zero" << LogIO::EXCEPTION;
		}
		SHARED_PTR<Record> regionPtr = _getRegion(region, True);

		ImagePadder padder(
			_image->getImage(), regionPtr.get(), box,
			chans, stokes, mask, outfile, overwrite
		);
		padder.setStretch(stretch);
		padder.setPaddingPixels(npixels, value, padmask);
		SHARED_PTR<ImageInterface<Float> > out(padder.pad(wantreturn));
		if (wantreturn) {
			return new image(out);
		}
		return 0;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::crop(
	const string& outfile, const vector<int>& axes,
	bool overwrite, const variant& region, const string& box,
	const string& chans, const string& stokes, const string& mask,
	bool  stretch, bool wantreturn
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
        if (axes.size() > 0) {
            std::set<int> saxes(axes.begin(), axes.end());
	    	if (*saxes.begin() < 0) {
			    _log << "All axes values must be >= 0" << LogIO::EXCEPTION;
		    }
        }
        std::set<uInt> saxes(axes.begin(), axes.end());
		SHARED_PTR<Record> regionPtr = _getRegion(region, True);

		ImageCropper<Float> cropper(
			_image->getImage(), regionPtr.get(), box,
			chans, stokes, mask, outfile, overwrite
		);
		cropper.setStretch(stretch);
        cropper.setAxes(saxes);
        SHARED_PTR<ImageInterface<Float> > out(cropper.crop(wantreturn));
		if (wantreturn) {
			return new image(out);
		}
		return 0;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::pixelvalue(const std::vector<int>& pixel) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached())
			return rstat;

		Record *outRec = _image->pixelvalue(Vector<Int> (pixel));
		rstat = fromRecord(*outRec);
		delete outRec;
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::putchunk(
	const variant& pixels,
	const vector<int>& blc, const vector<int>& inc,
	const bool list, const bool locking, const bool replicate
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return false;
		}
		if (_image->isFloat()) {
			_putchunk(
				_image->getImage(), pixels, blc, inc, list, locking, replicate
			);
		}
		else {
			if (
				pixels.type() == variant::COMPLEXVEC
			) {
				Array<Complex> pixelsArray;
				std::vector<std::complex<double> > pixelVector = pixels.getComplexVec();
				Vector<Int> shape = pixels.arrayshape();
				pixelsArray.resize(IPosition(shape));
				Vector<std::complex<double> > localpix(pixelVector);
				casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
				PixelValueManipulator<Complex>::put(
					_image->getComplexImage(), pixelsArray, Vector<Int>(blc),
					Vector<Int>(inc), list, locking,
					replicate
				);
			}
			else {
				_putchunk(
					_image->getComplexImage(), pixels,
					blc, inc, list, locking, replicate
				);
			}
		}
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

template<class T> void image::_putchunk(
	SPIIT myimage, const variant& pixels,
	const vector<int>& blc, const vector<int>& inc,
	const bool list, const bool locking, const bool replicate
) {
	Array<T> pixelsArray;
	Vector<Int> shape = pixels.arrayshape();
	pixelsArray.resize(IPosition(shape));
	if (pixels.type() == variant::DOUBLEVEC) {
		std::vector<double> pixelVector = pixels.getDoubleVec();
		Vector<Double> localpix(pixelVector);
		casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
	}
	else if (pixels.type() == variant::INTVEC) {
		std::vector<int> pixelVector = pixels.getIntVec();
		Vector<Int> localpix(pixelVector);
		casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
	}
	else {
		String types = myimage->dataType() == TpFloat
			? "doubles or ints"
			: "complexes, doubles, or ints";
		ThrowCc(
			"Unsupported type for pixels parameter. It "
			"must be either a vector of " + types
		);
	}
	PixelValueManipulator<T>::put(
		myimage, pixelsArray, Vector<Int>(blc),
		Vector<Int>(inc), list, locking,
		replicate
	);
}

bool image::putregion(const ::casac::variant& v_pixels,
		const ::casac::variant& v_pixelmask, const variant& region,
		const bool list, const bool usemask, const bool locking,
		const bool replicateArray) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}
		// create Array<Float> pixels
		Array<Float> pixels;
		if (isunset(v_pixels)) {
			// do nothing
		} else if (v_pixels.type() == ::casac::variant::DOUBLEVEC) {
			std::vector<double> pixelVector = v_pixels.getDoubleVec();
			Vector<Int> shape = v_pixels.arrayshape();
			pixels.resize(IPosition(shape));
			Vector<Double> localpix(pixelVector);
			casa::convertArray(pixels, localpix.reform(IPosition(shape)));
		} else if (v_pixels.type() == ::casac::variant::INTVEC) {
			std::vector<int> pixelVector = v_pixels.getIntVec();
			Vector<Int> shape = v_pixels.arrayshape();
			pixels.resize(IPosition(shape));
			Vector<Int> localpix(pixelVector);
			casa::convertArray(pixels, localpix.reform(IPosition(shape)));
		} else {
			_log << LogIO::SEVERE
					<< "pixels is not understood, try using an array "
					<< LogIO::POST;
			return False;
		}

		// create Array<Bool> mask
		Array<Bool> mask;
		if (isunset(v_pixelmask)) {
			// do nothing
		} else if (v_pixelmask.type() == ::casac::variant::DOUBLEVEC) {
			std::vector<double> maskVector = v_pixelmask.getDoubleVec();
			Vector<Int> shape = v_pixelmask.arrayshape();
			mask.resize(IPosition(shape));
			Vector<Double> localmask(maskVector);
			casa::convertArray(mask, localmask.reform(IPosition(shape)));
		} else if (v_pixelmask.type() == ::casac::variant::INTVEC) {
			std::vector<int> maskVector = v_pixelmask.getIntVec();
			Vector<Int> shape = v_pixelmask.arrayshape();
			mask.resize(IPosition(shape));
			Vector<Int> localmask(maskVector);
			casa::convertArray(mask, localmask.reform(IPosition(shape)));
		} else if (v_pixelmask.type() == ::casac::variant::BOOLVEC) {
			std::vector<bool> maskVector = v_pixelmask.getBoolVec();
			Vector<Int> shape = v_pixelmask.arrayshape();
			mask.resize(IPosition(shape));
			Vector<Bool> localmask(maskVector);
			// casa::convertArray(mask,localmask.reform(IPosition(shape)));
			mask = localmask.reform(IPosition(shape));
		} else {
			_log << LogIO::SEVERE
					<< "mask is not understood, try using an array "
					<< LogIO::POST;
			return False;
		}

		if (pixels.size() == 0 && mask.size() == 0) {
			_log << "You must specify at least either the pixels or the mask"
					<< LogIO::POST;
			return False;
		}

		SHARED_PTR<Record> theRegion(_getRegion(region, False));
		if (
			_image->putregion(
				pixels, mask, *theRegion, list, usemask,
				locking, replicateArray
			)
		) {
			_stats.reset(0);
			return True;
		}
		ThrowCc("Error putting region.");
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::pv(
	const string& outfile, const variant& start,
	const variant& end, const variant& center, const variant& length,
	const variant& pa, const variant& width, const string& unit,
	const bool overwrite, const variant& region, const string& chans,
	const string& stokes, const string& mask, const bool stretch,
	const bool wantreturn
) {
	if (detached()) {
		return 0;
	}
	try {
		_log << _ORIGIN;
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		SHARED_PTR<casa::MDirection> startMD, endMD, centerMD;
		Vector<Double> startPix, endPix, centerPix;
		SHARED_PTR<casa::Quantity> lengthQ;
		Double lengthD = 0;
		if (! start.empty() && ! end.empty()) {
			ThrowIf(
				! center.empty() || ! length.empty()
				|| ! pa.empty(),
				"None of center, length, nor pa may be specified if start and end are specified"
			);
			ThrowIf(
				start.type() != end.type(),
				"start and end must be the same data type"
			);
			casa::MDirection dir;
			_processDirection(startPix, dir, start, String("start"));
			if (startPix.size() == 0) {
				startMD.reset(new casa::MDirection(dir));
			}
			_processDirection(endPix, dir, end, "end");
			if (endPix.size() == 0) {
				endMD.reset(new casa::MDirection(dir));
			}
		}
		else if (
			! center.empty() && ! length.empty()
			&& ! pa.empty()
		) {
			ThrowIf(
				! start.empty() || ! end.empty(),
				"Neither start nor end may be specified "
				"if center, length, and pa are specified"
			);
			casa::MDirection dir;
			_processDirection(centerPix, dir, center, "center");
			if (centerPix.size() == 0) {
				centerMD.reset(new casa::MDirection(dir));
			}
			if (length.type() == variant::INT || length.type() == variant::DOUBLE) {
				lengthD = length.toDouble();
			}
			else {
				lengthQ.reset(
					new casa::Quantity(_casaQuantityFromVar(variant(length)))
				);
			}
		}
		else {
			ThrowCc(
				"Either both of start and end or all three of "
				"center, width, and pa must be specified"
			);
		}
		_log << _ORIGIN;

		uInt intWidth = 0;
		casa::Quantity qWidth;
		if (width.type() == variant::INT) {
			intWidth = width.toInt();
			ThrowIf(
				intWidth % 2 == 0 || intWidth < 1,
				"width must be an odd integer >= 1"
			);
		}
		else if (
			width.type() == variant::STRING
			|| width.type() == variant::RECORD
		) {
			qWidth = _casaQuantityFromVar(width);
		}
		else if (width.type() == variant::BOOLVEC && width.empty()) {
			intWidth = 1;
		}
		else {
			ThrowCc("Unsupported data type for width " + width.typeString());
		}
		if (outfile.empty() && ! wantreturn) {
			_log << LogIO::WARN << "outfile was not specified and wantreturn is false. "
				<< "The resulting image will be inaccessible" << LogIO::POST;
		}
		SHARED_PTR<Record> regionPtr = _getRegion(region, True);
		PVGenerator pv(
			_image->getImage(), regionPtr.get(),
			chans, stokes, mask, outfile, overwrite
		);
		if (startPix.size() == 2) {
			pv.setEndpoints(
				make_pair(startPix[0], startPix[1]),
				make_pair(endPix[0], endPix[1])
			);
		}
		else if (startMD) {
			pv.setEndpoints(*startMD, *endMD);
		}
		else if (centerMD) {
			if (lengthQ) {
				pv.setEndpoints(
					*centerMD, *lengthQ, _casaQuantityFromVar(variant(pa))
				);
			}
			else {
				pv.setEndpoints(
					*centerMD, lengthD, _casaQuantityFromVar(variant(pa))
				);
			}
		}
		else {
			if (lengthQ) {
				pv.setEndpoints(
					make_pair(centerPix[0], centerPix[1]),
					*lengthQ, _casaQuantityFromVar(variant(pa))
				);
			}
			else {
				pv.setEndpoints(
						make_pair(centerPix[0], centerPix[1]),
					lengthD, _casaQuantityFromVar(variant(pa))
				);
			}
		}
		if (intWidth == 0) {
			pv.setWidth(qWidth);
		}
		else {
			pv.setWidth(intWidth);
		}
		pv.setStretch(stretch);
		pv.setOffsetUnit(unit);
		_log << _ORIGIN;
		SPIIF out = pv.generate();
		image *ret = wantreturn ? new image(out) : 0;
		return ret;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

void image::_processDirection(
	Vector<Double>& pixel, casa::MDirection& dir,
	const variant& inputDirection, const String& paramName
) {
	variant::TYPE myType = inputDirection.type();
	ThrowIf(
		(
			myType == variant::INTVEC
			|| myType == variant::DOUBLEVEC
			|| myType == variant::STRINGVEC
		) &&
		inputDirection.size() != 2,
		"If specified as an array, " + paramName
		+ " must have exactly two elements"
	);
	pixel.resize(0);
	if (myType == variant::INTVEC || myType == variant::DOUBLEVEC) {
		pixel = Vector<Double>(_toDoubleVec(inputDirection));
	}
	else if(myType == variant::STRINGVEC) {
		vector<string> x = inputDirection.toStringVec();
		casa::Quantity q0 = _casaQuantityFromVar(variant(x[0]));
		casa::Quantity q1 = _casaQuantityFromVar(variant(x[1]));
		dir = casa::MDirection(q0, q1);
	}
	else if (myType == variant::STRING) {
		string parts[3];
		split(inputDirection.toString(), parts, 3, Regex("[, \n\t\r\v\f]+"));
		casa::MDirection::Types frame;
		casa::MDirection::getType(frame, parts[0]);
		dir = casa::MDirection::getType(frame, parts[0])
			? casa::MDirection(
				_casaQuantityFromVar(parts[1]),
				_casaQuantityFromVar(parts[2]), frame
			)
			: casa::MDirection(
				_casaQuantityFromVar(parts[0]),
				_casaQuantityFromVar(parts[1])
			);
	}
	else {
		ThrowCc("Unsupported type for " + paramName);
	}
}

image* image::rebin(
	const std::string& outfile, const std::vector<int>& bin,
	const variant& region, const ::casac::variant& vmask,
	bool dropdeg, bool overwrite, bool /* async */,
	bool stretch, bool crop
) {
	LogOrigin lor(_class, __func__);
	_log << lor;
	ThrowIf(
		detached(), "Unable to create image"
	);
	Vector<Int> mybin(bin);
	ThrowIf(
		anyTrue(mybin <= 0),
		"All binning factors must be positive."
	);
	try {
		std::vector<String> names;
		names += "outfile", "bin", "region", "mask",
			"dropdeg", "overwrite", "stretch", "crop";
		std::vector<variant> values;
		values += outfile, bin, region, vmask,
			dropdeg, overwrite, stretch, crop;

		vector<String> msgs = _newHistory(__func__, names, values);
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (_image->isFloat()) {
			SPIIF myfloat = _image->getImage();
			ImageRebinner<Float> rebinner(
				myfloat, _getRegion(region, True).get(),
				mask, outfile, overwrite
			);
			rebinner.setFactors(mybin);
			rebinner.setStretch(stretch);
			rebinner.setDropDegen(dropdeg);
			rebinner.addHistory(lor, msgs);
			rebinner.setCrop(crop);
			return new image(rebinner.rebin());
		}
		else {
			SPIIC myComplex = _image->getComplexImage();
			ImageRebinner<Complex> rebinner(
				myComplex, _getRegion(region, True).get(),
				mask, outfile, overwrite
			);
			rebinner.setFactors(mybin);
			rebinner.setStretch(stretch);
			rebinner.setDropDegen(dropdeg);
			rebinner.addHistory(lor, msgs);
			rebinner.setCrop(crop);
			return new image(rebinner.rebin());
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::regrid(
	const string& outfile, const vector<int>& inshape,
	const record& csys, const vector<int>& inaxes,
	const variant& region, const variant& vmask,
	const string& method, int decimate, bool replicate,
	bool doRefChange, bool dropDegenerateAxes,
	bool overwrite, bool forceRegrid,
	bool specAsVelocity, bool /* async */,
	bool stretch
) {
	try {
		LogOrigin lor(_class, __func__);
		_log << lor;
		if (detached()) {
		        throw AipsError("Unable to create image");
			return 0;
		}
		unique_ptr<Record> csysRec(toRecord(csys));
		unique_ptr<CoordinateSystem> coordinates(CoordinateSystem::restore(*csysRec, ""));
		ThrowIf (
			! coordinates.get(),
			"Invalid specified coordinate system record."
		);
		SHARED_PTR<Record> regionPtr(_getRegion(region, True));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<Int> axes;
		if (!((inaxes.size() == 1) && (inaxes[0] == -1))) {
			axes = inaxes;
		}
		vector<String> names;
		names += "outfile", "shape", "csys", "axes",
			"region", "mask", "method", "decimate",
			"replicate", "doref", "dropdegen",
			"overwrite", "force", "asvelocity", "stretch";
		vector<variant> values;
		values += outfile, inshape, csys, inaxes,
			region, vmask, method, decimate, replicate,
			doRefChange, dropDegenerateAxes,
			overwrite, forceRegrid,
			specAsVelocity, stretch;
		vector<String> msgs = _newHistory(__func__, names, values);
		if (_image->isFloat()) {
			ImageRegridder regridder(
				_image->getImage(), regionPtr.get(),
				mask, outfile, overwrite, *coordinates,
				IPosition(axes), IPosition(inshape)
			);
			return _regrid(
				regridder, method, decimate, replicate,
				doRefChange, forceRegrid, specAsVelocity,
				stretch, dropDegenerateAxes, lor, msgs
			);
		}
		else {
			ComplexImageRegridder regridder(
				_image->getComplexImage(), regionPtr.get(),
				mask, outfile, overwrite, *coordinates,
				IPosition(axes), IPosition(inshape)
			);
			return _regrid(
				regridder, method, decimate, replicate,
				doRefChange, forceRegrid, specAsVelocity,
				stretch, dropDegenerateAxes, lor, msgs
			);
		}

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

vector<String> image::_newHistory(
	const string& method, const vector<String>& names,
	const vector<variant>& values
) {
	AlwaysAssert(names.size() == values.size(), AipsError);
	vector<String> msgs;
	ostringstream os;
	os << "Ran ia." << method;
	msgs.push_back(os.str());
	vector<std::pair<String, variant> > inputs;
	for (uInt i=0; i<names.size(); i++) {
		inputs.push_back(make_pair(names[i], values[i]));
	}
	os.str("");
	os << "ia." << method << _inputsString(inputs);
	msgs.push_back(os.str());
	return msgs;
}

template <class T> image* image::_regrid(
	ImageRegridderBase<T>& regridder,
	const string& method, int decimate,	bool replicate,
	bool doRefChange, bool forceRegrid,
	bool specAsVelocity, bool stretch,
	bool dropDegenerateAxes, const LogOrigin& lor,
	const vector<String>& msgs
) {
	regridder.setMethod(method);
	regridder.setDecimate(decimate);
	regridder.setReplicate(replicate);
	regridder.setDoRefChange(doRefChange);
	regridder.setForceRegrid(forceRegrid);
	regridder.setSpecAsVelocity(specAsVelocity);
	regridder.setStretch(stretch);
	regridder.setDropDegen(dropDegenerateAxes);
	regridder.addHistory(lor, msgs);
	return new image(regridder.regrid());
}

image* image::rotate(
	const std::string& outfile, const std::vector<int>& inshape,
	const variant& inpa, const variant& region,
	const variant& vmask, const std::string& method,
	const int decimate, const bool replicate, const bool dropdeg,
	const bool overwrite, const bool /* async */, const bool stretch
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
		        throw AipsError("Unable to create image");
			return 0;
		}
		Vector<Int> shape(inshape);
		Quantum<Double> pa(_casaQuantityFromVar(inpa));
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		SPIIF pImOut(
			_image->rotate(
				outfile, shape, pa, *Region, mask, method,
				decimate, replicate, dropdeg, overwrite, stretch
			)
		);
		return new image(pImOut);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::rotatebeam(const variant& angle) {
	try {
		_log << _ORIGIN;
		if(detached()) {
			return False;
		}
		Quantum<Double> pa(_casaQuantityFromVar(angle));
		if (_image->isFloat()) {
			BeamManipulator<Float> bManip(_image->getImage());
			bManip.rotate(pa);
		}
		else {
			BeamManipulator<Complex> bManip(_image->getComplexImage());
			bManip.rotate(pa);
		}
		return true;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::rename(const std::string& name, const bool overwrite) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}

		if (_image->rename(name, overwrite)) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error renaming image.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::replacemaskedpixels(
	const variant& pixels,
	const variant& region, const variant& vmask,
	const bool updateMask, const bool list,
	const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return False;
	}
	try {
		SHARED_PTR<Record> regionPtr = _getRegion(region, True);

		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(_ORIGIN, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("pixels", pixels));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", vmask));
			inputs.push_back(make_pair("update", updateMask));
			inputs.push_back(make_pair("list", list));
			inputs.push_back(make_pair("stretch", stretch));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(_ORIGIN, os.str()));
		}
		if (_image->isFloat()) {
			SPIIF myfloat = _image->getImage();
			ImageMaskedPixelReplacer<Float> impr(
				myfloat, regionPtr.get(), mask
			);
			impr.setStretch(stretch);
			impr.replace(pixels.toString(), updateMask, list);
			_image.reset(new ImageAnalysis(myfloat));
			ImageHistory<Float> hist(myfloat);
			hist.addHistory(msgs);
		}
		else {
			SPIIC mycomplex = _image->getComplexImage();
			ImageMaskedPixelReplacer<Complex> impr(
				mycomplex, regionPtr.get(), mask
			);
			impr.setStretch(stretch);
			impr.replace(pixels.toString(), updateMask, list);
			_image.reset(new ImageAnalysis(mycomplex));
			ImageHistory<Complex> hist(mycomplex);
			hist.addHistory(msgs);
		}
		_stats.reset(0);
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::restoringbeam(int channel, int polarization) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		if (_image->isFloat()) {
			return fromRecord(
				_image->getImage()->imageInfo().beamToRecord(
					channel, polarization
				)
			);
		}
		else {
			return fromRecord(
				_image->getComplexImage()->imageInfo().beamToRecord(
					channel, polarization
				)
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image* image::sepconvolve(
	const std::string& outFile, const std::vector<int>& axes,
	const std::vector<std::string>& types,
	const ::casac::variant& widths,
	const double Scale, const variant& region,
	const ::casac::variant& vmask, const bool overwrite,
	const bool /* async */, const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		ThrowCc("Unable to create image");
	}
	try {
		UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
		SHARED_PTR<Record> pRegion(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<Int> smoothaxes(axes);
		Vector<String> kernels = toVectorString(types);

		int num = 0;
		Vector<Quantum<Double> > kernelwidths;
		if (widths.type() == ::casac::variant::INTVEC) {
			std::vector<int> widthsIVec = widths.toIntVec();
			num = widthsIVec.size();
			std::vector<double> widthsVec(num);
			for (int i = 0; i < num; i++)
				widthsVec[i] = widthsIVec[i];
			kernelwidths.resize(num);
			Unit u("pix");
			for (int i = 0; i < num; i++) {
				kernelwidths[i] = casa::Quantity(widthsVec[i], u);
			}
		}
		else if (widths.type() == ::casac::variant::DOUBLEVEC) {
			std::vector<double> widthsVec = widths.toDoubleVec();
			num = widthsVec.size();
			kernelwidths.resize(num);
			Unit u("pix");
			for (int i = 0; i < num; i++) {
				kernelwidths[i] = casa::Quantity(widthsVec[i], u);
			}
		}
		else if (widths.type() == ::casac::variant::STRING || widths.type()
				== ::casac::variant::STRINGVEC) {
			toCasaVectorQuantity(widths, kernelwidths);
			num = kernelwidths.size();
		}
		else {
			_log << LogIO::WARN << "Unrecognized kernelwidth datatype"
					<< LogIO::POST;
			return 0;
		}
		if (kernels.size() == 1 && kernels[0] == "") {
			kernels.resize(num);
			for (int i = 0; i < num; i++) {
				kernels[i] = "gauss";
			}
		}
		if (
			smoothaxes.size() == 0 || ((smoothaxes.size() == 1)
			&& (smoothaxes[0] = -1))
		) {
			smoothaxes.resize(num);
			for (int i = 0; i < num; i++) {
				smoothaxes[i] = i;
			}
		}
		SHARED_PTR<ImageInterface<Float> > pImOut(
			_image->sepconvolve(outFile,
				smoothaxes, kernels, kernelwidths,
				Scale, *pRegion, mask, overwrite,
				stretch
			)
		);
		return new image(pImOut);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::set(
	const variant& vpixels, int pixelmask,
	const variant& region, bool list
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}

		String pixels = vpixels.toString();
		if (pixels == "[]")
			pixels = "";
		SHARED_PTR<Record> pRegion(_getRegion(region, False));

		if (pixels == "" && pixelmask == -1) {
			_log << LogIO::WARN
					<< "You must specify at least either the pixels or the mask to set"
					<< LogIO::POST;
			return False;
		}
		if (_image->set(pixels, pixelmask, *pRegion, list)) {
			_stats.reset(0);
			return True;
		}
		ThrowCc("Error setting pixel values.");
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setbrightnessunit(const std::string& unit) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}
		Bool res = _image->isFloat()
			?  _image->getImage()->setUnits(Unit(unit))
			: _image->getComplexImage()->setUnits(Unit(unit));
		ThrowIf(! res, "Unable to set brightness unit");
		_stats.reset(0);
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setcoordsys(const ::casac::record& csys) {
	bool rstat(false);
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}

		Record *coordinates = toRecord(csys);
		rstat = _image->setcoordsys(*coordinates);
		delete coordinates;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::sethistory(const std::string& origin,
		const std::vector<std::string>& history) {
	try {
		if (detached()) {
			return false;
		}
		if ((history.size() == 1) && (history[0].size() == 0)) {
			LogOrigin lor("image", "sethistory");
			_log << lor << "history string is empty" << LogIO::POST;
		} else {
			if(_image->isFloat()) {
				ImageHistory<Float> hist(_image->getImage());
				hist.addHistory(origin, history);
			}
			else {
				ImageHistory<Complex> hist(_image->getComplexImage());
				hist.addHistory(origin, history);
			}
		}
		return True;
	}
	catch (const AipsError& x) {
		LogOrigin lor("image", __func__);
		_log << lor << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setmiscinfo(const ::casac::record& info) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return false;
		}
		std::unique_ptr<Record> tmp(toRecord(info));
		Bool res = _image->isFloat()
			? _image->getImage()->setMiscInfo(*tmp)
			: _image->getComplexImage()->setMiscInfo(*tmp);
		return res;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::vector<int> image::shape() {
	std::vector<int> rstat(0);
	_log << _ORIGIN;
	if (detached()) {
		return rstat;
	}
	try {
		rstat = _image->isFloat()
			? _image->getImage()->shape().asVector().tovector()
			: _image->getComplexImage()->shape().asVector().tovector();
		return rstat;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setrestoringbeam(
	const variant& major, const variant& minor,
	const variant& pa, const record& beam,
	bool remove, bool log,
	int channel, int polarization,
	const string& imagename
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return false;
		}
		std::unique_ptr<Record> rec(toRecord(beam));
		ImageBeamSet bs;
		if (! imagename.empty()) {
			ThrowIf(
				! major.empty() || ! minor.empty() || ! pa.empty(),
				"Cannot specify both imagename and major, minor, and/or pa"
			);
			ThrowIf(
				remove,	"remove cannot be true if imagename is specified"
			);
			ThrowIf(
				! beam.empty(),
				"beam must be empty if imagename specified"
			);
			ThrowIf(
				channel >= 0 || polarization >= 0,
				"Neither channel nor polarization can be non-negative if imagename is specified"
			);
			PtrHolder<ImageInterface<Float> > k;
			ImageUtilities::openImage(k, imagename);
			if (k.ptr() == 0) {
				PtrHolder<ImageInterface<Float> > c;
				ImageUtilities::openImage(c, imagename);
				ThrowIf(
					c.ptr() == 0,
					"Unable to open " + imagename
				);
				bs = c->imageInfo().getBeamSet();
			}
			else {
				bs = k->imageInfo().getBeamSet();
			}
			ThrowIf(
				bs.empty(),
				"Image " + imagename + " has no beam"
			);
		}
		if (_image->isFloat()) {
			BeamManipulator<Float> bManip(_image->getImage());
			bManip.setVerbose(log);
			if (remove) {
				bManip.remove();
			}
			else if (! bs.empty()) {
				bManip.set(bs);
			}
			else {
				bManip.set(
					_casaQuantityFromVar(major), _casaQuantityFromVar(minor),
					_casaQuantityFromVar(pa), *rec, channel, polarization
				);
			}
		}
		else {
			BeamManipulator<Complex> bManip(_image->getComplexImage());
			bManip.setVerbose(log);
			if (remove) {
				bManip.remove();
			}
			else if (! bs.empty()) {
				bManip.set(bs);
			}
			else {
				bManip.set(
					_casaQuantityFromVar(major), _casaQuantityFromVar(minor),
					_casaQuantityFromVar(pa), *rec, channel, polarization
				);
			}
		}
		return true;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::statistics(
	const vector<int>& axes, const variant& region,
	const variant& mask,
	const vector<double>& includepix,
	const vector<double>& excludepix, bool list, bool force,
	bool disk, bool robust, bool verbose,
	bool stretch, const string& logfile,
	bool append, const string& algorithm, double fence,
	const string& center, bool lside, double zscore,
	int maxiter, const string& clmethod
) {
	_log << _ORIGIN;
	if (detached()) {
		_log << "Image not attached" << LogIO::POST;
		return 0;
	}
	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		SHARED_PTR<Record> regionRec(_getRegion(region, True));
		String mtmp = mask.toString();
		if (mtmp == "false" || mtmp == "[]") {
			mtmp = "";
		}
		Vector<Int> tmpaxes(axes);
		if (tmpaxes.size() == 1 && tmpaxes[0] == -1) {
			tmpaxes.resize(0);
		}
		Vector<Float> tmpinclude;
		Vector<Float> tmpexclude;
		if (
			!(
				includepix.size() == 1
				&& includepix[0] == -1
			)
		) {
			tmpinclude.resize(includepix.size());
			for (uInt i=0; i<includepix.size(); i++) {
				tmpinclude[i] = includepix[i];
			}
		}
		if (!(excludepix.size() == 1 && excludepix[0] == -1)) {
			tmpexclude.resize(excludepix.size());
			for (uInt i = 0; i < excludepix.size(); i++) {
				tmpexclude[i] = excludepix[i];
			}
		}
		if (verbose) {
			_log << LogIO::NORMAL << "Determining stats for image "
				<< _name(True) << LogIO::POST;
		}
		Record ret;
		if (force || _stats.get() == 0) {
			_stats.reset(
				new ImageStatsCalculator(
					_image->getImage(), regionRec.get(), mtmp, verbose
				)
			);
		}
		else {
			_stats->setMask(mtmp);
			_stats->setRegion(regionRec ? *regionRec : Record());
		}
		String myalg = algorithm;
		myalg.downcase();
		if (myalg.startsWith("ch")) {
			_stats->configureChauvenet(zscore, maxiter);
		}
		else if (myalg.startsWith("cl")) {
			String mymethod = clmethod;
			mymethod.downcase();
			ImageStatsCalculator::PreferredClassicalAlgorithm method;
			if (mymethod.startsWith("a")) {
				method = ImageStatsCalculator::AUTO;
			}
			else if (mymethod.startsWith("t")) {
				method = ImageStatsCalculator::TILED_APPLY;
			}
			else if (mymethod.startsWith("f")) {
				method = ImageStatsCalculator::STATS_FRAMEWORK;
			}
			else {
				ThrowCc("Unsupported classical method " + clmethod);
			}
			_stats->configureClassical(method);
		}
		else if (myalg.startsWith("f")) {
			String mycenter = center;
			mycenter.downcase();
			FitToHalfStatisticsData::CENTER centerType;
			if (mycenter.startsWith("mea")) {
				centerType = FitToHalfStatisticsData::CMEAN;
			}
			else if (mycenter.startsWith("med")) {
				centerType = FitToHalfStatisticsData::CMEDIAN;
			}
			else if (mycenter.startsWith("z")) {
				centerType = FitToHalfStatisticsData::CVALUE;
			}
			else {
				ThrowCc("Unsupported center value " + center);
			}
			FitToHalfStatisticsData::USE_DATA useData = lside
				? FitToHalfStatisticsData::LE_CENTER
				: FitToHalfStatisticsData::GE_CENTER;
			_stats->configureFitToHalf(centerType, useData, 0.0);
		}
		else if (myalg.startsWith("h")) {
			_stats->configureHingesFences(fence);
		}
		else {
			ThrowCc("Unsupported algorithm " + algorithm);
		}
		_stats->setAxes(tmpaxes);
		_stats->setIncludePix(tmpinclude);
		_stats->setExcludePix(tmpexclude);
		_stats->setList(list);
        if (force) {
            _stats->forceNewStorage();
        }
        //_stats->setForce(force);
		_stats->setDisk(disk);
		_stats->setRobust(robust);
		_stats->setVerbose(verbose);
		_stats->setStretch(stretch);
		if (! logfile.empty()) {
			_stats->setLogfile(logfile);
		}
		_stats->setLogfileAppend(append);
		return fromRecord(_stats->calculate());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::twopointcorrelation(
	const std::string& outfile,
	const variant& region, const ::casac::variant& vmask,
	const std::vector<int>& axes, const std::string& method,
	const bool overwrite, const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return false;
	}
	try {
		String outFile(outfile);
		SHARED_PTR<Record> Region(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<Int> iAxes;
		if (!(axes.size() == 1 && axes[0] == -1)) {
			iAxes = axes;
		}
		return _image->twopointcorrelation(
			outFile, *Region, mask, iAxes,
			method, overwrite, stretch
		);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image* image::subimage(
	const string& outfile, const variant& region,
	const variant& vmask, const bool dropDegenerateAxes,
	const bool overwrite, const bool list, const bool stretch,
	const bool wantreturn
) {
	try {
		_log << _ORIGIN;
		ThrowIf(
			detached(),
			"Unable to create image"
		);
		SHARED_PTR<Record> regionRec = _getRegion(region, False);
		String regionStr = region.type() == variant::STRING
			? region.toString()
			: "";
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (outfile.empty() && ! wantreturn) {
			_log << LogIO::WARN << "outfile was not specified and wantreturn is false. "
				<< "The resulting image will be inaccessible" << LogIO::POST;
		}

		SHARED_PTR<ImageAnalysis> ia;
		if (_image->isFloat()) {
			ia.reset(
				new ImageAnalysis(
					_subimage<Float>(
						SHARED_PTR<ImageInterface<Float> >(
							_image->getImage()->cloneII()
						),
						outfile, *regionRec, mask, dropDegenerateAxes,
						overwrite, list, stretch
					)
				)
			);
		}
		else {
			ia.reset(
				new ImageAnalysis(
					_subimage<Complex>(
						SHARED_PTR<ImageInterface<Complex> >(
							_image->getComplexImage()->cloneII()
						),
						outfile, *regionRec, mask, dropDegenerateAxes,
						overwrite, list, stretch
					)
				)
			);
		}
		image *res = wantreturn ? new image(ia) : 0;
		return res;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	// so eclipse doesn't complain
	return nullptr;
}

template<class T> SPIIT image::_subimage(
	SPIIT clone,
	const String& outfile, const Record& region,
	const String& mask, bool dropDegenerateAxes,
	bool overwrite, bool list, bool stretch
) {
	return SPIIT(
		SubImageFactory<T>::createImage(
			*clone, outfile, region,
			mask, dropDegenerateAxes, overwrite, list, stretch
		)
	);
}

record* image::summary(
	const string& doppler, const bool list,
	const bool pixelorder, const bool verbose
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		return fromRecord(
			_image->summary(doppler, list, pixelorder, verbose)
		);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::tofits(
	const std::string& fitsfile, const bool velocity,
	const bool optical, const int bitpix, const double minpix,
	const double maxpix, const variant& region,
	const ::casac::variant& vmask, const bool overwrite,
	const bool dropdeg, const bool deglast, const bool dropstokes,
	const bool stokeslast, const bool wavelength, const bool airwavelength,
	const bool /* async */, const bool stretch,
	const bool history
) {
	_log << _ORIGIN;
	if (detached()) {
		return false;
	}
	try {
		ThrowIf(
			fitsfile.empty(),
			"fitsfile must be specified"
		);
		ThrowIf(
			fitsfile == "." || fitsfile == "..",
			"Invalid fitsfile name " + fitsfile
		);
		SHARED_PTR<Record> pRegion(_getRegion(region, False));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}

		String origin;
		{
			ostringstream buffer;
			buffer << "CASA ";
			VersionInfo::report(buffer);
			origin = String(buffer);
		}
		return _image->tofits(
			fitsfile, velocity, optical, bitpix, minpix,
			maxpix, *pRegion, mask, overwrite, dropdeg,
			deglast, dropstokes, stokeslast, wavelength,
			airwavelength, origin, stretch, history
		);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::toASCII(const std::string& outfile, const variant& region,
		const ::casac::variant& mask, const std::string& sep,
		const std::string& format, const double maskvalue, const bool overwrite,
		const bool stretch) {
	// sep is hard-wired as ' ' which is what imagefromascii expects
	_log << _ORIGIN;
	if (detached()) {
		return False;
	}

	try {
		String Mask;
		if (mask.type() == ::casac::variant::BOOLVEC) {
			Mask = "";
		}
		else if (
			mask.type() == ::casac::variant::STRING
			|| mask.type() == ::casac::variant::STRINGVEC
		) {
			Mask = mask.toString();
		}
		SHARED_PTR<Record> pRegion(_getRegion(region, False));
		return _image->toASCII(
			outfile, *pRegion, Mask, sep, format,
			maskvalue, overwrite, stretch
		);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::string image::type() {
	return "image";
}

//std::vector<double>
::casac::record*
image::topixel(const ::casac::variant& value) {
	//  std::vector<double> rstat;
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", __func__);
		if (detached())
			return rstat;

		Vector<Int> bla;
		CoordinateSystem cSys = _image->coordsys(bla);
		::casac::coordsys mycoords;
		//NOT using _image->toworld as most of the math is in casac namespace
		//in coordsys...should revisit this when casac::coordsys is cleaned
		mycoords.setcoordsys(cSys);
		rstat = mycoords.topixel(value);

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

::casac::record*
image::toworld(const ::casac::variant& value, const std::string& format, bool dovelocity) {
	record *rstat = 0;
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return rstat;
        }
		Vector<Double> pixel;
		if (isunset(value)) {
			pixel.resize(0);
		}
        else if (value.type() == ::casac::variant::DOUBLEVEC) {
			pixel = value.getDoubleVec();
		}
        else if (value.type() == ::casac::variant::INTVEC) {
			variant vcopy = value;
			Vector<Int> ipixel = vcopy.asIntVec();
			Int n = ipixel.size();
			pixel.resize(n);
			for (int i = 0; i < n; i++) {
				pixel[i] = ipixel[i];
            }
		}
        else if (value.type() == ::casac::variant::RECORD) {
			variant localvar(value);
			unique_ptr<Record> tmp(toRecord(localvar.asRecord()));
			if (tmp->isDefined("numeric")) {
				pixel = tmp->asArrayDouble("numeric");
			}
            else {
				_log << LogIO::SEVERE << "unsupported record type for value"
						<< LogIO::EXCEPTION;
				return rstat;
			}
		}
        else {
			_log << LogIO::SEVERE << "unsupported data type for value"
					<< LogIO::EXCEPTION;
			return rstat;
		}
		rstat = fromRecord(_image->toworld(pixel, format, dovelocity));
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::unlock() {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return False;
		}
		if (_image->isFloat()) {
			_image->getImage()->unlock();
		}
		else {
			_image->getComplexImage()->unlock();
		}
		return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::detached() const {
	if ( _image.get() == 0 || _image->detached()) {
		_log <<  _ORIGIN;
		_log << LogIO::SEVERE
			<< "Image is detached - cannot perform operation." << endl
			<< "Call image.open('filename') to reattach." << LogIO::POST;
		return True;
	}
	return False;
}

bool image::maketestimage(
	const std::string& outfile, const bool overwrite
) {
	try {
		_reset();
		_log << _ORIGIN;
		return _image->maketestimage(outfile, overwrite);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromimage(
	const string& infile, const string& outfile,
	const variant& region, const variant& vmask,
	const bool dropdeg, const bool overwrite
) {
	try {
		image *rstat(0);
		std::unique_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;
		String mask;

		if (vmask.type() == variant::BOOLVEC) {
			mask = "";
		}
		else if (
			vmask.type() == variant::STRING
			|| vmask.type() == variant::STRINGVEC
		) {
			mask = vmask.toString();
		}
		else if (vmask.type() == ::casac::variant::RECORD) {
			_log << LogIO::SEVERE
					<< "Don't support region masking yet, only valid LEL "
					<< LogIO::POST;
			throw AipsError("Unable to create image");
			return 0;
		}
		else {
			_log << LogIO::SEVERE
					<< "Mask is not understood, try a valid LEL string "
					<< LogIO::POST;
			throw AipsError("Unable to create image");
			return 0;
		}
		SHARED_PTR<Record> regionPtr(_getRegion(region, False, infile));
        SHARED_PTR<ImageInterface<Float> >outIm(
			newImage->newimage(
				infile, outfile,*regionPtr,
				mask, dropdeg, overwrite
			)
		);
		if (outIm) {
			rstat = new image(outIm);
		} else {
			rstat = new image();
		}
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromfile(const std::string& fileName) {
	try {
		image *rstat(0);
		std::unique_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;
		SHARED_PTR<ImageInterface<Float> > outIm(
			newImage->newimagefromfile(fileName)
		);
		if (outIm.get() != 0) {
			rstat =  new image(outIm);
		} else {
			rstat =  new image();
		}
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimage(const string& fileName) {
	image *rstat = newimagefromfile(fileName);
	if (!rstat)
			throw AipsError("Unable to create image");
	return rstat;
}

image* image::newimagefromarray(
	const string& outfile, const variant& pixels,
	const record& csys, const bool linear,
	const bool overwrite, const bool log
) {
	try {
		unique_ptr<ImageAnalysis> newImage(
			new ImageAnalysis()
		);
		_log << _ORIGIN;
		// Some protection.  Note that a Glish array, [], will
		// propagate through to here to have ndim=1 and shape=0
		Vector<Int> shape = pixels.arrayshape();
		uInt ndim = shape.size();
		if (ndim == 0) {
			_log << "The pixels array is empty" << LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < ndim; i++) {
			if (shape(i) <= 0) {
				_log << "The shape of the pixels array is invalid"
						<< LogIO::EXCEPTION;
			}
		}
		Array<Float> pixelsArray;
		if (pixels.type() == variant::DOUBLEVEC) {
			vector<double> pixelVector = pixels.getDoubleVec();
			Vector<Int> shape = pixels.arrayshape();
			pixelsArray.resize(IPosition(shape));
			Vector<Double> localpix(pixelVector);
			convertArray(pixelsArray, localpix.reform(IPosition(shape)));
		}
		else if (pixels.type() == ::casac::variant::INTVEC) {
			vector<int> pixelVector = pixels.getIntVec();
			Vector<Int> shape = pixels.arrayshape();
			pixelsArray.resize(IPosition(shape));
			Vector<Int> localpix(pixelVector);
			convertArray(pixelsArray, localpix.reform(IPosition(shape)));
		}
		else {
			_log << LogIO::SEVERE
					<< "pixels is not understood, try using an array "
					<< LogIO::POST;
			return new image();
		}
		unique_ptr<Record> coordinates(toRecord(csys));
        SHARED_PTR<ImageInterface<Float> > outIm(
			newImage->newimagefromarray(
				outfile, pixelsArray, *coordinates,
				linear, overwrite, log
			)
		);
        ThrowIf(
            ! outIm,
            "Unable to create image"
        );
		return new image(outIm);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromshape(
	const string& outfile, const vector<int>& shape,
	const record& csys, bool linear,
	bool overwrite, bool log, const string& type
) {
	try {
        LogOrigin lor("image", __func__);
		_log << lor;
		unique_ptr<Record> coordinates(toRecord(csys));
        String mytype = type;
        mytype.downcase();
        ThrowIf(
            ! mytype.startsWith("f") && ! mytype.startsWith("c"),
            "Input parm type must start with either 'f' or 'c'"
        );
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(lor, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("shape", shape));
			inputs.push_back(make_pair("csys", csys));
			inputs.push_back(make_pair("linear", linear));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("log", log));
			inputs.push_back(make_pair("type", type));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(lor, os.str()));
		}
        if (mytype.startsWith("f")) {
            SPIIF myfloat = ImageFactory::floatImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            return new image(myfloat);
        }
        else {
            SPIIC mycomplex = ImageFactory::complexImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            return new image(mycomplex);
        }
    }
    catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromfits(
	const string& outfile, const string& fitsfile,
	const int whichrep, const int whichhdu,
	const bool zeroBlanks, const bool overwrite
) {
	try {
		image *rstat(0);
		unique_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;
        SHARED_PTR<ImageInterface<Float> > outIm(
			newImage->newimagefromfits(
				outfile,
				fitsfile, whichrep, whichhdu, zeroBlanks, overwrite
			)
		);
		if (outIm.get() != 0) {
			rstat = new image(outIm);
		} else {
			rstat = new image();
		}
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::variant*
image::makearray(const double v, const std::vector<int>& shape) {
	int ndim = shape.size();
	int nelem = 1;
	for (int i = 0; i < ndim; i++)
		nelem *= shape[i];
	std::vector<double> element(nelem);
	for (int i = 0; i < nelem; i++)
		element[i] = v;
	return new ::casac::variant(element, shape);
}

casac::record*
image::recordFromQuantity(const casa::Quantity q) {
	::casac::record *r = 0;
	try {
		_log << LogOrigin("image", "recordFromQuantity");
		String error;
		casa::Record R;
		if (QuantumHolder(q).toRecord(error, R)) {
			r = fromRecord(R);
		} else {
			_log << LogIO::SEVERE << "Could not convert quantity to record."
					<< LogIO::POST;
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return r;
}

::casac::record*
image::recordFromQuantity(const Quantum<Vector<Double> >& q) {
	::casac::record *r = 0;
	try {
		_log << LogOrigin("image", "recordFromQuantity");
		String error;
		casa::Record R;
		if (QuantumHolder(q).toRecord(error, R)) {
			r = fromRecord(R);
		} else {
			_log << LogIO::SEVERE << "Could not convert quantity to record."
					<< LogIO::POST;
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return r;
}

casa::Quantity image::_casaQuantityFromVar(const ::casac::variant& theVar) {
	try {
		_log << _ORIGIN;
		casa::QuantumHolder qh;
		String error;
		if (
			theVar.type() == ::casac::variant::STRING
			|| theVar.type() == ::casac::variant::STRINGVEC
		) {
			ThrowIf(
				!qh.fromString(error, theVar.toString()),
				"Error " + error + " in converting quantity "
			);
		}
		else if (theVar.type() == ::casac::variant::RECORD) {
			//NOW the record has to be compatible with QuantumHolder::toRecord
			::casac::variant localvar(theVar); //cause its const
			unique_ptr<Record> ptrRec(toRecord(localvar.asRecord()));
			ThrowIf(
				!qh.fromRecord(error, *ptrRec),
				"Error " + error + " in converting quantity "
			);
		}
        else if (theVar.type() == variant::BOOLVEC) {
            return casa::Quantity();
        }
		return qh.asQuantity();
	}
	catch (const AipsError& x) {
		_log << LogOrigin("image", __func__);
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::isconform(const string& other) {
	_log << _ORIGIN;

	if (detached()) {
		return False;
	}
	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		ImageInterface<Float> *oth = 0;
		ImageUtilities::openImage(oth, String(other));
		if (oth == 0) {
			throw AipsError("Unable to open image " + other);
		}
		std::unique_ptr<ImageInterface<Float> > x(oth);
        SHARED_PTR<const ImageInterface<Float> > mine = _image->getImage();
		if (mine->shape().isEqual(x->shape()) && mine->coordinates().near(
				x->coordinates())) {
			Vector<String> mc = mine->coordinates().worldAxisNames();
			Vector<String> oc = x->coordinates().worldAxisNames();
			if (mc.size() != oc.size()) {
				return False;
			}
			for (uInt i = 0; i < mc.size(); i++) {
				if (mc[i] != oc[i]) {
					return False;
				}
			}
			return True;
		}
		return False;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

SHARED_PTR<Record> image::_getRegion(
	const variant& region, const bool nullIfEmpty, const string& otherImageName
) const {
	switch (region.type()) {
	case variant::BOOLVEC:
		return SHARED_PTR<Record>(nullIfEmpty ? 0 : new Record());
	case variant::STRING: {
		IPosition shape;
		CoordinateSystem csys;
		if (otherImageName.empty()) {
			ThrowIf(
				! _image,
				"No attached image. Cannot use a string value for region"
			);
			if (_image->isFloat()) {
				shape = _image->getImage()->shape();
				csys = _image->getImage()->coordinates();
			}
			else {
				shape = _image->getComplexImage()->shape();
				csys = _image->getComplexImage()->coordinates();
			}
		}
		else {
			PtrHolder<ImageInterface<Float> > image;
			ImageUtilities::openImage(image, otherImageName);
			if (image.ptr()) {
				shape = image->shape();
				csys = image->coordinates();
			}
			else {
				PtrHolder<ImageInterface<Complex> > imagec;
				ImageUtilities::openImage(imagec, otherImageName);
				ThrowIf(
					! imagec.ptr(),
					"Unable to open image " + otherImageName
				);
				shape = imagec->shape();
				csys = imagec->coordinates();
			}
		}
		return SHARED_PTR<Record>(
			region.toString().empty()
				? nullIfEmpty ? 0 : new Record()
				: new Record(
					CasacRegionManager::regionFromString(
						csys, region.toString(),
						_name(False), shape
					)
				)
		);
	}
	case variant::RECORD:
		{
			SHARED_PTR<variant> clon(region.clone());
			return SHARED_PTR<Record>(
				nullIfEmpty && region.size() == 0
					? 0
					: toRecord(
						SHARED_PTR<variant>(region.clone())->asRecord()
					)
			);
		}
	default:
		ThrowCc("Unsupported type for region " + region.typeString());
	}
}

vector<double> image::_toDoubleVec(const variant& v) {
	variant::TYPE type = v.type();
	ThrowIf(
		type != variant::INTVEC && type != variant::LONGVEC
		&& type != variant::DOUBLEVEC,
		"variant is not a numeric array"
	);
	vector<double> output;
	if (type == variant::INTVEC || type == variant::LONGVEC) {
		Vector<Int> x = v.toIntVec();
		std::copy(x.begin(), x.end(), std::back_inserter(output));
	}
	if (type == variant::DOUBLEVEC) {
		Vector<Double> x = v.toDoubleVec();
		std::copy(x.begin(), x.end(), std::back_inserter(output));
	}
	return output;
}



} // casac namespace
