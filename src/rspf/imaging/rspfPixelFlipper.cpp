//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Filter to toggle pixel values.
//
//*************************************************************************
// $Id: rspfPixelFlipper.cpp 21631 2012-09-06 18:10:55Z dburken $


#include <rspf/imaging/rspfPixelFlipper.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfImageData.h>
#include <OpenThreads/ScopedLock>
#include <cstdlib>

RTTI_DEF1(rspfPixelFlipper, "rspfPixelFlipper", rspfImageSourceFilter)

static rspfTrace traceDebug("rspfPixelFlipper:debug");

const char rspfPixelFlipper::PF_TARGET_VALUE_KW[]      = "target_value";
const char rspfPixelFlipper::PF_TARGET_RANGE_KW[]      = "target_range";
const char rspfPixelFlipper::PF_REPLACEMENT_VALUE_KW[] = "replacement_value";
const char rspfPixelFlipper::PF_REPLACEMENT_MODE_KW[]  = "replacement_mode";
const char rspfPixelFlipper::PF_CLAMP_VALUE_KW[]       = "clamp_value"; // deprecated by clamp_value_hi
const char rspfPixelFlipper::PF_CLAMP_VALUE_LO_KW[]    = "clamp_value_lo";
const char rspfPixelFlipper::PF_CLAMP_VALUE_HI_KW[]    = "clamp_value_hi";
const char rspfPixelFlipper::PF_CLIP_MODE_KW[]         = "border_clip_mode";

static const char TARGET_LOWER_LIMIT_PROP_NAME[] = "target_range_lower_limit";
static const char TARGET_UPPER_LIMIT_PROP_NAME[] = "target_range_upper_limit";

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfPixelFlipper.cpp 21631 2012-09-06 18:10:55Z dburken $";
#endif

rspfPixelFlipper::rspfPixelFlipper(rspfObject* owner)
   :
      rspfImageSourceFilter(owner),
      theTargetValueLo(0.0),
      theTargetValueHi(0.0),
      theReplacementValue(1.0),
      theReplacementMode(rspfPixelFlipper::REPLACE_BAND_IF_TARGET),
      theClampValueLo(rspf::nan()),
      theClampValueHi(rspf::nan()),
      theClampingMode(DISABLED),
      theClipMode(NONE)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfPixelFlipper::rspfPixelFlipper entered...\n";
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << "\n";
#endif      
   }
   
   setDescription(rspfString("Pixel Flipper"));
   enableSource();
}

rspfPixelFlipper::~rspfPixelFlipper()
{}

rspfRefPtr<rspfImageData> rspfPixelFlipper::getTile(
   const rspfIrect& tile_rect, rspf_uint32 resLevel)
{

   if(!theInputConnection)
   {
      return 0;
   }

   // Fetch tile from pointer from the input source.
   rspfRefPtr<rspfImageData> inputTile =
      theInputConnection->getTile(tile_rect, resLevel);
   
   if (!inputTile.valid() || !isSourceEnabled()) return inputTile;
   
   if (!inputTile->getBuf()) return inputTile;
   
   // Lock for the length of this method.
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);
   
   // Call the appropriate load method.
   switch (inputTile->getScalarType())
   {
      
      case RSPF_UCHAR:
      {
         flipPixels(rspf_uint8(0), inputTile.get(), resLevel);
         break;
      }
      
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         flipPixels(rspf_uint16(0), inputTile.get(), resLevel);
         break;
      }
      
      case RSPF_SSHORT16:
      {
         flipPixels(rspf_sint16(0), inputTile.get(), resLevel);
         break;
      }
      case RSPF_UINT32:
      {
         flipPixels(rspf_uint32(0), inputTile.get(), resLevel);
         break;
      }
      case RSPF_SINT32:
      {
         flipPixels(rspf_sint32(0), inputTile.get(), resLevel);
         break;
      }
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         flipPixels(float(0), inputTile.get(), resLevel);
         break;
      }
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         flipPixels(rspf_float64(0), inputTile.get(), resLevel);
         break;
      }
      
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfPixelFlipper::getTile Unsupported scalar type!" << endl;
         break;
      }
   }
   
   inputTile->validate();
   return inputTile;
}

template <class T>
void rspfPixelFlipper::flipPixels(T /* dummy */,
                                   rspfImageData* inputTile,
                                   rspf_uint32 resLevel)
{
   if (!inputTile) return;

   T targetLo    = static_cast<T>(theTargetValueLo);
   T targetHi    = static_cast<T>(theTargetValueHi);
   T replacement = static_cast<T>(theReplacementValue);
   T clampLo       = static_cast<T>(theClampValueLo);
   T clampHi       = static_cast<T>(theClampValueHi);

   // Get pointers to data for each band.
   rspf_uint32 bands = inputTile->getNumberOfBands();
   rspf_uint32 band;
   T** buf = new T*[bands];
   for(band=0; band<bands; ++band)
      buf[band] = static_cast<T*>(inputTile->getBuf(band));

   rspfIrect rect = inputTile->getImageRectangle();
   rspfIpt ul = rect.ul();
   rspfIpt lr = rect.lr();

   // Check the relation between tile rect and the area of interest (either bounding rect or
   // valid vertices polygons). If completely outside, we can return with empty buffer.get If
   // partially inside, we'll need to test individual pixels.
   bool is_outside_aoi = false;
   bool needsTesting = false;
   if ((theClipMode == BOUNDING_RECT) && (resLevel < theBoundingRects.size()))
   {
      if(!rect.intersects(theBoundingRects[resLevel]))
         is_outside_aoi = true;
      else
         needsTesting = !rect.completely_within(theBoundingRects[resLevel]);
   }
   else if ((theClipMode == VALID_VERTICES) && (resLevel < theValidVertices.size()))
   {
      bool ulFlag = theValidVertices[resLevel].isPointWithin(rect.ul());
      bool urFlag = theValidVertices[resLevel].isPointWithin(rect.ur());
      bool lrFlag = theValidVertices[resLevel].isPointWithin(rect.lr());
      bool llFlag = theValidVertices[resLevel].isPointWithin(rect.ll());
      if((!ulFlag) && (!urFlag) && (!lrFlag) && (!llFlag))
         is_outside_aoi = true;
      else
         needsTesting = !(ulFlag && urFlag && lrFlag && llFlag);
   }
   if (is_outside_aoi)
   {
      // none of the tile is inside so just return with empty tile:
      delete [] buf;
      return; 
   }

   rspf_uint32 i = 0;  // index into band buffers;
   rspfIpt pixel_loc; 
   bool can_replace, found_candidate;

   // Begin loop over each pixel in the tile. The individual bands are handled inside this loop:
   for(pixel_loc.y = ul.y; pixel_loc.y <= lr.y; ++pixel_loc.y)
   {
      for(pixel_loc.x = ul.x; pixel_loc.x <= lr.x; ++pixel_loc.x)
      {
         // First consider if we need to test the pixel for border clipping:
         if (needsTesting)
         {
            bool is_inside = true; // Assume it will pass the border test
            if (theClipMode == BOUNDING_RECT)
               is_inside = theBoundingRects[resLevel].pointWithin(pixel_loc);
            else if (theClipMode == VALID_VERTICES)
               is_inside = theValidVertices[resLevel].isPointWithin(pixel_loc);

            if (!is_inside)
            {
               // Remap this pixel to the replacement value (all bands)
               for (band=0; band<bands; ++band)
                  buf[band][i] = replacement;

               // Proceed to next pixel location:
               ++i;
               continue; 
            }
         }

         // If clamping specified, the target replacement function is disabled:
         if (theClampingMode)
         {
            switch (theReplacementMode)
            {
            case REPLACE_BAND_IF_TARGET:
            case REPLACE_ALL_BANDS_IF_ANY_TARGET:
               for (band=0; band<bands; ++band)
               {
                  if (!rspf::isnan(theClampValueLo) && (buf[band][i] < clampLo))
                     buf[band][i] = clampLo;
                  else if (!rspf::isnan(theClampValueHi) && (buf[band][i] > clampHi))
                     buf[band][i] = clampHi;
               }
               break;

            case REPLACE_BAND_IF_PARTIAL_TARGET: 
            case REPLACE_ALL_BANDS_IF_PARTIAL_TARGET:
               // First band loop to establish if pixel qualifies for replacement (at least one 
               // band must be valid):
               can_replace = false;
               found_candidate = false;
               for (band=0; (band < bands) && !(can_replace && found_candidate); ++band)
               {
                  if ((!rspf::isnan(theClampValueLo) && (buf[band][i] < clampLo)) ||
                     (!rspf::isnan(theClampValueHi) && (buf[band][i] > clampHi)))
                     found_candidate = true;
                  else
                     can_replace = true;
               }
               if (can_replace && found_candidate)
               {
                  // This pixel has at least one band with normal value, so need to rescan bands
                  // to find pixels that need replacing (are within the target range):
                  for (band=0; band<bands; ++band)
                  {
                     if (!rspf::isnan(theClampValueLo) && (buf[band][i] < clampLo))
                        buf[band][i] = clampLo;
                     else if (!rspf::isnan(theClampValueHi) && buf[band][i] > clampHi)
                        buf[band][i] = clampHi;
                  }
               }
               break;

            case REPLACE_ONLY_FULL_TARGETS:
               // First band loop to establish if pixel qualifies for replacement (all 
               // bands must be in target range):
               can_replace = true;
               for (band=0; (band < bands) && can_replace; ++band)
               {
                  if ((rspf::isnan(theClampValueLo) || (buf[band][i] >= clampLo)) &&
                      (rspf::isnan(theClampValueHi) || (buf[band][i] <= clampHi)))
                     can_replace = false;
               }
               if (can_replace)
               {
                  // Map all pixels to replacement value: 
                  for (band=0; band<bands; ++band)
                  {
                     if (!rspf::isnan(theClampValueLo) && (buf[band][i] < clampLo))
                        buf[band][i] = clampLo;
                     else if (!rspf::isnan(theClampValueHi) && buf[band][i] > clampHi)
                        buf[band][i] = clampHi;
                  }
               }
               break;
            } // close switch

            // Proceed to next pixel location:
            ++i;
            continue; 
         }

         // If we got here (the continue statement was not reached) then
         // the pixel value now needs to be checked for possible target replacement:
         switch (theReplacementMode)
         {
         case REPLACE_BAND_IF_TARGET:
            for (band=0; band<bands; ++band)
            {
               if ((buf[band][i] >= targetLo) && (buf[band][i] <=targetHi)) 
                  buf[band][i] = theReplacementValue;
            }
            break;

         case REPLACE_BAND_IF_PARTIAL_TARGET: 

            // First band loop to establish if pixel qualifies for replacement (at least one 
            // band must be valid):
            can_replace = false;
            found_candidate = false;
            for (band=0; (band < bands) && !(can_replace && found_candidate); ++band)
            {
               //  check for target range replacement qualification:
               if ((buf[band][i] < targetLo) || (buf[band][i] > targetHi))
                  can_replace = true; // Has valid band
               else
                  found_candidate = true; // found band within target range
            }
            if (can_replace && found_candidate)
            {
               // This pixel has at least one band with normal value, so need to rescan bands
               // to find pixels that need replacing (are within the target range):
               for (band=0; band<bands; ++band)
               {
                  if ((buf[band][i] >= targetLo) && (buf[band][i] <= targetHi)) 
                     buf[band][i] = theReplacementValue;
               }
            }
            break;

         case REPLACE_ALL_BANDS_IF_PARTIAL_TARGET:

            // First band loop to establish if pixel qualifies for replacement (at least one 
            // band must be valid):
            can_replace = false;
            found_candidate = false;
            for (band=0; (band < bands) && !(can_replace && found_candidate); ++band)
            {
               // check for target range replacement qualification:
               if ((buf[band][i] < targetLo) || (buf[band][i] > targetHi))
                  can_replace = true;
               else
                  found_candidate = true;
            }
            if (can_replace && found_candidate)
            {
               // This pixel has at least one band with normal value and one with target, so 
               // map all bands to target:
               for (band=0; band<bands; ++band)
                  buf[band][i] = theReplacementValue;
            }
            break;

         case REPLACE_ONLY_FULL_TARGETS:

            // First band loop to establish if pixel qualifies for replacement (all 
            // bands must be in target range):
            can_replace = true;
            for (band=0; (band < bands) && can_replace; ++band)
            {
               // check for target range replacement qualification:
               if ((buf[band][i] < targetLo) || (buf[band][i] > targetHi))
                  can_replace = false;
            }
            if (can_replace)
            {
               // Map all pixels to replacement value: 
               for (band=0; band<bands; ++band)
                  buf[band][i] = theReplacementValue;
            }
            break;

         case REPLACE_ALL_BANDS_IF_ANY_TARGET:

            // First band loop to establish if pixel qualifies for replacement (all 
            // bands must be in target range):
            can_replace = false;
            for (band=0; (band < bands) && !can_replace; ++band)
            {
               // check for target range replacement qualification:
               if ((buf[band][i] >= targetLo) && (buf[band][i] <= targetHi))
                  can_replace = true;
            }
            if (can_replace)
            {
               // Map all pixels to replacement value: 
               for (band=0; band<bands; ++band)
                  buf[band][i] = theReplacementValue;
            }
            break;
         } // close switch

         // Reached end of processing for one pixel location. Increment the band buffers index:
         ++i;
      } // end of loop over pixel_loc.x
   } // end of loop over pixel_loc.y
   
   delete [] buf;
   inputTile->validate();
}

template <class T> void rspfPixelFlipper::clipTile(T /* dummy */,
                                                    rspfImageData* inputTile,
                                                    rspf_uint32 resLevel)
{
   if(theClipMode == NONE)
   {
      theClipTileBuffer = 0;
      return;
   }
   if(!inputTile) return;
   if(!inputTile->getBuf()||
      (inputTile->getDataObjectStatus() == RSPF_EMPTY))
   {
      return;
   }
   allocateClipTileBuffer(inputTile);


   if(theClipTileBuffer.valid())
   {
      rspfIrect tileRect = inputTile->getImageRectangle();
      // force to all nulls
      theClipTileBuffer->setDataObjectStatus(RSPF_FULL);
      theClipTileBuffer->makeBlank();
      
      switch(theClipMode)
      {
         case NONE:
         {
            break;
         }
         case BOUNDING_RECT:
         {
            if(resLevel < theBoundingRects.size())
            {
               if(tileRect.completely_within(theBoundingRects[resLevel])||
                  theBoundingRects[resLevel].hasNans())
               {
                  return;
               }
               else
               {
                  if(tileRect.intersects(theBoundingRects[resLevel]))
                  {
                     rspfIrect clipRect = tileRect.clipToRect(theBoundingRects[resLevel]);

                     theClipTileBuffer->setImageRectangle(clipRect);
                     
                     theClipTileBuffer->loadTile(inputTile);
                     inputTile->makeBlank();
                     inputTile->loadTile(theClipTileBuffer.get());
                     inputTile->validate();
                  }
                  else
                  {
                     inputTile->makeBlank();
                  }
               }
            }
            break;
         }
         case VALID_VERTICES:
         {
            if(resLevel < theValidVertices.size())
            {
               const rspfPolygon& p = theValidVertices[resLevel];
               bool ulWithin = p.isPointWithin(tileRect.ul());
               bool urWithin = p.isPointWithin(tileRect.ur());
               bool lrWithin = p.isPointWithin(tileRect.lr());
               bool llWithin = p.isPointWithin(tileRect.ll());
               
               if(ulWithin&&
                  urWithin&&
                  lrWithin&&
                  llWithin)
               {
                  return;
               }
               else if(!(ulWithin|| // if none are in
                         urWithin||
                         lrWithin||
                         llWithin))
               {
                  inputTile->makeBlank();
                  return;
               }
               else
               {
                  rspfIpt ul = tileRect.ul();
                  rspfIpt origin;
                  rspf_uint32 x = 0;
                  rspf_uint32 y = 0;
                  rspf_uint32 w = inputTile->getWidth();
                  rspf_uint32 h = inputTile->getHeight();
                  rspf_uint32 offset = 0;
                  origin.y = ul.y;
                  for(y = 0; y < h; ++y)
                  {
                     origin.x = ul.x;
                     for(x = 0; x < w; ++x)
                     {
                        if(!p.isPointWithin(origin))
                        {
                           inputTile->setNull(offset);
                        }
                        ++offset;
                        ++origin.x;
                     }
                     ++origin.y;
                  }
               }
            }
            break;
         }
      }
   }
}


void rspfPixelFlipper::allocateClipTileBuffer(rspfRefPtr<rspfImageData> inputImage)
{
   if(inputImage.valid())
   {
      bool needDupFlag = false;
      if(theClipTileBuffer.valid())
      {
         if((theClipTileBuffer->getScalarType() != inputImage->getScalarType())||
            theClipTileBuffer->getNumberOfBands() != theClipTileBuffer->getNumberOfBands())
         {
            needDupFlag = true;
         }
         else
         {
            theClipTileBuffer->setImageRectangle(inputImage->getImageRectangle());
         }
      }
      else
      {
         needDupFlag = true;
      }
      if(needDupFlag)
      {
         theClipTileBuffer = (rspfImageData*)inputImage->dup();
      }
   }
}

void rspfPixelFlipper::initialize()
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);
   rspfImageSourceFilter::initialize();
   theValidVertices.clear();
   theBoundingRects.clear();

   rspf_uint32 rlevels = getNumberOfDecimationLevels();
   rspf_uint32 idx = 0;

   if(rlevels)
   {
      if(theValidVertices.size() != rlevels)
      {
         theValidVertices.resize(rlevels);
      }
      if(theBoundingRects.size() != rlevels)
      {
         theBoundingRects.resize(rlevels);
      }
      for(idx = 0; idx < rlevels; ++idx)
      {
         std::vector<rspfIpt> validVertices;
         getValidImageVertices(validVertices,
                               RSPF_CLOCKWISE_ORDER,
                               idx);
         theValidVertices[idx] = rspfPolygon(validVertices);
         theBoundingRects[idx] = getBoundingRect(idx);
      }
   }
}

rspfScalarType rspfPixelFlipper::getOutputScalarType() const
{
   if(theInputConnection)
   {
      rspfScalarType scalar = theInputConnection->getOutputScalarType();
      {
         if (scalar == RSPF_USHORT16 && theClampValueHi == 2047.0)
         {
            //---
            // Special case:
            // We have an unsigned 16 bit type but we want to call it
            // USHORT11 ( (2^11- 1) = 2047 ).
            //---
            return RSPF_USHORT11;
         }
         return scalar;
      }
   }
   return RSPF_SCALAR_UNKNOWN;
}

rspf_float64 rspfPixelFlipper::getMaxPixelValue (rspf_uint32 band) const
{
   const rspf_float64 MIN = rspfImageSourceFilter::getMinPixelValue(band);
   const rspf_float64 MAX = rspfImageSourceFilter::getMaxPixelValue(band);

   if ((theClampValueHi > MIN) && (theClampValueHi < MAX))
      return theClampValueHi;

   return MAX;
}

rspf_float64 rspfPixelFlipper::getMinPixelValue (rspf_uint32 band) const
{
   const rspf_float64 MIN = rspfImageSourceFilter::getMinPixelValue(band);
   const rspf_float64 MAX = rspfImageSourceFilter::getMaxPixelValue(band);

   if ((theClampValueLo > MIN) && (theClampValueLo < MAX))
      return theClampValueLo;

   return MIN;
}

bool rspfPixelFlipper::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   const char* lookupReturn;
   
   lookupReturn = kwl.find(prefix, PF_TARGET_VALUE_KW);
   if(lookupReturn)
   {
      setTargetValue(atof(lookupReturn));
   }

   lookupReturn = kwl.find(prefix, PF_TARGET_RANGE_KW);
   if(lookupReturn)
   {
      rspfString min_max_string (lookupReturn);
      rspfString separator (" ");
      rspf_float64 min_target = min_max_string.before(separator).toFloat64();
      rspf_float64 max_target = min_max_string.after(separator).toFloat64();
      setTargetRange(min_target, max_target);
   }

   lookupReturn = kwl.find(prefix, PF_REPLACEMENT_VALUE_KW);
   if(lookupReturn)
   {
      setReplacementValue(atof(lookupReturn));
   }

   lookupReturn = kwl.find(prefix, PF_REPLACEMENT_MODE_KW);
   if(lookupReturn)
   {
      rspfString modeString = lookupReturn;
      setReplacementMode(modeString);
   }

   lookupReturn = kwl.find(prefix, PF_CLAMP_VALUE_KW);
   if(lookupReturn)
   {
      setClampValue(atof(lookupReturn), true);
   }

   lookupReturn = kwl.find(prefix, PF_CLAMP_VALUE_LO_KW);
   if(lookupReturn)
   {
      setClampValue(atof(lookupReturn), false);
   }

   lookupReturn = kwl.find(prefix, PF_CLAMP_VALUE_HI_KW);
   if(lookupReturn)
   {
      setClampValue(atof(lookupReturn), true);
   }

   lookupReturn = kwl.find(prefix, PF_CLIP_MODE_KW);
   if(lookupReturn)
   {
      rspfString modeString = lookupReturn;
      setClipMode(modeString);
   }

   bool status = rspfImageSourceFilter::loadState(kwl, prefix);

   if (traceDebug())
   {
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }

   return status;
}

bool rspfPixelFlipper::saveState(rspfKeywordlist& kwl,
                                 const char* prefix) const
{
   // Call the base class saveState.
   rspfImageSourceFilter::saveState(kwl, prefix);

   if (theTargetValueHi != theTargetValueLo)
   {
      rspfString s (rspfString::toString(theTargetValueLo) + " " + 
         rspfString::toString(theTargetValueHi));
      kwl.add(prefix, PF_TARGET_RANGE_KW, s);
   }
   else
   {
      kwl.add(prefix, PF_TARGET_VALUE_KW, theTargetValueLo);
   }
   kwl.add(prefix, PF_REPLACEMENT_VALUE_KW, theReplacementValue);
   kwl.add(prefix, PF_REPLACEMENT_MODE_KW,  getReplacementModeString().c_str());

   if (theClampingMode)
   {
      kwl.add(prefix, PF_CLAMP_VALUE_LO_KW,    theClampValueLo);
      kwl.add(prefix, PF_CLAMP_VALUE_HI_KW,    theClampValueHi);
   }
   kwl.add(prefix, PF_CLIP_MODE_KW,  getClipModeString().c_str());

   return true;
}

void rspfPixelFlipper::setTargetValue(rspf_float64 target_value)
{
   //---
   // Since this is the value to replace we will allow for any value as it
   // won't affect the output null, min and max ranges.  This will fix a
   // tiled nitf with max of 2047(11bit) with edge tile fill values of 2048.
   //---
   OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);

   theTargetValueLo = target_value;
   theTargetValueHi = target_value; 
}

void rspfPixelFlipper::setTargetRange(rspf_float64 target_min, rspf_float64 target_max)
{
   //---
   // Since this is the value to replace we will allow for any value as it
   // won't affect the output null, min and max ranges.  This will fix a
   // tiled nitf with max of 2047(11bit) with edge tile fill values of 2048.
   //---
   OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);

   theTargetValueLo = target_min;
   theTargetValueHi = target_max; 
}

void rspfPixelFlipper::setReplacementValue(rspf_float64 replacement_value)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);

   // Range check to ensure within null, min and max of output radiometry.
   if (inRange(replacement_value))
   {
      theReplacementValue = replacement_value;
   }
}

void rspfPixelFlipper::setClampValue(rspf_float64 clamp_value, bool clamp_max_value)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);

   if (inRange(clamp_value))
   {
      // Stupid MS compiler complains if we do an |= on enum type. (OLK 1/11)
      int temp_int = (int) theClampingMode;
      if (clamp_max_value)
      {
         theClampValueHi = clamp_value;
         temp_int |= (int) CLAMPING_HI;
      }
      else
      {
         theClampValueLo = clamp_value;
         temp_int |= (int) CLAMPING_LO;
      }
      theClampingMode = (ClampingMode) temp_int;
   }
}

void rspfPixelFlipper::setClampValues(rspf_float64 clamp_value_lo, rspf_float64 clamp_value_hi)
{
   theClampingMode = DISABLED; // reset and let next calls set accordingly
   setClampValue(clamp_value_lo, false);
   setClampValue(clamp_value_hi, true);
}

void rspfPixelFlipper::setReplacementMode(rspfPixelFlipper::ReplacementMode mode)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);
   theReplacementMode = mode;
}

bool rspfPixelFlipper::setReplacementMode(const rspfString& modeString)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);

   rspfString mode = modeString;
   mode.upcase();
   if (mode == "REPLACE_BAND_IF_TARGET")
   {
      theReplacementMode = REPLACE_BAND_IF_TARGET;
   }
   else if (mode == "REPLACE_BAND_IF_PARTIAL_TARGET")
   {
      theReplacementMode = REPLACE_BAND_IF_PARTIAL_TARGET;
   }
   else if (mode == "REPLACE_ALL_BANDS_IF_PARTIAL_TARGET")
   {
      theReplacementMode = REPLACE_ALL_BANDS_IF_PARTIAL_TARGET;
   }
   else if (mode == "REPLACE_ONLY_FULL_TARGETS")
   {
      theReplacementMode = REPLACE_ONLY_FULL_TARGETS;
   }
   else if (mode == "REPLACE_ALL_BANDS_IF_ANY_TARGET")
   {
      theReplacementMode = REPLACE_ALL_BANDS_IF_ANY_TARGET;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfPixelFlipper::setReplacementMode warning:\n"
         << "Invalid mode:  " << modeString
         << endl;
      return false;
   }
   return true;
}

void rspfPixelFlipper::setClipMode(rspfPixelFlipper::ClipMode mode)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);
   theClipMode = mode;
}

void rspfPixelFlipper::setClipMode(const rspfString& modeString)
{
   rspfString mode = modeString;
   mode.downcase();
   if (mode == "none")
   {
      setClipMode(NONE);
   }
   else if (mode == "bounding_rect")
   {
      setClipMode(BOUNDING_RECT);
   }
   else if (mode == "valid_vertices")
   {
      setClipMode(VALID_VERTICES);
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfPixelFlipper::setClipMode warning:\n"
         << "Invalid mode:  " << modeString
         << endl;
   }
}

//rspf_float64 rspfPixelFlipper::getTargetValue() const
//{
//   return theTargetValueLo;
//}

rspf_float64 rspfPixelFlipper::getReplacementValue() const
{
   return theReplacementValue;
}

rspfPixelFlipper::ReplacementMode rspfPixelFlipper::getReplacementMode()  const
{
   return theReplacementMode;
}
   
rspfString rspfPixelFlipper::getReplacementModeString()  const
{
   switch(theReplacementMode)
   {
      case REPLACE_BAND_IF_TARGET:
         return rspfString("REPLACE_BAND_IF_TARGET");
      case REPLACE_BAND_IF_PARTIAL_TARGET:
         return rspfString("REPLACE_BAND_IF_PARTIAL_TARGET");
      case REPLACE_ALL_BANDS_IF_PARTIAL_TARGET:
         return rspfString("REPLACE_ALL_BANDS_IF_PARTIAL_TARGET");
      case REPLACE_ONLY_FULL_TARGETS:
         return rspfString("REPLACE_ONLY_FULL_TARGETS");
      case REPLACE_ALL_BANDS_IF_ANY_TARGET:
         return rspfString("REPLACE_ALL_BANDS_IF_ANY_TARGET");
      default:
         break;
   }

   return rspfString("UNKNOWN_MODE");
}

rspfString rspfPixelFlipper::getClipModeString()  const
{
   if (theClipMode == BOUNDING_RECT)
      return rspfString("bounding_rect");

   if (theClipMode == VALID_VERTICES)
      return rspfString("valid_vertices");

   return rspfString("none");
}

rspfPixelFlipper::ClipMode rspfPixelFlipper::getClipMode() const
{
   return theClipMode;
}

std::ostream& rspfPixelFlipper::print(std::ostream& out) const
{
   out << "rspfPixelFlipper::print:"
       << "\ntarget value Lo:    " << theTargetValueLo
       << "\ntarget value Hi:    " << theTargetValueHi
       << "\nreplacement value:  " << theReplacementValue
       << "\nclamp value Lo:     " << theClampValueLo
       << "\nclamp value Hi:     " << theClampValueHi
       << "\nreplacement mode:   " << getReplacementModeString().c_str()
       << "\nclip_mode:          " << getClipModeString().c_str()
      << endl;
   return out;
}

rspfRefPtr<rspfProperty> rspfPixelFlipper::getProperty(
   const rspfString& name)const
{
   // Lock for the length of this method.
	OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(theMutex);

   if (name == PF_TARGET_VALUE_KW)
   {
      rspfProperty* p =
         new rspfNumericProperty(name,
         rspfString::toString(theTargetValueLo));
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   if (name == PF_TARGET_RANGE_KW)
   {
      rspfProperty* p =
         new rspfNumericProperty(name,
         rspfString::toString(theTargetValueLo));
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   if (name == TARGET_UPPER_LIMIT_PROP_NAME)
   {
      rspfProperty* p =
         new rspfNumericProperty(name,
         rspfString::toString(theTargetValueHi));
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   else if (name == PF_REPLACEMENT_VALUE_KW)
   {
      rspfProperty* p =
         new rspfNumericProperty(name,
                                  rspfString::toString(theReplacementValue));
       p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   else if (name == PF_CLAMP_VALUE_LO_KW)
   {
      rspfProperty* p =
         new rspfNumericProperty(name, rspfString::toString(theClampValueLo));
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   else if (name == PF_CLAMP_VALUE_HI_KW)
   {
      rspfProperty* p =
         new rspfNumericProperty(name, rspfString::toString(theClampValueHi));
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   else if (name == PF_REPLACEMENT_MODE_KW)
   {
      vector<rspfString> constraintList(5);
      constraintList[0] = "REPLACE_BAND_IF_TARGET";
      constraintList[1] = "REPLACE_BAND_IF_PARTIAL_TARGET";
      constraintList[2] = "REPLACE_ALL_BANDS_IF_PARTIAL_TARGET";
      constraintList[3] = "REPLACE_ONLY_FULL_TARGETS";
      constraintList[4] = "REPLACE_ALL_BANDS_IF_ANY_TARGET";
      
      rspfStringProperty* p =
         new rspfStringProperty(name,
                                 getReplacementModeString(),
                                 false,
                                 constraintList);
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   else if (name == PF_CLIP_MODE_KW)
   {
      vector<rspfString> constraintList(3);
      constraintList[0] = "none";
      constraintList[1] = "bounding_rect";
      constraintList[2] = "valid_vertices";
      
      rspfStringProperty* p =
         new rspfStringProperty(name,
                                 getClipModeString(),
                                 false,
                                 constraintList);
      p->setCacheRefreshBit();
      return rspfRefPtr<rspfProperty>(p);
   }
   rspfRefPtr<rspfProperty> result = rspfSource::getProperty(name);

   if(result.valid())
   {
      if(result->getName() == rspfKeywordNames::ENABLED_KW)
      {
         result->clearChangeType();
         
         // we will at least say its a radiometric change
         //
         result->setCacheRefreshBit();
      }
   }

   return result;
}

void rspfPixelFlipper::setProperty(rspfRefPtr<rspfProperty> property)
{
   if (!property) return;

   rspfString os = property->valueToString();
   
   rspfString name = property->getName();
   if (name == PF_TARGET_VALUE_KW)
   {
      setTargetValue(os.toDouble());
   }
   if (name == TARGET_LOWER_LIMIT_PROP_NAME)
   {
      setTargetRange(os.toDouble(), theTargetValueHi);
   }
   if (name == TARGET_UPPER_LIMIT_PROP_NAME)
   {
      setTargetRange(theTargetValueLo, os.toDouble());
   }
   else if  (name == PF_REPLACEMENT_VALUE_KW)
   {
      setReplacementValue(os.toDouble());
   }
   else if  (name == PF_REPLACEMENT_MODE_KW)
   {
      setReplacementMode(os);
   }
   else if  (name == PF_CLAMP_VALUE_LO_KW)
   {
      setClampValue(os.toDouble(), false);
   }
   else if  (name == PF_CLAMP_VALUE_HI_KW)
   {
      setClampValue(os.toDouble(), true);
   }
   else if  (name == PF_CLIP_MODE_KW)
   {
      setClipMode(os);
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

void rspfPixelFlipper::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(PF_TARGET_VALUE_KW);
   propertyNames.push_back(TARGET_LOWER_LIMIT_PROP_NAME);
   propertyNames.push_back(TARGET_UPPER_LIMIT_PROP_NAME);
   propertyNames.push_back(PF_REPLACEMENT_VALUE_KW);
   propertyNames.push_back(PF_REPLACEMENT_MODE_KW);
   propertyNames.push_back(PF_CLAMP_VALUE_LO_KW);
   propertyNames.push_back(PF_CLAMP_VALUE_HI_KW);
   propertyNames.push_back(PF_CLIP_MODE_KW);
   
   rspfImageSourceFilter::getPropertyNames(propertyNames);
}

rspfString rspfPixelFlipper::getShortName()const
{
   return rspfString("Pixel flipper");
}

bool rspfPixelFlipper::inRange(rspf_float64 value) const
{
   if (!theInputConnection)
   {
      //---
      // Not initialized yet...  We're going to return true so that things
      // like loadState work with the assumption that the caller know's
      // the correct range.  This check is really intended for an uncheck
      // range on the gui side.
      //---
      return true;
   }
   
   const rspf_float64 NULL_PIX = rspfImageSourceFilter::getNullPixelValue();
   const rspf_float64 MIN_PIX  = rspfImageSourceFilter::getMinPixelValue();
   const rspf_float64 MAX_PIX  = rspfImageSourceFilter::getMaxPixelValue();

   if ( (value == NULL_PIX) || ((value >= MIN_PIX) && (value <= MAX_PIX)) )
   {
      return true;
   }

   rspfNotify(rspfNotifyLevel_WARN)
      << "\nrspfPixelFlipper::inRange WARNING:"
      << "\nvalue \"" << value
      << "\" is out of range!"
      << "\nInput source null = " << NULL_PIX
      << "\nInput source min  = " << MIN_PIX
      << "\nInput source max  = " << MAX_PIX
      << endl;

   return false;
}
