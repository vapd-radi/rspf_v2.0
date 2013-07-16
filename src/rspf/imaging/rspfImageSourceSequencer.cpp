//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: rspfImageSourceSequencer.cpp 20302 2011-11-29 14:21:12Z dburken $
#include <rspf/imaging/rspfImageSourceSequencer.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/imaging/rspfImageWriter.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF2(rspfImageSourceSequencer, "rspfImageSourceSequencer",
          rspfImageSource, rspfConnectableObjectListener);

static rspfTrace traceDebug("rspfImageSourceSequencer:debug");
   
rspfImageSourceSequencer::rspfImageSourceSequencer(rspfImageSource* inputSource,
                                                     rspfObject* owner)
:rspfImageSource(owner,
                  1,
                  1,
                  true,
                  false),
    theBlankTile(NULL),
    theAreaOfInterest(0,0,0,0),
    theTileSize(RSPF_DEFAULT_TILE_WIDTH, RSPF_DEFAULT_TILE_HEIGHT),
    theNumberOfTilesHorizontal(0),
    theNumberOfTilesVertical(0),
    theCurrentTileNumber(0)
{
   rspf::defaultTileSize(theTileSize);
   theAreaOfInterest.makeNan();
   theInputConnection    = inputSource;
   if(inputSource)
   {
     connectMyInputTo(0, inputSource);
     initialize(); // Derived class depends on this initialization to happen now. DO NOT MOVE.
   }
   addListener( (rspfConnectableObjectListener*)this);
}

rspfImageSourceSequencer::~rspfImageSourceSequencer()
{
   removeListener((rspfConnectableObjectListener*)this);
}

rspf_uint32 rspfImageSourceSequencer::getNumberOfTiles()const
{
   return (theNumberOfTilesHorizontal*theNumberOfTilesVertical);
}


rspf_uint32 rspfImageSourceSequencer::getNumberOfTilesHorizontal()const
{
   return theNumberOfTilesHorizontal;
}

rspf_uint32 rspfImageSourceSequencer::getNumberOfTilesVertical()const
{
   return theNumberOfTilesVertical;
}

rspfScalarType rspfImageSourceSequencer::getOutputScalarType() const
{
   if(theInputConnection)
   {
      return theInputConnection->getOutputScalarType();
   }
   
   return RSPF_SCALAR_UNKNOWN;
}

rspfIpt rspfImageSourceSequencer::getTileSize()const
{
   return theTileSize;
}

void rspfImageSourceSequencer::setTileSize(const rspfIpt& tileSize)
{
   theTileSize = tileSize;
   updateTileDimensions();
//   initialize();
}

void rspfImageSourceSequencer::setTileSize(int width, int height)
{
   setTileSize(rspfIpt(width, height));
}

void rspfImageSourceSequencer::updateTileDimensions()
{
   if(theAreaOfInterest.hasNans() ||
      theTileSize.hasNans())
   {
      theNumberOfTilesHorizontal = 0;
      theNumberOfTilesVertical   = 0;
   }
   else
   {
      rspf_int32 width  = theAreaOfInterest.width();
      rspf_int32 height = theAreaOfInterest.height();

      theNumberOfTilesHorizontal = static_cast<rspf_uint32>(ceil((double)width/(double)theTileSize.x));
      theNumberOfTilesVertical   = static_cast<rspf_uint32>(ceil((double)height/(double)theTileSize.y));
   }
}

void rspfImageSourceSequencer::initialize()
{
   theInputConnection = PTR_CAST(rspfImageSource, getInput(0));

   if(theInputConnection)
   {
      if(theTileSize.hasNans())
      {
         theTileSize.x = theInputConnection->getTileWidth();
         theTileSize.y = theInputConnection->getTileHeight();
      }

      rspfDrect rect = theInputConnection->getBoundingRect();
      if(rect.hasNans())
      {
         theAreaOfInterest.makeNan();
      }
      else
      {
         rect.stretchOut();
         setAreaOfInterest(rect);
      }
      updateTileDimensions();
      
      theBlankTile  = rspfImageDataFactory::instance()->create(this,
                                                                this);
      if(theBlankTile.valid())
      {
         theBlankTile->initialize();
      }
   }
}

bool rspfImageSourceSequencer::canConnectMyInputTo(rspf_int32 /* inputIndex */,
						    const rspfConnectableObject* object)const
{      
  return (object&& PTR_CAST(rspfImageSource, object));
}

void rspfImageSourceSequencer::connectInputEvent(rspfConnectionEvent& /* event */)
{
  initialize();
}

void rspfImageSourceSequencer::disconnectInputEvent(rspfConnectionEvent& /* event */)
{
  theInputConnection = PTR_CAST(rspfImageSource, getInput(0));
}

rspfIrect rspfImageSourceSequencer::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfDrect temp;
   temp.makeNan();

   if(!theInputConnection) return temp;
   
   if(theAreaOfInterest.hasNans())
   {
      return theInputConnection->getBoundingRect(resLevel);
   }
      
   return theAreaOfInterest;
}

void rspfImageSourceSequencer::getDecimationFactor(rspf_uint32 resLevel,
                                                    rspfDpt& result) const
{
   if(theInputConnection)
   {
      theInputConnection->getDecimationFactor(resLevel,
                                              result);
   }
   result.makeNan();
}

void rspfImageSourceSequencer::getDecimationFactors(vector<rspfDpt>& decimations) const
{
   if(theInputConnection)
   {
      theInputConnection->getDecimationFactors(decimations);
   }   
}

rspf_uint32 rspfImageSourceSequencer::getNumberOfDecimationLevels()const
{
   if(theInputConnection)
   {
      return theInputConnection->getNumberOfDecimationLevels();
   }

   return 0;
}

void rspfImageSourceSequencer::setAreaOfInterest(const rspfIrect& areaOfInterest)
{
   if(areaOfInterest.hasNans())
   {
      theAreaOfInterest.makeNan();
      theNumberOfTilesHorizontal = 0;
      theNumberOfTilesVertical   = 0;
   }

   // let's round it to the nearest pixel value before setting it.
   theAreaOfInterest = areaOfInterest;
   updateTileDimensions();
}

const rspfIrect& rspfImageSourceSequencer::getAreaOfInterest()const
{
   return theAreaOfInterest;
}

void rspfImageSourceSequencer::setToStartOfSequence()
{
   theCurrentTileNumber = 0;
}

rspfRefPtr<rspfImageData> rspfImageSourceSequencer::getTile(
   const rspfIrect& rect, rspf_uint32 resLevel)
{
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageData> tile =
         theInputConnection->getTile(rect, resLevel);
      if (tile.valid()&&tile->getBuf())
      {
         return tile;
      }
      else
      {
         // We should return a valid tile for the writers.
         theBlankTile->setImageRectangle(rect);
         return theBlankTile;
      }
   }

   return 0;
}

rspfRefPtr<rspfImageData> rspfImageSourceSequencer::getNextTile( rspf_uint32 resLevel )
{
   rspfRefPtr<rspfImageData> result = 0;
   if ( theInputConnection )
   {
      rspfIrect tileRect;
      if ( getTileRect( theCurrentTileNumber, tileRect ) )
      {
         ++theCurrentTileNumber;
         result = theInputConnection->getTile(tileRect, resLevel);
         if( !result.valid() || !result->getBuf() )
         {	 
            theBlankTile->setImageRectangle(tileRect);
            result = theBlankTile;
         }
      }
   }
   return result;
}

rspfRefPtr<rspfImageData> rspfImageSourceSequencer::getTile(
   rspf_int32 id, rspf_uint32 resLevel)
{
   static const char* MODULE= "rspfImageSourceSequencer::getTile(id, resLevel)";
   if(traceDebug())
   {
      CLOG << "entering.."<<endl;
   }

   rspfRefPtr<rspfImageData> result = 0;

   if(theInputConnection)
   {
      // if we have no tiles try to initialize.
      if(getNumberOfTiles() == 0)
      {
         initialize();
      }

      rspfIrect tileRect;
      if ( getTileRect( id, tileRect ) )
      {
         result = theInputConnection->getTile(tileRect, resLevel);
         if( !result.valid() || !result->getBuf() )
         {	 
            theBlankTile->setImageRectangle(tileRect);
            result = theBlankTile;
         }
      }
      else // getTileRect failed...
      {
         if(traceDebug())
         {
            CLOG << "was not able to get an origin for id = " << id << endl;
         }
      }
   }
   else // no connection...
   {
      if(traceDebug())
      {
         CLOG << "No input connection so returing NULL" << endl;
      }
   }
   if(traceDebug())
   {
      CLOG << "leaving.."<<endl;
   }
   
   return result;
}

bool rspfImageSourceSequencer::getTileOrigin(rspf_int32 id, rspfIpt& origin) const
{
   bool result = false;
   if( id >= 0 )
   {
      if( (theNumberOfTilesHorizontal > 0) && (theCurrentTileNumber < getNumberOfTiles()) )
      {
         rspf_int32 y = id/static_cast<rspf_int32>(theNumberOfTilesHorizontal);
         rspf_int32 x = id%static_cast<rspf_int32>(theNumberOfTilesHorizontal);
         if((x < static_cast<rspf_int32>(theNumberOfTilesHorizontal)) &&
            (y < static_cast<rspf_int32>(theNumberOfTilesVertical)))
         {           
            origin.x = theAreaOfInterest.ul().x + x*theTileSize.x;
            origin.y = theAreaOfInterest.ul().y + y*theTileSize.y;
            result = true;
         }
      }
   }
   return result;
}

bool rspfImageSourceSequencer::getTileRect(rspf_uint32 tile_id, rspfIrect& tileRect) const
{
   // Fetch tile origin for this tile:
   rspfIpt origin;
   bool result = getTileOrigin(tile_id, origin);
   if ( result )
   {
      // Establish the tile rect of interest for this tile:
      tileRect.set_ul (origin);
      tileRect.set_lrx(origin.x + theTileSize.x - 1);
      tileRect.set_lry(origin.y + theTileSize.y - 1);
   }
   return result;
}

double rspfImageSourceSequencer::getNullPixelValue(rspf_uint32 band)const
{
   if (theInputConnection)
   {
      return theInputConnection->getNullPixelValue(band);
   }

   return rspfImageSource::getNullPixelValue(band);
}

double rspfImageSourceSequencer::getMinPixelValue(rspf_uint32 band)const
{
   if (theInputConnection)
   {
      return theInputConnection->getMinPixelValue(band);
   }
   
   return rspfImageSource::getMinPixelValue(band);  
}

double rspfImageSourceSequencer::getMaxPixelValue(rspf_uint32 band)const
{
   if (theInputConnection)
   {
      return theInputConnection->getMaxPixelValue(band);
   }
   
   return rspfImageSource::getMaxPixelValue(band);  
}

rspf_uint32 rspfImageSourceSequencer::getNumberOfInputBands()const
{
   if(theInputConnection)
   {
      return theInputConnection->getNumberOfOutputBands();
   }

   return 0;
}
