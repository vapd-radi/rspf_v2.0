//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description:
//
//*************************************************************************
// $Id: rspfCmyVector.cpp 9966 2006-11-29 02:01:07Z gpotts $
#include <rspf/base/rspfCmyVector.h>
#include <rspf/base/rspfRgbVector.h>

rspfCmyVector::rspfCmyVector(const rspfRgbVector& rgb)
{
   theC = 255 - rgb.getR();
   theM = 255 - rgb.getG();
   theY = 255 - rgb.getB();
}

const rspfCmyVector& rspfCmyVector::operator = (const rspfRgbVector& rgb)
{
   theC = 255 - rgb.getR();
   theM = 255 - rgb.getG();
   theY = 255 - rgb.getB();
   
   return *this;
}
