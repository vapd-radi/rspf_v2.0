#include <cstdlib>
#include <rspf/projection/rspfRadialDecentLensDistortion.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfCommon.h>
RTTI_DEF1(rspfRadialDecentLensDistortion, "rspfRadialDecentLensDistortion",
          rspf2dTo2dTransform);
const char* rspfRadialDecentLensDistortion::PRINCIPAL_POINT_X_KW
                               = "principal_point_x";
const char* rspfRadialDecentLensDistortion::PRINCIPAL_POINT_Y_KW
                               = "principal_point_y";
const char* rspfRadialDecentLensDistortion::RADIAL_DISTORTION_COEFF_KW
                               = "radial_distortion_coeff_K";
const char* rspfRadialDecentLensDistortion::DECENT_DISTORTION_COEFF_KW
                               = "decent_distortion_coeff_P";
rspfRadialDecentLensDistortion::rspfRadialDecentLensDistortion
(const rspfKeywordlist& kwl, const char* prefix)
   :
   theCalibratedPrincipalPoint (0, 0),
   theRadialDistortionParameters(5),
   theDecentDistortionParameters(5)
{
   loadState(kwl, prefix);
}
bool rspfRadialDecentLensDistortion::loadState(const rspfKeywordlist& kwl,
                                                const char* prefix)
{
   const char* value;
   theCalibratedPrincipalPoint = rspfDpt(0, 0);
   value = kwl.find(prefix, PRINCIPAL_POINT_X_KW);
   if (value)
   {
      theCalibratedPrincipalPoint.x = atof(value);
   }
   value = kwl.find(prefix, PRINCIPAL_POINT_Y_KW);
   if (value)
   {
      theCalibratedPrincipalPoint.y = atof(value);
   }
   
   for (int i=0; i<5; ++i)
   {
      rspfString key = RADIAL_DISTORTION_COEFF_KW;
      key += rspfString::toString(i);
      value = kwl.find(prefix, key.c_str());
      if (!value)
      {
         theRadialDistortionParameters[i] = 0.0;
      }
      else
      {
         theRadialDistortionParameters[i] = rspfString::toDouble(value);
      }
      key = DECENT_DISTORTION_COEFF_KW;
      key += rspfString::toString(i);
      value = kwl.find(prefix, key.c_str());
      if (!value)
      {
         theDecentDistortionParameters[i] = 0.0;
      }
      else
      {
         theDecentDistortionParameters[i] = rspfString::toDouble(value);
      }
   }
   bool rtn_val = rspf2dTo2dTransform::loadState(kwl, prefix);
   return rtn_val;
}
bool rspfRadialDecentLensDistortion::saveState(rspfKeywordlist& kwl,
                                                const char* prefix) const
{
   kwl.add(prefix, PRINCIPAL_POINT_X_KW, theCalibratedPrincipalPoint.x);
   kwl.add(prefix, PRINCIPAL_POINT_Y_KW, theCalibratedPrincipalPoint.y);
   
   for(int i=0; i<5; ++i)
   {
      rspfString key = RADIAL_DISTORTION_COEFF_KW;
      key += rspfString::toString(i);
      kwl.add(prefix,
              key.c_str(),
              theRadialDistortionParameters[i],
              true,  // overwrite
              15); // scientific
      key = DECENT_DISTORTION_COEFF_KW;
      key += rspfString::toString(i);
      kwl.add(prefix,
              key.c_str(),
              theDecentDistortionParameters[i],
              true,  // overwrite
              15); // scientific
   }
   bool rtn_val = rspf2dTo2dTransform::saveState(kwl, prefix);
   return rtn_val;
}
   
double rspfRadialDecentLensDistortion::deltaR(double radius)const
{
  double rSquare = radius*radius;
  return theRadialDistortionParameters[0] +
     rSquare*(theRadialDistortionParameters[1] +
              rSquare*(theRadialDistortionParameters[2] +
                       rSquare*(theRadialDistortionParameters[3] +
                                rSquare*theRadialDistortionParameters[4])));
}
void rspfRadialDecentLensDistortion::forward(const rspfDpt& input_pt,
                                              rspfDpt&       output_pt)
   const
{
   rspfDpt deltaPosition(input_pt.x - theCalibratedPrincipalPoint.x,
                          input_pt.y - theCalibratedPrincipalPoint.y);
   double r = sqrt(rspf::square(deltaPosition.x) + 
                   rspf::square(deltaPosition.y));
   double dr = deltaR(r);
   double deltaX1 = deltaPosition.x*dr;
   double deltaY1 = deltaPosition.y*dr;
   double rSquare = r*r;
   double deltaX2 =  (1 + theDecentDistortionParameters[3]*rSquare +
                      theDecentDistortionParameters[4]*rSquare*rSquare)*
                     (theDecentDistortionParameters[1]*(rSquare + 2*deltaPosition.x*deltaPosition.x) +
                      2*theDecentDistortionParameters[2]*deltaPosition.x*deltaPosition.y);
   
   double deltaY2 =  (1 + theDecentDistortionParameters[3]*rSquare +  theDecentDistortionParameters[4]*rSquare*rSquare)*
                     (theDecentDistortionParameters[2]*(rSquare + 2*deltaPosition.y*deltaPosition.y) +
                      2*theDecentDistortionParameters[1]*deltaPosition.x*deltaPosition.y);
   output_pt.x = deltaPosition.x + deltaX1 + deltaX2;
   output_pt.y = deltaPosition.y + deltaY1 + deltaY2;
}
std::ostream& rspfRadialDecentLensDistortion::print(std::ostream& os) const
{
   os << "\nDump of rspfRadialDecentLensDistortion object" << endl;
   
   os << "  theCalibratedPrincipalPoint: " << theCalibratedPrincipalPoint;
   
   for (unsigned int i=0; i<5; i++)
      os << "\n  K["<<i<<"]: " << theRadialDistortionParameters[i];  
   
   for (unsigned int i=0; i<5; i++)
      os << "\n  P["<<i<<"]: " << theDecentDistortionParameters[i];  
   
   os << endl;
   
   return os;
}
