//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  Alpha HSI Sensor Model
//
//*******************************************************************
//  $Id$
#include <rspf/projection/rspfAlphaSensorHSI.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/support_data/rspfAlphaSensorSupportData.h>

static rspfTrace traceExec ("rspfAlphaSensorHSI:exec");
static rspfTrace traceDebug("rspfAlphaSensorHSI:debug");

RTTI_DEF1(rspfAlphaSensorHSI, "rspfAlphaSensorHSI", rspfSensorModel);


rspfAlphaSensorHSI::rspfAlphaSensorHSI()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHSI::rspfAlphaSensorHSI DEBUG:" << std::endl;
   }
   initAdjustableParameters();
   theSensorID = "AlphaHSI";
}

rspfAlphaSensorHSI::rspfAlphaSensorHSI(const rspfAlphaSensorHSI& src)
   :
   rspfAlphaSensor(src)
{
}

rspfObject* rspfAlphaSensorHSI::dup()const
{
   return new rspfAlphaSensorHSI(*this);
}

void rspfAlphaSensorHSI::imagingRay(const rspfDpt& imagePoint,
                                     rspfEcefRay& imageRay) const
{
   rspf_float64 line = imagePoint.y;

   // Form camera frame LOS vector
   rspf_float64 scanAngle = getScanAngle(line);
   rspfColumnVector3d camLOS(imagePoint.x - theImageSize.x/2,
                              m_adjustedFocalLength * tan(scanAngle),
                              m_adjustedFocalLength);

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
         << "rspfAlphaSensorHSI::imagingRay DEBUG:\n"
         << "  imagePoint = " << imagePoint << "\n"
         << "  imageRay = " << imageRay << "\n"
         << "  camLOS     = " << camLOS << "\n"
         << "  platPos    = " << platPos << "\n"
         << std::endl;
   }

}

void rspfAlphaSensorHSI::worldToLineSample(const rspfGpt& world_point,
                                                  rspfDpt& image_point) const
{   
   // Initialize at middle
   rspf_float64 refL = theImageSize.y/2;
   rspf_float64 drefL = 5;
   int nIter = 0;
   rspfColumnVector3d camLOS;
   rspfColumnVector3d camLOS1;

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
         if (ll==0)
            camLOS1 = camLOS;

         // Set function value
         rspf_float64 cScanAngle = atan(camLOS[1]/camLOS[2]);
         rspf_float64 scanAngle = getScanAngle(refl[ll]);
         Fx[ll] = cScanAngle - scanAngle;
      }

      // Compute numeric 1st derivative & new estimate for reference line (refL)
      rspf_float64 dFx = (Fx[1]-Fx[0]) / drefL;
      refL -= Fx[0]/dFx;

      nIter++;
   }

   rspf_float64 samp = m_adjustedFocalLength*camLOS1[0]/camLOS1[2] + theImageSize.x/2;

   rspfDpt p(samp, refL);
    
   image_point = p;
}

rspf_float64 rspfAlphaSensorHSI::getScanAngle(const rspf_float64& line) const
{
   rspf_float64 scanAngle = evalPoly(m_scanPoly, line);

   return scanAngle;
}

void rspfAlphaSensorHSI::updateModel()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHSI::updateModel DEBUG:" << std::endl;
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
         << "rspfAlphaSensorHSI::updateModel complete..." << std::endl;
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

void rspfAlphaSensorHSI::initAdjustableParameters()
{
   if (traceExec())
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAlphaSensorHSI::initAdjustableParameters: returning..." << std::endl;

   rspfAlphaSensor::initAdjustableParameters();
}

bool rspfAlphaSensorHSI::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHSI::loadState DEBUG:" << std::endl;
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
         << "rspfAlphaSensorHSI::loadState complete..." << std::endl;
   }
   
   return true;
}

bool rspfAlphaSensorHSI::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfAlphaSensor::saveState(kwl, prefix);

   return true;
}

bool rspfAlphaSensorHSI::initialize( const rspfAlphaSensorSupportData& supData )
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
