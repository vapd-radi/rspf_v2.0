//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
//*******************************************************************
//  $Id: rspfMatrix4x4.h 19824 2011-07-15 16:01:34Z dburken $

#ifndef rspfMatrix4x4_HEADER
#define rspfMatrix4x4_HEADER
#include <iostream>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/base/rspfColumnVector4d.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfQuaternion.h>

class RSPF_DLL rspfMatrix4x4
{
 public:
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfMatrix4x4& data)
    {
      return out << data.getData();
    }
  rspfMatrix4x4();
  rspfMatrix4x4(const NEWMAT::Matrix& m);
  rspfMatrix4x4(double v00, double v01, double v02, double v03,
                 double v10, double v11, double v12, double v13,
                 double v20, double v21, double v22, double v23,
                 double v30, double v31, double v32, double v33);
  inline explicit rspfMatrix4x4(const rspf::Quaternion& quat){ makeRotate(quat);}
  rspfMatrix4x4(const rspfMatrix4x4& rhs)
    :theData(4,4)
    {
      theData[0][0] = rhs.theData[0][0];
      theData[0][1] = rhs.theData[0][1];
      theData[0][2] = rhs.theData[0][2];
      theData[0][3] = rhs.theData[0][3];
      theData[1][0] = rhs.theData[1][0];
      theData[1][1] = rhs.theData[1][1];
      theData[1][2] = rhs.theData[1][2];
      theData[1][3] = rhs.theData[1][3];
      theData[2][0] = rhs.theData[2][0];
      theData[2][1] = rhs.theData[2][1];
      theData[2][2] = rhs.theData[2][2];
      theData[2][3] = rhs.theData[2][3];
      theData[3][0] = rhs.theData[3][0];
      theData[3][1] = rhs.theData[3][1];
      theData[3][2] = rhs.theData[3][2];
      theData[3][3] = rhs.theData[3][3];
    }
     void makeRotate( const rspf::Quaternion& quat);
     void setRotate( const rspf::Quaternion& quat);
     rspf::Quaternion getRotate()const;
     void getRotate(rspf::Quaternion& quat)const;
     
  rspfMatrix4x4 operator+ (const rspfMatrix4x4& rhs)const
    {
      return rspfMatrix4x4(theData[0][0] + rhs.theData[0][0], theData[0][1] + rhs.theData[0][1], theData[0][2] + rhs.theData[0][2], theData[0][3] + rhs.theData[0][3],
                            theData[1][0] + rhs.theData[1][0], theData[1][1] + rhs.theData[1][1], theData[1][2] + rhs.theData[1][2], theData[1][3] + rhs.theData[1][3],
                            theData[2][0] + rhs.theData[2][0], theData[2][1] + rhs.theData[2][1], theData[2][2] + rhs.theData[2][2], theData[2][3] + rhs.theData[2][3],
                            theData[3][0] + rhs.theData[3][0], theData[3][1] + rhs.theData[3][1], theData[3][2] + rhs.theData[3][2], theData[3][3] + rhs.theData[3][3]);
    }
  rspfMatrix4x4 operator- (const rspfMatrix4x4& rhs)const
    {
      return rspfMatrix4x4(theData[0][0] - rhs.theData[0][0], theData[0][1] - rhs.theData[0][1], theData[0][2] - rhs.theData[0][2], theData[0][3] - rhs.theData[0][3],
                            theData[1][0] - rhs.theData[1][0], theData[1][1] - rhs.theData[1][1], theData[1][2] - rhs.theData[1][2], theData[1][3] - rhs.theData[1][3],
                            theData[2][0] - rhs.theData[2][0], theData[2][1] - rhs.theData[2][1], theData[2][2] - rhs.theData[2][2], theData[2][3] - rhs.theData[2][3],
                            theData[3][0] - rhs.theData[3][0], theData[3][1] - rhs.theData[3][1], theData[3][2] - rhs.theData[3][2], theData[3][3] - rhs.theData[3][3]);
    }
  friend rspfColumnVector4d operator * (const rspfMatrix4x4& lhs, const rspfColumnVector4d& rhs)
      {
         return rspfColumnVector4d( (lhs.theData[0][0]*rhs[0] + lhs.theData[0][1]*rhs[1] + lhs.theData[0][2]*rhs[2] + lhs.theData[0][3]*rhs[3]),
                                     (lhs.theData[1][0]*rhs[0] + lhs.theData[1][1]*rhs[1] + lhs.theData[1][2]*rhs[2] + lhs.theData[1][3]*rhs[3]),
                                     (lhs.theData[2][0]*rhs[0] + lhs.theData[2][1]*rhs[1] + lhs.theData[2][2]*rhs[2] + lhs.theData[2][3]*rhs[3]),
                                     (lhs.theData[3][0]*rhs[0] + lhs.theData[3][1]*rhs[1] + lhs.theData[3][2]*rhs[2] + lhs.theData[3][3]*rhs[3]));
      
    }
   friend rspfColumnVector3d operator * (const rspfMatrix4x4& lhs, const rspfColumnVector3d& rhs)
      {
         // assume the 4 coordinate (homogeneous coord) of the 3d vector is 1
         return rspfColumnVector3d( (lhs.theData[0][0]*rhs[0] + lhs.theData[0][1]*rhs[1] + lhs.theData[0][2]*rhs[2] + lhs.theData[0][3]),
                                     (lhs.theData[1][0]*rhs[0] + lhs.theData[1][1]*rhs[1] + lhs.theData[1][2]*rhs[2] + lhs.theData[1][3]),
                                     (lhs.theData[2][0]*rhs[0] + lhs.theData[2][1]*rhs[1] + lhs.theData[2][2]*rhs[2] + lhs.theData[2][3]));         
      }
   rspfColumnVector4d rotateOnly(const rspfColumnVector4d& rhs)const
      {
         // multiply only by the 3x3 submatrix.
         return rspfColumnVector4d( (theData[0][0]*rhs[0] + theData[0][1]*rhs[1] + theData[0][2]*rhs[2]),
                                     (theData[1][0]*rhs[0] + theData[1][1]*rhs[1] + theData[1][2]*rhs[2]),
                                     (theData[2][0]*rhs[0] + theData[2][1]*rhs[1] + theData[2][2]*rhs[2]));         
      }
   rspfColumnVector3d rotateOnly(const rspfColumnVector3d& rhs)const
      {
         // assume the 4 coordinate (homogeneous coord) of the 3d vector is 1
         return rspfColumnVector3d( (theData[0][0]*rhs[0] + theData[0][1]*rhs[1] + theData[0][2]*rhs[2]),
                                     (theData[1][0]*rhs[0] + theData[1][1]*rhs[1] + theData[1][2]*rhs[2]),
                                     (theData[2][0]*rhs[0] + theData[2][1]*rhs[1] + theData[2][2]*rhs[2]));
      }
   rspfMatrix4x4 operator*(double scalar)const
      {
         return rspfMatrix4x4(theData[0][0]*scalar, theData[0][1]*scalar, theData[0][2]*scalar, theData[0][3]*scalar,
                               theData[1][0]*scalar, theData[1][1]*scalar, theData[1][2]*scalar, theData[1][3]*scalar,
                               theData[2][0]*scalar, theData[2][1]*scalar, theData[2][2]*scalar, theData[2][3]*scalar,
                               theData[0][0]*scalar, theData[0][1]*scalar, theData[0][2]*scalar, theData[0][3]*scalar);
         
      }
   rspfMatrix4x4 operator*(const rspfMatrix4x4& rhs)const
      {
         rspfMatrix4x4 m;
         
         // element 0,0 is first row time first column
         m.theData[0][0] = (theData[0][0]*rhs.theData[0][0] +
                         theData[0][1]*rhs.theData[1][0] +
                         theData[0][2]*rhs.theData[2][0] +
                         theData[0][3]*rhs.theData[3][0]);
         
         // element 0, 1 is first row second column
         m.theData[0][1] = (theData[0][0]*rhs.theData[0][1] +
                         theData[0][1]*rhs.theData[1][1] +
                         theData[0][2]*rhs.theData[2][1] +
                         theData[0][3]*rhs.theData[3][1]);
         
         // element 0, 2 is first row third column
         m.theData[0][2] = (theData[0][0]*rhs.theData[0][2] +
                         theData[0][1]*rhs.theData[1][2] +
                         theData[0][2]*rhs.theData[2][2] +
                         theData[0][3]*rhs.theData[3][2]);
         
         // element 0, 3 is first row fourth column
         m.theData[0][3] = (theData[0][0]*rhs.theData[0][3] +
                         theData[0][1]*rhs.theData[1][3] +
                         theData[0][2]*rhs.theData[2][3] +
                         theData[0][3]*rhs.theData[3][3]);
         
         // element 1, 0 is second row first column
         m.theData[1][0] = (theData[1][0]*rhs.theData[0][0] +
                         theData[1][1]*rhs.theData[1][0] +
                         theData[1][2]*rhs.theData[2][0] +
                         theData[1][3]*rhs.theData[3][0]);
         
         // element 1, 1 is second row second column
         m.theData[1][1] = (theData[1][0]*rhs.theData[0][1] +
                         theData[1][1]*rhs.theData[1][1] +
                         theData[1][2]*rhs.theData[2][1] +
                         theData[1][3]*rhs.theData[3][1]);
         
         // element 1, 2 is second row third column
         m.theData[1][2] = (theData[1][0]*rhs.theData[0][2] +
                         theData[1][1]*rhs.theData[1][2] +
                         theData[1][2]*rhs.theData[2][2] +
                         theData[1][3]*rhs.theData[3][2]);
         
         // element 1, 3 is second row fourth column
         m.theData[1][3] = (theData[1][0]*rhs.theData[0][3] +
                         theData[1][1]*rhs.theData[1][3] +
                         theData[1][2]*rhs.theData[2][3] +
                         theData[1][3]*rhs.theData[3][3]);
         
         // element 2, 0 is third row first column
         m.theData[2][0] = (theData[2][0]*rhs.theData[0][0] +
                         theData[2][1]*rhs.theData[1][0] +
                         theData[2][2]*rhs.theData[2][0] +
                         theData[2][3]*rhs.theData[3][0]);
         
         // element 2, 1 is third row second column
         m.theData[2][1] = (theData[2][0]*rhs.theData[0][1] +
                         theData[2][1]*rhs.theData[1][1] +
                         theData[2][2]*rhs.theData[2][1] +
                         theData[2][3]*rhs.theData[3][1]);
         
         // element 2, 2 is third row third column
         m.theData[2][2] = (theData[2][0]*rhs.theData[0][2] +
                         theData[2][1]*rhs.theData[1][2] +
                         theData[2][2]*rhs.theData[2][2] +
                         theData[2][3]*rhs.theData[3][2]);
         
         // element 2, 3 is third row fourth column
         m.theData[2][3] = (theData[2][0]*rhs.theData[0][3] +
                         theData[2][1]*rhs.theData[1][3] +
                         theData[2][2]*rhs.theData[2][3] +
                         theData[2][3]*rhs.theData[3][3]);
         
         // element 3, 0 is fourth row first column
         m.theData[3][0] = (theData[3][0]*rhs.theData[0][0] +
                         theData[3][1]*rhs.theData[1][0] +
                         theData[3][2]*rhs.theData[2][0] +
                         theData[3][3]*rhs.theData[3][0]);
         
         // element 3, 1 is fourth row second column
         m.theData[3][1] = (theData[3][0]*rhs.theData[0][1] +
                         theData[3][1]*rhs.theData[1][1] +
                         theData[3][2]*rhs.theData[2][1] +
                         theData[3][3]*rhs.theData[3][1]);
         
         // element 3, 2 is fourth row third column
         m.theData[3][2] = (theData[3][0]*rhs.theData[0][2] +
                         theData[3][1]*rhs.theData[1][2] +
                         theData[3][2]*rhs.theData[2][2] +
                         theData[3][3]*rhs.theData[3][2]);
         
         // element 3, 3 is fourth row fourth column
         m.theData[3][3] = (theData[3][0]*rhs.theData[0][3] +
                         theData[3][1]*rhs.theData[1][3] +
                         theData[3][2]*rhs.theData[2][3] +
                         theData[3][3]*rhs.theData[3][3]);
         
         return m;
      }

   /*!
    * uses the matrix package to compute the inverse
    */
   rspfMatrix4x4& i()
      {
         theData = theData.i();
         
         return *this;
      }
   
   /*!
    * Uses the matrix package to compute the transpose.
    */
   rspfMatrix4x4& t()
      {        
         theData = theData.t();

         return *this;
      }
   
   /*!
    * uses the matrix package to compute the determinant
    */
   double getDeterminant()const
      {
         return theData.Determinant();
      }
   
   /*!
    * Uses the matrix package to compute the eigenvalues for this
    * matrix
    */ 
   rspfColumnVector3d getEigenValues()const;

   bool isEqualTo(const rspfMatrix4x4& rhs,
                  rspfCompareType compareType=RSPF_COMPARE_FULL)const;

   void setZero();
   void setIdentity();
   static NEWMAT::Matrix createIdentity();
   static NEWMAT::Matrix createRotateOnly(const rspfMatrix4x4& aMatrix);
  
   static NEWMAT::Matrix createZero();
   static NEWMAT::Matrix createTranslationMatrix(double x,
                                                 double y,
                                                 double z);
   static NEWMAT::Matrix createRotationMatrix(double angleX,
                                              double angleY,
                                              double angleZ,
                                              rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   static NEWMAT::Matrix createRotationXMatrix(double angle,
                                               rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   static NEWMAT::Matrix createRotationYMatrix(double angle,
                                               rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   static NEWMAT::Matrix createRotationZMatrix(double angle,
                                               rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   
   static NEWMAT::Matrix createScaleMatrix(double X, double Y, double Z);
   
  const NEWMAT::Matrix& getData()const{return theData;}
  NEWMAT::Matrix& getData(){return theData;}

private:
  NEWMAT::Matrix theData;
  
}; // class RSPF_DLL rspfMatrix4x4

inline bool rspfMatrix4x4::isEqualTo(const rspfMatrix4x4& rhs,
                                      rspfCompareType /* compareType */)const
{
   return (rspf::almostEqual(theData[0][0], rhs.theData[0][0])&&
           rspf::almostEqual(theData[0][1], rhs.theData[0][1])&&
           rspf::almostEqual(theData[0][2], rhs.theData[0][2])&&
           rspf::almostEqual(theData[0][3], rhs.theData[0][3])&&
           rspf::almostEqual(theData[1][0], rhs.theData[1][0])&&
           rspf::almostEqual(theData[1][1], rhs.theData[1][1])&&
           rspf::almostEqual(theData[1][2], rhs.theData[1][2])&&
           rspf::almostEqual(theData[1][3], rhs.theData[1][3])&&
           rspf::almostEqual(theData[2][0], rhs.theData[2][0])&&
           rspf::almostEqual(theData[2][1], rhs.theData[2][1])&&
           rspf::almostEqual(theData[2][2], rhs.theData[2][2])&&
           rspf::almostEqual(theData[2][3], rhs.theData[2][3])&&
           rspf::almostEqual(theData[3][0], rhs.theData[3][0])&&
           rspf::almostEqual(theData[3][1], rhs.theData[3][1])&&
           rspf::almostEqual(theData[3][2], rhs.theData[3][2])&&
           rspf::almostEqual(theData[3][3], rhs.theData[3][3]));
}

#endif /* #ifndef rspfMatrix4x4_HEADER */