#include <sstream>
#include <rspf/projection/rspfLensDistortion.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
RTTI_DEF1(rspfLensDistortion, "rspfLensDistortion", rspf2dTo2dTransform);
bool rspfLensDistortion::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{
   rspf2dTo2dTransform::saveState(kwl, prefix);
   
   kwl.add(prefix,
           "center",
           rspfString::toString(theCenter.x) + " " + rspfString::toString(theCenter.y),
           true);
   return true;
}
bool rspfLensDistortion::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   const char* center = kwl.find(prefix, "center");
   theCenter.x = 0.0;
   theCenter.y = 0.0;
   
   if(center)
   {
      std::vector<rspfString> splitString;
      rspfString tempString(center);
      tempString = tempString.trim();
      tempString.split(splitString, " ");
      if(splitString.size() == 2)
      {
         theCenter.x = splitString[0].toDouble();
         theCenter.y = splitString[1].toDouble();
      }
   }
   return rspf2dTo2dTransform::loadState(kwl, prefix);
}
void rspfLensDistortion::distort(const rspfDpt& input, rspfDpt& output)const
{
   int iters = 0;
   
   rspfDpt left (input);
   rspfDpt left_dx;
   rspfDpt left_dy;
   rspfDpt right;
   rspfDpt right_dx;
   rspfDpt right_dy;
   rspfDpt dr_dx;
   rspfDpt dr_dy;
   rspfDpt r_diff;
   rspfDpt l_diff;
   double inverse_norm;
   
   do
   {
      left_dx.x = left.x + 1.0;
      left_dx.y = left.y;
      left_dy.x = left.x;
      left_dy.y = left.y + 1.0;
      
      undistort(left,    right);
      undistort(left_dx, right_dx);
      undistort(left_dy, right_dy);
      
      dr_dx.x = (right_dx.x - right.x); //e
      dr_dx.y = (right_dx.y - right.y); //g
      dr_dy.x = (right_dy.x - right.x); //f
      dr_dy.y = (right_dy.y - right.y); //h
      
      r_diff = input - right;
      
      inverse_norm = dr_dy.u*dr_dx.v - dr_dx.u*dr_dy.v; // fg-eh
      if (inverse_norm != 0)
      {
         l_diff.u = (-dr_dy.v*r_diff.u + dr_dy.u*r_diff.v)/inverse_norm;
         l_diff.v = ( dr_dx.v*r_diff.u - dr_dx.u*r_diff.v)/inverse_norm;
         left += l_diff;
      }
      else
      {
         l_diff.u = 0;
         l_diff.v = 0;
      }
      
      iters++;
      
   } while (((fabs(l_diff.u) > theConvergenceThreshold) ||
             (fabs(l_diff.v) > theConvergenceThreshold)) &&
            (iters < theMaxIterations));
   
   
   output = left;
}
