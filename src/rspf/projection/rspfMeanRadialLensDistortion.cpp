#include <rspf/projection/rspfMeanRadialLensDistortion.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
RTTI_DEF1(rspfMeanRadialLensDistortion, "rspfMeanRadialLensDistortion", rspfLensDistortion);
void rspfMeanRadialLensDistortion::solveCoefficients(const std::vector<double>& radialDistance,
                                                      const std::vector<double>& radialDistortion)
{
   NEWMAT::ColumnVector distanceVec((int)radialDistance.size());
   NEWMAT::ColumnVector distortionVec((int)radialDistortion.size());
   rspf_uint32 idx = 0;
   theCoefficients[0] = 0;
   theCoefficients[1] = 0;
   theCoefficients[2] = 0;
   theCoefficients[3] = 0;
   if(radialDistance.size() == radialDistortion.size())
   {
      for(idx = 0; idx < radialDistance.size();++idx)
      {
         distanceVec[idx]   = radialDistance[idx];
         distortionVec[idx] = radialDistortion[idx];
      }
      solveCoefficients(distanceVec,
                        distortionVec);
   }
}
void rspfMeanRadialLensDistortion::solveCoefficients(const NEWMAT::ColumnVector &radialDistance,
                                                      const NEWMAT::ColumnVector &radialDistortion)
{
   NEWMAT::ColumnVector coefficients(4);
   theCoefficients.resize(4);
   theCoefficients[0] = 0;
   theCoefficients[1] = 0;
   theCoefficients[2] = 0;
   theCoefficients[3] = 0;
   if((radialDistance.Nrows() == 0) ||
      (radialDistortion.Nrows() == 0))
   {
      return;
   }
   if(radialDistance.Nrows() == radialDistortion.Nrows())
   {
      NEWMAT::Matrix M(radialDistance.Nrows(), 4);
      
      for(long row = 0; row < M.Nrows(); row++)
      {
         M[row][0] = radialDistance[row];
         M[row][1] = M[row][0]*radialDistance[row]*radialDistance[row];
         M[row][2] = M[row][1]*radialDistance[row]*radialDistance[row];
         M[row][3] = M[row][2]*radialDistance[row]*radialDistance[row];
      }
      
      NEWMAT::Matrix mTranspose = M.t();
      coefficients = ((mTranspose*M).i())*(mTranspose*radialDistortion);
      theCoefficients[0] = coefficients[0];
      theCoefficients[1] = coefficients[1];
      theCoefficients[2] = coefficients[2];
      theCoefficients[3] = coefficients[3];
   }
   else
   {
   }
}
double rspfMeanRadialLensDistortion::deltaR(double radius)const
{
  double r3 = radius*radius*radius;
  double r5 = r3*radius*radius;
  double r7 = r5*radius*radius;
  return (radius*theCoefficients[0] +r3*theCoefficients[1] + r5*theCoefficients[2] + r7*theCoefficients[3]);
}
void rspfMeanRadialLensDistortion::undistort(const rspfDpt& input, rspfDpt& output)const
{
   rspfDpt deltaPosition(input.x - theCenter.x,
                          input.y - theCenter.y);
   double r = sqrt(deltaPosition.x*deltaPosition.x + 
                   deltaPosition.y*deltaPosition.y);
   double deltaRadius = deltaR(r);
   
   if((fabs(r) <= FLT_EPSILON)||
      (fabs(deltaRadius) <= FLT_EPSILON))
   {
      output = input;
      return;
   }
   
   double deltaX      = (deltaPosition.x*(deltaRadius/r)); 
   double deltaY      = (deltaPosition.y*(deltaRadius/r)); 
   
   output.x = deltaPosition.x-deltaX;
   output.y = deltaPosition.y-deltaY;
}
bool rspfMeanRadialLensDistortion::saveState(rspfKeywordlist& kwl,
                                              const char* prefix)const
{
   kwl.add(prefix,
           "k0",
           theCoefficients[0],
           true);
   
   kwl.add(prefix,
           "k1",
           theCoefficients[1],
           true);
   kwl.add(prefix,
           "k2",
           theCoefficients[2],
           true);
   
   kwl.add(prefix,
           "k3",
           theCoefficients[3],
           true);
   return rspfLensDistortion::saveState(kwl, prefix);
}
bool rspfMeanRadialLensDistortion::loadState(const rspfKeywordlist& kwl,
                                              const char* prefix)
{
   const char* k0 = kwl.find(prefix, "k0");
   const char* k1 = kwl.find(prefix, "k1");
   const char* k2 = kwl.find(prefix, "k2");
   const char* k3 = kwl.find(prefix, "k3");
   if(k1&&k2&&k3&&k3)
   {
      theCoefficients[0] = rspfString(k0).toDouble();
      theCoefficients[1] = rspfString(k1).toDouble();
      theCoefficients[2] = rspfString(k2).toDouble();
      theCoefficients[3] = rspfString(k3).toDouble();
   }
   else
   {
      std::vector<double> distances;
      std::vector<double> distortions;
      rspf_uint32 idx = 0;
      const char* distance   = kwl.find(prefix, "distance0");
      const char* distortion = kwl.find(prefix, "distortion0");
      while(distance&&distortion)
      {
         distances.push_back(rspfString(distance).toDouble());
         distortions.push_back(rspfString(distortion).toDouble());
         ++idx;
         distance   = kwl.find(prefix,
                               rspfString("distance")+rspfString::toString(idx));
         distortion = kwl.find(prefix,
                               rspfString("distortion")+rspfString::toString(idx));
      }
      solveCoefficients(distances, distortions);
   }
   return rspfLensDistortion::loadState(kwl, prefix);
   
}
