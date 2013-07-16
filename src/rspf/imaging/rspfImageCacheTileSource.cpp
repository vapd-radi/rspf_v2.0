//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Mingjie Su   
//
//********************************************************************
// $Id: rspfImageCacheTileSource.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $
#include <algorithm>
using namespace std;

#include <rspf/imaging/rspfImageCacheTileSource.h>

#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfNBandLutDataObject.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/support_data/rspfRpfFrame.h>
#include <rspf/imaging/rspfImageDataFactory.h>

#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfPolygon.h>

static rspfTrace traceDebug = rspfTrace("rspfImageCacheTileSource:debug");

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfImageCacheTileSource.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $";
#endif

RTTI_DEF1(rspfImageCacheTileSource, "rspfImageCacheTileSource", rspfImageCacheBase)

rspfImageCacheTileSource::rspfImageCacheTileSource()
   :
   rspfImageCacheBase(),
   m_tile(0),
   m_imageHandler(0),
   m_minPixelValue(rspf::nan()),
   m_maxPixelValue(rspf::nan()),
   m_nullPixelValue(rspf::nan())
{
}

rspfImageCacheTileSource::~rspfImageCacheTileSource()
{
   if(m_imageHandler.valid())
   {
      m_imageHandler = 0;
   }
  close();
}

rspfString rspfImageCacheTileSource::getShortName()const
{
  return rspfString("ImageCache");
}

rspfString rspfImageCacheTileSource::getLongName()const
{
  return rspfString("ImageCache reader");
}


void rspfImageCacheTileSource::close()
{
  deleteAll();
}

bool rspfImageCacheTileSource::open()
{
  if(traceDebug())
  {
    rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageCacheTileSource::open(): Entered....." << std::endl;
  }

  rspfFilename imageFile = theImageFile;
  bool result = true;

  if(isOpen() == false)
  {
    close();
    result = false;
  }

  if(result)
  {
    if (m_fileNames.size() == 0 && m_frameEntryArray.size() == 0)
    {
      if (buildFrameEntryArray(imageFile) == false)
      {
        return false;
      }
    }

    //---
    // Adjust image rect so not to go over the -180 to 180 and -90 to 90
    // bounds.
    //---
    setActualImageRect();

    // Set the base class image file name.
    theImageFile = imageFile;
    m_tile = rspfImageDataFactory::instance()->create(this, this);
    m_tile->initialize();
    completeOpen();
  }

  if(traceDebug())
  {
    rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageCacheTileSource::open(): Leaving at line" << __LINE__ << std::endl;
  }

  return result;
}

bool rspfImageCacheTileSource::buildFrameEntryArray(rspfFilename imageFile)
{
   static const char MODULE[] = "rspfImageCacheTileSource::buildFrameEntryArray";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered..." << endl;
   }

   std::ifstream in((imageFile).c_str() );
   std::string line;
   rspf_uint32 index = 0; // used throughout
   rspf_uint32 frameWidth = 0;
   rspf_uint32 frameHeight = 0;
   while( in.good() )
   {
      // Read in a line.
      std::getline(in, line);
      rspfString tmpStr = rspfString(line);
      if (index == 0)
      {
         std::vector<rspfString> box_lat_lon;
         tmpStr.split(box_lat_lon, "|");
         if (box_lat_lon.size() == 4)
         {
            std::vector<rspfString> pixelInfos = box_lat_lon[3].split(",");
            if (pixelInfos.size() == 3)
            {
               m_minPixelValue = rspfString::toFloat64(pixelInfos[0]);
               m_maxPixelValue = rspfString::toFloat64(pixelInfos[1]);
               m_nullPixelValue = rspfString::toFloat64(pixelInfos[2]);
            }
         }
      }
      else if (index == 1)
      {
         std::vector<rspfString> frame_lat_lon = tmpStr.split("|");
         rspfFilename firstFile = frame_lat_lon[0];
         if(m_workFrame->parseFile(firstFile) == rspfErrorCodes::RSPF_ERROR)
         {
            m_imageHandler = rspfImageHandlerRegistry::instance()->open(firstFile);
            if (m_imageHandler.valid())
            {
               frameWidth = m_imageHandler->getBoundingRect().width();
               frameHeight = m_imageHandler->getBoundingRect().height();
               break;
            }
         }
         else
         {
            return false;
         }
      }
      ++index;
   }
   in.close();

   return rspfImageCacheBase::buildFrameEntryArray(imageFile, frameWidth, frameHeight);
}

rspfRefPtr<rspfImageData> rspfImageCacheTileSource::getTile(
  const  rspfIrect& rect, rspf_uint32 resLevel)
{
  if (m_tile.valid())
  {
    // Image rectangle must be set prior to calling getTile.
    m_tile->setImageRectangle(rect);

    if ( getTile( m_tile.get(), resLevel ) == false )
    {
      if (m_tile->getDataObjectStatus() != RSPF_NULL)
      {
        m_tile->makeBlank();
      }
    }
  }

  return m_tile;
}

bool rspfImageCacheTileSource::getTile(rspfImageData* result,
                                      rspf_uint32 resLevel)
{
   bool status = false;
   
   //---
   // Not open, this tile source bypassed, or invalid res level,
   // return a blank tile.
   //---
   if( isOpen() && isSourceEnabled() && isValidRLevel(resLevel) &&
       result && (result->getNumberOfBands() == getNumberOfOutputBands()))
   {
      if ( resLevel > 0 )
      {
         status = getOverviewTile(resLevel, result);
      }
      
      if (!status) // Did not get an overview tile.
      {
         status = true;
         
         rspfIrect rect = result->getImageRectangle();
         
         rspfIrect imageRect = getImageRectangle();
         
         if ( rect.intersects(imageRect) )
         {
            //---
            // Start with a blank tile in case there is not total coverage
            // for rect.
            //---
            result->makeBlank();
            
            vector<rspfFrameEntryData> frames = getIntersectingEntries(rect);
            if(frames.size() > 0)
            {
               //---
               // Now lets render each frame.  Note we will have to find
               // subframes
               // that intersect the rectangle of interest for each frame.
               //---
               fillTile(rect, frames, result);
               
               // Revalidate tile status.
               result->validate();
            }
         }
         else
         {
            result->makeBlank();
         }
      }
   }
   return status;
}

void rspfImageCacheTileSource::fillTile(
   const rspfIrect& tileRect,
   const vector<rspfFrameEntryData>& framesInvolved,
   rspfImageData* tile)
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx < framesInvolved.size(); ++idx)
   {
      if (m_imageHandler.valid())
      {
         if (m_imageHandler->getFilename() != framesInvolved[idx].theFrameEntry.getFullPath())
         {
            m_imageHandler = 0;
            m_imageHandler = rspfImageHandlerRegistry::instance()->open(framesInvolved[idx].theFrameEntry.getFullPath());
         }
      }
      else
      {
         m_imageHandler = rspfImageHandlerRegistry::instance()->open(framesInvolved[idx].theFrameEntry.getFullPath());
      }

      if (m_imageHandler.valid())
      {
         if (m_imageHandler->isOpen() == false)
         {
            m_imageHandler->open(framesInvolved[idx].theFrameEntry.getFullPath());
         }

         rspfIrect frameRect(framesInvolved[idx].thePixelCol,
            framesInvolved[idx].thePixelRow,
            framesInvolved[idx].thePixelCol + m_frame_width  - 1,
            framesInvolved[idx].thePixelRow + m_frame_height - 1);

         rspfIrect clipRect = tileRect.clipToRect(frameRect);

         rspfIpt tempDelta(clipRect.ul().x - framesInvolved[idx].thePixelCol,
            clipRect.ul().y - framesInvolved[idx].thePixelRow);

         rspfIrect offsetRect(tempDelta.x,
            tempDelta.y,
            tempDelta.x + clipRect.width()-1,
            tempDelta.y + clipRect.height()-1);

         //if the current image size is different from the default size, resample the image data
         if ( ( m_imageHandler->getBoundingRect().width() != m_frame_width ) ||
              ( m_imageHandler->getBoundingRect().height() != m_frame_height ) )
         {

            //calculate the ratio
            rspf_float64 widthRatio = static_cast<rspf_float64>(m_imageHandler->getBoundingRect().width())/static_cast<rspf_float64>(m_frame_width);
            rspf_float64 heightRatio = static_cast<rspf_float64>(m_imageHandler->getBoundingRect().height())/static_cast<rspf_float64>(m_frame_height);

            //initialize the rect bounding for getting image data from image handler
            rspfDpt ul(offsetRect.ul().x*widthRatio, offsetRect.ul().y*heightRatio);
            rspfDpt lr(offsetRect.lr().x*widthRatio, offsetRect.lr().y*heightRatio);

            //---
            // Floor, ceil to avoid rect size of zero from ul, lr, delta being only fractional
            // resulting in size of 0 if cast to an int or passed to rspf<int>::round().
            //---
            rspfIrect actualOffsetRect(static_cast<rspf_int32>(std::floor(ul.x)),
                                        static_cast<rspf_int32>(std::floor(ul.y)),
                                        static_cast<rspf_int32>(std::ceil(lr.x)),
                                        static_cast<rspf_int32>(std::ceil(lr.y)));

            //get accurate rect in case the actual rect is larger than the rect of image 
            actualOffsetRect = actualOffsetRect.clipToRect(m_imageHandler->getBoundingRect());
            
            rspfRefPtr<rspfImageData> imageData = m_imageHandler->getTile(actualOffsetRect);
            if (imageData.valid())
            {
               rspfFilterResampler* resampler = new rspfFilterResampler();
               
               double denominatorY = 1.0;
               double denominatorX = 1.0;
               if(clipRect.height() > 2) // Cannot be zero.
               {
                  denominatorY = clipRect.height()-1.0;
               }
               if(clipRect.width() > 2) // Cannot be zero.
               {
                  denominatorX = clipRect.width()-1.0;
               }

               //create a tile and set the rect size
               rspfRefPtr<rspfImageData> tmpTile = (rspfImageData*)imageData->dup();
               tmpTile->setImageRectangle(offsetRect);
               tmpTile->makeBlank();

               rspfDpt deltaUl(((actualOffsetRect.ll().x - actualOffsetRect.ul().x)/denominatorX),
                                ((actualOffsetRect.ll().y - actualOffsetRect.ul().y)/denominatorY));
               rspfDpt deltaUr(((actualOffsetRect.lr().x - actualOffsetRect.ur().x)/denominatorX),
                                ((actualOffsetRect.lr().y - actualOffsetRect.ur().y)/denominatorY));
               rspfDpt length(offsetRect.width(),offsetRect.height());
               
               //resample the image data
               resampler->resample(imageData, 
                                   tmpTile,
                                   actualOffsetRect.ul(),
                                   actualOffsetRect.ur(),
                                   deltaUl,
                                   deltaUr,
                                   length);

               tile->loadTile(tmpTile->getBuf(), clipRect, RSPF_BSQ);

               tmpTile = 0;
               delete resampler;
               resampler = 0;
            }
         }
         else
         {
            rspfRefPtr<rspfImageData> imageData = m_imageHandler->getTile(offsetRect);
            if (imageData.valid())
            {
               tile->loadTile(imageData->getBuf(), clipRect, RSPF_BSQ);
            }
         }
      }
   }
}

rspfScalarType rspfImageCacheTileSource::getOutputScalarType() const
{
   if (m_imageHandler.valid())
   {
      return m_imageHandler->getOutputScalarType();
   }
   return RSPF_UCHAR;
}

double rspfImageCacheTileSource::getMinPixelValue(rspf_uint32 band)const
{
   if (rspf::isnan(m_minPixelValue))
   {
      return rspfImageHandler::getMinPixelValue(band);
   }
   return m_minPixelValue;
}

double rspfImageCacheTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   if (rspf::isnan(m_maxPixelValue))
   {
      return rspfImageHandler::getMaxPixelValue(band);
   }
   return m_maxPixelValue;
}

double rspfImageCacheTileSource::getNullPixelValue(rspf_uint32 band)const
{
   if (rspf::isnan(m_nullPixelValue))
   {
      return rspfImageHandler::getNullPixelValue(band);
   }
   return m_nullPixelValue;
}

rspf_uint32 rspfImageCacheTileSource::getNumberOfInputBands()const
{
   if (m_imageHandler.valid())
   {
      return m_imageHandler->getNumberOfInputBands();
   }
   return m_numberOfBands;
}

rspf_uint32 rspfImageCacheTileSource::getNumberOfOutputBands()const
{
   if (m_imageHandler.valid())
   {
      return m_imageHandler->getNumberOfInputBands();
   }
   return m_numberOfBands;
}
