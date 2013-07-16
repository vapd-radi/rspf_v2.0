//*****************************************************************************
// FILE: rspfSubImageTileSource.cc
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfSubImageTileSource.
//   This tile source permits specifying an offset that is to be applied to the
//   tile origin for all getTile() requests. It is intended for converting
//   a full-image space coordinate to a sub-image coordinate.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfSubImageTileSource.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <rspf/imaging/rspfSubImageTileSource.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfImageData.h>

RTTI_DEF1(rspfSubImageTileSource,
          "rspfSubImageTileSource",
          rspfImageSourceFilter);

static const char* SUBIMAGE_OFFSET_X = "offset_x";
static const char* SUBIMAGE_OFFSET_Y = "offset_y";

rspfSubImageTileSource::rspfSubImageTileSource()
   : rspfImageSourceFilter(),
     theSubImageOffset(0, 0),
     theTile(0)
{
}

rspfSubImageTileSource::rspfSubImageTileSource(rspfImageSource* inputSource)
   : rspfImageSourceFilter(inputSource), theSubImageOffset(0, 0),
     theTile(0)
{
}

rspfSubImageTileSource::rspfSubImageTileSource(rspfImageSource* inputSource,
                                                 const rspfIpt&   offset)
   : rspfImageSourceFilter(inputSource), theSubImageOffset(offset),
     theTile(0)
{
}

rspfSubImageTileSource::~rspfSubImageTileSource()
{
}

//*****************************************************************************
//  METHOD: rspfSubImageTileSource::getTile()
//  
//*****************************************************************************
rspfRefPtr<rspfImageData> rspfSubImageTileSource::getTile(
   const rspfIrect& rect,
   rspf_uint32  res_level)
{
   rspfIpt offset = theSubImageOffset;

   if(res_level)
   {
      //***
      // Determine the offset for the particular rr level requested:
      //***
      rspfDpt decimation_factor;
      theInputConnection->getDecimationFactor(res_level, decimation_factor);
      
      if(!decimation_factor.hasNans())
      {
         offset = rspfIpt((int)(theSubImageOffset.x*decimation_factor.x + 0.5),
                           (int)(theSubImageOffset.y*decimation_factor.y + 0.5));
      }
      else
      {
         offset = theSubImageOffset;
      }
   }
   //
   // Apply the offset to the requested rect and fetch tile. The tile's origin
   // needs to be adjusted to reflect the requested origin:
   //
   rspfIrect rect_prime (rect - offset);

   rspfRefPtr<rspfImageData> tile =
      rspfImageSourceFilter::getTile(rect_prime, res_level);
   
   if(theTile.valid())
   {
      theTile->setImageRectangle(rect_prime);

      if(tile.valid())
      {
         theTile->loadTile(tile.get());
      }
      else
      {
         theTile->makeBlank();
      }
   }
   else
   {
      if(tile.valid())
      {
         theTile = (rspfImageData*)tile->dup();
      }
   }
   if(theTile.valid())
   {
      theTile->setOrigin(rect.ul());
      theTile->validate();
   }
   
   return theTile;
}

//*****************************************************************************
//  METHOD: rspfSubImageTileSource::getBoundingRect()
//  
//*****************************************************************************
rspfIrect rspfSubImageTileSource::getBoundingRect(rspf_uint32 resLevel) const
{
   rspfDrect result;

   result.makeNan();
   if(!theInputConnection)
      return result;
   rspfDpt offset = theSubImageOffset;
   
   rspfDrect rect (theInputConnection->getBoundingRect(resLevel));
   if(resLevel)
   {
      rspfDpt decimation_factor;
      theInputConnection->getDecimationFactor(resLevel, decimation_factor);

      if(!decimation_factor.hasNans())
      {
         rect*=decimation_factor;
         offset= rspfDpt(theSubImageOffset.x*decimation_factor.x,
                          theSubImageOffset.y*decimation_factor.y);
      }
   }
   rspfDrect rect_prime (rect + offset);
   
   return rect_prime;
}

//*****************************************************************************
//  METHOD: rspfSubImageTileSource::getValidImageVertices()
//  
//*****************************************************************************
void rspfSubImageTileSource::getValidImageVertices(vector<rspfIpt>& vertices,
                                                    rspfVertexOrdering ordering,
                                                    rspf_uint32 /* resLevel */) const
{
   if(!theInputConnection)
   {
      vertices.clear();
      return;
   }

   rspfDpt offset (theSubImageOffset);

   theInputConnection->getValidImageVertices(vertices, ordering);
   vector<rspfIpt>::iterator vertex = vertices.begin();

   while (vertex != vertices.end())
   {
      (*vertex) += offset;
      vertex++;
   }
   
   return;
}

void rspfSubImageTileSource::initialize()
{
   rspfImageSourceFilter::initialize();

   theTile = NULL;
}

bool rspfSubImageTileSource::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   const char* offset_x = kwl.find(prefix, SUBIMAGE_OFFSET_X);
   const char* offset_y = kwl.find(prefix, SUBIMAGE_OFFSET_Y);

   if(offset_x)
   {
      theSubImageOffset.x = rspfString(offset_x).toLong();
      
   }
   if(offset_y)
   {
      theSubImageOffset.y = rspfString(offset_y).toLong();      
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfSubImageTileSource::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   bool result = rspfImageSourceFilter::saveState(kwl, prefix);
   
   kwl.add(prefix,
           SUBIMAGE_OFFSET_X,
           theSubImageOffset.x,
           true);

   kwl.add(prefix,
           SUBIMAGE_OFFSET_Y,
           theSubImageOffset.y,
           true);
   
   return result;        
}
