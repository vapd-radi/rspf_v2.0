//*****************************************************************************
// FILE: rspfRpcRcz2XyModel.cpp
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfRpcRcz2XyModel.
//   This is a replacement model utilizing the Rational Polynomial Coefficients
//   (RPC), a.k.a. Rapid Positioning Capability, and Universal Sensor Model
//   (USM).
//
// LIMITATIONS: Does not support parameter adjustment (YET)
//
//*****************************************************************************
//  $Id: rspfRpcRcz2XyModel.cpp 11522 2007-08-07 21:57:59Z dburken $

#include <rspf/projection/rspfRpcRcz2XyModel.h>
#include <rspf/elevation/rspfElevManager.h>


RTTI_DEF1(rspfRpcRcz2XyModel, "rspfRpcRcz2XyModel", rspfSensorModel);

#include <rspf/elevation/rspfHgtRef.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfTieGptSet.h>
#include <iterator>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfRpcModel:exec");
static rspfTrace traceDebug ("rspfRpcModel:debug");

static const int    MODEL_VERSION_NUMBER  = 1;
static const int    NUM_COEFFS        = 20;
static const char*  MODEL_TYPE        = "rspfRpcModel";
static const char*  POLY_TYPE_KW      = "polynomial_format";
static const char*  LINE_SCALE_KW     = "line_scale";
static const char*  SAMP_SCALE_KW     = "samp_scale";
static const char*  LAT_SCALE_KW      = "lat_scale";
static const char*  LON_SCALE_KW      = "long_scale";
static const char*  HGT_SCALE_KW      = "height_scale";
static const char*  LINE_OFFSET_KW    = "line_off";
static const char*  SAMP_OFFSET_KW    = "samp_off";
static const char*  LAT_OFFSET_KW     = "lat_off";
static const char*  LON_OFFSET_KW     = "long_off";
static const char*  HGT_OFFSET_KW     = "height_off";

static const char*  BIAS_ERROR_KW     = "bias_error";
static const char*  RAND_ERROR_KW     = "rand_error";

static const char*  LINE_NUM_COEF_KW  = "line_num_coeff_";
static const char*  LINE_DEN_COEF_KW  = "line_den_coeff_";
static const char*  SAMP_NUM_COEF_KW  = "samp_num_coeff_";
static const char*  SAMP_DEN_COEF_KW  = "samp_den_coeff_";

static const rspfString PARAM_NAMES[] ={"intrack_offset",
                                        "crtrack_offset",
                                        "intrack_scale",
                                        "crtrack_scale",
                                        "map_rotation",
                                        "yaw_offset"};
static const rspfString PARAM_UNITS[] ={"pixel",
                                        "pixel",
                                        "scale",
                                        "scale",
                                        "degrees",
                                        "degrees"};

//*****************************************************************************
//  DEFAULT CONSTRUCTOR: rspfRpcModel()
//  
//*****************************************************************************
rspfRpcRcz2XyModel::rspfRpcRcz2XyModel()
   :  rspfSensorModel(),
      thePolyType     (A),
      theLineScale    (0.0),
      theSampScale    (0.0),
      theLatScale     (0.0),
      theLonScale     (0.0),
      theHgtScale     (0.0),
      theLineOffset   (0.0),
      theSampOffset   (0.0),
      theLatOffset    (0.0),
      theLonOffset    (0.0),
      theHgtOffset    (0.0),
      theIntrackOffset(0.0),
      theCrtrackOffset(0.0),
      theIntrackScale (0.0),
      theCrtrackScale (0.0),
      theCosMapRot    (0.0),
      theSinMapRot    (0.0),
      theBiasError    (0.0),
      theRandError    (0.0)

{
   initAdjustableParameters();
}

//*****************************************************************************
//  COPY CONSTRUCTOR: rspfRpcModel(rspfRpcModel)
//  
//*****************************************************************************
rspfRpcRcz2XyModel::rspfRpcRcz2XyModel(const rspfRpcRcz2XyModel& model)
   :
      rspfSensorModel(model),
      thePolyType     (model.thePolyType),
      theLineScale    (model.theLineScale),
      theSampScale    (model.theSampScale),
      theLatScale     (model.theLatScale),
      theLonScale     (model.theLonScale),
      theHgtScale     (model.theHgtScale),
      theLineOffset   (model.theLineOffset),
      theSampOffset   (model.theSampOffset),
      theLatOffset    (model.theLatOffset),
      theLonOffset    (model.theLonOffset),
      theHgtOffset    (model.theHgtOffset),
      theIntrackOffset(model.theIntrackOffset),
      theCrtrackOffset(model.theCrtrackOffset),
      theIntrackScale(model.theIntrackScale),
      theCrtrackScale(model.theCrtrackScale),
      theCosMapRot    (model.theCosMapRot),
      theSinMapRot    (model.theSinMapRot),
      theBiasError    (model.theBiasError),
      theRandError    (model.theRandError)
{
   for (int i=0; i<20; ++i  )
   {
      theLineNumCoef[i] = model.theLineNumCoef[i];
      theLineDenCoef[i] = model.theLineDenCoef[i];
      theSampNumCoef[i] = model.theSampNumCoef[i];
      theSampDenCoef[i] = model.theSampDenCoef[i];
   }
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfRpcModel()
//  
//*****************************************************************************
rspfRpcRcz2XyModel::~rspfRpcRcz2XyModel()
{
}

void rspfRpcRcz2XyModel::setAttributes(rspf_float64 sampleOffset,
                                  rspf_float64 lineOffset,
                                  rspf_float64 sampleScale,
                                  rspf_float64 lineScale,
                                  rspf_float64 latOffset,
                                  rspf_float64 lonOffset,
                                  rspf_float64 heightOffset,
                                  rspf_float64 latScale,
                                  rspf_float64 lonScale,
                                  rspf_float64 heightScale,
                                  const std::vector<double>& xNumeratorCoeffs,
                                  const std::vector<double>& xDenominatorCoeffs,
                                  const std::vector<double>& yNumeratorCoeffs,
                                  const std::vector<double>& yDenominatorCoeffs,
                                  PolynomialType polyType,
                                  bool computeGsdFlag)
{
   thePolyType = polyType;
   
   theLineScale  = lineScale;
   theSampScale  = sampleScale;
   theLatScale   = latScale;
   theLonScale   = lonScale;
   theHgtScale   = heightScale;
   theLineOffset = lineOffset;
   theSampOffset = sampleOffset;
   theLatOffset  = latOffset;
   theLonOffset  = lonOffset;
   theHgtOffset  = heightOffset;
   theCosMapRot = 1;
   theSinMapRot = 0;

   if(xNumeratorCoeffs.size() == 20)
   {
      std::copy(xNumeratorCoeffs.begin(),
                xNumeratorCoeffs.end(),
                theSampNumCoef);
   }
   if(xDenominatorCoeffs.size() == 20)
   {
      std::copy(xDenominatorCoeffs.begin(),
                xDenominatorCoeffs.end(),
                theSampDenCoef);
   }
   if(yNumeratorCoeffs.size() == 20)
   {
      std::copy(yNumeratorCoeffs.begin(),
                yNumeratorCoeffs.end(),
                theLineNumCoef);
   }
   if(yDenominatorCoeffs.size() == 20)
   {
      std::copy(yDenominatorCoeffs.begin(),
                yDenominatorCoeffs.end(),
                theLineDenCoef);
   }

   if(computeGsdFlag)
   {
      try
      {
         // This will set theGSD and theMeanGSD. Method throws rspfException.
         computeGsd();
      }
      catch (const rspfException& e)
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfNitfRpcModel::rspfNitfRpcModel Caught Exception:\n"
               << e.what() << std::endl;
         }
      }
   }
}

void rspfRpcRcz2XyModel::setAttributes(rspfRpcRcz2XyModel::rpcModelStruct& model, bool computeGsdFlag)
{
	thePolyType = PolynomialType(model.type);

	theLineScale  = model.lineScale;
	theSampScale  = model.sampScale;
	theLatScale   = model.latScale;
	theLonScale   = model.lonScale;
	theHgtScale   = model.hgtScale;
	theLineOffset = model.lineOffset;
	theSampOffset = model.sampOffset;
	theLatOffset  = model.latOffset;
	theLonOffset  = model.lonOffset;
	theHgtOffset  = model.hgtOffset;
	theCosMapRot = 1;
	theSinMapRot = 0;

	int i;
	for(i = 0;i < 20;i++)
	{
		theSampNumCoef[i] = model.sampNumCoef[i];
		theLineNumCoef[i] = model.lineNumCoef[i];
		theSampDenCoef[i] = model.sampDenCoef[i];
		theLineDenCoef[i] = model.lineDenCoef[i];
	}

	if(computeGsdFlag)
	{
		try
		{
			// This will set theGSD and theMeanGSD. Method throws rspfException.
			computeGsd();
		}
		catch (const rspfException& e)
		{
			if (traceDebug())
			{
				rspfNotify(rspfNotifyLevel_DEBUG)
					<< "rspfNitfRpcModel::rspfNitfRpcModel Caught Exception:\n"
					<< e.what() << std::endl;
			}
		}
	}
}

void rspfRpcRcz2XyModel::setMetersPerPixel(const rspfDpt& metersPerPixel)
{
   theGSD = metersPerPixel;
   theMeanGSD = (theGSD.x+theGSD.y)*.5;
}

void rspfRpcRcz2XyModel::computeGsd()
{
   //---
   // Get the reference ground point. Note that we will NOT use theRefImgPt
   // as this can be set from tags and such that are not accurate.
   //---
   rspfGpt centerGpt;
   lineSampleHeightToWorld(theRefImgPt,
                           theHgtOffset,
                           centerGpt);
   if (centerGpt.hasNans())
   {
      std::string e = "rspfRpcRcz2XyModel::computeGSD error centerGpt has nans!";
      throw rspfException(e);
   }

   // Get the ground point to the right of the reference point.
   rspfGpt rightGpt;
   lineSampleHeightToWorld(theRefImgPt + rspfDpt(1, 0),
                           theHgtOffset,
                           rightGpt);
   if (rightGpt.hasNans())
   {
      std::string e = "rspfRpcRcz2XyModel::computeGSD error rightGpt has nans!";
      throw rspfException(e);
   }

   // Get the ground point one up from the reference point.
   rspfGpt topGpt;
   lineSampleHeightToWorld(theRefImgPt + rspfDpt(0, -1),
                           theHgtOffset,
                           topGpt);
   if (topGpt.hasNans())
   {
      std::string e = "rspfRpcRcz2XyModel::computeGSD error topGpt has nans!";
      throw rspfException(e);
   }

   rspfEcefPoint rightPt = rightGpt;
   rspfEcefPoint topPt   = topGpt;
   rspfEcefPoint origin  = centerGpt;
      
   theGSD.x   = (rightPt-origin).magnitude();
   theGSD.y   = (topPt-origin).magnitude();
   theMeanGSD = (theGSD.x + theGSD.y)/2.0;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfRpcRcz2XyModel::computGsd DEBUG:"
         << "\ntheGSD: " << theGSD
         << "\ntheMeanGSD: " << theMeanGSD << std::endl;
   }
}

void rspfRpcRcz2XyModel::setPositionError(const rspf_float64& biasError,
                                     const rspf_float64& randomError,
                                     bool initNominalPostionErrorFlag)
{
   theBiasError = biasError;
   theRandError = randomError;
   if (initNominalPostionErrorFlag)
   {
      theNominalPosError = sqrt(theBiasError*theBiasError +
                                theRandError*theRandError); // meters
   }
}

//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::worldToLineSample()
//  
//  Overrides base class implementation. Directly computes line-sample from
//  the polynomials.
//*****************************************************************************
void rspfRpcRcz2XyModel::worldToLineSample(const rspfGpt& ground_point,
                                      rspfDpt&       img_pt) const
{
   rspfDpt dpt = m_proj->forward(ground_point);

   //***
   // Constants for convergence tests:
   //***
   static const int    MAX_NUM_ITERATIONS  = 10;
   static const double CONVERGENCE_EPSILON = 0.1 * 0.5 * (theGSD.x + theGSD.y);  // pixels
   
   //***
   // The image point must be adjusted by the adjustable parameters as well
   // as the scale and offsets given as part of the RPC param normalization.
   //
   //      NOTE: U = line, V = sample
   //***
   double V    = (dpt.x - theLatOffset - theIntrackOffset) / (theLatScale+theIntrackScale);
   double U    = (dpt.y - theLonOffset - theCrtrackOffset) / (theLonScale+theCrtrackScale);

   //***
   // Rotate the normalized U, V by the map rotation error (adjustable param):
   //***
   double U_rot = theCosMapRot*U - theSinMapRot*V;
   double V_rot = theSinMapRot*U + theCosMapRot*V;
   U = U_rot; V = V_rot;

   double nhgt;

   if( ground_point.isHgtNan() )
   {
	   nhgt = (theHgtScale - theHgtOffset) / theHgtScale;
   }
   else
   {
	   nhgt = (ground_point.hgt - theHgtOffset) / theHgtScale;
   }


   // now apply adjust intrack and cross track
   //***
   // Initialize quantities to be used in the iteration for ground point:
   //***

   double nline      = 0.0;  // normalized latitude
   double nsamp      = 0.0;  // normalized longitude
   
   double epsilonU = CONVERGENCE_EPSILON/(theLatScale+theIntrackScale);
   double epsilonV = CONVERGENCE_EPSILON/(theLonScale+theCrtrackScale);
   int    iteration = 0;

   //***
   // Declare variables only once outside the loop. These include:
   // * polynomials (numerators Pu, Pv, and denominators Qu, Qv),
   // * partial derivatives of polynomials wrt X, Y,
   // * computed normalized image point: Uc, Vc,
   // * residuals of normalized image point: deltaU, deltaV,
   // * partial derivatives of Uc and Vc wrt X, Y,
   // * corrections to normalized lat, lon: deltaLat, deltaLon.
   //***
   double Nu, Du, Nv, Dv;
   double dNu_dLine, dDu_dLine, dNv_dLine, dDv_dLine;
   double dNu_dSamp, dDu_dSamp, dNv_dSamp, dDv_dSamp;
   double Uc, Vc;
   double deltaU, deltaV;
   double dU_dLine, dU_dSamp, dV_dLine, dV_dSamp, W;
   double deltaLine, deltaSamp;
   
   //***
   // Now iterate until the computed Uc, Vc is within epsilon of the desired
   // image point U, V:
   //***
   do
   {
      //***
      // Calculate the normalized line and sample Uc, Vc as ratio of
      // polynomials Pu, Qu and Pv, Qv:
      //***
      Nu = polynomial(nline, nsamp, nhgt, theLineNumCoef);
      Du = polynomial(nline, nsamp, nhgt, theLineDenCoef);
      Nv = polynomial(nline, nsamp, nhgt, theSampNumCoef);
      Dv = polynomial(nline, nsamp, nhgt, theSampDenCoef);
      Uc = Nu/Du;
      Vc = Nv/Dv;
      
      //***
      // Compute residuals between desired and computed line, sample:
      //***
      deltaU = U - Uc;
      deltaV = V - Vc;
      
      //***
      // Check for convergence and skip re-linearization if converged:
      //***
      if ((fabs(deltaU) > epsilonU) || (fabs(deltaV) > epsilonV))
      {
         //***
         // Analytically compute the partials of each polynomial wrt lat, lon:
         //***
         dNu_dLine = dPoly_dLine(nline, nsamp, nhgt, theLineNumCoef);
         dDu_dLine = dPoly_dLine(nline, nsamp, nhgt, theLineDenCoef);
         dNv_dLine = dPoly_dLine(nline, nsamp, nhgt, theSampNumCoef);
         dDv_dLine = dPoly_dLine(nline, nsamp, nhgt, theSampDenCoef);
         dNu_dSamp = dPoly_dSamp(nline, nsamp, nhgt, theLineNumCoef);
         dDu_dSamp = dPoly_dSamp(nline, nsamp, nhgt, theLineDenCoef);
         dNv_dSamp = dPoly_dSamp(nline, nsamp, nhgt, theSampNumCoef);
         dDv_dSamp = dPoly_dSamp(nline, nsamp, nhgt, theSampDenCoef);
         
         //***
         // Analytically compute partials of quotients U and V wrt lat, lon: 
         //***
         dU_dLine = (Du*dNu_dLine - Nu*dDu_dLine)/(Du*Du);
         dU_dSamp = (Du*dNu_dSamp - Nu*dDu_dSamp)/(Du*Du);
         dV_dLine = (Dv*dNv_dLine - Nv*dDv_dLine)/(Dv*Dv);
         dV_dSamp = (Dv*dNv_dSamp - Nv*dDv_dSamp)/(Dv*Dv);
         
         W = dU_dSamp*dV_dLine - dU_dLine*dV_dSamp;
         
         //***
         // Now compute the corrections to normalized lat, lon:
         //***
         deltaLine = (dU_dSamp*deltaV - dV_dSamp*deltaU) / W;
         deltaSamp = (dV_dLine*deltaU - dU_dLine*deltaV) / W;
         nline += deltaLine;
         nsamp += deltaSamp;
      }
      
      iteration++;
      
   } while (((fabs(deltaU)>epsilonU) || (fabs(deltaV)>epsilonV))
            && (iteration < MAX_NUM_ITERATIONS));
      
   //***
   // Test for exceeding allowed number of iterations. Flag error if so:
   //***
   if (iteration == MAX_NUM_ITERATIONS)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfRpcRcz2XyModel::lineSampleHeightToWorld: \nMax number of iterations reached in ground point "
                                         << "solution. Results are inaccurate." << endl;
   }

   //***
   // Now un-normalize the ground point lat, lon and establish return quantity:
   //***
   img_pt.line = nline*theLineScale + theLineOffset;
   img_pt.samp = nsamp*theSampScale + theSampOffset;
   return;

}

//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::lineSampleToWorld()
//  
//  Overrides base class implementation. Performs DEM intersection.
//*****************************************************************************
void  rspfRpcRcz2XyModel::lineSampleToWorld(const rspfDpt& imagePoint,
                                       rspfGpt&       worldPoint) const
{
	//lineSampleHeightToWorld2(imagePoint, theHgtOffset, worldPoint);
	//return;
	double epsilonHeight = 0.01;
	double oldHeight = theHgtScale;
	int iteration = 0;
	int MAX_NUM_ITERATIONS = 10;
	double deltaHeight;
	do 
	{
		lineSampleHeightToWorld(imagePoint, oldHeight, worldPoint);
		double newHeight = rspfElevManager::instance()->getHeightAboveEllipsoid(worldPoint);
		deltaHeight = newHeight - oldHeight;
		oldHeight = newHeight;
		iteration++;
	} while (fabs(deltaHeight) > epsilonHeight && (iteration < MAX_NUM_ITERATIONS));

	if (iteration == MAX_NUM_ITERATIONS)
	{
		rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfRpcRcz2XyModel::lineSampleToWorld: \nMax number of iterations reached in ground point "
			<< "solution. Results are inaccurate." << endl;
	}
	worldPoint.hgt = oldHeight;
}

//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::imagingRay()
//  
//  Constructs an RPC ray by intersecting 2 ellipsoid heights above and
//  below the RPC height offset, and then forming a vector between the two.
//
//*****************************************************************************
void rspfRpcRcz2XyModel::imagingRay(const rspfDpt& imagePoint,
                               rspfEcefRay&   imageRay) const
{
   //---
   // For "from point", "to point" we want the image ray to be from above the
   // ellipsoid down to Earth.
   // 
   // It appears the ray "from point" must be above the ellipsiod for the
   // rspfElevSource::intersectRay method; ultimately, the
   // rspfEllipsoid::nearestIntersection method, else it goes off in the
   // weeds...
   //---
   //double vectorLength = theHgtScale ? (theHgtScale * 2.0) : 1000.0;
	double vectorLength = (theHgtScale < 300000.0) ? (theHgtScale * 2.0) : 1000.0;	// loong

   rspfGpt gpt;

   // "from" point
   double intHgt = theHgtOffset + vectorLength;
   lineSampleHeightToWorld(imagePoint, intHgt, gpt);
   //lineSampleHeightToWorld2(imagePoint, intHgt, gpt);
   gpt = m_proj->inverse_do(rspfDpt(gpt.lat, gpt.lon), gpt);
   rspfEcefPoint intECFfrom(gpt);
   
   // "to" point
   lineSampleHeightToWorld(imagePoint, theHgtOffset, gpt);
   //lineSampleHeightToWorld2(imagePoint, theHgtOffset, gpt);
   gpt = m_proj->inverse_do(rspfDpt(gpt.lat, gpt.lon), gpt);
   rspfEcefPoint intECFto(gpt);
   
   // Construct ray
   rspfEcefRay ray(intECFfrom, intECFto);
   
   imageRay = ray;

}


//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::lineSampleHeightToWorld()
//  
//  Performs reverse projection of image line/sample to ground point.
//  The imaging ray is intersected with a level plane at height = rspfElevManager::instance().
//
//  NOTE: U = line, V = sample -- this differs from the convention.
//
//*****************************************************************************
void rspfRpcRcz2XyModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                            const double&   ellHeight,
                                            rspfGpt&       gpt) const
{
	//***
	// Normalize the lat, lon, hgt:
	//***
	double nline = (image_point.line - theLineOffset) / theLineScale;
	double nsamp = (image_point.samp - theSampOffset) / theSampScale;
	double nhgt = (ellHeight - theHgtOffset) / theHgtScale;

	//***
	// Compute the adjusted, normalized line (U) and sample (V):
	//***
	double NX = polynomial(nline, nsamp, nhgt, theLineNumCoef);
	double DX = polynomial(nline, nsamp, nhgt, theLineDenCoef);
	double NY = polynomial(nline, nsamp, nhgt, theSampNumCoef);
	double DY = polynomial(nline, nsamp, nhgt, theSampDenCoef);

	double U_rot  = NX / DX;
	double V_rot  = NY / DY;

	//***
	// U, V are normalized quantities. Need now to establish the image file
	// line and sample. First, back out the adjustable parameter effects
	// starting with rotation:
	//***
	double U = U_rot*theCosMapRot + V_rot*theSinMapRot;
	double V = V_rot*theCosMapRot - U_rot*theSinMapRot;

	//***
	// Now back out skew, scale, and offset adjustments:
	//***
	gpt.lat = V*(theLatScale+theIntrackScale) + theLatOffset + theIntrackOffset;
	gpt.lon = U*(theLonScale+theIntrackScale) + theLonOffset + theIntrackOffset;

	gpt = m_proj->inverse_do(rspfDpt(gpt.lat, gpt.lon), gpt);
	gpt.hgt = ellHeight;

	// if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcRcz2XyModel::worldToLineSample(): returning..." << std::endl;
	return;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcRcz2XyModel::polynomial
//  
//  Computes polynomial.
//  
//*****************************************************************************
double rspfRpcRcz2XyModel::polynomial(const double& nline, const double& nsamp,
                                 const double& H, const double* c) const
{
   double r;

   if (thePolyType == A)
   {
	  r = c[ 0]       + c[ 1]*nsamp     + c[ 2]*nline     + c[ 3]*H     +
		  c[ 4]*nsamp*nline   + c[ 5]*nsamp*H   + c[ 6]*nline*H   + c[ 7]*nsamp*nline*H +
		  c[ 8]*nsamp*nsamp   + c[ 9]*nline*nline   + c[10]*H*H   + c[11]*nsamp*nsamp*nsamp +
		  c[12]*nsamp*nsamp*nline + c[13]*nsamp*nsamp*H + c[14]*nsamp*nline*nline + c[15]*nline*nline*nline +
		  c[16]*nline*nline*H + c[17]*nsamp*H*H + c[18]*nline*H*H + c[19]*H*H*H;
   }
   else
   {
      r = c[ 0]       + c[ 1]*nsamp     + c[ 2]*nline     + c[ 3]*H     +
          c[ 4]*nsamp*nline   + c[ 5]*nsamp*H   + c[ 6]*nline*H   + c[ 7]*nsamp*nsamp   +
          c[ 8]*nline*nline   + c[ 9]*H*H   + c[10]*nsamp*nline*H + c[11]*nsamp*nsamp*nsamp +
          c[12]*nsamp*nline*nline + c[13]*nsamp*H*H + c[14]*nsamp*nsamp*nline + c[15]*nline*nline*nline +
          c[16]*nline*H*H + c[17]*nsamp*nsamp*H + c[18]*nline*nline*H + c[19]*H*H*H;
   }
   
   return r;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcRcz2XyModel::dPoly_dLat
//  
//  Computes derivative of polynomial wrt normalized Latitude P.
//  
//*****************************************************************************
double rspfRpcRcz2XyModel::dPoly_dLine(const double& nline, const double& nsamp,
                                 const double& H, const double* c) const
{
   double dr;

   if (thePolyType == A)
   {
      dr = c[2] + c[4]*nsamp + c[6]*H + c[7]*nsamp*H + 2*c[9]*nline + c[12]*nsamp*nsamp +
           2*c[14]*nsamp*nline + 3*c[15]*nline*nline +2*c[16]*nline*H + c[18]*H*H;
   }
   else
   {
      dr = c[2] + c[4]*nsamp + c[6]*H + 2*c[8]*nline + c[10]*nsamp*H + 2*c[12]*nsamp*nline +
           c[14]*nsamp*nsamp + 3*c[15]*nline*nline + c[16]*H*H + 2*c[18]*nline*H;
   }
   
   return dr;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcRcz2XyModel::dPoly_dLon
//  
//  Computes derivative of polynomial wrt normalized Longitude L.
//  
//*****************************************************************************
double rspfRpcRcz2XyModel::dPoly_dSamp(const double& nline, const double& nsamp,
                                 const double& H, const double* c) const
{
   double dr;

   if (thePolyType == A)
   {
      dr = c[1] + c[4]*nline + c[5]*H + c[7]*nline*H + 2*c[8]*nsamp + 3*c[11]*nsamp*nsamp +
           2*c[12]*nsamp*nline + 2*c[13]*nsamp*H + c[14]*nline*nline + c[17]*H*H;
   }
   else
   {
      dr = c[1] + c[4]*nline + c[5]*H + 2*c[7]*nsamp + c[10]*nline*H + 3*c[11]*nsamp*nsamp +
           c[12]*nline*nline + c[13]*H*H + 2*c[14]*nline*nsamp + 2*c[17]*nsamp*H;
   }
   return dr;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcRcz2XyModel::dPoly_dHgt
//  
//  Computes derivative of polynomial wrt normalized Height H.
//  
//*****************************************************************************
double rspfRpcRcz2XyModel::dPoly_dHgt(const double& nline, const double& nsamp,
                                 const double& H, const double* c) const
{
   double dr;

   if (thePolyType == A)
   {
      dr = c[3] + c[5]*nsamp + c[6]*nline + c[7]*nsamp*nline + 2*c[10]*H + c[13]*nsamp*nsamp +
           c[16]*nline*nline + 2*c[17]*nsamp*H + 2*c[18]*nline*H + 3*c[19]*H*H;
   }
   else
   {
      dr = c[3] + c[5]*nsamp + c[6]*nline + 2*c[9]*H + c[10]*nsamp*nline + 2*c[13]*nsamp*H +
           2*c[16]*nline*H + c[17]*nsamp*nsamp + c[18]*nline*nline + 3*c[19]*H*H;
   }
   return dr;
}

void rspfRpcRcz2XyModel::updateModel()
{
   theIntrackOffset    = computeParameterOffset(INTRACK_OFFSET);
   theCrtrackOffset    = computeParameterOffset(CRTRACK_OFFSET);
   theIntrackScale     = computeParameterOffset(INTRACK_SCALE);
   theCrtrackScale     = computeParameterOffset(CRTRACK_SCALE);
   double mapRotation  = computeParameterOffset(MAP_ROTATION);
   theCosMapRot        = rspf::cosd(mapRotation);
   theSinMapRot        = rspf::sind(mapRotation);
}

void rspfRpcRcz2XyModel::initAdjustableParameters()
{
   resizeAdjustableParameterArray(NUM_ADJUSTABLE_PARAMS);
   int numParams = getNumberOfAdjustableParameters();
   for (int i=0; i<numParams; i++)
   {
      setAdjustableParameter(i, 0.0);
      setParameterDescription(i, PARAM_NAMES[i]);
      setParameterUnit(i,PARAM_UNITS[i]);
   }
   setParameterSigma(INTRACK_OFFSET, 50.0);
   setParameterSigma(CRTRACK_OFFSET, 50.0);
   setParameterSigma(INTRACK_SCALE, 50.0);  
   setParameterSigma(CRTRACK_SCALE, 50.0);  
   setParameterSigma(MAP_ROTATION, 0.1);
//   setParameterSigma(YAW_OFFSET, 0.001);
}

rspfObject* rspfRpcRcz2XyModel::dup() const
{
   return new rspfRpcRcz2XyModel(*this);
}

//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::print()
//  
//  Formatted dump of data members.
//  
//*****************************************************************************
std::ostream& rspfRpcRcz2XyModel::print(std::ostream& out) const
{
	out << "\nDump of rspfRpcModel object at " << hex << this << ":\n"
		<< POLY_TYPE_KW   << ": " << thePolyType   << "\n"
		<< LINE_SCALE_KW  << ": " << theLineScale  << "\n"
		<< SAMP_SCALE_KW  << ": " << theSampScale  << "\n"
		<< LAT_SCALE_KW   << ": " << theLatScale   << "\n"
		<< LON_SCALE_KW   << ": " << theLonScale   << "\n"
		<< HGT_SCALE_KW   << ": " << theHgtScale   << "\n"
		<< LINE_OFFSET_KW << ": " << theLineOffset << "\n"
		<< SAMP_OFFSET_KW << ": " << theSampOffset << "\n"
		<< LAT_OFFSET_KW  << ": " << theLatOffset  << "\n"
		<< LON_OFFSET_KW  << ": " << theLonOffset  << "\n"
		<< HGT_OFFSET_KW  << ": " << theHgtOffset  << "\n"
		<< BIAS_ERROR_KW  << ": " << theBiasError  << "\n"
		<< RAND_ERROR_KW  << ": " << theRandError  << "\n"
		<< endl;

	for (int i=0; i<NUM_COEFFS; i++)
		out<<"  "<<LINE_NUM_COEF_KW<<"["<<i<<"]: "<<theLineNumCoef[i]<<endl;

	out << endl;
	for (int i=0; i<NUM_COEFFS; i++)
		out<<"  "<<LINE_DEN_COEF_KW<<"["<<i<<"]: "<<theLineDenCoef[i]<<endl;

	out << endl;
	for (int i=0; i<NUM_COEFFS; i++)
		out<<"  "<<SAMP_NUM_COEF_KW<<"["<<i<<"]: "<<theSampNumCoef[i]<<endl;

	out << endl;
	for (int i=0; i<NUM_COEFFS; i++)
		out<<"  "<<SAMP_DEN_COEF_KW<<"["<<i<<"]: "<<theSampDenCoef[i]<<endl;

	out << endl;

	return rspfSensorModel::print(out);
}

//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::saveState()
//  
//  Saves the model state to the KWL. This KWL also serves as a geometry file.
//  
//*****************************************************************************
bool rspfRpcRcz2XyModel::saveState(rspfKeywordlist& kwl,
                              const char* prefix) const
{
	if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::saveState(): entering..." << std::endl;
	kwl.add(prefix, rspfKeywordNames::TYPE_KW, MODEL_TYPE);
	rspfSensorModel::saveState(kwl, prefix);
	kwl.add(prefix, POLY_TYPE_KW,   ((char)thePolyType));
	kwl.add(prefix, LINE_SCALE_KW,  theLineScale);
	kwl.add(prefix, SAMP_SCALE_KW,  theSampScale);
	kwl.add(prefix, LAT_SCALE_KW,   theLatScale);
	kwl.add(prefix, LON_SCALE_KW,   theLonScale);
	kwl.add(prefix, HGT_SCALE_KW,   theHgtScale);
	kwl.add(prefix, LINE_OFFSET_KW, theLineOffset);
	kwl.add(prefix, SAMP_OFFSET_KW, theSampOffset);
	kwl.add(prefix, LAT_OFFSET_KW,  theLatOffset);
	kwl.add(prefix, LON_OFFSET_KW,  theLonOffset);
	kwl.add(prefix, HGT_OFFSET_KW,  theHgtOffset);
	kwl.add(prefix, BIAS_ERROR_KW,  theBiasError);
	kwl.add(prefix, RAND_ERROR_KW,  theRandError);
	for (int i=0; i<NUM_COEFFS; i++)
	{
		rspfString key;
		std::ostringstream os;
		os << setw(2) << setfill('0') << right << i;

		key = LINE_NUM_COEF_KW;
		key += os.str();
		kwl.add(prefix, key.c_str(), theLineNumCoef[i],
			true, 15);

		key = LINE_DEN_COEF_KW;
		key += os.str();
		kwl.add(prefix, key.c_str(), theLineDenCoef[i],
			true, 15);
		key = SAMP_NUM_COEF_KW;
		key += os.str();
		kwl.add(prefix, key.c_str(), theSampNumCoef[i],
			true, 15);
		key = SAMP_DEN_COEF_KW;
		key += os.str();
		kwl.add(prefix, key.c_str(), theSampDenCoef[i],
			true, 15);
	}

	if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::saveState(): returning..." << std::endl;
	return true;
}

//*****************************************************************************
//  METHOD: rspfRpcRcz2XyModel::loadState()
//  
//  Restores the model's state from the KWL. This KWL also serves as a
//  geometry file.
//  
//*****************************************************************************
bool rspfRpcRcz2XyModel::loadState(const rspfKeywordlist& kwl,
                              const char* prefix) 
{
	if (traceExec())
	{
		rspfNotify(rspfNotifyLevel_DEBUG)
			<< "DEBUG rspfRpcModel::loadState(): entering..." << std::endl;
	}
	const char* value;
	const char* keyword;
	bool success = rspfSensorModel::loadState(kwl, prefix);
	if (!success)
	{
		theErrorStatus++;
		if (traceExec())
		{
			rspfNotify(rspfNotifyLevel_DEBUG)
				<< "DEBUG rspfRpcModel::loadState(): returning with error..."
				<< std::endl;
		}
		return false;
	}

	value = kwl.find(prefix, BIAS_ERROR_KW);
	if (value)
	{
		theBiasError = rspfString(value).toDouble();
	}
	value = kwl.find(prefix, RAND_ERROR_KW);
	if (value)
	{
		theRandError = rspfString(value).toDouble();
	}

	keyword = POLY_TYPE_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	thePolyType = (PolynomialType) value[0];

	keyword = LINE_SCALE_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theLineScale = atof(value);

	keyword = SAMP_SCALE_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theSampScale = atof(value);

	keyword = LAT_SCALE_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theLatScale = atof(value);

	keyword = LON_SCALE_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theLonScale = atof(value);

	keyword = HGT_SCALE_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theHgtScale = atof(value);

	keyword = LINE_OFFSET_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theLineOffset = atof(value);

	keyword = SAMP_OFFSET_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theSampOffset = atof(value);

	keyword = LAT_OFFSET_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theLatOffset = atof(value);

	keyword = LON_OFFSET_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theLonOffset = atof(value);

	keyword = HGT_OFFSET_KW;
	value = kwl.find(prefix, keyword);
	if (!value)
	{
		rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
			<< "<" << keyword << ">. Check the keywordlist for proper syntax."
			<< std::endl;
		return false;
	}
	theHgtOffset = atof(value);
	for (int i=0; i<NUM_COEFFS; i++)
	{
		rspfString keyword;
		ostringstream os;
		os << setw(2) << setfill('0') << right << i;
		keyword = LINE_NUM_COEF_KW;
		keyword += os.str();
		value = kwl.find(prefix, keyword.c_str());
		if (!value)
		{
			rspfNotify(rspfNotifyLevel_FATAL)
				<< "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
				<< "<" << keyword << ">. Check the keywordlist for proper syntax."
				<< std::endl;
			return false;
		}
		theLineNumCoef[i] = atof(value);
		keyword = LINE_DEN_COEF_KW;
		keyword += os.str();
		value = kwl.find(prefix, keyword.c_str());
		if (!value)
		{
			rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
				<< "<" << keyword << ">. Check the keywordlist for proper syntax."
				<< std::endl;
			return false;
		}
		theLineDenCoef[i] = atof(value);

		keyword = SAMP_NUM_COEF_KW;
		keyword += os.str();
		value = kwl.find(prefix, keyword.c_str());
		if (!value)
		{
			rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
				<< "<" << keyword << ">. Check the keywordlist for proper syntax."
				<< std::endl;
			return false;
		}
		theSampNumCoef[i] = atof(value);
		keyword = SAMP_DEN_COEF_KW;
		keyword += os.str();
		value = kwl.find(prefix, keyword.c_str());
		if (!value)
		{
			rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcModel::loadState(): Error encountered parsing the following required keyword: "
				<< "<" << keyword << ">. Check the keywordlist for proper syntax."
				<< std::endl;
			return false;
		}
		theSampDenCoef[i] = atof(value);
	}

	theCosMapRot = 1.0;
	theSinMapRot = 0.0;
	updateModel();

	if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::loadState(): returning..." << std::endl;
	return true;
}

//*****************************************************************************
// STATIC METHOD: rspfRpcRcz2XyModel::writeGeomTemplate
//  
//  Writes a sample kwl to output stream.
//  
//*****************************************************************************
void rspfRpcRcz2XyModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::writeGeomTemplate(): entering..." << std::endl;

   os <<
      "//**************************************************************\n"
      "// Template for RPC model keywordlist\n"
      "//**************************************************************\n"
      << rspfKeywordNames::TYPE_KW << ": " << MODEL_TYPE << endl;

   rspfSensorModel::writeGeomTemplate(os);
   
   os << "//\n"
      << "// Derived-class rspfRpcModel Keywords:\n"
      << "//\n"
      << POLY_TYPE_KW << ": A|B\n"
      << "\n"
      << "// RPC data consists of coefficients and normalization \n"
      << "// parameters. The RPC keywords used here are compatible with \n"
      << "// keywords found in Ikonos \"rpc.txt\" files.\n"
      << "// First are the normalization parameters:\n"
      << LINE_OFFSET_KW << ": <float>\n"
      << SAMP_OFFSET_KW << ": <float>\n"
      << LAT_OFFSET_KW << ": <float>\n"
      << LON_OFFSET_KW << ": <float>\n"
      << HGT_OFFSET_KW << ": <float>\n"
      << LINE_SCALE_KW << ": <float>\n"
      << SAMP_SCALE_KW << ": <float>\n"
      << LAT_SCALE_KW << ": <float>\n"
      << LON_SCALE_KW << ": <float>\n"
      << HGT_SCALE_KW << ": <float>\n"
      << BIAS_ERROR_KW << ": <float>\n"
      << RAND_ERROR_KW << ": <float>\n"
      << "\n"
      << "// RPC Coefficients are specified with indexes. Coefficients \n "
      << "// are specified for the four polynomials: line numerator, line \n"
      << "// denominator, sample numerator, and sample denominator:" << endl;

   for (int i=1; i<=20; i++)
      os << LINE_NUM_COEF_KW << setw(2) << setfill('0') << right
         << i << ": <float>" << endl; 
   os << endl;
   for (int i=1; i<=20; i++)
      os << LINE_DEN_COEF_KW << setw(2) << setfill('0') << right
         << i << ": <float>" << endl; 
   os << endl;
   for (int i=1; i<=20; i++)
      os << SAMP_NUM_COEF_KW << setw(2) << setfill('0') << right
         << i << ": <float>" << endl; 
   os << endl;
   for (int i=1; i<=20; i++)
      os << SAMP_DEN_COEF_KW << setw(2) << setfill('0') << right
         << i << ": <float>" << endl; 
   os << "\n" <<endl;

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::writeGeomTemplate(): returning..." << std::endl;
   return;
}

bool rspfRpcRcz2XyModel::setupOptimizer(const rspfString& init_file)
{
   rspfKeywordlist kwl;

   if(kwl.addFile(rspfFilename(init_file)))
   {
      return loadState(kwl);
   }
   else
   {
      rspfRefPtr<rspfProjection> proj = rspfProjectionFactoryRegistry::instance()->createProjection(init_file);
      if(proj.valid())
      {
         kwl.clear();
         proj->saveState(kwl);
         
         return loadState(kwl);
      }
   }
   
   return false;
}

//*****************************************************************************
//  METHOD: rspfSarModel::getForwardDeriv()
//  
//  Compute partials of samp/line WRT to ground.
//  
//*****************************************************************************
rspfDpt rspfRpcRcz2XyModel::getForwardDeriv(int derivMode,
                                        const rspfGpt& pos,
                                        double h)
{
   // If derivMode (parmIdx) >= 0 call base class version
   // for "adjustable parameters"
   if (derivMode >= 0)
   {
      return rspfSensorModel::getForwardDeriv(derivMode, pos, h);
   }
   
   // Use alternative derivMode definitions
   else
   {
      rspfDpt returnData;

      //******************************************
      // OBS_INIT mode
      //    [1] 
      //    [2] 
      //  Note: In this mode, pos is used to pass
      //  in the (s,l) observations.
      //******************************************
      if (derivMode==OBS_INIT)
      {
         // Image coordinates
         rspfDpt obs;
         obs.samp = pos.latd();
         obs.line = pos.lond();
         theObs = obs;
      }

      //******************************************
      // EVALUATE mode
      //   [1] evaluate & save partials, residuals
      //   [2] return residuals
      //******************************************
      else if (derivMode==EVALUATE)
      {

      }

      //******************************************
      // P_WRT_X, P_WRT_Y, P_WRT_Z modes
      //   [1] 3 separate calls required
      //   [2] return 3 sets of partials
      //******************************************
      else if (derivMode==P_WRT_X)
      {
         returnData = theParWRTx;
      }

      else if (derivMode==P_WRT_Y)
      {
         returnData = theParWRTy;
      }

      else
      {
         returnData = theParWRTz;
      }

      return returnData;
   }
}

double rspfRpcRcz2XyModel::getBiasError() const
{
   return theBiasError;
}

double rspfRpcRcz2XyModel::getRandError() const
{
   return theRandError;
}


//*****************************************************************************
//  METHOD: rspfSarModel::getRpcParameters)
//  
//  Accessor for RPC parameter set.
//  
//*****************************************************************************
void rspfRpcRcz2XyModel::getRpcParameters(rspfRpcRcz2XyModel::rpcModelStruct& model) const
{
	model.lineScale  = theLineScale;
	model.sampScale  = theSampScale;
	model.latScale   = theLatScale;
	model.lonScale   = theLonScale;
	model.hgtScale   = theHgtScale;
	model.lineOffset = theLineOffset;
	model.sampOffset = theSampOffset;
	model.latOffset  = theLatOffset;
	model.lonOffset  = theLonOffset;
	model.hgtOffset  = theHgtOffset;

	for (int i=0; i<20; ++i)
	{
		model.lineNumCoef[i] = theLineNumCoef[i];
		model.lineDenCoef[i] = theLineDenCoef[i];
		model.sampNumCoef[i] = theSampNumCoef[i];
		model.sampDenCoef[i] = theSampDenCoef[i];
	}

	if (thePolyType == A)
	{
		model.type= 'A';
	}
	else
	{
		model.type= 'B';
	}
}


void rspfRpcRcz2XyModel::saveRpcModelStruct(fstream &fs)const
{
	fs<<"thePolyType	="<<thePolyType<<endl;
	fs<<"theIntrackOffset	="<<theIntrackOffset<<endl;
	fs<<"theCrtrackOffset	="<<theCrtrackOffset<<endl;
	fs<<"theIntrackScale	="<<theIntrackScale<<endl;
	fs<<"theCrtrackScale	="<<theCrtrackScale<<endl;
	fs<<"theCosMapRot	="<<theCosMapRot<<endl;
	fs<<"theSinMapRot	="<<theSinMapRot<<endl;
	fs<<"theBiasError	="<<theBiasError<<endl;
	fs<<"theRandError	="<<theRandError<<endl;

	fs<<endl;

	fs<<"theLineScale	="<<theLineScale<<endl;
	fs<<"theSampScale	="<<theSampScale<<endl;
	fs<<"theLatScale	="<<theLatScale<<endl;
	fs<<"theLonScale	="<<theLonScale<<endl;
	fs<<"theHgtScale	="<<theHgtScale<<endl;
	fs<<"theLineOffset	="<<theLineOffset<<endl;
	fs<<"theSampOffset	="<<theSampOffset<<endl;
	fs<<"theLatOffset	="<<theLatOffset<<endl;
	fs<<"theLonOffset	="<<theLonOffset<<endl;
	fs<<"theHgtOffset	="<<theHgtOffset<<endl;


	fs.setf(ios::fixed, ios::floatfield);
	fs.precision(6);
	//fs<<"theLineNumCoef	=";
	for(int i = 0;i < 20;i++)
	{
		fs<<theSampNumCoef[i]<<"\t";
		fs<<theSampDenCoef[i]<<"\t";
		fs<<theLineNumCoef[i]<<"\t";
		fs<<theLineDenCoef[i]<<"\n";
	}
	fs<<endl;

	//fs<<"theLineDenCoef	=";
	//for(int i = 0;i < 20;i++)
	//{
	//	fs<<theLineDenCoef[i]<<"\t";
	//}
	//fs<<endl;

	//fs<<"theSampNumCoef	=";
	//for(int i = 0;i < 20;i++)
	//{
	//	fs<<theSampNumCoef[i]<<"\t";
	//}
	//fs<<endl;

	//fs<<"theSampDenCoef	=";
	//for(int i = 0;i < 20;i++)
	//{
	//	fs<<theSampDenCoef[i]<<"\t";
	//}
	fs<<endl;
}