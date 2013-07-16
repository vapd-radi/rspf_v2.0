//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LGPL
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfApplanixUtmModel.cpp 20485 2012-01-23 18:22:38Z gpotts $
#include <sstream>
#include <rspf/projection/rspfApplanixUtmModel.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/support_data/rspfApplanixEOFile.h>
#include <rspf/base/rspfMatrix4x4.h>
#include <rspf/elevation/rspfElevManager.h>
static rspfTrace traceDebug("rspfApplanixUtmModel:debug");

RTTI_DEF1(rspfApplanixUtmModel, "rspfApplanixUtmModel", rspfSensorModel);

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfApplanixUtmModel.cpp 20485 2012-01-23 18:22:38Z gpotts $";
#endif

rspfApplanixUtmModel::rspfApplanixUtmModel()
   :theOmega(0.0),
    thePhi(0.0),
    theKappa(0.0),
    theBoreSightTx(0.0),
    theBoreSightTy(0.0),
    theBoreSightTz(0.0)
{
   theCompositeMatrix          = rspfMatrix4x4::createIdentity();
   theCompositeMatrixInverse   = rspfMatrix4x4::createIdentity();
   theFocalLength              = 55.0;
   thePixelSize = rspfDpt(.009, .009);
   theEcefPlatformPosition = rspfGpt(0.0,0.0, 1000.0);
   theGSD.x = 0.1524;
   theGSD.y = 0.1524;
   theMeanGSD = 0.1524;
   theLensDistortion = new rspfMeanRadialLensDistortion;
   initAdjustableParameters();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfApplanixUtmModel::rspfApplanixUtmModel DEBUG:" << endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)<< "RSPF_ID:  " << RSPF_ID << endl;
#endif
   }
}

rspfApplanixUtmModel::rspfApplanixUtmModel(const rspfApplanixUtmModel& src)
   :rspfSensorModel(src),
    theCompositeMatrix(src.theCompositeMatrix),
    theCompositeMatrixInverse(src.theCompositeMatrixInverse),
    theOmega(src.theOmega),
    thePhi(src.thePhi),
    theKappa(src.theKappa),
    theBoreSightTx(src.theBoreSightTx),
    theBoreSightTy(src.theBoreSightTy),
    theBoreSightTz(src.theBoreSightTz),
    thePrincipalPoint(src.thePrincipalPoint),
    thePixelSize(src.thePixelSize),
    theFocalLength(src.theFocalLength),
    theEcefPlatformPosition(src.theEcefPlatformPosition),
    thePlatformPosition(src.thePlatformPosition),
    theUtmZone(src.theUtmZone),
    theUtmHemisphere(src.theUtmHemisphere),
    theUtmPlatformPosition(src.theUtmPlatformPosition)
{
   if(src.theLensDistortion.valid())
   {
      theLensDistortion = new rspfMeanRadialLensDistortion(*(src.theLensDistortion.get()));
   }
   else
   {
      theLensDistortion = new rspfMeanRadialLensDistortion();
   }
}

rspfObject* rspfApplanixUtmModel::dup()const
{
   return new rspfApplanixUtmModel(*this);
}

void rspfApplanixUtmModel::imagingRay(const rspfDpt& image_point,
                                    rspfEcefRay&   image_ray) const
{
//    if(traceDebug())
//    {
//       rspfNotify(rspfNotifyLevel_DEBUG) << "rspfApplanixUtmModel::imagingRay: ..... entered" << std::endl;
//    }
    rspfDpt f1 ((image_point) - theRefImgPt);
   f1.x *= thePixelSize.x;
   f1.y *= -thePixelSize.y;
   rspfDpt film (f1 - thePrincipalPoint);
//    if(traceDebug())
//    {
//       rspfNotify(rspfNotifyLevel_DEBUG) << "pixel size   = " << thePixelSize << std::endl;
//       rspfNotify(rspfNotifyLevel_DEBUG) << "principal pt = " << thePrincipalPoint << std::endl;
//       rspfNotify(rspfNotifyLevel_DEBUG) << "film pt      = " << film << std::endl;
//    }
   if (theLensDistortion.valid())
   {
      rspfDpt filmOut;
      theLensDistortion->undistort(film, filmOut);
      film = filmOut;
   }
   
   rspfColumnVector3d cam_ray_dir (film.x,
                                    film.y,
                                    -theFocalLength);
   rspfEcefVector     ecf_ray_dir (theCompositeMatrix*cam_ray_dir);
   ecf_ray_dir = ecf_ray_dir*(1.0/ecf_ray_dir.magnitude());
   
   image_ray.setOrigin(theAdjEcefPlatformPosition);
   image_ray.setDirection(ecf_ray_dir);
   
//    if(traceDebug())
//    {
//       rspfNotify(rspfNotifyLevel_DEBUG) << "rspfApplanixUtmModel::imagingRay: ..... leaving" << std::endl;
//    }
}

void rspfApplanixUtmModel::lineSampleToWorld(const rspfDpt& image_point,
                                               rspfGpt&       gpt) const
{
   if (traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfApplanixEcefModel::lineSampleToWorld:entering..." << std::endl;
   
   if(image_point.hasNans())
   {
      gpt.makeNan();
      return;
   }
   //***
   // Extrapolate if image point is outside image:
   //***
  // if (!insideImage(image_point))
  // {
   //   gpt.makeNan();
//       gpt = extrapolate(image_point);
   //   return;
   //}

   //***
   // Determine imaging ray and invoke elevation source object's services to
   // intersect ray with terrain model:
   //***
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

void rspfApplanixUtmModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                                 const double&   heightEllipsoid,
                                                 rspfGpt&       worldPoint) const
{
//   if (!insideImage(image_point))
//   {
      //worldPoint.makeNan();
//       worldPoint = extrapolate(image_point, heightEllipsoid);
//   }
//   else
   {
      //***
      // First establish imaging ray from image point:
      //***
      rspfEcefRay ray;
      imagingRay(image_point, ray);
      rspfEcefPoint Pecf (ray.intersectAboveEarthEllipsoid(heightEllipsoid));
      worldPoint = rspfGpt(Pecf);
   }
}

void rspfApplanixUtmModel::worldToLineSample(const rspfGpt& world_point,
                                           rspfDpt&       image_point) const
{
#if 0
   if((theBoundGndPolygon.getNumberOfVertices() > 0)&&
      (!theBoundGndPolygon.hasNans()))
   {
      if (!(theBoundGndPolygon.pointWithin(world_point)))
      {
         image_point.makeNan();
//          image_point = extrapolate(world_point);
         return;
      }         
   }
#endif
   rspfEcefPoint g_ecf(world_point);
   rspfEcefVector ecfRayDir(g_ecf - theAdjEcefPlatformPosition);
   rspfColumnVector3d camRayDir (theCompositeMatrixInverse*ecfRayDir.data());
   
      
   double scale = -theFocalLength/camRayDir[2];
   rspfDpt film (scale*camRayDir[0], scale*camRayDir[1]);
      
   if (theLensDistortion.valid())
   {
      rspfDpt filmOut;
      theLensDistortion->distort(film, filmOut);
      film = filmOut;
    }
   
     rspfDpt f1(film + thePrincipalPoint);
    rspfDpt p1(f1.x/thePixelSize.x,
                -f1.y/thePixelSize.y);

    rspfDpt p0 (p1.x + theRefImgPt.x,
                 p1.y + theRefImgPt.y);
    
    image_point = p0;
}

void rspfApplanixUtmModel::updateModel()
{

   rspfGpt wgs84Pt;
   double metersPerDegree = wgs84Pt.metersPerDegree().x;
   double degreePerMeter = 1.0/metersPerDegree;
   double latShift = -computeParameterOffset(1)*theMeanGSD*degreePerMeter;
   double lonShift = computeParameterOffset(0)*theMeanGSD*degreePerMeter;

   rspfGpt gpt = thePlatformPosition;
//   gpt.height(0.0);
   double height = gpt.height();
   gpt.height(height + computeParameterOffset(5));
   gpt.latd(gpt.latd() + latShift);
   gpt.lond(gpt.lond() + lonShift);
   
   theAdjEcefPlatformPosition = gpt;
   rspfLsrSpace lsrSpace(theAdjEcefPlatformPosition);
   // ORIENT TO A UTM AXIS
   //
    NEWMAT::ColumnVector v(3);
   
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
    NEWMAT::ColumnVector v2 = lsrSpace.lsrToEcefRotMatrix()*v;
    rspfEcefVector zVector(v2[0], v2[1], v2[2]);
    zVector.normalize();
   
   // now lets create a UTM axis by creating a derivative at the center
   // by shift over a few pixels and subtracting
   //
   rspfUtmProjection utmProj;
   
   utmProj.setZone(theUtmZone);
   utmProj.setZone(theUtmHemisphere);
   utmProj.setMetersPerPixel(rspfDpt(1.0,1.0));
   rspfDpt midPt  = utmProj.forward(theAdjEcefPlatformPosition);
   rspfDpt rPt    = midPt + rspfDpt(10, 0.0);
   rspfDpt uPt    = midPt + rspfDpt(0.0, 10.0);
   rspfGpt wMidPt = utmProj.inverse(midPt);
   rspfGpt wRPt   = utmProj.inverse(rPt);
   rspfGpt wUPt   = utmProj.inverse(uPt);
   
   rspfEcefPoint ecefMidPt = wMidPt;
   rspfEcefPoint ecefRPt   = wRPt;
   rspfEcefPoint ecefUPt   = wUPt;
   
   rspfEcefVector east  = ecefRPt-ecefMidPt;
   rspfEcefVector north = ecefUPt-ecefMidPt;
   east.normalize();
   north.normalize();
   
   // now use the lsr space constructors to construct an orthogonal set of axes
   //
   lsrSpace = rspfLsrSpace(thePlatformPosition,
                            0,
                            north,
                            east.cross(north));
//   lsrSpace = rspfLsrSpace(thePlatformPosition);
   // DONE ORIENT TO UTM AXIS
   
   NEWMAT::Matrix platformLsrMatrix = lsrSpace.lsrToEcefRotMatrix();
   NEWMAT::Matrix orientationMatrix = (rspfMatrix3x3::createRotationXMatrix(theOmega+computeParameterOffset(2), RSPF_LEFT_HANDED)*
                                       rspfMatrix3x3::createRotationYMatrix(thePhi+computeParameterOffset(3), RSPF_LEFT_HANDED)*
                                       rspfMatrix3x3::createRotationZMatrix(theKappa+computeParameterOffset(4), RSPF_LEFT_HANDED));
   
   theCompositeMatrix         = platformLsrMatrix*orientationMatrix;
   theCompositeMatrixInverse  = theCompositeMatrix.i();

//   theAdjEcefPlatformPosition = theEcefPlatformPosition; 

   theBoundGndPolygon.resize(4);
   // rspf_float64 w = theImageClipRect.width();//*2.0;
   // rspf_float64 h = theImageClipRect.height();//*2.0;
   
   lineSampleToWorld(theImageClipRect.ul(),gpt);//+rspfDpt(-w, -h), gpt);
   theBoundGndPolygon[0] = gpt;
   lineSampleToWorld(theImageClipRect.ur(),gpt);//+rspfDpt(w, -h), gpt);
   theBoundGndPolygon[1] = gpt;
   lineSampleToWorld(theImageClipRect.lr(),gpt);//+rspfDpt(w, h), gpt);
   theBoundGndPolygon[2] = gpt;
   lineSampleToWorld(theImageClipRect.ll(),gpt);//+rspfDpt(-w, h), gpt);
   theBoundGndPolygon[3] = gpt;
}

void rspfApplanixUtmModel::initAdjustableParameters()
{
   resizeAdjustableParameterArray(6);
   
   setAdjustableParameter(0, 0.0);
   setParameterDescription(0, "x_offset");
   setParameterUnit(0, "pixels");

   setAdjustableParameter(1, 0.0);
   setParameterDescription(1, "y_offset");
   setParameterUnit(1, "pixels");

   setAdjustableParameter(2, 0.0);
   setParameterDescription(2, "orientation x");
   setParameterUnit(2, "degrees");

   setAdjustableParameter(3, 0.0);
   setParameterDescription(3, "orientation y");
   setParameterUnit(3, "degrees");

   setAdjustableParameter(4, 0.0);
   setParameterDescription(4, "orientation z");
   setParameterUnit(4, "degrees");

   setAdjustableParameter(5, 0.0);
   setParameterDescription(5, "Altitude delta");
   setParameterUnit(5, "meters");

   
   
   setParameterSigma(0, 20.0);
   setParameterSigma(1, 20.0);
   setParameterSigma(2, .1);
   setParameterSigma(3, .1);
   setParameterSigma(4, .1);
   setParameterSigma(5, 50);
}

void rspfApplanixUtmModel::setPrincipalPoint(rspfDpt principalPoint)
{
   thePrincipalPoint = principalPoint;
}

void rspfApplanixUtmModel::setPixelSize(const rspfDpt& pixelSize)
{
   thePixelSize = pixelSize;
}

void rspfApplanixUtmModel::setImageRect(const rspfDrect& rect)
{
   theImageClipRect = rect;
   theRefImgPt = rect.midPoint();
}

void rspfApplanixUtmModel::setFocalLength(double focalLength)
{
   theFocalLength = focalLength;
}

void rspfApplanixUtmModel::setPlatformPosition(const rspfGpt& gpt)
{
   theRefGndPt             = gpt;
   theEcefPlatformPosition = gpt;
   updateModel();
   
}

bool rspfApplanixUtmModel::saveState(rspfKeywordlist& kwl,
                                   const char* prefix) const
{
   rspfSensorModel::saveState(kwl, prefix);
   
   kwl.add(prefix, "type", "rspfApplanixUtmModel", true);

   kwl.add(prefix, "omega", theOmega, true);
   kwl.add(prefix, "phi", thePhi, true);
   kwl.add(prefix, "kappa", theKappa, true);
   kwl.add(prefix, "bore_sight_tx", theBoreSightTx*60, true);
   kwl.add(prefix, "bore_sight_ty", theBoreSightTy*60, true);
   kwl.add(prefix, "bore_sight_tz", theBoreSightTz*60, true);
   kwl.add(prefix, "principal_point", rspfString::toString(thePrincipalPoint.x) + " " + rspfString::toString(thePrincipalPoint.y));
   kwl.add(prefix, "pixel_size",      rspfString::toString(thePixelSize.x) + " " + rspfString::toString(thePixelSize.y));
   kwl.add(prefix, "focal_length", theFocalLength);
   kwl.add(prefix, "ecef_platform_position",
           rspfString::toString(theEcefPlatformPosition.x()) + " " +
           rspfString::toString(theEcefPlatformPosition.y()) + " " +
           rspfString::toString(theEcefPlatformPosition.z()));
   kwl.add(prefix, "latlonh_platform_position",
           rspfString::toString(thePlatformPosition.latd()) + " " +
           rspfString::toString(thePlatformPosition.lond()) + " " +
           rspfString::toString(thePlatformPosition.height()) + " " +
           thePlatformPosition.datum()->code());
   kwl.add(prefix,
           "utm_platform_position",
           rspfString::toString(theUtmPlatformPosition.x) + " " +
           rspfString::toString(theUtmPlatformPosition.y) + " " +
           rspfString::toString(theUtmPlatformPosition.z) + " " +
           thePlatformPosition.datum()->code(),
           true);
   kwl.add(prefix,
           "utm_zone",
           theUtmZone,
           true);
   kwl.add(prefix,
           "utm_hemisphere",
           theUtmHemisphere,
           true);
   kwl.add(prefix,
           "shift_values",
           rspfString::toString(theShiftValues.x()) + " " + 
           rspfString::toString(theShiftValues.y()) + " " +
           rspfString::toString(theShiftValues.z()),
           true);
   if(theLensDistortion.valid())
   {
      rspfString lensPrefix = rspfString(prefix)+"distortion.";
      theLensDistortion->saveState(kwl,
                                   lensPrefix.c_str());
   }
   
   return true;
}

bool rspfApplanixUtmModel::loadState(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   if(traceDebug())
   {
      std::cout << "rspfApplanixUtmModel::loadState: ......... entered" << std::endl;
   }
   theImageClipRect = rspfDrect(0,0,4076,4091);
   theRefImgPt      = rspfDpt(2046.0, 2038.5);
   
   rspfSensorModel::loadState(kwl, prefix);

   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }

   const char* eo_file           = kwl.find(prefix, "eo_file");
   const char* eo_id             = kwl.find(prefix, "eo_id");
   const char* omega             = kwl.find(prefix, "omega");
   const char* phi               = kwl.find(prefix, "phi");
   const char* kappa             = kwl.find(prefix, "kappa");
   const char* bore_sight_tx     = kwl.find(prefix, "bore_sight_tx");
   const char* bore_sight_ty     = kwl.find(prefix, "bore_sight_ty");
   const char* bore_sight_tz     = kwl.find(prefix, "bore_sight_tz");
   
   const char* principal_point   = kwl.find(prefix, "principal_point");
   const char* pixel_size        = kwl.find(prefix, "pixel_size");
   const char* focal_length      = kwl.find(prefix, "focal_length");
   const char* latlonh_platform_position = kwl.find(prefix, "latlonh_platform_position");
   const char* utm_platform_position = kwl.find(prefix, "utm_platform_position");
   const char* compute_gsd_flag  = kwl.find(prefix, "compute_gsd_flag");
   const char* utm_zone          = kwl.find(prefix, "utm_zone");
   const char* utm_hemisphere    = kwl.find(prefix, "utm_hemisphere");
   const char* camera_file       = kwl.find(prefix, "camera_file");
   const char* shift_values      = kwl.find(prefix, "shift_values");
   
   theCompositeMatrix          = rspfMatrix3x3::createIdentity();
   theCompositeMatrixInverse   = rspfMatrix3x3::createIdentity();
   theOmega                    = 0.0;
   thePhi                      = 0.0;
   theKappa                    = 0.0;
   theBoreSightTx              = 0.0;
   theBoreSightTy              = 0.0;
   theBoreSightTz              = 0.0;
   theFocalLength              = 55.0;
   thePixelSize = rspfDpt(.009, .009);
   theEcefPlatformPosition = rspfGpt(0.0,0.0, 1000.0);

   bool loadedFromEoFile = false;

   if(eo_id)
   {
      theImageID = eo_id;
   }
   // loading from standard eo file with given record id
   //
   if(eo_file)
   {
      rspfApplanixEOFile eoFile;
      
      if(eoFile.parseFile(rspfFilename(eo_file)))
      {
         rspfRefPtr<rspfApplanixEORecord> record = eoFile.getRecordGivenId(theImageID);

         if(record.valid())
         {
            loadedFromEoFile = true;
            theBoreSightTx = eoFile.getBoreSightTx()/60.0;
            theBoreSightTy = eoFile.getBoreSightTy()/60.0;
            theBoreSightTz = eoFile.getBoreSightTz()/60.0;
            theShiftValues = rspfEcefVector(eoFile.getShiftValuesX(),
                                             eoFile.getShiftValuesY(),
                                             eoFile.getShiftValuesZ());
            rspf_int32 easting  = eoFile.getFieldIdxLike("EASTING");
            rspf_int32 northing = eoFile.getFieldIdxLike("NORTHING");
            rspf_int32 height   = eoFile.getFieldIdxLike("HEIGHT");
            rspf_int32 omega    = eoFile.getFieldIdxLike("OMEGA");
            rspf_int32 phi      = eoFile.getFieldIdxLike("PHI");
            rspf_int32 kappa    = eoFile.getFieldIdxLike("KAPPA");

            if((omega>=0)&&
               (phi>=0)&&
               (kappa>=0)&&
               (height>=0)&&
               (easting>=0)&&
               (northing>=0))
            {
               theOmega = (*record)[omega].toDouble();
               thePhi   = (*record)[phi].toDouble();
               theKappa = (*record)[kappa].toDouble();
               double h = (*record)[height].toDouble();
               rspfString heightType = kwl.find(prefix, "height_type");
               if(eoFile.isUtmFrame())
               {
                  theUtmZone = eoFile.getUtmZone();
                  theUtmHemisphere = eoFile.getUtmHemisphere()=="North"?'N':'S';
                  rspfUtmProjection utmProj;
                  utmProj.setZone(theUtmZone);
                  utmProj.setHemisphere((char)theUtmHemisphere);
                  theUtmPlatformPosition.x = (*record)[easting].toDouble();
                  theUtmPlatformPosition.y = (*record)[northing].toDouble();
                  theUtmPlatformPosition.z = h;
                  thePlatformPosition = utmProj.inverse(rspfDpt(theUtmPlatformPosition.x,
                                                                 theUtmPlatformPosition.y));
                  thePlatformPosition.height(h);

                  if(eoFile.isHeightAboveMSL())
                  {
                     double offset = rspfGeoidManager::instance()->offsetFromEllipsoid(thePlatformPosition);
                     if(!rspf::isnan(offset))
                     {
                        thePlatformPosition.height(h + offset);
                        theUtmPlatformPosition.z = h + offset;
                     }
                  }
               }
               else
               {
                  return false;
               }
            }
            theEcefPlatformPosition = thePlatformPosition;
         }
         else
         {
            return false;
         }
      }
   }
   if(!loadedFromEoFile)
   {
      if(shift_values)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(shift_values);
         tempString = tempString.trim();
         tempString.split(splitString, " " );
         if(splitString.size() == 3)
         {
            theShiftValues = rspfEcefVector(splitString[0].toDouble(),
                                             splitString[1].toDouble(),
                                             splitString[2].toDouble());
         }
      }
      if(omega&&phi&&kappa)
      {
         theOmega = rspfString(omega).toDouble();
         thePhi   = rspfString(phi).toDouble();
         theKappa = rspfString(kappa).toDouble();
      }
      if(bore_sight_tx&&bore_sight_ty&&bore_sight_tz)
      {
         theBoreSightTx = rspfString(bore_sight_tx).toDouble()/60.0;
         theBoreSightTy = rspfString(bore_sight_ty).toDouble()/60.0;
         theBoreSightTz = rspfString(bore_sight_tz).toDouble()/60.0;
      }
      double lat=0.0, lon=0.0, h=0.0;
      if(utm_zone)
      {
         theUtmZone = rspfString(utm_zone).toInt32();
      }
      if(utm_hemisphere)
      {
         rspfString hem = utm_hemisphere;
         hem = hem.trim();
         hem = hem.upcase();
         theUtmHemisphere = *(hem.begin());
      }
      if(utm_platform_position)
      {
         rspfUtmProjection utmProj;
         std::vector<rspfString> splitString;
         rspfString tempString(utm_platform_position);
         tempString = tempString.trim();
         rspfString datumString;
         utmProj.setZone(theUtmZone);
         utmProj.setHemisphere((char)theUtmHemisphere);
         tempString.split(splitString, " ");
         if(splitString.size() > 2)
         {
            theUtmPlatformPosition.x = splitString[0].toDouble();
            theUtmPlatformPosition.y = splitString[1].toDouble();
            theUtmPlatformPosition.z = splitString[2].toDouble();
         }
         if(splitString.size() > 3)
         {
            datumString = splitString[3];
         }
         const rspfDatum* datum = rspfDatumFactory::instance()->create(datumString);
         if(datum)
         {
            utmProj.setDatum(datum);
         }
         
         thePlatformPosition = utmProj.inverse(rspfDpt(theUtmPlatformPosition.x,
                                                        theUtmPlatformPosition.y));
         thePlatformPosition.height(theUtmPlatformPosition.z);
         
         rspfString heightType = kwl.find(prefix, "height_type");
         if(heightType == "msl")
         {
            double offset = rspfGeoidManager::instance()->offsetFromEllipsoid(thePlatformPosition);
            if(rspf::isnan(offset) == false)
            {
               thePlatformPosition.height(thePlatformPosition.height() + offset);
               theUtmPlatformPosition.z = thePlatformPosition.height();
            }
         }
         theEcefPlatformPosition = thePlatformPosition;
      }
      else if(latlonh_platform_position)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(latlonh_platform_position);
         std::string datumString;
         tempString = tempString.trim();
         tempString.split(splitString, " ");
         if(splitString.size() > 2)
         {
            lat = splitString[0].toDouble();
            lon = splitString[1].toDouble();
            h = splitString[2].toDouble();
         }
         if(splitString.size() > 3)
         {
            datumString = splitString[3].string();
         }
         thePlatformPosition.latd(lat);
         thePlatformPosition.lond(lon);
         thePlatformPosition.height(h);
         const rspfDatum* datum = rspfDatumFactory::instance()->create(datumString);
         if(datum)
         {
            thePlatformPosition.datum(datum);
         }
         rspfString heightType = kwl.find(prefix, "height_type");
         if(heightType == "msl")
         {
            double offset = rspfGeoidManager::instance()->offsetFromEllipsoid(thePlatformPosition);
            if(rspf::isnan(offset) == false)
            {
               thePlatformPosition.height(thePlatformPosition.height() + offset);
            }
         }
         theEcefPlatformPosition = thePlatformPosition;
      }
   }
   if(principal_point)
   {
      std::vector<rspfString> splitString;
      rspfString tempString(principal_point);
      tempString = tempString.trim();
      tempString.split(splitString, " ");
      if(splitString.size() == 2)
      {
         thePrincipalPoint.x = splitString[0].toDouble();
         thePrincipalPoint.y = splitString[1].toDouble();
      }
   }
   if(pixel_size)
   {
      std::vector<rspfString> splitString;
      rspfString tempString(principal_point);
      tempString = tempString.trim();
      tempString.split(splitString, " ");
      if(splitString.size() == 2)
      {
         thePixelSize.x = splitString[0].toDouble();
         thePixelSize.y = splitString[1].toDouble();
      }
   }
   if(focal_length)
   {
      theFocalLength = rspfString(focal_length).toDouble();
   }

   if(camera_file)
   {
      rspfKeywordlist cameraKwl;
      rspfKeywordlist lensKwl;
      cameraKwl.add(camera_file);
      const char* sensor = cameraKwl.find("sensor");
      const char* image_size      = cameraKwl.find(prefix, "image_size");
      principal_point = cameraKwl.find("principal_point");
      focal_length    = cameraKwl.find("focal_length");
      pixel_size      = cameraKwl.find(prefix, "pixel_size");
      focal_length    = cameraKwl.find(prefix, "focal_length");
      const char* distortion_units = cameraKwl.find(prefix, "distortion_units");
      rspfUnitConversionTool tool;
      rspfUnitType unitType = RSPF_MILLIMETERS;

      if(distortion_units)
      {
         unitType = (rspfUnitType)rspfUnitTypeLut::instance()->getEntryNumber(distortion_units);

         if(unitType == RSPF_UNIT_UNKNOWN)
         {
            unitType = RSPF_MILLIMETERS;
         }
      }
      if(image_size)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(image_size);
         tempString = tempString.trim();
         tempString.split(splitString, " ");
         double w=1, h=1;
         if(splitString.size() == 2)
         {
            w = splitString[0].toDouble();
            h = splitString[1].toDouble();
         }
         theImageClipRect = rspfDrect(0,0,w-1,h-1);
         theRefImgPt      = rspfDpt(w/2.0, h/2.0);
      }
      if(sensor)
      {
         theSensorID = sensor;
      }
      if(principal_point)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(principal_point);
         tempString = tempString.trim();
         tempString.split(splitString, " ");
         if(splitString.size() == 2)
         {
            thePrincipalPoint.x = splitString[0].toDouble();
            thePrincipalPoint.y = splitString[1].toDouble();
         }
      }
      if(pixel_size)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(pixel_size);
         tempString = tempString.trim();
         tempString.split(splitString, " ");
         if(splitString.size() == 1)
         {
            thePixelSize.x = splitString[0].toDouble();
            thePixelSize.y = thePixelSize.x;
         }
         else if(splitString.size() == 2)
         {
            thePixelSize.x = splitString[0].toDouble();
            thePixelSize.y = splitString[1].toDouble();
         }
      }
      if(focal_length)
      {
         theFocalLength = rspfString(focal_length).toDouble();
      }

      cameraKwl.trimAllValues();
      
      
      rspfString regExpression =  rspfString("^(") + "d[0-9]+)";
      vector<rspfString> keys;
      cameraKwl.getSubstringKeyList( keys, regExpression );
      long numberOfDistortions = (long)keys.size();
      int offset = (int)rspfString("d").size();
      rspf_uint32 idx = 0;
      std::vector<int> numberList(numberOfDistortions);
      for(idx = 0; idx < (int)numberList.size();++idx)
      {
         rspfString numberStr(keys[idx].begin() + offset,
                               keys[idx].end());
         numberList[idx] = numberStr.toInt();
      }
      std::sort(numberList.begin(), numberList.end());
      double distance=0.0, distortion=0.0;
     
      for(idx = 0; idx < numberList.size(); ++idx)
      {
         rspfString value = cameraKwl.find(rspfString("d")+rspfString::toString(numberList[idx]));
         if(!value.empty())
         {
            std::istringstream inStr(value.c_str());
            inStr >> distance;
            rspf::skipws(inStr);
            inStr >> distortion;
#if 0
            std::vector<rspfString> splitString;
            rspfString tempString(value);
            tempString = tempString.trim();
            tempString.split(splitString, " ");
            std::cout << splitString.size() << std::endl;
            if(splitString.size() >= 2)
            {
               distance = splitString[0].toDouble();
               distortion = splitString[1].toDouble();
            }
#endif
            
            tool.setValue(distortion, unitType);
            lensKwl.add(rspfString("distance") + rspfString::toString(idx),
                        distance,
                        true);
            lensKwl.add(rspfString("distortion") + rspfString::toString(idx),
                        tool.getMillimeters(),
                        true);
         }
         lensKwl.add("convergence_threshold",
                     .00001,
                     true);
         if(pixel_size)
         {
            lensKwl.add("dxdy",
                        rspfString::toString(thePixelSize.x) + " " +rspfString::toString(thePixelSize.y),
                        true);
         }
         else
         {
            lensKwl.add("dxdy",
                        ".009 .009",
                        true);
         }
      }
      if(theLensDistortion.valid())
      {
         theLensDistortion->loadState(lensKwl,"");
      }
   }
   else
   {
      
      if(principal_point)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(principal_point);
         tempString = tempString.trim();
         tempString.split(splitString, " ");
         if(splitString.size() >= 2)
         {
            thePrincipalPoint.x = splitString[0].toDouble();
            thePrincipalPoint.y = splitString[1].toDouble();
         }
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "No principal_point found" << std::endl;
            return false;
         }
      }
      if(pixel_size)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(pixel_size);
         tempString = tempString.trim();
         tempString.split(splitString, " ");
         if(splitString.size() == 1)
         {
            thePixelSize.x = splitString[0].toDouble();
            thePixelSize.y = thePixelSize.x;
         }
         else if(splitString.size() == 2)
         {
            thePixelSize.x = splitString[0].toDouble();
            thePixelSize.y = splitString[1].toDouble();
         }
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "No pixel size found" << std::endl;
            return false;
         }
      }
      if(focal_length)
      {
         theFocalLength = rspfString(focal_length).toDouble();
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "No focal length found" << std::endl;
            return false;
         }
      }
      
      if(theLensDistortion.valid())
      {
         rspfString lensPrefix = rspfString(prefix)+"distortion.";
         theLensDistortion->loadState(kwl,
                                      lensPrefix.c_str());
      }
   }
   theRefGndPt = thePlatformPosition;
   
   updateModel();

   lineSampleToWorld(theRefImgPt, theRefGndPt);
   if(compute_gsd_flag)
   {
      if(rspfString(compute_gsd_flag).toBool())
      {
         rspfGpt right;
         rspfGpt top;
         lineSampleToWorld(theRefImgPt + rspfDpt(1.0, 0.0),
                           right);
         lineSampleToWorld(theRefImgPt + rspfDpt(1.0, 0.0),
                           top);

         rspfEcefVector horizontal = rspfEcefPoint(theRefGndPt)-rspfEcefPoint(right);
         rspfEcefVector vertical   = rspfEcefPoint(theRefGndPt)-rspfEcefPoint(top);

         theGSD.x = horizontal.length();
         theGSD.y = vertical.length();
         theMeanGSD = (theGSD.x+theGSD.y)*.5;
      }
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theOmega:              " << theOmega << std::endl
                                          << "thePhi:                " << thePhi   << std::endl
                                          << "theKappa:              " << theKappa << std::endl;
      std::cout << "platform position:     " << thePlatformPosition << std::endl;
      std::cout << "platform position ECF: " << theEcefPlatformPosition << std::endl;
      std::cout << "rspfApplanixModel::loadState: ......... leaving" << std::endl;
   }

   return true;
}

bool rspfApplanixUtmModel::setupOptimizer(const rspfString& init_file)
{
   rspfKeywordlist kwl;
   kwl.addFile(init_file.c_str());

   return loadState(kwl);
}
