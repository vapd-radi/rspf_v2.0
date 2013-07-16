//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//********************************************************************
// $Id: rspfRpfCacheTileSource.cpp 1361 2010-08-26 00:24:22Z david.burken $
#include <algorithm>
using namespace std;

#include <rspf/imaging/rspfRpfCacheTileSource.h>

#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfNBandLutDataObject.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
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
#include <rspf/support_data/rspfRpfToc.h>
#include <rspf/support_data/rspfRpfTocEntry.h>
#include <rspf/support_data/rspfRpfCompressionSection.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfCylEquAreaProjection.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfPolygon.h>

static rspfTrace traceDebug = rspfTrace("rspfRpfCacheTileSource:debug");

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfRpfCacheTileSource.cpp 1361 2010-08-26 00:24:22Z david.burken $";
#endif

RTTI_DEF1(rspfRpfCacheTileSource, "rspfRpfCacheTileSource", rspfImageHandler)

const rspf_uint32 rspfRpfCacheTileSource::CIBCADRG_FRAME_WIDTH  = 1536;
const rspf_uint32 rspfRpfCacheTileSource::CIBCADRG_FRAME_HEIGHT = 1536;

rspfRpfCacheTileSource::rspfRpfCacheTileSource()
   :
   rspfImageHandler(),

   // a CADRG and CIBis a 64*64*12 bit buffer and must divide by 8 to
   // convert to bytes   
   m_compressedBuffer(new rspf_uint8[(64*64*12)/8]),

   // whether CIB or CADRG we will alocate the buffer
   // to the larger of the 2 (CADRG is 3 bands) 
   m_uncompressedBuffer(new rspf_uint8[256*256*3]),
   
   m_numberOfLines(0),
   m_numberOfSamples(0),
   m_tile(0),
   m_fileNames(),
   m_tileSize(128, 128),
   m_productType(RSPF_PRODUCT_TYPE_UNKNOWN),
   m_workFrame(new rspfRpfFrame),
   m_bBox_LL_Lon(0.0),
   m_bBox_LL_Lat(0.0),
   m_bBox_UR_Lon(0.0),
   m_bBox_UR_Lat(0.0),
   m_numOfFramesVertical(0),
   m_numOfFramesHorizontal(0),
   m_frameEntryArray()
{
  if (traceDebug())
  {
    rspfNotify(rspfNotifyLevel_DEBUG)
      << "rspfRpfCacheTileSource::rspfRpfCacheTileSource entered...\n";
#ifdef RSPF_ID_ENABLED
    rspfNotify(rspfNotifyLevel_DEBUG)
      << "RSPF_ID:  " << RSPF_ID << "\n";
#endif      
  }

  m_actualImageRect.makeNan();

}

rspfRpfCacheTileSource::~rspfRpfCacheTileSource()
{
  if(m_compressedBuffer)
  {
    delete [] m_compressedBuffer;
    m_compressedBuffer = 0;
  }
  if(m_uncompressedBuffer)
  {
    delete [] m_uncompressedBuffer;
    m_uncompressedBuffer = 0;
  }
  if(m_workFrame)
  {
    delete m_workFrame;
    m_workFrame = 0;
  }
  close();
}

rspfString rspfRpfCacheTileSource::getShortName()const
{
  return rspfString("RpfCache");
}

rspfString rspfRpfCacheTileSource::getLongName()const
{
  return rspfString("RpfCache reader");
}


void rspfRpfCacheTileSource::close()
{
  deleteAll();
}

bool rspfRpfCacheTileSource::isOpen()const
{
  //return (theTableOfContents!=0);
  rspfString ext = theImageFile.ext().downcase();

  if(ext == "rpf")
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool rspfRpfCacheTileSource::open()
{
  if(traceDebug())
  {
    rspfNotify(rspfNotifyLevel_DEBUG) << "rspfRpfCacheTileSource::open(): Entered....." << std::endl;
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
    
    setCurrentEntry(0);
    //---
    // Adjust image rect so not to go over the -180 to 180 and -90 to 90
    // bounds.
    //---
    setActualImageRect();

    // Set the base class image file name.
    theImageFile = imageFile;
    m_tile = rspfImageDataFactory::instance()->create(this, this);
    m_tile->initialize();
  }

  if(traceDebug())
  {
    rspfNotify(rspfNotifyLevel_DEBUG) << "rspfRpfCacheTileSource::open(): Leaving at line" << __LINE__ << std::endl;
  }

  return result;
}

bool rspfRpfCacheTileSource::buildFrameEntryArray(rspfFilename imageFile)
{
   static const char MODULE[] = "rspfRpfCacheTileSource::buildFrameEntryArray";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered..." << endl;
   }

   std::ifstream in((imageFile).c_str() );
   
   std::string line;
   rspf_uint32 index = 0; // used throughout
   while( in.good() )
   {
      // Read in a line.
      std::getline(in, line);

      rspfString tmpStr = rspfString(line);
      if (!tmpStr.empty())
      {
         if (index == 0)
         {
            std::vector<rspfString> box_lat_lon;
            tmpStr.split(box_lat_lon, "|");
            
            if (box_lat_lon.size() > 2)
            {
               std::vector<rspfString> ll;
               std::vector<rspfString> ur;
               
               box_lat_lon[0].split(ll, ",");
               box_lat_lon[1].split(ur, ",");
               rspfString bandStr = box_lat_lon[2];
               
               m_bBox_LL_Lon = ll[0].toFloat64();
               m_bBox_LL_Lat = ll[1].toFloat64();
               m_bBox_UR_Lon = ur[0].toFloat64();
               m_bBox_UR_Lat = ur[1].toFloat64();
               
               checkLongitude(m_bBox_LL_Lon, m_bBox_UR_Lon);
               
               rspf_int32 bands = rspfString(bandStr).toInt32();
               if (bands == 1)
               {
                  m_productType = RSPF_PRODUCT_TYPE_CIB;
               }
               else if (bands == 3)
               {
                  m_productType = RSPF_PRODUCT_TYPE_CADRG;
               }
            }
            else
            {
               return false;
            }
         }
         else if (index == 1)
         {
            m_fileNames.push_back(tmpStr);
            std::vector<rspfString> frame_lat_lon = tmpStr.split("|");
            if (frame_lat_lon.size() > 2)
            {
               rspfString ll_lon_lat = frame_lat_lon[1];
               rspfString ur_lon_lat = frame_lat_lon[2];
               rspf_float64 ll_lon = rspfString(ll_lon_lat.split(",")[0]).toDouble();
               rspf_float64 ll_lat = rspfString(ll_lon_lat.split(",")[1]).toDouble();
               rspf_float64 ur_lon = rspfString(ur_lon_lat.split(",")[0]).toDouble();
               rspf_float64 ur_lat = rspfString(ur_lon_lat.split(",")[1]).toDouble();

               checkLongitude(ll_lon, ur_lon);
               
               rspf_float64 bBox_lat_diff = std::fabs(m_bBox_UR_Lat - m_bBox_LL_Lat);
               rspf_float64 bBox_lon_diff = std::fabs(m_bBox_UR_Lon - m_bBox_LL_Lon);
               
               rspf_float64 lat_diff = std::fabs(ur_lat - ll_lat);
               rspf_float64 lon_diff = std::fabs(ur_lon - ll_lon);
               
               m_numOfFramesVertical   = static_cast<rspf_uint32>(bBox_lat_diff/lat_diff + 0.5);
               m_numOfFramesHorizontal = static_cast<rspf_uint32>(bBox_lon_diff/lon_diff + 0.5);
               
               m_numberOfLines   = m_numOfFramesVertical*CIBCADRG_FRAME_HEIGHT;
               m_numberOfSamples = m_numOfFramesHorizontal*CIBCADRG_FRAME_WIDTH;

               if ( traceDebug() )
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << "\nscene height in degrees: " << bBox_lat_diff
                     << "\nscene width in degrees:  " << bBox_lon_diff
                     << "\nframe height in degrees: " << lat_diff
                     << "\nframe width in degrees:  " << lon_diff
                     << "\nframes vertical:         " << m_numOfFramesVertical
                     << "\nframes horizontal:       " << m_numOfFramesHorizontal
                     << "\nlines:                   " << m_numberOfLines
                     << "\nsamples:                 " << m_numberOfSamples << "\n";
               }
            }
            else
            {
               return false;
            }
         }
         else
         {
            m_fileNames.push_back(tmpStr);
         }
         
      } // matches: if (!tmpStr.empty())
      
      ++index;
      
   } // matches: while( in.good() )
   in.close();

   m_frameEntryArray.resize(m_numOfFramesVertical);
   for(index = 0; index < m_frameEntryArray.size(); ++index)
   {
      m_frameEntryArray[index].resize(m_numOfFramesHorizontal);
   }

   for(index = 0; index < m_fileNames.size(); ++index)
   {
      rspfString tmpStr = m_fileNames[index];
      std::vector<rspfString> frameInfos = tmpStr.split("|");
      if (frameInfos.size() > 1)
      {
         rspfString fileName = frameInfos[0];
         rspfString ll_lon_lat = frameInfos[1];
         rspfString ur_lon_lat = frameInfos[2];
         double ll_lon = rspfString(ll_lon_lat.split(",")[0]).toDouble();
         double ll_lat = rspfString(ll_lon_lat.split(",")[1]).toDouble();
         double ur_lon = rspfString(ur_lon_lat.split(",")[0]).toDouble();
         double ur_lat = rspfString(ur_lon_lat.split(",")[1]).toDouble();
         
         checkLongitude(ll_lon, ur_lon);
         
         rspf_float64 tmpColPostion = std::fabs(ll_lon - m_bBox_LL_Lon);
         rspf_float64 tmpFrameLon = std::fabs(ur_lon - ll_lon);
         
         rspf_float64 tmpRowPostion = std::fabs(m_bBox_UR_Lat - ur_lat);
         rspf_float64 tmpFrameLat = std::fabs(ur_lat - ll_lat);
         
         rspf_uint32 colNum = static_cast<rspf_uint32>(tmpColPostion/tmpFrameLon + 0.5);
         rspf_uint32 rowNum = static_cast<rspf_uint32>(tmpRowPostion/tmpFrameLat + 0.5);
         
         if (colNum >= m_numOfFramesHorizontal)
         {
            colNum = m_numOfFramesHorizontal-1; // Clamp to last column.
         }
         
         if (rowNum >= m_numOfFramesVertical)
         {
            rowNum = m_numOfFramesVertical-1; // Clamp to last row.
         }
         
         rspfRpfFrameEntry tempEntry;
         tempEntry.setEntry(rspfFilename(""), fileName);
         m_frameEntryArray[rowNum][colNum] = tempEntry;
      }
      else
      {
         return false;
      }
   }

   return true;
}

void rspfRpfCacheTileSource::checkLongitude(rspf_float64& leftLon,
                                             const rspf_float64& rightLon) const
{
   //---
   // Test for scene coordinates being 180 to 180 and adjust leftLon to -180
   // if so.
   //
   // NOTE:
   // Setting tolerance to 1/7200 about 15 meters.
   // Not sure if this is too loose or not. (drb)
   //---
   const rspf_float64 TOLERANCE = 0.000138889; // 1/7200 about 15 meters.

   if ( rspf::almostEqual(leftLon, 180.0, TOLERANCE) )
   {
      if ( rspf::almostEqual(rightLon, 180.0, TOLERANCE) )
      {
         leftLon = -180.0;
      }
   }
}

rspfRefPtr<rspfImageData> rspfRpfCacheTileSource::getTile(
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

bool rspfRpfCacheTileSource::getTile(rspfImageData* result,
                                      rspf_uint32 resLevel)
{
   bool status = false;
   
   //---
   // Not open, this tile source bypassed, or invalid res level,
   // return a blank tile.
   //---
   if( isOpen() && isSourceEnabled() && isValidRLevel(resLevel) &&
       result && (result->getNumberOfBands() == getNumberOfOutputBands()) &&
       (m_productType != RSPF_PRODUCT_TYPE_UNKNOWN) )
   {
      if ( resLevel > 0 )
      {
         //---
         // Check for overview tile.  Some overviews can contain r0 so always
         // call even if resLevel is 0 (if overview is not virtual).  Method 
         // returns true on success, false on error.
         //---
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

rspf_uint32 rspfRpfCacheTileSource::getNumberOfInputBands()const
{
  switch(m_productType)
  {
  case  RSPF_PRODUCT_TYPE_UNKNOWN:
    {
      return 0;
    }
  case RSPF_PRODUCT_TYPE_CIB:
    {
      return 1;
    }
  case RSPF_PRODUCT_TYPE_CADRG:
    {
      return 3;
    }
  }

  return 0;
}

rspf_uint32 rspfRpfCacheTileSource::getNumberOfOutputBands()const
{
  switch(m_productType)
  {
  case  RSPF_PRODUCT_TYPE_UNKNOWN:
    {
      return 0;
    }
  case RSPF_PRODUCT_TYPE_CIB:
    {
      return 1;
    }
  case RSPF_PRODUCT_TYPE_CADRG:
    {
      return 3;
    }
  }

  return 0;
}

rspf_uint32 rspfRpfCacheTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return m_numberOfLines;
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfLines(reduced_res_level);
   }
   
   return 0;
}

rspf_uint32 rspfRpfCacheTileSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return m_numberOfSamples;
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfSamples(reduced_res_level);
   }
   
   return 0;
}

void rspfRpfCacheTileSource::setActualImageRect()
{
   m_actualImageRect = rspfIrect(0,0,m_numberOfLines, m_numberOfSamples);
}

rspfIrect rspfRpfCacheTileSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   return rspfIrect(0,                         // upper left x
                     0,                         // upper left y
                     getNumberOfSamples(reduced_res_level) - 1,  // lower right x
                     getNumberOfLines(reduced_res_level)   - 1); // lower right y                     
}

rspfRefPtr<rspfImageGeometry> rspfRpfCacheTileSource::getImageGeometry()
{
   if (theGeometry.valid()) return theGeometry;
   
   // datum
   // WGS 84
   rspfKeywordlist kwl;
   const char* prefix = 0; // legacy
   kwl.add(prefix,
           rspfKeywordNames::DATUM_KW,
           "WGE",
           true);   
   
   rspfGpt ul(m_bBox_UR_Lat,m_bBox_LL_Lon);
   rspfGpt ll(m_bBox_LL_Lat,m_bBox_LL_Lon);
   rspfGpt ur(m_bBox_UR_Lat,m_bBox_UR_Lon);
   rspfGpt lr(m_bBox_LL_Lat,m_bBox_UR_Lon);

   double latInterval = fabs(ul.latd() - lr.latd())/ getNumberOfLines();
   double lonInterval = fabs(ul.lond() - ur.lond())/ getNumberOfSamples();

   kwl.add(prefix,
           rspfKeywordNames::UL_LAT_KW,
           ul.latd(),//-(latInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::UL_LON_KW,
           ul.lond(),//+(lonInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::LL_LAT_KW,
           ll.latd(),//+(latInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::LL_LON_KW,
           ll.lond(),//+(lonInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::LR_LAT_KW,
           lr.latd(),//+(latInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::LR_LON_KW,
           lr.lond(),//-(lonInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::UR_LAT_KW,
           ur.latd(),//-(latInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::UR_LON_KW,
           ur.lond(),//-(latInterval/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_INPUT_BANDS_KW,
           getNumberOfInputBands(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_OUTPUT_BANDS_KW,
           getNumberOfOutputBands(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_LINES_KW,
           getNumberOfLines(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_SAMPLES_KW,
           getNumberOfSamples(),
           true);


   //---
   // Make a projection to get the easting / northing of the tie point and
   // the scale in meters.  This will only be used by the CIB.
   //---
   kwl.add(prefix,
           rspfKeywordNames::DATUM_KW,
           "WGE",
           true);

   rspfGpt origin((ul.latd()+lr.latd())*.5,
                   (ul.lond()+lr.lond())*.5,
                   0.0);

   double deltaLatPerPixel = latInterval;
   double deltaLonPerPixel = lonInterval;

   rspfDpt tie;

   tie.lat = ul.latd() - deltaLatPerPixel/2.0;
   tie.lon = ul.lond() + deltaLonPerPixel/2.0;

   kwl.add(prefix, 
           rspfKeywordNames::TIE_POINT_XY_KW,
           tie.toString(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
           deltaLatPerPixel,
           true);

   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
           deltaLonPerPixel,
           true);

   kwl.add(prefix,
           rspfKeywordNames::ORIGIN_LATITUDE_KW,
           origin.latd(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::CENTRAL_MERIDIAN_KW,
           origin.lond(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LAT_KW,
           ul.latd()-(deltaLatPerPixel/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LON_KW,
           ul.lond()+(deltaLonPerPixel/2.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfEquDistCylProjection",
           true);
  
   // Capture this for next time.
   theGeometry = new rspfImageGeometry;
   theGeometry->loadState(kwl, prefix);

   // Set image things the geometry object should know about.
   initImageParameters( theGeometry.get() ); 

   return theGeometry;
}

rspfScalarType rspfRpfCacheTileSource::getOutputScalarType() const
{
   return RSPF_UCHAR;
}

rspf_uint32 rspfRpfCacheTileSource::getTileWidth() const
{
   return m_tileSize.x;
}

rspf_uint32 rspfRpfCacheTileSource::getTileHeight() const
{
   return m_tileSize.y;
}

bool rspfRpfCacheTileSource::setCurrentEntry(rspf_uint32 entryIdx)
{
   // Clear the geometry.
   theGeometry = 0;

   // Must clear or openOverview will use last entries.
   theOverviewFile.clear();

   if(!setEntryToRender(entryIdx))
   {
      return false;
   }
   completeOpen();

   return true;
}

void rspfRpfCacheTileSource::getEntryList(std::vector<rspf_uint32>& entryList)const
{
   entryList.push_back(0);
}

bool rspfRpfCacheTileSource::setEntryToRender(rspf_uint32 /* index */)
{
   if(isOpen())
   { 
      populateLut();
      return true;
   }

   return false;
}

bool rspfRpfCacheTileSource::isValidRLevel(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return true;
   }
   else if (theOverview.valid())
   {
      return theOverview->isValidRLevel(reduced_res_level);
   }
   else
   {
      return false;
   }
}

vector<rspfRpfCacheTileSource::rspfFrameEntryData> rspfRpfCacheTileSource::getIntersectingEntries(const rspfIrect& rect)
{
   vector<rspfFrameEntryData> result;

   // make sure we have the Toc entry to render
   if(!isOpen()) return result;

   rspfIrect imageRect = getImageRectangle();
   if(rect.intersects(imageRect))
   {
      rspfIrect clipRect  = rect.clipToRect(imageRect);
      rspfIrect frameRect(clipRect.ul().x/CIBCADRG_FRAME_WIDTH,
                           clipRect.ul().y/CIBCADRG_FRAME_HEIGHT,
                           clipRect.lr().x/CIBCADRG_FRAME_WIDTH,
                           clipRect.lr().y/CIBCADRG_FRAME_HEIGHT);

      for(rspf_int32 row = frameRect.ul().y; row <= frameRect.lr().y; ++row)
      {
         for(rspf_int32 col = frameRect.ul().x; col <= frameRect.lr().x; ++col)
         {
            rspfRpfFrameEntry tempEntry = m_frameEntryArray[row][col];
            if(tempEntry.exists())
            {
               result.push_back(rspfFrameEntryData(row,
                                                    col,
                                                    row*CIBCADRG_FRAME_HEIGHT,
                                                    col*CIBCADRG_FRAME_WIDTH,
                                                    tempEntry));
            }
         }
      }
   }

   return result;
}

void rspfRpfCacheTileSource::fillTile(
   const rspfIrect& tileRect,
   const vector<rspfFrameEntryData>& framesInvolved,
   rspfImageData* tile)
{
   rspf_uint32 idx = 0;
   for(idx = 0;
       idx < framesInvolved.size();
       ++idx)
   {

      if(m_workFrame->parseFile(framesInvolved[idx].theFrameEntry.getFullPath())
         == rspfErrorCodes::RSPF_OK)
      {
         // we will fill a subtile.  We pass in which frame it is and the position of the frame.
         // the actual pixel will be 1536*row and 1536 *col.
         if(m_productType == RSPF_PRODUCT_TYPE_CIB)
         {
            fillSubTileCib(*m_workFrame,
                           tileRect,
                           framesInvolved[idx],
                           tile);
         }
         else
         {
            fillSubTileCadrg(*m_workFrame,
                             tileRect,
                             framesInvolved[idx],
                             tile);
         }
      }
   }
}

void rspfRpfCacheTileSource::fillSubTileCadrg(
   const rspfRpfFrame&  aFrame,
   const rspfIrect& tileRect,
   const rspfFrameEntryData& frameEntryData,
   rspfImageData* tile)
{;
// first let's grab the absolute position of the frame rectangle in pixel space
   rspfIrect frameRect(frameEntryData.thePixelCol,
                        frameEntryData.thePixelRow,
                        frameEntryData.thePixelCol + CIBCADRG_FRAME_WIDTH  - 1,
                        frameEntryData.thePixelRow + CIBCADRG_FRAME_HEIGHT - 1);


// now clip it to the tile
   rspfIrect clipRect = tileRect.clipToRect(frameRect);

   const rspfRpfCompressionSection* compressionSection = aFrame.getCompressionSection();

   if(!compressionSection)
   {
      return;
   }

   const vector<rspfRpfColorGrayscaleTable>& colorTable =
      aFrame.getColorGrayscaleTable();

// ESH 03/2009 -- Partial fix for ticket #646.
// Crash fix on reading RPFs: Make sure the colorTable vector 
// has entries before trying to make use of them. 
   int numTables = (int)colorTable.size();
   if ( numTables <= 0 )
   {
      return;
   }

   rspf_uint8 *tempRows[3];

   tempRows[0] = m_uncompressedBuffer;
   tempRows[1] = (m_uncompressedBuffer + 256*256);
   tempRows[2] = (tempRows[1] + 256*256);

// find the shift to 0,0
   rspfIpt tempDelta(clipRect.ul().x - frameEntryData.thePixelCol,
                      clipRect.ul().y - frameEntryData.thePixelRow);


// In order to compute the subframe we will need the corner offsets of
// the upper left of the frame and the upper left of the clip rect.  The
// clip rect should be completely within the frame.  This just translates the value
// to make the upper left of the frame be 0,0.
//
   rspfIrect offsetRect(tempDelta.x,
                         tempDelta.y,
                         tempDelta.x + clipRect.width()-1,
                         tempDelta.y + clipRect.height()-1);

// each subframe is 64x64.  We will actually use 256x256 since
// we will be uncompressing them.  Note CADRG is a 256x256 tile
// compressed to 64x64x12 bit data
//
   rspfIrect subFrameRect(offsetRect.ul().x/256,
                           offsetRect.ul().y/256,
                           (offsetRect.lr().x)/256,
                           (offsetRect.lr().y)/256);

   rspf_uint32 readPtr  = 0;

   rspf_int32 row = 0;
   rspf_int32 col = 0;
   rspf_uint32 i = 0;
   rspf_uint32 j = 0;
   rspf_int32 upperY = subFrameRect.lr().y;
   rspf_int32 upperX = subFrameRect.lr().x;
   rspf_int32 lowerY = subFrameRect.ul().y;
   rspf_int32 lowerX = subFrameRect.ul().x; 
   for(row = lowerY; row <= upperY; ++row)
   {
      for(col = lowerX; col <= upperX; ++col)
      {
         readPtr = 0;
         if(aFrame.fillSubFrameBuffer(m_compressedBuffer, 0, row, col))
         {
            for (i = 0; i < 256; i += 4)
            {
               for (j = 0; j < 256; j += 8)
               {
                  rspf_uint16 firstByte  = m_compressedBuffer[readPtr++] & 0xff;
                  rspf_uint16 secondByte = m_compressedBuffer[readPtr++] & 0xff;
                  rspf_uint16 thirdByte  = m_compressedBuffer[readPtr++] & 0xff;

                  //because dealing with half-bytes is hard, we
                  //uncompress two 4x4 tiles at the same time. (a
                  //4x4 tile compressed is 12 bits )
                  // this little code was grabbed from openmap software.

                  /* Get first 12-bit value as index into VQ table */
                  // I think we need to swap
                  rspf_uint16 val1 = (firstByte << 4) | (secondByte >> 4);

                  /* Get second 12-bit value as index into VQ table*/
                  rspf_uint16 val2 = ((secondByte & 0x000F) << 8) | thirdByte;

                  for (rspf_uint32 t = 0; t < 4; ++t)
                  {
                     for (rspf_uint32 e = 0; e < 4; ++e)
                     {
                        rspf_uint16 tableVal1 = compressionSection->getTable()[t].theData[val1*4 + e] & 0xff;
                        rspf_uint16 tableVal2 = compressionSection->getTable()[t].theData[val2*4 + e] & 0xff;

                        rspf_uint32 pixindex = ((i+t)*256) +
                           (j + e);
                        const rspf_uint8* color1 = colorTable[0].getStartOfData(tableVal1);
                        const rspf_uint8* color2 = colorTable[0].getStartOfData(tableVal2);


                        tempRows[0][pixindex] = color1[0];
                        tempRows[1][pixindex] = color1[1];
                        tempRows[2][pixindex] = color1[2];

                        tempRows[0][pixindex+4] = color2[0];
                        tempRows[1][pixindex+4] = color2[1];
                        tempRows[2][pixindex+4] = color2[2];
                     } //for e
                  } //for t
               }  /* for j */
            } //for i
         }
         else
         {
            memset(m_uncompressedBuffer, 0, 256*256*3);
         }
         rspf_int32 tempCol = col*256;
         rspf_int32 tempRow = row*256;
         rspfIrect subRectToFill(frameRect.ul().x + tempCol,
                                  frameRect.ul().y + tempRow,
                                  frameRect.ul().x + tempCol + 255,
                                  frameRect.ul().y + tempRow + 255);
         tile->loadTile(m_uncompressedBuffer,
                        subRectToFill,
                        RSPF_BSQ);
      }
   }
}

void rspfRpfCacheTileSource::fillSubTileCib(
   const rspfRpfFrame&  aFrame,
   const rspfIrect& tileRect,
   const rspfFrameEntryData& frameEntryData,
   rspfImageData* tile)
{
   // first let's grab the absolute position of the frame rectangle in pixel
   // space
   rspfIrect frameRect(frameEntryData.thePixelCol,
                        frameEntryData.thePixelRow,
                        frameEntryData.thePixelCol + CIBCADRG_FRAME_WIDTH  - 1,
                        frameEntryData.thePixelRow + CIBCADRG_FRAME_HEIGHT - 1);


   // now clip it to the tile
   rspfIrect clipRect = tileRect.clipToRect(frameRect);

   const rspfRpfCompressionSection* compressionSection = aFrame.getCompressionSection();

   if(!compressionSection)
   {
      return;
   }

   const vector<rspfRpfColorGrayscaleTable>& colorTable =
      aFrame.getColorGrayscaleTable();

   // ESH 03/2009 -- Partial fix for ticket #646.
   // Crash fix on reading RPFs: Make sure the colorTable vector 
   // has entries before trying to make use of them. 
   int numTables = (int)colorTable.size();
   if ( numTables <= 0 )
   {
      return;
   }

   // check to see if it does overlap.  If it doesn't then the width and height
   // will be a single point
   {
      rspf_uint8 *tempRow;

      tempRow = m_uncompressedBuffer;

      // find the shift to 0,0
      rspfIpt tempDelta(clipRect.ul().x - frameEntryData.thePixelCol,
                         clipRect.ul().y - frameEntryData.thePixelRow);

      // In order to compute the subframe we will need the corner offsets of
      // the upper left of the frame and the upper left of the clip rect.  The
      // clip rect should be completely within the frame.  This just translates the value
      // to make the upper left of the frame be 0,0.
      //
      rspfIrect offsetRect(tempDelta.x,
                            tempDelta.y,
                            tempDelta.x + clipRect.width()-1,
                            tempDelta.y + clipRect.height()-1);

      // each subframe is 64x64.  We will actually use 256x256 since
      // we will be uncompressing them.  Note CADRG is a 256x256 tile
      // compressed to 64x64x12 bit data
      //
      rspfIrect subFrameRect(offsetRect.ul().x/256,
                              offsetRect.ul().y/256,
                              (offsetRect.lr().x)/256,
                              (offsetRect.lr().y)/256);

      rspf_int32 row = 0;
      rspf_int32 col = 0;
      rspf_uint32 i = 0;
      rspf_uint32 j = 0;
      rspf_uint32 readPtr = 0;
      for(row = subFrameRect.ul().y; row <= subFrameRect.lr().y; ++row)
      {
         for(col = subFrameRect.ul().x; col <= subFrameRect.lr().x; ++col)
         {
            readPtr = 0;
            if(aFrame.fillSubFrameBuffer(m_compressedBuffer, 0, row, col))
            {
               for (i = 0; i < 256; i += 4)
               {
                  for (j = 0; j < 256; j += 8)
                  {
                     rspf_uint16 firstByte  = m_compressedBuffer[readPtr++] & 0xff;
                     rspf_uint16 secondByte = m_compressedBuffer[readPtr++] & 0xff;
                     rspf_uint16 thirdByte  = m_compressedBuffer[readPtr++] & 0xff;

                     //because dealing with half-bytes is hard, we
                     //uncompress two 4x4 tiles at the same time. (a
                     //4x4 tile compressed is 12 bits )
                     // this little code was grabbed from openmap software.

                     /* Get first 12-bit value as index into VQ table */
                     // I think we need to swap
                     rspf_uint16 val1 = (firstByte << 4) | (secondByte >> 4);

                     /* Get second 12-bit value as index into VQ table*/
                     rspf_uint16 val2 = ((secondByte & 0x000F) << 8) | thirdByte;

                     for (rspf_uint32 t = 0; t < 4; ++t)
                     {
                        for (rspf_uint32 e = 0; e < 4; ++e)
                        {
                           rspf_uint16 tableVal1 = compressionSection->getTable()[t].theData[val1*4 + e] & 0xff;
                           rspf_uint16 tableVal2 = compressionSection->getTable()[t].theData[val2*4 + e] & 0xff;

                           rspf_uint32 pixindex = ((i+t)*256) + (j + e);
                           const rspf_uint8* color1 = colorTable[0].getStartOfData(tableVal1);
                           const rspf_uint8* color2 = colorTable[0].getStartOfData(tableVal2);


                           tempRow[pixindex]      = color1[0];
                           tempRow[pixindex + 4]  = color2[0];
                        } //for e
                     } //for t
                  }  /* for j */
               } //for i
            }
            else
            {
               memset(m_uncompressedBuffer, 0, 256*256);
            }
            rspf_int32 tCol = col*256;
            rspf_int32 tRow = row*256;
            rspfIrect subRectToFill(frameRect.ul().x + tCol,
                                     frameRect.ul().y + tRow,
                                     frameRect.ul().x + tCol + 255,
                                     frameRect.ul().y + tRow + 255);
            tile->loadTile(m_uncompressedBuffer,
                           subRectToFill,
                           RSPF_BSQ);
         }
      }
   }  
}

void rspfRpfCacheTileSource::allocateForProduct()
{
   if(m_productType ==  RSPF_PRODUCT_TYPE_UNKNOWN)
   {
      return;
   }
   if(m_uncompressedBuffer)
   {
      delete [] m_uncompressedBuffer;
      m_uncompressedBuffer = 0;
   }
   if(m_compressedBuffer)
   {
      delete [] m_compressedBuffer;
      m_compressedBuffer = 0;
   }

   // a CADRG and CIBis a 64*64*12 bit buffer and must divide by 8 to
   // convert to bytes
   m_compressedBuffer   = new rspf_uint8[(64*64*12)/8];
   if(m_productType == RSPF_PRODUCT_TYPE_CIB)
   {
      m_uncompressedBuffer = new rspf_uint8[256*256];
   }
   else
   {
      m_uncompressedBuffer = new rspf_uint8[256*256*3];
   }

   m_tile = rspfImageDataFactory::instance()->create(this, this);
   m_tile->initialize();
}

void rspfRpfCacheTileSource::deleteAll()
{
   theOverview = 0;
}

bool rspfRpfCacheTileSource::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   bool result = rspfImageHandler::saveState(kwl, prefix);

   return result;
}

bool rspfRpfCacheTileSource::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   const char* MODULE = "rspfRpfCacheTileSource::loadState";

   if(traceDebug())
   {
      CLOG << "Entering..." << endl;
   }
   bool result = rspfImageHandler::loadState(kwl, prefix);

   if(!result)
   {
      if(traceDebug())
      {
         CLOG << "Leaving..." << endl;
      }
      return false;
   }
   const char* lookup = 0;
   lookup = kwl.find(rspfString(prefix), "entry");
   rspf_int32 entry = rspfString(lookup).toInt32();

   // if an entry is specified then
   // call the open with an entry number
   if(lookup)
   {
      if(traceDebug())
      {
         CLOG << "Leaving..." << endl;
      }
      result = rspfImageHandler::open(theImageFile);
      setCurrentEntry(entry);
      return result;
   }

   result = rspfImageHandler::open(theImageFile);

   return result;
}

rspf_uint32 rspfRpfCacheTileSource::getImageTileWidth() const
{
   return 256;
}

rspf_uint32 rspfRpfCacheTileSource::getImageTileHeight() const
{
   return 256;
}

bool rspfRpfCacheTileSource::isCib() const
{
   return (m_productType==RSPF_PRODUCT_TYPE_CIB);
}

bool rspfRpfCacheTileSource::isCadrg() const
{
   return (m_productType==RSPF_PRODUCT_TYPE_CADRG);
}

rspfRefPtr<rspfProperty> rspfRpfCacheTileSource::getProperty(const rspfString& name)const
{
   if(name == "file_type")
   {
      if(m_productType == RSPF_PRODUCT_TYPE_CIB)
      {
         return new rspfStringProperty("file_type", "CIB");
      }
      else if(m_productType == RSPF_PRODUCT_TYPE_CADRG)
      {
         return new rspfStringProperty("file_type", "CADRG");
      }
      return 0;
   }
   return rspfImageHandler::getProperty(name);
}

void rspfRpfCacheTileSource::populateLut()
{
   theLut = 0;
   if(m_fileNames.size() > 0)
   {
      // bool found = false;
      rspfRpfFrameEntry tempEntry;
      rspfRpfFrame aFrame;
      rspfFilename fileName = m_fileNames[0].split("|")[0];
      // if (fileName.exists())
      // {
      //    found = true;
      // }

      if(aFrame.parseFile(fileName)
         == rspfErrorCodes::RSPF_OK)
      {
         const vector<rspfRpfColorGrayscaleTable>& colorTable =
            aFrame.getColorGrayscaleTable();

         // ESH 03/2009 -- Partial fix for ticket #646.
         // Crash fix on reading RPFs: Make sure the colorTable vector 
         // has entries before trying to make use of them. 
         int numTables = (int)colorTable.size();

         rspf_uint32 numElements = (numTables > 0) ? colorTable[0].getNumberOfElements() : 0;
         if(numElements > 0)
         {
            if((m_productType == RSPF_PRODUCT_TYPE_CIB)||
               (m_productType == RSPF_PRODUCT_TYPE_CADRG))
            {
               theLut = new rspfNBandLutDataObject(numElements,
                                                    3,
                                                    RSPF_UINT8);
            }
            else
            {
               theLut = 0;
               return;
            }
            rspf_uint32 idx = 0;

            for(idx = 0; idx < numElements;++idx)
            {
               const rspf_uint8* startOfData = colorTable[0].getStartOfData(idx);
               switch(m_productType)
               {
                  case RSPF_PRODUCT_TYPE_CIB:
                  {
                     (*theLut)[idx][0] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(startOfData[0]);
                     (*theLut)[idx][1] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(startOfData[0]);
                     (*theLut)[idx][2] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(startOfData[0]);
                     break;
                  }
                  case RSPF_PRODUCT_TYPE_CADRG:
                  {
                     (*theLut)[idx][0] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(startOfData[0]);
                     (*theLut)[idx][1] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(startOfData[1]);
                     (*theLut)[idx][2] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(startOfData[2]);
                     break;
                  }
                  default:
                  {
                     break;
                  }
               }
            }
         }
      }
   }
}

void rspfRpfCacheTileSource::establishDecimationFactors()
{
   theDecimationFactors.clear();
   
   // Just needed to set the first R level here, the base class can do the rest:
   rspfImageHandler::establishDecimationFactors();
}

