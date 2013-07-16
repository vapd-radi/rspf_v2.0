//*****************************************************************************
// FILE: rspfRpcXyz2RcModel.cpp
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfRpcXyz2RcModel.
//   This is a replacement model utilizing the Rational Polynomial Coefficients
//   (RPC), a.k.a. Rapid Positioning Capability, and Universal Sensor Model
//   (USM).
//
// LIMITATIONS: Does not support parameter adjustment (YET)
//
//*****************************************************************************
//  $Id: rspfRpcXyz2RcModel.cpp 11522 2007-08-07 21:57:59Z dburken $

#include <rspf/projection/rspfRpcXyz2RcModel.h>
#include <rspf/elevation/rspfElevManager.h>

RTTI_DEF1(rspfRpcXyz2RcModel, "rspfRpcXyz2RcModel", rspfSensorModel);

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
static const char*  MODEL_TYPE        = "rspfRpcXyz2RcModel";
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
rspfRpcXyz2RcModel::rspfRpcXyz2RcModel()
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
rspfRpcXyz2RcModel::rspfRpcXyz2RcModel(const rspfRpcXyz2RcModel& model)
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
//  DESTRUCTOR: ~rspfRpcXyz2RcModel()
//  
//*****************************************************************************
rspfRpcXyz2RcModel::~rspfRpcXyz2RcModel()
{
}

void rspfRpcXyz2RcModel::setAttributes(rspf_float64 sampleOffset,
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

void rspfRpcXyz2RcModel::setAttributes(rspfRpcXyz2RcModel::rpcModelStruct& model, bool computeGsdFlag)
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

void rspfRpcXyz2RcModel::setMetersPerPixel(const rspfDpt& metersPerPixel)
{
   theGSD = metersPerPixel;
   theMeanGSD = (theGSD.x+theGSD.y)*.5;
}

void rspfRpcXyz2RcModel::computeGsd()
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
      std::string e = "rspfRpcXyz2RcModel::computeGSD error centerGpt has nans!";
      throw rspfException(e);
   }

   // Get the ground point to the right of the reference point.
   rspfGpt rightGpt;
   lineSampleHeightToWorld(theRefImgPt + rspfDpt(1, 0),
                           theHgtOffset,
                           rightGpt);
   if (rightGpt.hasNans())
   {
      std::string e = "rspfRpcXyz2RcModel::computeGSD error rightGpt has nans!";
      throw rspfException(e);
   }

   // Get the ground point one up from the reference point.
   rspfGpt topGpt;
   lineSampleHeightToWorld(theRefImgPt + rspfDpt(0, -1),
                           theHgtOffset,
                           topGpt);
   if (topGpt.hasNans())
   {
      std::string e = "rspfRpcXyz2RcModel::computeGSD error topGpt has nans!";
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
         << "rspfRpcXyz2RcModel::computGsd DEBUG:"
         << "\ntheGSD: " << theGSD
         << "\ntheMeanGSD: " << theMeanGSD << std::endl;
   }
}

void rspfRpcXyz2RcModel::setPositionError(const rspf_float64& biasError,
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
//  METHOD: rspfRpcXyz2RcModel::worldToLineSample()
//  
//  Overrides base class implementation. Directly computes line-sample from
//  the polynomials.
//*****************************************************************************
void rspfRpcXyz2RcModel::worldToLineSample(const rspfGpt& ground_point,
                                      rspfDpt&       img_pt) const
{
   // if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcXyz2RcModel::worldToLineSample(): entering..." << std::endl;

   if(ground_point.isLatNan() || ground_point.isLonNan() )
   {
      img_pt.makeNan();
      return;
   }

   //***
   // First check if the world point is inside bounding rectangle:
   //***
//   rspfDpt wdp (ground_point);
//    if (!(theBoundGndPolygon.pointWithin(wdp)))
//    {
//      img_pt = extrapolate(ground_point);
//       if (traceExec())  CLOG << "returning..." << endl;
//       return;
//    }
         
   //***
   // Normalize the lat, lon, hgt:
   //***
   rspfDpt dpt = m_proj->forward(ground_point);
   double nlat = (dpt.x - theLatOffset) / theLatScale;
   double nlon = (dpt.y - theLonOffset) / theLonScale;
   double nhgt;

   if( ground_point.isHgtNan() )
   {
      nhgt = (theHgtScale - theHgtOffset) / theHgtScale;
   }
   else
   {
      nhgt = (ground_point.hgt - theHgtOffset) / theHgtScale;
   }

   //***
   // Compute the adjusted, normalized line (U) and sample (V):
   //***
   double Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
   double Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
   double Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
   double Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
   double U_rot  = Pu / Qu;
   double V_rot  = Pv / Qv;

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
   img_pt.line = U*(theLineScale+theIntrackScale) + theLineOffset + theIntrackOffset;
   
   img_pt.samp = V*(theSampScale+theCrtrackScale) + theSampOffset + theCrtrackOffset;

   // if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcXyz2RcModel::worldToLineSample(): returning..." << std::endl;
   return;
}

//*****************************************************************************
//  METHOD: rspfRpcXyz2RcModel::lineSampleToWorld()
//  
//  Overrides base class implementation. Performs DEM intersection.
//*****************************************************************************
void  rspfRpcXyz2RcModel::lineSampleToWorld(const rspfDpt& imagePoint,
                                       rspfGpt&       worldPoint) const
{
	//lineSampleHeightToWorld2(imagePoint, theHgtOffset, worldPoint);
	//return;
	double vectorLength = (theHgtScale < 300000.0) ? (theHgtScale * 2.0) : 1000.0;	// loong

	rspfGpt gpt;

	// "from" point
	double intHgt = theHgtOffset + vectorLength;
	lineSampleHeightToWorld(imagePoint, intHgt, gpt);
	//lineSampleHeightToWorld2(imagePoint, intHgt, gpt);
	worldPoint = m_proj->inverse_do(rspfDpt(gpt.lat, gpt.lon), gpt);
	worldPoint.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(worldPoint);
	return;

   if(!imagePoint.hasNans())
   {
      rspfEcefRay ray;
      imagingRay(imagePoint, ray);
	  if (m_proj) worldPoint.datum(m_proj->getDatum());
      rspfElevManager::instance()->intersectRay(ray, worldPoint);
   }
   else
   {
      worldPoint.makeNan();
   }
}

//*****************************************************************************
//  METHOD: rspfRpcXyz2RcModel::imagingRay()
//  
//  Constructs an RPC ray by intersecting 2 ellipsoid heights above and
//  below the RPC height offset, and then forming a vector between the two.
//
//*****************************************************************************
void rspfRpcXyz2RcModel::imagingRay(const rspfDpt& imagePoint,
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
//  METHOD: rspfRpcXyz2RcModel::lineSampleHeightToWorld()
//  
//  Performs reverse projection of image line/sample to ground point.
//  The imaging ray is intersected with a level plane at height = rspfElevManager::instance().
//
//  NOTE: U = line, V = sample -- this differs from the convention.
//
//*****************************************************************************
void rspfRpcXyz2RcModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                            const double&   ellHeight,
                                            rspfGpt&       gpt) const
{
	//lineSampleHeightToWorld_LM(image_point, ellHeight, gpt); return;
   // if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcXyz2RcModel::lineSampleHeightToWorld: entering..." << std::endl;

   //***
   // Extrapolate if point is outside image:
   //***
//    if (!insideImage(image_point))
//    {
//       gpt = extrapolate(image_point, ellHeight);
//       if (traceExec())  CLOG << "returning..." << endl;
//       return;
//    }

   //***
   // Constants for convergence tests:
   //***
   static const int    MAX_NUM_ITERATIONS  = 10;
   static const double CONVERGENCE_EPSILON = 0.1;  // pixels
   
   //***
   // The image point must be adjusted by the adjustable parameters as well
   // as the scale and offsets given as part of the RPC param normalization.
   //
   //      NOTE: U = line, V = sample
   //***
   double U    = (image_point.y-theLineOffset - theIntrackOffset) / (theLineScale+theIntrackScale);
   double V    = (image_point.x-theSampOffset - theCrtrackOffset) / (theSampScale+theCrtrackScale);

   //***
   // Rotate the normalized U, V by the map rotation error (adjustable param):
   //***
   double U_rot = theCosMapRot*U - theSinMapRot*V;
   double V_rot = theSinMapRot*U + theCosMapRot*V;
   U = U_rot; V = V_rot;


   // now apply adjust intrack and cross track
   //***
   // Initialize quantities to be used in the iteration for ground point:
   //***

   // loong 修改初始值 以减少不收敛的情况
   double nlat      = 0.0;  // normalized latitude
   double nlon      = 0.0;  // normalized longitude   
   double nhgt;

   if(rspf::isnan(ellHeight))
   {
     nhgt = (theHgtScale - theHgtOffset) / theHgtScale;  // norm height
   }
   else
   {
      nhgt = (ellHeight - theHgtOffset) / theHgtScale;  // norm height
   }
   
   double epsilonU = CONVERGENCE_EPSILON/(theLineScale+theIntrackScale);
   double epsilonV = CONVERGENCE_EPSILON/(theSampScale+theCrtrackScale);
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
   double Pu, Qu, Pv, Qv;
   double dPu_dLat, dQu_dLat, dPv_dLat, dQv_dLat;
   double dPu_dLon, dQu_dLon, dPv_dLon, dQv_dLon;
   double Uc, Vc;
   double deltaU, deltaV;
   double dU_dLat, dU_dLon, dV_dLat, dV_dLon, W;
   double deltaLat, deltaLon;
   
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
      Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
      Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
      Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
      Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
      Uc = Pu/Qu;
      Vc = Pv/Qv;
      
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
         dPu_dLat = dPoly_dLat(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dLat = dPoly_dLat(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dLat = dPoly_dLat(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dLat = dPoly_dLat(nlat, nlon, nhgt, theSampDenCoef);
         dPu_dLon = dPoly_dLon(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dLon = dPoly_dLon(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dLon = dPoly_dLon(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dLon = dPoly_dLon(nlat, nlon, nhgt, theSampDenCoef);
         
         //***
         // Analytically compute partials of quotients U and V wrt lat, lon: 
         //***
         dU_dLat = (Qu*dPu_dLat - Pu*dQu_dLat)/(Qu*Qu);
         dU_dLon = (Qu*dPu_dLon - Pu*dQu_dLon)/(Qu*Qu);
         dV_dLat = (Qv*dPv_dLat - Pv*dQv_dLat)/(Qv*Qv);
         dV_dLon = (Qv*dPv_dLon - Pv*dQv_dLon)/(Qv*Qv);
         
         W = dU_dLon*dV_dLat - dU_dLat*dV_dLon;
         
         //***
         // Now compute the corrections to normalized lat, lon:
         //***
         deltaLat = (dU_dLon*deltaV - dV_dLon*deltaU) / W;
         deltaLon = (dV_dLat*deltaU - dU_dLat*deltaV) / W;
         nlat += deltaLat;
         nlon += deltaLon;

		 // loong commented
		 rspfGpt temp;
		 temp = rspfGpt(nlat*theLatScale + theLatOffset, nlon*theLonScale + theLonOffset);
		 temp = m_proj->inverse_do(rspfDpt(temp.lat, temp.lon), temp);
		 nhgt = rspfElevManager::instance()->getHeightAboveEllipsoid(temp);
		 nhgt = (nhgt - theHgtOffset) / theHgtScale;
      }
      
      iteration++;
      
   } while (((fabs(deltaU)>epsilonU) || (fabs(deltaV)>epsilonV))
            && (iteration < MAX_NUM_ITERATIONS));
      
   //***
   // Test for exceeding allowed number of iterations. Flag error if so:
   //***
   if (iteration == MAX_NUM_ITERATIONS)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfRpcXyz2RcModel::lineSampleHeightToWorld: \nMax number of iterations reached in ground point "
                                         << "solution. Results are inaccurate." << endl;
   }

   //***
   // Now un-normalize the ground point lat, lon and establish return quantity:
   //***
   gpt.lat = nlat*theLatScale + theLatOffset;
   gpt.lon = nlon*theLonScale + theLonOffset;
   gpt.hgt = ellHeight;

   // loong commented
   //gpt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(gpt);
   
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcXyz2RcModel::polynomial
//  
//  Computes polynomial.
//  
//*****************************************************************************
double rspfRpcXyz2RcModel::polynomial(const double& P, const double& L,
                                 const double& H, const double* c) const
{
   double r;

   if (thePolyType == A)
   {
      r = c[ 0]       + c[ 1]*L     + c[ 2]*P     + c[ 3]*H     +
          c[ 4]*L*P   + c[ 5]*L*H   + c[ 6]*P*H   + c[ 7]*L*P*H +
          c[ 8]*L*L   + c[ 9]*P*P   + c[10]*H*H   + c[11]*L*L*L +
          c[12]*L*L*P + c[13]*L*L*H + c[14]*L*P*P + c[15]*P*P*P +
          c[16]*P*P*H + c[17]*L*H*H + c[18]*P*H*H + c[19]*H*H*H;
   }
   else
   {
      r = c[ 0]       + c[ 1]*L     + c[ 2]*P     + c[ 3]*H     +
          c[ 4]*L*P   + c[ 5]*L*H   + c[ 6]*P*H   + c[ 7]*L*L   +
          c[ 8]*P*P   + c[ 9]*H*H   + c[10]*L*P*H + c[11]*L*L*L +
          c[12]*L*P*P + c[13]*L*H*H + c[14]*L*L*P + c[15]*P*P*P +
          c[16]*P*H*H + c[17]*L*L*H + c[18]*P*P*H + c[19]*H*H*H;
   }
   
   return r;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcXyz2RcModel::dPoly_dLat
//  
//  Computes derivative of polynomial wrt normalized Latitude P.
//  
//*****************************************************************************
double rspfRpcXyz2RcModel::dPoly_dLat(const double& P, const double& L,
                                 const double& H, const double* c) const
{
   double dr;

   if (thePolyType == A)
   {
      dr = c[2] + c[4]*L + c[6]*H + c[7]*L*H + 2*c[9]*P + c[12]*L*L +
           2*c[14]*L*P + 3*c[15]*P*P +2*c[16]*P*H + c[18]*H*H;
   }
   else
   {
      dr = c[2] + c[4]*L + c[6]*H + 2*c[8]*P + c[10]*L*H + 2*c[12]*L*P +
           c[14]*L*L + 3*c[15]*P*P + c[16]*H*H + 2*c[18]*P*H;
   }
   
   return dr;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcXyz2RcModel::dPoly_dLon
//  
//  Computes derivative of polynomial wrt normalized Longitude L.
//  
//*****************************************************************************
double rspfRpcXyz2RcModel::dPoly_dLon(const double& P, const double& L,
                                 const double& H, const double* c) const
{
   double dr;

   if (thePolyType == A)
   {
      dr = c[1] + c[4]*P + c[5]*H + c[7]*P*H + 2*c[8]*L + 3*c[11]*L*L +
           2*c[12]*L*P + 2*c[13]*L*H + c[14]*P*P + c[17]*H*H;
   }
   else
   {
      dr = c[1] + c[4]*P + c[5]*H + 2*c[7]*L + c[10]*P*H + 3*c[11]*L*L +
           c[12]*P*P + c[13]*H*H + 2*c[14]*P*L + 2*c[17]*L*H;
   }
   return dr;
}

//*****************************************************************************
// PRIVATE METHOD: rspfRpcXyz2RcModel::dPoly_dHgt
//  
//  Computes derivative of polynomial wrt normalized Height H.
//  
//*****************************************************************************
double rspfRpcXyz2RcModel::dPoly_dHgt(const double& P, const double& L,
                                 const double& H, const double* c) const
{
   double dr;

   if (thePolyType == A)
   {
      dr = c[3] + c[5]*L + c[6]*P + c[7]*L*P + 2*c[10]*H + c[13]*L*L +
           c[16]*P*P + 2*c[17]*L*H + 2*c[18]*P*H + 3*c[19]*H*H;
   }
   else
   {
      dr = c[3] + c[5]*L + c[6]*P + 2*c[9]*H + c[10]*L*P + 2*c[13]*L*H +
           2*c[16]*P*H + c[17]*L*L + c[18]*P*P + 3*c[19]*H*H;
   }
   return dr;
}

void rspfRpcXyz2RcModel::updateModel()
{
   theIntrackOffset    = computeParameterOffset(INTRACK_OFFSET);
   theCrtrackOffset    = computeParameterOffset(CRTRACK_OFFSET);
   theIntrackScale     = computeParameterOffset(INTRACK_SCALE);
   theCrtrackScale     = computeParameterOffset(CRTRACK_SCALE);
   double mapRotation  = computeParameterOffset(MAP_ROTATION);
   theCosMapRot        = rspf::cosd(mapRotation);
   theSinMapRot        = rspf::sind(mapRotation);
}

void rspfRpcXyz2RcModel::initAdjustableParameters()
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

rspfObject* rspfRpcXyz2RcModel::dup() const
{
   return new rspfRpcXyz2RcModel(*this);
}

//*****************************************************************************
//  METHOD: rspfRpcXyz2RcModel::print()
//  
//  Formatted dump of data members.
//  
//*****************************************************************************
std::ostream& rspfRpcXyz2RcModel::print(std::ostream& out) const
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
//  METHOD: rspfRpcXyz2RcModel::saveState()
//  
//  Saves the model state to the KWL. This KWL also serves as a geometry file.
//  
//*****************************************************************************
bool rspfRpcXyz2RcModel::saveState(rspfKeywordlist& kwl,
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
//  METHOD: rspfRpcXyz2RcModel::loadState()
//  
//  Restores the model's state from the KWL. This KWL also serves as a
//  geometry file.
//  
//*****************************************************************************
bool rspfRpcXyz2RcModel::loadState(const rspfKeywordlist& kwl,
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
// STATIC METHOD: rspfRpcXyz2RcModel::writeGeomTemplate
//  
//  Writes a sample kwl to output stream.
//  
//*****************************************************************************
void rspfRpcXyz2RcModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcXyz2RcModel::writeGeomTemplate(): entering..." << std::endl;

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

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcXyz2RcModel::writeGeomTemplate(): returning..." << std::endl;
   return;
}

bool rspfRpcXyz2RcModel::setupOptimizer(const rspfString& init_file)
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

vector<rspfDpt> rspfRpcXyz2RcModel::getModelForwardDeriv(const rspfGpt& pos,
									  double h)
{
	//求有理函数模型个参数的偏微分

	// 按照以下顺序
	/*************************
	double theLineNumCoef[20];
	double theLineDenCoef[20];
	double theSampNumCoef[20];
	double theSampDenCoef[20];
	**************************/

	int np = getModelParamNum();
	std::vector<rspfDpt> imDerp(np);

	for(int parmIdx = 0;parmIdx < 80;parmIdx++)
	{
		if(0 <= parmIdx && 20 > parmIdx)
		{
			//theLineNumCoef
			double den = 0.5/h;
			rspfDpt res, res1, res2;
			double middle = theLineNumCoef[parmIdx];
			//set parm to high value
			theLineNumCoef[parmIdx] = middle + h;
			worldToLineSample(pos, res1);
			//set parm to low value and get difference
			theLineNumCoef[parmIdx] = middle - h;
			worldToLineSample(pos, res2);
			imDerp[parmIdx] = (res1 - res2)*den;
			theLineNumCoef[parmIdx] = middle;
		}
		else if(20 <= parmIdx && 40 > parmIdx)
		{
			//theLineDenCoef
			double den = 0.5/h;
			rspfDpt res, res1, res2;
			double middle = theLineDenCoef[parmIdx-20];
			//set parm to high value
			theLineDenCoef[parmIdx-20] = middle + h;
			worldToLineSample(pos, res1);
			//set parm to low value and get difference
			theLineDenCoef[parmIdx-20] = middle - h;
			worldToLineSample(pos, res2);
			imDerp[parmIdx] = (res1 - res2)*den;
			theLineDenCoef[parmIdx-20] = middle;
		}
		else if(40 <= parmIdx && 60 > parmIdx)
		{
			//theSampNumCoef
			double den = 0.5/h;
			rspfDpt res, res1, res2;
			double middle = theSampNumCoef[parmIdx-40];
			//set parm to high value
			theSampNumCoef[parmIdx-40] = middle + h;
			worldToLineSample(pos, res1);
			//set parm to low value and get difference
			theSampNumCoef[parmIdx-40] = middle - h;
			worldToLineSample(pos, res2);
			imDerp[parmIdx] = (res1 - res2)*den;
			theSampNumCoef[parmIdx-40] = middle;
		}
		else
		{
			//theSampDenCoef
			double den = 0.5/h;
			rspfDpt res, res1, res2;
			double middle = theSampDenCoef[parmIdx-60];
			//set parm to high value
			theSampDenCoef[parmIdx-60] = middle + h;
			worldToLineSample(pos, res1);
			//set parm to low value and get difference
			theSampDenCoef[parmIdx-60] = middle - h;
			worldToLineSample(pos, res2);
			imDerp[parmIdx] = (res1 - res2)*den;
			theSampDenCoef[parmIdx-60] = middle;
		}
	}
	return imDerp;
	//


	//// 有理函数模型的公式如下
	///************************************************************************
	//if (thePolyType == A)
	//{
	//	r = c[ 0]       + c[ 1]*L     + c[ 2]*P     + c[ 3]*H     +
	//		c[ 4]*L*P   + c[ 5]*L*H   + c[ 6]*P*H   + c[ 7]*L*P*H +
	//		c[ 8]*L*L   + c[ 9]*P*P   + c[10]*H*H   + c[11]*L*L*L +
	//		c[12]*L*L*P + c[13]*L*L*H + c[14]*L*P*P + c[15]*P*P*P +
	//		c[16]*P*P*H + c[17]*L*H*H + c[18]*P*H*H + c[19]*H*H*H;
	//}
	//else
	//{
	//	r = c[ 0]       + c[ 1]*L     + c[ 2]*P     + c[ 3]*H     +
	//		c[ 4]*L*P   + c[ 5]*L*H   + c[ 6]*P*H   + c[ 7]*L*L   +
	//		c[ 8]*P*P   + c[ 9]*H*H   + c[10]*L*P*H + c[11]*L*L*L +
	//		c[12]*L*P*P + c[13]*L*H*H + c[14]*L*L*P + c[15]*P*P*P +
	//		c[16]*P*H*H + c[17]*L*L*H + c[18]*P*P*H + c[19]*H*H*H;
	//}
	//************************************************************************/
	//
	////***
	//// Normalize the lat, lon, hgt:
	////***
	//double nlat = (pos.lat - theLatOffset) / theLatScale;
	//double nlon = (pos.lon - theLonOffset) / theLonScale;
	//double nhgt;

	//if( rspf::isnan(pos.hgt) )
	//{
	//nhgt = (theHgtScale - theHgtOffset) / theHgtScale;
	//}
	//else
	//{
	//nhgt = (pos.hgt - theHgtOffset) / theHgtScale;
	//}

	////***
	//// Compute the normalized line (Un) and sample (Vn):
	////***
	//double Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
	//double Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
	//double Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
	//double Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
	//double Un  = Pu / Qu;
	//double Vn  = Pv / Qv;

	////***
	//// Compute the actual line (U) and sample (V):
	////***
	//double U  = Un*theLineScale + theLineOffset;
	//double V  = Vn*theSampScale + theSampOffset;

	//vector<double> coeff;
	//if(thePolyType == A)
	//{
	//	double tmp[] = {	1.0,				nlat,			nlon,			nhgt, 
	//			nlat*nlon,		nlat*nhgt,		nlon*nhgt,		nlat*nlon*nhgt,
	//			nlat*nlat,		nlon*nlon,		nhgt*nhgt,		nlat*nlat*nlat,
	//			nlat*nlat*nlon, nlat*nlat*nhgt,	nlat*nlon*nlon,	nlon*nlon*nlon,
	//			nlon*nlon*nhgt, nlat*nhgt*nhgt,	nlon*nhgt*nhgt,	nhgt*nhgt*nhgt};
	//	std::copy(tmp, tmp+20, std::back_inserter(coeff));
	//}
	//else
	//{
	//	double tmp[] = {	1.0,				nlat,			nlon,			nhgt, 
	//		nlat*nlon,		nlat*nhgt,		nlon*nhgt,		nlat*nlat,
	//		nlon*nlon,		nhgt*nhgt,		nlat*nlon*nhgt,	nlat*nlat*nlat,
	//		nlat*nlon*nlon, nlat*nhgt*nhgt,	nlat*nlat*nlon,	nlon*nlon*nlon,
	//		nlon*nhgt*nhgt, nlat*nlat*nhgt,	nlon*nlon*nhgt,	nhgt*nhgt*nhgt};
	//	std::copy(tmp, tmp+20, std::back_inserter(coeff));
	//}

	//int np = getModelParamNum();
	//std::vector<rspfDpt> imDerp(np);
	//for(int parmIdx = 0;parmIdx < 80;parmIdx++)
	//{
	//	if(0 <= parmIdx && 20 > parmIdx)
	//	{
	//		//theLineNumCoef
	//		imDerp[parmIdx].line = coeff[parmIdx] / Qu;
	//		imDerp[parmIdx].samp = 0.0;
	//		imDerp[parmIdx].line *= theLineScale;
	//	}
	//	else if(20 <= parmIdx && 40 > parmIdx)
	//	{
	//		//theLineDenCoef
	//		imDerp[parmIdx].line = -coeff[parmIdx - 20] / (Qu * Qu);
	//		imDerp[parmIdx].samp = 0.0;
	//		imDerp[parmIdx].line *= theLineScale;
	//	}
	//	else if(40 <= parmIdx && 60 > parmIdx)
	//	{
	//		//theSampNumCoef
	//		imDerp[parmIdx].line = 0.0;
	//		imDerp[parmIdx].samp = coeff[parmIdx - 40] / Qv;
	//		imDerp[parmIdx].samp *= theSampScale;
	//	}
	//	else
	//	{
	//		//theSampDenCoef
	//		imDerp[parmIdx].line = 0.0;
	//		imDerp[parmIdx].samp = -coeff[parmIdx - 60] / (Qv * Qv);
	//		imDerp[parmIdx].samp *= theSampScale;
	//	}
	//}
	//return imDerp;
}

//*****************************************************************************
//  METHOD: rspfSarModel::getForwardDeriv()
//  
//  Compute partials of samp/line WRT to ground.
//  
//*****************************************************************************
rspfDpt rspfRpcXyz2RcModel::getForwardDeriv(int derivMode,
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
         //***
         // Normalize the lat, lon, hgt:
         //***
         double nlat = (pos.lat - theLatOffset) / theLatScale;
         double nlon = (pos.lon - theLonOffset) / theLonScale;
         double nhgt;

         if( rspf::isnan(pos.hgt) )
         {
            nhgt = (theHgtScale - theHgtOffset) / theHgtScale;
         }
         else
         {
            nhgt = (pos.hgt - theHgtOffset) / theHgtScale;
         }
         
         //***
         // Compute the normalized line (Un) and sample (Vn):
         //***
         double Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
         double Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
         double Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
         double Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
         double Un  = Pu / Qu;
         double Vn  = Pv / Qv;
         
         //***
         // Compute the actual line (U) and sample (V):
         //***
         double U  = Un*theLineScale + theLineOffset;
         double V  = Vn*theSampScale + theSampOffset;

         //***
         // Compute the partials of each polynomial wrt lat, lon, hgt
         //***
         double dPu_dLat, dQu_dLat, dPv_dLat, dQv_dLat;
         double dPu_dLon, dQu_dLon, dPv_dLon, dQv_dLon;
         double dPu_dHgt, dQu_dHgt, dPv_dHgt, dQv_dHgt;
         dPu_dLat = dPoly_dLat(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dLat = dPoly_dLat(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dLat = dPoly_dLat(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dLat = dPoly_dLat(nlat, nlon, nhgt, theSampDenCoef);
         dPu_dLon = dPoly_dLon(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dLon = dPoly_dLon(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dLon = dPoly_dLon(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dLon = dPoly_dLon(nlat, nlon, nhgt, theSampDenCoef);
         dPu_dHgt = dPoly_dHgt(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dHgt = dPoly_dHgt(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dHgt = dPoly_dHgt(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dHgt = dPoly_dHgt(nlat, nlon, nhgt, theSampDenCoef);
         
         //***
         // Compute partials of quotients U and V wrt lat, lon, hgt 
         //***
         double dU_dLat, dU_dLon, dU_dHgt, dV_dLat, dV_dLon, dV_dHgt;
         dU_dLat = (Qu*dPu_dLat - Pu*dQu_dLat)/(Qu*Qu);
         dU_dLon = (Qu*dPu_dLon - Pu*dQu_dLon)/(Qu*Qu);
         dU_dHgt = (Qu*dPu_dHgt - Pu*dQu_dHgt)/(Qu*Qu);
         dV_dLat = (Qv*dPv_dLat - Pv*dQv_dLat)/(Qv*Qv);
         dV_dLon = (Qv*dPv_dLon - Pv*dQv_dLon)/(Qv*Qv);
         dV_dHgt = (Qv*dPv_dHgt - Pv*dQv_dHgt)/(Qv*Qv);
         
         //***
         // Apply necessary scale factors 
         //***
        dU_dLat *= theLineScale/theLatScale;
        dU_dLon *= theLineScale/theLonScale;
        dU_dHgt *= theLineScale/theHgtScale;
        dV_dLat *= theSampScale/theLatScale;
        dV_dLon *= theSampScale/theLonScale;
        dV_dHgt *= theSampScale/theHgtScale;

        dU_dLat *= DEG_PER_RAD;
        dU_dLon *= DEG_PER_RAD;
        dV_dLat *= DEG_PER_RAD;
        dV_dLon *= DEG_PER_RAD;

         // Save the partials referenced to ECF
         rspfEcefPoint location(pos);
         NEWMAT::Matrix jMat(3,3);
         pos.datum()->ellipsoid()->jacobianWrtEcef(location, jMat);
         //  Line
         theParWRTx.u = dU_dLat*jMat(1,1)+dU_dLon*jMat(2,1)+dU_dHgt*jMat(3,1);
         theParWRTy.u = dU_dLat*jMat(1,2)+dU_dLon*jMat(2,2)+dU_dHgt*jMat(3,2);
         theParWRTz.u = dU_dLat*jMat(1,3)+dU_dLon*jMat(2,3)+dU_dHgt*jMat(3,3);
         //  Samp
         theParWRTx.v = dV_dLat*jMat(1,1)+dV_dLon*jMat(2,1)+dV_dHgt*jMat(3,1);
         theParWRTy.v = dV_dLat*jMat(1,2)+dV_dLon*jMat(2,2)+dV_dHgt*jMat(3,2);
         theParWRTz.v = dV_dLat*jMat(1,3)+dV_dLon*jMat(2,3)+dV_dHgt*jMat(3,3);

         // Residuals
         rspfDpt resid(theObs.samp-V, theObs.line-U);
         returnData = resid;
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

double rspfRpcXyz2RcModel::getBiasError() const
{
   return theBiasError;
}

double rspfRpcXyz2RcModel::getRandError() const
{
   return theRandError;
}


//*****************************************************************************
//  METHOD: rspfSarModel::getRpcParameters)
//  
//  Accessor for RPC parameter set.
//  
//*****************************************************************************
void rspfRpcXyz2RcModel::getRpcParameters(rspfRpcXyz2RcModel::rpcModelStruct& model) const
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

void rspfRpcXyz2RcModel::saveRpcModelStruct(fstream &fs)const
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
