//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspf2dLinearRegression.cpp 9966 2006-11-29 02:01:07Z gpotts $
#include <rspf/base/rspf2dLinearRegression.h>

rspf2dLinearRegression::rspf2dLinearRegression()
{
   clear();
}

void rspf2dLinearRegression::solve()
{
   
//    theSumX           = 0.0;
//    theSumXX          = 0.0;
//    theSumY           = 0.0;
//    theSumYY          = 0.0;
//    theIntercept      = 0.0;
//    theSlope          = 0.0;
//    theSumXY          = 0.0;
//    rspf_uint32 idx = 0;
//    for(idx = 0; idx < thePoints.size(); ++idx)
//    {
//       theSumX  += thePoints[idx].x;
//       theSumY  += thePoints[idx].y;
//       theSumXX += thePoints[idx].x*thePoints[idx].x;
//       theSumYY += thePoints[idx].y*thePoints[idx].y;
//       theSumXY += thePoints[idx].x*thePoints[idx].y; 
      
//    }
   
   double numberOfPoints = (double)theNumberOfPoints;

   if(numberOfPoints < 1)
   {
      theSlope = 0.0;
      theIntercept = 0.0;
      return;
   }
   double Sxx = theSumXX - ((theSumX*theSumX)/numberOfPoints);
   double Sxy = theSumXY - ((theSumX*theSumY)/numberOfPoints);

   if(fabs(Sxx) < FLT_EPSILON)
   {
      theSlope = 0.0;
      theIntercept = 0.0;
      return;
   }
   
   theSlope     = Sxy/Sxx;
   theIntercept = (theSumY - theSlope*theSumX)/numberOfPoints;

   theSolvedFlag = true;
}