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
// $Id: rspfTrimFilter.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/imaging/rspfTrimFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfDpt.h>

static rspfTrace traceDebug("rspfTrimFilter:debug");

RTTI_DEF1(rspfTrimFilter,
          "rspfTrimFilter",
          rspfImageSourceFilter);

rspfTrimFilter::rspfTrimFilter()
   :rspfImageSourceFilter(),
    theLeftPercent(0.0),
    theRightPercent(0.0),
    theTopPercent(0.0),
    theBottomPercent(0.0)
{
   theCutter = new rspfPolyCutter;
}

rspfTrimFilter::~rspfTrimFilter()
{
   theCutter = 0;
}

rspfRefPtr<rspfImageData> rspfTrimFilter::getTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;
   }

   if(!theValidVertices.size())
   {
      return rspfImageSourceFilter::getTile(rect, resLevel);
   }
   
   rspfRefPtr<rspfImageData> tile =
      theCutter->getTile(rect, resLevel);

   if(!isSourceEnabled()||!tile.valid())
   {
      return tile;
   }
   if(tile->getDataObjectStatus() == RSPF_NULL ||
      tile->getDataObjectStatus() == RSPF_EMPTY)
   {
      return tile;
   }

   return theCutter->getTile(rect, resLevel);
//    theTile->setImageRectangle(rect);
//    theTile->setDataObjectStatus(RSPF_FULL);

//    theTile->makeBlank();
//    rspfIrect boundingRect = getBoundingRect(resLevel);
//    rspfIrect tileRect     = tile->getImageRectangle();
//    rspfIrect clipRect     = boundingRect.clipToRect(tileRect);

//    theTile->loadTile(tile->getBuf(),
//                      tile->getImageRectangle(),
//                      clipRect,
//                      RSPF_BSQ);
   
//    theTile->validate();
   
}

void rspfTrimFilter::getValidImageVertices(
   vector<rspfIpt>& validVertices,
   rspfVertexOrdering ordering,
   rspf_uint32 resLevel)const
{
   rspfImageSource::getValidImageVertices(validVertices,
                                           ordering,
                                           resLevel);
   if(validVertices.size()==4)
   {
      rspfDpt averagePt((rspfDpt(validVertices[0])+
                          rspfDpt(validVertices[1])+
                          rspfDpt(validVertices[2])+
                          rspfDpt(validVertices[3]))*.25);
      rspfDpt averageTop( ( rspfDpt(validVertices[0]+
                                      validVertices[1])*.5) );
      rspfDpt averageBottom( ( rspfDpt(validVertices[2]+
                                         validVertices[3])*.5 ) );
      rspfDpt averageLeft( (rspfDpt(validVertices[0]+
                                      validVertices[3])*.5) );
      rspfDpt averageRight( (rspfDpt(validVertices[1]+
                                       validVertices[2])*.5) );

      rspfDpt topAxis    = (averageTop-averagePt);
      rspfDpt rightAxis = (averageRight-averagePt);
      rspfDpt leftAxis  = (averageLeft-averagePt);
      rspfDpt bottomAxis = (averageBottom-averagePt);

      double topLen    = topAxis.length();
      double bottomLen = bottomAxis.length();
      double rightLen  = rightAxis.length();
      double leftLen   = leftAxis.length();

      if((topLen > FLT_EPSILON)&&
         (bottomLen > FLT_EPSILON)&&
         (leftLen > FLT_EPSILON)&&
         (rightLen > FLT_EPSILON))
      {
         topAxis    = topAxis*(1.0/topLen);
         bottomAxis = bottomAxis*(1.0/bottomLen);
         leftAxis   = leftAxis*(1.0/leftLen);
         rightAxis  = rightAxis*(1.0/rightLen);

         rspf_uint32 idx = 0;
         rspf_uint32 count = 0;
         for(idx = 0; idx < validVertices.size(); ++idx)
         {
            rspfDpt axis[2];
            double   axisLen[2];
            rspfDpt diff  = validVertices[idx]-averagePt;
            double testLen = (topAxis.x*diff.x+
                              topAxis.y*diff.y);
            count = 0;
            if(testLen >= -FLT_EPSILON)
            {
               axis[count]    = topAxis;
               axisLen[count] = testLen*(1.0-theTopPercent);
               ++count;
            }

            if(count < 2)
            {
               testLen = (bottomAxis.x*diff.x+
                          bottomAxis.y*diff.y);
               if(testLen >= -FLT_EPSILON)
               {
                  axis[count]    = bottomAxis;
                  axisLen[count] = testLen*(1.0-theBottomPercent);
                  ++count;
               }
            }
            if(count < 2)
            {
               testLen = (leftAxis.x*diff.x+
                          leftAxis.y*diff.y);
               if(testLen >= -FLT_EPSILON)
               {
                  axis[count]    = leftAxis;
                  axisLen[count] = testLen*(1.0-theLeftPercent);
                  ++count;
               }
            }
            if(count < 2)
            {
               testLen = (rightAxis.x*diff.x+
                          rightAxis.y*diff.y);
               if(testLen >= -FLT_EPSILON)
               {
                  axis[count]    = rightAxis;
                  axisLen[count] = testLen*(1.0-theRightPercent);
                  ++count;
               }
            }
            if(count == 2)
            {
               validVertices[idx] = (averagePt + (axis[0]*axisLen[0] +
                                                  axis[1]*axisLen[1]));
            }
         }
      }
   }
//    rspfIrect rect = getBoundingRect();

//    if(ordering == RSPF_CLOCKWISE_ORDER)
//    {
//       validVertices.push_back(rect.ul());
//       validVertices.push_back(rect.ur());
//       validVertices.push_back(rect.lr());
//       validVertices.push_back(rect.ll());
//    }
//    else
//    {
//       validVertices.push_back(rect.ul());
//       validVertices.push_back(rect.ll());
//       validVertices.push_back(rect.lr());
//       validVertices.push_back(rect.ur());
//    }
}

rspfIrect rspfTrimFilter::getBoundingRect(rspf_uint32 resLevel)const
{

   vector<rspfIpt> validVertices;
   rspfIrect result;
   result.makeNan();
   getValidImageVertices(validVertices, RSPF_CLOCKWISE_ORDER, resLevel);
   
   if(validVertices.size())
   {
      result = rspfIrect(validVertices);
   }

   return result;
}

void rspfTrimFilter::initialize()
{
   if(theInputConnection)
   {
      theCutter->disconnectMyInput(0, false, false);
      if(getInput())
      {
         theCutter->connectMyInputTo(0, getInput());
      }
      theCutter->initialize();
      getValidImageVertices(theValidVertices);
      theCutter->setNumberOfPolygons(1);
      theCutter->setPolygon(theValidVertices);
   }
}

bool rspfTrimFilter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   kwl.add(prefix,
           "left_percent",
           theLeftPercent,
           true);
   kwl.add(prefix,
           "right_percent",
           theRightPercent,
           true);
   kwl.add(prefix,
           "top_percent",
           theTopPercent,
           true);
   kwl.add(prefix,
           "bottom_percent",
           theBottomPercent,
           true);

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfTrimFilter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   const char* leftPerc   = kwl.find(prefix, "left_percent");
   const char* rightPerc  = kwl.find(prefix, "right_percent");
   const char* topPerc    = kwl.find(prefix, "top_percent");
   const char* bottomPerc = kwl.find(prefix, "bottom_percent");

   if(leftPerc)
   {
      theLeftPercent = rspfString(leftPerc).toDouble();
   }
   if(rightPerc)
   {
      theRightPercent = rspfString(rightPerc).toDouble();
   }
   if(topPerc)
   {
      theTopPercent = rspfString(topPerc).toDouble();
   }
   if(bottomPerc)
   {
      theBottomPercent = rspfString(bottomPerc).toDouble();
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

rspfRefPtr<rspfProperty> rspfTrimFilter::getProperty(const rspfString& name)const
{
   rspfProperty* prop = 0;
   if(name == "left_percent")
   {
      prop = new rspfNumericProperty(name,
                                      rspfString::toString(theLeftPercent),
                                      0.0, 1.0);
      prop->setFullRefreshBit();
   }
   else if(name == "right_percent")
   {
      prop = new rspfNumericProperty(name,
                                      rspfString::toString(theRightPercent),
                                      0.0, 1.0);
      prop->setFullRefreshBit();
   }
   else if(name == "top_percent")
   {
      prop = new rspfNumericProperty(name,
                                      rspfString::toString(theTopPercent),
                                      0.0, 1.0);
      prop->setFullRefreshBit();
   }
   else if(name == "bottom_percent")
   {
      prop = new rspfNumericProperty(name,
                                      rspfString::toString(theBottomPercent),
                                      0.0, 1.0);
      prop->setFullRefreshBit();
   }

   if(prop) return prop;
   
   return rspfImageSourceFilter::getProperty(name);
}

void rspfTrimFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid())return;
   rspfString name = property->getName();
   
   if(name == "left_percent")
   {
      theLeftPercent = property->valueToString().toDouble();
   }
   else if(name == "right_percent")
   {
      theRightPercent = property->valueToString().toDouble();
   }
   else if(name == "top_percent")
   {
      theTopPercent = property->valueToString().toDouble();
   }
   else if(name == "bottom_percent")
   {
      theBottomPercent = property->valueToString().toDouble();
   }
   else
   {
      return rspfImageSourceFilter::setProperty(property);
   }
}

void rspfTrimFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back("left_percent");
   propertyNames.push_back("right_percent");
   propertyNames.push_back("top_percent");
   propertyNames.push_back("bottom_percent");
}
