//*************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
// Description:
// 
// Image handler used for tile-files when the full image is distributed across multiple files,
// as is the case for tiled Quickbird imagery. This is not for use with band-separate formats,
// but only "spatially-separate" schemes.
//
//*************************************************************************************************
//  $Id: rspfTiledImageHandler.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $

#include <algorithm>

#include <rspf/imaging/rspfTiledImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRegExp.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfTiffOverviewBuilder.h>
#include <rspf/imaging/rspfTiffTileSource.h>

RTTI_DEF1(rspfTiledImageHandler, "rspfTiledImageHandler", rspfImageHandler)

// Static trace for debugging
static rspfTrace traceDebug("rspfTiledImageHandler:debug");

//*************************************************************************************************
//!  Constructor (default):
//*************************************************************************************************
rspfTiledImageHandler::rspfTiledImageHandler()
   : m_tileFiles(0),
     m_tile(0),
     m_fullImgRect(),
#if USING_SUB_OVRS
     m_startOvrResLevel(-1),
     m_lockSubOvrs(false)
#else
     m_startOvrResLevel(1)
#endif
{
}

//*************************************************************************************************
//! Destructor:
//*************************************************************************************************
rspfTiledImageHandler::~rspfTiledImageHandler()
{
   close();
}

//*************************************************************************************************
//! Deletes the overview and clears the valid image vertices.  Derived
//! classes should implement.
//*************************************************************************************************
void rspfTiledImageHandler::close()
{
   vector<rspfTileFile>::iterator iter = m_tileFiles.begin();
   while (iter != m_tileFiles.end())
   {
      (*iter).imageHandler->close();
      ++iter;
   }
   m_tileFiles.clear();
}

//*************************************************************************************************
//! Derived classes must implement this method to be concrete.
//! @return true if open, false if not.
//*************************************************************************************************
bool rspfTiledImageHandler::isOpen()const
{
   if (m_tileFiles.size() > 0)
      return true;

   return false;
}

//*************************************************************************************************
//! Initialize tile buffer to natch image datatype
//*************************************************************************************************
void rspfTiledImageHandler::allocate()
{
   m_tile = 0;
   if ((m_tileFiles.size() == 0))
      return;

   // Just copy the image data tile of the first tile-file:
   rspfRefPtr<rspfImageHandler> h0 = m_tileFiles[0].imageHandler;
   rspfIrect rect (h0->getBoundingRect());
   rect.set_lr(rect.ul() + rspfIpt(h0->getTileWidth()-1, h0->getTileHeight()-1));
   rspfRefPtr<rspfImageData> source_tile = m_tileFiles[0].imageHandler->getTile(rect);
   if (!source_tile.valid())
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "rspfTiledImageHandler::allocate() -- Could not"
         "determine file-tile image data for allocating image tile. Aborting."<<endl;
      return;
   }

   m_tile = (rspfImageData*) source_tile->dup();
}

//*************************************************************************************************
//! Fills the requested tile by pulling pixels from multiple file tiles as needed.
//*************************************************************************************************
rspfRefPtr<rspfImageData> rspfTiledImageHandler::getTile(const rspfIrect& tile_rect, 
                                                             rspf_uint32 resLevel)
{
   // First verify that there are file-tiles available:
   if (m_tileFiles.size() == 0)
      return rspfRefPtr<rspfImageData>();

   // Allocate data tile if needed:
   if(!m_tile.valid())
   {
      allocate();
      if(!m_tile.valid())
         return rspfRefPtr<rspfImageData>();
   }

   // Check if res level represents data inside the overall scene overview:
   if (theOverview.valid() && (resLevel >= m_startOvrResLevel))
      return theOverview->getTile(tile_rect, resLevel);

   // Loop over all tile-files to find which intersect the requested image rect. This necessarily
   // needs to be done at full res coordinates, so need to adjust by res level requested:
   rspfDpt decimation_factor;
   const rspf_uint32 BANDS = m_tile->getNumberOfBands();
   // const rspf_uint32 PPB   = m_tile->getSizePerBand(); // pixels per band
   // bool none_found = true;

   m_tile->setImageRectangle(tile_rect);
   rspf_uint32 wd, hd, ws, hs;
   m_tile->getWidthHeight(wd, hd);

   // Always start with blank tile.
   m_tile->makeBlank();
   
   // See if any point of the requested tile is in the image.
   if ( tile_rect.intersects( m_fullImgRect ) )
   {
      vector<rspfTileFile>::iterator tf_iter = m_tileFiles.begin();
      rspfRefPtr<rspfImageData> source_tile = 0;
      while (tf_iter != m_tileFiles.end())
      {
         if (( (*tf_iter).subImageRects.size() > resLevel) &&
             tile_rect.intersects((*tf_iter).subImageRects[resLevel]))
         {
            // Current image handler lies within requested rect, need to adjust this rect to be 
            // relative to this subimage offset before fetching the tile:
            rspfIrect relative_rect (tile_rect - (*tf_iter).subImageRects[resLevel].ul());
            source_tile = (*tf_iter).imageHandler->getTile(relative_rect, resLevel);
            
            // Quick check to see if a full tile was returned, in which case we can just return that
            // tile instead of looping below:
            //if (source_tile->getDataObjectStatus() == RSPF_FULL)
            //{
            //   source_tile->setImageRectangle(tile_rect);
            //   return source_tile;
            //}

            // Set the tile's rect back to full image space before saving to the list:
            source_tile->getWidthHeight(ws, hs);
            for (rspf_uint32 band = 0; band < BANDS; ++band)
            {
               const rspf_uint16 null_pixel = (rspf_uint16) m_tile->getNullPix(band);
               const rspf_uint16* s = (const rspf_uint16*) source_tile->getBuf(band);
               rspf_uint16* d = (rspf_uint16*) m_tile->getBuf(band);
               rspf_uint32 is = 0; 
               rspf_uint32 id = 0; 
               for (rspf_uint32 y=0; (y<hd)&&(y<hs); y++)
               {
                  for (rspf_uint32 x=0; x<wd; x++)
                  {
                     if (x < ws)
                     {
                        if (s[is] != null_pixel)
                           d[id] = s[is];
                        ++is;
                     }
                     ++id;
                  }
               }
            }
         }
         ++tf_iter;
      }
      
      m_tile->validate();
      
   } // Matches: if ( tile_rect.intersects( m_fullImgRect ) )

   return m_tile;
}

//*************************************************************************************************
//! @param resLevel Reduced resolution level to return lines of.
//! Default = 0
//! @return The number of lines for specified reduced resolution level.
//*************************************************************************************************
rspf_uint32 rspfTiledImageHandler::getNumberOfLines(rspf_uint32 resLevel) const
{
   if (m_fullImgRect.hasNans())
      return 0;

   // Using simple decimation by powers of 2:
   rspf_uint32 numlines = m_fullImgRect.height() >> resLevel;
   return numlines;
}

//*************************************************************************************************
//! @param resLevel Reduced resolution level to return samples of.
//! Default = 0
//! @return The number of samples for specified reduced resolution level.
//*************************************************************************************************
rspf_uint32 rspfTiledImageHandler::getNumberOfSamples(rspf_uint32 resLevel) const
{
   if (m_fullImgRect.hasNans())
      return 0;

   // Using simple decimation by powers of 2:
   rspf_uint32 numsamps = m_fullImgRect.width() >> resLevel;
   return numsamps;
}

//*************************************************************************************************
//! Method to save the state of an object to a keyword list.
//! Return true if ok or false on error.
//*************************************************************************************************
bool rspfTiledImageHandler::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
  return rspfImageHandler::saveState(kwl, prefix);
}

//*************************************************************************************************
//! Method to the load (recreate) the state of an object from a keyword
//! list.  Return true if ok or false on error.
//*************************************************************************************************
bool rspfTiledImageHandler::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if (!rspfImageHandler::loadState(kwl, prefix))
      return false;

   return open();
}

//*************************************************************************************************
//! Indicates whether or not the image is tiled internally.
//! This implementation returns true if (getImageTileWidth() &&
//! getImageTileHeight()) are both non zero.
//! Override in the image handler if something else is desired.
//! Returns true if tiled, false if not.
//*************************************************************************************************
bool rspfTiledImageHandler::isImageTiled() const
{
   return true;
}

//*************************************************************************************************
// Returns the number of bands of the first tile since all tiles need to have the same pixel type.
//*************************************************************************************************
rspf_uint32 rspfTiledImageHandler::getNumberOfInputBands() const
{
   if ((m_tileFiles.size() == 0) || (!m_tileFiles[0].imageHandler.valid()))
      return 0;

   return m_tileFiles[0].imageHandler->getNumberOfInputBands();
}

//*************************************************************************************************
//! Returns the tile width of the image or 0 if the image is not tiled.
//! Note: this is not the same as the rspfImageSource::getTileWidth which
//! returns the output tile width which can be different than the internal
//! image tile width on disk.
//*************************************************************************************************
rspf_uint32 rspfTiledImageHandler::getImageTileWidth() const
{
   if ((m_tileFiles.size() == 0) || (!m_tileFiles[0].imageHandler.valid()))
      return 0;

  if (m_tileFiles[0].imageHandler->isImageTiled())
     return m_tileFiles[0].imageHandler->getImageTileWidth();

  return m_tileFiles[0].imageHandler->getNumberOfSamples();
}

//*************************************************************************************************
//! Returns the tile height of the image or 0 if the image is not tiled.
//! Note: this is not the same as the rspfImageSource::getTileHeight which
//! returns the output tile height which can be different than the internal
//! image tile height on disk.
//*************************************************************************************************
rspf_uint32 rspfTiledImageHandler::getImageTileHeight() const
{
   if ((m_tileFiles.size() == 0) || (!m_tileFiles[0].imageHandler.valid()))
      return 0;

   if (m_tileFiles[0].imageHandler->isImageTiled())
      return m_tileFiles[0].imageHandler->getImageTileHeight();

   return m_tileFiles[0].imageHandler->getNumberOfLines();
}

//*************************************************************************************************
//! Returns scalar type of first tile (should be the same for all tiles)
//*************************************************************************************************
rspfScalarType rspfTiledImageHandler::getOutputScalarType() const
{
   if ((m_tileFiles.size() == 0) || (!m_tileFiles[0].imageHandler.valid()))
      return RSPF_SCALAR_UNKNOWN;

   return m_tileFiles[0].imageHandler->getOutputScalarType();
}

//*************************************************************************************************
//! Returns overall bounding rect in image space.
//*************************************************************************************************
rspfIrect rspfTiledImageHandler::getImageRectangle(rspf_uint32 resLevel) const
{
   if (resLevel == 0)
      return m_fullImgRect;

   rspfDpt decimation;
   getDecimationFactor(resLevel, decimation);

   return m_fullImgRect*decimation;
}


#if USING_SUB_OVRS
//*************************************************************************************************
// Overrides base class
//*************************************************************************************************
void rspfTiledImageHandler::completeOpen()
{
   openOverview();
   openValidVertices();
}

//*************************************************************************************************
//! Will build over files, one for each tile file.
//! @param includeFullResFlag if true the full resolution layer will also
//! be put in the overview format.  This is handy for inefficient formats.
//*************************************************************************************************
bool rspfTiledImageHandler::buildOverview(rspfImageHandlerOverviewCompressionType ctype, 
                                            rspf_uint32 qual,
                                            rspfFilterResampler::rspfFilterResamplerType rtype,
                                            bool ifr_flag)
{
   if (m_tileFiles.size() == 0)
      return false;

   bool all_ok = true;
   vector<rspfTileFile>::iterator iter = m_tileFiles.begin();
   while ((iter != m_tileFiles.end()) && all_ok)
   {
      if (!(*iter).imageHandler->hasOverviews())
      {
         rspfNotify(rspfNotifyLevel_INFO)<<"\nBuilding overview for <"
            <<(*iter).imageHandler->getFilename()<<">"<<std::endl;

         all_ok = (*iter).imageHandler->buildOverview(ctype, qual, rtype, ifr_flag);
      }
      ++iter;
   }

   // Overviews for subimage tile-files have been processed. However, we need to check the
   // preferences for the overview stop dimension to see if we need additional decimation of the
   // overall scene to achieve the final desired dimension.
   rspfString stop_dim_str = 
      rspfPreferences::instance()->findPreference(rspfKeywordNames::OVERVIEW_STOP_DIMENSION_KW);
   if (!stop_dim_str.empty())
   {
      rspf_uint32 stop_dim = stop_dim_str.toUInt32();
      rspfDpt decimation = theDecimationFactors.back();
      rspf_uint32 nlines = decimation.y * m_fullImgRect.height();
      rspf_uint32 nsamps = decimation.x * m_fullImgRect.width();
      if ((nlines > stop_dim) || (nsamps > stop_dim))
      {
         // Need to continue decimating. These decimations are stored in an overall scene file.
         // Need subimage OVRs open for this next operation:
         openOverview();

         rspfTiffOverviewBuilder tob;
         tob.setInputSource(this);
         theOverviewFile = getFilenameWithThisExtension("ovr");

         // This magically takes over where we left off in decimating:
         if (tob.buildOverview(theOverviewFile))
         {
            // Overview was successfully written, now open it:
            theOverview = new rspfTiffTileSource;
            all_ok = openOverview();
         }
      }
   }

   return all_ok;
}

//*************************************************************************************************
// Will open overview files for all tile files. Actually, the tile-files will already have opened
// their individual overviews, so this method only verifies the fact and initializes the 
// associated subimage rects at all decimation levels.
//*************************************************************************************************
bool rspfTiledImageHandler::openOverview()
{
   if (m_tileFiles.size() == 0)
      return false;

   theDecimationFactors.clear();
   bool all_ok = true;
   rspfRefPtr<rspfImageHandler> handler = 0;
   vector<rspfDpt> factors;
   rspf_uint32 min_num_rlevels = 999;

   // Loop over all tile-files to verify their OVR was opened and to calculate the associated 
   // subimage rects at each res level:
   vector<rspfTileFile>::iterator iter = m_tileFiles.begin();
   while ((iter != m_tileFiles.end()) && all_ok)
   {
      handler = (*iter).imageHandler;
      if (!handler.valid())
      {
         ++iter;
         continue;
      }

      if ((*iter).subImageRects.size() == 0)
      {
         rspfNotify(rspfNotifyLevel_INFO)<<"\nBounding image rectangle not defined for <"
            <<(*iter).imageHandler->getFilename()<<">! Cannot proceed with overviews."<<std::endl;
         all_ok = false;
      }

      else if (handler->getOverview())
      {
         // Only verified that overview for tile-file was opened. Check number of decimation levels
         // to latch minimum:
         (*iter).overviewIsOpen = true;
         handler->getDecimationFactors(factors);

         if (factors.size() < min_num_rlevels)
            min_num_rlevels = (rspf_uint32) factors.size();

         // Need to determine the sub image rects at each decimation level:
         rspfIrect subRectR0 ((*iter).subImageRects[0]);
         for (rspf_uint32 i=1; i<(rspf_uint32)factors.size(); i++)
         {
            rspfIrect r (factors[i].x * subRectR0.ul().x, factors[i].y * subRectR0.ul().y,
               factors[i].x * subRectR0.lr().x, factors[i].y * subRectR0.lr().y);
            (*iter).subImageRects.push_back(r);
         }
      }
      else
      {
         rspfNotify(rspfNotifyLevel_INFO)<<"\nNo overview available for <"
            <<(*iter).imageHandler->getFilename()<<">"<<std::endl;
         all_ok = false;
      }
      ++iter;
   }

   // Assuming here that the first tile will always provide a good representation of the decimations
   // for all res levels in all tile-files. This may not be true.
   if (m_tileFiles.size() && m_tileFiles[0].imageHandler.valid())
   {
      m_tileFiles[0].imageHandler->getDecimationFactors(theDecimationFactors);
      theDecimationFactors.resize(min_num_rlevels);
   }

   // Check for overall scene overview file, that takes over where the individual tile-file
   // overviews leave off:
   theOverviewFile = getFilenameWithThisExtension("ovr");
   if (!theOverviewFile.exists())
      theOverviewFile.setExtension("OVR");
   if (theOverviewFile.exists())
   {
      m_lockSubOvrs = true; // prevent closing and reopening of subimage ovrs
      rspfImageHandler::openOverview(theOverviewFile);
      m_lockSubOvrs = false; // reset
      if (theOverview.valid())
      {
         vector<rspfDpt> extra_decimations;
         theOverview->getDecimationFactors(extra_decimations);
         if (extra_decimations.size())
         {
            // Obtain the decimations and add it to our decimation list:
            m_startOvrResLevel = (rspf_uint32) theDecimationFactors.size();
            rspfDpt start_decimation (theDecimationFactors.back() * 0.5);
            rspfDpt decimation;
            for (rspf_uint32 i=0; i<extra_decimations.size(); i++)
            {
               decimation.x = start_decimation.x*extra_decimations[i].x;
               decimation.y = start_decimation.y*extra_decimations[i].y;
               theDecimationFactors.push_back(decimation);
            }
         }
      }
   }

   return all_ok;
}

//*************************************************************************************************
//! Will close all tile files' overviews.
//*************************************************************************************************
void rspfTiledImageHandler::closeOverview()
{
   if (theOverview.valid())
      theOverview = 0;

   if (m_lockSubOvrs == false)
   {
      vector<rspfTileFile>::iterator iter = m_tileFiles.begin();
      while ((iter != m_tileFiles.end()))
      {
         if ((*iter).imageHandler.valid())
            (*iter).imageHandler->closeOverview();

         ++iter;
      }
   }
}

//*************************************************************************************************
//! Will return TRUE if all tile files have overviews.
//*************************************************************************************************
bool rspfTiledImageHandler::hasOverviews() const
{
   bool all_have_ovrs = true;
   vector<rspfTileFile>::const_iterator iter = m_tileFiles.begin();
   while ((iter != m_tileFiles.end()) && all_have_ovrs)
   {
      if (((*iter).imageHandler.valid()) && !((*iter).imageHandler->hasOverviews()))
         all_have_ovrs = false;
      ++iter;
   }
   return all_have_ovrs;
}

//*************************************************************************************************
//! Returns the number of decimation (resolution) levels. This is the minimum number
//! among all sub-image tile-files.
//*************************************************************************************************
rspf_uint32 rspfTiledImageHandler::getNumberOfDecimationLevels() const
{
   return (rspf_uint32) theDecimationFactors.size();
}

#endif /* #if USING_SUB_OVRS */

