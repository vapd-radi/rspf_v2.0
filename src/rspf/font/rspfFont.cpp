#include <rspf/font/rspfFont.h>
#include "math.h"
#include <iostream>
using namespace std;
#include <rspf/matrix/newmatio.h>
RTTI_DEF2(rspfFont, "rspfFont", rspfObject, rspfErrorStatusInterface);
void rspfFont::setRotation(double rotationInDegrees)
{
    theRotation    = rotationInDegrees;
    computeMatrix();
}
void rspfFont::setScale(double horizontal,
                         double vertical)
{
   theHorizontalScale = horizontal;
   theVerticalScale   = vertical;
   
   computeMatrix();   
}
void rspfFont::computeMatrix()
{
   NEWMAT::Matrix r(2,2);
   NEWMAT::Matrix s(2,2);
   double radRotation = theRotation*M_PI/180.0;
   
   r << cos(radRotation) << -sin(radRotation)
     << sin(radRotation) << cos(radRotation);
   
   s << theHorizontalScale << theVerticalShear
     << theHorizontalShear << theVerticalScale;
   theAffineTransform = s*r;
}
void rspfFont::getBoundingClipBox(rspfIrect& clippedBox)
{
   getBoundingBox(clippedBox);
   
   if(!theClippingBox.hasNans()&&
      (theClippingBox.width()>1)&&(theClippingBox.height()>1) )
   {
      clippedBox = clippedBox.clipToRect(theClippingBox);
   }
}
rspfIrect rspfFont::getBoundingClipBox()
{
   rspfIrect clipBox;
   getBoundingClipBox(clipBox);
   
   return clipBox;
}
