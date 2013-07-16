//*****************************************************************************
// FILE: rspfRpcModel.cpp
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfRpcModel.
//   This is a replacement model utilizing the Rational Polynomial Coefficients
//   (RPC), a.k.a. Rapid Positioning Capability, and Universal Sensor Model
//   (USM).
//
// LIMITATIONS: Does not support parameter adjustment (YET)
//
//*****************************************************************************
//  $Id: rspfRpcModel.cpp 20600 2012-02-20 15:03:14Z gpotts $

#include <rspf/projection/rspfRpcModel.h>
#include <rspf/elevation/rspfElevManager.h>
RTTI_DEF1(rspfRpcModel, "rspfRpcModel", rspfSensorModel);
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

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>

#include <rspf/base/rspfTieGptSet.h>
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
rspfRpcModel::rspfRpcModel()
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
rspfRpcModel::rspfRpcModel(const rspfRpcModel& model)
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
rspfRpcModel::~rspfRpcModel()
{
}
void rspfRpcModel::setAttributes(rspf_float64 sampleOffset,
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
               << "rspfRpcModel::setAttributes Caught Exception:\n"
               << e.what() << std::endl;
         }
      }
   }
}
void rspfRpcModel::setMetersPerPixel(const rspfDpt& metersPerPixel)
{
   theGSD = metersPerPixel;
   theMeanGSD = (theGSD.x+theGSD.y)*.5;
}
void rspfRpcModel::setPositionError(const rspf_float64& biasError,
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
//  METHOD: rspfRpcModel::worldToLineSample()
//  
//  Overrides base class implementation. Directly computes line-sample from
//  the polynomials.
//*****************************************************************************
void rspfRpcModel::worldToLineSample(const rspfGpt& ground_point,
                                      rspfDpt&       img_pt) const
{
   // if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::worldToLineSample(): entering..." << std::endl;
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
   double nlat = (ground_point.lat - theLatOffset) / theLatScale;
   double nlon = (ground_point.lon - theLonOffset) / theLonScale;
   double nhgt;
   if( ground_point.isHgtNan() )
   {
      // nhgt = (theHgtScale - theHgtOffset) / theHgtScale;
      nhgt = ( - theHgtOffset) / theHgtScale;
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

   // if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::worldToLineSample(): returning..." << std::endl;
   return;
}

//*****************************************************************************
//  METHOD: rspfRpcModel::lineSampleToWorld()
//  
//  Overrides base class implementation. Performs DEM intersection.
//*****************************************************************************
void  rspfRpcModel::lineSampleToWorld(const rspfDpt& imagePoint,
                                       rspfGpt&       worldPoint) const
{

	//lineSampleHeightToWorld(imagePoint, theHgtOffset, worldPoint);
	//worldPoint.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(worldPoint);
	//if(!worldPoint.hasNans()) return;

   if(!imagePoint.hasNans())
   {
      rspfEcefRay ray;
	  imagingRay(imagePoint, ray);
	  if (m_proj) worldPoint.datum(m_proj->getDatum());	//loong
      rspfElevManager::instance()->intersectRay(ray, worldPoint);
   }
   else
   {
      worldPoint.makeNan();
   }
}

//*****************************************************************************
//  METHOD: rspfRpcModel::imagingRay()
//  
//  Constructs an RPC ray by intersecting 2 ellipsoid heights above and
//  below the RPC height offset, and then forming a vector between the two.
//
//*****************************************************************************
void rspfRpcModel::imagingRay(const rspfDpt& imagePoint,
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
   double vectorLength = theHgtScale ? (theHgtScale * 2.0) : 1000.0;

   rspfGpt gpt;
   
   // "from" point
   double intHgt = theHgtOffset + vectorLength;
   lineSampleHeightToWorld(imagePoint, intHgt, gpt);
   rspfEcefPoint intECFfrom(gpt);
   
   // "to" point
   lineSampleHeightToWorld(imagePoint, theHgtOffset, gpt);
   rspfEcefPoint intECFto(gpt);
   
   // Construct ray
   rspfEcefRay ray(intECFfrom, intECFto);
   
   imageRay = ray;
}


//*****************************************************************************
//  METHOD: rspfRpcModel::lineSampleHeightToWorld()
//  
//  Performs reverse projection of image line/sample to ground point.
//  The imaging ray is intersected with a level plane at height = elev.
//
//  NOTE: U = line, V = sample -- this differs from the convention.
//
//*****************************************************************************
void rspfRpcModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                            const double&   ellHeight,
                                            rspfGpt&       gpt) const
{
   // if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::lineSampleHeightToWorld: entering..." << std::endl;

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

	   //// loong commented
	   //rspfGpt temp;
	   //temp = rspfGpt(nlat*theLatScale + theLatOffset, nlon*theLonScale + theLonOffset);
	   //nhgt = rspfElevManager::instance()->getHeightAboveEllipsoid(temp);
	   //nhgt = (nhgt - theHgtOffset) / theHgtScale;

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
		}

		iteration++;
      
   } while (((fabs(deltaU)>epsilonU) || (fabs(deltaV)>epsilonV))
            && (iteration < MAX_NUM_ITERATIONS));
      
   //***
   // Test for exceeding allowed number of iterations. Flag error if so:
   //***
   if (iteration == MAX_NUM_ITERATIONS)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfRpcModel::lineSampleHeightToWorld: \nMax number of iterations reached in ground point "
                                         << "solution. Results are inaccurate." << endl;
   }

   //***
   // Now un-normalize the ground point lat, lon and establish return quantity:
   //***
	gpt.lat = nlat*theLatScale + theLatOffset;
	gpt.lon = nlon*theLonScale + theLonOffset;
	//gpt.hgt = nhgt * theHgtScale + theHgtOffset;
	gpt.hgt = ellHeight;
   
}
double rspfRpcModel::polynomial(const double& P, const double& L,
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
double rspfRpcModel::dPoly_dLat(const double& P, const double& L,
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
double rspfRpcModel::dPoly_dLon(const double& P, const double& L,
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
double rspfRpcModel::dPoly_dHgt(const double& P, const double& L,
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
void rspfRpcModel::updateModel()
{
   theIntrackOffset    = computeParameterOffset(INTRACK_OFFSET);
   theCrtrackOffset    = computeParameterOffset(CRTRACK_OFFSET);
   theIntrackScale     = computeParameterOffset(INTRACK_SCALE);
   theCrtrackScale     = computeParameterOffset(CRTRACK_SCALE);
   double mapRotation  = computeParameterOffset(MAP_ROTATION);
   theCosMapRot        = rspf::cosd(mapRotation);
   theSinMapRot        = rspf::sind(mapRotation);
}
void rspfRpcModel::initAdjustableParameters()
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
}
rspfObject* rspfRpcModel::dup() const
{
   return new rspfRpcModel(*this);
}
std::ostream& rspfRpcModel::print(std::ostream& out) const
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
bool rspfRpcModel::saveState(rspfKeywordlist& kwl,
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
bool rspfRpcModel::loadState(const rspfKeywordlist& kwl,
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
   
   // loong
   //prefix = "projection.";
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
void rspfRpcModel::writeGeomTemplate(ostream& os)
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
bool rspfRpcModel::setupOptimizer(const rspfString& init_file)
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
rspfDpt rspfRpcModel::getForwardDeriv(int derivMode,
                                        const rspfGpt& pos,
                                        double h)
{
   if (derivMode >= 0)
   {
      return rspfSensorModel::getForwardDeriv(derivMode, pos, h);
   }
   
   else
   {
      rspfDpt returnData;
      if (derivMode==OBS_INIT)
      {
         rspfDpt obs;
         obs.samp = pos.latd();
         obs.line = pos.lond();
         theObs = obs;
      }
      else if (derivMode==EVALUATE)
      {
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
         
         double Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
         double Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
         double Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
         double Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
         double Un  = Pu / Qu;
         double Vn  = Pv / Qv;
         
         double U  = Un*theLineScale + theLineOffset;
         double V  = Vn*theSampScale + theSampOffset;
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
         
         double dU_dLat, dU_dLon, dU_dHgt, dV_dLat, dV_dLon, dV_dHgt;
         dU_dLat = (Qu*dPu_dLat - Pu*dQu_dLat)/(Qu*Qu);
         dU_dLon = (Qu*dPu_dLon - Pu*dQu_dLon)/(Qu*Qu);
         dU_dHgt = (Qu*dPu_dHgt - Pu*dQu_dHgt)/(Qu*Qu);
         dV_dLat = (Qv*dPv_dLat - Pv*dQv_dLat)/(Qv*Qv);
         dV_dLon = (Qv*dPv_dLon - Pv*dQv_dLon)/(Qv*Qv);
         dV_dHgt = (Qv*dPv_dHgt - Pv*dQv_dHgt)/(Qv*Qv);
         
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
         rspfEcefPoint location(pos);
         NEWMAT::Matrix jMat(3,3);
         pos.datum()->ellipsoid()->jacobianWrtEcef(location, jMat);
         theParWRTx.u = dU_dLat*jMat(1,1)+dU_dLon*jMat(2,1)+dU_dHgt*jMat(3,1);
         theParWRTy.u = dU_dLat*jMat(1,2)+dU_dLon*jMat(2,2)+dU_dHgt*jMat(3,2);
         theParWRTz.u = dU_dLat*jMat(1,3)+dU_dLon*jMat(2,3)+dU_dHgt*jMat(3,3);
         theParWRTx.v = dV_dLat*jMat(1,1)+dV_dLon*jMat(2,1)+dV_dHgt*jMat(3,1);
         theParWRTy.v = dV_dLat*jMat(1,2)+dV_dLon*jMat(2,2)+dV_dHgt*jMat(3,2);
         theParWRTz.v = dV_dLat*jMat(1,3)+dV_dLon*jMat(2,3)+dV_dHgt*jMat(3,3);
         rspfDpt resid(theObs.samp-V, theObs.line-U);
         returnData = resid;
      }
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
double rspfRpcModel::getBiasError() const
{
   return theBiasError;
}
double rspfRpcModel::getRandError() const
{
   return theRandError;
}
void rspfRpcModel::getRpcParameters(rspfRpcModel::rpcModelStruct& model) const
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

void rspfRpcModel::saveRpcModelStruct(fstream &fs)const
{
	fs<<"thePolyType	="<<char(thePolyType)<<endl;
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
		fs<<setw(10)<<theSampNumCoef[i]<<"\t";
		fs<<setw(10)<<theSampDenCoef[i]<<"\t";
		fs<<setw(10)<<theLineNumCoef[i]<<"\t";
		fs<<setw(10)<<theLineDenCoef[i]<<"\n";
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


void rspfRpcModel::saveRpcModelStruct(rspfRpcModel::rpcModelStruct& rpcStruct)const
{
	rpcStruct.type = thePolyType;
	rpcStruct.lineScale = theLineScale;
	rpcStruct.sampScale = theSampScale;
	rpcStruct.latScale = theLatScale;
	rpcStruct.lonScale = theLonScale;
	rpcStruct.hgtScale = theHgtScale;
	rpcStruct.lineOffset = theLineOffset;
	rpcStruct.sampOffset = theSampOffset;
	rpcStruct.latOffset = theLatOffset;
	rpcStruct.lonOffset = theLonOffset;
	rpcStruct.hgtOffset = theHgtOffset;
	int i;
	for(i = 0;i < 20;i++)
	{
		rpcStruct.sampNumCoef[i] = theSampNumCoef[i];
		rpcStruct.sampDenCoef[i] = theSampDenCoef[i];
		rpcStruct.lineNumCoef[i] = theLineNumCoef[i];
		rpcStruct.lineDenCoef[i] = theLineDenCoef[i];
	}
}

void rspfRpcModel::setAttributes(rspfRpcModel::rpcModelStruct& model, bool computeGsdFlag)
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

double rspfRpcModel::optimizeFit(const rspfTieGptSet& tieSet, double* targetVariance)
{
	int nGpt = static_cast<int>(tieSet.size());
	if(nGpt < 1)
	{
		return 0.0;
	}
	else if(nGpt < 2)
	{
		m_modelOptimizeType = OptImageTranslation;
	}
	else if(nGpt < 3)
	{
		//m_modelOptimizeType = OptImageTrans_scale;
		m_modelOptimizeType = OptImageTranslation;
	}
	else
	{
		m_modelOptimizeType = OptImageAffine;
	}
	return rspfSensorModel::optimizeFit(tieSet, targetVariance);
}

rspf_uint32 rspfRpcModel::getNumberOfAdjustableParameters()const
{
	if(OptNone == m_modelOptimizeType)
	{
		return 0;
	}
	if(OptImageTranslation == m_modelOptimizeType)
	{
		return 2;
	}
	else if(OptImageTrans_scale == m_modelOptimizeType)
	{
		return 4;
	}
	else if(OptImageAffine == m_modelOptimizeType)
	{
		return rspfAdjustableParameterInterface::getNumberOfAdjustableParameters();
	}
	else
	{
		return rspfAdjustableParameterInterface::getNumberOfAdjustableParameters();
	}
}
