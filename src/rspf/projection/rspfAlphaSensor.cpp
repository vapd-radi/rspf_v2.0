//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  Alpha Sensor Base Class
//
//*******************************************************************
//  $Id$
#include <rspf/projection/rspfAlphaSensor.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfLsrRay.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfMatrix4x4.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/matrix/newmatio.h>

static rspfTrace traceExec ("rspfAlphaSensor:exec");
static rspfTrace traceDebug("rspfAlphaSensor:debug");

RTTI_DEF1(rspfAlphaSensor, "rspfAlphaSensor", rspfSensorModel);

enum
{
   PARAM_ADJ_LON_OFFSET   = 0,
   PARAM_ADJ_LAT_OFFSET = 1,
   PARAM_ADJ_ALTITUDE_OFFSET = 2, 
   PARAM_ADJ_ROLL_OFFSET = 3,
   PARAM_ADJ_PITCH_OFFSET = 4,
   PARAM_ADJ_HDG_OFFSET = 5,
   PARAM_ADJ_FOCAL_LENGTH_OFFSET = 6,
   PARAM_ADJ_COUNT = 7
};

rspfAlphaSensor::rspfAlphaSensor()
   :
   m_rollBias(0.0),
   m_pitchBias(0.0),
   m_headingBias(0.0),
   m_fov(0.0),
   m_slitRot(0.0),
   m_focalLength(0.0),
   m_adjustedFocalLength(0.0)
{

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensor::rspfAlphaSensor DEBUG:" << std::endl;
   }
   m_cam2Platform.ReSize(3,3);
   m_cam2Platform = 0.0;
   
   initAdjustableParameters();
}

rspfAlphaSensor::rspfAlphaSensor(const rspfAlphaSensor& src)
   :
   rspfSensorModel(src),
   m_rollBias(src.m_rollBias),
   m_pitchBias(src.m_pitchBias),
   m_headingBias(src.m_headingBias),
   m_fov(src.m_fov),
   m_slitRot(src.m_slitRot),
   m_focalLength(src.m_focalLength),
   m_rollPoly(src.m_rollPoly),
   m_pitchPoly(src.m_pitchPoly),
   m_headingPoly(src.m_headingPoly),
   m_lonPoly(src.m_lonPoly),
   m_latPoly(src.m_latPoly),
   m_altPoly(src.m_altPoly),
   m_scanPoly(src.m_scanPoly),
   m_cam2Platform(src.m_cam2Platform),
   m_adjustedFocalLength(src.m_adjustedFocalLength)
{
}

rspfObject* rspfAlphaSensor::dup()const
{
   return new rspfAlphaSensor(*this);
}

void rspfAlphaSensor::lineSampToWorld(const rspfDpt& imagePoint,
                                             rspfGpt& worldPt) const
{
   rspfEcefRay ray;
   imagingRay(imagePoint, ray);
   rspfElevManager::instance()->intersectRay(ray, worldPt);
}  

void rspfAlphaSensor::lineSampleHeightToWorld(const rspfDpt& imagePoint,
                                               const double&   heightEllipsoid,
                                                     rspfGpt& worldPt) const
{
   rspfEcefRay ray;
   imagingRay(imagePoint, ray);
   rspfEcefPoint pecf(ray.intersectAboveEarthEllipsoid(heightEllipsoid));
   worldPt = rspfGpt(pecf);

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensorHSI::lineSampleHeightToWorld DEBUG:" << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "  imagePoint = " << imagePoint << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "  heightEllipsoid = " << heightEllipsoid << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "  ray = " << ray << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "  worldPt = " << worldPt << std::endl;
   }
}

void rspfAlphaSensor::setFov(const double fov)
{
   m_fov = fov;
   m_focalLength = (theImageSize.x/2)/tan((m_fov/DEG_PER_RAD)/2);
}

void rspfAlphaSensor::setRollBias(const double rollBias)
{
   m_rollBias = rollBias;
}

void rspfAlphaSensor::setPitchBias(const double pitchBias)
{
   m_pitchBias = pitchBias;
}

void rspfAlphaSensor::setHeadingBias(const double headingBias)
{
   m_headingBias = headingBias;
}

void rspfAlphaSensor::setSlitRot(const double slitRot)
{
   m_slitRot = slitRot;
}

void rspfAlphaSensor::setRollPoly(const std::vector< rspf_float64 > rollPoly)
{
   m_rollPoly = rollPoly;
}

void rspfAlphaSensor::setPitchPoly(const std::vector< rspf_float64 > pitchPoly)
{
   m_pitchPoly = pitchPoly;
}

void rspfAlphaSensor::setHeadingPoly(const std::vector< rspf_float64 > headingPoly)
{
   m_headingPoly = headingPoly;
}

void rspfAlphaSensor::setLonPoly(const std::vector< rspf_float64 > lonPoly)
{
   m_lonPoly = lonPoly;
}

void rspfAlphaSensor::setLatPoly(const std::vector< rspf_float64 > latPoly)
{
   m_latPoly = latPoly;
}

void rspfAlphaSensor::setAltPoly(const std::vector< rspf_float64 > altPoly)
{
   m_altPoly = altPoly;
}

void rspfAlphaSensor::setScanPoly(const std::vector< rspf_float64 > scanPoly)
{
   m_scanPoly = scanPoly;
}

void rspfAlphaSensor::getPositionOrientation(const rspf_float64& line,
                                                    rspfEcefPoint& pos,
                                                    NEWMAT::Matrix& cam2EcfRot) const
{
   // Platform position
   pos = getCameraPosition(line);

   // Local platform orientation
   NEWMAT::Matrix platform2Local = getPlatform2LocalRot(line);

   // Form local<-ECF matrix
   rspfGpt posG = rspfGpt(pos);
   NEWMAT::Matrix local2Ecf = formLLAmat(posG.latr(), posG.lonr(), 0.0);

   // Form full ECF<-camera matrix
   cam2EcfRot = local2Ecf.t() * platform2Local * m_cam2Platform;
}

rspfEcefPoint rspfAlphaSensor::getCameraPosition(const rspf_float64& line) const
{
   // Interpolate position at given line number
   rspf_float64 lat = evalPoly(m_latPoly, line);
   rspf_float64 lon = evalPoly(m_lonPoly, line);
   rspf_float64 alt = evalPoly(m_altPoly, line);
   rspfGpt cameraPositionEllipsoid(lat, lon, alt);

   // Update adjusted position
   double deltap = computeParameterOffset(PARAM_ADJ_LAT_OFFSET)/
      cameraPositionEllipsoid.metersPerDegree().y;
   double deltal = computeParameterOffset(PARAM_ADJ_LON_OFFSET)/
      cameraPositionEllipsoid.metersPerDegree().x;
   
   rspfGpt adjustedCameraPosition = rspfGpt(cameraPositionEllipsoid.latd()   + deltap,
                                              cameraPositionEllipsoid.lond()   + deltal,
                                              cameraPositionEllipsoid.height() + computeParameterOffset(PARAM_ADJ_ALTITUDE_OFFSET));

   rspfEcefPoint pos(adjustedCameraPosition);

   return pos;
}

NEWMAT::Matrix rspfAlphaSensor::getPlatform2LocalRot(const rspf_float64& line) const
{
   // Interpolate orientation at given line number
   rspf_float64 roll = evalPoly(m_rollPoly, line);
   rspf_float64 pitch = evalPoly(m_pitchPoly, line);
   rspf_float64 heading = evalPoly(m_headingPoly, line);

   // Form orientation matrix
   NEWMAT::Matrix rmat = formHPRmat(-roll, -pitch, -heading);

   return rmat;
}

rspf_float64 rspfAlphaSensor::evalPoly(const std::vector<rspf_float64>& polyCoef,
                                         const rspf_float64& line) const
{
   int nCoef = polyCoef.size();

   rspf_float64 result = polyCoef[nCoef-1];

   if (nCoef > 1)
   {
      for(int i=nCoef-2; i >= 0 ; --i)
          result = result * line + polyCoef[i];
   }

   return result;
}

void rspfAlphaSensor::updateModel()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensor::updateModel DEBUG:" << std::endl;
   }

   // Apply bias corrections
   double r = rspf::degreesToRadians(m_rollBias    + computeParameterOffset(PARAM_ADJ_ROLL_OFFSET));
   double p = rspf::degreesToRadians(m_pitchBias   + computeParameterOffset(PARAM_ADJ_PITCH_OFFSET));
   double h = rspf::degreesToRadians(m_headingBias + computeParameterOffset(PARAM_ADJ_HDG_OFFSET));
   
   // Form bias rotation matrix
   m_cam2Platform = formHPRmat(r, p, h);
   
   // Apply focal length correction
   m_focalLength = (theImageSize.x/2)/tan((m_fov/DEG_PER_RAD)/2);
   m_adjustedFocalLength = m_focalLength + computeParameterOffset(PARAM_ADJ_FOCAL_LENGTH_OFFSET);
}

void rspfAlphaSensor::initAdjustableParameters()
{
   if (traceExec())
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAlphaSensor::initAdjustableParameters: returning..." << std::endl;
   resizeAdjustableParameterArray(PARAM_ADJ_COUNT);
   
   setAdjustableParameter(PARAM_ADJ_LON_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_LON_OFFSET, "lon_offset");
   setParameterUnit(PARAM_ADJ_LON_OFFSET, "meters");
   setParameterSigma(PARAM_ADJ_LON_OFFSET, 10);
   
   setAdjustableParameter(PARAM_ADJ_LAT_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_LAT_OFFSET, "lat_offset");
   setParameterUnit(PARAM_ADJ_LAT_OFFSET, "meters");
   setParameterSigma(PARAM_ADJ_LAT_OFFSET, 10);
   
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
   
   setAdjustableParameter(PARAM_ADJ_HDG_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_HDG_OFFSET, "heading_offset");
   setParameterUnit(PARAM_ADJ_HDG_OFFSET, "degrees");
   setParameterSigma(PARAM_ADJ_HDG_OFFSET, 5);
   
   setAdjustableParameter(PARAM_ADJ_FOCAL_LENGTH_OFFSET, 0.0);
   setParameterDescription(PARAM_ADJ_FOCAL_LENGTH_OFFSET, "focal_length_offset");
   setParameterUnit(PARAM_ADJ_FOCAL_LENGTH_OFFSET, "pixels");
   setParameterSigma(PARAM_ADJ_FOCAL_LENGTH_OFFSET, 20.0);   
}

bool rspfAlphaSensor::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensor::loadState DEBUG:" << std::endl;
   }

   theGSD.makeNan();
   theRefImgPt.makeNan();
   rspfSensorModel::loadState(kwl, prefix);

   rspfString fov = kwl.find(prefix, "fov");
   rspfString number_samples = kwl.find(prefix, "number_samples");
   rspfString number_lines = kwl.find(prefix, "number_lines");
   rspfString rollBias = kwl.find(prefix, "roll_bias"); 
   rspfString pitchBias = kwl.find(prefix, "pitch_bias"); 
   rspfString headingBias = kwl.find(prefix, "heading_bias"); 
   rspfString slitRot = kwl.find(prefix, "slit_rotation"); 

   rspf_uint32 pcount;
   const char* lookup;

   // Roll polynomial
   m_rollPoly.clear();
   pcount = kwl.numberOf("roll_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "roll_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_rollPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() roll_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }

   // Pitch polynomial
   m_pitchPoly.clear();
   pcount = kwl.numberOf("pitch_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "pitch_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_pitchPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() pitch_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }

   // Heading polynomial
   m_headingPoly.clear();
   pcount = kwl.numberOf("heading_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "heading_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_headingPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() heading_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }

   // Latitude polynomial
   m_latPoly.clear();
   pcount = kwl.numberOf("lat_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "lat_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_latPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() lat_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }

   // Longitude polynomial
   m_lonPoly.clear();
   pcount = kwl.numberOf("lon_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "lon_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_lonPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() lon_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }

   // Altitude polynomial
   m_altPoly.clear();
   pcount = kwl.numberOf("alt_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "alt_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_altPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() alt_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }

   // Scan angle polynomial
   m_scanPoly.clear();
   pcount = kwl.numberOf("scan_poly_coeff");
   if (pcount>0)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
      while ( (found < pcount) && (count < 100) )
      {
         rspfString kw = "scan_poly_coeff";
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            m_scanPoly.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfAlphaSensor::loadState() scan_poly_coeff lookup failure..."
            << std::endl;
      }
      return false;
   }


   if(!number_samples.empty())
   {
      theImageSize = rspfIpt(number_samples.toDouble(), number_lines.toDouble());
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
   if(!fov.empty())
   {
      m_fov = fov.toDouble();
   }
   if(!rollBias.empty())
   {
      m_rollBias = rollBias.toDouble();
   }
   if(!pitchBias.empty())
   {
      m_pitchBias = pitchBias.toDouble();
   }
   if(!headingBias.empty())
   {
      m_headingBias = headingBias.toDouble();
   }
   if(!slitRot.empty())
   {
      m_slitRot = slitRot.toDouble();
   }
   
   updateModel();
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAlphaSensor::loadState complete..." << std::endl;
   }
   
   return true;
}

bool rspfAlphaSensor::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfSensorModel::saveState(kwl, prefix);
   kwl.add(prefix, "roll_bias", rspfString::toString(m_rollBias), true);
   kwl.add(prefix, "pitch_bias", rspfString::toString(m_pitchBias), true);
   kwl.add(prefix, "heading_bias", rspfString::toString(m_headingBias), true);
   kwl.add(prefix, "fov", rspfString::toString(m_fov) ,true);
   kwl.add(prefix, "slit_rotation", rspfString::toString(m_slitRot) ,true);
   kwl.add(prefix, "image_size", theImageSize.toString() ,true);

   rspf_uint32 i;
   for (i = 0; i < m_rollPoly.size(); ++i)
   {
      rspfString kw = "roll_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_rollPoly[i]);
   }
   for (i = 0; i < m_pitchPoly.size(); ++i)
   {
      rspfString kw = "pitch_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_pitchPoly[i]);
   }
   for (i = 0; i < m_headingPoly.size(); ++i)
   {
      rspfString kw = "heading_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_headingPoly[i]);
   }
   for (i = 0; i < m_lonPoly.size(); ++i)
   {
      rspfString kw = "lon_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_lonPoly[i]);
   }
   for (i = 0; i < m_latPoly.size(); ++i)
   {
      rspfString kw = "lat_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_latPoly[i]);
   }
   for (i = 0; i < m_altPoly.size(); ++i)
   {
      rspfString kw = "alt_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_altPoly[i]);
   }
   for (i = 0; i < m_scanPoly.size(); ++i)
   {
      rspfString kw = "scan_poly_coeff";
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, m_scanPoly[i]);
   }

   return true;
}


// Form ARINC 705 standard {heading/pitch/roll} rotation matrix
// Rotates local<-body
NEWMAT::Matrix rspfAlphaSensor::formHPRmat(const rspf_float64& r,
                                            const rspf_float64& p,
                                            const rspf_float64& h)const
{
   rspf_float64 cp = cos(p);
   rspf_float64 sp = sin(p);
   rspf_float64 ch = cos(h);
   rspf_float64 sh = sin(h);
   rspf_float64 cr = cos(r);
   rspf_float64 sr = sin(r);

   NEWMAT::Matrix rollM   = rspfMatrix3x3::create(  1,  0,  0,
                                                     0, cr,-sr,
                                                     0, sr, cr);

   NEWMAT::Matrix pitchM  = rspfMatrix3x3::create( cp,  0, sp,
                                                     0,  1,  0,
                                                   -sp,  0, cp);

   NEWMAT::Matrix hdgM    = rspfMatrix3x3::create( ch,-sh,  0,
                                                    sh, ch,  0,
                                                     0,  0,  1);

   NEWMAT::Matrix body2LocalRot = hdgM * pitchM * rollM;

   return body2LocalRot;
}


// Form local <- ECF rotation matrix
NEWMAT::Matrix rspfAlphaSensor::formLLAmat(const rspf_float64& lat,
                                            const rspf_float64& lon,
                                            const rspf_float64& az)const
{
   rspf_float64 cp = cos(lat);
   rspf_float64 sp = sin(lat);
   rspf_float64 cl = cos(lon);
   rspf_float64 sl = sin(lon);
   rspf_float64 ca = cos(az);
   rspf_float64 sa = sin(az);

   NEWMAT::Matrix ecf2LocalRot(3,3);
   ecf2LocalRot(1,1) = -sl*sa - sp*cl*ca;
   ecf2LocalRot(1,2) =  cl*sa - sp*sl*ca;
   ecf2LocalRot(1,3) =  cp*ca;
   ecf2LocalRot(2,1) =  sl*ca - sp*cl*sa;
   ecf2LocalRot(2,2) = -cl*ca - sp*sl*sa;
   ecf2LocalRot(2,3) =  cp*sa;
   ecf2LocalRot(3,1) =  cp*cl;
   ecf2LocalRot(3,2) =  cp*sl;
   ecf2LocalRot(3,3) =  sp;

   return ecf2LocalRot;
}
