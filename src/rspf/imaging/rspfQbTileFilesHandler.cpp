//*************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
// Description: Image handler used for tiled Quickbird imagery. 
//
//*************************************************************************************************
//  $Id: rspfQbTileFilesHandler.cpp 2814 2011-07-05 13:40:16Z oscar.kramer $

#include <rspf/imaging/rspfQbTileFilesHandler.h>
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
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfTiffOverviewBuilder.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/support_data/rspfQuickbirdTile.h>
#include <rspf/projection/rspfQuickbirdRpcModel.h>
#include <algorithm>

RTTI_DEF1(rspfQbTileFilesHandler, "rspfQbTileFilesHandler", rspfTiledImageHandler)

// Static trace for debugging
static rspfTrace traceDebug("rspfQbTileFilesHandler:debug");

//*************************************************************************************************
//!  Constructor (default):
//*************************************************************************************************
rspfQbTileFilesHandler::rspfQbTileFilesHandler()
{
}

//*************************************************************************************************
//! Destructor:
//*************************************************************************************************
rspfQbTileFilesHandler::~rspfQbTileFilesHandler()
{
}

//*************************************************************************************************
//! Opens the image file given entry index. This class does not support multiple entries.
//! @param imageFile File to open.
//! @param entryIndex
//! @return false if entry index other than 0 specified.
//*************************************************************************************************
bool rspfQbTileFilesHandler::open()
{
   static const char* MODULE = "rspfQbTileFilesHandler::open() -- ";
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_INFO)
         <<MODULE << "Entering...\nimage: " << theImageFile << "\n";
   }
   
   // Test for extension: image.til
   rspfString ext = theImageFile.ext().downcase();
   if ( ext == "ovr" )
   {
      return false; // Don't try to open overviews.
   }
   
   rspfRegExp regExp("^[t|T][i|I][l|L]");
   if ( regExp.find( ext.c_str() ) )
   {
      if (!theImageFile.exists())
      {
         return false;
      }
   }
      
   // Use rspfQuickbirdTile object for parsing the TIL file and fetching the data structure 
   // containing tile-file info:
   if (traceDebug())
      rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"OPENING rspfQuickbirdTile..."<<std::endl;
   
   rspfQuickbirdTile qbt;
   bool success = qbt.open(theImageFile);
   if (!success)
      return false;
   
   // Populate our own data structure given the QBT map:
   rspfFilename image_dir (theImageFile.path());
   const rspfQuickbirdTile::TileMap& qbtTileMap = qbt.getMap();

   m_fullImgRect.makeNan();
   rspfIrect subImageRect;
   rspfQuickbirdTile::TileMap::const_iterator qbt_iter = qbtTileMap.begin();
   rspfImageHandlerRegistry* factory = rspfImageHandlerRegistry::instance();
   while (qbt_iter != qbtTileMap.end())
   {
      rspfQuickbirdTileInfo info ((*qbt_iter).second);
      rspfTileFile tilefile;
      tilefile.tileFilename = (*qbt_iter).second.theFilename;
      tilefile.tileFilename.setPath(image_dir);
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"Using factory to open <"
            <<tilefile.tileFilename<<">"<<std::endl;
      }

      // Now open up the handlers for this tile-file:
      tilefile.imageHandler = factory->open(tilefile.tileFilename);
      if (tilefile.imageHandler.valid())
      {
         // Valid handler, populate remaining items of interest and push it on our list...
         // Fetch image coordinates in full-view:
         subImageRect.set_ulx((*qbt_iter).second.theUlXOffset);
         subImageRect.set_uly((*qbt_iter).second.theUlYOffset);
         subImageRect.set_lrx((*qbt_iter).second.theLrXOffset);
         subImageRect.set_lry((*qbt_iter).second.theLrYOffset);
         tilefile.subImageRects.push_back(subImageRect);

         // Update bounding image rectangle:
         m_fullImgRect = m_fullImgRect.combine(subImageRect);
         m_tileFiles.push_back(tilefile);
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"Pushed <"<<tilefile.tileFilename<<"> on"
               "to tile-files list."<<std::endl;
         }
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)<<MODULE<<"TIL file listed <"
            <<tilefile.tileFilename<<"> as one of the tiles, but the file could not be opened. "
            <<"This is being ignored but may cause a problem downstream."<<std::endl;
      }
   
      ++qbt_iter;   
   }

   // Need to check the full-image rect for validity. May need to explore the .IMD file if this 
   // was not properly initialized:
   if (m_fullImgRect.hasNans())
   {
      if (!computeImageRects())
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"Could not establish image rectangles."
               <<std::endl;
         }
         return false;
      }
   }

   if (traceDebug())
      rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"Finished loading tiles."<<std::endl;

   bool open_good = false;
   if ((m_tileFiles.size() != 0) && m_tileFiles[0].imageHandler.valid())
   {
      completeOpen();
      open_good = true;
      
      if (traceDebug())
         rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"Open successful."<<std::endl;
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_INFO)<<MODULE<<"Returning with <"
         <<rspfString::toString(open_good)<<">"<<std::endl;
   }
   return open_good;
}

//*************************************************************************************************
//! Fetch the tile-files common RPC data file and create the projection for this image
//*************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfQbTileFilesHandler::getImageGeometry()
{
   // Try external geom first:
   theGeometry = getExternalImageGeometry();
   if (theGeometry.valid())
      return theGeometry;  // We should return here.
   
   // The dataset is expected to have an RPC model associated with it:
   rspfRefPtr<rspfQuickbirdRpcModel> model = new rspfQuickbirdRpcModel(this);
   if (!model->getErrorStatus())
   {
      theGeometry = new rspfImageGeometry;
      theGeometry->setProjection(model.get());
      
      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );

      return theGeometry;
   }
   return rspfRefPtr<rspfImageGeometry>();
}

//*************************************************************************************************
//! Initializes tile image rects by considering adjacent row/col offsets. Called when TIL
//! doesn't contain the info. Returns true if successful.
//*************************************************************************************************
bool rspfQbTileFilesHandler::computeImageRects()
{
   try
   {
      rspfQuickbirdMetaData oqmd;
      if (!oqmd.open(theImageFile))
         throw 1;

      rspfIpt image_size (oqmd.getImageSize());
      if (image_size.hasNans())
         throw 2;

      rspfIpt full_image_lr (image_size.x-1, image_size.y-1);
      m_fullImgRect.set_ulx(0);
      m_fullImgRect.set_uly(0);
      m_fullImgRect.set_lr(full_image_lr);

      // Now consider the subrects for each tile. It is assumed that all tiles (except right
      // and bottom) will be of uniform size. The TIL file should have at least provided the row
      // and column offsets for each tile into the full image. This is equivalent to the UL corner.
      // It is presumed that the TIL failed to provide the LR corner since this method was called.
      int dx=0, dy=0, i=0;
      int num_tiles = (int) m_tileFiles.size();
      while (((dx == 0) || (dy == 0)) && (i < num_tiles-1))
      {
         if (dx == 0)
            dx = m_tileFiles[i+1].subImageRects[0].ul().x - m_tileFiles[i].subImageRects[0].ul().x;
         if (dy == 0)
            dy = m_tileFiles[i+1].subImageRects[0].ul().y - m_tileFiles[i].subImageRects[0].ul().y;
         ++i;
      }

      // We should have the nominal tile size now. Check this is so:
      if ((dx == 0) || (dy == 0))
      {
         // Prior scheme failed for establishing the tile dimensions. Final resort is to open the first
         // available tile file and query its dimensions:
         if (num_tiles == 0)
            throw 3;
         if (m_tileFiles[0].imageHandler.valid())
         {
            // we have an open handler, so just query it:
            dy = m_tileFiles[0].imageHandler->getNumberOfLines();
            dx = m_tileFiles[0].imageHandler->getNumberOfSamples();
         }
         else
         {
            // Struck out, just assume the tile is square:
            if ((dx == 0) && (dy == 0))
               throw 4;
            if (dy == 0)
               dy = dx;
            else
               dx = dy;
         }
      }

      // Loop over all tiles to properly assign their image rectangles:
      for (i=0; i<num_tiles; i++)
      {
         rspfIpt ul (m_tileFiles[i].subImageRects[0].ul());
         rspfIpt lr (ul.x + dx - 1, ul.y + dy - 1);

         // Check to make sure we don't exceed extents of full image:
         if (lr.x > full_image_lr.x)
            lr.x = full_image_lr.x;
         if (lr.y > full_image_lr.y)
            lr.y = full_image_lr.y;

         m_tileFiles[i].subImageRects[0].set_lr(lr);
      }
   }
   
   catch (...)
   {
      // Could not establish the rectangles:
      return false;
   }

   return true;
}

