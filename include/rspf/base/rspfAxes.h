//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
//
// Contains class declaration for rspfAxes.  This will allow you to
// define three orthogonal rspfAxes and an origin in 3-D space.  This
// information will be used to allow you to place points relative
// to the defined axes.
//*******************************************************************
//  $Id: rspfAxes.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfAxes_HEADER
#define rspfAxes_HEADER
#include <iostream>
#include <iomanip>
using namespace std;
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/base/rspfMatrix4x4.h>
class RSPFDLLEXPORT rspfAxes
{
public:

   friend inline ostream& operator <<(ostream &out, const rspfAxes &axes);
   
   /*!
    * Default constructor is at position 0, 0, 0 with unit axes
    * 
    */
   rspfAxes(const rspfColumnVector3d &origin=rspfColumnVector3d(0,0,0),
             const rspfColumnVector3d &xAxis=rspfColumnVector3d(1,0,0),
             const rspfColumnVector3d &yAxis=rspfColumnVector3d(0,1,0),
             const rspfColumnVector3d &zAxis=rspfColumnVector3d(0,0,1))
      :
         theOrigin(origin),
         theXAxis(xAxis),
         theYAxis(yAxis),
         theZAxis(zAxis)
      {}

   rspfColumnVector3d projectPoint(const rspfColumnVector3d &pt)
      {
         return rspfColumnVector3d(theXAxis.dot(pt),
                                    theYAxis.dot(pt),
                                    theZAxis.dot(pt));
      }

   rspfColumnVector3d pointRelative(const rspfColumnVector3d& pt)
      {
         return projectPoint(pt - theOrigin);
      }
   
   void rotateAxes(const rspfMatrix3x3& m)
      {
         theXAxis = m*theXAxis;
         theYAxis = m*theYAxis;
         theZAxis = m*theZAxis;
      }
   void rotateAxes(const rspfMatrix4x4& m)
      {
         theXAxis = m.rotateOnly(theXAxis);
         theYAxis = m.rotateOnly(theYAxis);
         theZAxis = m.rotateOnly(theZAxis);
      }
   void transformOrigin(const rspfMatrix3x3& m)
      {
         theOrigin = m*theOrigin;
      }
   void transformOrigin(const rspfMatrix4x4& m)
      {
         theOrigin = m*theOrigin;
      }
   const rspfColumnVector3d& origin()const{return theOrigin;}
   const rspfColumnVector3d& xAxis()const{return theXAxis;}
   const rspfColumnVector3d& yAxis()const{return theYAxis;}
   const rspfColumnVector3d& zAxis()const{return theZAxis;}
   
private:
   rspfColumnVector3d theOrigin; // Is the origin of this axes
   rspfColumnVector3d theXAxis;  // Is the direction of the x axis
   rspfColumnVector3d theYAxis;  // Is the direction of the y axis
   rspfColumnVector3d theZAxis;  // Is the direction of the z axis
};

inline ostream& operator <<(ostream &out, const rspfAxes &axes)
{
   return out << "position: " << axes.theOrigin << endl
              << "x axis  : " << axes.theXAxis  << endl
              << "Y axis  : " << axes.theYAxis  << endl
              << "z axis  : " << axes.theZAxis;
}

#endif
