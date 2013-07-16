//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Remapper to adjust hue, saturation and intensity.
//
//*************************************************************************
// $Id: rspfHsiRemapper.cpp 19714 2011-06-03 17:23:45Z gpotts $

#include <cstdlib>
#include <rspf/imaging/rspfHsiRemapper.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNormRgbVector.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfNumericProperty.h>

RTTI_DEF1(rspfHsiRemapper, "rspfHsiRemapper", rspfImageSourceFilter)

static rspfTrace traceDebug("rspfHsiRemapper:debug");

//***
// State keywords:
//***
static const char MASTER_HUE_OFFSET_KW[] = "hsi_master_hue_offset";
static const char MASTER_SATURATION_OFFSET_KW[] = "hsi_master_saturation_offset";
static const char MASTER_INTENSITY_OFFSET_KW[] = "hsi_master_intensity_offset";
static const char MASTER_INTENSITY_LOW_CLIP_KW[] = "hsi_master_intensity_low_clip";
static const char MASTER_INTENSITY_HIGH_CLIP_KW[] = "hsi_master_intensity_high_clip";

static const char RED_HUE_OFFSET_KW[] = "hsi_red_hue_offset";
static const char RED_HUE_LOW_RANGE_KW[] = "hsi_red_hue_low_range";
static const char RED_HUE_HIGH_RANGE_KW[] = "hsi_red_hue_high_range";
static const char RED_HUE_BLEND_RANGE_KW[] = "hsi_red_hue_blend_range";
static const char RED_SATURATION_OFFSET_KW[] = "hsi_red_saturation_offset";
static const char RED_INTENSITY_OFFSET_KW[] = "hsi_red_intensity_offset";

static const char YELLOW_HUE_OFFSET_KW[] = "hsi_yellow_hue_offset";
static const char YELLOW_HUE_LOW_RANGE_KW[] = "hsi_yellow_hue_low_range";
static const char YELLOW_HUE_HIGH_RANGE_KW[] = "hsi_yellow_hue_high_range";
static const char YELLOW_HUE_BLEND_RANGE_KW[] = "hsi_yellow_hue_blend_range";
static const char YELLOW_SATURATION_OFFSET_KW[] = "hsi_yellow_saturation_offset";
static const char YELLOW_INTENSITY_OFFSET_KW[] = "hsi_yellow_intensity_offset";

static const char GREEN_HUE_OFFSET_KW[] = "hsi_green_hue_offset";
static const char GREEN_HUE_LOW_RANGE_KW[] = "hsi_green_hue_low_range";
static const char GREEN_HUE_HIGH_RANGE_KW[] = "hsi_green_hue_high_range";
static const char GREEN_HUE_BLEND_RANGE_KW[] = "hsi_green_hue_blend_range";
static const char GREEN_SATURATION_OFFSET_KW[] = "hsi_green_saturation_offset";
static const char GREEN_INTENSITY_OFFSET_KW[] = "hsi_green_intensity_offset";

static const char CYAN_HUE_OFFSET_KW[] = "hsi_cyan_hue_offset";
static const char CYAN_HUE_LOW_RANGE_KW[] = "hsi_cyan_hue_low_range";
static const char CYAN_HUE_HIGH_RANGE_KW[] = "hsi_cyan_hue_high_range";
static const char CYAN_HUE_BLEND_RANGE_KW[] = "hsi_cyan_hue_blend_range";
static const char CYAN_SATURATION_OFFSET_KW[] = "hsi_cyan_saturation_offset";
static const char CYAN_INTENSITY_OFFSET_KW[] = "hsi_cyan_intensity_offset";

static const char BLUE_HUE_OFFSET_KW[] = "hsi_blue_hue_offset";
static const char BLUE_HUE_LOW_RANGE_KW[] = "hsi_blue_hue_low_range";
static const char BLUE_HUE_HIGH_RANGE_KW[] = "hsi_blue_hue_high_range";
static const char BLUE_HUE_BLEND_RANGE_KW[] = "hsi_blue_hue_blend_range";
static const char BLUE_SATURATION_OFFSET_KW[] = "hsi_blue_saturation_offset";
static const char BLUE_INTENSITY_OFFSET_KW[] = "hsi_blue_intensity_offset";

static const char MAGENTA_HUE_OFFSET_KW[] = "hsi_magenta_hue_offset";
static const char MAGENTA_HUE_LOW_RANGE_KW[] = "hsi_magenta_hue_low_range";
static const char MAGENTA_HUE_HIGH_RANGE_KW[] = "hsi_magenta_hue_high_range";
static const char MAGENTA_HUE_BLEND_RANGE_KW[] = "hsi_magenta_hue_blend_range";
static const char MAGENTA_SATURATION_OFFSET_KW[] = "hsi_magenta_saturation_offset";
static const char MAGENTA_INTENSITY_OFFSET_KW[] = "hsi_magenta_intensity_offset";

static const char WHITE_OBJECT_CLIP_KW[] = "hsi_white_object_clip";

static const double DEFAULT_BLEND = 15.0;
static const double MAX_BLEND     = 30.0;

rspfHsiRemapper::rspfHsiRemapper()
   :
      rspfImageSourceFilter      (),  // base class
      
      theTile                     (NULL),
      theBuffer                   (NULL),
      theNormalizedMinPix         (0.0),

      theMasterHueOffset          (0.0),
      theMasterSaturationOffset   (0.0),
      theMasterIntensityOffset    (0.0),
      theMasterIntensityLowClip   (0.0),
      theMasterIntensityHighClip  (1.0),
      
      theRedHueOffset             (0.0),
      theRedHueLowRange           (-30.0),
      theRedHueHighRange          (30.0),
      theRedHueBlendRange         (DEFAULT_BLEND),
      theRedSaturationOffset      (0.0),
      theRedIntensityOffset       (0.0),
      
      theYellowHueOffset          (0.0),
      theYellowHueLowRange        (30.0),
      theYellowHueHighRange       (90.0),
      theYellowHueBlendRange      (DEFAULT_BLEND),
      theYellowSaturationOffset   (0.0),
      theYellowIntensityOffset    (0.0),
      
      theGreenHueOffset           (0.0),
      theGreenHueLowRange         (90.0),
      theGreenHueHighRange        (150.0),
      theGreenHueBlendRange       (DEFAULT_BLEND),
      theGreenSaturationOffset    (0.0),
      theGreenIntensityOffset     (0.0),
      
      theCyanHueOffset            (0.0),
      theCyanHueLowRange          (150.0),
      theCyanHueHighRange         (210.0),
      theCyanHueBlendRange        (DEFAULT_BLEND),
      theCyanSaturationOffset     (0.0),
      theCyanIntensityOffset      (0.0),
      
      theBlueHueOffset            (0.0),
      theBlueHueLowRange          (210.0),
      theBlueHueHighRange         (270.0),
      theBlueHueBlendRange        (DEFAULT_BLEND),
      theBlueSaturationOffset     (0.0),
      theBlueIntensityOffset      (0.0),
      
      theMagentaHueOffset         (0.0),
      theMagentaHueLowRange       (270.0),
      theMagentaHueHighRange      (330.0),
      theMagentaHueBlendRange     (DEFAULT_BLEND),
      theMagentaSaturationOffset  (0.0),
      theMagentaIntensityOffset   (0.0),

      theWhiteObjectClip          (1.0)
{
   //***
   // Set the base class "theEnableFlag" to off since no adjustments have been
   // made yet.
   //***
   //disableSource();
   theValidFlag = false;
   // Construction not complete.
}

rspfHsiRemapper::~rspfHsiRemapper()
{
   theTile = NULL;
   if (theBuffer)
   {
      delete [] theBuffer;
      theBuffer = NULL;
   }
}

rspfRefPtr<rspfImageData> rspfHsiRemapper::getTile(
   const rspfIrect& tile_rect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }

   // Fetch tile from pointer from the input source.
   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(
      tile_rect, resLevel);

   // Check for remap bypass or a null tile return from input:
   if (!isSourceEnabled() || !inputTile||!theValidFlag)
   {
      return inputTile;
   }

   // Check for first time through or size change.
   if ( !theTile.valid() ||  
        tile_rect.height() != theTile->getHeight() ||
        tile_rect.width()  != theTile->getWidth() )
   {
      allocate(tile_rect);
   }

   //---
   // Set the image rectangle of the tile.
   // Note that this will resize the tiles buffers if 
   theTile->setImageRectangle(tile_rect);

   // Get its status.
   rspfDataObjectStatus tile_status = inputTile->getDataObjectStatus();
   if ( (tile_status == RSPF_NULL) || (tile_status == RSPF_EMPTY) )
   {
      theTile->makeBlank();
      return theTile;
   }

   double* rgbBuf[3];
   const rspf_uint32 PPT = theTile->getSizePerBand();  // Pixels Per Tile

   rgbBuf[0] = theBuffer;
   rgbBuf[1] = &(theBuffer[PPT]);
   rgbBuf[2] = &(rgbBuf[1][PPT]);
   
   switch(inputTile->getNumberOfBands())
   {
      case 1:
      case 2:
      {
         // Copy the first band only.
         inputTile->copyTileBandToNormalizedBuffer(0, rgbBuf[0]);
         memcpy(rgbBuf[1], rgbBuf[0], PPT*sizeof(double));
         memcpy(rgbBuf[2], rgbBuf[0], PPT*sizeof(double));
	 break;
      }
      case 3:
      {
         inputTile->copyTileToNormalizedBuffer(theBuffer);
         break;
      }
      default:
      {
         inputTile->copyTileBandToNormalizedBuffer(0, rgbBuf[0]);
         inputTile->copyTileBandToNormalizedBuffer(1, rgbBuf[1]);
         inputTile->copyTileBandToNormalizedBuffer(2, rgbBuf[2]);
	 break;
      }
   }   

   rspfNormRgbVector rgb;
   rspfHsiVector     hsi;
   double r = 0.0;
   double g = 0.0;
   double b = 0.0;
   double h = 0.0;
   double s = 0.0;
   double i = 0.0;
   rspf_uint32 idx;
   // Convert the rgb value to hsi and adjust values.
   for (idx=0; idx<PPT; ++idx)
   {
      r = rgbBuf[0][idx];
      g = rgbBuf[1][idx];
      b = rgbBuf[2][idx];
      
      rgb = rspfNormRgbVector(r,g,b);
      hsi = rgb;

      h   = hsi.getH();
      s   = hsi.getS();
      i   = hsi.getI();
      
      if(i > FLT_EPSILON)
      {
         double h_offset = theMasterHueOffset;
         double s_offset = theMasterSaturationOffset;
         double i_offset = theMasterIntensityOffset;

         //***
         // Note:  For the purpose of checking to see if in the red range,
         //        make the hue negative if it's >= 315.0 and < 360.0.
         //        The red low and high ranges are stored in the same manner.
         //***
         double red_hue  = h;
         if (red_hue  >= 315.0 && red_hue  < 360.0) red_hue  = red_hue - 360.0;
                          
         if ( red_hue >= theRedHueLowRange && red_hue <= theRedHueHighRange)
         {
            // Adjust the reds.
            
            double bf = 1.0; // blend factor
            if ( red_hue < (theRedHueLowRange + theRedHueBlendRange) )
            {
               bf = (red_hue - theRedHueLowRange) / theRedHueBlendRange;
            }
            else if ( red_hue > (theRedHueHighRange - theRedHueBlendRange) )
            {
               bf = (theRedHueHighRange - red_hue) / theRedHueBlendRange;
            }
            
            h_offset += (theRedHueOffset        * bf);
            s_offset += (theRedSaturationOffset * bf);
            i_offset += (theRedIntensityOffset  * bf);
         }
            
         if (h >= theYellowHueLowRange && h <= theYellowHueHighRange) 
         {
            // Adjust the yellows.
            
            double bf = 1.0; // blend factor
            if ( h < (theYellowHueLowRange + theYellowHueBlendRange) )
            {
               bf = (h - theYellowHueLowRange) / theYellowHueBlendRange;
            }
            else if ( h > (theYellowHueHighRange - theYellowHueBlendRange) )
            {
               bf = (theYellowHueHighRange - h) / theYellowHueBlendRange;
            }
            
            h_offset += (theYellowHueOffset        * bf);
            s_offset += (theYellowSaturationOffset * bf);
            i_offset += (theYellowIntensityOffset  * bf);
         }
         
         if (h >= theGreenHueLowRange && h <= theGreenHueHighRange) 
         {
            // Adjust the greens. 

            double bf = 1.0; // blend factor
            if ( h < (theGreenHueLowRange + theGreenHueBlendRange) )
            {
               bf = (h - theGreenHueLowRange) / theGreenHueBlendRange;
            }
            else if ( h > (theGreenHueHighRange - theGreenHueBlendRange) )
            {
               bf = (theGreenHueHighRange - h) / theGreenHueBlendRange;
            }
            
            h_offset += (theGreenHueOffset        * bf);
            s_offset += (theGreenSaturationOffset * bf);
            i_offset += (theGreenIntensityOffset  * bf);
         }

         if (h >= theCyanHueLowRange && h <= theCyanHueHighRange) 
         {
            // Adjust the cyans.
            
            double bf = 1.0; // blend factor
            if ( h < (theCyanHueLowRange + theCyanHueBlendRange) )
            {
               bf = (h - theCyanHueLowRange) / theCyanHueBlendRange;
            }
            else if ( h > (theCyanHueHighRange - theCyanHueBlendRange) )
            {
               bf = (theCyanHueHighRange - h) / theCyanHueBlendRange;
            }
            
            h_offset += (theCyanHueOffset        * bf);
            s_offset += (theCyanSaturationOffset * bf);
            i_offset += (theCyanIntensityOffset  * bf);
         }

         if (h >= theBlueHueLowRange && h <= theBlueHueHighRange) 
         {
            // Adjust the blues.
            
            double bf = 1.0; // blend factor
            if ( h < (theBlueHueLowRange + theBlueHueBlendRange) )
            {
               bf = (h - theBlueHueLowRange) / theBlueHueBlendRange;
            }
            else if ( h > (theBlueHueHighRange - theBlueHueBlendRange) )
            {
               bf = (theBlueHueHighRange - h) / theBlueHueBlendRange;
            }
            
            h_offset += (theBlueHueOffset        * bf);
            s_offset += (theBlueSaturationOffset * bf);
            i_offset += (theBlueIntensityOffset  * bf);
         }

         if (h >= theMagentaHueLowRange && h <= theMagentaHueHighRange) 
         {
            // Adjust the magentas.
            
            double bf = 1.0; // blend factor
            if ( h < (theMagentaHueLowRange + theMagentaHueBlendRange) )
            {
               bf = (h - theMagentaHueLowRange) / theMagentaHueBlendRange;
            }
            else if ( h > (theMagentaHueHighRange - theMagentaHueBlendRange) )
            {
               bf = (theMagentaHueHighRange - h) / theMagentaHueBlendRange;
            }
            
            h_offset += (theMagentaHueOffset        * bf);
            s_offset += (theMagentaSaturationOffset * bf);
            i_offset += (theMagentaIntensityOffset  * bf);
         }
         
         // Apply the hue offset.
         h += h_offset;

         // Make sure the hue is between 0 and 360...
         if (h < 0.0) h += 360.0;
         else if (h >= 360) h -= 360.0;

         // Apply the saturation offset clamping/clipping to 0.0/1.0.
         s += s_offset;
         s = (s > 0.0 ? (s < 1.0 ? s : 1.0) : 0.0);
         
         // Apply the intensity offset clamping/clipping to 0.0/1.0.
         i += i_offset;
         i = (i > 0.0 ? (i < 1.0 ? i : 1.0) : 0.0);

         // Stretch the intensity channel.
//          i = (i - theMasterIntensityLowClip) *
//              ( 1.0 / (theMasterIntensityHighClip -
//                       theMasterIntensityLowClip) );
         i = (i - theMasterIntensityLowClip)/(theMasterIntensityHighClip -
                                              theMasterIntensityLowClip);
         
         hsi.setH(h);
         hsi.setS(s);
         hsi.setI(i);

         rgb = hsi;
         
         r = rgb.getR();
         g = rgb.getG();
         b = rgb.getB();
         if ( (theWhiteObjectClip < 1.0) &&
              (r > theWhiteObjectClip)   &&
              (g > theWhiteObjectClip)   &&
              (b > theWhiteObjectClip) )
         {
            r = theWhiteObjectClip;
            g = theWhiteObjectClip;
            b = theWhiteObjectClip;
         }
         // Do min/max range check and assign back to buffer.
         rgbBuf[0][idx] = r > theNormalizedMinPix ? (r < 1.0 ? r : 1.0) :
            theNormalizedMinPix;
         rgbBuf[1][idx] = g > theNormalizedMinPix ? (g < 1.0 ? g : 1.0) :
            theNormalizedMinPix;
         rgbBuf[2][idx] = b > theNormalizedMinPix ? (b < 1.0 ? b : 1.0) :
            theNormalizedMinPix;
      } // End of "if(i > FLT_EPSILON)"
      else
      {
         rgbBuf[0][idx] = 0.0;
         rgbBuf[1][idx] = 0.0;
         rgbBuf[2][idx] = 0.0;
      }
         
   }  // End of loop through pixels in a tile.
   // Copy the buffer to the output tile.
   theTile->copyNormalizedBufferToTile(theBuffer);
   
   // Update the tile status.
   theTile->validate();

   return theTile;
}

void rspfHsiRemapper::initialize()
{
   rspfImageSourceFilter::initialize();
   if (theTile.valid())
   {
      theTile = NULL;
      if(theBuffer)
      {
         delete []theBuffer;
         theBuffer = NULL;
      }
   }
}

void rspfHsiRemapper::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   rspfString name = property->getName();
   if(name == MASTER_HUE_OFFSET_KW)
   {
      setMasterHueOffset(property->valueToString().toDouble());
   }
   else if(name == MASTER_SATURATION_OFFSET_KW)
   {
      setMasterSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == MASTER_INTENSITY_OFFSET_KW)
   {
      setMasterIntensityOffset(property->valueToString().toDouble());
   }
   else if(name == MASTER_INTENSITY_LOW_CLIP_KW)
   {
      setMasterIntensityLowClip(property->valueToString().toDouble());
   }
   else if(name == MASTER_INTENSITY_HIGH_CLIP_KW)
   {
      setMasterIntensityHighClip(property->valueToString().toDouble());
   }
   else if(name == RED_HUE_OFFSET_KW)
   {
      setRedHueOffset(property->valueToString().toDouble());
   }
   else if(name == RED_HUE_LOW_RANGE_KW)
   {
      setRedHueLowRange(property->valueToString().toDouble());
   }
   else if(name == RED_HUE_HIGH_RANGE_KW)
   {
      setRedHueHighRange(property->valueToString().toDouble());
   }
   else if(name == RED_HUE_BLEND_RANGE_KW)
   {
      setRedHueBlendRange(property->valueToString().toDouble());
   }
   else if(name == RED_SATURATION_OFFSET_KW)
   {
      setRedSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == RED_INTENSITY_OFFSET_KW)
   {
      setRedIntensityOffset(property->valueToString().toDouble());
  }
   else if(name == YELLOW_HUE_OFFSET_KW)
   {
      setYellowHueOffset(property->valueToString().toDouble());
   }
   else if(name == YELLOW_HUE_LOW_RANGE_KW)
   {
      setYellowHueLowRange(property->valueToString().toDouble());
   }
   else if(name == YELLOW_HUE_HIGH_RANGE_KW)
   {
      setYellowHueHighRange(property->valueToString().toDouble());
   }
   else if(name == YELLOW_HUE_BLEND_RANGE_KW)
   {
      setYellowHueBlendRange(property->valueToString().toDouble());
   }
   else if(name == YELLOW_SATURATION_OFFSET_KW)
   {
      setYellowSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == YELLOW_INTENSITY_OFFSET_KW)
   {
      setYellowIntensityOffset(property->valueToString().toDouble());
   }
   else if(name == GREEN_HUE_OFFSET_KW)
   {
      setGreenHueOffset(property->valueToString().toDouble());
   }
   else if(name == GREEN_HUE_LOW_RANGE_KW)
   {
      setGreenHueLowRange(property->valueToString().toDouble());
   }
   else if(name == GREEN_HUE_HIGH_RANGE_KW)
   {
      setGreenHueHighRange(property->valueToString().toDouble());
   }
   else if(name == GREEN_HUE_BLEND_RANGE_KW)
   {
      setGreenHueBlendRange(property->valueToString().toDouble());
   }
   else if(name == GREEN_SATURATION_OFFSET_KW)
   {
      setGreenSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == GREEN_INTENSITY_OFFSET_KW)
   {
      setGreenIntensityOffset(property->valueToString().toDouble());
   }
   else if(name == CYAN_HUE_OFFSET_KW)
   {
      setCyanHueOffset(property->valueToString().toDouble());
   }
   else if(name == CYAN_HUE_LOW_RANGE_KW)
   {
      setCyanHueLowRange(property->valueToString().toDouble());
   }
   else if(name == CYAN_HUE_HIGH_RANGE_KW)
   {
      setCyanHueHighRange(property->valueToString().toDouble());
   }
   else if(name == CYAN_HUE_BLEND_RANGE_KW)
   {
      setCyanHueBlendRange(property->valueToString().toDouble());
   }
   else if(name == CYAN_SATURATION_OFFSET_KW)
   {
      setCyanSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == CYAN_INTENSITY_OFFSET_KW)
   {
      setCyanIntensityOffset(property->valueToString().toDouble());
   }
   else if(name == BLUE_HUE_OFFSET_KW)
   {
      setBlueHueOffset(property->valueToString().toDouble());
   }
   else if(name == BLUE_HUE_LOW_RANGE_KW)
   {
      setBlueHueLowRange(property->valueToString().toDouble());
   }
   else if(name == BLUE_HUE_HIGH_RANGE_KW)
   {
      setBlueHueHighRange(property->valueToString().toDouble());
   }
   else if(name == BLUE_HUE_BLEND_RANGE_KW)
   {
      setBlueHueBlendRange(property->valueToString().toDouble());
   }
   else if(name == BLUE_SATURATION_OFFSET_KW)
   {
      setBlueSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == BLUE_INTENSITY_OFFSET_KW)
   {
      setBlueIntensityOffset(property->valueToString().toDouble());
   }
   else if(name == MAGENTA_HUE_OFFSET_KW)
   {
      setMagentaHueOffset(property->valueToString().toDouble());
   }
   else if(name == MAGENTA_HUE_LOW_RANGE_KW)
   {
      setMagentaHueLowRange(property->valueToString().toDouble());
   }
   else if(name == MAGENTA_HUE_HIGH_RANGE_KW)
   {
      setMagentaHueHighRange(property->valueToString().toDouble());
   }
   else if(name == MAGENTA_HUE_BLEND_RANGE_KW)
   {
      setMagentaHueBlendRange(property->valueToString().toDouble());
   }
   else if(name == MAGENTA_SATURATION_OFFSET_KW)
   {
      setMagentaSaturationOffset(property->valueToString().toDouble());
   }
   else if(name == MAGENTA_INTENSITY_OFFSET_KW)
   {
      setMagentaIntensityOffset(property->valueToString().toDouble());
   }
   else if(name == WHITE_OBJECT_CLIP_KW)
   {
      setWhiteObjectClip(property->valueToString().toDouble());
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfHsiRemapper::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result;
   if(name == MASTER_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMasterHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == MASTER_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMasterSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
  }
   else if(name == MASTER_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMasterIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == MASTER_INTENSITY_LOW_CLIP_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMasterIntensityLowClip), 0, 1);
      result->setCacheRefreshBit();
   }
   else if(name == MASTER_INTENSITY_HIGH_CLIP_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMasterIntensityHighClip), 0, 1);
      result->setCacheRefreshBit();
   }
   else if(name == RED_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theRedHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == RED_HUE_LOW_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theRedHueLowRange), -30, 30);
      result->setCacheRefreshBit();
   }
   else if(name == RED_HUE_HIGH_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theRedHueHighRange), -30, 30);
      result->setCacheRefreshBit();
   }
   else if(name == RED_HUE_BLEND_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theRedHueBlendRange), 0, 30);
      result->setCacheRefreshBit();
   }
   else if(name == RED_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theRedSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == RED_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theRedIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == YELLOW_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theYellowHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == YELLOW_HUE_LOW_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theYellowHueLowRange), 30, 90);
      result->setCacheRefreshBit();
   }
   else if(name == YELLOW_HUE_HIGH_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theYellowHueHighRange), 30, 90);
      result->setCacheRefreshBit();
   }
   else if(name == YELLOW_HUE_BLEND_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theYellowHueBlendRange), 0, 30);
      result->setCacheRefreshBit();
   }
   else if(name == YELLOW_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theYellowSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == YELLOW_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theYellowIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == GREEN_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theGreenHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == GREEN_HUE_LOW_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theGreenHueLowRange), 90, 150);
      result->setCacheRefreshBit();
   }
   else if(name == GREEN_HUE_HIGH_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theGreenHueHighRange), 90, 150);
      result->setCacheRefreshBit();
   }
   else if(name == GREEN_HUE_BLEND_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theGreenHueBlendRange), 0, 30);
      result->setCacheRefreshBit();
   }
   else if(name == GREEN_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theGreenSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == GREEN_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theGreenIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == CYAN_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theCyanHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == CYAN_HUE_LOW_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theCyanHueLowRange), 150, 210);
      result->setCacheRefreshBit();
   }
   else if(name == CYAN_HUE_HIGH_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theCyanHueHighRange), 150, 210);
      result->setCacheRefreshBit();
   }
   else if(name == CYAN_HUE_BLEND_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theCyanHueBlendRange), 0, 30);
      result->setCacheRefreshBit();
   }
   else if(name == CYAN_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theCyanSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == CYAN_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theCyanIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == BLUE_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theBlueHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == BLUE_HUE_LOW_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theBlueHueLowRange), 210, 270);
      result->setCacheRefreshBit();
   }
   else if(name == BLUE_HUE_HIGH_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theBlueHueHighRange), 210, 270);
      result->setCacheRefreshBit();
   }
   else if(name == BLUE_HUE_BLEND_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theBlueHueBlendRange), 0, 30);
      result->setCacheRefreshBit();
   }
   else if(name == BLUE_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theBlueSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == BLUE_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theBlueIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == MAGENTA_HUE_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMagentaHueOffset), -180, 180);
      result->setCacheRefreshBit();
   }
   else if(name == MAGENTA_HUE_LOW_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMagentaHueLowRange), 270, 330);
      result->setCacheRefreshBit();
   }
   else if(name == MAGENTA_HUE_HIGH_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMagentaHueHighRange), 270, 330);
      result->setCacheRefreshBit();
   }
   else if(name == MAGENTA_HUE_BLEND_RANGE_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMagentaHueBlendRange), 0, 30);
      result->setCacheRefreshBit();
   }
   else if(name == MAGENTA_SATURATION_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMagentaSaturationOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == MAGENTA_INTENSITY_OFFSET_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theMagentaIntensityOffset), -1, 1);
      result->setCacheRefreshBit();
   }
   else if(name == WHITE_OBJECT_CLIP_KW)
   {
      result = new rspfNumericProperty(name, rspfString::toString(theWhiteObjectClip), 0.8, 1.0);
      result->setCacheRefreshBit();
   }
   else
   {
     result = rspfImageSourceFilter::getProperty(name);
   }
   
   return result;
}

void rspfHsiRemapper::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(MASTER_HUE_OFFSET_KW);
   propertyNames.push_back(MASTER_SATURATION_OFFSET_KW);
   propertyNames.push_back(MASTER_INTENSITY_OFFSET_KW);
   propertyNames.push_back(MASTER_INTENSITY_LOW_CLIP_KW);
   propertyNames.push_back(MASTER_INTENSITY_HIGH_CLIP_KW);
   propertyNames.push_back(RED_HUE_OFFSET_KW);
   propertyNames.push_back(RED_HUE_LOW_RANGE_KW);
   propertyNames.push_back(RED_HUE_HIGH_RANGE_KW);
   propertyNames.push_back(RED_HUE_BLEND_RANGE_KW);
   propertyNames.push_back(RED_SATURATION_OFFSET_KW);
   propertyNames.push_back(RED_INTENSITY_OFFSET_KW);
   propertyNames.push_back(YELLOW_HUE_OFFSET_KW);
   propertyNames.push_back(YELLOW_HUE_LOW_RANGE_KW);
   propertyNames.push_back(YELLOW_HUE_HIGH_RANGE_KW);
   propertyNames.push_back(YELLOW_HUE_BLEND_RANGE_KW);
   propertyNames.push_back(YELLOW_SATURATION_OFFSET_KW);
   propertyNames.push_back(YELLOW_INTENSITY_OFFSET_KW);
   propertyNames.push_back(GREEN_HUE_OFFSET_KW);
   propertyNames.push_back(GREEN_HUE_LOW_RANGE_KW);
   propertyNames.push_back(GREEN_HUE_HIGH_RANGE_KW);
   propertyNames.push_back(GREEN_HUE_BLEND_RANGE_KW);
   propertyNames.push_back(GREEN_SATURATION_OFFSET_KW);
   propertyNames.push_back(GREEN_INTENSITY_OFFSET_KW);
   propertyNames.push_back(CYAN_HUE_OFFSET_KW);
   propertyNames.push_back(CYAN_HUE_LOW_RANGE_KW);
   propertyNames.push_back(CYAN_HUE_HIGH_RANGE_KW);
   propertyNames.push_back(CYAN_HUE_BLEND_RANGE_KW);
   propertyNames.push_back(CYAN_SATURATION_OFFSET_KW);
   propertyNames.push_back(CYAN_INTENSITY_OFFSET_KW);
   propertyNames.push_back(BLUE_HUE_OFFSET_KW);
   propertyNames.push_back(BLUE_HUE_LOW_RANGE_KW);
   propertyNames.push_back(BLUE_HUE_HIGH_RANGE_KW);
   propertyNames.push_back(BLUE_HUE_BLEND_RANGE_KW);
   propertyNames.push_back(BLUE_SATURATION_OFFSET_KW);
   propertyNames.push_back(BLUE_INTENSITY_OFFSET_KW);
   propertyNames.push_back(MAGENTA_HUE_OFFSET_KW);
   propertyNames.push_back(MAGENTA_HUE_LOW_RANGE_KW);
   propertyNames.push_back(MAGENTA_HUE_HIGH_RANGE_KW);
   propertyNames.push_back(MAGENTA_HUE_BLEND_RANGE_KW);
   propertyNames.push_back(MAGENTA_SATURATION_OFFSET_KW);
   propertyNames.push_back(MAGENTA_INTENSITY_OFFSET_KW);
   propertyNames.push_back(WHITE_OBJECT_CLIP_KW);
}


void rspfHsiRemapper::allocate(const rspfIrect& rect)
{
   
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this,this);
      theTile->initialize();

      rspf_uint32 width  = rect.width();
      rspf_uint32 height = rect.height();
      if(theBuffer)
      {
         delete [] theBuffer;
         theBuffer = 0;
      }
      rspf_uint32 size = width * height * 3; // Buffer always 3 bands.
      theBuffer = new double[size];
      memset(theBuffer, '\0', sizeof(double) * size);
      
      // Get the minimum normalized pixel value.
      theNormalizedMinPix = calculateMinNormValue();
   }
}

bool rspfHsiRemapper::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   static const char MODULE[] = "rspfHsiRemapper::loadState()";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entering..." << endl;
   }

   // Make a prefix.
   rspfString tmpPrefix;
   if (prefix) tmpPrefix += prefix;
   
   const char* lookupReturn;

   lookupReturn = kwl.find(tmpPrefix.c_str(), MASTER_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setMasterHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MASTER_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setMasterSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MASTER_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setMasterIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MASTER_INTENSITY_LOW_CLIP_KW);
   if(lookupReturn)
   {
      setMasterIntensityLowClip(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MASTER_INTENSITY_HIGH_CLIP_KW);
   if(lookupReturn)
   {
      setMasterIntensityHighClip(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), RED_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setRedHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), RED_HUE_LOW_RANGE_KW);
   if(lookupReturn)
   {
      setRedHueLowRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), RED_HUE_HIGH_RANGE_KW);
   if(lookupReturn)
   {
      setRedHueHighRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), RED_HUE_BLEND_RANGE_KW);
   if(lookupReturn)
   {
      setRedHueBlendRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), RED_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setRedSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), RED_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setRedIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), YELLOW_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setYellowHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), YELLOW_HUE_LOW_RANGE_KW);
   if(lookupReturn)
   {
      setYellowHueLowRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), YELLOW_HUE_HIGH_RANGE_KW);
   if(lookupReturn)
   {
      setYellowHueHighRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), YELLOW_HUE_BLEND_RANGE_KW);
   if(lookupReturn)
   {
      setYellowHueBlendRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), YELLOW_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setYellowSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), YELLOW_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setYellowIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), GREEN_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setGreenHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), GREEN_HUE_LOW_RANGE_KW);
   if(lookupReturn)
   {
      setGreenHueLowRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), GREEN_HUE_HIGH_RANGE_KW);
   if(lookupReturn)
   {
      setGreenHueHighRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), GREEN_HUE_BLEND_RANGE_KW);
   if(lookupReturn)
   {
      setGreenHueBlendRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), GREEN_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setGreenSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), GREEN_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setGreenIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), CYAN_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setCyanHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), CYAN_HUE_LOW_RANGE_KW);
   if(lookupReturn)
   {
      setCyanHueLowRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), CYAN_HUE_HIGH_RANGE_KW);
   if(lookupReturn)
   {
      setCyanHueHighRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), CYAN_HUE_BLEND_RANGE_KW);
   if(lookupReturn)
   {
      setCyanHueBlendRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), CYAN_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setCyanSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), CYAN_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setCyanIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), BLUE_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setBlueHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), BLUE_HUE_LOW_RANGE_KW);
   if(lookupReturn)
   {
      setBlueHueLowRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), BLUE_HUE_HIGH_RANGE_KW);
   if(lookupReturn)
   {
      setBlueHueHighRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), BLUE_HUE_BLEND_RANGE_KW);
   if(lookupReturn)
   {
      setBlueHueBlendRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), BLUE_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setBlueSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), BLUE_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setBlueIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MAGENTA_HUE_OFFSET_KW);
   if(lookupReturn)
   {
      setMagentaHueOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MAGENTA_HUE_LOW_RANGE_KW);
   if(lookupReturn)
   {
      setMagentaHueLowRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MAGENTA_HUE_HIGH_RANGE_KW);
   if(lookupReturn)
   {
      setMagentaHueHighRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MAGENTA_HUE_BLEND_RANGE_KW);
   if(lookupReturn)
   {
      setMagentaHueBlendRange(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MAGENTA_SATURATION_OFFSET_KW);
   if(lookupReturn)
   {
      setMagentaSaturationOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), MAGENTA_INTENSITY_OFFSET_KW);
   if(lookupReturn)
   {
      setMagentaIntensityOffset(atof(lookupReturn));
   }

   lookupReturn = kwl.find(tmpPrefix.c_str(), WHITE_OBJECT_CLIP_KW);
   if(lookupReturn)
   {
      setWhiteObjectClip(atof(lookupReturn));
   }

   //***
   // Initialize the base class.  Do this last so that the enable/disable
   // doesn't get overridden by the "set*" methods.
   //***
   rspfImageSourceFilter::loadState(kwl, tmpPrefix.c_str());
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << ""
         << *this
         << "\nreturning..."
         << endl;
   }

   return true;
}

bool rspfHsiRemapper::saveState(rspfKeywordlist& kwl,
                                 const char* prefix) const
{
   static const char MODULE[] = "rspfHsiRemapper::saveStateTo()";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << "entering..." << endl;
   }

   // Call the base class getStateFrom.
   rspfImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix, MASTER_HUE_OFFSET_KW, theMasterHueOffset);

   kwl.add(prefix, MASTER_SATURATION_OFFSET_KW, theMasterSaturationOffset);

   kwl.add(prefix, MASTER_INTENSITY_OFFSET_KW, theMasterIntensityOffset);

   kwl.add(prefix, MASTER_INTENSITY_LOW_CLIP_KW, theMasterIntensityLowClip);

   kwl.add(prefix, MASTER_INTENSITY_HIGH_CLIP_KW, theMasterIntensityHighClip);

   kwl.add(prefix, RED_HUE_OFFSET_KW, theRedHueOffset);

   kwl.add(prefix, RED_HUE_LOW_RANGE_KW, theRedHueLowRange);

   kwl.add(prefix, RED_HUE_HIGH_RANGE_KW, theRedHueHighRange);

   kwl.add(prefix, RED_HUE_BLEND_RANGE_KW, theRedHueBlendRange);

   kwl.add(prefix, RED_SATURATION_OFFSET_KW, theRedSaturationOffset);
           
   kwl.add(prefix, RED_INTENSITY_OFFSET_KW, theRedIntensityOffset);

   kwl.add(prefix, YELLOW_HUE_OFFSET_KW, theYellowHueOffset);

   kwl.add(prefix, YELLOW_HUE_LOW_RANGE_KW, theYellowHueLowRange);

   kwl.add(prefix, YELLOW_HUE_HIGH_RANGE_KW, theYellowHueHighRange);
           
   kwl.add(prefix, YELLOW_HUE_BLEND_RANGE_KW, theYellowHueBlendRange);

   kwl.add(prefix, YELLOW_SATURATION_OFFSET_KW, theYellowSaturationOffset);

   kwl.add(prefix, YELLOW_INTENSITY_OFFSET_KW, theYellowIntensityOffset);

   kwl.add(prefix, GREEN_HUE_OFFSET_KW, theGreenHueOffset);

   kwl.add(prefix, GREEN_HUE_LOW_RANGE_KW, theGreenHueLowRange);

   kwl.add(prefix, GREEN_HUE_HIGH_RANGE_KW, theGreenHueHighRange);

   kwl.add(prefix, GREEN_HUE_BLEND_RANGE_KW, theGreenHueBlendRange);
           
   kwl.add(prefix, GREEN_SATURATION_OFFSET_KW, theGreenSaturationOffset);
           
   kwl.add(prefix, GREEN_INTENSITY_OFFSET_KW, theGreenIntensityOffset);

   kwl.add(prefix, CYAN_HUE_OFFSET_KW, theCyanHueOffset);

   kwl.add(prefix, CYAN_HUE_LOW_RANGE_KW, theCyanHueLowRange);

   kwl.add(prefix, CYAN_HUE_HIGH_RANGE_KW, theCyanHueHighRange);
           
   kwl.add(prefix, CYAN_HUE_BLEND_RANGE_KW, theCyanHueBlendRange);
           
   kwl.add(prefix, CYAN_SATURATION_OFFSET_KW, theCyanSaturationOffset);
           
   kwl.add(prefix, CYAN_INTENSITY_OFFSET_KW, theCyanIntensityOffset);

   kwl.add(prefix, BLUE_HUE_OFFSET_KW, theBlueHueOffset);

   kwl.add(prefix, BLUE_HUE_LOW_RANGE_KW, theBlueHueLowRange);

   kwl.add(prefix, BLUE_HUE_HIGH_RANGE_KW, theBlueHueHighRange);

   kwl.add(prefix, BLUE_HUE_BLEND_RANGE_KW, theBlueHueBlendRange);

   kwl.add(prefix, BLUE_SATURATION_OFFSET_KW, theBlueSaturationOffset);
           
   kwl.add(prefix, BLUE_INTENSITY_OFFSET_KW, theBlueIntensityOffset);
           
   kwl.add(prefix, MAGENTA_HUE_OFFSET_KW, theMagentaHueOffset);
           
   kwl.add(prefix, MAGENTA_HUE_LOW_RANGE_KW, theMagentaHueLowRange);
           
   kwl.add(prefix, MAGENTA_HUE_HIGH_RANGE_KW, theMagentaHueHighRange);
           
   kwl.add(prefix, MAGENTA_HUE_BLEND_RANGE_KW, theMagentaHueBlendRange);
           
   kwl.add(prefix, MAGENTA_SATURATION_OFFSET_KW, theMagentaSaturationOffset);

   kwl.add(prefix, MAGENTA_INTENSITY_OFFSET_KW, theMagentaIntensityOffset);

   kwl.add(prefix, WHITE_OBJECT_CLIP_KW, theWhiteObjectClip);

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << "returning..." << endl;
   }

   return true;
}

void rspfHsiRemapper::resetGroup(int color_group)
{
   switch (color_group)
   {
      case RED:
         resetRed();
         break;
      case YELLOW:
         resetYellow();
         break;
      case GREEN:
         resetGreen();
         break;
      case CYAN:
         resetCyan();
         break;
      case BLUE:
         resetBlue();
         break;
      case MAGENTA:
         resetMagenta();
         break;
      case ALL:
         resetMaster();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setSaturationOffset NOTICE:  Range error!"
            << endl;
         break;
   }
   verifyEnabled();
}

void rspfHsiRemapper::resetAll()
{
   theMasterHueOffset          = 0.0;
   theMasterSaturationOffset   = 0.0;
   theMasterIntensityOffset    = 0.0;
   theMasterIntensityLowClip   = 0.0;
   theMasterIntensityHighClip  = 1.0;
   
   theRedHueOffset             = 0.0;
   theRedHueLowRange           = -30.0;
   theRedHueHighRange          = 30.0;
   theRedHueBlendRange         = DEFAULT_BLEND;
   theRedSaturationOffset      = 0.0;
   theRedIntensityOffset       = 0.0;
   
   theYellowHueOffset          = 0.0;
   theYellowHueLowRange        = 30.0;
   theYellowHueHighRange       = 90.0;
   theYellowHueBlendRange      = DEFAULT_BLEND;
   theYellowSaturationOffset   = 0.0;
   theYellowIntensityOffset    = 0.0;
   
   theGreenHueOffset           = 0.0;
   theGreenHueLowRange         = 90.0;
   theGreenHueHighRange        = 150.0;
   theGreenHueBlendRange       = DEFAULT_BLEND;
   theGreenSaturationOffset    = 0.0;
   theGreenIntensityOffset     = 0.0;
   
   theCyanHueOffset            = 0.0;
   theCyanHueLowRange          = 150.0;
   theCyanHueHighRange         = 210.0;
   theCyanHueBlendRange        = DEFAULT_BLEND;
   theCyanSaturationOffset     = 0.0;
   theCyanIntensityOffset      = 0.0;
   
   theBlueHueOffset            = 0.0;
   theBlueHueLowRange          = 210.0;
   theBlueHueHighRange         = 270.0;
   theBlueHueBlendRange        = DEFAULT_BLEND;
   theBlueSaturationOffset     = 0.0;
   theBlueIntensityOffset      = 0.0;
   
   theMagentaHueOffset         = 0.0;
   theMagentaHueLowRange       = 270.0;
   theMagentaHueHighRange      = 330.0;
   theMagentaHueBlendRange     = DEFAULT_BLEND;
   theMagentaSaturationOffset  = 0.0;
   theMagentaIntensityOffset   = 0.0;

   theWhiteObjectClip          = 1.0;
   theValidFlag = false;
   //disableSource();
}

void rspfHsiRemapper::resetMaster()
{
   theMasterHueOffset          = 0.0;
   theMasterSaturationOffset   = 0.0;
   theMasterIntensityOffset    = 0.0;
   theMasterIntensityLowClip   = 0.0;
   theMasterIntensityHighClip  = 1.0;

   verifyEnabled();
}

void rspfHsiRemapper::resetRed()
{
   theRedHueOffset             = 0.0;
   theRedHueLowRange           = -30.0;
   theRedHueHighRange          = 30.0;
   theRedHueBlendRange         = DEFAULT_BLEND;
   theRedSaturationOffset      = 0.0;
   theRedIntensityOffset       = 0.0;

   verifyEnabled();
}

void rspfHsiRemapper::resetYellow()
{
   theYellowHueOffset          = 0.0;
   theYellowHueLowRange        = 30.0;
   theYellowHueHighRange       = 90.0;
   theYellowHueBlendRange      = DEFAULT_BLEND;
   theYellowSaturationOffset   = 0.0;
   theYellowIntensityOffset    = 0.0;

   verifyEnabled();
}

void rspfHsiRemapper::resetGreen()
{
   theGreenHueOffset           = 0.0;
   theGreenHueLowRange         = 90.0;
   theGreenHueHighRange        = 150.0;
   theGreenHueBlendRange       = DEFAULT_BLEND;
   theGreenSaturationOffset    = 0.0;
   theGreenIntensityOffset     = 0.0;

   verifyEnabled();
}

void rspfHsiRemapper::resetCyan()
{
   theCyanHueOffset            = 0.0;
   theCyanHueLowRange          = 150.0;
   theCyanHueHighRange         = 210.0;
   theCyanHueBlendRange        = DEFAULT_BLEND;
   theCyanSaturationOffset     = 0.0;
   theCyanIntensityOffset      = 0.0;

   verifyEnabled();
}

void rspfHsiRemapper::resetBlue()
{
   theBlueHueOffset            = 0.0;
   theBlueHueLowRange          = 210.0;
   theBlueHueHighRange         = 270.0;
   theBlueHueBlendRange        = DEFAULT_BLEND;
   theBlueSaturationOffset     = 0.0;
   theBlueIntensityOffset      = 0.0;

   verifyEnabled();
}

void rspfHsiRemapper::resetMagenta()
{
   theMagentaHueOffset         = 0.0;
   theMagentaHueLowRange       = 270.0;
   theMagentaHueHighRange      = 330.0;
   theMagentaHueBlendRange     = DEFAULT_BLEND;
   theMagentaSaturationOffset  = 0.0;
   theMagentaIntensityOffset   = 0.0;

   verifyEnabled();
}

void rspfHsiRemapper::verifyEnabled()
{
   // Start off disabled...
   //disableSource();
   theValidFlag = false;
   if (!theInputConnection)
   {
      //***
      // Since this filter can be constructed with no input connection do not
      // output and error, simply return.
      //***
      return;
   }
   
   //***
   // Add all the offsets and the low clips.
   // If greater than zero enable getTile method.
   //***
   double d = theMasterHueOffset         +
           theMasterSaturationOffset  +
           theMasterIntensityOffset   +
           theMasterIntensityLowClip  +
           theRedHueOffset            +
           theRedSaturationOffset     +
           theRedIntensityOffset      +
           theYellowHueOffset         +
           theYellowSaturationOffset  +
           theYellowIntensityOffset   +
           theGreenHueOffset          +
           theGreenSaturationOffset   +
           theGreenIntensityOffset    +
           theCyanHueOffset           +
           theCyanSaturationOffset    +
           theCyanIntensityOffset     +
           theBlueHueOffset           +
           theBlueSaturationOffset    +
           theBlueIntensityOffset     +
           theMagentaHueOffset        +
           theMagentaSaturationOffset +
           theMagentaIntensityOffset;
   
   if ( d != 0.0 ||
        theMasterIntensityHighClip != 1.0 ||
        theWhiteObjectClip != 1.0 )
   {
      theValidFlag = true;
     // enableSource();
   }
}

void rspfHsiRemapper::setMasterHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theMasterHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMasterHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setMasterSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theMasterSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMasterSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setMasterIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theMasterIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMasterIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setMasterIntensityLowClip(double clip)
{
   if (clip >= 0.0 && clip < theMasterIntensityHighClip)
   {
      theMasterIntensityLowClip = clip;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMasterIntensityClip range error:"
         << "\nLow clip of " << clip << " is out of range!"
         << "\nMust be equal to or greater than 0.0 and less than"
         << "\nthe high clip of " << theMasterIntensityHighClip
         << endl;
   }
}

void rspfHsiRemapper::setMasterIntensityHighClip(double clip)
{
   if (clip <= 1.0 && clip > theMasterIntensityLowClip)
   {
      theMasterIntensityHighClip = clip;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMasterIntensityHighClip range error:"
         << "\nHigh clip of " << clip << " is out of range!"
         << "\nMust be greater than low clip of "
         << theMasterIntensityLowClip << " and less than 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setRedHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theRedHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setRedHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setRedHueLowRange(double range)
{
   //***
   // Default red range: 330 - 30
   // Allow 315 to 15 as long as it's less than the high range.
   // Note:  Store the range as a negative if it's between 315 and 360.
   //***
   double r  = range;
   double h  = theRedHueHighRange;

   if (r >= 315.0 && r < 360.0) r = r - 360.0;
   if (h >= 315.0 && h < 360.0) h = h - 360.0;

   if (r >= -45.0 && r <= 15.0 && r < h)
   {
      theRedHueLowRange = r;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setRedHueLow range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setRedHueHighRange(double range)
{
   if (range == 360.0) range = 0.0;
   
   //***
   // Default red range: 330 - 30
   // Allow 345 to 45 as long as it's greater than the low range.
   // Note:  Store the range as a negative if it's between 345 and 360.
   //***

   double r = range;
   double l = theRedHueLowRange;

   if (r >= 315.0 && r < 360.0) r = r - 360.0;
   if (l >= 315.0 && l < 360.0) l = l - 360.0;

   if (r >= -15.0 && r <= 45 && l < r)
   {
      theRedHueHighRange = r;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setRedHueHigh range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setRedHueBlendRange(double range)
{
   //***
   // Allowable range: 0.0 to 30.0 as long as it's not greater than the
   // half range.  If so snap it to half range.
   //***
   if ( range >= 0.0  &&
        range <= MAX_BLEND )
   {
      double half_range = (theRedHueHighRange - theRedHueLowRange) / 2.0;

      if (range <= half_range)
      {
         theRedHueBlendRange = range;
      }
      else
      {
         // Put it in the middle of the range.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHsiRemapper::setRedHueBlendRange range error:"
            << "\nRange of " << range << " is greater than the full range"
            << "\ndivided by 2!"
            << endl;
         
         theRedHueBlendRange = half_range;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setRedHueBlendRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setRedSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theRedSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setRedSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setRedIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theRedIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setRedIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setYellowHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theYellowHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setYellowHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setYellowHueLowRange(double range)
{
   //***
   // Default yellow range: 30 - 90
   // Allow 15 to 75 as long as it's less than the high range.
   //***
   if ( range >= 15.0 &&
        range <= 75.0 &&
        range < theYellowHueHighRange )
   {
      theYellowHueLowRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setYellowHueLowRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setYellowHueHighRange(double range)
{
   //***
   // Default yellow range: 30 - 90
   // Allow 45 to 105 as long as it's greater than the low range.
   //***
   if ( range >= 45.0  &&
        range <= 105.0 &&
        range > theYellowHueLowRange )
   {
      theYellowHueHighRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setYellowHueHighRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setYellowHueBlendRange(double range)
{
   //***
   // Allowable range: 0.0 to 30.0 as long as it's not greater than the
   // half range.  If so snap it to half range.
   //***
   if ( range >= 0.0  &&
        range <= MAX_BLEND )
   {
      double half_range = (theYellowHueHighRange - theYellowHueLowRange) / 2.0;
      
      if (range <= half_range)
      {
         theYellowHueBlendRange = range;
      }
      else
      {
         // Put it in the middle of the range.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHsiRemapper::setYellowHueBlendRange range error:"
            << "\nRange of " << range << " is greater than the full range"
            << "\ndivided by 2!"
            << endl;
         
         theYellowHueBlendRange = half_range;
      } 
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setYellowHueBlendRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setYellowSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theYellowSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setYellowSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setYellowIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theYellowIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setYellowIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setGreenHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theGreenHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setGreenHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setGreenHueLowRange(double range)
{
   //***
   // Default green range:  90 - 150
   // Allow 75 to 135 as long as it's less than the high range.
   //***

   if ( range >= 75.0 &&
        range <= 135.0 &&
        range < theGreenHueHighRange )
   {
      theGreenHueLowRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setGreenHueLowRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setGreenHueHighRange(double range)
{
   //***
   // Default green range:  90 - 150
   // Allow 105 to 165 as long as it's greater than the low range.
   //***

   if ( range >= 105.0  &&
        range <= 165.0 &&
        range > theGreenHueLowRange )
   {
      theGreenHueHighRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setGreenHueHighRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setGreenHueBlendRange(double range)
{
   //***
   // Allowable range: 0.0 to 30.0 as long as it's not greater than the
   // half range.  If so snap it to half range.
   //***
   if ( range >= 0.0  &&
        range <= MAX_BLEND )
   {
      double half_range = (theGreenHueHighRange - theGreenHueLowRange) / 2.0;

      if (range <= half_range)
      {
         theGreenHueBlendRange = range;
      }
      else
      {
         // Put it in the middle of the range.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHsiRemapper::setGreenHueBlendRange range error:"
            << "\nRange of " << range << " is greater than the full range"
            << "\ndivided by 2!"
            << endl;
         
         theGreenHueBlendRange = half_range;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setGreenHueBlendRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setGreenSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theGreenSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setGreenSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setGreenIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theGreenIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setGreenIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setCyanHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theCyanHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setCyanHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setCyanHueLowRange(double range)
{
   //***
   // Default cyan range: 150 - 210
   // Allow 135 to 195 as long as it's less than the high range.
   //***
   
   if ( range >= 135.0 &&
        range <= 195.0 &&
        range < theCyanHueHighRange )
   {
      theCyanHueLowRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setCyanHueLowRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setCyanHueHighRange(double range)
{
   //***
   // Default cyan range: 150 - 210
   // Allow 165 to 225 as long as it's greater than the low range.
   //***
   
   if ( range >= 165.0 &&
        range <= 225.0 &&
        range > theCyanHueLowRange )
   {
      theCyanHueHighRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setCyanHueHighRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setCyanHueBlendRange(double range)
{
   //***
   // Allowable range: 0.0 to 30.0 as long as it's not greater than the
   // half range.  If so snap it to half range.
   //***
   if ( range >= 0.0  &&
        range <= MAX_BLEND )
   {
      double half_range = (theCyanHueHighRange - theCyanHueLowRange) / 2.0;
      
      if (range <= half_range)
      {
         theCyanHueBlendRange = range;
      }
      else
      {
         // Put it in the middle of the range.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHsiRemapper::setCyanHueBlendRange range error:"
            << "\nRange of " << range << " is greater than the full range"
            << "\ndivided by 2!"
            << endl;
         
         theCyanHueBlendRange = half_range;
      } 
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setCyanHueBlendRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setCyanSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theCyanSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setCyanSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setCyanIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theCyanIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setCyanIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setBlueHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theBlueHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setBlueHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setBlueHueLowRange(double range)
{
   //***
   // Default blue range: 210 - 270
   // Allow 195 to 255 as long as it's less than the high range.
   //***
   if ( range >= 195.0 &&
        range <= 255.0 &&
        range < theBlueHueHighRange )
   {
      theBlueHueLowRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setBlueHueLowRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setBlueHueHighRange(double range)
{
   //***
   // Default blue range: 210 - 270
   // Allow 225 to 285 as long as it's greater than the low range.
   //***
   if ( range >= 225.0  &&
        range <= 285.0 &&
        range > theBlueHueLowRange )
   {
      theBlueHueHighRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setBlueHueHighRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setBlueHueBlendRange(double range)
{
   //***
   // Allowable range: 0.0 to 30.0 as long as it's not greater than the
   // half range.  If so snap it to half range.
   //***
   if ( range >= 0.0  &&
        range <= MAX_BLEND )
   {
      double half_range = (theBlueHueHighRange - theBlueHueLowRange) / 2.0;

      if (range <= half_range)
      {
         theBlueHueBlendRange = range;
      }
      else
      {
         // Put it in the middle of the range.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHsiRemapper::setBlueHueBlendRange range error:"
            << "\nRange of " << range << " is greater than the full range"
            << "\ndivided by 2!"
            << endl;
         
         theBlueHueBlendRange = half_range;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setBlueHueBlendRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setBlueSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theBlueSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setBlueSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setBlueIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theBlueIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setBlueIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setMagentaHueOffset(double offset)
{
   if (offset >= -180.0 && offset <= 180.0)
   {
      theMagentaHueOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMagentaHueOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -180.0 and less than"
         << "\nor equal to 180.0"
         << endl;
   }
}

void rspfHsiRemapper::setMagentaHueLowRange(double range)
{
   //***
   // Default magenta range:  270 - 330
   // Allow 255 to 315 as long as it's less than the high range.
   //***
   if ( range >= 255.0 &&
        range <= 315.0 &&
        range < theMagentaHueHighRange )
   {
      theMagentaHueLowRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMagentaHueLowRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setMagentaHueHighRange(double range)
{
   //***
   // Default magenta range:  270 - 330
   // Allow 285 to 345 as long as it's greater than the low range.
   //***
   if ( range >= 285.0  &&
        range <= 345.0 &&
        range > theMagentaHueLowRange )
   {
      theMagentaHueHighRange = range;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMagentaHueHighRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setMagentaHueBlendRange(double range)
{
   //***
   // Allowable range: 0.0 to 30.0 as long as it's not greater than the
   // half range.  If so snap it to half range.
   //***
   if ( range >= 0.0  &&
        range <= MAX_BLEND )
   {
      double half_range = (theMagentaHueHighRange-theMagentaHueLowRange)/2.0;

      if (range <= half_range)
      {
         theMagentaHueBlendRange = range;
      }
      else
      {
         // Put it in the middle of the range.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHsiRemapper::setMagentaHueBlendRange range error:"
            << "\nRange of " << range << " is greater than the full range"
            << "\ndivided by 2!"
            << endl;
         
         theMagentaHueBlendRange = half_range;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMagentaHueBlendRange range error:"
         << "\nRange of " << range << " is out of range!"
         << endl;
   }
}

void rspfHsiRemapper::setMagentaSaturationOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theMagentaSaturationOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMagentaSaturationOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

void rspfHsiRemapper::setMagentaIntensityOffset(double offset)
{
   if (offset >= -1.0 && offset <= 1.0)
   {
      theMagentaIntensityOffset = offset;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHsiRemapper::setMagentaIntensityOffset range error:"
         << "\nOffset of " << offset << " is out of range!"
         << "\nMust be equal to or greater than -1.0 and less than"
         << "\nor equal to 1.0"
         << endl;
   }
}

double rspfHsiRemapper::calculateMinNormValue()
{
   static const char MODULE[] = "rspfHsiRemapper::calculateMinNormValue";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " Entered..." << endl;
   }
   
   if (!theTile)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nClass not initialized!"
         << endl;
      return 0.0;
   }

   double min_pix = theTile->getMinPix(0);
   double max_pix = theTile->getMaxPix(0);

   for (rspf_uint32 band=1; band<theTile->getNumberOfBands(); ++band)
   {
      if (min_pix != theTile->getMinPix(band))
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " Warning:"
            << "\nMixed minimum values for bands..."
            << endl;
      }
      if (max_pix != theTile->getMaxPix(band))
      {
         rspfNotify(rspfNotifyLevel_WARN) << " Warning:"
              << "\nMixed maximum values for bands..."
              << endl;
      }
   }

   if (min_pix < 0.0)
   {
      //
      // Assigning some arbituary number for float data.
      // This should really come from the normalizer (rspfImageData) of
      // the data.
      //
      return RSPF_DEFAULT_MIN_PIX_NORM_DOUBLE;
   }

   return (min_pix / max_pix);
}

rspf_uint32 rspfHsiRemapper::getNumberOfOutputBands() const
{
   if (isSourceEnabled()&&theValidFlag) // Always rgb tile out...
   {
      return 3;
   }

   // Filter bypassed so getTile will return input tile.
   return rspfImageSourceFilter::getNumberOfInputBands();
}


double rspfHsiRemapper::getMasterHueOffset () const
{
   return theMasterHueOffset;
}

double rspfHsiRemapper::getMasterSaturationOffset() const
{
   return theMasterSaturationOffset;
}

double rspfHsiRemapper::getMasterIntensityOffset() const
{
   return theMasterIntensityOffset;
}

double rspfHsiRemapper::getMasterIntensityLowClip() const
{
   return theMasterIntensityLowClip;
}

double rspfHsiRemapper::getMasterIntensityHighClip() const
{
   return theMasterIntensityHighClip;
}

double rspfHsiRemapper::getRedHueOffset() const
{
   return theRedHueOffset;
}

double rspfHsiRemapper::getRedHueLowRange() const
{
   return theRedHueLowRange;
}

double rspfHsiRemapper::getRedHueHighRange() const
{
   return theRedHueHighRange;
}

double rspfHsiRemapper::getRedHueBlendRange() const
{
   return theRedHueBlendRange;
}

double rspfHsiRemapper::getRedSaturationOffset() const
{
   return theRedSaturationOffset;
}

double rspfHsiRemapper::getRedIntensityOffset() const
{
   return theRedIntensityOffset;
}

double rspfHsiRemapper::getYellowHueOffset () const
{
   return theYellowHueOffset;
}

double rspfHsiRemapper::getYellowHueLowRange() const
{
   return theYellowHueLowRange;
}

double rspfHsiRemapper::getYellowHueHighRange() const
{
   return theYellowHueHighRange;
}

double rspfHsiRemapper::getYellowHueBlendRange() const
{
   return theYellowHueBlendRange;
}

double rspfHsiRemapper::getYellowSaturationOffset() const
{
   return theYellowSaturationOffset;
}

double rspfHsiRemapper::getYellowIntensityOffset() const
{
   return theYellowIntensityOffset;
}

double rspfHsiRemapper::getGreenHueOffset () const
{
   return theGreenHueOffset;
}

double rspfHsiRemapper::getGreenHueLowRange() const
{
   return theGreenHueLowRange;
}

double rspfHsiRemapper::getGreenHueHighRange() const
{
   return theGreenHueHighRange;
}

double rspfHsiRemapper::getGreenHueBlendRange() const
{
   return theGreenHueBlendRange;
}

double rspfHsiRemapper::getGreenSaturationOffset() const
{
   return theGreenSaturationOffset;
}

double rspfHsiRemapper::getGreenIntensityOffset() const
{
   return theGreenIntensityOffset;
}

double rspfHsiRemapper::getCyanHueOffset () const
{
   return theCyanHueOffset;
}

double rspfHsiRemapper::getCyanHueLowRange() const
{
   return theCyanHueLowRange;
}

double rspfHsiRemapper::getCyanHueHighRange() const
{
   return theCyanHueHighRange;
}

double rspfHsiRemapper::getCyanHueBlendRange() const
{
   return theCyanHueBlendRange;
}

double rspfHsiRemapper::getCyanSaturationOffset() const
{
   return theCyanSaturationOffset;
}

double rspfHsiRemapper::getCyanIntensityOffset() const
{
   return theCyanIntensityOffset;
}

double rspfHsiRemapper::getBlueHueOffset () const
{
   return theBlueHueOffset;
}

double rspfHsiRemapper::getBlueHueLowRange() const
{
   return theBlueHueLowRange;
}

double rspfHsiRemapper::getBlueHueHighRange() const
{
   return theBlueHueHighRange;
}

double rspfHsiRemapper::getBlueHueBlendRange() const
{
   return theBlueHueBlendRange;
}

double rspfHsiRemapper::getBlueSaturationOffset() const
{
   return theBlueSaturationOffset;
}

double rspfHsiRemapper::getBlueIntensityOffset() const
{
   return theBlueIntensityOffset;
}

double rspfHsiRemapper::getMagentaHueOffset () const
{
   return theMagentaHueOffset;
}

double rspfHsiRemapper::getMagentaHueLowRange() const
{
   return theMagentaHueLowRange;
}

double rspfHsiRemapper::getMagentaHueHighRange() const
{
   return theMagentaHueHighRange;
}

double rspfHsiRemapper::getMagentaHueBlendRange() const
{
   return theMagentaHueBlendRange;
}

double rspfHsiRemapper::getMagentaSaturationOffset() const
{
   return theMagentaSaturationOffset;
}

double rspfHsiRemapper::getMagentaIntensityOffset() const
{
   return theMagentaIntensityOffset;
}

void rspfHsiRemapper::setHueOffset(int color_group, double offset)
{
   switch (color_group)
   {
      case RED:
         setRedHueOffset(offset);
         break;
      case YELLOW:
         setYellowHueOffset(offset);
         break;
      case GREEN:
         setGreenHueOffset(offset);
         break;
      case CYAN:
         setCyanHueOffset(offset);
         break;
      case BLUE:
         setBlueHueOffset(offset);
         break;
      case MAGENTA:
         setMagentaHueOffset(offset);
         break;
      case ALL:
         setMasterHueOffset(offset);
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setHueOffset NOTICE:  Range error!" << endl;
   }
}

void rspfHsiRemapper::setHueLowRange(int color_group, double range)
{
   switch (color_group)
   {
      case RED:
         setRedHueLowRange(range);
         break;
      case YELLOW:
         setYellowHueLowRange(range);
         break;
      case GREEN:
         setGreenHueLowRange(range);
         break;
      case CYAN:
         setCyanHueLowRange(range);
         break;
      case BLUE:
         setBlueHueLowRange(range);
         break;
      case MAGENTA:
         setMagentaHueLowRange(range);
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setHueLowRange NOTICE:  Range error!"
            << endl;
   }
}

void rspfHsiRemapper::setHueHighRange(int color_group, double range)
{
   switch (color_group)
   {
      case RED:
         setRedHueHighRange(range);
         break;
      case YELLOW:
         setYellowHueHighRange(range);
         break;
      case GREEN:
         setGreenHueHighRange(range);
         break;
      case CYAN:
         setCyanHueHighRange(range);
         break;
      case BLUE:
         setBlueHueHighRange(range);
         break;
      case MAGENTA:
         setMagentaHueHighRange(range);
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setHueHighRange NOTICE:  Range error!"
            << endl;
   }
}

void rspfHsiRemapper::setHueBlendRange(int color_group, double range)
{
   switch (color_group)
   {
      case RED:
         setRedHueBlendRange(range);
         break;
      case YELLOW:
         setYellowHueBlendRange(range);
         break;
      case GREEN:
         setGreenHueBlendRange(range);
         break;
      case CYAN:
         setCyanHueBlendRange(range);
         break;
      case BLUE:
         setBlueHueBlendRange(range);
         break;
      case MAGENTA:
         setMagentaHueBlendRange(range);
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setHueBlendRange NOTICE:  Range error!"
            << endl;
   }
}

void rspfHsiRemapper::setSaturationOffset(int color_group, double offset)
{
   switch (color_group)
   {
      case RED:
         setRedSaturationOffset(offset);
         break;
      case YELLOW:
         setYellowSaturationOffset(offset);
         break;
      case GREEN:
         setGreenSaturationOffset(offset);
         break;
      case CYAN:
         setCyanSaturationOffset(offset);
         break;
      case BLUE:
         setBlueSaturationOffset(offset);
         break;
      case MAGENTA:
         setMagentaSaturationOffset(offset);
         break;
      case ALL:
         setMasterSaturationOffset(offset);
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setSaturationOffset NOTICE:  Range error!"
            << endl;
         break;
   }
}

void rspfHsiRemapper::setIntensityOffset(int color_group, double offset)
{
   switch (color_group)
   {
      case RED:
         setRedIntensityOffset(offset);
         break;
      case YELLOW:
         setYellowIntensityOffset(offset);
         break;
      case GREEN:
         setGreenIntensityOffset(offset);
         break;
      case CYAN:
         setCyanIntensityOffset(offset);
         break;
      case BLUE:
         setBlueIntensityOffset(offset);
         break;
      case MAGENTA:
         setMagentaIntensityOffset(offset);
         break;
      case ALL:
         setMasterIntensityOffset(offset);
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::setIntensityOffset NOTICE:  Range error!"
            << endl;
         break;
   }
}

double rspfHsiRemapper::getHueOffset(int color_group) const
{
   switch (color_group)
   {
      case RED:
         return getRedHueOffset();
         break;
      case YELLOW:
         return getYellowHueOffset();
         break;
      case GREEN:
         return getGreenHueOffset();
         break;
      case CYAN:
         return getCyanHueOffset();
         break;
      case BLUE:
         return getBlueHueOffset();
         break;
      case MAGENTA:
         return getMagentaHueOffset();
         break;
      case ALL:
         return getMasterHueOffset();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::getHueOffset NOTICE:  Range error!"
            << endl;
         break;
   }
   return 0.0;
}

double rspfHsiRemapper::getHueLowRange(int color_group) const
{
   switch (color_group)
   {
      case RED:
         return getRedHueLowRange();
         break;
      case YELLOW:
         return getYellowHueLowRange();
         break;
      case GREEN:
         return getGreenHueLowRange();
         break;
      case CYAN:
         return getCyanHueLowRange();
         break;
      case BLUE:
         return getBlueHueLowRange();
         break;
      case MAGENTA:
         return getMagentaHueLowRange();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::getHueLowRange NOTICE:  Range error!"
            << endl;
         break;
   }
   return 0.0;
}

double rspfHsiRemapper::getHueHighRange(int color_group) const
{
   switch (color_group)
   {
      case RED:
         return getRedHueHighRange();
         break;
      case YELLOW:
         return getYellowHueHighRange();
         break;
      case GREEN:
         return getGreenHueHighRange();
         break;
      case CYAN:
         return getCyanHueHighRange();
         break;
      case BLUE:
         return getBlueHueHighRange();
         break;
      case MAGENTA:
         return getMagentaHueHighRange();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::getHueHighRange NOTICE:  Range error!"
            << endl;
         break;
   }
   return 0.0;
}

double rspfHsiRemapper::getHueBlendRange(int color_group) const
{
   switch (color_group)
   {
      case RED:
         return getRedHueBlendRange();
         break;
      case YELLOW:
         return getYellowHueBlendRange();
         break;
      case GREEN:
         return getGreenHueBlendRange();
         break;
      case CYAN:
         return getCyanHueBlendRange();
         break;
      case BLUE:
         return getBlueHueBlendRange();
         break;
      case MAGENTA:
         return getMagentaHueBlendRange();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::getHueBlendRange NOTICE:  Range error!"
            << endl;
         break;
   }
   return 0.0;
}

double rspfHsiRemapper::getSaturationOffset(int color_group) const
{
   switch (color_group)
   {
      case RED:
         return getRedSaturationOffset();
         break;
      case YELLOW:
         return getYellowSaturationOffset();
         break;
      case GREEN:
         return getGreenSaturationOffset();
         break;
      case CYAN:
         return getCyanSaturationOffset();
         break;
      case BLUE:
         return getBlueSaturationOffset();
         break;
      case MAGENTA:
         return getMagentaSaturationOffset();
         break;
      case ALL:
         return getMasterSaturationOffset();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::getSaturationOffset NOTICE:  Range error!"
            << endl;
         break;
   }
   return 0.0;
}

double rspfHsiRemapper::getIntensityOffset(int color_group) const
{
   switch (color_group)
   {
      case RED:
         return getRedIntensityOffset();
         break;
      case YELLOW:
         return getYellowIntensityOffset();
         break;
      case GREEN:
         return getGreenIntensityOffset();
         break;
      case CYAN:
         return getCyanIntensityOffset();
         break;
      case BLUE:
         return getBlueIntensityOffset();
         break;
      case MAGENTA:
         return getMagentaIntensityOffset();
         break;
      case ALL:
         return getMasterIntensityOffset();
         break;
      default:
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfHsiRemapper::getIntensityOffset NOTICE:  Range error!"
            << endl;
         break;
   }
   return 0.0;
}

void rspfHsiRemapper::setWhiteObjectClip(double clip)
{
   if ( (clip >= .8) && (clip <= 1.0) )
   {
      theWhiteObjectClip = clip;
      verifyEnabled();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "HsiRemapper::setWhiteObjectClip range error:"
         << "\nClip of " << clip << " is out of range!"
         << "\nMust be between .8 and 1.0"
         << endl;
   }
}

double rspfHsiRemapper::getWhiteObjectClip() const
{
   return theWhiteObjectClip;
}

void rspfHsiRemapper::resetWhiteObjectClip()
{
   theWhiteObjectClip = 1.0;
   verifyEnabled();
}

rspfString rspfHsiRemapper::getLongName()const
{
   return rspfString("HSI Remapper, filter with controls for all HSI(Hue, Saturation, and Intensity) adjustments.");
}

rspfString rspfHsiRemapper::getShortName()const
{
   return rspfString("HSI Remapper");
}

ostream& rspfHsiRemapper::print(ostream& os) const
{
   os << setiosflags(ios::fixed) << setprecision(2)
      << "\nrspfHsiRemapper:"
      << "\ntheEnableFlag:                " << theEnableFlag

      << "\ntheMasterHueOffset:           " << theMasterHueOffset
      << "\ntheMasterSaturationOffset:    " << theMasterSaturationOffset
      << "\ntheMasterIntensityOffset:     " << theMasterIntensityOffset
      << "\ntheMasterIntensityLowClip:    " << theMasterIntensityLowClip
      << "\ntheMasterIntensityHighClip:   " << theMasterIntensityHighClip
      
      << "\ntheRedHueOffset:              " << theRedHueOffset
      << "\ntheRedHueLowRange:            " << theRedHueLowRange
      << "\ntheRedHueHighRange:           " << theRedHueHighRange
      << "\ntheRedSaturationOffset:       " << theRedSaturationOffset
      << "\ntheRedIntensityOffset:        " << theRedIntensityOffset
      
      << "\ntheYellowHueOffset:           " << theYellowHueOffset
      << "\ntheYellowHueLowRange:         " << theYellowHueLowRange
      << "\ntheYellowHueHighRange:        " << theYellowHueHighRange
      << "\ntheYellowSaturationOffset:    " << theYellowSaturationOffset
      << "\ntheYellowIntensityOffset:     " << theYellowIntensityOffset
      
      << "\ntheGreenHueOffset:            " << theGreenHueOffset
      << "\ntheGreenHueLowRange:          " << theGreenHueLowRange
      << "\ntheGreenHueHighRange:         " << theGreenHueHighRange
      << "\ntheGreenSaturationOffset:     " << theGreenSaturationOffset
      << "\ntheGreenIntensityOffset:      " << theGreenIntensityOffset
      
      << "\ntheCyanHueOffset:             " << theCyanHueOffset
      << "\ntheCyanHueLowRange:           " << theCyanHueLowRange
      << "\ntheCyanHueHighRange:          " << theCyanHueHighRange
      << "\ntheCyanSaturationOffset:      " << theCyanSaturationOffset
      << "\ntheCyanIntensityOffset:       " << theCyanIntensityOffset
      
      << "\ntheBlueHueOffset:             " << theBlueHueOffset
      << "\ntheBlueHueLowRange:           " << theBlueHueLowRange
      << "\ntheBlueHueHighRange:          " << theBlueHueHighRange
      << "\ntheBlueSaturationOffset:      " << theBlueSaturationOffset
      << "\ntheBlueIntensityOffset:       " << theBlueIntensityOffset
      
      << "\ntheMagentaHueOffset:          " << theMagentaHueOffset
      << "\ntheMagentaHueLowRange:        " << theMagentaHueLowRange
      << "\ntheMagentaHueHighRange:       " << theMagentaHueHighRange
      << "\ntheMagentaSaturationOffset:   " << theMagentaSaturationOffset
      << "\ntheMagentaIntensityOffset:    " << theMagentaIntensityOffset

      << "\ntheWhiteObjectClip:           " << theWhiteObjectClip
      << endl;
   
   return os;
}

ostream& operator<<(ostream& os, const rspfHsiRemapper& hr)
{
   return hr.print(os);
}
