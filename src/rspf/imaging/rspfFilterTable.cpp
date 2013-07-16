//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
//----------------------------------------------------------------------------
// $Id: rspfFilterTable.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <iostream>
#include <rspf/imaging/rspfFilterTable.h>
#include <rspf/imaging/rspfFilter.h>

rspfFilterTable::rspfFilterTable()
   :theWeights(0),
    theWidth(0),
    theHeight(0),
    theWidthHeight(0),
    theXSupport(0),
    theYSupport(0),
    theFilterSteps(0)
{
   
}

rspfFilterTable::~rspfFilterTable()
{
   if(theWeights)
   {
      delete [] theWeights;
      theWeights = 0;
   }
}

void rspfFilterTable::buildTable(rspf_uint32  filterSteps,
                                  const rspfFilter& filter)
{
   buildTable(filterSteps, filter, filter);
}

void rspfFilterTable::buildTable(rspf_uint32  filterSteps,
                                  const rspfFilter& xFilter,
                                  const rspfFilter& yFilter)
{
   rspf_int32 subpixelSample=0;
   rspf_int32 subpixelLine=0;
   double dx   = 0.0;
   double dy   = 0.0;
   double kernelH   = 0.0;
   double kernelV   = 0.0;
   double x = 0;
   double y = 0;

   double xsupport = ceil(xFilter.getSupport());
   double ysupport = ceil(yFilter.getSupport());
   double left    = 0;
   double right   = 0;
   double top     = 0;
   double bottom  = 0;

   theXSupport = (rspf_uint32)xsupport;
   theYSupport = (rspf_uint32)ysupport;
   theFilterSteps = filterSteps;
   theWidth  = (2*theXSupport);
   theHeight = (2*theYSupport);
   theWidthHeight = theWidth*theHeight;
   
   allocateWeights();
   left   = -(xsupport-1);
   right  = xsupport;
   top    = -(ysupport-1);
   bottom = ysupport;
   rspf_uint32 idx = 0;
      
   for (subpixelLine = 0; subpixelLine < (int)filterSteps; 
        subpixelLine++)
   {
     
     // Calculate subpixel sample step.
     // ---------------------------------- 
     dy = subpixelLine / (double)(filterSteps);
     for (subpixelSample = 0; subpixelSample < (int)filterSteps; 
	  subpixelSample++)
       {
      
	 // Calculate subpixel sample step.
	 // ---------------------------------- 
	 dx = subpixelSample / (double)(filterSteps);
	 
	 for (kernelV=top; kernelV<=bottom;
	      kernelV++)
	   {
	     y = kernelV - dy;
	     double tempWeight = yFilter.filter(y, yFilter.getSupport());
	     for(kernelH=left; kernelH<=right;++kernelH)
	       {
		 x = kernelH - dx;
         
		 // Get the weight for the current pixel.
		 //   ----------------------------------------
		 theWeights[idx] = tempWeight*xFilter.filter(x, xFilter.getSupport());
		 ++idx;
	       }
	   }
       }
   }
}

rspf_uint32 rspfFilterTable::getWidthByHeight()const
{
   return theWidthHeight;
}

rspf_uint32 rspfFilterTable::getXSupport()const
{
   return theXSupport;
}

rspf_uint32 rspfFilterTable::getYSupport()const
{
   return theYSupport;
}

rspf_uint32 rspfFilterTable::getWidth()const
{
   return theWidth;
}

rspf_uint32 rspfFilterTable::getHeight()const
{
   return theHeight;
}

void rspfFilterTable::allocateWeights()
{
   if(theWeights)
   {
      delete [] theWeights;
      theWeights = 0;
   }

   rspf_uint32 size = (theWidthHeight*(theFilterSteps*theFilterSteps));

   if(size)
   {
      theWeights = new double[size];
   }
}
