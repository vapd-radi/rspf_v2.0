//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Abstract class for rendering vector data.  Derived classes should implement
// the "rasterizeVectorData" data method.
//
//*******************************************************************
//  $Id: rspfVectorRenderer.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <iostream>
using namespace std;

#include <rspf/imaging/rspfVectorRenderer.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfFilter.h>
#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/projection/rspfImageViewTransformFactory.h>

RTTI_DEF1(rspfVectorRenderer, "rspfVectorRenderer", rspfImageSourceFilter)


rspfVectorRenderer::rspfVectorRenderer()
   :
      rspfImageSourceFilter(),
      theImageViewTransform(NULL),
      theBoundingRect(),
      theAreaOfInterest(),
      theNumberOfAoiObjects(0),
      theNumberOfObjects(0)
{
}

rspfVectorRenderer::rspfVectorRenderer(rspfImageSource* inputSource,
                                         rspfImageViewTransform* transform)
   : rspfImageSourceFilter(inputSource),
     theImageViewTransform(transform),
     theBoundingRect(),
     theAreaOfInterest(),
     theNumberOfAoiObjects(0),
     theNumberOfObjects(0)
{
   
}


rspfVectorRenderer::~rspfVectorRenderer()
{
   if (theImageViewTransform)
   {
      delete theImageViewTransform;
      theImageViewTransform = NULL;
   }
}

bool rspfVectorRenderer::saveState(rspfKeywordlist& /* kwl */,
                                    const char* /* prefix */ ) const
{
   return false;
}

bool rspfVectorRenderer::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   rspfString newPrefix = prefix + rspfString("image_view_trans.");

   if(theImageViewTransform)
   {
      delete theImageViewTransform;
      theImageViewTransform = NULL;
   }

   theImageViewTransform
      = rspfImageViewTransformFactory::instance()->
      createTransform(kwl, newPrefix.c_str());
   
   return rspfImageSource::loadState(kwl, prefix);
}

void
rspfVectorRenderer::setImageViewTransform(rspfImageViewTransform* transform)
{
   if(theImageViewTransform)
   {
      delete theImageViewTransform;
   }
   theImageViewTransform = transform;
}

void rspfVectorRenderer::setAreaOfInterest(const rspfDrect& aoi)
{
   theAreaOfInterest = aoi;
}

rspfDrect rspfVectorRenderer::getAreaOfInterest() const
{
   return theAreaOfInterest;
}

rspfDrect rspfVectorRenderer::getBoundingRect(long /* resLevel */)const
{
   return theBoundingRect;
}

rspf_int32 rspfVectorRenderer::getNumberOfAoiObjects() const
{
   return theNumberOfAoiObjects;
}

rspf_int32 rspfVectorRenderer::getNumberOfObjects() const
{
   return theNumberOfObjects;
}

void rspfVectorRenderer::setBoundingRect(rspfDrect& rect)
{
   theBoundingRect = rect;
}

void rspfVectorRenderer::setNumberOfAoiObjects(rspf_int32 number_of_objects)
{
   theNumberOfAoiObjects = number_of_objects;
}

void rspfVectorRenderer::setNumberOfObjects(rspf_int32 number_of_objects)
{
   theNumberOfObjects = number_of_objects;
}
