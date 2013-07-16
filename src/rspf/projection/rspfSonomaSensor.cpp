#include <rspf/projection/rspfSonomaSensor.h>
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
static rspfTrace traceDebug("rspfSonomaSensor:debug");
RTTI_DEF1(rspfSonomaSensor, "rspfSonomaSensor", rspfSensorModel);
rspfSonomaSensor::rspfSonomaSensor()
{
   m_utmProjection = new rspfUtmProjection;
   initAdjustableParameters();
   theSensorID = "Sonoma";
}
void rspfSonomaSensor::imagingRay(const rspfDpt& /* image_point */,
                                   rspfEcefRay&   /* image_ray */) const
{
#if 0
   rspfColumnVector3d v(image_point.x,image_point.y,1.0);
   rspfColumnVector3d v2(0,0,0);
   v2 = m_compositeMatrix*v2;
   v = m_compositeMatrix*(m_mount*(m_pixelToCamera*v));
   rspfEcefVector vec = rspfEcefPoint(v[0], v[1], v[2]) - rspfEcefPoint(v2[0], v2[1], v2[2]); //rspfEcefPoint(v[0], v[1], v[2]) - m_ecefPlatformPosition;
   vec.normalize();
   image_ray.setOrigin(v2);
   image_ray.setDirection(vec);
   
   std::cout << "image ====== " << image_point << std::endl;
   std::cout << "V2 ====== " << v2 << std::endl;
   std::cout << "v ====== " << v << std::endl;
   std::cout << "platform ====== " << m_ecefPlatformPosition << std::endl;
#endif
#if 0
   rspfDpt f1 ((image_point) - theRefImgPt);
   f1.x *= m_pixelSize.x;
   f1.y *= -m_pixelSize.y;
   rspfDpt film (f1 - m_principalPoint);
 
   
   rspfColumnVector3d cam_ray_dir (film.x,
                                    film.y,
                                    -(m_focalLength+computeParameterOffset(6)));
   
   rspfEcefVector     ecf_ray_dir (m_compositeMatrix*m_mount*cam_ray_dir);
   ecf_ray_dir = ecf_ray_dir*(1.0/ecf_ray_dir.magnitude());
   
   image_ray.setOrigin(m_ecefPlatformPosition);
   image_ray.setDirection(ecf_ray_dir);
#endif
}
void rspfSonomaSensor::lineSampleHeightToWorld(const rspfDpt& image_point,
                                                const double&   heightEllipsoid,
                                                rspfGpt&       worldPoint) const
{
   if (!insideImage(image_point))
   {
      worldPoint.makeNan();
      worldPoint = extrapolate(image_point, heightEllipsoid);
   }
   else
   {
      rspfColumnVector3d origin = m_compositeMatrix*rspfColumnVector3d(0,0,0);
      rspfColumnVector3d v = m_compositeMatrix*(m_mount*(m_pixelToCamera*rspfColumnVector3d(image_point.x, image_point.y, 1.0)));
      rspfDpt3d rayOrigin(origin[0], origin[1], origin[2]);
      rspfDpt3d rayDirection(v[0]-origin[0],
                              v[1]-origin[1],
                              v[2]-origin[2]);
      rayDirection = rayDirection/rayDirection.length();
      rspfDpt3d result;
      double h = heightEllipsoid;
      
      if(rspf::isnan(heightEllipsoid))
      {
         h = 0.0;
      }
      
      intersectRayWithHeight(*(m_utmProjection.get()), result, rayOrigin, rayDirection, h);
               
      worldPoint = m_utmProjection->inverse(rspfDpt(result.x, result.y));
      worldPoint.height(result.z);
   }
   
}
void rspfSonomaSensor::lineSampleToWorld(const rspfDpt& image_point,
                                                rspfGpt&       gpt) const
{
   if(image_point.hasNans())
   {
      gpt.makeNan();
      return;
   }
   
   rspfColumnVector3d origin = m_compositeMatrix*rspfColumnVector3d(0,0,0);
   rspfColumnVector3d v = m_compositeMatrix*(m_mount*(m_pixelToCamera*rspfColumnVector3d(image_point.x, image_point.y, 1.0)));
   rspfDpt3d rayOrigin(origin[0], origin[1], origin[2]);
   rspfDpt3d rayDirection(v[0]-origin[0],
                           v[1]-origin[1],
                           v[2]-origin[2]);
   rayDirection = rayDirection/rayDirection.length();
   rspfDpt3d result;
   intersectRay(*(m_utmProjection.get()), result, rayOrigin, rayDirection);
   gpt = m_utmProjection->inverse(rspfDpt(result.x, result.y));
   gpt.height(result.z);
   
#if 0
   rspfEcefRay ray;
   imagingRay(image_point, ray);
   rspfElevManager::instance()->intersectRay(ray, gpt);
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "image_point = " << image_point << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "ray = " << ray << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "gpt = " << gpt << std::endl;
   }
#endif   
   if (traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSensorModel::lineSampleToWorld: returning..." << std::endl;
   return;
   
}
void rspfSonomaSensor::updateModel()
{
   rspfGpt gpt = m_platformPositionEllipsoid;
   m_ecefPlatformPosition = gpt;
   double r = rspf::degreesToRadians(m_roll);
   double p = rspf::degreesToRadians(-(90+m_pitch));
   double y = rspf::degreesToRadians(m_heading);
   
   NEWMAT::Matrix rollM = rspfMatrix4x4(cos(r), -sin(r), 0.0, 0.0,
                                         sin(r), cos(r), 0.0, 0.0,
                                         0.0, 0.0, 1.0, 0.0,
                                         0.0, 0.0, 0.0, 1.0).getData();
   NEWMAT::Matrix pitchM = rspfMatrix4x4(1.0, 0.0, 0.0, 0.0,    
                                          0.0, cos(p), -sin(p), 0.0,
                                          0.0, sin(p), cos(p), 0.0,
                                          0.0, 0.0, 0.0, 1.0).getData();
   NEWMAT::Matrix headingM = rspfMatrix4x4(cos(y), -sin(y), 0.0, 0.0,
                                            sin(y), cos(y), 0.0, 0.0,
                                            0, 0, 1.0, 0.0,
                                            0.0, 0.0, 0.0, 1.0).getData();
   
   m_utmProjection->setZone(rspfUtmProjection::computeZone(m_platformPositionEllipsoid));
   m_utmProjection->setHemisphere(m_platformPositionEllipsoid.latd()>=0.0?'N':'S');
   m_pixelToCamera = rspfMatrix3x3::create(m_pixelSize.x/m_focalLength, 0.0, -(m_pixelSize.x/m_focalLength)*theImageSize.x/2,
                                            0.0, -m_pixelSize.x/m_focalLength,   (m_pixelSize.x/m_focalLength)*theImageSize.y/2,
                                            0.0, 0.0,-1.0);
   rspfDpt utm = m_utmProjection->forward(m_platformPositionEllipsoid);
   
   m_compositeMatrixInverse = rollM * pitchM * headingM * rspfMatrix4x4::createTranslationMatrix(-utm.x, -utm.y, -m_platformPositionEllipsoid.height());
   m_compositeMatrix = m_compositeMatrixInverse.i();
   
#if 0
   rspfGpt gpt = m_platformPositionEllipsoid;
   m_ecefPlatformPosition = gpt;
   
   double r = rspf::degreesToRadians(m_roll);
   double p = rspf::degreesToRadians(-(90+m_pitch));
   double y = rspf::degreesToRadians(m_heading);
   
   NEWMAT::Matrix rollM = rspfMatrix4x4(cos(r), -sin(r), 0.0, 0.0,
                                         sin(r), cos(r), 0.0, 0.0,
                                         0.0, 0.0, 1.0, 0.0,
                                         0.0, 0.0, 0.0, 1.0).getData();
   NEWMAT::Matrix pitchM = rspfMatrix4x4(1.0, 0.0, 0.0, 0.0,    
                                          0.0, cos(p), -sin(p), 0.0,
                                          0.0, sin(p), cos(p), 0.0,
                                          0.0, 0.0, 0.0, 1.0).getData();
   NEWMAT::Matrix headingM = rspfMatrix4x4(cos(y), -sin(y), 0.0, 0.0,
                                            sin(y), cos(y), 0.0, 0.0,
                                            0, 0, 1.0, 0.0,
                                            0.0, 0.0, 0.0, 1.0).getData();
   rspfLsrSpace lsrSpace(m_ecefPlatformPosition);//,m_heading+computeParameterOffset(4));
   
   NEWMAT::Matrix platformLsrMatrix4x4 = rspfMatrix4x4::createIdentity();
#endif
#if 0  
   m_utmProjection.setZone(rspfUtmProjection::computeZone(m_platformPositionEllipsoid));
   m_utmProjection.setHemisphere(m_platformPositionEllipsoid.latd()>=0.0?'N':'S');
   m_utmProjection.setMetersPerPixel(rspfDpt(1.0,1.0));
   rspfDpt midPt  = m_utmProjection.forward(m_ecefPlatformPosition);
   rspfDpt rPt    = midPt + rspfDpt(10, 0.0);
   rspfDpt uPt    = midPt + rspfDpt(0.0, 10.0);
   rspfGpt wMidPt = m_utmProjection.inverse(midPt);
   rspfGpt wRPt   = m_utmProjection.inverse(rPt);
   rspfGpt wUPt   = m_utmProjection.inverse(uPt);
   
   rspfEcefPoint ecefMidPt = wMidPt;
   rspfEcefPoint ecefRPt   = wRPt;
   rspfEcefPoint ecefUPt   = wUPt;
   
   rspfEcefVector east  = ecefRPt-ecefMidPt;
   rspfEcefVector north = ecefUPt-ecefMidPt;
   east.normalize();
   north.normalize();
   
   lsrSpace = rspfLsrSpace(m_ecefPlatformPosition,
                            east,
                            north,
                            0);
#endif
   
   
#if 0
   NEWMAT::Matrix platformLsrMatrix = lsrSpace.ecefToLsrRotMatrix();
   platformLsrMatrix4x4[0][0] = platformLsrMatrix[0][0];
   platformLsrMatrix4x4[1][0] = platformLsrMatrix[1][0];
   platformLsrMatrix4x4[2][0] = platformLsrMatrix[2][0];
   platformLsrMatrix4x4[0][1] = platformLsrMatrix[0][1];
   platformLsrMatrix4x4[1][1] = platformLsrMatrix[1][1];
   platformLsrMatrix4x4[2][1] = platformLsrMatrix[2][1];
   platformLsrMatrix4x4[0][2] = platformLsrMatrix[0][2];
   platformLsrMatrix4x4[1][2] = platformLsrMatrix[1][2];
   platformLsrMatrix4x4[2][2] = platformLsrMatrix[2][2];
  
   m_compositeMatrix   = (rollM*pitchM*headingM*platformLsrMatrix4x4).i();
   m_compositeMatrixInverse = m_compositeMatrix.i();
#endif
   
#if 0
   platformLsrMatrix4x4[0][0] = platformLsrMatrix[0][0];
   platformLsrMatrix4x4[1][0] = platformLsrMatrix[1][0];
   platformLsrMatrix4x4[2][0] = platformLsrMatrix[2][0];
   platformLsrMatrix4x4[0][1] = platformLsrMatrix[0][1];
   platformLsrMatrix4x4[1][1] = platformLsrMatrix[1][1];
   platformLsrMatrix4x4[2][1] = platformLsrMatrix[2][1];
   platformLsrMatrix4x4[0][2] = platformLsrMatrix[0][2];
   platformLsrMatrix4x4[1][2] = platformLsrMatrix[1][2];
   platformLsrMatrix4x4[2][2] = platformLsrMatrix[2][2];
   NEWMAT::Matrix m = (platformLsrMatrix4x4*(rspfMatrix4x4::createRotationZMatrix(r)*rspfMatrix4x4::createRotationXMatrix(p)*rspfMatrix4x4::createRotationZMatrix(y)))*
                       rspfMatrix4x4::createTranslationMatrix(-m_ecefPlatformPosition.x(), -m_ecefPlatformPosition.y(), -m_ecefPlatformPosition.z());
   m_compositeMatrixInverse = m;
   m_compositeMatrix = m.i();
   NEWMAT::Matrix pixelToCamera = NEWMAT::Matrix(3, 3);
   double f  = m_focalLength/m_pixelSize.x;
   double w  = theImageClipRect.width();
   double h  = theImageClipRect.height();
   pixelToCamera << f << 0 << (-f*w/2.0) << 0  << -f << (f*h/2.0) << 0 << 0 <<-1;
   pixelToCamera = pixelToCamera.t();
   m_pixelToCamera = pixelToCamera;
#endif
   theBoundGndPolygon.resize(0);
#if 0
   theBoundGndPolygon.resize(4);
   rspf_float64 w2 = theImageClipRect.width()*2;
   rspf_float64 h2 = theImageClipRect.height()*2;
   lineSampleToWorld(theImageClipRect.ul()+rspfDpt(-w2, -h2), gpt);
   theBoundGndPolygon[0] = gpt;
   lineSampleToWorld(theImageClipRect.ur()+rspfDpt(w2, -h2), gpt);
   theBoundGndPolygon[1] = gpt;
   lineSampleToWorld(theImageClipRect.lr()+rspfDpt(w2, h2), gpt);
   theBoundGndPolygon[2] = gpt;
   lineSampleToWorld(theImageClipRect.ll()+rspfDpt(-w2, h2), gpt);
   theBoundGndPolygon[3] = gpt;
#endif
   lineSampleToWorld(theRefImgPt, theRefGndPt);
   
   
   
   
   
   computeGsd();
}
void rspfSonomaSensor::initAdjustableParameters()
{
   resizeAdjustableParameterArray(7);
   
   setAdjustableParameter(0, 0.0);
   setParameterDescription(0, "x_offset");
   setParameterUnit(0, "meters");
   
   setAdjustableParameter(1, 0.0);
   setParameterDescription(1, "y_offset");
   setParameterUnit(1, "meters");
   
   setAdjustableParameter(2, 0.0);
   setParameterDescription(2, "roll");
   setParameterUnit(2, "degrees");
   
   setAdjustableParameter(3, 0.0);
   setParameterDescription(3, "pitch");
   setParameterUnit(3, "degrees");
   
   setAdjustableParameter(4, 0.0);
   setParameterDescription(4, "heading");
   setParameterUnit(4, "degrees");
   
   setAdjustableParameter(5, 0.0);
   setParameterDescription(5, "Altitude delta");
   setParameterUnit(5, "meters");
   
   setAdjustableParameter(6, 0.0);
   setParameterDescription(6, "focal length delta");
   setParameterUnit(6, "meters");
   
   setParameterSigma(0, 10);
   setParameterSigma(1, 10);
   setParameterSigma(2, 1);
   setParameterSigma(3, 1);
   setParameterSigma(4, 5);
   setParameterSigma(5, 100);
   setParameterSigma(6, 50);
   
}
bool rspfSonomaSensor::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }
   theGSD.makeNan();
   theRefImgPt.makeNan();
   rspfSensorModel::loadState(kwl, prefix);
   if(theRefImgPt.hasNans())
   {
      theRefImgPt = theImageClipRect.midPoint();
   }
   rspfString mount           = kwl.find(prefix, "mount");
   rspfString pixel_size = kwl.find(prefix, "pixel_size");
   rspfString principal_point = kwl.find(prefix, "principal_point");
   rspfString focal_length = kwl.find(prefix, "focal_length");
   rspfString roll;
   rspfString pitch;
   rspfString heading;
   rspfString platform_position;
   m_roll    = 0;
   m_pitch   = 0;
   m_heading     = 0;
   roll = kwl.find(prefix, "roll"); 
   pitch = kwl.find(prefix, "pitch"); 
   heading = kwl.find(prefix, "heading"); 
   platform_position = kwl.find(prefix, "platform_position");
   bool result = (!pixel_size.empty()&&
                  !principal_point.empty()&&
                  !focal_length.empty()&&
                  !platform_position.empty());
   if(mount)
   {
      mount = mount.trim();
      std::vector<rspfString> values;
      
      mount.split(values, " ");
      bool valid = true;
      if(values.size() == 9)
      {
         m_mount = rspfMatrix3x3::createIdentity();
      }
      else if((values.size() == 16) ||
              (values.size() == 12))
      {
         m_mount = rspfMatrix4x4::createIdentity();
      }
      else 
      {
         valid = false;
      }
      rspf_uint32 idx = 0;
      rspf_int32 row = -1;
      for(idx = 0; idx < values.size(); ++idx)
      {
         if(idx%4 == 0) ++row;
         m_mount[row][idx%4] = values[idx].toDouble();
      }
      m_mount = m_mount.i();
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
   if(!heading.empty())
   {
      m_heading   = heading.toDouble();
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
   theImageSize = rspfDpt(theImageClipRect.width(),
                           theImageClipRect.height());
   
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
            << "rspfSonomaSensor::loadState Caught Exception:\n"
            << e.what() << std::endl;
         }
      }
   }
   
   return result;
}
bool rspfSonomaSensor::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfSensorModel::saveState(kwl, prefix);
   kwl.add(prefix, "roll", rspfString::toString(m_roll), true);
   kwl.add(prefix, "pitch", rspfString::toString(m_pitch), true);
   kwl.add(prefix, "heading", rspfString::toString(m_heading), true);
   kwl.add(prefix, "principal_point", m_principalPoint.toString(), true);
   kwl.add(prefix, "pixel_size", m_pixelSize.toString(), true);
   kwl.add(prefix, "platform_postion",m_platformPosition.toString() ,true);
   kwl.add(prefix, "focal_length", rspfString::toString(m_focalLength) ,true);
   
   return true;
}
bool rspfSonomaSensor::intersectRay(const rspfMapProjection& proj, rspfDpt3d& result, rspfDpt3d& origin, rspfDpt3d& dir)const
{
   rspfPlane plane(0.0, 0.0, 1.0, 0.0);
   
   static const double CONVERGENCE_THRESHOLD = 0.0001; // meters
   static const int    MAX_NUM_ITERATIONS    = 50;
   
   double          h; // height above ellipsoid
   bool            intersected;
   rspfDpt3d  prev_intersect_pt (origin);
   rspfDpt3d  new_intersect_pt;
   double          distance;
   bool            done = false;
   int             iteration_count = 0;
   
   if(dir.hasNans()) 
   {
      result.makeNan();
      return false;
   }
   
   rspfGpt gpt = proj.inverse(rspfDpt(origin.x, origin.y));
   do
   {
      h = rspfElevManager::instance()->getHeightAboveEllipsoid(gpt);
      
      if ( rspf::isnan(h) ) h = 0.0;
      plane.setOffset(-h);
      intersected = plane.intersect(new_intersect_pt, origin, dir);
      if (!intersected)
      {
         result.makeNan();
         done = true;
      }
      else
      {
         gpt =  proj.inverse(rspfDpt(new_intersect_pt.x, new_intersect_pt.y));
         gpt.height(new_intersect_pt.z);
         result = new_intersect_pt;
         distance = (new_intersect_pt - prev_intersect_pt).length();
         if (distance < CONVERGENCE_THRESHOLD)
         {
            done = true;
         }
         else
         {
            prev_intersect_pt = new_intersect_pt;
         }
      }
      
      iteration_count++;
      
   } while ((!done) && (iteration_count < MAX_NUM_ITERATIONS));
   
   return intersected;
}
bool rspfSonomaSensor::intersectRayWithHeight(const rspfMapProjection& /* proj */, rspfDpt3d& result, rspfDpt3d& origin, rspfDpt3d& dir, double h)const
{
   rspfPlane plane(0.0, 0.0, 1.0, -h);
   bool intersected = plane.intersect(result, origin, dir);
  
   
   return intersected;
}
