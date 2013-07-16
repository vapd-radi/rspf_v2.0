//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description: This is a 3-D vector without the homogeneous
//              coordinate.
//
//*******************************************************************
//  $Id: rspfColumnVector3d.h 20074 2011-09-08 16:16:27Z gpotts $
#ifndef rspfColumnVector3d_HEADER
#define rspfColumnVector3d_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfNotifyContext.h>
#include <float.h> // for FLT_EPSILON
#include <iomanip>
#include <iostream>
#include <cmath>
using namespace std;

#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfColumnVector3d
{
public:
   rspfColumnVector3d()
      {
         data[0]=0;
         data[1]=0;
         data[2]=0;
      }

   rspfColumnVector3d(double x, double y, double z=0)
      {
         data[0]=x;
         data[1]=y;
         data[2]=z;
      }

   rspfColumnVector3d(const rspfColumnVector3d &rhs)
   {
      data[0] = rhs.data[0];
      data[1] = rhs.data[1];
      data[2] = rhs.data[2];
   }
   
   explicit rspfColumnVector3d(const NEWMAT::ColumnVector& rhs)
   {
      if ( (rhs.Ncols() == 1) && (rhs.Nrows() > 2) )
      {
         data[0] = rhs[0];
         data[1] = rhs[1];
         data[2] = rhs[2];
      }
      else
      {
         data[0]=0;
         data[1]=0;
         data[2]=0;
      }
   }

   const rspfColumnVector3d& operator=(const NEWMAT::ColumnVector& rhs)
   {
      if ( (rhs.Ncols() == 1) && (rhs.Nrows() == 3) )
      {
         data[0] = rhs[0];
         data[1] = rhs[1];
         data[2] = rhs[2];
      }
      return *this;
   }

   friend ostream& operator <<(ostream& out, const rspfColumnVector3d& v)
      {
         return out << setiosflags(ios::fixed) << setprecision(15)
                    << v[0] << " " << v[1] <<" "
                    << v[2];
      }

   friend rspfColumnVector3d operator*(double scalar,
					const rspfColumnVector3d &v)
      {
         return rspfColumnVector3d(v.data[0]*scalar,
                                    v.data[1]*scalar,
                                    v.data[2]*scalar);
      }

   /*!
    *  Multiplies column vector times matrix.
    *  Note:
    *  If "lhs" is not a 3x3 that's an error and a blank column vector
    *  is returned.
    */
   friend rspfColumnVector3d operator*(const NEWMAT::Matrix& lhs,
                                        const rspfColumnVector3d& rhs)
   {
      if ((lhs.Ncols() == 3) && (lhs.Nrows() == 3))
      {
         return rspfColumnVector3d( (lhs[0][0]*rhs[0] + lhs[0][1]*rhs[1] + lhs[0][2]*rhs[2]),
                                     (lhs[1][0]*rhs[0] + lhs[1][1]*rhs[1] + lhs[1][2]*rhs[2]),
                                     (lhs[2][0]*rhs[0] + lhs[2][1]*rhs[1] + lhs[2][2]*rhs[2]));
      }
      else if((lhs.Ncols() == 4) && (lhs.Nrows() == 4))
      {
         return rspfColumnVector3d( (lhs[0][0]*rhs[0] + lhs[0][1]*rhs[1] + lhs[0][2]*rhs[2] + lhs[0][3]) ,
                                     (lhs[1][0]*rhs[0] + lhs[1][1]*rhs[1] + lhs[1][2]*rhs[2] + lhs[1][3]) ,
                                     (lhs[2][0]*rhs[0] + lhs[2][1]*rhs[1] + lhs[2][2]*rhs[2] + lhs[2][3]));
      }
      rspfNotify(rspfNotifyLevel_WARN) << "Multiplying a 3 row column vector by an invalid matrix" << std::endl;
      return rhs;
   }

   double& operator [](int index)
      {
         return data[index];
      }

   const double& operator [](int index)const
      {
         return data[index];
      }

   rspfColumnVector3d operator*(double scalar)const
      {
         return rspfColumnVector3d(data[0]*scalar,
                                    data[1]*scalar,
                                    data[2]*scalar);
      }

   rspfColumnVector3d operator +(const rspfColumnVector3d &rhs)const
      {
         return rspfColumnVector3d( data[0]+rhs[0],
                                     data[1]+rhs[1],
                                     data[2]+rhs[2]);
      }

   const rspfColumnVector3d& operator +=(const rspfColumnVector3d &rhs)
      {
         data[0] += rhs[0];
         data[1] += rhs[1];
         data[2] += rhs[2];

         return *this;
      }

   rspfColumnVector3d operator - ()const
      {
         return rspfColumnVector3d(-data[0],
                                    -data[1],
                                    -data[2]);
      }

   rspfColumnVector3d operator /(double scalar)const
      {
         return rspfColumnVector3d( data[0]/scalar,
                                     data[1]/scalar,
                                     data[2]/scalar);
      }

   const rspfColumnVector3d& operator /=(double scalar)
      {
         data[0]/=scalar;
         data[1]/=scalar;
         data[2]/=scalar;

         return *this;
      }

   rspfColumnVector3d operator -(const rspfColumnVector3d &rhs)const
      {
         return rspfColumnVector3d( data[0]-rhs[0],
                                     data[1]-rhs[1],
                                     data[2]-rhs[2]);
      }

   const rspfColumnVector3d& operator -=(const rspfColumnVector3d &rhs)
      {
         data[0] -= rhs[0];
         data[1] -= rhs[1];
         data[2] -= rhs[2];

         return *this;
      }

   bool operator ==(const rspfColumnVector3d &rhs) const
      { return ((fabs(data[0] - rhs[0]) <= FLT_EPSILON) &&
		(fabs(data[1] - rhs[1]) <= FLT_EPSILON) &&
		(fabs(data[2] - rhs[2]) <= FLT_EPSILON));
      }

   bool operator !=(const rspfColumnVector3d &rhs) const
      { return !(*this == rhs); }

   double magnitude()const
      {
         return sqrt(data[0]*data[0] +
                     data[1]*data[1] +
                     data[2]*data[2]);
      }
   inline double norm2()const //!speedup
      {
          return data[0]*data[0] +
                 data[1]*data[1] +
                 data[2]*data[2];
      }
   double dot(const rspfColumnVector3d &rhs) const
      {
         return (data[0]*rhs[0]+
                 data[1]*rhs[1]+
                 data[2]*rhs[2]);
      }

   rspfColumnVector3d unit() const
   {
      double mag = magnitude();

      if(fabs(mag) > FLT_EPSILON)
      {
         mag = 1.0/mag;
         return (*this *mag);
      }

      return *this;
   }

   rspfColumnVector3d cross(const rspfColumnVector3d &rhs) const
      {
         return rspfColumnVector3d( data[1]*rhs[2] - data[2]*rhs[1],
                               data[2]*rhs[0] - data[0]*rhs[2],
                               data[0]*rhs[1] - data[1]*rhs[0]);
      }

   const rspfColumnVector3d& xAligned()
      {
         data[0] = 1;
         data[1] = 0;
         data[2] = 0;

         return *this;
      }
   const rspfColumnVector3d& yAligned()//
      {
         data[0] = 0;
         data[1] = 1;
         data[2] = 0;

         return *this;
      }
   const rspfColumnVector3d& zAligned() //
      {
         data[0] = 0;
         data[1] = 0;
         data[2] = 1;

         return *this;
      }

   /**
    * @brief To string method.
    * 
    * @param precision Output floating point precision.
    * 
    * @return rspfString representing point.
    *
    * Output format:
    * ( 0.0000000,  0.0000000,  0.00000000 )
    *   -----x----  -----y----  ------z----
    */
   rspfString toString(rspf_uint32 precision=15) const;

   /**
    * @brief Initializes this point from string.
    *
    * Expected format:
    * 
    * ( 0.0000000,  0.0000000,  0.00000000 )
    *   -----x----  -----y----  ------z----
    *
    * @param s String to initialize from.
    */
   void toPoint(const std::string& s);
   
private:
   double data[3];
};



#endif
