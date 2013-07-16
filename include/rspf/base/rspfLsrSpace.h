//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//  This class maintains the relationship between a local space rectangular
//  (LSR) coordinate system and the earth-centered, earth-fixed (ECEF) system.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfLsrSpace.h 12790 2008-05-05 13:41:33Z dburken $

#ifndef rspfLsrSpace_HEADER
#define rspfLsrSpace_HEADER

#include <rspf/base/rspfEcefPoint.h>
  // #include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/base/rspfNotifyContext.h>

class rspfGpt;
class rspfEcefVector;


//*****************************************************************************
//  CLASS: rspfLsrSpace
//
//  This class maintains the relationship between a local space rectangular
//  (LSR) coordinate system and the earth-centered, earth-fixed (ECEF) system.
//
//*****************************************************************************
class RSPFDLLEXPORT rspfLsrSpace
{
public:
   /*!
    * DEFAULT & COPY CONSTRUCTOR: 
    */
   rspfLsrSpace() {}
   
   rspfLsrSpace(const rspfLsrSpace& copy_this)
      :
	 theLsrToEcefRotMatrix(copy_this.theLsrToEcefRotMatrix),
	 theOrigin(copy_this.theOrigin) {}

   /*!
    * CONSTRUCTORS: The following three constructors accept an ECEF origin
    * and two of the three axes direction vectors. Note the use of the dummy
    * "int" argument (not used) to hold the place of the unspecified axes. 
    */
   rspfLsrSpace(const rspfEcefPoint&  origin,
                 const rspfEcefVector& x_dir_ecf_vec,
                 const rspfEcefVector& y_dir_ecf_vec,
                 const int z_not_provided_space_holder);

   rspfLsrSpace(const rspfEcefPoint&  origin,
                 const rspfEcefVector& x_dir_ecf_vec,
                 const int y_not_provided_space_holder,
                 const rspfEcefVector& z_dir_ecf_vec);

   rspfLsrSpace(const rspfEcefPoint&  origin,
                 const int x_not_provided_space_holder,
                 const rspfEcefVector& y_dir_ecf_vec,
                 const rspfEcefVector& z_dir_ecf_vec);

   /*!
    * CONSTRUCTOR: rspfLsrSpace(rspfGpt, y_azimuth)
    * This constructor sets up a local coordinate system centered at the
    * specified groundpoint, with the Z-axis normal to the ellipsoid and the
    * Y-axis rotated clockwise from north by the y_azimuth. This angle defaults
    * to 0, producing an East-North-Up (ENU) system.
    */
   rspfLsrSpace(const rspfGpt& origin,
                 const double&   y_azimuth=0);

   /*!
    * CONSTRUCTOR: rspfLsrSpace(rspfEcefPt, NEWMAT::Matrix)
    * This constructor sets up a local coordinate system centered at the
    * specified ECF, with given LSR-to-ECF rotation.
    */
   rspfLsrSpace(const rspfEcefPoint& origin,
                 const NEWMAT::Matrix& lsrToEcfRot)
      : theLsrToEcefRotMatrix (lsrToEcfRot), theOrigin (origin) {}

   ~rspfLsrSpace() {}

   /*!
    * OPERATORS:
    */
   bool operator == (const rspfLsrSpace&) const;
   
   bool operator != (const rspfLsrSpace& that) const
      { return !(*this == that); }
   
   const rspfLsrSpace& operator = (const rspfLsrSpace& space);

   /*!
    * DATA ACCESS METHODS:
    */
   const rspfEcefPoint&  origin()          const { return theOrigin; }
   const NEWMAT::Matrix&  lsrToEcefRotMatrix() const
      { return theLsrToEcefRotMatrix; }
   NEWMAT::Matrix ecefToLsrRotMatrix() const
      { return theLsrToEcefRotMatrix.t(); }

   /*!
    * METHOD: lsrSpaceErrorMessage()
    * Convenience method accessible to all owners of an rspfLsrSpace for
    * displaying an error message when LSR spaces do not match between
    * objects. All operations between LSR objects must be in a common space.
    */
   static ostream& lsrSpaceErrorMessage(ostream& os=rspfNotify(rspfNotifyLevel_INFO));
   
   /*!
    * Debug Dump: 
    */
   void print(ostream& stream = rspfNotify(rspfNotifyLevel_INFO)) const;
   
   friend ostream& operator<< (ostream& os , const rspfLsrSpace& instance)
      { instance.print(os); return os; }

private:
   NEWMAT::Matrix   theLsrToEcefRotMatrix;
   rspfEcefPoint   theOrigin;
};

#endif


