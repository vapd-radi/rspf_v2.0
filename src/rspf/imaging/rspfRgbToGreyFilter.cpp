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
// $Id: rspfRgbToGreyFilter.cpp 19223 2011-03-24 13:38:21Z dburken $

#include <rspf/imaging/rspfRgbToGreyFilter.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>

RTTI_DEF1(rspfRgbToGreyFilter, "rspfRgbToGreyFilter", rspfImageSourceFilter)

rspfRgbToGreyFilter::rspfRgbToGreyFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theBlankTile(NULL),
    theTile(NULL),
    theC1(1.0/3.0),
    theC2(1.0/3.0),
    theC3(1.0/3.0)
{
}

rspfRgbToGreyFilter::rspfRgbToGreyFilter(rspfImageSource* inputSource,
                                           double c1,
                                           double c2,
                                           double c3)
   : rspfImageSourceFilter(NULL, inputSource),
     theBlankTile(NULL),
     theTile(NULL),
     theC1(c1),
     theC2(c2),
     theC3(c3)
{
}

rspfRgbToGreyFilter::rspfRgbToGreyFilter(rspfObject* owner,
                                           rspfImageSource* inputSource,
                                           double c1,
                                           double c2,
                                           double c3)
   : rspfImageSourceFilter(owner, inputSource),
     theBlankTile(NULL),
     theTile(NULL),
     theC1(c1),
     theC2(c2),
     theC3(c3)
{
}
rspfRgbToGreyFilter::~rspfRgbToGreyFilter()
{
}

rspfRefPtr<rspfImageData> rspfRgbToGreyFilter::getTile(const rspfIrect& tileRect,
                                              rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;  // This filter requires an input.
   }

   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(tileRect,
                                                           resLevel);   
   if(!isSourceEnabled() || !inputTile.valid())
   {
      return inputTile;
   }

   if(!theTile)
   {
      allocate(); // First time through...
   }

   // if (!theTile) // throw exeption...

   if( inputTile->getDataObjectStatus() == RSPF_NULL ||
       inputTile->getDataObjectStatus() == RSPF_EMPTY )
   {
      theBlankTile->setImageRectangle(tileRect);
      return theBlankTile;
   }

   // Set the origin, resize if needed of the output tile.
   theTile->setImageRectangle(tileRect);

   // Filter the tile.
   runUcharTransformation(inputTile);

   // Always validate to set the status.
   theTile->validate();
   
   return theTile;
}


void rspfRgbToGreyFilter::initialize()
{
   // Base class will recapture "theInputConnection".
   rspfImageSourceFilter::initialize();
}

void rspfRgbToGreyFilter::allocate()
{
   if(theInputConnection)
   {
      theTile      = NULL;
      theBlankTile = NULL;

      if(isSourceEnabled())
      {
         theBlankTile = new rspfU8ImageData(this,
                                             1,
                                             theInputConnection->getTileWidth(),
                                             theInputConnection->getTileHeight());  
         
         
         theTile = (rspfImageData*)theBlankTile->dup();
         theTile->initialize();
      }
   }
}

rspf_uint32 rspfRgbToGreyFilter::getNumberOfOutputBands() const
{
   if(isSourceEnabled())
   {
      return 1;
   }

   return rspfImageSourceFilter::getNumberOfOutputBands();
}

bool rspfRgbToGreyFilter::saveState(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   rspfImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,
           "c1",
           theC1,
           true);
   kwl.add(prefix,
           "c2",
           theC2,
           true);
   kwl.add(prefix,
           "c3",
           theC2,
           true);
   
   return true;
}

bool rspfRgbToGreyFilter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   rspfImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "c1");
   if(lookup)
   {
      theC1 = rspfString(lookup).toDouble();
   }
   lookup = kwl.find(prefix, "c2");
   if(lookup)
   {
      theC2 = rspfString(lookup).toDouble();
   }
   lookup = kwl.find(prefix, "c3");
   if(lookup)
   {
      theC3 = rspfString(lookup).toDouble();
   }
   return true;
}

void rspfRgbToGreyFilter::runUcharTransformation(rspfRefPtr<rspfImageData>& tile)
{   
   unsigned char** bandSrc = new unsigned char*[tile->getNumberOfBands()];
   unsigned char* bandDest;
   
   if(tile->getNumberOfBands() == 1)
   {
      bandSrc[0]  = static_cast<unsigned char*>(tile->getBuf(0));
      bandSrc[1]  = static_cast<unsigned char*>(tile->getBuf(0));
      bandSrc[2]  = static_cast<unsigned char*>(tile->getBuf(0));
   }
   else if(tile->getNumberOfBands() == 2)
   {
      bandSrc[0]  = static_cast<unsigned char*>(tile->getBuf(0));
      bandSrc[1]  = static_cast<unsigned char*>(tile->getBuf(1));
      bandSrc[2]  = static_cast<unsigned char*>(tile->getBuf(1));      
   }
   else if(tile->getNumberOfBands() >= 3)
   {
      bandSrc[0]  = static_cast<unsigned char*>(tile->getBuf(0));
      bandSrc[1]  = static_cast<unsigned char*>(tile->getBuf(1));
      bandSrc[2]  = static_cast<unsigned char*>(tile->getBuf(2));      
   }
   bandDest = static_cast<unsigned char*>(theTile->getBuf());
   
   rspf_int32 offset;

   rspf_int32 upperBound = tile->getWidth()*tile->getHeight();
   for(offset = 0; offset < upperBound; ++offset)
   {
      rspf_int32 value;
      
      value = rspf::round<int>(theC1*(bandSrc[0][offset]) +
                    theC2*(bandSrc[1][offset]) +
                    theC3*(bandSrc[2][offset]));
      
      value = value<255?value:255;
      value = value>0?value:0;

      bandDest[offset] = value;
   }

   delete [] bandSrc;
}

rspfString rspfRgbToGreyFilter::getShortName()const
{
   return rspfString("grey");
}

rspfString rspfRgbToGreyFilter::getLongName()const
{
   return rspfString("rgb to grey scale filter");
}

rspfScalarType rspfRgbToGreyFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      return RSPF_UCHAR;
   }
   
   return rspfImageSourceFilter::getOutputScalarType();
}
