//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//  This class maintains the relationship between a local space rectangular
//  (LSR) coordinate system and the earth-centered, earth-fixed (ECEF) system.
//
// SOFTWARE HISTORY:
//
//   09Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//
//*****************************************************************************
//  $Id: rspfLsrSpace.cpp 17593 2010-06-17 19:07:26Z dburken $

#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfEcefVector.h>


//*****************************************************************************
//  CONSTRUCTOR: rspfLsrSpace(origin, x_dir, y_dir, int)
//
//  Constructs the space given origin, and X and Y ECEF directions. The int
//  argument is a place holder only and not used.
//  
//*****************************************************************************
rspfLsrSpace::rspfLsrSpace(const rspfEcefPoint&  origin,
                             const rspfEcefVector& x_dir_ecf_vec,
                             const rspfEcefVector& y_dir_ecf_vec,
                             int   /* z_not_provided_space_holder */)
   : theOrigin (origin)
{
   //***
   // Compute the remaining axis given first two:
   //***
   rspfColumnVector3d xdir (x_dir_ecf_vec.data().unit());
   rspfColumnVector3d ydir (y_dir_ecf_vec.data().unit());
   rspfColumnVector3d zdir (xdir.cross(ydir));

   //***
   // Fill the rotation matrix:
   //***
   theLsrToEcefRotMatrix = rspfMatrix3x3::create(xdir[0], ydir[0], zdir[0],
                                                  xdir[1], ydir[1], zdir[1],
                                                  xdir[2], ydir[2], zdir[2]);
}

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrSpace
//  
//  Constructs the space given origin, and X and Z ECEF directions. The int
//  argument is a place holder only and not used.
//  
//*****************************************************************************
rspfLsrSpace::rspfLsrSpace(const rspfEcefPoint&  origin,
                             const rspfEcefVector& x_dir_ecf_vec,
                             int   /* y_not_provided_space_holder */,
                             const rspfEcefVector& z_dir_ecf_vec)
   : theOrigin (origin)
{
   //***
   // Compute the remaining axis given first two:
   //***
   rspfColumnVector3d xdir (x_dir_ecf_vec.data().unit());
   rspfColumnVector3d zdir (z_dir_ecf_vec.data().unit());
   rspfColumnVector3d ydir (zdir.cross(xdir));

   //***
   // Fill the rotation matrix:
   //***
   theLsrToEcefRotMatrix = rspfMatrix3x3::create(xdir[0], ydir[0], zdir[0],
                                                  xdir[1], ydir[1], zdir[1],
                                                  xdir[2], ydir[2], zdir[2]);
}

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrSpace
//  
//  Constructs the space given origin, and Y and Z ECEF directions. The int
//  argument is a place holder only and not used.
//  
//*****************************************************************************
rspfLsrSpace::rspfLsrSpace(const rspfEcefPoint&  origin,
                             int   /* x_not_provided_space_holder */,
                             const rspfEcefVector& y_dir_ecf_vec,
                             const rspfEcefVector& z_dir_ecf_vec)
   : theOrigin (origin)
{
   //***
   // Compute the remaining axis given first two:
   //***
   rspfColumnVector3d ydir (y_dir_ecf_vec.data().unit());
   rspfColumnVector3d zdir (z_dir_ecf_vec.data().unit());
   rspfColumnVector3d xdir (ydir.cross(zdir));

   //***
   // Fill the rotation matrix:
   //***
   theLsrToEcefRotMatrix = rspfMatrix3x3::create(xdir[0], ydir[0], zdir[0],
                                                  xdir[1], ydir[1], zdir[1],
                                                  xdir[2], ydir[2], zdir[2]);
}

//*****************************************************************************
//  CONSTRUCTORS: rspfLsrSpace(rspfGpt, y_azimuth)
//
//  This constructor sets up a local coordinate system centered at the
//  specified groundpoint, with the Z-axis normal to the ellipsoid and the
//  Y-axis rotated clockwise from north by the y_azimuth. This angle defaults
//  to 0, producing an East-North-Up system.
//  
//*****************************************************************************
rspfLsrSpace::rspfLsrSpace(const rspfGpt& origin,
                             const double&   y_azimuth)
{
   //***
   // Convert ground point origin to ECEF coordinates:
   //***
   theOrigin = rspfEcefPoint(origin);
   
   //***
   // Establish the component vectors for ENU system::
   //***
   double sin_lat = rspf::sind(origin.lat);
   double cos_lat = rspf::cosd(origin.lat);
   double sin_lon = rspf::sind(origin.lon);
   double cos_lon = rspf::cosd(origin.lon);
   
   rspfColumnVector3d E (-sin_lon,
                          cos_lon,
                          0.0);
   rspfColumnVector3d N (-sin_lat*cos_lon,
                          -sin_lat*sin_lon,
                          cos_lat);
   rspfColumnVector3d U (E.cross(N));

   //
   // Fill rotation matrix with these components, rotated about the Z axis
   // by the azimuth indicated:
   //
   if (std::abs(y_azimuth) > FLT_EPSILON)
   {
      double cos_azim = rspf::cosd(y_azimuth);
      double sin_azim = rspf::sind(y_azimuth);
      rspfColumnVector3d X (cos_azim*E - sin_azim*N);
      rspfColumnVector3d Y (sin_azim*E + cos_azim*N);
      rspfColumnVector3d Z (X.cross(Y));
      
      theLsrToEcefRotMatrix
         = rspfMatrix3x3::create(X[0], Y[0], Z[0],
                                  X[1], Y[1], Z[1],
                                  X[2], Y[2], Z[2]);
   }
   else
   {
      //***
      // No azimuth rotation, so simplify:
      //***
      theLsrToEcefRotMatrix = rspfMatrix3x3::create(E[0], N[0], U[0],
                                                     E[1], N[1], U[1],
                                                     E[2], N[2], U[2]);
   }
}

//*****************************************************************************
//  OPERATOR: ==
//*****************************************************************************
bool rspfLsrSpace::operator == (const rspfLsrSpace& that) const
{
   if (theOrigin != that.origin())
      return false;
   
   if (theLsrToEcefRotMatrix != that.theLsrToEcefRotMatrix)
      return false;

   return true;
}
   
//*****************************************************************************
//  OPERATOR: =
//*****************************************************************************
const rspfLsrSpace& rspfLsrSpace::operator = (const rspfLsrSpace& space)
{
   theOrigin = space.theOrigin;
   theLsrToEcefRotMatrix = space.theLsrToEcefRotMatrix;
   return *this;
}

//*****************************************************************************
//  STATIC METHOD: lsrSpaceErrorMessage()
//  
//  Convenience method accessible to all owners of an rspfLsrSpace for
//  displaying an error message when LSR spaces do not match between
//  objects. All operations between LSR objects must be in a common space.
//  
//*****************************************************************************
ostream& rspfLsrSpace::lsrSpaceErrorMessage(ostream& os)
{
   os<<"rspfLsrSpace ERROR: An operation was attempted between two LSR \n"
     <<"  objects with differing LSR spaces. This is an illegal condition.\n"
     <<"  Please check the data and/or report the error to RSPF development."
     << std::endl;

   return os;
}

//*****************************************************************************
//  METHOD: print()
//  
//  Dumps contents to stream for debug purposes. Defaults to cout.
//  
//*****************************************************************************
void rspfLsrSpace::print(ostream& stream) const
{
   stream << "(rspfLsrSpace)"
          << "\n   theOrigin = " << theOrigin
          << "\n   theLsrToEcefRotMatrix = \n" << theLsrToEcefRotMatrix << std::endl;
}
   
