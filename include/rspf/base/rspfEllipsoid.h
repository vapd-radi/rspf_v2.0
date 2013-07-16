//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// This is the class declaration for rspfEllipsoid. Though valid for the
// general class of geometric shape, this implementation contains additional
// methods suited to the Earth ellipsoid.
//
//*******************************************************************
//  $ID$

#ifndef rspfEllipsoid_HEADER
#define rspfEllipsoid_HEADER 1

#include <cmath> /* std::sqrt */

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>
#include <rspf/matrix/newmat.h>

class rspfEcefRay;
class rspfEcefPoint;
class rspfEcefVector;
class rspfMatrix4x4;
class rspfKeywordlist;
class rspfDpt;

/*!****************************************************************************
 *
 * CLASS:  rspfEllipsoid
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfEllipsoid
{
public:

   /*!
    * CONSTRUCTORS...
    */
   rspfEllipsoid(const rspfEllipsoid &ellipsoid);
   rspfEllipsoid(const rspfString &name,
                  const rspfString &code,
                  const double &major_axis,
                  const double &minor_axis,
                  rspf_uint32 epsg_code=0);
   rspfEllipsoid(const double &major_axis,
                  const double &minor_axis);
   rspfEllipsoid();
   
   virtual ~rspfEllipsoid(){};

   /*!
    * ACCESS METHOD...
    */
   const rspfString& name()const{return theName;}
   const rspfString& code()const{return theCode;}

   const double&    a()const{return theA;} // major axis
   const double&    b()const{return theB;} // minor axis

   const double& getA()const{return theA;}
   const double& getB()const{return theB;}
   const double& getFlattening()const{return theFlattening;}
   
   void setA(double a){theA = a;computeFlattening();}
   void setB(double b){theB = b;computeFlattening();}
   void setAB(double a, double b){theA = a; theB = b; computeFlattening();}
   void setEpsgCode(rspf_uint32 code) {theEpsgCode = code;}
   double eccentricitySquared() const { return theEccentricitySquared; }
   
   double flattening()const { return theFlattening; }
   
   double eccentricity()const { return std::sqrt(theEccentricitySquared); }
   
   rspf_uint32 getEpsgCode() const;

   /*!
    * METHOD: nearestIntersection()
    * Returns the point of nearest intersection of the ray with the ellipsoid.
    * The first version performs the intersection at the ellipsoid surface.
    * The second version computes the ray's intersection with a surface at
    * some offset outside (for positive offset) of the ellipsoid (think
    * elevation).
    */
   bool nearestIntersection(const rspfEcefRay&  ray,
                            rspfEcefPoint& rtnPt) const;
   bool nearestIntersection(const rspfEcefRay&  ray,
                            const double&        offset,
                            rspfEcefPoint& rtnPt) const;
   
   /*!
    * METHOD: evaluate()
    * evaluate will evalate the function at location x, y, z (ECEF).
    */
   double   evaluate(const rspfEcefPoint&)const;

   /*!
    * METHOD: gradient()
    * Compute the partials along location x, y, and z and place
    * the result in the result vector.
    */
   void gradient(const rspfEcefPoint& location,
                 rspfEcefVector& result)const;
   rspfEcefVector gradient(const rspfEcefPoint& loc)const;
   
   /*!
    * METHOD: prinRadiiOfCurv()
    * Computes the meridional radius and prime vertical at given point.
    */
   void prinRadiiOfCurv(const rspfEcefPoint& location,
                        double& merRadius,
                        double& primeVert)const;
   
   /*!
    * METHOD: jacobianWrtEcef()
    * Forms Jacobian of partials of geodetic WRT ECF.
    */
   void jacobianWrtEcef(const rspfEcefPoint& location,
                        NEWMAT::Matrix& jMat)const;
   
   /*!
    * METHOD: jacobianWrtGeo()
    * Forms Jacobian of partials of ECF WRT geodetic.
    */
   void jacobianWrtGeo(const rspfEcefPoint& location,
                       NEWMAT::Matrix& jMat)const;
   
   /*!
    * Computes the "geodetic" radius for a given latitude in DEGREES:
    */
   double geodeticRadius(const double& latitude) const;
   
   /*!
    * Computes the "geodetic" radius of curvature of the ellipsoid in the east-west (x) and
    * north-south (y) directions for a given latitude in DEGREES:
    */
   void geodeticRadii(const double& latitude, rspfDpt& radii) const;

   void latLonHeightToXYZ(double lat, double lon, double height,
                          double &x, double &y, double &z)const;
   void XYZToLatLonHeight(double x, double y, double z,
                          double& lat, double& lon, double& height)const;

   //---
   // this is a utility from open scene graph that allows you to create a
   // local space rotational
   // and translation matrix
   //---
   void computeLocalToWorldTransformFromXYZ(double x, double y, double z,
                                            rspfMatrix4x4& localToWorld)const;
   
   bool operator ==(const rspfEllipsoid& rhs)const
   {
      return ( (theName == rhs.theName)&&
               (theCode == rhs.theCode)&&
               (theA    == rhs.theA)&&
               (theB    == rhs.theB)&&
               (theFlattening == rhs.theFlattening));
   }

   bool operator!=(const rspfEllipsoid& rhs) const
   {
      return ( (theName != rhs.theName)||
               (theCode != rhs.theCode)||
               (theA    != rhs.theA)||               
               (theB    != rhs.theB)||
               (theFlattening != rhs.theFlattening));
   }

   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;

   const rspfEllipsoid& operator=(const rspfEllipsoid& copy_me);

   virtual bool isEqualTo(const rspfEllipsoid& rhs,
                          rspfCompareType compareType=RSPF_COMPARE_FULL)const;

protected:
   void computeFlattening()
   {
      theFlattening = (theA - theB)/theA;
   }
   
   rspfString theName;
   rspfString theCode;
   mutable rspf_uint32 theEpsgCode;
   double    theA;           //semi-major axis in meters
   double    theB;           //semi-minor axis in meters
   double    theFlattening;
   double    theA_squared;
   double    theB_squared;
   double    theEccentricitySquared;

}; // class RSPFDLLEXPORT rspfEllipsoid

inline bool rspfEllipsoid::isEqualTo(const rspfEllipsoid& rhs,
                                      rspfCompareType /* compareType */)const
{
   return ((theName == rhs.theName)&&
           (theCode == rhs.theCode)&&
           (theEpsgCode ==rhs.theEpsgCode)&&
           rspf::almostEqual(theA, rhs.theA)&&
           rspf::almostEqual(theB, rhs.theB)&&
           rspf::almostEqual(theFlattening, rhs.theFlattening)&&
           rspf::almostEqual(theEccentricitySquared, rhs.theEccentricitySquared));
}

#endif /* #ifndef rspfEllipsoid_HEADER */
