#ifndef rspfSonomaSensor_HEADER
#define rspfSonomaSensor_HEADER
#include "rspfSensorModel.h"
#include "rspfUtmProjection.h"
#include <rspf/base/rspfDpt3d.h>
/*******************************************************************************************
 *
 * The Sonoma system uses an LN200 IMU.  The orientation are euler angles of the form
 * RzRxRz (roll*pitch*heading) where roll and heading are defined about the Rz.  The sonoma 
 * software does their own camera callibration and appears to be a mtrix that goes from IMU 
 * to boresite allignment and for the rspf intersections we need to invert
 * camera mount allignment matrix so we go from camer to IMU boresite allignment.
 *
 * Also talking with some contacts it appears that the intersections are done to a affine hyper plane
 * and the coordinates for the initial ray origin and direction are UTM where the zone and hemishpere 
 * is initialized based on the platfrom position.  Their height values are MSL and so I use 
 * the geoid grid to shift to the ellipsoid.  We have our own intersectRay that intersects a plane
 * plane equation.
 *
 *
 * The load state keywords are:
 * mount:
 * pitch:
 * heading: 
 * platform_position: (0,0, 0,WGE)
 * pixel_size: (1,1)
 * focal_length: .02
 * principal_point: (0,0)
 * rect: 0 0 4007 2671
 * mount: 1.034918 -0.003693 0.309045 0.000000 -0.112617 1.064566 0.362789 0.000000 -0.269597 -0.391096 1.002515 0.000000
 * sensor: Sonoma
 * type: rspfSonomaSensor
 *
 *
 * the mount is a orientation that we invert to go from camera to imu allignment.  The roll pitch and heading are euler
 * angles in degrees.  The pixel size and focal lengths were defined as meters and we kept the units here to be meter
 * units.  The platform position is of the form (Lat, Lon, Height).  We were not given any lens distortion measurments
 * and the mount orientation was calculated by the sonoma product and was not part of the IMU information.  
 *
 *******************************************************************************************/
class RSPF_DLL rspfSonomaSensor : public rspfSensorModel
{
public:
   rspfSonomaSensor();
   rspfSonomaSensor(const rspfSonomaSensor& src)
   :rspfSensorModel(src),
   m_compositeMatrix(src.m_compositeMatrix),
   m_compositeMatrixInverse(src.m_compositeMatrixInverse),
   m_pixelToCamera(src.m_pixelToCamera),
   m_mount(src.m_mount),
   m_roll(src.m_roll),
   m_pitch(src.m_pitch),
   m_heading(src.m_heading),
   m_principalPoint(src.m_principalPoint), // in millimeters
   m_pixelSize(src.m_pixelSize),      // in millimeters
   m_focalLength(src.m_focalLength),    // in millimeters
   m_ecefPlatformPosition(src.m_ecefPlatformPosition),
   m_platformPosition(src.m_platformPosition),
   m_platformPositionEllipsoid(src.m_platformPositionEllipsoid)
   {
   }
   virtual rspfObject* dup()const
   {
      return new rspfSonomaSensor(*this);
   }
   
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   void lineSampleToWorld(const rspfDpt& image_point,
                          rspfGpt&       gpt) const;
   
   virtual void updateModel();
   
   void setRollPitchHeading(double r, double p, double y)
   {
      m_roll  = r;
      m_pitch = p;
      m_heading   = y;
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
   class rspfPlane
   {
   public:
      rspfPlane(double nx=0, double ny=0, double nz=1.0,
                 double offset=1.0)
      :m_nx(nx),
      m_ny(ny),
      m_nz(nz),
      m_offset(offset)
      {
      }
      bool intersect(rspfDpt3d& result, rspfDpt3d& origin, rspfDpt3d& ray)
      {
         
         double numerator   = m_offset + origin.x*m_nx+origin.y*m_ny + origin.z*m_nz;
         double denominator = ray.z*m_nx + ray.y*m_ny + ray.z*m_nz;
         if(rspf::almostEqual(denominator, 0.0))
         {
            return false;
         }
         
         double t = -numerator/denominator;
         result.x = origin.x + t*ray.x;
         result.y = origin.y + t*ray.y;
         result.z = origin.z + t*ray.z;
         
         return true;
      }
      
      void setOffset(double offset)
      {
         m_offset = offset;
      }
   protected:
      double m_nx,m_ny,m_nz;
      double m_offset;
   };
   bool intersectRay(const rspfMapProjection& proj, rspfDpt3d& result, rspfDpt3d& origin, rspfDpt3d& dir)const;
   bool intersectRayWithHeight(const rspfMapProjection& proj, rspfDpt3d& result, rspfDpt3d& origin, rspfDpt3d& dir, double h)const;
   rspfRefPtr<rspfUtmProjection> m_utmProjection;
   NEWMAT::Matrix m_compositeMatrix;
   NEWMAT::Matrix m_compositeMatrixInverse;
   NEWMAT::Matrix m_pixelToCamera;
   NEWMAT::Matrix m_mount;
   double         m_roll;
   double         m_pitch;
   double         m_heading;
   rspfDpt       m_principalPoint; // in meters
   rspfDpt       m_pixelSize;      // in meters
   double         m_focalLength;    // in meters
   rspfEcefPoint m_ecefPlatformPosition;
   rspfGpt       m_platformPosition;
   rspfGpt       m_platformPositionEllipsoid;
   
   
   TYPE_DATA
};
#endif
