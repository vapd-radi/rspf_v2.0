//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer (rspf port by D. Burken)
//
// Description:
//
// Contains definition of class rspfSpot5Model.
//
//*****************************************************************************
// $Id: rspfSpot5Model.cpp 19658 2011-05-26 13:16:06Z gpotts $

#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

#include <rspf/projection/rspfSpot5Model.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/support_data/rspfSpotDimapSupportData.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfLsrPoint.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfLsrRay.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF1(rspfSpot5Model, "rspfSpot5Model", rspfSensorModel);


//---
// Define Trace flags for use within this file:
//---
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfSpot5Model:exec");
static rspfTrace traceDebug ("rspfSpot5Model:debug");

static const rspf_int32 MODEL_VERSION_NUMBER = 1;

static const char* PARAM_NAMES[] = { "roll_offset",
                                     "pitch_offset",
                                     "yaw_offset",
                                     "roll_rate",
                                     "pitch_rate",
                                     "yaw_rate",
                                     "focal_length_offset" };

static const char* PARAM_UNITS[] = { "degrees",   // degrees
                                     "degrees",   // degrees
                                     "degrees",   // degrees
                                     "degrees",   // degrees/sec
                                     "degrees",   // degrees/sec
                                     "degrees",   // degrees/sec
                                     "unknown" }; // percent deviation from nominal

static const rspf_float64 SIGMA[] = { 0.0001,   // degrees
                                       0.0001,   // degrees
                                       0.0003,   // degrees
                                       0.00002,  // delta degrees
                                       0.00002,  // delta degrees
                                       0.00005,  // delta degrees
                                       0.0001 }; // percent

rspfSpot5Model::rspfSpot5Model()
   :
   rspfSensorModel      (),
   theSupportData        (NULL),
   theMetaDataFile       ("NOT ASSIGNED"),
   theIllumAzimuth       (0.0),
   theIllumElevation     (0.0),
   thePositionError      (0.0),
   theRefImagingTime     (0.0),
   theRefImagingTimeLine (0.0),
   theLineSamplingPeriod (0.0),
//   theSatToOrbRotation   (3, 3),
//   theOrbToEcfRotation   (3, 3),
   theRollOffset         (0.0),
   thePitchOffset        (0.0),
   theYawOffset          (0.0),
   theRollRate           (0.0),
   thePitchRate          (0.0),
   theYawRate            (0.0),
   theFocalLenOffset     (0.0)
{
   initAdjustableParameters();
}

rspfSpot5Model::rspfSpot5Model(rspfSpotDimapSupportData* sd)
   :
   rspfSensorModel      (),
   theSupportData        (sd),
   theMetaDataFile       ("NOT ASSIGNED"),
   theIllumAzimuth       (0.0),
   theIllumElevation     (0.0),
   thePositionError      (0.0),
   theRefImagingTime     (0.0),
   theRefImagingTimeLine (0.0),
   theLineSamplingPeriod (0.0),
//   theSatToOrbRotation   (3, 3),
//   theOrbToEcfRotation   (3, 3),
   theRollOffset         (0.0),
   thePitchOffset        (0.0),
   theYawOffset          (0.0),
   theRollRate           (0.0),
   thePitchRate          (0.0),
   theYawRate            (0.0),
   theFocalLenOffset     (0.0)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model(dimap_file) Constructor: entering..." << std::endl;

   //---
   // Instantiate the support data classes after establishing the filenames:
   //---
   loadSupportData();
   if (getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model(dimap_file) Constructor: returning with error..." << std::endl;
      return;
   }

   //---
   // initialize remaining data members:
   //---
   initAdjustableParameters();
   updateModel();

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model(dimap_file) Constructor: returning..." << std::endl;
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfSpot5Model()
//
//*****************************************************************************
rspfSpot5Model::~rspfSpot5Model()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG DESTRUCTOR: ~rspfSpot5Model(): entering..." << std::endl;

   theSupportData = 0;
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG DESTRUCTOR: ~rspfSpot5Model(): returning..." << std::endl;
}

rspfSpot5Model::rspfSpot5Model(const rspfSpot5Model& rhs)
   :rspfSensorModel(rhs)
{
   if(rhs.theSupportData.valid())
   {
      theSupportData = (rspfSpotDimapSupportData*)rhs.theSupportData->dup();
   }
   loadSupportData();
   updateModel();
}


void rspfSpot5Model::computeSatToOrbRotation(NEWMAT::Matrix& result, rspf_float64 t)const
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfSpot5Model::computeSatToOrbRotation(): entering..."
      << std::endl;
   }
   //---
   // Linearly interpolate attitudes angles:
   //---
   rspfDpt3d att;
   theSupportData->getAttitude(t, att);

   //---
   // Apply the attitude adjustable parameters:
   //---
   double dt = theRefImagingTime - t;
   att.x     += thePitchOffset + dt*thePitchRate;
   att.y     += theRollOffset  + dt*theRollRate;
   att.z     += theYawOffset   + dt*theYawRate;

   //---
   // Compute trig functions to populate rotation matrices: ANGLES IN RADIANS
   //---
   double cp = cos(att.x);
   double sp = sin(att.x);
   double cr = cos(att.y);
   double sr = sin(att.y);
   double cy = cos(att.z);
   double sy = sin(att.z);

   //---
   // Populate rotation matrix:
   //---
   result = NEWMAT::Matrix(3,3);
   result << (cr*cy) << (-cr*sy) << (-sr)
   << (cp*sy+sp*sr*cy) << (cp*cy-sp*sr*sy) << (sp*cr)
   << (-sp*sy+cp*sr*cy) << (-sp*cy-cp*sr*sy) <<  cp*cr;


   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::computeSatToOrbRotation(): returning..." << std::endl;
}

#if 0
//*****************************************************************************
//  METHOD
//*****************************************************************************
void rspfSpot5Model::computeSatToOrbRotation(rspf_float64 t)const
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfSpot5Model::computeSatToOrbRotation(): entering..."
         << std::endl;
   }

   //---
   // Linearly interpolate attitudes angles:
   //---
   rspfDpt3d att;
   theSupportData->getAttitude(t, att);

   //---
   // Apply the attitude adjustable parameters:
   //---
   double dt = theRefImagingTime - t;
   att.x     += thePitchOffset + dt*thePitchRate;
   att.y     += theRollOffset  + dt*theRollRate;
   att.z     += theYawOffset   + dt*theYawRate;

   //---
   // Compute trig functions to populate rotation matrices: ANGLES IN RADIANS
   //---
    double cp = cos(att.x);
    double sp = sin(att.x);
    double cr = cos(att.y);
    double sr = sin(att.y);
    double cy = cos(att.z);
    double sy = sin(att.z);

   //---
   // Populate rotation matrix:
   //---
    theSatToOrbRotation = NEWMAT::Matrix(3,3);
    theSatToOrbRotation << (cr*cy) << (-cr*sy) << (-sr)
                        << (cp*sy+sp*sr*cy) << (cp*cy-sp*sr*sy) << (sp*cr)
                        << (-sp*sy+cp*sr*cy) << (-sp*cy-cp*sr*sy) <<  cp*cr;


    if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::computeSatToOrbRotation(): returning..." << std::endl;
}
#endif
//*****************************************************************************
// PUBLIC METHOD: rspfSpot5Model::updateModel()
//
//  Updates the model parameters given the normalized adjustable parameter
//  array.
//
//*****************************************************************************
void rspfSpot5Model::updateModel()
{
   clearErrorStatus();

   try
   {
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::updateModel(): entering..." << std::endl;

      if(getNumberOfAdjustableParameters() < 1)
      {
         theRollOffset     = 0;
         thePitchOffset    = 0;
         theYawOffset      = 0;
         theRollRate       = 0;
         thePitchRate      = 0;
         theYawRate        = 0;
         theFocalLenOffset = 0;
      }
      else
      {
         theRollOffset     = computeParameterOffset(0);
         thePitchOffset    = computeParameterOffset(1);
         theYawOffset      = computeParameterOffset(2);
         theRollRate       = computeParameterOffset(3);
         thePitchRate      = computeParameterOffset(4);
         theYawRate        = computeParameterOffset(5);
         theFocalLenOffset = computeParameterOffset(6);
      }
      theSeedFunction = 0;
      rspfGpt ulg, urg, lrg, llg;
      lineSampleToWorld(theImageClipRect.ul(), ulg);
      lineSampleToWorld(theImageClipRect.ur(), urg);
      lineSampleToWorld(theImageClipRect.lr(), lrg);
      lineSampleToWorld(theImageClipRect.ll(), llg);
      theSeedFunction = new rspfBilinearProjection(theImageClipRect.ul(),
                                                    theImageClipRect.ur(),
                                                    theImageClipRect.lr(),
                                                    theImageClipRect.ll(),
                                                    ulg,
                                                    urg,
                                                    lrg,
                                                    llg);

      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::updateModel(): returning..." << std::endl;
   }
   catch(...)
   {
      setErrorStatus(rspfErrorCodes::RSPF_ERROR);
   }
}

void rspfSpot5Model::initAdjustableParameters()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::initAdjustableParameters(): entering..." << std::endl;

   //---
   // Allocate storage for adjustables and assign their names and units
   // strings.
   //---
   resizeAdjustableParameterArray(7);
   rspf_uint32 numParams = getNumberOfAdjustableParameters();

   //---
   // Initialize base-class adjustable parameter array:
   //---
   for (rspf_uint32 i=0; i<numParams; ++i)
   {
      setAdjustableParameter(i, 0.0);
      setParameterDescription(i, PARAM_NAMES[i]);
      setParameterUnit(i,PARAM_UNITS[i]);
      setParameterSigma(i, SIGMA[i]);
   }

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::initAdjustableParameters(): returning..." << std::endl;
}

void rspfSpot5Model::loadSupportData()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSpot5Model::loadSupportData(): entering..." << std::endl;

   //---
   // Check for good support data:
   //---
   if (!theSupportData)
   {
      setErrorStatus();
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfSpot5Model::loadSupportData(): Null SpotDimapSupportData pointer passed to"
                                          << " constructor! Aborting..." << std::endl;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::loadSupportData(): returning..." << std::endl;
      return;
   }

   if (theSupportData->getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      setErrorStatus();
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfSpot5Model::loadSupportData(): Bad SpotDimapSupportData detected. Aborting..."
                                          << std::endl;
      if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::loadSupportData(): returning..." << std::endl;
      return;
   }

   //---
   // Initialize some member variables from the support data:
   //---
   theSensorID     = theSupportData->getSensorID();
   theImageID      = theSupportData->getImageID();
   theMetaDataFile = theSupportData->getMetadataFile();

   // Center of frame, sub image if we have one.
   theSupportData->getRefGroundPoint(theRefGndPt);

   theSupportData->getSunAzimuth(theIllumAzimuth);
   theSupportData->getSunElevation(theIllumElevation);
   rspfDpt sz;
   theSupportData->getImageSize(sz);
   theImageSize = sz;
   theSupportData->getRefLineTime(theRefImagingTime);
   theSupportData->getRefLineTimeLine(theRefImagingTimeLine);

   theSupportData->getLineSamplingPeriod(theLineSamplingPeriod);
   theSupportData->getSubImageOffset(theSpotSubImageOffset);

   //---
   // We make this zero base as the base rspfSensorModel does not know about
   // any sub image we have.
   //---
   theSupportData->getImageRect(theImageClipRect);
   theSupportData->getRefImagePoint(theRefImgPt);

   rspfGpt p1;
   rspfGpt p2;
   rspfGpt p3;
   rspfGpt p4;


   // I need to find the nominal scale of the spot 5 dataset

   //---
   // Position error is a function of whether star tracker information was
   // available:
   //---
   if (theSupportData->isStarTrackerUsed())
   {
      thePositionError = 50.0;
   }
   else
   {
      thePositionError = 200.0; // meters
   }
   updateModel();
   lineSampleToWorld(theImageClipRect.ul(), p1);
   lineSampleToWorld(theImageClipRect.ur(), p2);
   lineSampleToWorld(theImageClipRect.lr(), p3);
   lineSampleToWorld(theImageClipRect.ll(), p4);

//    theSupportData->getUlCorner(p1);
//    theSupportData->getUrCorner(p2);
//    theSupportData->getLrCorner(p3);
//    theSupportData->getLlCorner(p4);

   rspfDpt v[4]; // temporarily holds vertices for ground polygon
   v[0] = p1;
   v[1] = p2;
   v[2] = p3;
   v[3] = p4;
   theBoundGndPolygon = rspfPolygon(4, v);


   rspfGpt cgpt, hgpt, vgpt;
   // rspfEcefPoint hVector, vVector;
   rspfDpt midpt = theImageClipRect.midPoint();

   lineSampleToWorld(midpt, cgpt);
   lineSampleToWorld(midpt + rspfDpt(1,0), hgpt);
   lineSampleToWorld(midpt + rspfDpt(0,1), vgpt);

   theGSD     = rspfDpt((rspfEcefPoint(cgpt) - rspfEcefPoint(hgpt)).magnitude(),
			 (rspfEcefPoint(cgpt) - rspfEcefPoint(vgpt)).magnitude());

   theMeanGSD = (theGSD.x+theGSD.y)/2.0;

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::loadSupportData(): returning..." << std::endl;
}

rspfObject* rspfSpot5Model::dup() const
{
   return new rspfSpot5Model(*this);
}

std::ostream& rspfSpot5Model::print(std::ostream& out) const
{
   // Capture stream flags since we are going to mess with them.
   std::ios_base::fmtflags f = out.flags();

   out << "\nDump of rspfSpot5Model at address " << (hex) << this
       << (dec)
       << "\n------------------------------------------------"
       << "\n  theImageID            = " << theImageID
       << "\n  theMetadataFile       = " << theMetaDataFile
       << "\n  theIllumAzimuth       = " << theIllumAzimuth
       << "\n  theIllumElevation     = " << theIllumElevation
       << "\n  thePositionError      = " << thePositionError
       << "\n  theImageSize          = " << theImageSize
       << "\n  theRefGndPt           = " << theRefGndPt
       << "\n  theRefImgPt           = " << theRefImgPt
       << "\n  theRefImagingTime     = " << theRefImagingTime
       << "\n  theRefImagingTimeLine = " << theRefImagingTimeLine
       << "\n  theLineSamplingPeriod = " << theLineSamplingPeriod
       << "\n  theRollOffset         = " << theRollOffset
       << "\n  thePitchOffset        = " << thePitchOffset
       << "\n  theYawOffset          = " << theYawOffset
       << "\n  theRollRate           = " << theRollRate
       << "\n  thePitchRate          = " << thePitchRate
       << "\n  theYawRate            = " << theYawRate
       << "\n  theFocalLenOffset     = " << theFocalLenOffset
       << "\n------------------------------------------------"
       << "\n  " << endl;

   // Set the flags back.
   out.flags(f);

   return rspfSensorModel::print(out);
}

bool rspfSpot5Model::saveState(rspfKeywordlist& kwl,
                          const char* prefix) const
{
  if(theSupportData.valid())
  {
     rspfString supportPrefix = rspfString(prefix) + "support_data.";
     theSupportData->saveState(kwl, supportPrefix);
  }
  else
  {
     return false;
  }

   return rspfSensorModel::saveState(kwl, prefix);
}

bool rspfSpot5Model::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   rspfString supportPrefix = rspfString(prefix) + "support_data.";

   if(!theSupportData)
   {
      theSupportData = new rspfSpotDimapSupportData;
   }

   if(theSupportData->loadState(kwl, supportPrefix))
   {
      if(!rspfSensorModel::loadState(kwl, prefix))
      {
         return false;
      }
   }
   else
   {
      return false;
   }

   loadSupportData();
   updateModel();

   return (getErrorStatus()==rspfErrorCodes::RSPF_OK);
}

void rspfSpot5Model::imagingRay(const rspfDpt& image_point,
                                 rspfEcefRay&   image_ray) const
{
   bool runtime_dbflag = 0;
   NEWMAT::Matrix satToOrbit;
   rspfDpt iPt = image_point;
   iPt.samp += theSpotSubImageOffset.samp;
   iPt.line += theSpotSubImageOffset.line;

   //
   // 1. Establish time of line imaging:
   //
   double t_line = theRefImagingTime +
                   theLineSamplingPeriod*(iPt.line - theRefImagingTimeLine);
   if (traceDebug() || runtime_dbflag)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG Spot5Model::imagingRay():------------ BEGIN DEBUG PASS ---------------" << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG Spot5Model::imagingRay(): t_line = " << t_line << std::endl;
   }

   //
   // 2. Interpolate ephemeris position and velocity (in ECF):
   //
   rspfEcefPoint  tempEcefPoint;
   rspfEcefPoint  P_ecf;
   theSupportData->getPositionEcf(t_line, P_ecf);
   theSupportData->getVelocityEcf(t_line, tempEcefPoint);
   rspfEcefVector V_ecf(tempEcefPoint.x(),
                         tempEcefPoint.y(),
                         tempEcefPoint.z());
   if (traceDebug() || runtime_dbflag)
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG:\n\tP_ecf = " << P_ecf
         << "\n\t V_ecf = " << V_ecf << std::endl;
   }

   //
   // 3. Establish the look direction in Vehicle LSR space (S_sat).
   //    ANGLES IN RADIANS
   //
    rspf_float64 Psi_x;
    theSupportData->getPixelLookAngleX(iPt.samp, Psi_x);
    rspf_float64 Psi_y;
    theSupportData->getPixelLookAngleY(iPt.samp, Psi_y);
    if (traceDebug() || runtime_dbflag)
    {
       rspfNotify(rspfNotifyLevel_DEBUG)
          << "DEBUG:\n\t Psi_x = " << Psi_x
          << "\n\t Psi_y = " << Psi_y << endl;
    }

    rspfColumnVector3d u_sat (-tan(Psi_y), tan(Psi_x), -(1.0 + theFocalLenOffset));
    if (traceDebug() || runtime_dbflag)
    {
       rspfNotify(rspfNotifyLevel_DEBUG)
          << "DEBUG \n\t u_sat = " << u_sat << endl;
    }

   //
   // 4. Transform vehicle LSR space look direction vector to orbital LSR space
   //    (S_orb):
   //
    computeSatToOrbRotation(satToOrbit, t_line);

    rspfColumnVector3d u_orb = (satToOrbit*u_sat).unit();
    if (traceDebug() || runtime_dbflag)
    {
       rspfNotify(rspfNotifyLevel_DEBUG)
          << "DEBUG:\n\t theSatToOrbRotation = " << satToOrbit
          << "\n\t u_orb = " << u_orb << endl;
    }

   //
   // 5. Transform orbital LSR space look direction vector to ECF.
   //
   //   a. S_orb space Z-axis (Z_orb) is || to the ECF radial vector (P_ecf),
   //   b. X_orb axis is computed as cross-product between velocity and radial,
   //   c. Y_orb completes the orthogonal S_orb coordinate system.
   //
    rspfColumnVector3d Z_orb (P_ecf.x(),
                               P_ecf.y(),
                               P_ecf.z());
    Z_orb = Z_orb.unit();

    rspfColumnVector3d X_orb = rspfColumnVector3d(V_ecf.x(),
                                                    V_ecf.y(),
                                                    V_ecf.z()).cross(Z_orb).unit();
    rspfColumnVector3d Y_orb = Z_orb.cross(X_orb);

    NEWMAT::Matrix orbToEcfRotation = NEWMAT::Matrix(3, 3);
    orbToEcfRotation << X_orb[0] << Y_orb[0] << Z_orb[0]
                        << X_orb[1] << Y_orb[1] << Z_orb[1]
                        << X_orb[2] << Y_orb[2] << Z_orb[2];


   rspfColumnVector3d u_ecf  = (orbToEcfRotation*u_orb);
    if (traceDebug() || runtime_dbflag)
    {
       rspfNotify(rspfNotifyLevel_DEBUG)
          << "DEBUG:\n\t orbToEcfRotation = " << orbToEcfRotation
          << "\n\t u_ecf = " << u_ecf << endl;
    }

   //
   // Establish the imaging ray given direction and origin:
   //
    image_ray = rspfEcefRay(P_ecf, rspfEcefVector(u_ecf[0], u_ecf[1], u_ecf[2]));

    if (traceExec())
    {
       rspfNotify(rspfNotifyLevel_DEBUG)
          << "DEBUG Spot5Model::imagingRay(): returning..." << std::endl;
    }
}

void rspfSpot5Model::lineSampleHeightToWorld(const rspfDpt& image_point,
                                              const rspf_float64& heightEllipsoid,
                                              rspfGpt& worldPoint) const
{
//   if (!insideImage(image_point))
   if ( !theImageClipRect.pointWithin(image_point, 1.0-FLT_EPSILON) )   
   {
      if(theSeedFunction.valid())
      {
         theSeedFunction->lineSampleToWorld(image_point, worldPoint);
      }
      else
      {
         worldPoint = extrapolate(image_point, heightEllipsoid);
      }
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::lineSampleHeightToWorld(): returning..." << std::endl;
      return;
   }
   //***
   // First establish imaging ray from image point:
   //***
   rspfEcefRay imaging_ray;
   imagingRay(image_point, imaging_ray);
   if(m_proj) {
	   rspfEcefPoint Pecf (imaging_ray.intersectAboveEarthEllipsoid(heightEllipsoid,m_proj->getDatum()));
	   worldPoint = rspfGpt(Pecf,m_proj->getDatum());
   }
   else
   {
	   rspfEcefPoint Pecf (imaging_ray.intersectAboveEarthEllipsoid(heightEllipsoid));
	   worldPoint = rspfGpt(Pecf);

   }
}

// rspfDpt rspfSpot5Model::extrapolate (const rspfGpt& gp) const
// {
//     rspfDpt temp;

//     temp.makeNan();

//     return temp;

//   rspfDpt tempGpt = gp;
//   rspfDpt dest;
//   theGroundToImageMap.map(tempGpt, dest);

//  return dest;

// }

// rspfGpt rspfSpot5Model::extrapolate (const rspfDpt& ip,
// 				       const double& height) const
// {
//   return rspfGpt(rspf::nan(), rspf::nan(), rspf::nan(), 0);

//    rspfDpt dest;

//    theImageToGroundMap.map(ip, dest);


//    return rspfGpt(dest.lat, dest.lon, rspf::nan(), origin().datum());
// }

bool
rspfSpot5Model::setupOptimizer(const rspfString& init_file)
{
   rspfFilename spot5Test = init_file;
   rspfFilename geomFile = init_file;
   geomFile = geomFile.setExtension("geom");
   bool tryKwl = false;

   if(!spot5Test.exists())
   {
      spot5Test = geomFile.path();
      spot5Test = spot5Test.dirCat(rspfFilename("METADATA.DIM"));
      if(spot5Test.exists() == false)
      {
         spot5Test = geomFile.path();
         spot5Test = spot5Test.dirCat(rspfFilename("metadata.dim"));
      }
   }
   if(spot5Test.exists())
   {
      rspfRefPtr<rspfSpotDimapSupportData> meta = new rspfSpotDimapSupportData;
      if(meta->loadXmlFile(spot5Test))
      {
         initFromMetadata(meta.get());
         if (getErrorStatus())
         {
            tryKwl = true;
            meta=0;
         }
         else
         {
            return true;
         }
      }
      else
      {
         meta=0;
         tryKwl = true;
      }
   }
   if(tryKwl)
   {
      rspfKeywordlist kwl;
      if(kwl.addFile(init_file.c_str()))
      {
         return loadState(kwl);
      }
   }
   return false;
}

bool
rspfSpot5Model::initFromMetadata(rspfSpotDimapSupportData* sd)
{
   // init parms
   theSupportData        = sd;
   theMetaDataFile       = "NOT ASSIGNED";
   theIllumAzimuth       = 0.0;
   theIllumElevation     = 0.0;
   thePositionError      = 0.0;
   theRefImagingTime     = 0.0;
   theLineSamplingPeriod = 0.0;
//   theSatToOrbRotation   = 0.0; //matrix
//   theOrbToEcfRotation   = 0.0; //matrix
   theRollOffset         = 0.0;
   thePitchOffset        = 0.0;
   theYawOffset          = 0.0;
   theRollRate           = 0.0;
   thePitchRate          = 0.0;
   theYawRate            = 0.0;
   theFocalLenOffset     = 0.0;

   //---
   // Instantiate the support data classes after establishing the filenames:
   //---
   loadSupportData();
   if (getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSpot5Model::initFromMetadata(dimap_file): returning with error..." << std::endl;
      return false;
   }

   //---
   // initialize remaining data members:
   //---
   initAdjustableParameters();
   updateModel();
   return true;
}
