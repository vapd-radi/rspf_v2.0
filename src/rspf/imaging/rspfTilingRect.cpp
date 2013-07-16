//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Mingjie Su
// 
// Description: implementation for image generator
//
//*************************************************************************
// $Id: rspfTilingRect.cpp 2725 2011-06-15 18:13:07Z david.burken $

#include <sstream>
#include <iomanip>
#include <rspf/imaging/rspfTilingRect.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfEsriShapeFileInterface.h>

static rspfTrace traceDebug("rspfTilingRect:debug");

RTTI_DEF1(rspfTilingRect, "rspfTilingRect", rspfTiling);

rspfTilingRect::rspfTilingRect()
      : rspfTiling(),
      m_clipToAoi(true),
      m_lastWidth(0),
      m_lastHeight(0),
      m_useOffset(false),
      m_offsetInPixels(0,0),
      m_tilingDistanceInPixels(0,0),
      m_tilinResolutionDistance(rspf::nan(),rspf::nan()),
      m_tilingResolutionUnitType(RSPF_UNIT_UNKNOWN),
      m_fractionalPixelMisalignment(rspf::nan(),rspf::nan())
{
}

rspfTilingRect::~rspfTilingRect()
{
}

bool rspfTilingRect::initializeBase(const rspfMapProjection& proj,
   const rspfIrect& boundingRect,
   rspfDpt& convertedTilingDistance)
{
   theMapProjection = (rspfMapProjection*)proj.dup();
   theImageRect     = boundingRect;

   rspfDpt gsd = theMapProjection->getMetersPerPixel();
   // Find image center and GSD at that point:
   if (theMapProjection->isGeographic() && theTilingDistanceUnitType != RSPF_DEGREES)
   {
      rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry (0, theMapProjection.get());
      geom->setImageSize(rspfIpt(theImageRect.width(), theImageRect.height()));
      gsd = geom->getMetersPerPixel();
   }

   m_tilingDistanceInPixels = theTilingDistance;

   //if resolution and resolution type are defined, need to reset degree/meter per pixel and the image rect. do calculation below
   rspf_float64 latDiff = 0.0;
   rspf_float64 lonDiff = 0.0;
   rspf_uint32 imageWidth = 0;
   rspf_uint32 imageHeight = 0;
   if (!m_tilinResolutionDistance.hasNans())
   {
      rspfDpt result(0,0);
      if (m_tilingResolutionUnitType == RSPF_DEGREES)
      {
         rspfGpt ulg;
         rspfGpt llg;
         rspfGpt lrg;
         rspfGpt urg;

         theMapProjection->lineSampleToWorld(theImageRect.ul(), ulg);
         theMapProjection->lineSampleToWorld(theImageRect.ll(), llg);
         theMapProjection->lineSampleToWorld(theImageRect.lr(), lrg);
         theMapProjection->lineSampleToWorld(theImageRect.ur(), urg);

         latDiff = fabs(ulg.lat - llg.lat);
         lonDiff = fabs(urg.lon - llg.lon);

         if (!theMapProjection->isGeographic())
         {
            rspf_float64 tileHorizatalNum = lonDiff/m_tilinResolutionDistance.x;
            rspf_float64 tileVerticalNum = latDiff/m_tilinResolutionDistance.y;

            if (theTilingDistanceUnitType != RSPF_PIXEL) //convert tiling distance to pixel if unit type is not pixel
            {
               rspfDpt convertedDis = m_tilingDistanceInPixels;
               getConvertedDistance(convertedDis, m_tilingDistanceInPixels);
               imageWidth = (rspf_uint32)convertedDis.x/gsd.x*tileHorizatalNum;
               imageHeight = (rspf_uint32)convertedDis.y/gsd.y*tileVerticalNum;
            }
            else
            {
               imageWidth = (rspf_uint32)m_tilingDistanceInPixels.x*tileHorizatalNum;
               imageHeight = (rspf_uint32)m_tilingDistanceInPixels.y*tileVerticalNum;
            }
            result = rspfDpt(theImageRect.width()*gsd.x/imageWidth, theImageRect.height()*gsd.y/imageHeight);
            gsd = result;

            theMapProjection->setMetersPerPixel(result);
         }
         else
         {
            result = rspfDpt(m_tilinResolutionDistance.x/m_tilingDistanceInPixels.x,
               m_tilinResolutionDistance.y/m_tilingDistanceInPixels.y);

            theMapProjection->setDecimalDegreesPerPixel(result);
         }
      }
      else //only handle degree type for resolution
      {
         return false;
      }
   }

   if (theTilingDistanceUnitType != RSPF_PIXEL)
   {
      rspfDpt paddingSize = getPaddingSize();//convert padding size to meters or degree
      getConvertedDistance(convertedTilingDistance, theTilingDistance);

      rspfDpt convertedOffset = m_offsetInPixels;
      if (m_useOffset)
      {
         getConvertedDistance(convertedOffset, m_offsetInPixels);
      }

      if (theMapProjection->isGeographic() && theTilingDistanceUnitType == RSPF_DEGREES)
      {
         thePaddingSizeInPixels.x = rspf::round<int>(paddingSize.x / theMapProjection->getDecimalDegreesPerPixel().x);
         thePaddingSizeInPixels.y = rspf::round<int>(paddingSize.y / theMapProjection->getDecimalDegreesPerPixel().y);

         double intpart;
         m_fractionalPixelMisalignment.x = modf(convertedTilingDistance.x / theMapProjection->getDecimalDegreesPerPixel().x, &intpart);
         m_fractionalPixelMisalignment.y = modf(convertedTilingDistance.y / theMapProjection->getDecimalDegreesPerPixel().y, &intpart);

         m_tilingDistanceInPixels.x = rspf::round<int>(convertedTilingDistance.x / theMapProjection->getDecimalDegreesPerPixel().x);
         m_tilingDistanceInPixels.y = rspf::round<int>(convertedTilingDistance.y / theMapProjection->getDecimalDegreesPerPixel().y);

         if (m_useOffset)
         {
            m_offsetInPixels.x = rspf::round<int>(convertedOffset.x / theMapProjection->getDecimalDegreesPerPixel().x);
            m_offsetInPixels.y = rspf::round<int>(convertedOffset.y / theMapProjection->getDecimalDegreesPerPixel().y);
         }
      }
      else
      {
         thePaddingSizeInPixels.x = rspf::round<int>(paddingSize.x / gsd.x);
         thePaddingSizeInPixels.y = rspf::round<int>(paddingSize.y / gsd.y);

         double intpart;
         m_fractionalPixelMisalignment.x = modf(convertedTilingDistance.x / gsd.x, &intpart);
         m_fractionalPixelMisalignment.y = modf(convertedTilingDistance.y / gsd.y, &intpart);

         m_tilingDistanceInPixels.x = rspf::round<int>(convertedTilingDistance.x / gsd.x);
         m_tilingDistanceInPixels.y = rspf::round<int>(convertedTilingDistance.y / gsd.y);

         if (m_useOffset)
         {
            m_offsetInPixels.x = rspf::round<int>(convertedOffset.x / gsd.x);
            m_offsetInPixels.y = rspf::round<int>(convertedOffset.y / gsd.y);
         }
      }
   }
   else
   {
      if (theMapProjection->isGeographic())
      {
         theTilingDistance.x = theTilingDistance.x * theMapProjection->getDecimalDegreesPerPixel().x;
         theTilingDistance.y = theTilingDistance.y * theMapProjection->getDecimalDegreesPerPixel().y;
      }
      else
      {
         theTilingDistance.x = theTilingDistance.x * gsd.x;
         theTilingDistance.y = theTilingDistance.y * gsd.y;
      }
   }

   //re-calculation the image rect if resolution and resolution type defined
   if (!m_tilinResolutionDistance.hasNans())
   {
      if (theMapProjection->isGeographic())
      {
         imageWidth = (rspf_uint32)m_tilingDistanceInPixels.x * lonDiff;
         imageHeight = (rspf_uint32)m_tilingDistanceInPixels.y * latDiff;
      }

      //avoid very small pixel offset by calculation if tiling distance unit type is pixel
      if (theTilingDistanceUnitType == RSPF_PIXEL)
      {
         if (imageWidth % (rspf_int32)m_tilingDistanceInPixels.x != 0)
         {
            rspf_int32 numX =  rspf::round<int>((double)imageWidth/m_tilingDistanceInPixels.x);
            imageWidth = m_tilingDistanceInPixels.x * numX;
         }
         if (imageHeight % (rspf_int32)m_tilingDistanceInPixels.y != 0)
         {
            rspf_int32 numY =  rspf::round<int>((double)imageHeight/m_tilingDistanceInPixels.y);
            imageHeight = m_tilingDistanceInPixels.y * numY;
         }
      }
    
      theImageRect = rspfIrect(boundingRect.ul(),
         rspfIpt((boundingRect.ul().x + imageWidth), boundingRect.ul().y),
         rspfIpt((boundingRect.ul().x + imageWidth), (imageHeight - boundingRect.ul().y)),
         rspfIpt(boundingRect.ul().x , (imageHeight - boundingRect.ul().y)));
   }
   return true;
}

bool rspfTilingRect::initialize(const rspfMapProjection& proj,
   const rspfIrect& boundingRect)
{
   bool result = false;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTilingRect::initialize DEBUG: Entered..."
         << "\nBounding rect === " << boundingRect
         << std::endl;
   }

   rspfDpt convertedTilingDistance = theTilingDistance;
   result = initializeBase(proj, boundingRect, convertedTilingDistance);
   rspfDrect tilingRectInPixels = theImageRect;

   // Let's setup the bounding tile rect.
   if(theMapProjection->isGeographic())
   {
      rspfGpt ul;
      rspfGpt ur;
      rspfGpt lr;
      rspfGpt ll;

      theMapProjection->lineSampleToWorld(theImageRect.ul(), ul);
      theMapProjection->lineSampleToWorld(theImageRect.ur(), ur);
      theMapProjection->lineSampleToWorld(theImageRect.lr(), lr);
      theMapProjection->lineSampleToWorld(theImageRect.ll(), ll);

      theTilingRect = rspfDrect(rspfDpt(ul), rspfDpt(ur), rspfDpt(lr), rspfDpt(ll),
         RSPF_RIGHT_HANDED);

      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG: Geographic input bounds =  "
            << theTilingRect
            << std::endl;
      }

      if (!m_clipToAoi)
      {
         theTilingRect.stretchToTileBoundary(convertedTilingDistance);
      }

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

      theMapProjection->lineSampleToEastingNorthing(theImageRect.ul(), ul);
      theMapProjection->lineSampleToEastingNorthing(theImageRect.ur(), ur);
      theMapProjection->lineSampleToEastingNorthing(theImageRect.lr(), lr);
      theMapProjection->lineSampleToEastingNorthing(theImageRect.ll(), ll);
      theTilingRect = rspfDrect(ul, ur, lr, ll, RSPF_RIGHT_HANDED);

      if (!m_clipToAoi)
      {
         theTilingRect.stretchToTileBoundary(convertedTilingDistance);
      }
   }

   theTotalHorizontalTiles = rspf::round<int>(fabs(tilingRectInPixels.ur().x - tilingRectInPixels.ul().x)/m_tilingDistanceInPixels.x);
   theTotalVerticalTiles   = rspf::round<int>(fabs(tilingRectInPixels.ur().y - tilingRectInPixels.lr().y)/m_tilingDistanceInPixels.y);

   if (m_clipToAoi)
   {
      if (m_useOffset)
      {
         rspf_float32 totalHorizontalTilesAfterOffset = 
            ((fabs(tilingRectInPixels.ur().x - tilingRectInPixels.ul().x))-m_tilingDistanceInPixels.x)/(m_tilingDistanceInPixels.x - m_offsetInPixels.x) + 1;
         rspf_float32 totalVerticalTilesAfterOffset = 
            ((fabs(tilingRectInPixels.ur().y - tilingRectInPixels.lr().y))-m_tilingDistanceInPixels.y)/(m_tilingDistanceInPixels.y - m_offsetInPixels.y) + 1;

         rspf_float32 lastHorizontal = totalHorizontalTilesAfterOffset - (rspf_int32)totalHorizontalTilesAfterOffset;
         rspf_float32 lastVertical = totalVerticalTilesAfterOffset - (rspf_int32)totalVerticalTilesAfterOffset;

         m_lastWidth = rspf::round<int>(fabs(lastHorizontal* (m_tilingDistanceInPixels.x - m_offsetInPixels.x) + m_offsetInPixels.x));
         m_lastHeight = rspf::round<int>(fabs(lastVertical* (m_tilingDistanceInPixels.y - m_offsetInPixels.y) + m_offsetInPixels.y));

         theTotalHorizontalTiles = rspf::round<int>(totalHorizontalTilesAfterOffset);
         theTotalVerticalTiles = rspf::round<int>(totalVerticalTilesAfterOffset);

         if (m_lastWidth > 0 && (lastHorizontal < 0.5))//if lastHorizontal is less than 0.5, the last tile will not add to total tiles after rounding, do here
         {
            theTotalHorizontalTiles = theTotalHorizontalTiles + 1;
         }
         if (m_lastHeight > 0 && (lastVertical < 0.5))
         {
            theTotalVerticalTiles = theTotalVerticalTiles + 1;
         }
      }
      else
      {
         rspf_int32 tmpX = (rspf_int32)(fabs(tilingRectInPixels.ur().x - tilingRectInPixels.ul().x)) + 1;
         rspf_int32 tmpY = (rspf_int32)(fabs(tilingRectInPixels.ur().y - tilingRectInPixels.lr().y)) + 1;

         rspf_int32 modValueX = tmpX % (rspf_int32)m_tilingDistanceInPixels.x;
         rspf_int32 modValueY = tmpY % (rspf_int32)m_tilingDistanceInPixels.y;
         if (modValueX >= 1)
         {
            m_lastWidth = modValueX;
         }
         else
         {
            m_lastWidth = m_tilingDistanceInPixels.x;
         }
         if (modValueY >= 1)
         {
            m_lastHeight = modValueY;
         }
         else
         {
            m_lastHeight = m_tilingDistanceInPixels.y;
         }

         if (m_lastWidth > 0)
         {  
            if (m_lastWidth < m_tilingDistanceInPixels.x/2)
            {
               theTotalHorizontalTiles = theTotalHorizontalTiles + 1;
            }
         }
         if (m_lastHeight > 0)
         {
            if (m_lastHeight < m_tilingDistanceInPixels.y/2)
            {
               theTotalVerticalTiles = theTotalVerticalTiles + 1;
            }
         }
      }
   }

   theTotalTiles = theTotalHorizontalTiles*theTotalVerticalTiles;

   // reset the tile id to the beginning
   reset();

   if(traceDebug())
   {
      print(rspfNotify(rspfNotifyLevel_DEBUG));

      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfTiling::initialize DEBUG: Leaving..." << std::endl;
   }

   return result;
}

bool rspfTilingRect::next(rspfRefPtr<rspfMapProjection>& resultProjection,
   rspfIrect& resultingBounds,
   rspfString& resultingName)
{
   bool result = true;

   if(theTileId < theTotalTiles)
   {
      rspfDpt origin;
      rspf_int64 row=0;
      rspf_int64 col=0;

      getRowCol(row, col, theTileId);

      getTileName(resultingName,
         (row+1), (col+1), theTileId);

      getOrigin(origin, row, col);

      if (!m_fractionalPixelMisalignment.isNan())
      {
         if (row == 0 && col != 0)
         {
            if (m_fractionalPixelMisalignment.x >= 0.5)
            {
               if (theMapProjection->isGeographic())
               {
                  origin.x = origin.x + (1 - m_fractionalPixelMisalignment.x) * theMapProjection->getDecimalDegreesPerPixel().x * col;
               }
               else
               {
                  origin.x = origin.x + (1 - m_fractionalPixelMisalignment.x) * theMapProjection->getMetersPerPixel().x * col;
               }
            }  
            else
            {
               if (theMapProjection->isGeographic())
               {
                  origin.x = origin.x - m_fractionalPixelMisalignment.x * theMapProjection->getDecimalDegreesPerPixel().x * col;
               }
               else
               {
                  origin.x = origin.x - m_fractionalPixelMisalignment.x * theMapProjection->getMetersPerPixel().x * col;
               }
            }
         }
         else if (col == 0 && row != 0)
         {
            if (m_fractionalPixelMisalignment.y >= 0.5)
            {
               if (theMapProjection->isGeographic())
               {
                  origin.y = origin.y - (1 - m_fractionalPixelMisalignment.y) * theMapProjection->getDecimalDegreesPerPixel().y * row;
               }
               else
               {
                  origin.y = origin.y - (1 - m_fractionalPixelMisalignment.y) * theMapProjection->getMetersPerPixel().y * row;
               }
            }
            else
            {
               if (theMapProjection->isGeographic())
               {
                  origin.y = origin.y + m_fractionalPixelMisalignment.y * theMapProjection->getDecimalDegreesPerPixel().y * row;
               }
               else
               {
                  origin.y = origin.y + m_fractionalPixelMisalignment.y * theMapProjection->getMetersPerPixel().y * row;
               }  
            }
         }
         else if (col != 0 && row != 0)
         {
            if (m_fractionalPixelMisalignment.x >= 0.5)
            {
               if (theMapProjection->isGeographic())
               {
                  origin.x = origin.x + (1 - m_fractionalPixelMisalignment.x) * theMapProjection->getDecimalDegreesPerPixel().x * col;
               }
               else
               {
                  origin.x = origin.x + (1 - m_fractionalPixelMisalignment.x) * theMapProjection->getMetersPerPixel().x * col;
               }
            }  
            else
            {
               if (theMapProjection->isGeographic())
               {
                  origin.x = origin.x - m_fractionalPixelMisalignment.x * theMapProjection->getDecimalDegreesPerPixel().x * col;
               }
               else
               {
                  origin.x = origin.x - m_fractionalPixelMisalignment.x * theMapProjection->getMetersPerPixel().x * col;
               }
            }

            if (m_fractionalPixelMisalignment.y >= 0.5)
            {
               if (theMapProjection->isGeographic())
               {
                  origin.y = origin.y - (1 - m_fractionalPixelMisalignment.y) * theMapProjection->getDecimalDegreesPerPixel().y * row;
               }
               else
               {
                  origin.y = origin.y - (1 - m_fractionalPixelMisalignment.y) * theMapProjection->getMetersPerPixel().y * row;
               }
            }
            else
            {
               if (theMapProjection->isGeographic())
               {
                  origin.y = origin.y + m_fractionalPixelMisalignment.y * theMapProjection->getDecimalDegreesPerPixel().y * row;
               }
               else
               {
                  origin.y = origin.y + m_fractionalPixelMisalignment.y * theMapProjection->getMetersPerPixel().y * row;
               }  
            }
         }
      }

      rspfIpt pixels = rspfDpt(m_tilingDistanceInPixels.x, m_tilingDistanceInPixels.y);

      if(theMapProjection->isGeographic())
      {
         theMapProjection->setUlTiePoints(
            rspfGpt(origin.lat,
            origin.lon,
            0.0,
            theMapProjection->origin().datum()));
      }
      else
      {
         theMapProjection->setUlTiePoints(rspfDpt(origin.x, origin.y));
      }

      if (m_clipToAoi || m_useOffset)
      {
         if (row == 0 && col == 0)
         {
            resultingBounds = rspfIrect(0, 0,
               (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
               (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);
         }
         else if (row == 0 && col != 0)
         {
            if (col == (theTotalHorizontalTiles-1))
            {
               resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x, 0,
                  (m_lastWidth-1),
                  (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);

               if (m_useOffset)
               {
                  rspf_int32 lr_x = -((rspf_int32)m_offsetInPixels.x*col)- resultingBounds.width();
                  resultingBounds = rspfIrect(-((rspf_int32)m_offsetInPixels.x*col), 0,
                     lr_x, (resultingBounds.height()-1));
               }
            }
            else
            {
               resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x, 0,
                  (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
                  (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);

               if (m_useOffset)
               {
                  resultingBounds = rspfIrect(-((rspf_int32)m_offsetInPixels.x*col), 0,
                     (resultingBounds.width()-1)-((rspf_int32)m_offsetInPixels.x*col), 
                     (resultingBounds.height()-1));
               }
            }
         }
         else if (row != 0 && col == 0)
         {
            if (row == (theTotalVerticalTiles-1))
            {
               resultingBounds = rspfIrect(0, -(rspf_int32)thePaddingSizeInPixels.y,
                  (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
                  (m_lastHeight-1));

               if (m_useOffset)
               {
                  rspf_int32 lr_y = -((rspf_int32)m_offsetInPixels.y*row)-resultingBounds.height();
                  resultingBounds = rspfIrect(0, -((rspf_int32)m_offsetInPixels.y*row),
                     (resultingBounds.width()-1), lr_y);
               }
            }
            else
            {
               resultingBounds = rspfIrect(0,
                  -(rspf_int32)thePaddingSizeInPixels.y,
                  (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
                  (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);

               if (m_useOffset)
               {
                  resultingBounds = rspfIrect(0, -((rspf_int32)m_offsetInPixels.y*row),
                     (resultingBounds.width()-1), 
                     (resultingBounds.height()-1)-((rspf_int32)m_offsetInPixels.y*row));
               }
            }
         }
         else if (row != 0 && col != 0)
         {
            if (row == (theTotalVerticalTiles-1) && col == (theTotalHorizontalTiles-1))
            {
               resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x,
                  -(rspf_int32)thePaddingSizeInPixels.y,
                  (m_lastWidth-1),
                  (m_lastHeight-1));

               if (m_useOffset)
               {
                  rspf_int32 lr_x = -((rspf_int32)m_offsetInPixels.x*col)-resultingBounds.width();
                  rspf_int32 lr_y = -((rspf_int32)m_offsetInPixels.y*row)-resultingBounds.height();
                  resultingBounds = rspfIrect(-((rspf_int32)m_offsetInPixels.x*col), 
                     -((rspf_int32)m_offsetInPixels.y*row),
                     lr_x, lr_y);
               }
            }
            else if (row != (theTotalVerticalTiles-1) && col == (theTotalHorizontalTiles-1))
            {
               resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x,
                  -(rspf_int32)thePaddingSizeInPixels.y,
                  (m_lastWidth-1),
                  (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);

               if (m_useOffset)
               {
                  rspf_int32 lr_x = -((rspf_int32)m_offsetInPixels.x*col)-resultingBounds.width();
                  resultingBounds = rspfIrect(-((rspf_int32)m_offsetInPixels.x*col), 
                     -((rspf_int32)m_offsetInPixels.y*row),
                     lr_x, (resultingBounds.height()-1)-((rspf_int32)m_offsetInPixels.y*row));
               }
            }
            else if (row == (theTotalVerticalTiles-1) && col != (theTotalHorizontalTiles-1))
            {
               resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x,
                  -(rspf_int32)thePaddingSizeInPixels.y,
                  (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
                  (m_lastHeight-1));

               if (m_useOffset)
               {
                  rspf_int32 lr_y = -((rspf_int32)m_offsetInPixels.y*row)-resultingBounds.height();
                  resultingBounds = rspfIrect(-((rspf_int32)m_offsetInPixels.x*col), 
                     -((rspf_int32)m_offsetInPixels.y*row),
                     (resultingBounds.width()-1)-((rspf_int32)m_offsetInPixels.x*col), 
                     lr_y);
               }
            }
            else
            {
               resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x,
                  -(rspf_int32)thePaddingSizeInPixels.y,
                  (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
                  (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);

               if (m_useOffset)
               {
                  resultingBounds = rspfIrect(-((rspf_int32)m_offsetInPixels.x*col), 
                     -((rspf_int32)m_offsetInPixels.y*row),
                     (resultingBounds.width()-1)-((rspf_int32)m_offsetInPixels.x*col), 
                     (resultingBounds.height()-1)-((rspf_int32)m_offsetInPixels.y*row));
               }
            }
         }
      }
      else
      {
         resultingBounds = rspfIrect(-(rspf_int32)thePaddingSizeInPixels.x,
            -(rspf_int32)thePaddingSizeInPixels.y,
            (pixels.x-1) + (rspf_int32)thePaddingSizeInPixels.x,
            (pixels.y-1) + (rspf_int32)thePaddingSizeInPixels.y);

         if (m_useOffset)
         {
            resultingBounds = rspfIrect(-(rspf_int32)m_offsetInPixels.x, 
               -(rspf_int32)m_offsetInPixels.y,
               (resultingBounds.width()-1), (resultingBounds.height()-1));
         }
      }

      if(traceDebug()&&resultProjection.valid())
      {
         rspfKeywordlist projKwl;

         resultProjection->saveState(projKwl);
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfTilingRect::next DEBUG: rect = " << resultingBounds
            << "\nproj                          = " << projKwl
            << "\ntileName                      = " << resultingName
            << "\norigin                        = " << origin << std::endl;
      }

      resultProjection = theMapProjection;

      ++theTileId; 
   }
   else
   {
      result = false;
   }

   return result;
}

bool rspfTilingRect::saveState(rspfKeywordlist& kwl,
                            const char* prefix)const
{
   std::stringstream tilingDistance;

   tilingDistance << theTilingDistance.x << " " << theTilingDistance.y << std::endl;
   
   kwl.add(prefix,
           "tile_size",
           tilingDistance.str().c_str(),
           true);

   kwl.add(prefix,
           "units",
           rspfUnitTypeLut::instance()->
           getTableIndexString(theTilingDistanceUnitType),
           true);
   
   std::ostringstream padding;

   padding << thePaddingSizeInPixels.x << " " << thePaddingSizeInPixels.y;
   kwl.add(prefix,
           "padding",
           padding.str().c_str(),
           true);

   kwl.add(prefix,
      "output_file_name",
      theTileNameMask.c_str(),
      true);

   kwl.add(prefix,
      "clip_to_aoi",
      m_clipToAoi,
      true);

   return true;
}

bool rspfTilingRect::loadState(const rspfKeywordlist& kwl,
                            const char* prefix)
{
   bool result = false;
   
   // Get the mask if any.
   theTileNameMask = kwl.find(prefix, "output_file_name");
   if (!theTileNameMask.contains("%f%")) //%f% format is used for feature id in a shape file
   {
      setTileNameMask(theTileNameMask);
   }
  
   const char* paddingSizeInPixels = kwl.find(prefix, "padding");

   if (paddingSizeInPixels)
   {
      rspfString x,y;
      std::istringstream padding(rspfString(paddingSizeInPixels).c_str());
      padding >> x >> y;
      thePaddingSizeInPixels.x = x.toFloat64();
      thePaddingSizeInPixels.y = y.toFloat64();
   }
  
   const char* tilingDistance = kwl.find(prefix, "tile_size");

   const char* tilingDistanceType = kwl.find(prefix, "units");
   
   result = tilingDistance&&tilingDistanceType&&paddingSizeInPixels;
   
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

   theTilingDistanceUnitType = (rspfUnitType)
      rspfUnitTypeLut::instance()->getEntryNumber(
      rspfString(tilingDistanceType).c_str());

   const char* clipToAoi = kwl.find(prefix, "clip_to_aoi");
   if (clipToAoi)
   {
      if (!rspfString(clipToAoi).toBool())
      {
         m_clipToAoi = false;
      }
   }

   const char* offset = kwl.find(prefix, "use_offset");
   if (offset)
   {
      rspfString width, height;
      std::istringstream distanceOffset(rspfString(offset).c_str());
      distanceOffset >> width >> height;
      m_offsetInPixels.x = width.toFloat64();
      m_offsetInPixels.y = height.toFloat64();
      if (m_offsetInPixels.x > 0 || m_offsetInPixels.y > 0.0)
      {
         m_useOffset = true;
         result = true;
      }
   }

   const char* tileRes = kwl.find(prefix, "resolution");
   if (tileRes)
   {
      theDelta = rspfDpt(0,0);
      x="";
      y="";
      std::istringstream tileResValues(rspfString(tileRes).c_str());
      tileResValues >> x >> y;

      m_tilinResolutionDistance.x = x.toFloat64();
      m_tilinResolutionDistance.y = y.toFloat64();
   }
 
   const char* tilingResType = kwl.find(prefix, "resolution_units");
   if (tilingResType)
   {
      m_tilingResolutionUnitType = (rspfUnitType)
         rspfUnitTypeLut::instance()->getEntryNumber(
         rspfString(tilingResType).c_str());
   }

   if (traceDebug())
   {
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   return result;
}

rspfDpt rspfTilingRect::getPaddingSize()const
{
   rspfDpt result = thePaddingSizeInPixels;

   if (!theMapProjection.valid())
      return result;

   if(theMapProjection->isGeographic() && theTilingDistanceUnitType == RSPF_DEGREES)
   {
      return result;
   }
   else
   {
      rspfUnitConversionTool unitConverter(1.0, theTilingDistanceUnitType);
      unitConverter.setValue(result.x, theTilingDistanceUnitType);
      result.x = unitConverter.getValue(RSPF_METERS);

      unitConverter.setValue(result.y, theTilingDistanceUnitType);
      result.y = unitConverter.getValue(RSPF_METERS);
   }

   return result;
}

void rspfTilingRect::getConvertedDistance(rspfDpt& pt, rspfDpt inputDistance) const
{
   if (!theMapProjection)
   {
      // This should not happen...
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiling::getConvertedDistance WARNING"
         << " projection not set!" << endl;
      pt = inputDistance;
      return;
   }

   if(theMapProjection->isGeographic() && theTilingDistanceUnitType == RSPF_DEGREES)
   {
      pt = inputDistance;
   }
   else
   {
      rspfUnitConversionTool unitConverter(theMapProjection->origin(),
         1.0,
         theTilingDistanceUnitType);

      unitConverter.setValue(inputDistance.x,
         theTilingDistanceUnitType);
      pt.x = unitConverter.getValue(RSPF_METERS);

      unitConverter.setValue(inputDistance.y,
         theTilingDistanceUnitType);
      pt.y = unitConverter.getValue(RSPF_METERS);
   }
}
