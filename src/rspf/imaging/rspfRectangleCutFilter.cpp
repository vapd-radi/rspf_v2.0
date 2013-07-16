//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRectangleCutFilter.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfRectangleCutFilter.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageGeometry.h>

static rspfTrace traceDebug("rspfRectangleCutFilter:debug");

RTTI_DEF1(rspfRectangleCutFilter,
          "rspfRectangleCutFilter",
          rspfImageSourceFilter);

rspfRectangleCutFilter::rspfRectangleCutFilter(rspfObject* owner,
                                     rspfImageSource* inputSource)
   :rspfImageSourceFilter(owner, inputSource),
    theCutType(RSPF_RECTANGLE_NULL_OUTSIDE)
{
   theRectangle.makeNan();
}

rspfRectangleCutFilter::rspfRectangleCutFilter(rspfImageSource* inputSource)
   :rspfImageSourceFilter(NULL, inputSource),
    theCutType(RSPF_RECTANGLE_NULL_OUTSIDE)
{
   theRectangle.makeNan();
}


rspfRefPtr<rspfImageData> rspfRectangleCutFilter::getTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;
   }
   
   rspfRefPtr<rspfImageData> tile =
      theInputConnection->getTile(rect, resLevel);
   if(!isSourceEnabled()||theRectangle.hasNans()||!tile.valid())
   {
      return tile;
   }
   if(tile->getDataObjectStatus() == RSPF_NULL ||
      tile->getDataObjectStatus() == RSPF_EMPTY)
   {
      return tile;
   }
   rspf_int32 decimationIndex = min((rspf_int32)resLevel,
                                     (rspf_int32)theDecimationList.size()-1);

   rspfIrect cutRect = theRectangle*theDecimationList[decimationIndex];
   
   rspfIrect inputRectangle = tile->getImageRectangle();
   if(theCutType==RSPF_RECTANGLE_NULL_INSIDE)
   {
      if(cutRect.intersects(inputRectangle))
      {
         if(!inputRectangle.completely_within(cutRect))
         {
            rspf_int32 ulx = inputRectangle.ul().x;
            rspf_int32 uly = inputRectangle.ul().y;
            rspf_int32 w   = tile->getWidth();
            rspf_int32 h   = tile->getHeight();
            rspf_int32 offset = 0;
            rspfIpt tempPoint(ulx, uly);
            
            for(rspf_int32 y = 0;  y < h; ++tempPoint.y,++y)
            {
               tempPoint.x = ulx;
               for(rspf_int32 x = 0; x < w; ++tempPoint.x,++x)
               {
                  if(cutRect.pointWithin(tempPoint))
                  {
                     tile->setNull(offset);
                  }
                  ++offset;
               }
            }
            tile->validate();
         }
         else
         {
            tile->makeBlank();
         }
      }
   }
   else // cut out the outide
   {      
      if(cutRect.intersects(inputRectangle))
      {
         if(!inputRectangle.completely_within(cutRect))
         {
            rspf_int32 ulx = inputRectangle.ul().x;
            rspf_int32 uly = inputRectangle.ul().y;
            rspf_int32 w   = tile->getWidth();
            rspf_int32 h   = tile->getHeight();
            rspf_int32 offset = 0;
            rspfIpt tempPoint(ulx, uly);
            
            for(rspf_int32 y = 0;  y < h; ++tempPoint.y,++y)
            {
               tempPoint.x = ulx;
               for(rspf_int32 x = 0; x < w; ++tempPoint.x,++x)
               {
                  if(!cutRect.pointWithin(tempPoint))
                  {
                     tile->setNull(offset);
                  }
                  ++offset;
               }
            }
            tile->validate();
         }
      }
      else
      {
         tile->makeBlank();
      }
   }
   
   return tile;
}

void rspfRectangleCutFilter::setRectangle(const rspfIrect& rect)
{
   theRectangle = rect;

   if(theRectangle.hasNans())
   {
      if(theInputConnection)
      {
         theRectangle = theInputConnection->getBoundingRect();
      }
   }
}

void rspfRectangleCutFilter::getValidImageVertices(
   vector<rspfIpt>& validVertices,
   rspfVertexOrdering ordering,
   rspf_uint32 /* resLevel */)const
{
   rspfIrect rect = getBoundingRect();

   if(ordering == RSPF_CLOCKWISE_ORDER)
   {
      validVertices.push_back(rect.ul());
      validVertices.push_back(rect.ur());
      validVertices.push_back(rect.lr());
      validVertices.push_back(rect.ll());
   }
   else
   {
      validVertices.push_back(rect.ul());
      validVertices.push_back(rect.ll());
      validVertices.push_back(rect.lr());
      validVertices.push_back(rect.ur());
   }
}

rspfIrect rspfRectangleCutFilter::getBoundingRect(rspf_uint32 resLevel)const
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfRectangleCutFilter::getBoundingRect DEBUG: entered..." << std::endl;
   }
   rspfIrect result;

   result.makeNan();
   if(!theInputConnection)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfRectangleCutFilter::getBoundingRect DEBUG: Input connection was not valid so leaving" << std::endl;
      }
      return result;
   }

   result = theInputConnection->getBoundingRect(resLevel);
   if(isSourceEnabled())
   {
      if(theCutType == RSPF_RECTANGLE_NULL_OUTSIDE)
      {
         rspfDpt decimation;
         getDecimationFactor(resLevel, decimation);
         rspfIrect cutRect = theRectangle;
         if(!decimation.hasNans())
         {
            cutRect = theRectangle*decimation;
         }
         result = cutRect;
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfRectangleCutFilter::getBoundingRect DEBUG: cut rect = " << result  << std::endl;
   }
   return result;
}

void rspfRectangleCutFilter::initialize()
{
   theDecimationList.clear();
   if(theInputConnection)
   {
      getDecimationFactors(theDecimationList);
   }
   if(theDecimationList.empty())
   {
      theDecimationList.push_back(rspfDpt(1,1));
   }
   if(theRectangle.hasNans())
   {
      setRectangle(theRectangle);
   }
}

bool rspfRectangleCutFilter::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   rspfString newPrefix = prefix;
   newPrefix+="clip_rect.";
   
   theRectangle.saveState(kwl, newPrefix);

   if(theCutType == RSPF_RECTANGLE_NULL_INSIDE)
   {
      kwl.add(prefix,
              "cut_type",
              "null_inside",
              true);
   }
   else if(theCutType == RSPF_RECTANGLE_NULL_OUTSIDE)
   {
      kwl.add(prefix,
              "cut_type",
              "null_outside",
              true);      
   }
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfRectangleCutFilter::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   rspfString newPrefix = prefix;
   
   rspfString rect = kwl.find(prefix, "rect");
   if(!rect.empty())
   {
      theRectangle.toRect(rect);
   }
   else 
   {
      newPrefix+="clip_rect.";
      
      theRectangle.loadState(kwl, newPrefix.c_str());
   }

   const char* cutType = kwl.find(prefix, "cut_type");
   if(cutType)
   {
      rspfString c = cutType;
      if(c == "null_inside")
      {
         theCutType = RSPF_RECTANGLE_NULL_INSIDE;
      }
      else if(c == "null_outside")
      {
         theCutType = RSPF_RECTANGLE_NULL_OUTSIDE;         
      }
      else
      {
         theCutType = static_cast<rspfRectangleCutType>(rspfString(cutType).toLong());
      }
   }

   return rspfImageSourceFilter::loadState(kwl, prefix);
}

const rspfIrect& rspfRectangleCutFilter::getRectangle()const
{
   return theRectangle;
}

rspfRectangleCutFilter::rspfRectangleCutType rspfRectangleCutFilter::getCutType()const
{
   return theCutType;
}

void rspfRectangleCutFilter::setCutType(rspfRectangleCutType cutType)
{
   theCutType = cutType;
}
