//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  Alpha HRI Sensor Model
//
//*******************************************************************
//  $Id$
#include <rspf/projection/rspfAlphaSensorHRI.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/support_data/rspfAlphaSensorSupportData.h>

static rspfTrace traceExec ("rspfAlphaSensorHRI:exec");
static rspfTrace traceDebug("rspfAlphaSensorHRI:debug");

RTTI_DEF1(rspfAlphaSensorHRI, "rspfAlphaSensorHRI", rspfSensorModel);


rspfAlphaSensorHRI::rspfAlphaSensorHRI()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHRI::rspfAlphaSensorHRI DEBUG:" << std::endl;
   }
   initAdjustableParameters();
   theSensorID = "AlphaHRI";
}

rspfAlphaSensorHRI::rspfAlphaSensorHRI(const rspfAlphaSensorHRI& src)
   :
   rspfAlphaSensor(src)
{
}

rspfObject* rspfAlphaSensorHRI::dup()const
{
   return new rspfAlphaSensorHRI(*this);
}

void rspfAlphaSensorHRI::imagingRay(const rspfDpt& imagePoint,
                                     rspfEcefRay& imageRay) const
{
   rspf_float64 line = imagePoint.y;

   rspf_float64 samp = imagePoint.x - theImageSize.x/2;

   // Flip x because raw image is mirrored in sample direction
   samp = -samp;


   // Form camera frame LOS vector
   rspfColumnVector3d camLOS(0.0, samp, -m_adjustedFocalLength);

   // Compute camera position & orientation matrix
   rspfEcefPoint platPos;
   NEWMAT::Matrix cam2EcfRot;
   getPositionOrientation(line, platPos, cam2EcfRot);

   // Rotate camera vector to ECF
   rspfColumnVector3d ecfLOS = cam2EcfRot * camLOS.unit();

   // Construct ECF image ray
   imageRay.setOrigin(platPos);
   rspfEcefVector ecfRayDir(ecfLOS);
   imageRay.setDirection(ecfRayDir);


   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHRI::imagingRay DEBUG:\n"
         << "  imagePoint = " << imagePoint << "\n"
         << "  imageRay = " << imageRay << "\n"
         << "  camLOS     = " << camLOS << "\n"
         << "  platPos    = " << platPos << "\n"
         << std::endl;
   }

}

void rspfAlphaSensorHRI::worldToLineSample(const rspfGpt& world_point,
                                                  rspfDpt& image_point) const
{   
   // Initialize at middle
   rspf_float64 refL = theImageSize.y/2;
   rspf_float64 drefL = 5;
   int nIter = 0;
   rspfColumnVector3d camLOS;

   // Iterate using Newton's method
   while (nIter<3)
   {
      rspf_float64 Fx[2];
      rspf_float64 refl[2];
      refl[0] = refL;
      refl[1] = refL + drefL;

      for (int ll=0; ll<2; ++ll)
      {
         // Compute camera position & orientation matrix
         rspfEcefPoint platPos;
         NEWMAT::Matrix cam2EcfRot;
         getPositionOrientation(refl[ll], platPos, cam2EcfRot);

         // Compute ECF vector
         rspfEcefPoint worldPointECF = rspfEcefPoint(world_point);
         rspfColumnVector3d ecfLOS = worldPointECF.data() - platPos.data();

         // Rotate to camera frame
         camLOS = cam2EcfRot.t() * ecfLOS;

         // Set function value
         Fx[ll] = camLOS[0];
      }

      // Compute numeric 1st derivative & new estimate for reference line (refL)
      rspf_float64 dFx = (Fx[1]-Fx[0]) / drefL;
      refL -= Fx[0]/dFx;

      nIter++;
   }

   rspf_float64 samp = -m_adjustedFocalLength*camLOS[1]/camLOS[2] + theImageSize.x/2;

   // Flip x because raw image is mirrored in sample direction
   samp = theImageSize.x - samp;

   rspfDpt p(samp, refL);
    
   image_point = p;
}

void rspfAlphaSensorHRI::updateModel()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHRI::updateModel DEBUG:" << std::endl;
   }
   rspfAlphaSensor::updateModel();
     
   try
   {
      computeGsd();
   }
   catch(...)
   {
      
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHRI::updateModel complete..." << std::endl;
   }

   // Ref point
   lineSampleToWorld(theRefImgPt, theRefGndPt);


   // Bounding rectangle
   rspfGpt gpt;
   theBoundGndPolygon.clear();

   lineSampleToWorld(theImageClipRect.ul(),gpt); //+rspfDpt(-w, -h), gpt);
   theBoundGndPolygon.addPoint(gpt.lond(), gpt.latd());

   lineSampleToWorld(theImageClipRect.ur(),gpt); //+rspfDpt(w, -h), gpt);
   theBoundGndPolygon.addPoint(gpt.lond(), gpt.latd());

   lineSampleToWorld(theImageClipRect.lr(),gpt); //+rspfDpt(w, h), gpt);
   theBoundGndPolygon.addPoint(gpt.lond(), gpt.latd());

   lineSampleToWorld(theImageClipRect.ll(),gpt); //+rspfDpt(-w, h), gpt);
   theBoundGndPolygon.addPoint(gpt.lond(), gpt.latd());
}

void rspfAlphaSensorHRI::initAdjustableParameters()
{
   if (traceExec())
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAlphaSensorHRI::initAdjustableParameters: returning..." << std::endl;

   rspfAlphaSensor::initAdjustableParameters();
}

bool rspfAlphaSensorHRI::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHRI::loadState DEBUG:" << std::endl;
   }

   rspfAlphaSensor::loadState(kwl, prefix);
   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }

   updateModel();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHRI::loadState complete..." << std::endl;
   }
   
   return true;
}

bool rspfAlphaSensorHRI::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfAlphaSensor::saveState(kwl, prefix);

   return true;
}

bool rspfAlphaSensorHRI::initialize( const rspfAlphaSensorSupportData& supData )
{
   bool result = true; // Currently no error checking.

   rspfDpt imageSize = supData.getImageSize();
   setImageSize(imageSize);
   setImageRect(rspfDrect(0,0,imageSize.x-1, imageSize.y-1));
   setRefImgPt(rspfDpt(imageSize.x*.5, imageSize.y*.5));
   
   setFov(supData.getFov());
   setRollBias(supData.getRollBias());
   setPitchBias(supData.getPitchBias());
   setHeadingBias(supData.getHeadingBias());
   setSlitRot(supData.getSlitRot());
   
   setRollPoly(supData.getRollPoly());
   setPitchPoly(supData.getPitchPoly());
   setHeadingPoly(supData.getHeadingPoly());
   setLonPoly(supData.getLonPoly());
   setLatPoly(supData.getLatPoly());
   setAltPoly(supData.getAltPoly());
   setScanPoly(supData.getScanPoly());
   
   updateModel();

   return result;
}
