//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  Class provides an elevation image source from the elevation
// manager.
// 
//*******************************************************************
//  $Id: rspfElevImageSource.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfElevImageSource.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/projection/rspfLlxyProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfImageData.h>
#include <ctime>

RTTI_DEF1(rspfElevImageSource, "rspfElevImageSource", rspfImageSource)

// Static trace for debugging
static rspfTrace traceDebug("rspfElevImage:debug");
   
rspfElevImageSource::rspfElevImageSource()
   :
      rspfImageSource(NULL,
                       0,
                       0,
                       true,
                       false),// output list is not fixed
      theElevManager(NULL),
      theTile(NULL),
      theTiePoint(),
      theLatSpacing(0.0),
      theLonSpacing(0.0),
      theNumberOfLines(0),
      theNumberOfSamps(0)
{}

rspfElevImageSource::rspfElevImageSource(rspfObject* owner)
   :
      rspfImageSource(owner,
                       0,
                       0,
                       true,
                       false),// output list is not fixed
      theElevManager(NULL),
      theTile(NULL),
      theTiePoint(),
      theLatSpacing(0.0),
      theLonSpacing(0.0),
      theNumberOfLines(0),
      theNumberOfSamps(0)
{}

rspfElevImageSource::rspfElevImageSource(rspfObject* owner,
                                           const rspfGpt& tie,
                                           double latSpacing,
                                           double lonSpacing,
                                           rspf_uint32 numberLines,
                                           rspf_uint32 numberSamples)
   :
      rspfImageSource(owner,
                       0,
                       0,
                       true,
                       false),// output list is not fixed
      theElevManager(NULL),
      theTile(NULL),
      theTiePoint(tie),
      theLatSpacing(latSpacing),
      theLonSpacing(lonSpacing),
      theNumberOfLines(numberLines),
      theNumberOfSamps(numberSamples)
{
   initialize();
}

rspfElevImageSource::rspfElevImageSource(rspfObject* owner,
                                           const rspfKeywordlist& kwl,
                                           const char* prefix)
   :
      rspfImageSource(owner,
                       0,
                       0,
                       true,
                       false),
      theElevManager(NULL),
      theTile(NULL),
      theTiePoint(),
      theLatSpacing(0),
      theLonSpacing(0),
      theNumberOfLines(0),
      theNumberOfSamps(0)
{
   if (loadState(kwl, prefix) == false)
   {
      setErrorStatus();
   }
}

rspfElevImageSource::~rspfElevImageSource()
{
}

rspfRefPtr<rspfImageData> rspfElevImageSource::getTile(
   const  rspfIrect& tile_rect,
   rspf_uint32 resLevel)
{
   if (!theTile.get())
   {
      return theTile;
   }
   
   // First make sure our tile is the right size.
   rspf_int32 w = tile_rect.width();
   rspf_int32 h = tile_rect.height();
   rspf_int32 tileW = theTile->getWidth();
   rspf_int32 tileH = theTile->getHeight();
   if( (w != tileW) || (h != tileH) )
   {
      theTile->setWidth(w);
      theTile->setHeight(h);
      if((w*h)!=(tileW*tileH))
      {
         theTile->initialize();

         //***
         // Initialize can reset the min max to defaults if the min happens
         // to be "0" so reset it just in case.
         // NOTE:  We need to fix initialize!
         //***
         theTile->setMinPix(theElevManager->getMinHeightAboveMSL(), 0);
         theTile->setMaxPix(theElevManager->getMaxHeightAboveMSL(), 0);
      }
   }

   // Set the origin.
   theTile->setOrigin(tile_rect.ul());
      
   
   if(!isSourceEnabled())
   {
      // This tile source bypassed.
      theTile->makeBlank();
      return theTile;
   }

   //***
   // No overview support yet...
   //***
   if (resLevel)
   {
      // NOTE:  Need to add overview support.
      cerr << "rspfElevImageSource::getTile ERROR:\nOverviews not supported!"
           << endl;
      theTile->makeBlank();
      return theTile;
   }

   rspfIrect image_rect = getImageRectangle(0);

   if ( !tile_rect.intersects(image_rect) )
   {
      // No point in the tile falls within the set boundaries of this source.
      theTile->makeBlank();
      return theTile;
   }

   // Ok fill the tile with the data from the post...
   rspfIrect clip_rect = tile_rect.clipToRect(image_rect);

   if ( !tile_rect.completely_within(clip_rect) )
   {
      // Start with a blank tile since it won't be filled all the way.
      theTile->makeBlank();
   }


   // Move the buffer pointer to the first valid pixel.
   rspf_uint32 tile_width = theTile->getWidth();
   
   rspf_int32 start_offset = (clip_rect.lr().y - tile_rect.ul().y) * tile_width +
      clip_rect.ul().x - tile_rect.ul().x;

   //***
   // Since most elevation formats have posts organized positive line up,
   // start at the lower left side of the cell so all reads are going
   // forward in the file.
   //***
   double start_lat = theTiePoint.latd() - theLatSpacing *
      (clip_rect.lr().y - image_rect.ul().y);
   if (start_lat < -90.0)
   {
      start_lat = -(start_lat + 180.0);  // Wrapped around the south poll.
   }

   double lon = theTiePoint.lond() + theLonSpacing *
      (clip_rect.ul().x  - image_rect.ul().x);
   if (lon > 180.0)
   {
      lon -= 360.0; // Went across the central meridian.
   }

   // Copy the data.
   rspf_uint32 clipHeight = clip_rect.height();
   rspf_uint32 clipWidth  = clip_rect.width();

   // Get a pointer to the tile buffer.
   rspf_float32* buf = static_cast<rspf_float32*>(theTile->getBuf());

   for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
   {
      double lat = start_lat;
      rspf_int32 offset = start_offset;
      for (rspf_uint32 line = 0; line < clipHeight; ++line)
      {
         rspfGpt gpt(lat, lon);
         buf[offset+sample] = theElevManager->getHeightAboveMSL(gpt);
         
         lat += theLatSpacing;
         if (lat > 90) lat = 180.0 - lat;

         offset -= tile_width;
      }
      
      lon += theLonSpacing;
      if (lon > 180.0) lon = lon - 360.0; // Went across the central meridian.
   }
   
#if 0   
   for (rspf_uint32 line = 0; line < clipHeight; ++line)
   {
      double lon = start_lon;
      for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
      {
         rspfGpt gpt(lat, lon);
         buf[sample] = theElevManager->getHeightAboveMSL(gpt);
         lon += theLonSpacing;
         if (start_lon > 180.0)
         {
            start_lon -= 360.0; // Went across the central meridian.
         }
      }

      buf += tile_width;
      lat -= theLatSpacing;
      if (lat < -90.0) lat = -(lat + 180.0);// Wrapped around the south poll.
   }
#endif
   
   theTile->validate();
   return theTile;
}

bool rspfElevImageSource::saveState(rspfKeywordlist& kwl,
                                  const char* prefix) const
{
   static const char MODULE[] = "rspfElevImageSource::saveState";

   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      cerr << MODULE
           << " ERROR detected in keyword list!  State not saved."
           << endl;
      return false;
   }

   // Save the state of the base class.
   rspfImageSource::saveState(kwl, prefix);

   // Save the tie point.
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LAT_KW,
           theTiePoint.latd(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LON_KW,
           theTiePoint.lond(),
           true);

   // Save the post spacing.
   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
           theLatSpacing,
           true);
   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
           theLonSpacing,
           true);

   // Save the image size
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_LINES_KW,
           theNumberOfLines,
           true);
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_SAMPLES_KW,
           theNumberOfSamps,
           true);

   // Save the min / max pixel values.
   kwl.add(prefix,
           rspfKeywordNames::MIN_VALUE_KW,
           getMinPixelValue(0),
           true);
   kwl.add(prefix,
           rspfKeywordNames::MAX_VALUE_KW,
           getMaxPixelValue(0),
           true);

   return true;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfElevImageSource::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   static const char MODULE[] = "rspfElevImageSource::loadState";

   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      cerr << MODULE
           << " ERROR detected in keyword list!  State not load."
           << endl;
      return false;
   }

   // Base class...
   rspfImageSource::loadState(kwl, prefix);
   
   const char* lookup;

   // Get the tie point.
   lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_LAT_KW);
   if (lookup)
   {
      theTiePoint.latd(rspfString(lookup).toDouble());
   }
   else
   {
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nRequired keyword not found:  "
              << rspfKeywordNames::TIE_POINT_LAT_KW
              << "\nReturning false"
              << endl;
      }

      return false;
   }
   
   lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_LON_KW);
   if (lookup)
   {
      theTiePoint.lond(rspfString(lookup).toDouble());
   }
   else
   {
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nRequired keyword not found:  "
              << rspfKeywordNames::TIE_POINT_LON_KW
              << "\nReturning false"
              << endl;
      }

      return false;
   }

   // Get the post spacing.
   lookup = kwl.find(prefix, rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT);
   if (lookup)
   {
      theLatSpacing = rspfString(lookup).toDouble();
   }
   else
   {
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nRequired keyword not found:  "
              << rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT
              << "\nReturning false"
              << endl;
      }

      return false;
   }

   lookup = kwl.find(prefix, rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON);
   if (lookup)
   {
      theLonSpacing = rspfString(lookup).toDouble();
   }
   else
   {
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nRequired keyword not found:  "
              << rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON
              << "\nReturning false"
              << endl;
      }

      return false;
   }

   // Get the image size.
   lookup = kwl.find(prefix, rspfKeywordNames::NUMBER_LINES_KW);
   if (lookup)
   {
      theNumberOfLines = rspfString(lookup).toULong();
   }
   else
   {
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nRequired keyword not found:  "
              << rspfKeywordNames::NUMBER_LINES_KW
              << "\nReturning false"
              << endl;
      }

      return false;
   }

   lookup = kwl.find(prefix, rspfKeywordNames::NUMBER_SAMPLES_KW);
   if (lookup)
   {
      theNumberOfSamps = rspfString(lookup).toULong();
   }
   else
   {
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nRequired keyword not found:  "
              << rspfKeywordNames::NUMBER_SAMPLES_KW
              << "\nReturning false"
              << endl;
      }

      return false;
   }

   initialize();

   //***
   // See if the min / max keyword was set and reset it.
   // Note this must be done after initialize since it sets the min / max from
   // the elevation manager.
   //***
   lookup = kwl.find(prefix, rspfKeywordNames::MIN_VALUE_KW);
   if (lookup)
   {
      setMinPixelValue(rspfString(lookup).toDouble());
   }
   lookup = kwl.find(prefix, rspfKeywordNames::MAX_VALUE_KW);
   if (lookup)
   {
      setMaxPixelValue(rspfString(lookup).toDouble());
   }

   if (getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      return false;
   }

   //***
   // Reset the base class to have a fixed input list of "0" size.
   // Note:  To not do this will result in a core dump destroying objects
   // connected to this output.
   //***
   theInputObjectList.clear();
   theInputListIsFixedFlag = true;
   theOutputListIsFixedFlag = false;
  
   return true;
}

void rspfElevImageSource::initialize()
{
   static const char MODULE[] = "rspfElevImageSource::initialize";
   
   if (traceDebug()) CLOG << " Entered..." << endl;
   
   //***
   // First see if the manager pointer has been captured.
   //***
   if (!theElevManager) theElevManager = rspfElevManager::instance();

   if (!theElevManager)
   {
      setErrorStatus();
      cerr << MODULE << "ERROR:\nNULL elevation manager pointer!"
           << "\nObject not initialized!" << endl;
      return;
   }

   // Basic sanity checks.
   if (!theLatSpacing || !theLonSpacing ||
       !theNumberOfLines || !theNumberOfSamps)
   {
      setErrorStatus();
      cerr << MODULE << "ERROR:"
           << "\nMust set latitude/longitude spacing and number of line and"
           << " samples."
           << "Object not initialized!" << endl;
      return;
      
   }

   // Check the ground point.
   if ( theTiePoint.latd() > 90.0  || theTiePoint.latd() < -90.0  ||
        theTiePoint.lond() > 180.0 || theTiePoint.lond() < -180.0 )
   {
      setErrorStatus();
      cerr << MODULE << "ERROR:\nBogus tie point."
           << "\nObject not initialized!" << endl;
      return;
   }

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nTie point:  " << theTiePoint
           << "\nLatitude spacing:  " << theLatSpacing
           << "\nLongitude spacing:  " << theLonSpacing
           << "\nLines:              " << theNumberOfLines
           << "\nSamples:            " << theNumberOfSamps
           << endl;
   }
   
   //***
   // Since this will return float data we need to set the min / max values
   // of the data so that anybody who remaps it to eight bit will do it
   // properly.  So scan the entire image rectangle using the manager.
   // This will do two things, force the elevation manager to load all the
   // cells, and at the same time the min / max elevation value will be set.
   // NOTE:
   // ??? Should the elevation manager clear the list of sources prior to
   // the code segment so that the min / max is only from the cells we
   // need!
   //***

   cout << "Initializing elevation manager..." << endl;

#if 0
   // Loop in the longitude or sample direction.
   time_t start_t = time(NULL);
   double lon = theTiePoint.lond();
   for (rspf_uint32 samp = 0; samp < theNumberOfSamps; ++samp)
   {
      double lat = theTiePoint.latd() - theLatSpacing * (theNumberOfLines - 1);
      if (lat < -90.0) lat = -(lat + 180.0); // Wrapped around the south poll.
      
      for (rspf_uint32 line = 0; line < theNumberOfLines; ++line)
      {
         rspfGpt gpt(lat, lon);
         theElevManager->getHeightAboveMSL(gpt);

         lat += theLatSpacing;
         if (lat > 90) lat = 180 - lat; // Went across poll.
      }
      
      lon += theLonSpacing;
      if (lon > 180.0) lon -= 360.0; // Went across the central meridian.
   }
   time_t stop_t = time(NULL);
   cout << "Finished loop two..." << endl;
   cout << "Elapsed time for loop two:  " << (stop_t - start_t) << endl;
#endif
   
   theTile = new rspfImageData(this,
                                RSPF_FLOAT);
   theTile->initialize();

   // Set the min / max for any normalization down the chain...
   theTile->setMinPix(theElevManager->getMinHeightAboveMSL(), 0);
   theTile->setMaxPix(theElevManager->getMaxHeightAboveMSL(), 0);

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nMin pix:            " << theTile->getMinPix(0)
           << "\nMax pix:            " << theTile->getMaxPix(0)
           << endl;
   }
}

rspfIrect rspfElevImageSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   rspfIrect result(0, 0, theNumberOfSamps-1, theNumberOfLines-1);
   
   if (reduced_res_level != 0)
   {
      cerr << "rspfElevImageSource::getImageRectangle ERROR:"
           << "\nOnly R0 is supported." << endl;
   }

   return result;
}

bool rspfElevImageSource::getImageGeometry(rspfKeywordlist& kwl,
                                            const char* prefix)
{
   // Save off the image dimensions.
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_LINES_KW,
           theNumberOfLines,
           true);
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_SAMPLES_KW,
           theNumberOfLines,
           true);

   // Save off the projection info (tie and post spacing).
   rspfLlxyProjection proj(theTiePoint, theLatSpacing, theLonSpacing);
   return proj.saveState(kwl, prefix);
}

void rspfElevImageSource::getDecimationFactor(rspf_uint32 resLevel, rspfDpt& result) const
{
   if (resLevel)
   {
      cerr << "rspfElevImageSource::getDecimationFactor ERROR:"
           << "\nReduced res sets currently not supported!"
           << endl;
   }

   result.line = 1.0;
   result.samp = 1.0;
}

void rspfElevImageSource::getDecimationFactors(vector<rspfDpt>& decimations) const
{
   rspfDpt pt(1.0, 1.0);
   decimations.clear();
   decimations.push_back(pt);
}

rspf_uint32 rspfElevImageSource::getNumberOfDecimationLevels()const
{
   return 1;
}

rspf_uint32 rspfElevImageSource::getNumberOfInputBands() const
{
   return 1;
}

rspfScalarType rspfElevImageSource::getOutputScalarType() const
{
   return RSPF_FLOAT;
}

rspf_uint32 rspfElevImageSource::getTileWidth() const
{
   if (theTile.get()) return theTile->getWidth();

   return 0;
}

rspf_uint32 rspfElevImageSource::getTileHeight() const
{
   if (theTile.get()) return theTile->getHeight();

   return 0;
}

void rspfElevImageSource::changeTileSize(const rspfIpt& size)
{
   if (traceDebug())
   {
      cout << "rspfElevImageSource::changeTileSize DEBUG:"
           << "\nx size:  " << size.x
           << "\ny size:  " << size.y
           << endl;
   }

   if (!theTile)
   {
      cerr << "rspfElevImageSource::changeTileSize ERROR:"
           << "\nObject not initialized!  Returning..." << endl;
      return;
   }

   theTile = new rspfImageData(this,
                                RSPF_FLOAT,
                                1,
                                size.x,
                                size.y);
   theTile->initialize();
   
   // Set the min / max for any normalization down the chain...
   theTile->setMinPix(theElevManager->getMinHeightAboveMSL(), 0);
   theTile->setMaxPix(theElevManager->getMaxHeightAboveMSL(), 0);
}

rspf_uint32 rspfElevImageSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level)
   {
      cerr << "rspfElevImageSource::getNumberOfLines ERROR:"
           << "\nReduced res sets currently not supported!"
           << endl;
      return 0;
   }

   return theNumberOfLines;
}

rspf_uint32 rspfElevImageSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level)
   {
      cerr << "rspfElevImageSource::getNumberOfSamples ERROR:"
           << "\nReduced res sets currently not supported!"
           << endl;
      return 0;
   }

   return theNumberOfSamps;
}

double rspfElevImageSource::getMinPixelValue(rspf_uint32 band) const
{
   if (band)
   {
      cerr << "rspfElevImageSource::getMinPixelValue ERROR:"
           << "\nReduced res sets currently not supported!"
           << endl;
      return 0.0;
   }

   if (theTile.get())
   {
      return theTile->getMinPix(0);
   }
   else if (theElevManager)
   {
      return theElevManager->getMinHeightAboveMSL();
   }

   return 0.0;
}

double rspfElevImageSource::getMaxPixelValue(rspf_uint32 band) const
{
   if (band)
   {
      cerr << "rspfElevImageSource::getMaxPixelValue ERROR:"
           << "\nReduced res sets currently not supported!"
           << endl;
      return 0.0;
   }

   if (theTile.get())
   {
      return theTile->getMaxPix(0);
   }
   else if (theElevManager)
   {
      return theElevManager->getMaxHeightAboveMSL();
   }

   return 0.0;
}

void rspfElevImageSource::setMinPixelValue(rspf_float64 min_pix)
{
   if (theTile.get()) 
   {
      theTile->setMinPix(min_pix, 0);
      if (traceDebug())
      {
         cout << "rspfElevImageSource::setMinPixelValue DEBUG:"
              << "\nMin pixel value:  " << min_pix
              << endl;
      }
   }
   else
   {
      cerr << "rspfElevImageSource::setMinPixelValue ERROR:"
           << "\nObject not initialized!"
           << endl;
   }
}

void rspfElevImageSource::setMaxPixelValue(rspf_float64 max_pix)
{
   if (theTile.get()) 
   {
      theTile->setMaxPix(max_pix, 0);
      if (traceDebug())
      {
         cout << "rspfElevImageSource::setMaxPixelValue DEBUG:"
              << "\nMax pixel value:  " << max_pix
              << endl;
      }
   }
   else
   {
      cerr << "rspfElevImageSource::setMinPixelValue ERROR:"
           << "\nObject not initialized!"
           << endl;
   }
}      
