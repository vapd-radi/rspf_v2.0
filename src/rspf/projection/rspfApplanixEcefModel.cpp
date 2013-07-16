//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfApplanixEcefModel.cpp 20485 2012-01-23 18:22:38Z gpotts $
#include <sstream>
#include <rspf/projection/rspfApplanixEcefModel.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/support_data/rspfApplanixEOFile.h>
#include <rspf/base/rspfMatrix4x4.h>
#include <rspf/elevation/rspfElevManager.h>
static rspfTrace traceDebug("rspfApplanixEcefModel:debug");

RTTI_DEF1(rspfApplanixEcefModel, "rspfApplanixEcefModel", rspfSensorModel);

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfApplanixEcefModel.cpp 20485 2012-01-23 18:22:38Z gpotts $";
#endif

rspfApplanixEcefModel::rspfApplanixEcefModel()
{
   theCompositeMatrix          = rspfMatrix4x4::createIdentity();
   theCompositeMatrixInverse   = rspfMatrix4x4::createIdentity();
   theRoll                     = 0.0;
   thePitch                    = 0.0;
   theHeading                  = 0.0;
   theFocalLength              = 55.0;
   thePixelSize = rspfDpt(.009, .009);
   theEcefPlatformPosition = rspfGpt(0.0,0.0, 1000.0);
   theAdjEcefPlatformPosition = rspfGpt(0.0,0.0, 1000.0);
   theGSD.x = 0.1524;
   theGSD.y = 0.1524;
   theMeanGSD = 0.1524;
   theLensDistortion = new rspfMeanRadialLensDistortion;
   initAdjustableParameters();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfApplanixEcefModel::rspfApplanixEcefModel DEBUG:" << endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)<< "RSPF_ID:  " << RSPF_ID << endl;
#endif
   }
}
rspfApplanixEcefModel::rspfApplanixEcefModel(const rspfDrect& imageRect,
                                               const rspfGpt& platformPosition,
                                               double roll,
                                               double pitch,
                                               double heading,
                                               const rspfDpt& /* principalPoint */, // in millimeters
                                               double focalLength, // in millimeters
                                               const rspfDpt& pixelSize) // in millimeters
{
   theImageClipRect = imageRect;
   theRefImgPt      = theImageClipRect.midPoint();
   theCompositeMatrix          = rspfMatrix4x4::createIdentity();
   theCompositeMatrixInverse   = rspfMatrix4x4::createIdentity();
   theRoll                     = roll;
   thePitch                    = pitch;
   theHeading                  = heading;
   theFocalLength              = focalLength;
   thePixelSize                = pixelSize;
   theEcefPlatformPosition     = platformPosition;
   theAdjEcefPlatformPosition  = platformPosition;
   theLensDistortion           = new rspfMeanRadialLensDistortion;
   initAdjustableParameters();
   updateModel();

   try
   {
      // Method throws rspfException.
      computeGsd();
   }
   catch (const rspfException& e)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfApplanixEcefModel Constructor caught Exception:\n"
         << e.what() << std::endl;
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "rspfApplanixEcefModel::rspfApplanixEcefModel DEBUG:" << endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)<< "RSPF_ID:  " << RSPF_ID << endl;
#endif
   }
}

rspfApplanixEcefModel::rspfApplanixEcefModel(const rspfApplanixEcefModel& src)
   :rspfSensorModel(src)
{
   initAdjustableParameters();
   
   if(src.theLensDistortion.valid())
   {
      theLensDistortion = new rspfMeanRadialLensDistortion(*(src.theLensDistortion.get()));
   }
   else
   {
      theLensDistortion = new rspfMeanRadialLensDistortion();
   }
}

rspfObject* rspfApplanixEcefModel::dup()const
{
   return new rspfApplanixEcefModel(*this);
}

void rspfApplanixEcefModel::imagingRay(const rspfDpt& image_point,
                                    rspfEcefRay&   image_ray) const
{
//    if(traceDebug())
//    {
//       rspfNotify(rspfNotifyLevel_DEBUG) << "rspfApplanixEcefModel::imagingRay: ..... entered" << std::endl;
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
//       rspfNotify(rspfNotifyLevel_DEBUG) << "rspfApplanixEcefModel::imagingRay: ..... leaving" << std::endl;
//    }
}

void rspfApplanixEcefModel::lineSampleToWorld(const rspfDpt& image_point,
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
//   if (!insideImage(image_point))
//   {
//      gpt.makeNan();
//       gpt = extrapolate(image_point);
//      return;
//   }

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

void rspfApplanixEcefModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                                 const double&   heightEllipsoid,
                                                 rspfGpt&       worldPoint) const
{
   if (!insideImage(image_point))
   {
      worldPoint.makeNan();
//       worldPoint = extrapolate(image_point, heightEllipsoid);
   }
   else
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

void rspfApplanixEcefModel::worldToLineSample(const rspfGpt& world_point,
                                           rspfDpt&       image_point) const
{
   if((theBoundGndPolygon.getNumberOfVertices() > 0)&&
      (!theBoundGndPolygon.hasNans()))
   {
      if (!(theBoundGndPolygon.pointWithin(world_point)))
      {
//         image_point.makeNan();
//          image_point = extrapolate(world_point);
//         return;
      }         
   }
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

void rspfApplanixEcefModel::updateModel()
{
   rspfGpt gpt;
   rspfGpt wgs84Pt;
   double metersPerDegree = wgs84Pt.metersPerDegree().x;
   double degreePerMeter = 1.0/metersPerDegree;
   double latShift = -computeParameterOffset(1)*theMeanGSD*degreePerMeter;
   double lonShift = computeParameterOffset(0)*theMeanGSD*degreePerMeter;

   gpt = theEcefPlatformPosition;
   double height = gpt.height();
   gpt.height(height + computeParameterOffset(5));
   gpt.latd(gpt.latd() + latShift);
   gpt.lond(gpt.lond() + lonShift);
   theAdjEcefPlatformPosition = gpt;
   rspfLsrSpace lsrSpace(theAdjEcefPlatformPosition, theHeading+computeParameterOffset(4));

   // make a left handed roational matrix;
   rspfMatrix4x4 lsrMatrix(lsrSpace.lsrToEcefRotMatrix());
   NEWMAT::Matrix orientation = (rspfMatrix4x4::createRotationXMatrix(thePitch+computeParameterOffset(3), RSPF_LEFT_HANDED)*
                                 rspfMatrix4x4::createRotationYMatrix(theRoll+computeParameterOffset(2), RSPF_LEFT_HANDED));
   theCompositeMatrix        = (lsrMatrix.getData()*orientation);
   theCompositeMatrixInverse = theCompositeMatrix.i();

   theBoundGndPolygon.resize(4);
   // rspf_float64 w = theImageClipRect.width()*2.0;
   // rspf_float64 h = theImageClipRect.height()*2.0;
   theExtrapolateImageFlag = false;
   theExtrapolateGroundFlag = false;

   lineSampleToWorld(theImageClipRect.ul(),gpt);//+rspfDpt(-w, -h), gpt);
   theBoundGndPolygon[0] = gpt;
   lineSampleToWorld(theImageClipRect.ur(),gpt);//+rspfDpt(w, -h), gpt);
   theBoundGndPolygon[1] = gpt;
   lineSampleToWorld(theImageClipRect.lr(),gpt);//+rspfDpt(w, h), gpt);
   theBoundGndPolygon[2] = gpt;
   lineSampleToWorld(theImageClipRect.ll(),gpt);//+rspfDpt(-w, h), gpt);
   theBoundGndPolygon[3] = gpt;
}

void rspfApplanixEcefModel::initAdjustableParameters()
{
   
   resizeAdjustableParameterArray(6);
   
   setAdjustableParameter(0, 0.0);
   setParameterDescription(0, "x_offset");
   setParameterUnit(0, "pixels");

   setAdjustableParameter(1, 0.0);
   setParameterDescription(1, "y_offset");
   setParameterUnit(1, "pixels");

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
   setParameterDescription(5, "altitude");
   setParameterUnit(5, "meters");
   
   
   setParameterSigma(0, 20.0);
   setParameterSigma(1, 20.0);
   setParameterSigma(2, .1);
   setParameterSigma(3, .1);
   setParameterSigma(4, .1);
   setParameterSigma(5, 50);
}

void rspfApplanixEcefModel::setPrincipalPoint(rspfDpt principalPoint)
{
   thePrincipalPoint = principalPoint;
}

void rspfApplanixEcefModel::setRollPitchHeading(double roll,
                                                 double pitch,
                                                 double heading)
{
   theRoll    = roll;
   thePitch   = pitch;
   theHeading = heading;
   
   updateModel();
}

void rspfApplanixEcefModel::setPixelSize(const rspfDpt& pixelSize)
{
   thePixelSize = pixelSize;
}

void rspfApplanixEcefModel::setImageRect(const rspfDrect& rect)
{
   theImageClipRect = rect;
   theRefImgPt = rect.midPoint();
}

void rspfApplanixEcefModel::setFocalLength(double focalLength)
{
   theFocalLength = focalLength;
}

void rspfApplanixEcefModel::setPlatformPosition(const rspfGpt& gpt)
{
   theRefGndPt            = gpt;
   theEcefPlatformPosition = gpt;
   updateModel();
   
}

bool rspfApplanixEcefModel::saveState(rspfKeywordlist& kwl,
                                   const char* prefix) const
{
   rspfSensorModel::saveState(kwl, prefix);
   
   kwl.add(prefix, "type", "rspfApplanixEcefModel", true);

   kwl.add(prefix, "roll", theRoll, true);
   kwl.add(prefix, "pitch", thePitch, true);
   kwl.add(prefix, "heading", theHeading, true);
   kwl.add(prefix, "principal_point", rspfString::toString(thePrincipalPoint.x) + " " + rspfString::toString(thePrincipalPoint.y));
   kwl.add(prefix, "pixel_size",      rspfString::toString(thePixelSize.x) + " " + rspfString::toString(thePixelSize.y));
   kwl.add(prefix, "focal_length", theFocalLength);
   kwl.add(prefix, "ecef_platform_position",
           rspfString::toString(theEcefPlatformPosition.x()) + " " +
           rspfString::toString(theEcefPlatformPosition.y()) + " " +
           rspfString::toString(theEcefPlatformPosition.z()));

   if(theLensDistortion.valid())
   {
      rspfString lensPrefix = rspfString(prefix)+"distortion.";
      theLensDistortion->saveState(kwl,
                                   lensPrefix.c_str());
   }
   
   return true;
}

bool rspfApplanixEcefModel::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   if(traceDebug())
   {
      std::cout << "rspfApplanixEcefModel::loadState: ......... entered" << std::endl;
   }

   theImageClipRect = rspfDrect(0,0,4076,4091);
   theRefImgPt      = rspfDpt(2046.0, 2038.5);

   rspfSensorModel::loadState(kwl, prefix);
   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }
   theEcefPlatformPosition    = rspfGpt(0.0,0.0,1000.0);
   theAdjEcefPlatformPosition = rspfGpt(0.0,0.0,1000.0);
   theRoll    = 0.0;
   thePitch   = 0.0;
   theHeading = 0.0;
   // bool computeGsdFlag = false;
   const char* roll              = kwl.find(prefix, "roll");
   const char* pitch             = kwl.find(prefix, "pitch");
   const char* heading           = kwl.find(prefix, "heading");
   const char* principal_point   = kwl.find(prefix, "principal_point");
   const char* pixel_size        = kwl.find(prefix, "pixel_size");
   const char* focal_length      = kwl.find(prefix, "focal_length");
   const char* ecef_platform_position = kwl.find(prefix, "ecef_platform_position");
   const char* latlonh_platform_position = kwl.find(prefix, "latlonh_platform_position");
   const char* compute_gsd_flag  = kwl.find(prefix, "compute_gsd_flag");
   const char* eo_file           = kwl.find(prefix, "eo_file");
   const char* camera_file       = kwl.find(prefix, "camera_file");
   const char* eo_id             = kwl.find(prefix, "eo_id");
   bool result = true;
   if(eo_id)
   {
      theImageID = eo_id;
   }
   if(eo_file)
   {
      rspfApplanixEOFile eoFile;
      if(eoFile.parseFile(rspfFilename(eo_file)))
      {
         rspfRefPtr<rspfApplanixEORecord> record = eoFile.getRecordGivenId(theImageID);
         if(record.valid())
         {
            rspf_int32 rollIdx    = eoFile.getFieldIdx("ROLL");
            rspf_int32 pitchIdx   = eoFile.getFieldIdx("PITCH");
            rspf_int32 headingIdx = eoFile.getFieldIdx("HEADING");
            rspf_int32 xIdx       = eoFile.getFieldIdx("X");
            rspf_int32 yIdx       = eoFile.getFieldIdx("Y");
            rspf_int32 zIdx       = eoFile.getFieldIdx("Z");

            if((rollIdx >= 0)&&
               (pitchIdx >= 0)&&
               (headingIdx >= 0)&&
               (xIdx >= 0)&&
               (yIdx >= 0)&&
               (zIdx >= 0))
            {
               theRoll    = (*record)[rollIdx].toDouble();
               thePitch   = (*record)[pitchIdx].toDouble();
               theHeading = (*record)[headingIdx].toDouble();
               theEcefPlatformPosition = rspfEcefPoint((*record)[xIdx].toDouble(),
                                                        (*record)[yIdx].toDouble(),
                                                        (*record)[zIdx].toDouble());
               theAdjEcefPlatformPosition = theEcefPlatformPosition;
            }
            else
            {
               return false;
            }
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN) << "rspfApplanixEcefModel::loadState()  Image id " << theImageID << " not found in eo file " << eo_file << std::endl;
            
            return false;
         }
      }
      else
      {
         return false;
      }
      // computeGsdFlag = true;
   }
   else
   {
      if(roll)
      {
         theRoll = rspfString(roll).toDouble();
      }
      if(pitch)
      {
         thePitch = rspfString(pitch).toDouble();
      }
      if(heading)
      {
         theHeading = rspfString(heading).toDouble();
      }
      if(ecef_platform_position)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(ecef_platform_position);
         tempString.split(splitString, rspfString(" "));
         if(splitString.size() > 2)
         {
            theEcefPlatformPosition  = rspfEcefPoint(splitString[0].toDouble(),
                                                      splitString[1].toDouble(),
                                                      splitString[2].toDouble());
         }
      }
      else if(latlonh_platform_position)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(latlonh_platform_position);
         tempString.split(splitString, rspfString(" "));
         std::string datumString;
         double lat=0.0, lon=0.0, h=0.0;
         if(splitString.size() > 2)
         {
            lat = splitString[0].toDouble();
            lon = splitString[1].toDouble();
            h = splitString[2].toDouble();
         }
         
         theEcefPlatformPosition = rspfGpt(lat,lon,h);
      }
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
         tempString.split(splitString, rspfString(" "));
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
         tempString.split(splitString, rspfString(" "));
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
         tempString.split(splitString, rspfString(" "));
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
                        rspfString(pixel_size) + " " + rspfString(pixel_size),
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
         tempString.split(splitString, rspfString(" "));
         if(splitString.size() == 2)
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
            result = false;
         }
      }
      if(pixel_size)
      {
         std::vector<rspfString> splitString;
         rspfString tempString(pixel_size);
         tempString.split(splitString, rspfString(" "));
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
            result = false;
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
            result = false;
         }
      }
      
      if(theLensDistortion.valid())
      {
         rspfString lensPrefix = rspfString(prefix)+"distortion.";
         if(!theLensDistortion->loadState(kwl,
                                          lensPrefix.c_str()))
         {
            result = false;
         }
      }
   }
   theRefGndPt = theEcefPlatformPosition;
   theRefGndPt.height(0.0);

   updateModel();

   if(compute_gsd_flag)
   {
      if(rspfString(compute_gsd_flag).toBool())
      {
         try
         {
            //---
            // This will set theGSD and theMeanGSD. Method throws
            // rspfException.
            //---
            computeGsd();
         }
         catch (const rspfException& e)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfApplanixEcefModel::loadState Caught Exception:\n"
               << e.what() << std::endl;
         }
      }
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << std::setprecision(15) << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "roll:     " << theRoll << std::endl
                                          << "pitch:    " << thePitch << std::endl
                                          << "heading:  " << theHeading << std::endl
                                          << "platform: " << theEcefPlatformPosition << std::endl
                                          << "focal len: " << theFocalLength << std::endl
                                          << "principal: " << thePrincipalPoint << std::endl
                                          << "Ground:    " << rspfGpt(theEcefPlatformPosition) << std::endl;
   }
   return result;
}

bool rspfApplanixEcefModel::setupOptimizer(const rspfString& init_file)
{
   rspfKeywordlist kwl;
   kwl.addFile(init_file.c_str());

   return loadState(kwl);
}
