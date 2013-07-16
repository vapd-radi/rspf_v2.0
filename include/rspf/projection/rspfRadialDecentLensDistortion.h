#ifndef rspfRadialDecentLensDistortion_HEADER
#define rspfRadialDecentLensDistortion_HEADER
#include <iostream>
using namespace std;
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspf2dTo2dTransform.h>
class RSPFDLLEXPORT rspfRadialDecentLensDistortion
   : public rspf2dTo2dTransform
{
public:
   rspfRadialDecentLensDistortion() {}
   rspfRadialDecentLensDistortion
      (const rspfDpt& calibratedPrincipalPoint,
       const NEWMAT::ColumnVector &radialDistortionParameters,
       const NEWMAT::ColumnVector &decentDistortionParameters)
      : theCalibratedPrincipalPoint(calibratedPrincipalPoint),
        theRadialDistortionParameters(radialDistortionParameters),
        theDecentDistortionParameters(decentDistortionParameters)
      {  }
   rspfRadialDecentLensDistortion(const rspfRadialDecentLensDistortion& copy)
      : theCalibratedPrincipalPoint (copy.theCalibratedPrincipalPoint),
        theRadialDistortionParameters (copy.theRadialDistortionParameters),
        theDecentDistortionParameters (copy.theDecentDistortionParameters) { }
   rspfRadialDecentLensDistortion(const rspfKeywordlist& kwl,
                                   const char* prefix);
   virtual ~rspfRadialDecentLensDistortion(){}
   
   /*!
    * Implementation of base class 2D-to-2D transformation. The "forward"
    * transformation is defined here as going from an undistorted ideal point to
    * a distorted real point, i.e., adding distortion.
    * 
    * Also available (implemented in the base class) are:
    *   inverse(distorted_point_in, undistorted_pt_out)
    *   inverse(undistort_this_pt)
    */
   virtual void forward(const rspfDpt& undistorted_point_in,
                        rspfDpt&       distorted_point_out) const;
   virtual void forward(rspfDpt&  modify_this) const 
      {
         rspfDpt output;
         forward(modify_this, output);
         modify_this = output;
      }
      
   
   /*!
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   /*!
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   /*!
    * Set methods provide alternative initialization scheme to loadState:
    */
   void setPrincipalPoint(const rspfDpt pp);
   void setRadialDistortionParams(const NEWMAT::ColumnVector& params);
   void setDecentDistortionParams(const NEWMAT::ColumnVector& params);
   /*!
    * Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   
   static const char* PRINCIPAL_POINT_X_KW;
   static const char* PRINCIPAL_POINT_Y_KW;
   static const char* RADIAL_DISTORTION_COEFF_KW;
   static const char* DECENT_DISTORTION_COEFF_KW;
   
private:
   /*!
    * default implementation Will solve the polynomial:
    *
    * k0 + k1*r^2 + k2*r^4 + k3*r^6 + k4*r^8
    */
   virtual double deltaR(double radius)const;
   
   rspfDpt     theCalibratedPrincipalPoint;
   NEWMAT::ColumnVector theRadialDistortionParameters;
   NEWMAT::ColumnVector theDecentDistortionParameters;  
   TYPE_DATA
};
inline void
rspfRadialDecentLensDistortion::setPrincipalPoint(const rspfDpt pp)
{
   theCalibratedPrincipalPoint = pp;
}
inline void rspfRadialDecentLensDistortion::setRadialDistortionParams
   (const NEWMAT::ColumnVector& params)
{
   theRadialDistortionParameters = params;
}
inline void rspfRadialDecentLensDistortion::setDecentDistortionParams
   (const NEWMAT::ColumnVector& params)
{
   theDecentDistortionParameters = params;
}
#endif
