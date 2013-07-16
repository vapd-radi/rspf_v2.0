//*******************************************************************
// Copyright (C) 2004 Garrett Potts
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: implementation for image generator
//
//*************************************************************************
// $Id: rspfTiling.cpp 20103 2011-09-17 16:10:42Z dburken $

#include <sstream>
#include <iomanip>
#include <rspf/imaging/rspfTiling.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfTiling:debug");

RTTI_DEF1(rspfTiling, "rspfTiling", rspfObject);

rspfTiling::rspfTiling()
      :theTilingDistance(0,0),
       theTilingDistanceUnitType(RSPF_UNIT_UNKNOWN),
       theDelta(0,0),
       theDeltaType(rspfTilingDeltaType_TOTAL_PIXELS),
       theOutputSizeInBytes(0),
       theNumberOfBands(1),
       theNumberOfBytesPerPixelPerBand(1),
       thePaddingSizeInPixels(0.0, 0.0),
       theMapProjection(),
       theImageRect(),
       theTilingRect(),
       theTileId(-1),
       theTotalHorizontalTiles(0),
       theTotalVerticalTiles(0),
       theTotalTiles(0),
       theTileNameMask(),
       theEdgeToEdgeFlag(false)
{
}

bool rspfTiling::initialize(const rspfMapProjection& proj,
                             const rspfIrect& boundingRect)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiling::initialize DEBUG: Entered..."
         << "\nBounding rect === " << boundingRect
         << std::endl;
   }
   
   bool result = true;
   theMapProjection = (rspfMapProjection*)proj.dup();
   theImageRect     = boundingRect;

   result = validate();

   rspfDpt convertedTilingDistance = theTilingDistance;

   if(result)
   {
      // Let's setup the bounding tile rect.
      if(theTilingDistanceUnitType != RSPF_PIXEL)
      {
         getConvertedTilingDistance(convertedTilingDistance);
         
         if(theMapProjection->isGeographic())
         {
            rspfGpt ul;
            rspfGpt ur;
            rspfGpt lr;
            rspfGpt ll;

            theMapProjection->lineSampleToWorld(theImageRect.ul(),
                                                ul);
            theMapProjection->lineSampleToWorld(theImageRect.ur(),
                                                ur);
            theMapProjection->lineSampleToWorld(theImageRect.lr(),
                                                lr);
            theMapProjection->lineSampleToWorld(theImageRect.ll(),
                                                ll);

            theTilingRect = rspfDrect(rspfDpt(ul),
                                       rspfDpt(ur),
                                       rspfDpt(lr),
                                       rspfDpt(ll),
                                       RSPF_RIGHT_HANDED);

            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG: Geographic input bounds =  "
                  << theTilingRect
                  << std::endl;
            }
            
            theTilingRect.stretchToTileBoundary(convertedTilingDistance);
            
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG: Geographic input bounds stretched =  "
                  << theTilingRect << std::endl;
            }
            
            clampGeographic(theTilingRect);

            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG: Geographic input bounds clamped =  "
                  << theTilingRect << std::endl;
            }

         }  // End "if(theMapProjection->isGeographic())"
         
         else // meters
         {
            rspfDpt ul;
            rspfDpt ur;
            rspfDpt lr;
            rspfDpt ll;
            
            theMapProjection->lineSampleToEastingNorthing(theImageRect.ul(),
                                                          ul);
            theMapProjection->lineSampleToEastingNorthing(theImageRect.ur(),
                                                          ur);
            theMapProjection->lineSampleToEastingNorthing(theImageRect.lr(),
                                                          lr);
            theMapProjection->lineSampleToEastingNorthing(theImageRect.ll(),
                                                          ll);
            theTilingRect = rspfDrect(ul,
                                       ur,
                                       lr,
                                       ll,
                                       RSPF_RIGHT_HANDED);
            
            theTilingRect.stretchToTileBoundary(convertedTilingDistance);
         }
      }
      else
      {
         rspf_int32 remainderX = (rspf_int32)((theImageRect.width()%((rspf_int32)theDelta.x)) != 0);
         rspf_int32 remainderY = (rspf_int32)((theImageRect.height()%((rspf_int32)theDelta.y)) != 0);
         
         rspf_int32 w = (rspf_int32)((theImageRect.width()/(rspf_int32)theDelta.x)*theDelta.x +
                                       (remainderX*theDelta.x));
         rspf_int32 h = (rspf_int32)((theImageRect.height()/(rspf_int32)theDelta.y)*theDelta.y +
                                       (remainderY*theDelta.y));
         theTilingRect = rspfDrect(theImageRect.ul().x,
                                    theImageRect.ul().y,
                                    theImageRect.ul().x + (w-1),
                                    theImageRect.ul().y + (h-1));

         // since we are pixel space we force delta to be 1 pixel unit
         //
         theDelta.x   = 1.0;
         theDelta.y   = 1.0;
         theDeltaType = rspfTilingDeltaType_PER_PIXEL;
      }
   }
   
   theTotalHorizontalTiles = rspf::round<int>(fabs(theTilingRect.ur().x - theTilingRect.ul().x)/convertedTilingDistance.x);
   theTotalVerticalTiles   = rspf::round<int>(fabs(theTilingRect.ur().y - theTilingRect.lr().y)/convertedTilingDistance.y);
   theTotalTiles           = theTotalHorizontalTiles*theTotalVerticalTiles;

   // reset the tile id to the beginning
   //
   reset();
   
   if(traceDebug())
   {
      print(rspfNotify(rspfNotifyLevel_DEBUG));
      
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfTiling::initialize DEBUG: Leaving..." << std::endl;
   }
   return result;
}

bool rspfTiling::next(rspfRefPtr<rspfMapProjection>& resultProjection,
                       rspfIrect& resultingBounds,
                       rspfString& resultingName)const
{
   bool result = true;
   
   if(theTileId < theTotalTiles)
   {
      rspfDpt origin;
      rspf_int64 row=0;
      rspf_int64 col=0;

      getRowCol(row, col, theTileId);

      getTileName(resultingName,
                  row, col, theTileId);
      
      getOrigin(origin, row, col);

      if(theTilingDistanceUnitType == RSPF_PIXEL)
      {
         rspfIpt ul = origin;

         resultingBounds =
            rspfIrect(ul.x - (rspf_int32)thePaddingSizeInPixels.x,
                       ul.y - (rspf_int32)thePaddingSizeInPixels.y,
                       ul.x + ((rspf_int32)theTilingDistance.x-1) +
                       (rspf_int32)thePaddingSizeInPixels.x,
                       ul.y + ((rspf_int32)theTilingDistance.y-1) +
                       (rspf_int32)thePaddingSizeInPixels.y);
      }
      else
      {
         
         rspfDpt deltaPerPixel = getDeltaPerPixel();
         rspfIpt pixels;

         if(theDeltaType == rspfTilingDeltaType_TOTAL_PIXELS)
         {
            pixels =   theDelta;
         }
         else
         {
            pixels =   rspfDpt(theTilingDistance.x/theDelta.x,
                                theTilingDistance.y/theDelta.y);
         }

         if(theMapProjection->isGeographic())
         {
            if ( theEdgeToEdgeFlag )
            {
               theMapProjection->setUlTiePoints(
                  rspfGpt( origin.lat - (deltaPerPixel.y/2.0),
                            origin.lon + (deltaPerPixel.x/2.0),
                            0.0,
                            theMapProjection->origin().datum()) );
            }
            else
            {
               theMapProjection->setUlTiePoints(
                  rspfGpt(origin.lat,
                           origin.lon,
                           0.0,
                           theMapProjection->origin().datum()));
               
            }
            theMapProjection->setDecimalDegreesPerPixel(deltaPerPixel);
         }
         else
         {
            if ( theEdgeToEdgeFlag )
            {
               theMapProjection->setUlTiePoints(
                  rspfDpt( origin.x + (deltaPerPixel.x/2.0),
                            origin.y - (deltaPerPixel.y/2.0) ) );
            }
            else
            {
               theMapProjection->setUlTiePoints(rspfDpt(origin.x, origin.y));
            }
            theMapProjection->setMetersPerPixel(deltaPerPixel);
         }
         resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x,
                                      -(rspf_int32)thePaddingSizeInPixels.y,
                                      (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
                                      (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);
      }
      resultProjection = theMapProjection;

      ++theTileId;
      if(traceDebug()&&resultProjection.valid())
      {
         rspfKeywordlist projKwl;
         
         resultProjection->saveState(projKwl);
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfTiling::next DEBUG: rect = " << resultingBounds
            << "\nproj                          = " << projKwl
            << "\ntileName                      = " << resultingName
            << "\norigin                        = " << origin << std::endl;
      }
   }
   else
   {
      result = false;
   }

   
   return result;
}


void rspfTiling::reset()
{
   theTileId = 0;
}

void rspfTiling::setTilingDistance(const rspfDpt& tilingDistance,
                                    rspfUnitType unitType)
{
   theTilingDistance         = tilingDistance;
   theTilingDistanceUnitType = unitType;
}

void rspfTiling::setDelta(const rspfDpt& delta,
                           rspfTilingDeltaType deltaType)
{
   theDelta     = delta;
   theDeltaType = deltaType;
}

bool rspfTiling::validate()const
{
   bool result = true;

   if(theMapProjection.valid())
   {
      if(theMapProjection->isGeographic())
      {
         switch(theTilingDistanceUnitType)
         {
            case RSPF_DEGREES:
            case RSPF_RADIANS:
            case RSPF_SECONDS:
            case RSPF_MINUTES:
            case RSPF_PIXEL:
            {
               break;
            }
            default:
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfTiling::validate()\n"
                  << "Map projeciton requires tiling in angular units but the"
                  << "\nspacing is in non angular" << std::endl;
               result = false;
            }
         }
      }
      else
      {
         switch(theTilingDistanceUnitType)
         {
            case RSPF_METERS:
            case RSPF_FEET:
            case RSPF_US_SURVEY_FEET:
            case RSPF_NAUTICAL_MILES:
            case RSPF_PIXEL:
            case RSPF_MILES:
            case RSPF_MILLIMETERS:
            {
               break;
            }
            default:
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfTiling::validate()"
                  << "\nMap projeciton requires tiling using linear spacing"
                  << " but the spacing is in non linear" << std::endl;
               result = false;
            }
         }
      }
   }
   else
   {
      result = false;
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiling::validate()"
         << "\nthe image projection is null and so not valid"
         << std::endl;
   }
   if((theDelta.x <= FLT_EPSILON) ||
      (theDelta.y <= FLT_EPSILON))
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiling::validate() the delta is too small or is negative"
         << "value = " << theDelta << std::endl;
      result = false;
   }
   if((theTilingDistance.x <= FLT_EPSILON)||
      (theTilingDistance.y <= FLT_EPSILON))
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiling::validate()"
         << "\nthe tiling distance is too small or is negative"
         << "Value = " << theTilingDistance << std::endl;
   }
   
   if(theImageRect.hasNans())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiling::validate() the image rect has nans"
         << std::endl;
      result = false;
   }
   
   return result;
}

bool rspfTiling::isAngularUnit(rspfUnitType unitType)const
{
   bool result = false;
   
   switch(unitType)
   {
      case RSPF_DEGREES:
      case RSPF_RADIANS:
      case RSPF_SECONDS:
      case RSPF_MINUTES:
      {
         result = true;
         break;
      }
      default:
      {
         break;
      }
   }
   return result;
}

bool rspfTiling::isLinearUnit(rspfUnitType unitType)const
{
   bool result = false;
   
   switch(unitType)
   {
      case RSPF_METERS:
      case RSPF_FEET:
      case RSPF_US_SURVEY_FEET:
      case RSPF_NAUTICAL_MILES:
      case RSPF_MILES:
      case RSPF_MILLIMETERS:
      {
         result = true;
         break;
      }
      default:
      {
         break;
      }
   }
   return result;
   
}

void rspfTiling::setTileNameMask(const rspfString& mask)
{
   if(theTileNameMask == "")
   {
      theTileNameMask = "tile%i%";
   }
   if(mask.contains("%i%")||
      mask.contains("%oc%")||
      mask.contains("%or%")||
      mask.contains("%r%")||
      mask.contains("%c%")||
      mask.contains("%SRTM%"))
   {
      theTileNameMask = mask;
   }
   else
   {
      theTileNameMask += "%i%";
   }
}

rspfString rspfTiling::getTileNameMask()const
{
   return theTileNameMask;
}

bool rspfTiling::getRowCol(rspf_int64& row,
                            rspf_int64& col,
                            rspf_int64 tileId)const
{
   bool result = true;
   
   if((theTotalHorizontalTiles > 0)&&
      (tileId < theTotalTiles))
   {
      row = tileId/static_cast<rspf_int64>(theTotalHorizontalTiles);
      col = tileId%static_cast<rspf_int64>(theTotalHorizontalTiles);
      if((col >= static_cast<rspf_int32>(theTotalHorizontalTiles)) &&
         (row >= static_cast<rspf_int32>(theTotalVerticalTiles)))
      {           
         result = false;
      }
   }
   return result;
}

bool rspfTiling::getOrigin(rspfDpt& origin,
                            rspf_int64 tileId)const
{
   rspf_int64 row=0;
   rspf_int64 col=0;
   
   bool result = getRowCol(row, col, tileId);
   if(result)
   {
      result = getOrigin(origin, row, col);
   }

   return result;
}

bool rspfTiling::getOrigin(rspfDpt& origin,
                            rspf_int64 row,
                            rspf_int64 col)const
{
   bool result = false;

   if((col < static_cast<rspf_int64>(theTotalHorizontalTiles)) &&
      (row < static_cast<rspf_int64>(theTotalVerticalTiles)))
   {
      rspfDpt convertedTilingDistance;
      getConvertedTilingDistance(convertedTilingDistance);

      result = true;
      if(theTilingRect.orientMode() == RSPF_RIGHT_HANDED)
      {
         origin.x = theTilingRect.ul().x + col*convertedTilingDistance.x;
         origin.y = theTilingRect.ul().y - row*convertedTilingDistance.y;
      }
      else
      {
         origin.x = theTilingRect.ul().x + col*convertedTilingDistance.x;
         origin.y = theTilingRect.ul().y + row*convertedTilingDistance.y;
      }
   }

   return result;
}

rspfDpt rspfTiling::getDeltaPerPixel()const
{
   
   rspfDpt result = theDelta;

   if (!theMapProjection)
   {
      return result;
   }

   if(theDeltaType == rspfTilingDeltaType_TOTAL_PIXELS)
   {
      result = rspfDpt(theTilingDistance.x/theDelta.x,
                        theTilingDistance.y/theDelta.y);
   }

   rspfUnitConversionTool unitConverter(theMapProjection->origin(),
                                         1.0,
                                         theTilingDistanceUnitType);
   if(theMapProjection->isGeographic())
   {
      unitConverter.setValue(result.x, theTilingDistanceUnitType);
      result.x = unitConverter.getValue(RSPF_DEGREES);
      
      unitConverter.setValue(result.y,
                             theTilingDistanceUnitType);
      result.y = unitConverter.getValue(RSPF_DEGREES);
   }
   else
   {
      unitConverter.setValue(result.x,
                             theTilingDistanceUnitType);
      result.x = unitConverter.getValue(RSPF_METERS);
      
      unitConverter.setValue(result.y,
                             theTilingDistanceUnitType);
      result.y = unitConverter.getValue(RSPF_METERS);
   }

   return result;
}

void rspfTiling::getTileName(rspfString& resultingName,
                              rspf_int64 row,
                              rspf_int64 col,
                              rspf_int64 id)const
{
   if(theTileNameMask == "")
   {
      ostringstream idString;
      idString << id;
      resultingName = rspfString("tile") + idString.str().c_str();
   }
   else if(theTileNameMask.contains("%SRTM%"))
   {
      // must be a 1x1 degree tiling distance
      //
      if((theTilingDistanceUnitType == RSPF_DEGREES)&&
         (theTilingDistance.x == 1.0)&&
         (theTilingDistance.x == 1.0))
      {
         ostringstream latString;
         ostringstream lonString;
         
         resultingName = "";
         rspfDpt origin;
         getOrigin(origin, row, col);
         // the name is lower left and not upper left 
         // subtract one degree
         //
         origin.lat -=1;
         latString << setfill('0') << setw(2) << fabs(origin.lat);
         lonString << setfill('0') << setw(3) << fabs(origin.lon);
         
         if(origin.lat < 0.0)
         {
            resultingName += "S";
         }
         else 
         {
            resultingName += "N";
         }
         resultingName += latString.str().c_str();
         if(origin.lon < 0.0)
         {
            resultingName += "W";
         }
         else
         {
            resultingName += "E";
         }
         resultingName += lonString.str().c_str();
         resultingName += ".hgt";
      }
   }
   else
   {
      ostringstream rowString;
      ostringstream colString;
      ostringstream idString;
      ostringstream oRowString;
      ostringstream oColString;
      rspfDpt      origin;
      getOrigin(origin, row, col);
      
      rowString << row;
      colString << col;
      idString  << id;
      oRowString << row;
      oColString << col;
      
      resultingName = theTileNameMask;
      resultingName = resultingName.substitute("%r%",
                                               rowString.str().c_str());
      resultingName = resultingName.substitute("%c%",
                                               colString.str().c_str());
      resultingName = resultingName.substitute("%i%",
                                               idString.str().c_str());
      resultingName = resultingName.substitute("%oc",
                                               oColString.str().c_str());
      resultingName = resultingName.substitute("%or",
                                               oRowString.str().c_str());
   }
}

void rspfTiling::clampGeographic(rspfDrect& rect)const
{
   rspfDpt ul = rect.ul();
   rspfDpt lr = rect.lr();

   if(ul.lat > 90) ul.lat = 90.0;
   if(lr.lat < -90.0) lr.lat = -90.0;

   if(ul.lon < -180) ul.lon = -180;
   if(lr.lon > 180)  lr.lon = 180;

   rect = rspfDrect(ul, lr, rect.orientMode());
}

bool rspfTiling::saveState(rspfKeywordlist& kwl,
                            const char* prefix)const
{
   std::stringstream tilingDistance;

   tilingDistance << theTilingDistance.x << " " << theTilingDistance.y << std::endl;
   
   kwl.add(prefix,
           "tiling_distance",
           tilingDistance.str().c_str(),
           true);

   
   kwl.add(prefix,
           "tiling_distance_type",
           rspfUnitTypeLut::instance()->
           getTableIndexString(theTilingDistanceUnitType),
           true);
   std::ostringstream delta;
   delta << theDelta.x << " " << theDelta.y;
   kwl.add(prefix,
           "delta",
           delta.str().c_str(),
           true);
   
   if(theDeltaType == rspfTilingDeltaType_TOTAL_PIXELS)
   {
      kwl.add(prefix,
              "delta_type",
              "total_pixels",
              true);
   }
   else
   {
      kwl.add(prefix,
              "delta_type",
              "delta_per_pixels",
              true);
   }
   std::ostringstream padding;

   padding << thePaddingSizeInPixels.x << " " << thePaddingSizeInPixels.y;
   kwl.add(prefix,
           "padding_size_in_pixels",
           padding.str().c_str(),
           true);

   kwl.add(prefix,
           "tile_name_mask",
           theTileNameMask.c_str(),
           true);

   if (theOutputSizeInBytes)
   {
      kwl.add(prefix,
              "output_size_in_bytes",
              rspfString::toString(theOutputSizeInBytes).c_str(),
              true);
      kwl.add(prefix,
              "output_bands",
              theNumberOfBands,
              true);
      kwl.add(prefix,
              "output_bytes_per_pixel_per_band",
              theNumberOfBytesPerPixelPerBand,
              true);
   }

   kwl.add(prefix,
           "edge_to_edge",
           rspfString::toString(theEdgeToEdgeFlag),
           true);
   
   
   return true;
}

bool rspfTiling::loadState(const rspfKeywordlist& kwl,
                            const char* prefix)
{
   bool result = false;
   
   // Get the mask if any.
   theTileNameMask = kwl.find(prefix, "tile_name_mask");
   setTileNameMask(theTileNameMask);

   const char* paddingSizeInPixels = kwl.find(prefix,
                                              "padding_size_in_pixels");
   if (paddingSizeInPixels)
   {
      rspfString x,y;
      std::istringstream padding(rspfString(paddingSizeInPixels).c_str());
      padding >> x >> y;
      thePaddingSizeInPixels.x = x.toFloat64();
      thePaddingSizeInPixels.y = y.toFloat64();
   }
   
   // Check to tile by size in bytes.
   const char* lookup;
   if (kwl.numberOf(prefix, "output_size_in_"))
   {
      lookup = kwl.find(prefix, "output_size_in_bytes");
      if (lookup)
      {
         theOutputSizeInBytes = rspfString(lookup).toInt64();
      }
      lookup = kwl.find(prefix, "output_size_in_kilo_bytes");
      if (lookup)
      {
         theOutputSizeInBytes = rspfString(lookup).toInt64() * 1024;
      }
      lookup = kwl.find(prefix, "output_size_in_mega_bytes");
      if (lookup)
      {
         theOutputSizeInBytes = rspfString(lookup).toInt64() * 1024 * 1024;
      }
   }

   if (theOutputSizeInBytes)
   {
      
      lookup = kwl.find(prefix, "output_bands");
      if (lookup)
      {
         theNumberOfBands = rspfString(lookup).toInt32();
      }
      lookup = kwl.find(prefix, "output_bytes_per_pixel_per_band");
      if (lookup)
      {
         theNumberOfBytesPerPixelPerBand = rspfString(lookup).toInt32();
      }
      
      rspf_int64 pixelsPerBand = theOutputSizeInBytes /
         (theNumberOfBands * theNumberOfBytesPerPixelPerBand);
      
      rspf_int32 oneDimension =
         static_cast<rspf_int32>(floor(sqrt((double)pixelsPerBand)));

      //---
      // TODO: Clamp to power of two option. (drb)
      //---
      // theDelta.x = oneDimension;
      // theDelta.y = oneDimension;
      // theTilingDistance.x = 1;
      // theTilingDistance.y = 1;
      
      theDelta.x = 1;
      theDelta.y = 1;
      theTilingDistance.x = oneDimension;
      theTilingDistance.y = oneDimension;
      theDeltaType = rspfTilingDeltaType_TOTAL_PIXELS;
      theTilingDistanceUnitType = RSPF_PIXEL;
      result = true;
   }
   else
   {
      const char* tilingDistance = kwl.find(prefix,
                                            "tiling_distance");
      const char* tilingDistanceType = kwl.find(prefix,
                                                "tiling_distance_type");
      const char* delta = kwl.find(prefix,
                                   "delta");
      
      const char* deltaType = kwl.find(prefix,
                                       "delta_type");
      
      result = tilingDistance&&tilingDistanceType&&delta&&deltaType&&paddingSizeInPixels;
      rspfString x,y;
      std::istringstream distance(rspfString(tilingDistance).c_str());
      distance >> x >> y;
      theTilingDistance.x = x.toFloat64();
      theTilingDistance.y = y.toFloat64();
      if(theTilingDistance.x <= 0.0)
      {
         theTilingDistance.x = 1.0;
      }
      if(theTilingDistance.y<=0.0)
      {
         theTilingDistance.y = theTilingDistance.x;
      }
      
      // unitLut.getTableIndexString(theTilingDistanceUnitType), ?????
      
      theTilingDistanceUnitType = (rspfUnitType)
         rspfUnitTypeLut::instance()->getEntryNumber(
            rspfString(tilingDistanceType).c_str());
      
      theDelta = rspfDpt(0,0);
      x="";
      y="";
      std::istringstream deltaValues(rspfString(delta).c_str());
      deltaValues >> x >> y;
      theDelta.x = x.toFloat64();
      theDelta.y = y.toFloat64();
      
      if(theDelta.x <= 0.0)
      {
         theDelta.x = 1.0;
      }
      
      if(theDelta.y <= 0.0)
      {
         theDelta.y = theDelta.x;
      }
      
      if(rspfString(deltaType).downcase().contains("total"))
      {
         theDeltaType = rspfTilingDeltaType_TOTAL_PIXELS;
      }
      else
      {
         theDeltaType = rspfTilingDeltaType_PER_PIXEL;
      }
   }
   
   lookup = kwl.find(prefix, "edge_to_edge");
   if ( lookup )
   {
      theEdgeToEdgeFlag = rspfString::toBool(lookup);
   }
      
   if (traceDebug())
   {
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }

   return result;
}

std::ostream& rspfTiling::print(std::ostream& out) const
{
   out << "theTilingDistance:                 " << theTilingDistance
       << "\ntheTilingDistanceUnitType:       " << theTilingDistanceUnitType
       << "\ntheDelta:                        " << theDelta
       << "\ntheDeltaType:                    " << theDeltaType
       << "\nthePaddingSizeInPixels:          " << thePaddingSizeInPixels
       << "\ntheImageRect:                    " << theImageRect
       << "\ntheTilingRect:                   " << theTilingRect
       << "\ntheTileId:                       " << theTileId
       << "\ntheTotalHorizontalTiles:         " << theTotalHorizontalTiles
       << "\ntheTotalVerticalTiles:           " << theTotalVerticalTiles
       << "\ntheTotalTiles:                   " << theTotalTiles
       << "\ntheTileNameMask:                 " << theTileNameMask
       << "\ntheOutputSizeInBytes:            " << theOutputSizeInBytes
       << "\ntheNumberOfBands:                " << theNumberOfBands
       << "\ntheNumberOfBytesPerPixelPerBand: " <<
      theNumberOfBytesPerPixelPerBand
       << "\ntheEdgeToEdgeFlag:               " << theEdgeToEdgeFlag
       << "\n";

   if (theMapProjection.valid())
   {
      out << "\ntheMapProjection:\n" << theMapProjection->print(out);
   }
   else
   {
      out << "theMapProjection is not set.";
   }
   out << endl;

   return out;
}

void rspfTiling::getConvertedTilingDistance(rspfDpt& pt) const
{
   if(theTilingDistanceUnitType == RSPF_PIXEL)
   {
      pt = theTilingDistance;
      return;
   }
   if (!theMapProjection)
   {
      // This should not happen...
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiling::getConvertedTilingDistance WARNING"
         << " projection not set!" << endl;
      pt = theTilingDistance;
      return;
   }
   rspfUnitConversionTool unitConverter(theMapProjection->origin(),
                                         1.0,
                                         theTilingDistanceUnitType);

   if(theMapProjection->isGeographic())
   {
      unitConverter.setValue(theTilingDistance.x,
                             theTilingDistanceUnitType);
      pt.x = unitConverter.getValue(RSPF_DEGREES);
      
      unitConverter.setValue(theTilingDistance.y,
                             theTilingDistanceUnitType);
      pt.y = unitConverter.getValue(RSPF_DEGREES);
   }
   else
   {
      unitConverter.setValue(theTilingDistance.x,
                             theTilingDistanceUnitType);
      pt.x = unitConverter.getValue(RSPF_METERS);
      
      unitConverter.setValue(theTilingDistance.y,
                             theTilingDistanceUnitType);
      pt.y = unitConverter.getValue(RSPF_METERS);
   }
}
