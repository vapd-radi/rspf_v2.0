//-----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Class definition of rspfImageModel.
//
//-----------------------------------------------------------------------------
// $Id$

#include <string>

#include <rspf/imaging/rspfImageModel.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfException.h>
#include <rspf/imaging/rspfImageHandler.h>

RTTI_DEF1(rspfImageModel, "rspfImageModel", rspfObject);

rspfImageModel::rspfImageModel()
   : rspfObject(),
     theSubImageOffset(),
     theDecimationFactors(),
     theLines(0),
     theSamples(0),
     theTargetRrds(0)
{
}

rspfImageModel::~rspfImageModel()
{
}

void rspfImageModel::initialize(const rspfImageHandler& ih)
{
   theSubImageOffset = rspfDpt();
   theLines          = ih.getNumberOfLines(0);
   theSamples        = ih.getNumberOfSamples(0);
   ih.getDecimationFactors(theDecimationFactors);
}

void rspfImageModel::rnToR0(rspf_uint32 rrds,
                             const rspfDpt& rnPt,
                             rspfDpt& r0Pt) const
{
   if ( rrds < theDecimationFactors.size() )
   {
      r0Pt.x = rnPt.x / theDecimationFactors[rrds].x;
      r0Pt.y = rnPt.y / theDecimationFactors[rrds].y;
   }
   else
   {
      std::string e = "rspfImageModel::rnToR0 rrds out of range!";
      throw rspfException(e);
   }
}
void rspfImageModel::rnToR0(const rspfDpt& rnPt, rspfDpt& r0Pt) const
{
   rnToR0(theTargetRrds, rnPt, r0Pt);
}
   
void rspfImageModel::r0ToRn(rspf_uint32 rrds,
                             const rspfDpt& r0Pt,
                             rspfDpt& rnPt) const
{
   if ( rrds < theDecimationFactors.size() )
   {
      rnPt.x = r0Pt.x * theDecimationFactors[rrds].x;
      rnPt.y = r0Pt.y * theDecimationFactors[rrds].y;
   }
   else
   {
      std::string e = "rspfImageModel::r0ToRn rrds out of range!";
      throw rspfException(e);
   }
}

void rspfImageModel::r0ToRn(const rspfDpt& r0Pt, rspfDpt& rnPt) const
{
   r0ToRn(theTargetRrds, r0Pt, rnPt);
}

void rspfImageModel::getSubImageOffset(rspf_uint32 rrds,
                                        rspfDpt& offset) const
{
   if ( rrds < theDecimationFactors.size() )
   {
      offset.x = theSubImageOffset.x * theDecimationFactors[rrds].x;
      offset.y = theSubImageOffset.y * theDecimationFactors[rrds].y;
   }
   else
   {
      std::string e = "rspfImageModel::getSubImageOffset rrds out of range!";
      throw rspfException(e);
   } 
}

void rspfImageModel::getImageRectangle(rspf_uint32 rrds,
                                        rspfDrect& rect) const
{
   if ( rrds < theDecimationFactors.size() )
   {
      rspf_float64 lrX = theSamples * theDecimationFactors[rrds].x - 1.0;
      rspf_float64 lrY = theLines   * theDecimationFactors[rrds].y - 1.0;
      rspfDrect r(0.0, 0.0, lrX, lrY);
      rect = r;
   }
   else
   {
      std::string e = "rspfImageModel::getImageRectangle rrds out of range!";
      throw rspfException(e);
   } 
}

void rspfImageModel::getBoundingRectangle(rspf_uint32 rrds,
                                           rspfDrect& rect) const
{
   if ( rrds < theDecimationFactors.size() )
   {
      rspf_float64 urX = theSubImageOffset.x * theDecimationFactors[rrds].x;
      rspf_float64 urY = theSubImageOffset.y * theDecimationFactors[rrds].y;
      
      rspf_float64 lrX = urX + theSamples*theDecimationFactors[rrds].x - 1.0;
      rspf_float64 lrY = urY + theLines  *theDecimationFactors[rrds].y - 1.0;
      rspfDrect r(0, 0, lrX, lrY);
      rect = r;
   }
   else
   {
      std::string e =
         "rspfImageModel::getBoundingRectangle rrds out of range!";
      throw rspfException(e);
   }  
}
rspf_uint32 rspfImageModel::getNumberOfDecimationLevels()const
{
   return (rspf_uint32)theDecimationFactors.size();
}

void rspfImageModel::setTargetRrds(rspf_uint32 rrds)
{
   theTargetRrds = rrds;
}

rspf_uint32 rspfImageModel::getTargetRrds() const
{
   return theTargetRrds;
}

   

