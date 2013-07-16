#ifndef rspfSkyBoxLearSensor_HEADER
#define rspfSkyBoxLearSensor_HEADER
#include "rspfSensorModel.h"
#include "rspfUtmProjection.h"
#include <rspf/base/rspfDpt3d.h>
class RSPF_DLL rspfSkyBoxLearSensor : public rspfSensorModel
{
public:
   rspfSkyBoxLearSensor();
   rspfSkyBoxLearSensor(const rspfSkyBoxLearSensor& src)
   :rspfSensorModel(src),
   m_air2Ned(src.m_air2Ned),
   m_ecef2Ned(src.m_ecef2Ned),
   m_ecef2NedInverse(src.m_ecef2NedInverse),
   m_roll(src.m_roll),
   m_pitch(src.m_pitch),
   m_yaw(src.m_yaw),
   m_principalPoint(src.m_principalPoint), // in meters
   m_pixelSize(src.m_pixelSize),      // in meters
   m_focalLength(src.m_focalLength),    // in meters
   m_ecefPlatformPosition(src.m_ecefPlatformPosition),
   m_platformPosition(src.m_platformPosition),
   m_platformPositionEllipsoid(src.m_platformPositionEllipsoid),
   m_fovIntrack(src.m_fovIntrack),
   m_fovCrossTrack(src.m_fovCrossTrack),
   m_adjustedPlatformPosition(src.m_adjustedPlatformPosition)
   {
   }
   virtual rspfObject* dup()const
   {
      return new rspfSkyBoxLearSensor(*this);
   }
   
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   
   virtual void lineSampeToWorld(const rspfDpt& image_point,
                         rspfGpt&       worldPoint) const;
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                const double&   heightEllipsoid,
                                rspfGpt&       worldPoint) const;
   virtual void updateModel();
   
   void setRollPitchYaw(double r, double p, double y)
   {
      m_roll  = r;
      m_pitch = p;
      m_yaw   = y;
   }
   void setFocalLength(double value)
   {
      m_focalLength = value;
   }
   void setPlatformPosition(const rspfGpt& value)
   {
      m_platformPosition     = value;
      m_ecefPlatformPosition = value;
   }
   
   void setPrincipalPoint(const rspfDpt& value)
   {
      m_principalPoint = value;
   }
   void setPixelSize(const rspfDpt& value)
   {
      m_pixelSize = value;
   }
   inline virtual bool useForward()const {return true;} //!ground to image faster (you don't need DEM) //TBC
   
   virtual void initAdjustableParameters();
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   
protected:
   NEWMAT::Matrix m_air2Ned;
   NEWMAT::Matrix m_ecef2Ned;
   NEWMAT::Matrix m_ecef2NedInverse;
   double         m_roll;
   double         m_pitch;
   double         m_yaw;
   rspfDpt       m_principalPoint; // in meters
   rspfDpt       m_pixelSize;      // in meters
   double         m_focalLength;    // in meters
   rspfEcefPoint m_ecefPlatformPosition;
   rspfGpt       m_platformPosition;
   rspfGpt       m_platformPositionEllipsoid;
   double         m_fovIntrack;
   double         m_fovCrossTrack;
   
   rspfGpt       m_adjustedPlatformPosition;
   TYPE_DATA
};
#endif
