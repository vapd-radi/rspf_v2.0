#include <rspf/projection/rspfSkyBoxLearSensor.h>
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
static rspfTrace traceDebug("rspfSkyBoxLearSensor:debug");
RTTI_DEF1(rspfSkyBoxLearSensor, "rspfSkyBoxLearSensor", rspfSensorModel);
enum
{
   PARAM_ADJ_LON_OFFSET   = 0,
   PARAM_ADJ_LAT_OFFSET,
   PARAM_ADJ_ALTITUDE_OFFSET, 
   PARAM_ADJ_ROLL_OFFSET,
   PARAM_ADJ_PITCH_OFFSET,
   PARAM_ADJ_YAW_OFFSET,
   PARAM_ADJ_FOCAL_LENGTH_OFFSET,
   PARAM_ADJ_COUNT // here as last paramter for resizing the adjustment list
};
rspfSkyBoxLearSensor::rspfSkyBoxLearSensor()
{
   initAdjustableParameters();
   theSensorID = "SkyBoxLear";
}
void rspfSkyBoxLearSensor::imagingRay(const rspfDpt& image_point,
                                      rspfEcefRay&   image_ray) const
{
   rspfDpt f1 (image_point - theRefImgPt);
   f1.x *= m_pixelSize.x;
   f1.y *= -m_pixelSize.y;
   rspfDpt film (f1 - m_principalPoint);
   
   
   
   rspfColumnVector3d rc_body(film.y, film.x, m_focalLength + computeParameterOffset(PARAM_ADJ_FOCAL_LENGTH_OFFSET));
   rspfColumnVector3d rc_ecef = m_ecef2NedInverse*m_air2Ned*rc_body;
   image_ray.setOrigin(m_adjustedPlatformPosition);
   rspfEcefVector     ecf_ray_dir (rc_ecef);
   image_ray.setDirection(ecf_ray_dir);
   
}
void rspfSkyBoxLearSensor::lineSampeToWorld(const rspfDpt& image_point,
                                            rspfGpt&       worldPoint) const
{
   rspfEcefRay ray;
   imagingRay(image_point, ray);
   rspfElevManager::instance()->intersectRay(ray, worldPoint);
   
}
void rspfSkyBoxLearSensor::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                                   const double&   heightEllipsoid,
                                                   rspfGpt&       worldPt) const
{
   rspfEcefRay ray;
   imagingRay(lineSampPt, ray);
   rspfEcefPoint Pecf (ray.intersectAboveEarthEllipsoid(heightEllipsoid));
   worldPt = rspfGpt(Pecf);
}
void rspfSkyBoxLearSensor::updateModel()
{
   m_adjustedPlatformPosition = rspfGpt(m_platformPositionEllipsoid.latd()   + computeParameterOffset(PARAM_ADJ_LAT_OFFSET),
                                         m_platformPositionEllipsoid.lond()   + computeParameterOffset(PARAM_ADJ_LON_OFFSET),
                                         m_platformPositionEllipsoid.height() + computeParameterOffset(PARAM_ADJ_ALTITUDE_OFFSET));
   
   double r = rspf::degreesToRadians(m_roll  + computeParameterOffset(PARAM_ADJ_ROLL_OFFSET));
   double p = rspf::degreesToRadians(m_pitch + computeParameterOffset(PARAM_ADJ_PITCH_OFFSET) );
   double y = rspf::degreesToRadians(m_yaw   + computeParameterOffset(PARAM_ADJ_YAW_OFFSET));
   NEWMAT::Matrix rollM   = rspfMatrix3x3::create(1, 0, 0,
                                                   0, cos(r), sin(r),
                                                   0, -sin(r), cos(r));
   NEWMAT::Matrix pitchM  = rspfMatrix3x3::create(cos(p), 0, -sin(p),
                                                   0,      1, 0,
                                                   sin(p), 0, cos(p));
   NEWMAT::Matrix yawM    = rspfMatrix3x3::create(cos(y), sin(y), 0,
                                                   -sin(y), cos(y), 0,
                                                   0,0,1); 
   m_air2Ned = rollM*pitchM*yawM;
   
   double sin_lat = rspf::sind(m_adjustedPlatformPosition.latd());
   double cos_lat = rspf::cosd(m_adjustedPlatformPosition.latd());
   double sin_lon = rspf::sind(m_adjustedPlatformPosition.lond());
   double cos_lon = rspf::cosd(m_adjustedPlatformPosition.lond());
   m_ecef2Ned = (rspfMatrix3x3::create(0,0,1,0,1,0,-1,0,0)*
                 rspfMatrix3x3::create(cos_lat, 0, sin_lat, 0,1,0, -sin_lat,0,cos_lat)*
                 rspfMatrix3x3::create(cos_lon,sin_lon,0, -sin_lon, cos_lon, 0, 0, 0, 1));
   m_ecef2NedInverse = m_ecef2Ned.t();
   double adjustedFocal = m_focalLength + computeParameterOffset(PARAM_ADJ_FOCAL_LENGTH_OFFSET);
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
   
   try
   {
      computeGsd();
   }
   catch(...)
   {
      
   }
}
void rspfSkyBoxLearSensor::initAdjustableParameters()
{
   resizeAdjustableParameterArray(PARAM_ADJ_COUNT);
   
   setAdjustableParameter(PARAM_ADJ_LON_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_LON_OFFSET, "lon_offset");
   setParameterUnit(PARAM_ADJ_LON_OFFSET, "degrees");
   setParameterSigma(PARAM_ADJ_LON_OFFSET, .001);
   
   setAdjustableParameter(PARAM_ADJ_LAT_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_LAT_OFFSET, "lat_offset");
   setParameterUnit(PARAM_ADJ_LAT_OFFSET, "degrees");
   setParameterSigma(PARAM_ADJ_LAT_OFFSET, .001);
   
   setAdjustableParameter(PARAM_ADJ_ALTITUDE_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_ALTITUDE_OFFSET, "altitude_offset");
   setParameterUnit(PARAM_ADJ_ALTITUDE_OFFSET, "meters");
   setParameterSigma(PARAM_ADJ_ALTITUDE_OFFSET, 10);
   
   setAdjustableParameter(PARAM_ADJ_ROLL_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_ROLL_OFFSET, "roll_offset");
   setParameterUnit(PARAM_ADJ_ROLL_OFFSET, "degrees");
   setParameterSigma(PARAM_ADJ_ROLL_OFFSET, 10);
   
   setAdjustableParameter(PARAM_ADJ_PITCH_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_PITCH_OFFSET, "pitch_offset");
   setParameterUnit(PARAM_ADJ_PITCH_OFFSET, "degrees");
   setParameterSigma(PARAM_ADJ_PITCH_OFFSET, 10);
   
   setAdjustableParameter(PARAM_ADJ_YAW_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_YAW_OFFSET, "yaw_offset");
   setParameterUnit(PARAM_ADJ_YAW_OFFSET, "degrees");
   setParameterSigma(PARAM_ADJ_YAW_OFFSET, .04);
   
   setAdjustableParameter(PARAM_ADJ_FOCAL_LENGTH_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_FOCAL_LENGTH_OFFSET, "focal_length_offset");
   setParameterUnit(PARAM_ADJ_FOCAL_LENGTH_OFFSET, "meters");
   setParameterSigma(PARAM_ADJ_FOCAL_LENGTH_OFFSET, .004);   
}
bool rspfSkyBoxLearSensor::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   theGSD.makeNan();
   theRefImgPt.makeNan();
   rspfSensorModel::loadState(kwl, prefix);
   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }
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
         computeGsd();
      }
      catch (const rspfException& e)
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfSkyBoxLearSensor::loadState Caught Exception:\n"
            << e.what() << std::endl;
         }
      }
   }
   
   return result;
}
bool rspfSkyBoxLearSensor::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfSensorModel::saveState(kwl, prefix);
   kwl.add(prefix, "roll", rspfString::toString(m_roll), true);
   kwl.add(prefix, "pitch", rspfString::toString(m_pitch), true);
   kwl.add(prefix, "yaw", rspfString::toString(m_yaw), true);
   kwl.add(prefix, "principal_point", m_principalPoint.toString(), true);
   kwl.add(prefix, "pixel_size", m_pixelSize.toString(), true);
   kwl.add(prefix, "platform_postion",m_platformPosition.toString() ,true);
   kwl.add(prefix, "focal_length", rspfString::toString(m_focalLength) ,true);
   kwl.add(prefix, "image_size", theImageSize.toString() ,true);
   
   return true;
}
