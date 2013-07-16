//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Class definition of rspfWatermarkFilter.
// Applies an image or watermark to image.  Positioning is based on mode.
// Density is base on alpha weight.
//
//----------------------------------------------------------------------------
// $Id: rspfWatermarkFilter.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <vector>

#include <rspf/imaging/rspfWatermarkFilter.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageDataFactory.h>

#include <rspf/imaging/rspfScalarRemapper.h>


RTTI_DEF1(rspfWatermarkFilter, "rspfWatermarkFilter", rspfImageSourceFilter)

static rspfTrace traceDebug(rspfString("rspfWatermarkFilter:debug"));

// Keywords:
static const char WATERMARK_MODE_KW[] = "watermark_mode";
static const char WEIGHT_KW[]         = "weight";

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfWatermarkFilter.cpp 15766 2009-10-20 12:37:09Z gpotts $";
#endif

const rspf_float64 DEFAULT_WEIGHT = 0.20;

rspfWatermarkFilter::rspfWatermarkFilter()
   :
   theFilename(rspfFilename::NIL),
   theWatermarkWeight(DEFAULT_WEIGHT),
   theTile(NULL),
   theWatermark(NULL),
   theMode(rspfWatermarkFilter::UPPER_LEFT),
   theInputBoundingRect(),
   theDirtyFlag(true)
{
   theEnableFlag = true;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfPixelFlipper::rspfPixelFlipper" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif      
   }
}

rspfWatermarkFilter::~rspfWatermarkFilter()
{
   // Dereference tiles and force a delete if last reference.
   theWatermark = NULL;
   theTile      = NULL;
}

rspfString rspfWatermarkFilter::getShortName() const
{
   return rspfString("Watermark Filter");
}

rspfString rspfWatermarkFilter::getLongName() const
{
   return rspfString("Watermark Filter - Applies watermark image onto getTile Request.");
}

rspfString rspfWatermarkFilter::getDescription() const
{
   rspfString description;
   description = getLongName();
   description += "\n";
   description += "Valid modes:\n";

   vector<rspfString> list;
   getModeList(list);

   vector<rspfString>::const_iterator i = list.begin();
   while (i != list.end())
   {
      description +=(*i);
      description += "\n";
      ++i;
   }

   return description;
}

rspfRefPtr<rspfImageData> rspfWatermarkFilter::getTile(
   const rspfIrect& tile_rect, rspf_uint32 resLevel)
{
   // Lock for the length of this method.
   // Check for input.
   if (!theInputConnection)
   {
      if (theTile.valid())
      {
         theTile->setImageRectangle(tile_rect);
         theTile->makeBlank();
      }
      return theTile;
   }

   // Fetch a tile from from the input source.
   rspfRefPtr<rspfImageData> inputTile =
      theInputConnection->getTile(tile_rect, resLevel);

   // Check for bypass.
   if (theEnableFlag == false) return inputTile;

   // Check for weight being 0.0.
   if (theWatermarkWeight == 0.0) return inputTile;

   //---
   // Check for dirty state.
   // Note: This is set in initialize if something changes.
   //---
   if (theDirtyFlag == true)
   {
      if (allocate() == false) // Something not right if false.
      {
         return inputTile;
      }
   }   
   
   // We will only watermark (process) within the input bounding rectangle.
   if (tile_rect.intersects(theInputBoundingRect) == false)
   {
      return inputTile;
   }

   // Capture the rectangle and blank out theTile.
   theTile->setImageRectangle(tile_rect);

   if (inputTile.valid() &&
       (inputTile->getDataObjectStatus() != RSPF_NULL))
   {
      // Copy the inputTile to theTile.
      theTile->loadTile(inputTile.get());
   }
   else 
   {
      theTile->makeBlank();
   }

   // Write the watermarks...
   switch(theTile->getScalarType())
   {
      case RSPF_UINT8:
      {
         fill(rspf_uint8(0));
         break;
      }
      case RSPF_SINT8:
      {
         fill(rspf_sint8(0));
         break;
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         fill(rspf_uint16(0));
         break;
      }
      case RSPF_SINT16:
      {
         fill(rspf_sint16(0));
         break;
      }
      case RSPF_UINT32:
      {
         fill(rspf_uint32(0));
         break;
      }
      case RSPF_SINT32:
      {
         fill(rspf_sint32(0));
         break;
      }
      case RSPF_FLOAT32: 
      case RSPF_NORMALIZED_FLOAT:
      {
         fill(rspf_float32(0));
         break;
      }
      case RSPF_FLOAT64: 
      case RSPF_NORMALIZED_DOUBLE:
      {
         fill(rspf_float32(0));
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Scalar type = " << theTile->getScalarType()
            << " Not supported by rspfWatermarkFilter" << std::endl;
         return inputTile;
      }
   }

   return theTile;
}

template <class T> void rspfWatermarkFilter::fill(T /* dummy */)
{
   const rspfIrect TILE_RECT = theTile->getImageRectangle();

   // We will only fill data within the input bounding rect.
   const rspfIrect CLIPPED_TILE_RECT =
      TILE_RECT.clipToRect(theInputBoundingRect);

   // Get the bounding rectangles.
   vector<rspfIrect> rects(0);
   getIntersectingRects(rects);

   if (rects.size() == 0)
   {
      return;
   }

   //---
   // Have watermark rectangles that intersect this tile so we need to process.
   //---
   rspf_uint32 band = 0;
   rspf_float64 inputPixWeight = 1.0 - theWatermarkWeight;

   // Get a pointers to the watermark buffers (wmBuf) and nulls wn.
   T** wmBuf = new T*[theWatermarkNumberOfBands];
   for (band = 0; band < theWatermarkNumberOfBands; ++band)
   {
      wmBuf[band] = static_cast<T*>(theWatermark->getBuf(band));
   }
   
   // Get a pointers to the output tile buffers and nulls in.
   T** otBuf = new T*[theInputNumberOfBands];
   for (band = 0; band < theInputNumberOfBands; ++band)
   {
      otBuf[band] = static_cast<T*>(theTile->getBuf(band));
   }

   // Get the width of the buffers for indexing.
   rspf_int32 wmWidth = static_cast<rspf_int32>(theWatermark->getWidth());
   rspf_int32 otWidth = static_cast<rspf_int32>(theTile->getWidth());

   const rspf_float64* wmNull = theWatermark->getNullPix();
   const rspf_float64* otMin  = theTile->getMinPix();
   const rspf_float64* otMax  = theTile->getMaxPix();
   const rspf_float64* otNull = theTile->getNullPix();
   
      
   // Control loop through intersecting rectangles.
   vector<rspfIrect>::const_iterator i = rects.begin();
   while (i != rects.end())
   {
      if ( (*i).intersects(CLIPPED_TILE_RECT) )
      {
         //---
         // This is the rectangle we want to fill relative to requesting
         // image space.
         //---
         const rspfIrect CLIPPED_WATERMARRK_RECT =
            (*i).clipToRect(CLIPPED_TILE_RECT);

         rspf_int32 clipHeight = CLIPPED_WATERMARRK_RECT.height();
         rspf_int32 clipWidth  = CLIPPED_WATERMARRK_RECT.width();

         // Compute the starting offset into the wmBuf and otBuf.
         rspf_int32 wmOffset =
            (CLIPPED_WATERMARRK_RECT.ul().y - (*i).ul().y) * wmWidth +
            CLIPPED_WATERMARRK_RECT.ul().x  - (*i).ul().x;
         rspf_int32 otOffset =
            (CLIPPED_WATERMARRK_RECT.ul().y - TILE_RECT.ul().y)* otWidth +
             CLIPPED_WATERMARRK_RECT.ul().x - TILE_RECT.ul().x;
         
         // Line loop...
         for (rspf_int32 line = 0; line < clipHeight; ++line)
         {
            // Sample loop...
            for (rspf_int32 sample = 0; sample < clipWidth; ++sample)
            {
               // Output band control loop until all output bands are filled.
               rspf_uint32 otBand = 0;
               while (otBand < theInputNumberOfBands)
               {
                  // Band loop through the watermark.
                  for (rspf_uint32 wmBand = 0;
                       wmBand < theWatermarkNumberOfBands;
                       ++wmBand)
                  {
                     if (wmBuf[wmBand][wmOffset+sample] != wmNull[wmBand])
                     {
                        // Apply the weight to the input pixel.
                        rspf_float64 p1 =
                           (otBuf[otBand][otOffset+sample] != otNull[otBand]) ?
                           otBuf[otBand][otOffset+sample] * inputPixWeight :
                           0.0;

                        // Apply the Weight to the watermark pixel.
                        rspf_float64 p2 =
                           wmBuf[wmBand][wmOffset+sample]*theWatermarkWeight;

                        // Add them up.
                        rspf_float64 p3 = p1 + p2;

                        // Cast to output type with range checking.
                        otBuf[otBand][otOffset+sample] = static_cast<T>(
                           ( (p3 >= otMin[otBand]) ?
                             (p3 < otMax[otBand] ? p3 : otMax[otBand]) :
                             otNull[otBand]) );
                     }
                     ++otBand;
                     
                     // We stop when we reach here.  All output bands filled.
                     if (otBand == theInputNumberOfBands)
                     {
                        break;
                     }
                     
                  } // End of band through watermark.
                  
               } // End of outer band loop.
               
            } // End of sample loop.

            wmOffset += wmWidth;
            otOffset += otWidth;
            
         } // End of line loop.

      } // End "if ( (*i).intersects(TILE_RECT) )"
      
      ++i; // Go to next rectangle to fill if any.
      
   } // End of "while (i != rects.end())"

   // Clean up.
   delete [] wmBuf;
   delete [] otBuf;
   
   theTile->validate();
}

void rspfWatermarkFilter::getIntersectingRects(vector<rspfIrect>& rects)
{
   switch(theMode)
   {
      case UPPER_LEFT:
         getUpperLeftRect(rects);
         break;
      case UPPER_CENTER:
         getUpperCenterRect(rects);
         break;
      case UPPER_RIGHT:
         getUpperRightRect(rects);
         break;
      case CENTER:
         getCenterRect(rects);
         break;
      case LOWER_LEFT:
         getLowerLeftRect(rects);
         break;
      case LOWER_CENTER:
         getLowerCenterRect(rects);
         break;
      case LOWER_RIGHT:
         getLowerRightRect(rects);
         break;
      case UNIFORM_DENSE:
         getUniformDenseRects(rects);
         break;
      case UNIFORM_SPARSE:
         getUniformSparceRects(rects);
         break;
      default:
         break;
   }
}

void rspfWatermarkFilter::getUpperLeftRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   theWatermark->setOrigin(theInputBoundingRect.ul());
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
}

void rspfWatermarkFilter::getUpperCenterRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth = theWatermark->getImageRectangle().width();
   rspf_uint32 inputWidth = theInputBoundingRect.width();
   rspfIpt origin = theInputBoundingRect.ul();
   
   // Input wider than watermark so center.
   if (inputWidth > watermarkWidth)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>((inputWidth - watermarkWidth) / 2);
      origin.x = origin.x + offset;
   }
   
   theWatermark->setOrigin(origin);
   
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
}  

void rspfWatermarkFilter::getUpperRightRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth =
      theWatermark->getImageRectangle().width();
   rspf_uint32 inputWidth = theInputBoundingRect.width();
   
   rspfIpt origin = theInputBoundingRect.ul();
   
   // Input wider than watermark so center.
   if (inputWidth > watermarkWidth)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>(inputWidth - watermarkWidth);
      origin.x = origin.x + offset;
   }
   
   theWatermark->setOrigin(origin);
   
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
}  

void rspfWatermarkFilter::getCenterRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth = theWatermark->getImageRectangle().width();
   rspf_uint32 watermarkHeight = theWatermark->getImageRectangle().height();
   rspf_uint32 inputWidth = theInputBoundingRect.width();
   rspf_uint32 inputHeight = theInputBoundingRect.height();
   rspfIpt origin = theInputBoundingRect.ul();
   
   // Input wider than watermark so center.
   if (inputWidth > watermarkWidth)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>((inputWidth - watermarkWidth) / 2);
      origin.x = origin.x + offset;
   }
   // Input higher than watermark so center.
   if (inputHeight > watermarkHeight)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>((inputHeight - watermarkHeight) / 2);
      origin.y = origin.y + offset;
   }
   
   theWatermark->setOrigin(origin);
   
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
}

void rspfWatermarkFilter::getLowerLeftRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkHeight = theWatermark->getImageRectangle().height();
   rspf_uint32 inputHeight = theInputBoundingRect.height();
   rspfIpt origin = theInputBoundingRect.ul();
   
   // Input higher than watermark so apply offset.
   if (inputHeight > watermarkHeight)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>(inputHeight - watermarkHeight);
      origin.y = origin.y + offset;
   }
   
   theWatermark->setOrigin(origin);
   
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
}

void rspfWatermarkFilter::getLowerCenterRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth = theWatermark->getImageRectangle().width();
   rspf_uint32 watermarkHeight = theWatermark->getImageRectangle().height();
   rspf_uint32 inputWidth = theInputBoundingRect.width();
   rspf_uint32 inputHeight = theInputBoundingRect.height();
   rspfIpt origin = theInputBoundingRect.ul();
   
   // Input wider than watermark so center.
   if (inputWidth > watermarkWidth)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>((inputWidth - watermarkWidth) / 2);
      origin.x = origin.x + offset;
   }
   // Input higher than watermark so apply offset.
   if (inputHeight > watermarkHeight)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>(inputHeight - watermarkHeight);
      origin.y = origin.y + offset;
   }
   
   theWatermark->setOrigin(origin);
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
}

void rspfWatermarkFilter::getLowerRightRect(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth = theWatermark->getImageRectangle().width();
   rspf_uint32 watermarkHeight = theWatermark->getImageRectangle().height();
   rspf_uint32 inputWidth = theInputBoundingRect.width();
   rspf_uint32 inputHeight = theInputBoundingRect.height();
   rspfIpt origin = theInputBoundingRect.ul();
   
   // Input wider than watermark so center.
   if (inputWidth > watermarkWidth)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>(inputWidth - watermarkWidth);
      origin.x = origin.x + offset;
   }
   // Input higher than watermark so apply offset.
   if (inputHeight > watermarkHeight)
   {
      rspf_int32 offset =
         static_cast<rspf_int32>(inputHeight - watermarkHeight);
      origin.y = origin.y + offset;
   }
   
   theWatermark->setOrigin(origin);
   rspfIrect r = theWatermark->getImageRectangle();
   if (r.intersects(CLIP_RECT))
   {
      rects.push_back(r);
   }
   
}

void rspfWatermarkFilter::getUniformDenseRects(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth = theWatermark->getImageRectangle().width();
   rspf_uint32 watermarkHeight = theWatermark->getImageRectangle().height();
   rspf_uint32 inputWidth = theInputBoundingRect.width();
   rspf_uint32 inputHeight = theInputBoundingRect.height();

   rspf_uint32 watermarksHigh = inputHeight / watermarkHeight;
   if (inputHeight % watermarkHeight) ++watermarksHigh;
   
   rspf_uint32 watermarksWide = inputWidth / watermarkWidth;
   if (inputWidth % watermarkWidth) ++watermarksWide;

   rspf_int32 xOffset = static_cast<rspf_int32>(watermarkWidth);
   rspf_int32 yOffset = static_cast<rspf_int32>(watermarkHeight);
   
   rspfIpt origin = theInputBoundingRect.ul();
   
   for (rspf_uint32 y = 0; y < watermarksHigh; ++y)
   {
      for (rspf_uint32 x = 0; x < watermarksWide; ++x)
      {
         theWatermark->setOrigin(origin);
         rspfIrect r = theWatermark->getImageRectangle();
         if (r.intersects(CLIP_RECT))
         {
            rects.push_back(r);
         }
         origin.x = origin.x + xOffset;
      }
      origin.y = origin.y + yOffset;
      origin.x = theInputBoundingRect.ul().x;
   }
}

void rspfWatermarkFilter::getUniformSparceRects(vector<rspfIrect>& rects)
{
   // First clip the rect to the bounding image rectangle.
   const rspfIrect CLIP_RECT =
      theTile->getImageRectangle().clipToRect(theInputBoundingRect);
   rspf_uint32 watermarkWidth  = theWatermark->getImageRectangle().width();
   rspf_uint32 watermarkHeight = theWatermark->getImageRectangle().height();
   rspf_uint32 inputWidth      = theInputBoundingRect.width();
   rspf_uint32 inputHeight     = theInputBoundingRect.height();
   rspf_uint32 gapWidth   = watermarkWidth/2;
   rspf_uint32 gapHeight  = watermarkHeight/2;

   rspf_uint32 watermarksHigh = inputHeight/(watermarkHeight+gapHeight);
   if (inputHeight % watermarkHeight) ++watermarksHigh;
   
   rspf_uint32 watermarksWide = inputWidth/(watermarkWidth+gapWidth);
   if (inputWidth % watermarkWidth) ++watermarksWide;

   rspf_int32 xOffset = static_cast<rspf_int32>(watermarkWidth  + gapWidth);
   rspf_int32 yOffset = static_cast<rspf_int32>(watermarkHeight + gapHeight);

   rspfIpt origin = theInputBoundingRect.ul();
   
   for (rspf_uint32 y = 0; y < watermarksHigh; ++y)
   {
      for (rspf_uint32 x = 0; x < watermarksWide; ++x)
      {
         theWatermark->setOrigin(origin);
         rspfIrect r = theWatermark->getImageRectangle();
         if (r.intersects(CLIP_RECT))
         {
            rects.push_back(r);
         }
         origin.x = origin.x + xOffset;
      }
      origin.y = origin.y + yOffset;
      origin.x = theInputBoundingRect.ul().x;
   }
}

void rspfWatermarkFilter::initialize()
{
   //---
   // If state is not already dirty and there is an input connection
   // check for:
   // 
   // 1) Scalar change
   // 2) band number change
   // 3) bounding box change
   //
   // Set state to dirty on a change.
   //
   // NOTE:  This method intentionally only sets the dirty state and doesn't do
   // anything else as it is called repetitively during chain setup or chain
   // state changes.
   //
   // The first getTile call will do the real work, call allocate(),
   // if the state is dirty.
   //---

   // Lock for the length of this method.
   // Set the input connection.
   rspfImageSourceFilter::initialize();

   // Once dirty flag is set no need to do it again.
   if (theDirtyFlag == false)
   {
      if (theInputConnection)
      {
         // Check for scalar type change.
         if (theInputScalarType != theInputConnection->getOutputScalarType())
         {
            theDirtyFlag = true;
            return;
         }

         // Check for band change.
         if (theInputNumberOfBands !=
             theInputConnection->getNumberOfOutputBands())
         {
            theDirtyFlag = true;
            return;
         }

         // Check for bounding rectangle change.
         if ( theInputBoundingRect != theInputConnection->getBoundingRect() )
         {
            theDirtyFlag = true;
            return;
         }
      }
   }
}

bool rspfWatermarkFilter::allocate()
{
   // Capture the bounding rect:
   theInputBoundingRect = theInputConnection->getBoundingRect();

   // Capture the scalar type:
   theInputScalarType = theInputConnection->getOutputScalarType();

   // Capture the number of bands:
   theInputNumberOfBands = theInputConnection->getNumberOfOutputBands();

   //---
   // Check the watermark scalar type.
   //---
   if (theWatermark.valid())
   {
      if (theWatermark->getScalarType() != theInputScalarType)
      {
         //---
         // We'll need to make a new one with a scalar remapper after it.
         //---
         theWatermark = NULL; // We'll need to make a new one.
      }
   }

   //---
   // Make a new watermark tile.  This will do a scalar remap if needed.
   // If we don't have a watermark no point in going on...
   //---
   if (!theWatermark)
   {
      if (openWatermarkFile() == false)
      {
         return false;
      }
   }

   if (theTile.valid())
   {
      if ( (theTile->getScalarType()    != theInputScalarType) ||
           (theTile->getNumberOfBands() != theInputNumberOfBands) )
      {
         theTile = NULL;  // We'll need to make a new one.
      }
   }

   // Make a new output tile if we need to.
   if (!theTile)
   {
      theTile = rspfImageDataFactory::instance()->create(this,
                                                          theInputConnection);
      if (theTile.valid())
      {
         theTile->initialize();
      }
      else
      {
         return false;
      }
   }

   //---
   // If we get here things are good so clear the dirty flag so we don't
   // get called again needlessly.
   //---
   theDirtyFlag = false;
   
   return true;
}

bool rspfWatermarkFilter::openWatermarkFile()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfWatermarkFilter::openWatermarkFile DEBUG: entered..."
         << std::endl;
   }

   if (!theInputConnection || (theFilename == rspfFilename::NIL))
   {
      return false;
   }

   theWatermark = NULL; // This will destroy any previous tiles.

   // Open the watermark image.
   rspfRefPtr<rspfImageHandler> ih =
      rspfImageHandlerRegistry::instance()->open(theFilename);
   if (!ih)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfWatermarkFilter::openWatermarkFile"
            << "\nCould not open: " << theFilename 
            << std::endl;
      }
      
      return false;
   }
   if (ih->getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfWatermarkFilter::openWatermarkFile"
            << "\nError reading image:  " << theFilename
            << std::endl; 
         return false;
      }
   }

   ih->initialize();
   rspfRefPtr<rspfImageSource> imageSource = ih.get();
   rspfRefPtr<rspfScalarRemapper> remapper;
   
   if (ih->getOutputScalarType() != theInputConnection->getOutputScalarType())
   {
      // Remap the watemark to the same scalar type as the input.
      remapper = new rspfScalarRemapper(imageSource.get(),
                                         theInputConnection->
                                         getOutputScalarType());
      remapper->initialize();
      imageSource = remapper.get();
   }
   
   // Get the full image rectangle.
   theWatermark = imageSource->getTile(ih->getImageRectangle(), 0);

   // Cleanup...
   if (remapper.valid())
   {
      remapper->disconnect();
      remapper = NULL;
   }
   if(ih.valid())
   {
      ih->disconnect();
      ih = 0;
   }
   imageSource = 0;
   
   if (theWatermark.valid() == false)
   {
      return false;
   }

   // Capture the bands as we will need this repetitively.
   theWatermarkNumberOfBands = theWatermark->getNumberOfBands();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfWatermarkFilter::openWatermarkFile DEBUG:"
         << *(theWatermark.get())
         << endl;
   }

   return true;
}

bool rspfWatermarkFilter::saveState(rspfKeywordlist& kwl,
                                     const char* prefix) const
{
   kwl.add(prefix,
           rspfKeywordNames::FILENAME_KW,
           theFilename.c_str());
   kwl.add(prefix,
           WATERMARK_MODE_KW,
           getModeString().c_str());
   kwl.add(prefix,
           WEIGHT_KW,
           theWatermarkWeight);

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfWatermarkFilter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   // Do this first so connections get set up.
   if (rspfImageSourceFilter::loadState(kwl, prefix) == false)
   {
      return false;
   }

   const char* lookupReturn;

   lookupReturn = kwl.find(prefix, WEIGHT_KW);
   if(lookupReturn)
   {
      setWeight(rspfString(lookupReturn).toDouble());
   }

   lookupReturn = kwl.find(prefix, WATERMARK_MODE_KW);
   if(lookupReturn)
   {
      setMode(rspfString(lookupReturn));
   }

   lookupReturn = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if(lookupReturn)
   {
      setFilename(lookupReturn);
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfWatermarkFilter::loadState DEBUG:"
         << std::endl;
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }

   return true;
}

rspfRefPtr<rspfProperty> rspfWatermarkFilter::getProperty(
   const rspfString& name) const
{
   // Lock for the length of this method.
   if (name == rspfKeywordNames::FILENAME_KW)
   {
      rspfFilenameProperty* ofp =
         new rspfFilenameProperty(name, theFilename);
      ofp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      ofp->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>((rspfProperty*)ofp);
   }
   else if (name == WATERMARK_MODE_KW)
   {
      vector<rspfString> constraintList;
      getModeList(constraintList);

      rspfStringProperty* p =
         new rspfStringProperty(name,
                                 getModeString(),
                                 false,
                                 constraintList);
       p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   else if (name == WEIGHT_KW)
   {
      rspfProperty* p =
         new rspfNumericProperty(name,
                                  rspfString::toString(theWatermarkWeight));
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }

   return rspfImageSourceFilter::getProperty(name);
}

void rspfWatermarkFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if (!property) return;

   rspfString os = property->valueToString();
   
   rspfString name = property->getName();
   if (name == rspfKeywordNames::FILENAME_KW)
   {
      setFilename(os);
   }
   else if  (name == WATERMARK_MODE_KW)
   {
      setMode(os);
   }
   else if  (name == WEIGHT_KW)
   {
      setWeight(os.toDouble());
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

void rspfWatermarkFilter::getPropertyNames(
   std::vector<rspfString>& propertyNames) const
{
   propertyNames.push_back(rspfKeywordNames::FILENAME_KW);
   propertyNames.push_back(WATERMARK_MODE_KW);
   propertyNames.push_back(WEIGHT_KW);
   rspfImageSourceFilter::getPropertyNames(propertyNames);
}

void rspfWatermarkFilter::getModeList(vector<rspfString>& list) const
{
   list.clear();
   list.resize(rspfWatermarkFilter::END);
   list[0] = rspfString("upper_left");
   list[1] = rspfString("upper_center");
   list[2] = rspfString("upper_right");
   list[3] = rspfString("center");
   list[4] = rspfString("lower_left");
   list[5] = rspfString("lower_center");
   list[6] = rspfString("lower_right");
   list[7] = rspfString("uniform_dense");
   list[8] = rspfString("uniform_sparse");
}

rspfWatermarkFilter::WatermarkMode rspfWatermarkFilter::getMode() const
{
   return theMode;
}

rspfString rspfWatermarkFilter::getModeString() const
{
   switch(theMode)
   {
      case UPPER_LEFT:
         return rspfString("upper_left");
      case UPPER_CENTER:
         return rspfString("upper_center");
      case UPPER_RIGHT:
         return rspfString("upper_right");
      case CENTER:
         return rspfString("center");
      case LOWER_LEFT:
         return rspfString("lower_left");
      case LOWER_CENTER:
         return rspfString("lower_center");
      case LOWER_RIGHT:
         return rspfString("lower_right");
      case UNIFORM_DENSE:
         return rspfString("uniform_dense");
      case UNIFORM_SPARSE:
         return rspfString("uniform_sparse");
      default:
         break;
   }

   return rspfString("UNKNOWN_MODE");
}

void rspfWatermarkFilter::setFilename(const rspfFilename& file)
{
   if (file != theFilename)
   {
      theFilename = file;
      theWatermark = NULL; // Will be reallocated next getTile.
      theDirtyFlag = true;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfWatermarkFilter::setFilename DEBUG:" << std::endl;
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
}

void rspfWatermarkFilter::setMode(const rspfString& mode)
{
   rspfString os = mode;
   os.downcase();
   
   if (os == "upper_left")
   {
      theMode = UPPER_LEFT;
   }
   else if (os == "upper_center")
   {
      theMode = UPPER_CENTER;
   }
   else if (os == "upper_right")
   {
      theMode = UPPER_RIGHT;
   }
   else if (os == "center")
   {
      theMode = CENTER;
   }
   else if (os == "lower_left")
   {
      theMode = LOWER_LEFT;
   }
   else if (os == "lower_center")
   {
      theMode = LOWER_CENTER;
   }
   else if (os == "lower_right")
   {
      theMode = LOWER_RIGHT;
   }
   else if (os == "uniform_dense")
   {
      theMode = UNIFORM_DENSE;
   }
   else if (os == "uniform_sparse")
   {
      theMode = UNIFORM_SPARSE;
   }
   else
   {
      // Invalid mode...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfWatermarkFilter::setMode DEBUG:"
            << "\nInvalid mode!  " << mode
            << std::endl;
      }
   }
}

void rspfWatermarkFilter::setWeight(rspf_float64 weight)
{
   if ( (weight >= 0.0) && (weight <= 1.0) )
   {
      theWatermarkWeight = weight;
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfWatermarkFilter::setWeight DEBUG:" << std::endl;
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   
}

std::ostream& rspfWatermarkFilter::print(std::ostream& out) const
{
   out << "rspfWatermarkFilter::print"
       << "\ntheFilename:        " << theFilename
       << "\ntheWatermarkWeight: " << theWatermarkWeight
       << "\ntheMode:            " << getModeString()
       << std::endl;
   return rspfImageSourceFilter::print(out);
}

