#include <rspf/projection/rspfBuckeyeSensor.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfLsrRay.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfMatrix4x4.h>
#include <rspf/base/rspfCsvFile.h>
#include <rspf/elevation/rspfElevManager.h>
static rspfTrace traceDebug("rspfBuckeyeSensor:debug");
RTTI_DEF1(rspfBuckeyeSensor, "rspfBuckeyeSensor", rspfSensorModel);
rspfBuckeyeSensor::rspfBuckeyeSensor()
{
   m_lensDistortion = new rspfSmacCallibrationSystem;
   initAdjustableParameters();
}
void rspfBuckeyeSensor::imagingRay(const rspfDpt& image_point,
                                   rspfEcefRay&   image_ray) const
{
   rspfDpt f1 ((image_point) - theRefImgPt);
   f1.x *= m_pixelSize.x;
   f1.y *= -m_pixelSize.y;
   rspfDpt film (f1 - m_principalPoint);
   if (m_lensDistortion.valid())
   {
      rspfDpt filmOut;
      m_lensDistortion->undistort(film, filmOut);
      film = filmOut;
   }
   
   rspfColumnVector3d cam_ray_dir (film.x,
                                    film.y,
                                    -(m_focalLength+computeParameterOffset(6)));
   rspfEcefVector     ecf_ray_dir (m_compositeMatrix*cam_ray_dir);
   ecf_ray_dir = ecf_ray_dir*(1.0/ecf_ray_dir.magnitude());
   
   image_ray.setOrigin(m_ecefPlatformPosition);
   image_ray.setDirection(ecf_ray_dir);
}
void rspfBuckeyeSensor::lineSampleHeightToWorld(const rspfDpt& image_point,
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
      rspfEcefRay ray;
      imagingRay(image_point, ray);
      rspfEcefPoint Pecf (ray.intersectAboveEarthEllipsoid(heightEllipsoid));
      worldPoint = rspfGpt(Pecf);
   }
   
}
void rspfBuckeyeSensor::lineSampleToWorld(const rspfDpt& image_point,
                                          rspfGpt&       gpt) const
{
   if(image_point.hasNans())
   {
      gpt.makeNan();
      return;
   }
   rspfEcefRay ray;
   imagingRay(image_point, ray);
   rspfElevManager::instance()->intersectRay(ray, gpt);
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "image_point = " << image_point << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "ray = " << ray << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "gpt = " << gpt << std::endl;
   }
   
   if (traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSensorModel::lineSampleToWorld: returning..." << std::endl;
   return;
   
}
#if 0
void rspfBuckeyeSensor::worldToLineSample(const rspfGpt& world_point,
                                          rspfDpt&       image_point) const
{
#if 0
   if((theBoundGndPolygon.getNumberOfVertices() > 0)&&
      (!theBoundGndPolygon.hasNans()))
   {
      if (!(theBoundGndPolygon.pointWithin(world_point)))
      {
         image_point.makeNan();
         return;
      }         
   }
#endif
   rspfEcefPoint g_ecf(world_point);
   rspfEcefVector ecfRayDir(g_ecf - m_ecefPlatformPosition);
   rspfColumnVector3d camRayDir (m_compositeMatrixInverse*ecfRayDir.data());
   
   
   double scale = -m_focalLength/camRayDir[2];
   rspfDpt film (scale*camRayDir[0], scale*camRayDir[1]);
   
   if (m_lensDistortion.valid())
   {
      rspfDpt filmOut;
      m_lensDistortion->distort(film, filmOut);
      film = filmOut;
   }
   
   rspfDpt f1(film + m_principalPoint);
   rspfDpt p1(f1.x/m_pixelSize.x,
               -f1.y/m_pixelSize.y);
   
   rspfDpt p0 (p1.x + theRefImgPt.x,
                p1.y + theRefImgPt.y);
   
   image_point = p0;
}
#endif
void rspfBuckeyeSensor::updateModel()
{
   rspfGpt gpt;
   rspfGpt wgs84Pt;
   double metersPerDegree = wgs84Pt.metersPerDegree().y;
   double degreePerMeter = 1.0/metersPerDegree;
   double latShift = computeParameterOffset(1)*degreePerMeter;
   double lonShift = computeParameterOffset(0)*degreePerMeter;
   
   gpt = m_platformPosition;
   double height = gpt.height();
   gpt.height(height + computeParameterOffset(5));
   gpt.latd(gpt.latd() + latShift);
   gpt.lond(gpt.lond() + lonShift);
   
   m_ecefPlatformPosition = gpt;
   rspfLsrSpace lsrSpace(m_ecefPlatformPosition, m_yaw+computeParameterOffset(4));
   
   NEWMAT::Matrix platformLsrMatrix = lsrSpace.lsrToEcefRotMatrix();
   NEWMAT::Matrix orientationMatrix = (                                      
                                       rspfMatrix3x3::createRotationXMatrix(m_pitch+computeParameterOffset(3), RSPF_LEFT_HANDED)
                                       *rspfMatrix3x3::createRotationYMatrix(m_roll+computeParameterOffset(2), RSPF_LEFT_HANDED)
                                      );
   
   m_compositeMatrix         = platformLsrMatrix*orientationMatrix;
   m_compositeMatrixInverse  = m_compositeMatrix.i();
   
   
   theBoundGndPolygon.resize(4);
   rspf_float64 w = theImageClipRect.width()*2.0;
   rspf_float64 h = theImageClipRect.height()*2.0;
   lineSampleToWorld(theImageClipRect.ul()+rspfDpt(-w, -h), gpt);
   theBoundGndPolygon[0] = gpt;
   lineSampleToWorld(theImageClipRect.ur()+rspfDpt(w, -h), gpt);
   theBoundGndPolygon[1] = gpt;
   lineSampleToWorld(theImageClipRect.lr()+rspfDpt(w, h), gpt);
   theBoundGndPolygon[2] = gpt;
   lineSampleToWorld(theImageClipRect.ll()+rspfDpt(-w, h), gpt);
   theBoundGndPolygon[3] = gpt;
   lineSampleToWorld(theRefImgPt, theRefGndPt);
}
void rspfBuckeyeSensor::initAdjustableParameters()
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
   setParameterDescription(4, "yaw");
   setParameterUnit(4, "degrees");
   
   setAdjustableParameter(5, 0.0);
   setParameterDescription(5, "Altitude delta");
   setParameterUnit(5, "meters");
   
   setAdjustableParameter(6, 0.0);
   setParameterDescription(6, "focal length delta");
   setParameterUnit(6, "millimeters");
   
   setParameterSigma(0, 10);
   setParameterSigma(1, 10);
   setParameterSigma(2, 1);
   setParameterSigma(3, 1);
   setParameterSigma(4, 5);
   setParameterSigma(5, 100);
   setParameterSigma(6, 50);
}
bool rspfBuckeyeSensor::loadState(const rspfKeywordlist& kwl, const char* prefix)
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
   rspfString framemeta_gsti = kwl.find(prefix, "framemeta_gsti");
   rspfString frame_number = kwl.find(prefix, "frame_number");
   rspfString pixel_size = kwl.find(prefix, "pixel_size");
   rspfString principal_point = kwl.find(prefix, "principal_point");
   rspfString focal_length = kwl.find(prefix, "focal_length");
   rspfString smac_radial = kwl.find(prefix, "smac_radial");
   rspfString smac_decent = kwl.find(prefix, "smac_decent");
   rspfString roll;
   rspfString pitch;
   rspfString yaw;
   rspfString platform_position;
   m_roll    = 0;
   m_pitch   = 0;
   m_yaw     = 0;
   if(!framemeta_gsti.empty()&&
      !frame_number.empty())
   {
      rspfCsvFile csv(" \t"); // we will use tab or spaces as seaparator
      if(csv.open(framemeta_gsti))
      {
         if(csv.readHeader())
         {
            rspfRefPtr<rspfCsvFile::Record> record;
            bool foundFrameNumber = false;
            while( ((record = csv.nextRecord()).valid()) && !foundFrameNumber)
            {
               if( (*record)["Frame#"] == frame_number)
               {
                  foundFrameNumber = true;
                  roll = (*record)["Roll(deg)"];
                  pitch = (*record)["Pitch(deg)"];
                  yaw = (*record)["Yaw(deg)"];
                  platform_position = "(" + (*record)["Lat(deg)"] + "," 
                                          + (*record)["Lon(deg)"]+ ","
                                          + (*record)["HAE(m)"] + ",WGE)";
               }
            }
         }
      }
   }
   else
   {
      roll = kwl.find(prefix, "roll"); 
      pitch = kwl.find(prefix, "pitch"); 
      yaw = kwl.find(prefix, "yaw"); 
      platform_position = kwl.find(prefix, "platform_position");
   }
   bool result = (!pixel_size.empty()&&
                  !principal_point.empty()&&
                  !focal_length.empty()&&
                  !platform_position.empty());
   
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
   }
   m_lensDistortion = 0;
   if(!smac_radial.empty()&&
      !smac_decent.empty())
   {
      std::vector<rspfString> radial;
      std::vector<rspfString> decent;
      smac_radial.split(radial, " ");
      smac_decent.split(decent, " ");
      if((radial.size() == 5)&&
         (decent.size() == 4))
      {
         m_lensDistortion = new rspfSmacCallibrationSystem(radial[0].toDouble(),
                                                            radial[1].toDouble(),
                                                            radial[2].toDouble(),
                                                            radial[3].toDouble(),
                                                            radial[4].toDouble(),
                                                            decent[0].toDouble(),
                                                            decent[1].toDouble(),
                                                            decent[2].toDouble(),
                                                            decent[3].toDouble());
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
               << "rspfBuckeyeSensor::loadState Caught Exception:\n"
               << e.what() << std::endl;
         }
      }
   }
   
   return result;
}
bool rspfBuckeyeSensor::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfSensorModel::saveState(kwl, prefix);
   kwl.add(prefix, "roll", rspfString::toString(m_roll), true);
   kwl.add(prefix, "pitch", rspfString::toString(m_pitch), true);
   kwl.add(prefix, "yaw", rspfString::toString(m_yaw), true);
   kwl.add(prefix, "principal_point", m_principalPoint.toString(), true);
   kwl.add(prefix, "pixel_size", m_pixelSize.toString(), true);
   if(m_lensDistortion.valid())
   {
      kwl.add(prefix, "smac_radial",
              rspfString::toString(m_lensDistortion->symmetricRadialDistortionCoefficients()[0], 20) + " " + 
              rspfString::toString(m_lensDistortion->symmetricRadialDistortionCoefficients()[1], 20) + " " +
              rspfString::toString(m_lensDistortion->symmetricRadialDistortionCoefficients()[2], 20) + " " +
              rspfString::toString(m_lensDistortion->symmetricRadialDistortionCoefficients()[3], 20) + " " +
              rspfString::toString(m_lensDistortion->symmetricRadialDistortionCoefficients()[4], 20)
              ,true);
      kwl.add(prefix, "smac_decent",
              rspfString::toString(m_lensDistortion->decenteringDistortionCoefficients()[0], 20) + " " + 
              rspfString::toString(m_lensDistortion->decenteringDistortionCoefficients()[1], 20) + " " +
              rspfString::toString(m_lensDistortion->decenteringDistortionCoefficients()[2], 20) + " " +
              rspfString::toString(m_lensDistortion->decenteringDistortionCoefficients()[3], 20)
              ,true);
   }
   kwl.add(prefix, "platform_postion",m_platformPosition.toString() ,true);
   kwl.add(prefix, "focal_length", rspfString::toString(m_focalLength) ,true);
   
   return true;
}
