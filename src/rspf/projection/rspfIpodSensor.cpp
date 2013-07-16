//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Sonoma
//*******************************************************************
//  $Id$
#include <rspf/projection/rspfIpodSensor.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfLsrRay.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfMatrix4x4.h>
#include <rspf/base/rspfCsvFile.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/matrix/newmatio.h>

static rspfTrace traceDebug("rspfIpodSensor:debug");

RTTI_DEF1(rspfIpodSensor, "rspfIpodSensor", rspfSensorModel);

rspfIpodSensor::rspfIpodSensor()
{
   initAdjustableParameters();
   theSensorID = "Ipod";
}

void rspfIpodSensor::imagingRay(const rspfDpt& image_point,
                                   rspfEcefRay&   image_ray) const
{

   rspfDpt f1 (image_point - theRefImgPt);
   f1.x *= m_pixelSize.x;
   f1.y *= -m_pixelSize.y; 
   rspfDpt film (f1 - m_principalPoint);
   
   // once we have them we can add distortion fixes 
   // here on the film
   //
   
   
   // now orient the local film point to the ECEF axes
   // in a north east down lsr space.
   //
   rspfColumnVector3d rc_body(film.y, film.x, m_focalLength + computeParameterOffset(6));  
   rspfColumnVector3d rc_ecef = m_ecef2NedInverse*m_air2Ned*rc_body;
   image_ray.setOrigin(m_adjustedPlatformPosition);
   rspfEcefVector     ecf_ray_dir (rc_ecef);
   image_ray.setDirection(ecf_ray_dir);

}

void rspfIpodSensor::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                     const double&   heightEllipsoid,
                                     rspfGpt&       worldPt) const
{
   if (!insideImage(lineSampPt))
   {
      worldPt.makeNan();
      //       worldPoint = extrapolate(image_point, heightEllipsoid);
   }
   else
   {
      //***
      // First establish imaging ray from image point:
      //***
      rspfEcefRay ray;
      imagingRay(lineSampPt, ray);
      rspfEcefPoint Pecf (ray.intersectAboveEarthEllipsoid(heightEllipsoid));
      worldPt = rspfGpt(Pecf);
   }
}


void rspfIpodSensor::updateModel()
{

   m_adjustedPlatformPosition = rspfGpt(m_platformPositionEllipsoid.latd()   + computeParameterOffset(1),
                                         m_platformPositionEllipsoid.lond()   + computeParameterOffset(0),
                                         m_platformPositionEllipsoid.height() + computeParameterOffset(5));
   
   double r = rspf::degreesToRadians(m_roll  + computeParameterOffset(2));
   double p = rspf::degreesToRadians(m_pitch + computeParameterOffset(3) );
   double y = rspf::degreesToRadians(m_yaw   + computeParameterOffset(4));
   NEWMAT::Matrix rollM   = rspfMatrix3x3::create(1, 0, 0,
                                                    0, cos(-r), sin(-r),
                                                    0, -sin(-r), cos(-r));
   NEWMAT::Matrix pitchM  = rspfMatrix3x3::create(cos(-p), 0, -sin(-p),
                                                    0,      1, 0,
                                                    sin(-p), 0, cos(-p));
   NEWMAT::Matrix yawM    = rspfMatrix3x3::create(cos(-y), sin(-y), 0,
                                                -sin(-y), cos(-y), 0,
                                                0,0,1); 


   

   NEWMAT::Matrix camAM   = rspfMatrix3x3::create(0,1,0,-1,0,0,0,0,1); //90 deg rotation @ 3- for camera orientation on copter

   NEWMAT::Matrix camM   = rspfMatrix3x3::create(-1,0,0,0,-1,0,0,0,1); //180 deg rotation @ 3- for corner alignment (mirroring)

   m_air2Ned = camM*rollM*pitchM*yawM*camAM;
   
   double sin_lat = rspf::sind(m_adjustedPlatformPosition.latd());
   double cos_lat = rspf::cosd(m_adjustedPlatformPosition.latd());
   double sin_lon = rspf::sind(m_adjustedPlatformPosition.lond());
   double cos_lon = rspf::cosd(m_adjustedPlatformPosition.lond());
   m_ecef2Ned = (rspfMatrix3x3::create(0,0,1,0,1,0,-1,0,0)*
                 rspfMatrix3x3::create(cos_lat, 0, sin_lat, 0,1,0, -sin_lat,0,cos_lat)*
                 rspfMatrix3x3::create(cos_lon,sin_lon,0, -sin_lon, cos_lon, 0, 0, 0, 1));
   m_ecef2NedInverse = m_ecef2Ned.t();
   double adjustedFocal = m_focalLength + computeParameterOffset(6);
   m_fovIntrack    = (atan((theImageSize.y*theRefImgPt.y)/adjustedFocal));
   m_fovCrossTrack = (atan((theImageSize.x*theRefImgPt.x)/adjustedFocal));
   
   
   lineSampleToWorld(theRefImgPt, theRefGndPt);
   
   theBoundGndPolygon.resize(4);
   rspf_float64 w = theImageClipRect.width()*2.0;
   rspf_float64 h = theImageClipRect.height()*2.0;
   rspfGpt gpt;
   
   lineSampleToWorld(theImageClipRect.ul()+rspfDpt(-w, -h), gpt);
   theBoundGndPolygon[0] = gpt;
   lineSampleToWorld(theImageClipRect.ur()+rspfDpt(w, -h), gpt);
   theBoundGndPolygon[1] = gpt;
   lineSampleToWorld(theImageClipRect.lr()+rspfDpt(w, h), gpt);
   theBoundGndPolygon[2] = gpt;
   lineSampleToWorld(theImageClipRect.ll()+rspfDpt(-w, h), gpt);
   theBoundGndPolygon[3] = gpt;
   try{
   	computeGsd();
   }
   catch(...)
   {
   }

}

void rspfIpodSensor::initAdjustableParameters()
{
   resizeAdjustableParameterArray(7);
   
   setAdjustableParameter(0, 0.0);
   setParameterDescription(0, "lon_offset");
   setParameterUnit(0, "degrees");
   
   setAdjustableParameter(1, 0.0);
   setParameterDescription(1, "lat_offset");
   setParameterUnit(1, "degrees");
   
   setAdjustableParameter(2, 0.0);
   setParameterDescription(2, "roll");
   setParameterUnit(2, "degrees");
   
   setAdjustableParameter(3, 0.0);
   setParameterDescription(3, "pitch");
   setParameterUnit(3, "degrees");
   
   setAdjustableParameter(4, 0.0);
   setParameterDescription(4, "yaw");
   setParameterUnit(4, "degrees");
   
   setAdjustableParameter(5, 0.0);
   setParameterDescription(5, "Altitude delta");
   setParameterUnit(5, "meters");
   
   setAdjustableParameter(6, 0.0);
   setParameterDescription(6, "focal length delta");
   setParameterUnit(6, "meters");
   
   setParameterSigma(0, .001);
   setParameterSigma(1, .001);
   setParameterSigma(2, 10);
   setParameterSigma(3, 10);
   setParameterSigma(4, 10);
   setParameterSigma(5, 100);
   setParameterSigma(6, .04);
   
}

bool rspfIpodSensor::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }
   theGSD.makeNan();
   theRefImgPt.makeNan();
   rspfSensorModel::loadState(kwl, prefix);
   rspfString image_size = kwl.find(prefix, "image_size");
   rspfString pixel_size = kwl.find(prefix, "pixel_size");
   rspfString principal_point = kwl.find(prefix, "principal_point");
   rspfString focal_length = kwl.find(prefix, "focal_length");
   rspfString roll;
   rspfString pitch;
   rspfString yaw;
   rspfString platform_position;
   m_roll    = 0;
   m_pitch   = 0;
   m_yaw     = 0;
   roll      = kwl.find(prefix, "roll"); 
   pitch     = kwl.find(prefix, "pitch"); 
   yaw       = kwl.find(prefix, "yaw"); 
   platform_position = kwl.find(prefix, "platform_position");
   bool result = (!pixel_size.empty()&&
                  !principal_point.empty()&&
                  !focal_length.empty()&&
                  !platform_position.empty());
   if(!image_size.empty())
   {
      theImageSize.toPoint(image_size);
      theRefImgPt = rspfDpt(theImageSize.x*.5, theImageSize.y*.5);
      theImageClipRect = rspfDrect(0,0,theImageSize.x-1, theImageSize.y-1);
   }
   if(theImageClipRect.hasNans())
   {
      theImageClipRect = rspfDrect(0,0,theImageSize.x-1,theImageSize.y-1);
   }
   if(theRefImgPt.hasNans())
   {
      theRefImgPt = theImageClipRect.midPoint();
   }
   if(!focal_length.empty())
   {
      m_focalLength = focal_length.toDouble();
   }
   if(!pixel_size.empty())
   {
      m_pixelSize.toPoint(pixel_size);
   }
   if(!roll.empty())
   {
      m_roll = roll.toDouble();
   }
   if(!pitch.empty())
   {
      m_pitch = pitch.toDouble();
   }
   if(!yaw.empty())
   {
      m_yaw   = yaw.toDouble();
   }
   if(!principal_point.empty())
   {
      m_principalPoint.toPoint(principal_point);
   }
   if(!platform_position.empty())
   {
      m_platformPosition.toPoint(platform_position);
      m_platformPositionEllipsoid = m_platformPosition;
      double offset = rspfGeoidManager::instance()->offsetFromEllipsoid(m_platformPosition);
      if(!rspf::isnan(offset))
      {
         m_platformPositionEllipsoid.height(m_platformPosition.height() + offset);
      }
   }
   updateModel();
   
   if(theGSD.isNan())
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
            << "rspfIpodSensor::loadState Caught Exception:\n"
            << e.what() << std::endl;
         }
      }
   }
   
   return result;
}

bool rspfIpodSensor::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfSensorModel::saveState(kwl, prefix);
   kwl.add(prefix, "roll", rspfString::toString(m_roll), true);
   kwl.add(prefix, "pitch", rspfString::toString(m_pitch), true);
   kwl.add(prefix, "yaw", rspfString::toString(m_yaw), true);
   kwl.add(prefix, "principal_point", m_principalPoint.toString(), true);
   kwl.add(prefix, "pixel_size", m_pixelSize.toString(), true);
   kwl.add(prefix, "platform_position",m_platformPosition.toString() ,true);
   kwl.add(prefix, "focal_length", rspfString::toString(m_focalLength) ,true);
   kwl.add(prefix, "image_size", theImageSize.toString() ,true);
   
   return true;
}


