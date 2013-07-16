#ifndef rspfQuaternion_HEADER
#define rspfQuaternion_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfDpt3d.h>
class rspfMatrix4x4;
namespace rspf
{


/** A quaternion class. It can be used to represent an orientation in 3D space.*/
class RSPF_DLL Quaternion
{

    public:

        typedef rspf_float64 value_type;

        value_type  theVector[4];    // a four-vector

        inline Quaternion()
        {
           theVector[0]=0.0; theVector[1]=0.0; theVector[2]=0.0; theVector[3]=1.0;
        }

        inline Quaternion( value_type x, value_type y, value_type z, value_type w )
        {
            theVector[0]=x;
            theVector[1]=y;
            theVector[2]=z;
            theVector[3]=w;
        }

/*         inline Quaternion( const Vec4f& v ) */
/*         { */
/*             theVector[0]=v.x(); */
/*             theVector[1]=v.y(); */
/*             theVector[2]=v.z(); */
/*             theVector[3]=v.w(); */
/*         } */

/*         inline Quaternion( const Vec4d& v ) */
/*         { */
/*             theVector[0]=v.x(); */
/*             theVector[1]=v.y(); */
/*             theVector[2]=v.z(); */
/*             theVector[3]=v.w(); */
/*         } */

        inline Quaternion(rspf_float64 angle, const rspfDpt3d& axis)
        {
            makeRotate(angle,axis);
        }
        inline Quaternion(rspf_float64 angle, const rspfColumnVector3d& axis)
        {
            makeRotate(angle,axis);
        }
/*         inline Quaternion( value_type angle, const Vec3d& axis) */
/*         { */
/*             makeRotate(angle,axis); */
/*         } */

/*         inline Quaternion( value_type angle1, const Vec3f& axis1,  */
/*                            value_type angle2, const Vec3f& axis2, */
/*                            value_type angle3, const Vec3f& axis3) */
/*         { */
/*             makeRotate(angle1,axis1,angle2,axis2,angle3,axis3); */
/*         } */

/*         inline Quaternion( value_type angle1, const Vec3d& axis1,  */
/*                      value_type angle2, const Vec3d& axis2, */
/*                      value_type angle3, const Vec3d& axis3) */
/*         { */
/*             makeRotate(angle1,axis1,angle2,axis2,angle3,axis3); */
/*         } */

        inline Quaternion& operator = (const Quaternion& v)
           {
              theVector[0]=v.theVector[0];
              theVector[1]=v.theVector[1];
              theVector[2]=v.theVector[2];
              theVector[3]=v.theVector[3];
              return *this;
           }

        inline bool operator == (const Quaternion& v) const { return theVector[0]==v.theVector[0] && theVector[1]==v.theVector[1] && theVector[2]==v.theVector[2] && theVector[3]==v.theVector[3]; }

        inline bool operator != (const Quaternion& v) const { return theVector[0]!=v.theVector[0] || theVector[1]!=v.theVector[1] || theVector[2]!=v.theVector[2] || theVector[3]!=v.theVector[3]; }

        inline bool operator <  (const Quaternion& v) const
        {
            if (theVector[0]<v.theVector[0]) return true;
            else if (theVector[0]>v.theVector[0]) return false;
            else if (theVector[1]<v.theVector[1]) return true;
            else if (theVector[1]>v.theVector[1]) return false;
            else if (theVector[2]<v.theVector[2]) return true;
            else if (theVector[2]>v.theVector[2]) return false;
            else return (theVector[3]<v.theVector[3]);
        }


        inline void set(value_type x, value_type y, value_type z, value_type w)
        {
            theVector[0]=x;
            theVector[1]=y;
            theVector[2]=z;
            theVector[3]=w;
        }
        
        void set(const rspfMatrix4x4& matrix);

        void get(rspfMatrix4x4& matrix) const;
        

        inline value_type & operator [] (int i) { return theVector[i]; }
        inline value_type   operator [] (int i) const { return theVector[i]; }

        inline value_type & x() { return theVector[0]; }
        inline value_type & y() { return theVector[1]; }
        inline value_type & z() { return theVector[2]; }
        inline value_type & w() { return theVector[3]; }

        inline value_type x() const { return theVector[0]; }
        inline value_type y() const { return theVector[1]; }
        inline value_type z() const { return theVector[2]; }
        inline value_type w() const { return theVector[3]; }

        /** return true if the Quaternion represents a zero rotation, and therefore can be ignored in computations.*/
        bool zeroRotation() const { return theVector[0]==0.0 && theVector[1]==0.0 && theVector[2]==0.0 && theVector[3]==1.0; } 


         /* ------------------------------------------------------------- 
                   BASIC ARITHMETIC METHODS            
        Implemented in terms of Vec4s.  Some Vec4 operators, e.g.
        operator* are not appropriate for quaternions (as
        mathematical objects) so they are implemented differently.
        Also define methods for conjugate and the multiplicative inverse.            
        ------------------------------------------------------------- */
        /// Multiply by scalar 
        inline const Quaternion operator * (value_type rhs) const
        {
            return Quaternion(theVector[0]*rhs, theVector[1]*rhs, theVector[2]*rhs, theVector[3]*rhs);
        }

        /// Unary multiply by scalar 
        inline Quaternion& operator *= (value_type rhs)
        {
            theVector[0]*=rhs;
            theVector[1]*=rhs;
            theVector[2]*=rhs;
            theVector[3]*=rhs;
            return *this;        // enable nesting
        }

        /// Binary multiply 
        inline const Quaternion operator*(const Quaternion& rhs) const
        {
            return Quaternion( rhs.theVector[3]*theVector[0] + rhs.theVector[0]*theVector[3] + rhs.theVector[1]*theVector[2] - rhs.theVector[2]*theVector[1],
                 rhs.theVector[3]*theVector[1] - rhs.theVector[0]*theVector[2] + rhs.theVector[1]*theVector[3] + rhs.theVector[2]*theVector[0],
                 rhs.theVector[3]*theVector[2] + rhs.theVector[0]*theVector[1] - rhs.theVector[1]*theVector[0] + rhs.theVector[2]*theVector[3],
                 rhs.theVector[3]*theVector[3] - rhs.theVector[0]*theVector[0] - rhs.theVector[1]*theVector[1] - rhs.theVector[2]*theVector[2] );
        }

        /// Unary multiply 
        inline Quaternion& operator*=(const Quaternion& rhs)
        {
            value_type x = rhs.theVector[3]*theVector[0] + rhs.theVector[0]*theVector[3] + rhs.theVector[1]*theVector[2] - rhs.theVector[2]*theVector[1];
            value_type y = rhs.theVector[3]*theVector[1] - rhs.theVector[0]*theVector[2] + rhs.theVector[1]*theVector[3] + rhs.theVector[2]*theVector[0];
            value_type z = rhs.theVector[3]*theVector[2] + rhs.theVector[0]*theVector[1] - rhs.theVector[1]*theVector[0] + rhs.theVector[2]*theVector[3];
            theVector[3]   = rhs.theVector[3]*theVector[3] - rhs.theVector[0]*theVector[0] - rhs.theVector[1]*theVector[1] - rhs.theVector[2]*theVector[2];

            theVector[2] = z;
            theVector[1] = y;
            theVector[0] = x;

            return (*this);            // enable nesting
        }

        /// Divide by scalar 
        inline Quaternion operator / (value_type rhs) const
        {
            value_type div = 1.0/rhs;
            return Quaternion(theVector[0]*div, theVector[1]*div, theVector[2]*div, theVector[3]*div);
        }

        /// Unary divide by scalar 
        inline Quaternion& operator /= (value_type rhs)
        {
            value_type div = 1.0/rhs;
            theVector[0]*=div;
            theVector[1]*=div;
            theVector[2]*=div;
            theVector[3]*=div;
            return *this;
        }

        /// Binary divide 
        inline const Quaternion operator/(const Quaternion& denom) const
        {
            return ( (*this) * denom.inverse() );
        }

        /// Unary divide 
        inline Quaternion& operator/=(const Quaternion& denom)
        {
            (*this) = (*this) * denom.inverse();
            return (*this);            // enable nesting
        }

        /// Binary addition 
        inline const Quaternion operator + (const Quaternion& rhs) const
        {
            return Quaternion(theVector[0]+rhs.theVector[0], theVector[1]+rhs.theVector[1],
                theVector[2]+rhs.theVector[2], theVector[3]+rhs.theVector[3]);
        }

        /// Unary addition
        inline Quaternion& operator += (const Quaternion& rhs)
        {
            theVector[0] += rhs.theVector[0];
            theVector[1] += rhs.theVector[1];
            theVector[2] += rhs.theVector[2];
            theVector[3] += rhs.theVector[3];
            return *this;            // enable nesting
        }

        /// Binary subtraction 
        inline const Quaternion operator - (const Quaternion& rhs) const
        {
            return Quaternion(theVector[0]-rhs.theVector[0], theVector[1]-rhs.theVector[1],
                theVector[2]-rhs.theVector[2], theVector[3]-rhs.theVector[3] );
        }

        /// Unary subtraction 
        inline Quaternion& operator -= (const Quaternion& rhs)
        {
            theVector[0]-=rhs.theVector[0];
            theVector[1]-=rhs.theVector[1];
            theVector[2]-=rhs.theVector[2];
            theVector[3]-=rhs.theVector[3];
            return *this;            // enable nesting
        }

        /** Negation operator - returns the negative of the quaternion.
        Basically just calls operator - () on the Vec4 */
        inline const Quaternion operator - () const
        {
            return Quaternion (-theVector[0], -theVector[1], -theVector[2], -theVector[3]);
        }

        /// Length of the quaternion = sqrt( vec . vec )
        value_type length() const
        {
           return std::sqrt( theVector[0]*theVector[0] + theVector[1]*theVector[1] + theVector[2]*theVector[2] + theVector[3]*theVector[3]);
        }

        /// Length of the quaternion = vec . vec
        value_type length2() const
        {
            return theVector[0]*theVector[0] + theVector[1]*theVector[1] + theVector[2]*theVector[2] + theVector[3]*theVector[3];
        }

        /// Conjugate 
        inline Quaternion conj () const
        { 
             return Quaternion( -theVector[0], -theVector[1], -theVector[2], theVector[3] );
        }

        /// Multiplicative inverse method: q^(-1) = q^*/(q.q^*)
        inline const Quaternion inverse () const
        {
             return conj() / length2();
         }

      /* -------------------------------------------------------- 
               METHODS RELATED TO ROTATIONS
        Set a quaternion which will perform a rotation of an
        angle around the axis given by the vector (x,y,z).
        Should be written to also accept an angle and a Vec3?

        Define Spherical Linear interpolation method also

        Not inlined - see the Quat.cpp file for implementation
        -------------------------------------------------------- */
        void makeRotate( value_type  angle, 
                         value_type  x, value_type  y, value_type  z );
        void makeRotate ( value_type  angle, const rspfColumnVector3d& vec )
        { makeRotate(angle, vec[0], vec[1], vec[2]);}
        void makeRotate ( value_type  angle, const rspfDpt3d& vec )
        { makeRotate(angle, vec.x, vec.y, vec.z);}

        void makeRotate ( value_type  angle1, const rspfColumnVector3d& axis1, 
                          value_type  angle2, const rspfColumnVector3d& axis2,
                          value_type  angle3, const rspfColumnVector3d& axis3);
        void makeRotate ( value_type  angle1, const rspfDpt3d& axis1, 
                          value_type  angle2, const rspfDpt3d& axis2,
                          value_type  angle3, const rspfDpt3d& axis3);

        /** Make a rotation Quaternion which will rotate vec1 to vec2.
            Generally take a dot product to get the angle between these
            and then use a cross product to get the rotation axis
            Watch out for the two special cases when the vectors
            are co-incident or opposite in direction.*/
        void makeRotate( const rspfColumnVector3d& vec1, const rspfColumnVector3d& vec2 );
        /** Make a rotation Quaternion which will rotate vec1 to vec2.
            Generally take a dot product to get the angle between these
            and then use a cross product to get the rotation axis
            Watch out for the two special cases of when the vectors
            are co-incident or opposite in direction.*/
        void makeRotate( const rspfDpt3d& vec1, const rspfDpt3d& vec2 );
    
/*         void makeRotate_original( const Vec3d& vec1, const Vec3d& vec2 ); */

        /**
         * Return the angle and vector components represented by the quaternion.
         * Angle returned is in degrees.
         */
        void getRotate ( value_type & angle, value_type & x, value_type & y, value_type & z ) const;

        /**
         * Return the angle and vector components represented by the quaternion.
         * Angle returned is in degrees.
         */
        void getRotate ( value_type& angle, rspfDpt3d& vec ) const
        { getRotate(angle, vec.x, vec.y, vec.z);}


        /** Spherical Linear Interpolation.
        As it goes from 0 to 1, the Quaternion object goes from "from" to "to". */
        void slerp   ( value_type  t, const Quaternion& from, const Quaternion& to);
        /** Rotate a vector by this quaternion.*/
        rspfDpt3d operator* (const rspfDpt3d& v) const
        {
            // nVidia SDK implementation
            rspfDpt3d uv, uuv; 
            rspfDpt3d qvec(theVector[0], theVector[1], theVector[2]);
            uv = qvec ^ v;
            uuv = qvec ^ uv; 
            uv *= ( 2.0f * theVector[3] ); 
            uuv *= 2.0f; 
            return v + uv + uuv;
        }
        friend rspfDpt3d operator *(  const rspfDpt3d& lhs,  const rspf::Quaternion& rhs)
        {
           rspfDpt3d uv, uuv; 
           rspfDpt3d qvec(rhs.theVector[0], rhs.theVector[1], rhs.theVector[2]);
           uv = qvec ^ lhs;
           uuv = qvec ^ uv; 
           uv *= ( 2.0f * rhs.theVector[3] ); 
           uuv *= 2.0f;
           
           return lhs + uv + uuv;
        }
    
};    // end of class prototype

}    // end of namespace

#endif 
