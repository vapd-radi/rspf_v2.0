///*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//   Contains implementation of class rspfEllipsoid. The implementation is
//   actually for an OBLATE SPHEROID (x.radius = y.radius) as Earth is
//   considered.
//
// SOFTWARE HISTORY:
//>
//   06Aug2001  Garrett Potts, Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfEllipsoid.cpp 21164 2012-06-24 20:17:58Z dhicks $

#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfMatrix4x4.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfEllipsoid:exec");
static rspfTrace traceDebug ("rspfEllipsoid:debug");


//*****************************************************************************
//  CONSTRUCTOR: rspfEllipsoid #1 (COPY)
//  
//*****************************************************************************
rspfEllipsoid::rspfEllipsoid(const rspfEllipsoid &ellipsoid)
   :
      theName(ellipsoid.theName),
      theCode(ellipsoid.theCode),
      theEpsgCode(ellipsoid.theEpsgCode),
      theA(ellipsoid.theA),
      theB(ellipsoid.theB),
      theFlattening(ellipsoid.theFlattening),
      theA_squared(ellipsoid.theA_squared),
      theB_squared(ellipsoid.theB_squared),
      theEccentricitySquared(ellipsoid.theEccentricitySquared)
{
   if ( theEpsgCode == 0 )
   {
      theEpsgCode = rspfEllipsoidFactory::instance()->findEpsgCode(theCode);
   }
}

//*****************************************************************************
//  CONSTRUCTOR: rspfEllipsoid #2
//  
//*****************************************************************************
rspfEllipsoid::rspfEllipsoid(const rspfString &name,
                               const rspfString &code,
                               const double &a,
                               const double &b,
                               rspf_uint32 epsg_code)
   :
      theName(name),
      theCode(code),
      theEpsgCode(epsg_code),
      theA(a),
      theB(b),
      theA_squared(a*a),
      theB_squared(b*b)
{
   if (theEpsgCode == 0)
   theEpsgCode = rspfEllipsoidFactory::instance()->findEpsgCode(theCode);

   computeFlattening();   
   theEccentricitySquared = 2*theFlattening - theFlattening*theFlattening;
}

rspfEllipsoid::rspfEllipsoid()
{
   const rspfEllipsoid* ellipse = rspfEllipsoidFactory::instance()->wgs84();
   
   *this = *ellipse;
}

//*****************************************************************************
//  CONSTRUCTOR: rspfEllipsoid #3
//  
//*****************************************************************************
rspfEllipsoid::rspfEllipsoid(const double &a,
                               const double &b)
   :
      theName(""), // initialize to empty
      theCode(""),
      theEpsgCode(0),
      theA(a),
      theB(b),
      theA_squared(a*a),
      theB_squared(b*b)
{
   // First check if this is just WGS84:
   const rspfEllipsoid* wgs84 = rspfEllipsoidFactory::instance()->wgs84();
   if ((theA == wgs84->theA) && (theB == wgs84->theB))
   {
      *this = *wgs84;
   }
   else
   {
      computeFlattening();
      theEccentricitySquared = 2*theFlattening - theFlattening*theFlattening;
   }
}

//*****************************************************************************
//  METHOD: rspfEllipsoid::nearestIntersection
//  
//*****************************************************************************
bool rspfEllipsoid::nearestIntersection(const rspfEcefRay &ray,
                                         rspfEcefPoint& rtnPt) const
{
   return nearestIntersection(ray, 0.0, rtnPt);
}

//*****************************************************************************
//  METHOD: rspfEllipsoid::nearestIntersection
//  
//   geographic objects that are derive this class will asssume that
//   the reference datum is wgs84 and that the ray origin is a
//   geocentric coordinate relative to the wgs84 datum.  Will return
//   true if the object was intersected and false otherwise.
//  
//   The nearest intersection will use the ray sphere intersection
//   found in most ray tracers.  We will take a Ray defined by the
//   parametric equation:
//  
//     x = x0 + dxt
//     y = y0 + dyt
//     z = z0 + dzt
//  
//   and intersect this with the equation of a spheroid:
//  
//     x^2/theXRadius^2 + y^2/theYRadius^2 + z^2/theZRadius^2 = 1
//  
//  
//   the intersection is achived by substituting the parametric line
//   into the equation of the sphereroid.  By doing this you should
//   get a quadratic in t and the equation should look like this:
//  
//    a*t^2 + b*t + c = 0
//  
//      let a = dx^2/theXRadius^2 + dy^2/theYRadius^2 + dz^2/theZRadius^2
//      let b = 2*(x0*dx/theXRadius^2 +y0*dy/theYRadius^2 + z0*dz/theZRadius^2
//      let c = x0^2/theXRadius^2 + y0^2/theYRadius^2 + z0^2/theZRadius^2 - 1
//  
//  
//    Now solve the quadratic (-b +- sqrt(b^2 - 4ac) ) / 2a
//  
//    After solving for t, the parameter is applied to the ray to determine
//    the 3D point position in X,Y,Z, passed back in rtnPt. The boolean
//    "true" is returned if an intersection was found.
//
//*****************************************************************************
bool rspfEllipsoid::nearestIntersection(const rspfEcefRay& ray,
                                         const double&       offset,
                                         rspfEcefPoint&     rtnPt) const
{
   static const char MODULE[] = "rspfEllipsoid::nearestIntersection";
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG: " << MODULE << ", entering...\n";

   
   double A_squared = (theA + offset)*(theA + offset);
   double B_squared = (theB + offset)*(theB + offset);

//    double adjustedOffset = offset/theA;
//    double bOverA = theB/theA;
//    double A_squared = (1.0+adjustedOffset)*(1.0+adjustedOffset);
//    double B_squared = ((bOverA+adjustedOffset)*(bOverA+adjustedOffset));//(theB + offset)*(theB + offset);
   
   //***
   // get the origin and direction of ray:
   //***
   rspfEcefPoint  start = ray.origin();   
   rspfEcefVector direction = ray.direction();
//    start = rspfEcefPoint(start.x()/theA,
//                           start.y()/theA,
//                           start.z()/theA);
   //***
   // Solve the coefficents of the quadratic formula
   //***
   double a = ((direction.x() * direction.x())/A_squared) +
              ((direction.y() * direction.y())/A_squared) +
              ((direction.z() * direction.z())/B_squared);

   double b = 2.0*( ((start.x()*direction.x())/A_squared) +
                    ((start.y()*direction.y())/A_squared) +
                    ((start.z()*direction.z())/B_squared) );

   double c = ((start.x()*start.x())/A_squared) +
              ((start.y()*start.y())/A_squared) +
              ((start.z()*start.z())/B_squared) - 1.0;
   
   //***
   // solve the quadratic
   //***
   double root = b*b - 4*a*c;
   double t;
   if(root < 0.0)
   {
      return false;
   }
   else
   {
      double squareRoot = sqrt(root);
      double t1 = (-b + squareRoot ) / (2.0*a);
      double t2 = (-b - squareRoot ) / (2.0*a);

      //***
      // sort t1 and t2 and take the nearest intersection if they
      // are in front of the ray.
      //***
      if(t2 < t1)
      {
         double temp = t1;
         t1 = t2;
         t2 = temp;
      }     

       if(t1 > 0.0)
          t = t1;
       else
          t = t2;
//      t = t1;
   }

   //***
   // Now apply solved t to ray to extrapolate correct distance to intersection
   //***
//    bool rtnval = false;
//    if (t >= 0)
//    {
//       rtnval = true;
//       rtnPt  = ray.extend(t); 
// //       rtnPt  = ray.extend(t*theA); 
//    }
      
   bool rtnval = true;
   rtnPt  = ray.extend(t); 

   return rtnval; 
}


//*****************************************************************************
//  METHOD: rspfEllipsoid::evaluate(rspfColumnVector3d)
//  
//  Returns neg number if inside, 0 if on, and pos number if outside of
//  ellipsoid.
//  
//*****************************************************************************
double rspfEllipsoid::evaluate(const rspfEcefPoint &location)const
{
   //***
   // get the axis
   //***
   return (((location.x() * location.x())/theA_squared) +
           ((location.y() * location.y())/theA_squared) +
           ((location.z() * location.z())/theB_squared) - 1.0);   
}
 
//*****************************************************************************
//  METHOD: rspfEllipsoid::gradient()  version A
//  
//  Returns vector normal to the ellipsoid at point specified.
//  
//*****************************************************************************
void rspfEllipsoid::gradient(const rspfEcefPoint& location,
                              rspfEcefVector&      result) const
{
   result.x() = (2.0*location.x())/theA_squared;
   result.y() = (2.0*location.y())/theA_squared;
   result.z() = (2.0*location.z())/theB_squared;
}

//*****************************************************************************
//  METHOD: rspfEllipsoid::gradient()  version B
//  
//  Returns vector normal to the ellipsoid at point specified.
//  
//*****************************************************************************
rspfEcefVector
rspfEllipsoid::gradient(const rspfEcefPoint &location)const
{
   rspfEcefVector result;
   gradient(location, result);
   return result;
}
   
   
bool rspfEllipsoid::loadState(const rspfKeywordlist& kwl,
                               const char* prefix)
{
   const char* lookup = kwl.find(prefix, rspfKeywordNames::ELLIPSE_CODE_KW);
   bool foundCode = false;
   if(lookup)
   {
      const rspfEllipsoid* ellipse = rspfEllipsoidFactory::instance()->create(rspfString(lookup));

      if(ellipse)
      {
         foundCode = true;
         *this = *ellipse;
      }
   }

   lookup = kwl.find(prefix, rspfKeywordNames::ELLIPSE_EPSG_CODE_KW);
   if (lookup)
   {
      theEpsgCode = rspfString(lookup).toUInt32();
   }

   if(!foundCode)
   {     
      const char* majorAxis = kwl.find(prefix,
                                       rspfKeywordNames::MAJOR_AXIS_KW);
      const char* minorAxis = kwl.find(prefix,
                                       rspfKeywordNames::MAJOR_AXIS_KW);

      theName = "";
      theCode = "";
      if(majorAxis && minorAxis)
      {
         theA = rspfString(majorAxis).toDouble();
         theB = rspfString(minorAxis).toDouble();

         computeFlattening();
         theA_squared = theA*theA;
         theB_squared = theB*theB;
      }
      else
      {
         const rspfEllipsoid* ellipse = rspfEllipsoidFactory::instance()->wgs84();
         
         *this = *ellipse;
      }      
   }

   return true;
}

bool rspfEllipsoid::saveState(rspfKeywordlist& kwl,
                               const char* prefix)const
{
   if(theCode != "")
   {
      kwl.add(prefix,
              rspfKeywordNames::ELLIPSE_CODE_KW,
              theCode.c_str(),
              true);

      kwl.add(prefix,
              rspfKeywordNames::ELLIPSE_NAME_KW,
              theName.c_str(),
              true);
   }
   if (theEpsgCode)
   {
      kwl.add(prefix, rspfKeywordNames::ELLIPSE_EPSG_CODE_KW, theEpsgCode, true);
   }

   kwl.add(prefix,
           rspfKeywordNames::MAJOR_AXIS_KW,
           theA,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::MINOR_AXIS_KW,
           theB,
           true);

   return true;
}

 
//*****************************************************************************
//  METHOD: rspfEllipsoid::prinRadiiOfCurv()
//  
//  Computes the meridional radius and prime vertical at given point.
//  
//*****************************************************************************
void rspfEllipsoid::prinRadiiOfCurv(const rspfEcefPoint& location,
                                           double& merRadius,
                                           double& primeVert) const
{
   double lat, lon, hgt;
   XYZToLatLonHeight(location.x(), location.y(), location.z(), lat, lon, hgt);
   
   double sinPhi = sin(lat*RAD_PER_DEG);
   double phiFac = 1.0 - theEccentricitySquared*sinPhi*sinPhi;
   primeVert = theA / sqrt(phiFac);
   merRadius = theA*(1.0-theEccentricitySquared) / sqrt(phiFac*phiFac*phiFac);
}

 
//*****************************************************************************
//  METHOD: rspfEllipsoid::jacobianWrtEcef()
//  
//  Forms Jacobian of partials of geodetic WRT ECF at given point.
//           -                           -
//           | pLat/pX  pLat/pY  pLat/pZ |
//    jMat = | pLon/pX  pLon/pY  pLon/pZ |
//           | pHgt/pX  pHgt/pY  pHgt/pZ |
//           -                           -
//  
//*****************************************************************************
void rspfEllipsoid::jacobianWrtEcef(const rspfEcefPoint& location,
                                           NEWMAT::Matrix& jMat) const
{
   double primeVert;
   double merRadius;
   double lat, lon, hgt;
   
   XYZToLatLonHeight(location.x(), location.y(), location.z(), lat, lon, hgt);
   prinRadiiOfCurv(location, merRadius, primeVert);
   
   double sinPhi = sin(lat*RAD_PER_DEG);
   double cosPhi = cos(lat*RAD_PER_DEG);
   double sinLam = sin(lon*RAD_PER_DEG);
   double cosLam = cos(lon*RAD_PER_DEG);
   double N_plus_h = primeVert + hgt;
   double M_plus_h = merRadius + hgt;
   
   jMat(1,1) = -sinPhi * cosLam / M_plus_h;
   jMat(2,1) = -sinLam / (cosPhi * N_plus_h);
   jMat(3,1) = cosPhi * cosLam;
   jMat(1,2) = -sinPhi * sinLam / M_plus_h;
   jMat(2,2) =  cosLam / (cosPhi * N_plus_h);
   jMat(3,2) = cosPhi * sinLam;
   jMat(1,3) = cosPhi / M_plus_h;
   jMat(2,3) = 0.0;
   jMat(3,3) = sinPhi;
}

 
//*****************************************************************************
//  METHOD: rspfEllipsoid::jacobianWrtGeo()
//  
//  Forms Jacobian of partials of ECF WRT geodetic at given point.
//           -                           -
//           | pX/pLat  pX/pLon  pX/pHgt |
//    jMat = | pY/pLat  pY/pLon  pY/pHgt |
//           | pZ/pLat  pZ/pLon  pZ/pHgt |
//           -                           -
//  
//*****************************************************************************
void rspfEllipsoid::jacobianWrtGeo(const rspfEcefPoint& location,
                                          NEWMAT::Matrix& jMat) const
{
   double primeVert;
   double merRadius;
   double lat, lon, hgt;
   
   XYZToLatLonHeight(location.x(), location.y(), location.z(), lat, lon, hgt);
   prinRadiiOfCurv(location, merRadius, primeVert);
   
   double sinPhi = sin(lat*RAD_PER_DEG);
   double cosPhi = cos(lat*RAD_PER_DEG);
   double sinLam = sin(lon*RAD_PER_DEG);
   double cosLam = cos(lon*RAD_PER_DEG);
   double N_plus_h = primeVert + hgt;
   double M_plus_h = merRadius + hgt;
   
   jMat(1,1) = -M_plus_h * sinPhi * cosLam;
   jMat(2,1) = -M_plus_h * sinPhi * sinLam;
   jMat(3,1) =  M_plus_h * cosPhi;
   jMat(1,2) = -N_plus_h * cosPhi * sinLam;
   jMat(2,2) =  N_plus_h * cosPhi * cosLam;
   jMat(3,2) = 0.0;
   jMat(1,3) = cosPhi * cosLam;
   jMat(2,3) = cosPhi * sinLam;
   jMat(3,3) = sinPhi;
}


//*****************************************************************************
//  METHOD: rspfEllipsoid::geodeticRadius()
//  
//  Computes the "geodetic" radius for a given latitude in degrees
//  
//*****************************************************************************
double rspfEllipsoid::geodeticRadius(const double& lat) const
{
   double cos_lat = rspf::cosd(lat);
   double sin_lat = rspf::sind(lat);
   double cos2_lat = cos_lat*cos_lat;
   double sin2_lat = sin_lat*sin_lat;
   double a2_cos = theA_squared*cos_lat;
   double b2_sin = theB_squared*sin_lat;
   
   return sqrt( ( (a2_cos*a2_cos) + (b2_sin*b2_sin) )/ (theA_squared*cos2_lat + theB_squared*sin2_lat));
}

//*************************************************************************************************
//  Computes the "geodetic" radius of curvature of the ellipsoid in the east-west (x) and
//  north-south (y) directions for a given latitude in DEGREES.
//  Taken from http://en.wikipedia.org/wiki/Earth_radius
//*************************************************************************************************
void rspfEllipsoid::geodeticRadii(const double& lat, rspfDpt& radii) const
{
   double cos_lat = rspf::cosd(lat);
   double sin_lat = rspf::sind(lat);
   double cos2_lat = cos_lat*cos_lat;
   double sin2_lat = sin_lat*sin_lat;
   double H = theA_squared*cos2_lat + theB_squared*sin2_lat;
   double H3 = H*H*H;

   radii.x = theA_squared/sqrt(H);
   radii.y = theA_squared*theB_squared/sqrt(H3);
}

void rspfEllipsoid::latLonHeightToXYZ(double lat, double lon, double height,
                                       double &x, double &y, double &z)const
{
    double sin_latitude = rspf::sind(lat);
    double cos_latitude = rspf::cosd(lat);
    double N = theA / sqrt( 1.0 - theEccentricitySquared*sin_latitude*sin_latitude);
    x = (N+height)*cos_latitude*rspf::cosd(lon);
    y = (N+height)*cos_latitude*rspf::sind(lon);
    z = (N*(1-theEccentricitySquared)+height)*sin_latitude;
}

void rspfEllipsoid::XYZToLatLonHeight(double x, double y, double z,
                                       double& lat, double& lon, double& height)const
{
   double d = sqrt(x*x + y*y);

   double phi2 = z / ((1 - theEccentricitySquared) * d);
   double p = 1.0;
   double phi1 = 0.0;
   double N1 = 0.0;
   double height1 = 0.0;
   int iterIdx = 0;
   const int MAX_ITER = 10;
   if (fabs(phi2) > 1e-16 )
   {
      while ( (p > 1e-17) && (iterIdx < MAX_ITER))
      {
         phi1 = phi2;
         N1 = theA / sqrt(1.0 - (theEccentricitySquared * pow(sin(phi1), 2.0)));
         height1 = (d / cos(phi1) - N1);
         phi2 = atan((z / d) * (1.0 + (theEccentricitySquared * N1 * sin(phi1)) / z));
         p = fabs(phi2 - phi1);
         ++iterIdx;
         /* printf("phi: %e   phi2: %e   p: %e  \n", phi1, phi2, p); */
      }                                                                                           
   }                                                                                                   
   else
   {                                                                                              
      phi1 = phi2;                                                                                
      N1 = theA / sqrt(1.0 - (theEccentricitySquared * pow(sin(phi1), 2.0)));                    
      height1 = (d / cos(phi1)) - N1;                                                             
   }                                                                                                   

   /* *Latitude = phi2 * 180/PI; */
   /* *Longitude = atan2(Y, X) * 180/PI; */
   lat = phi2*DEG_PER_RAD; 
   lon = atan2(y, x)*DEG_PER_RAD;                                                                
   height = height1; 

#if 0
    double p = sqrt(x*x + y*y);
    double theta = atan(z*theA/ (p*theB));
    double eDashSquared = (theA*theA - theB*theB)/
                          (theB*theB);

    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    lat = atan( (z + eDashSquared*theB*sin_theta*sin_theta*sin_theta) /
                     (p - theEccentricitySquared*theA*cos_theta*cos_theta*cos_theta) );
    lon = atan2(y,x);

    double sin_latitude = sin(lat);
    double N = theA / sqrt( 1.0 - theEccentricitySquared*sin_latitude*sin_latitude);

    height = p/cos(lat) - N;
    lat*=DEG_PER_RAD;
    lon*=DEG_PER_RAD;
#endif
}

void rspfEllipsoid::computeLocalToWorldTransformFromXYZ(double x, double y, double z,
                                                         rspfMatrix4x4& localToWorld)const
{
   localToWorld = rspfMatrix4x4::createIdentity();
   NEWMAT::Matrix& m = localToWorld.getData();

   // put in the translation
   m[0][3] = x;
   m[1][3] = y;
   m[2][3] = z;
   


    // normalize X,Y,Z
    double inverse_length = 1.0/sqrt(x*x + y*y + z*z);
    
    x *= inverse_length;
    y *= inverse_length;
    z *= inverse_length;

    double length_XY = sqrt(x*x + y*y);
    double inverse_length_XY = 1.0/length_XY;

    // Vx = |(-Y,X,0)|
    m[0][0] = -y*inverse_length_XY;
    m[1][0] = x*inverse_length_XY;
    m[2][0] = 0.0;

    // Vy = /(-Z*X/(sqrt(X*X+Y*Y), -Z*Y/(sqrt(X*X+Y*Y),sqrt(X*X+Y*Y))| 
    double Vy_x = -z*x*inverse_length_XY;
    double Vy_y = -z*y*inverse_length_XY;
    double Vy_z = length_XY;
    inverse_length = 1.0/sqrt(Vy_x*Vy_x + Vy_y*Vy_y + Vy_z*Vy_z);            
    m[0][1] = Vy_x*inverse_length;
    m[1][1] = Vy_y*inverse_length;
    m[2][1] = Vy_z*inverse_length;

    // Vz = (X,Y,Z)
    m[0][2] = x;
    m[1][2] = y;
    m[2][2] = z;
   
}

rspf_uint32 rspfEllipsoid::getEpsgCode() const
{
   if (!theCode.empty() && (theEpsgCode == 0))
      theEpsgCode = rspfEllipsoidFactory::instance()->findEpsgCode(theCode);
   return theEpsgCode;
}

const rspfEllipsoid& rspfEllipsoid::operator=(const rspfEllipsoid& copy_me)
{
   if (this != &copy_me)
   {
      theName = copy_me.theName;
      theCode = copy_me.theCode;
      theEpsgCode = copy_me.theEpsgCode;
      theA = copy_me.theA;  
      theB = copy_me.theB;  
      theFlattening = copy_me.theFlattening;
      theA_squared = copy_me.theA_squared;
      theB_squared = copy_me.theB_squared;
      theEccentricitySquared = copy_me.theEccentricitySquared;
   }
   return *this;
}