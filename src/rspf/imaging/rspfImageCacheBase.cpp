//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Mingjie Su
//
//********************************************************************
// $Id: rspfImageCacheBase.cpp 2685 2011-06-07 16:24:41Z david.burken $
#include <algorithm>
using namespace std;

#include <rspf/imaging/rspfImageCacheBase.h>

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

static rspfTrace traceDebug = rspfTrace("rspfImageCacheBase:debug");

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfImageCacheBase.cpp 2685 2011-06-07 16:24:41Z david.burken $";
#endif

RTTI_DEF1(rspfImageCacheBase, "rspfImageCacheBase", rspfImageHandler)

rspfImageCacheBase::rspfImageCacheBase()
   :
   rspfImageHandler(),
   m_actualImageRect(),
   m_numberOfLines(0),
   m_numberOfSamples(0),
   m_numberOfBands(0),
   m_fileNames(),
   m_tileSize(128, 128),
   m_workFrame(new rspfRpfFrame),
   m_bBox_LL_Lon(0.0),
   m_bBox_LL_Lat(0.0),
   m_bBox_UR_Lon(0.0),
   m_bBox_UR_Lat(0.0),
   m_numOfFramesVertical(0),
   m_numOfFramesHorizontal(0),
   m_frame_width(0),
   m_frame_height(0),
   m_frameEntryArray()
{
  if (traceDebug())
  {
    rspfNotify(rspfNotifyLevel_DEBUG)
      << "rspfImageCacheBase::rspfImageCacheBase entered...\n";
#ifdef RSPF_ID_ENABLED
    rspfNotify(rspfNotifyLevel_DEBUG)
      << "RSPF_ID:  " << RSPF_ID << "\n";
#endif      
  }

  m_actualImageRect.makeNan();

}

rspfImageCacheBase::~rspfImageCacheBase()
{
  if(m_workFrame)
  {
    delete m_workFrame;
    m_workFrame = 0;
  }
  close();
}

void rspfImageCacheBase::close()
{
  deleteAll();
}

bool rspfImageCacheBase::isOpen()const
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

bool rspfImageCacheBase::buildFrameEntryArray(rspfFilename imageFile, 
                                               rspf_uint32 frameWidth, 
                                               rspf_uint32 frameHeight)
{
   static const char MODULE[] = "rspfImageCacheBase::buildFrameEntryArray";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered..." << endl;
   }

   // use to check if tiles have overlap pixels
   rspf_float64 avgLat = 0.0;
   rspf_float64 avgLon = 0.0;

   m_frame_width = frameWidth;
   m_frame_height = frameHeight;

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
               
               m_numberOfBands = rspfString(bandStr).toInt32();
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

               avgLon = std::fabs(bBox_lon_diff/m_numOfFramesHorizontal);
               avgLat = std::fabs(bBox_lat_diff/m_numOfFramesVertical);
               
               m_numberOfLines   = m_numOfFramesVertical*m_frame_height;
               m_numberOfSamples = m_numOfFramesHorizontal*m_frame_width;

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

         if (std::fabs(tmpFrameLon - avgLon) > 0.002)
         {
            rspfNotify(rspfNotifyLevel_WARN) << fileName << " has overlap pixels with other image." << std::endl;
            return false;
         }
         
         rspf_float64 tmpRowPostion = std::fabs(m_bBox_UR_Lat - ur_lat);
         rspf_float64 tmpFrameLat = std::fabs(ur_lat - ll_lat);

         if (std::fabs(tmpFrameLat - avgLat) > 0.002)
         {
            rspfNotify(rspfNotifyLevel_WARN) << fileName << " has overlap pixels with other image." << std::endl;
            return false;
         }
         
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

void rspfImageCacheBase::checkLongitude(rspf_float64& leftLon,
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

rspf_uint32 rspfImageCacheBase::getNumberOfInputBands()const
{
   return m_numberOfBands;
}

rspf_uint32 rspfImageCacheBase::getNumberOfOutputBands()const
{
  return m_numberOfBands;
}

rspf_uint32 rspfImageCacheBase::getNumberOfLines(rspf_uint32 reduced_res_level) const
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

rspf_uint32 rspfImageCacheBase::getNumberOfSamples(rspf_uint32 reduced_res_level) const
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

void rspfImageCacheBase::setActualImageRect()
{
   m_actualImageRect = rspfIrect(0,0,m_numberOfLines, m_numberOfSamples);
}

rspfIrect rspfImageCacheBase::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   return rspfIrect(0,                         // upper left x
                     0,                         // upper left y
                     getNumberOfSamples(reduced_res_level) - 1,  // lower right x
                     getNumberOfLines(reduced_res_level)   - 1); // lower right y                     
}



rspf_uint32 rspfImageCacheBase::getTileWidth() const
{
   return m_tileSize.x;
}

rspf_uint32 rspfImageCacheBase::getTileHeight() const
{
   return m_tileSize.y;
}

void rspfImageCacheBase::getEntryList(std::vector<rspf_uint32>& entryList)const
{
   entryList.push_back(0);
}

bool rspfImageCacheBase::isValidRLevel(rspf_uint32 reduced_res_level) const
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

vector<rspfImageCacheBase::rspfFrameEntryData> rspfImageCacheBase::getIntersectingEntries(const rspfIrect& rect)
{
   vector<rspfFrameEntryData> result;

   // make sure we have the Toc entry to render
   if(!isOpen()) return result;

   rspfIrect imageRect = getImageRectangle();
   if(rect.intersects(imageRect))
   {
      rspfIrect clipRect  = rect.clipToRect(imageRect);
      rspfIrect frameRect(clipRect.ul().x/m_frame_width,
                           clipRect.ul().y/m_frame_height,
                           clipRect.lr().x/m_frame_width,
                           clipRect.lr().y/m_frame_height);

      for(rspf_int32 row = frameRect.ul().y; row <= frameRect.lr().y; ++row)
      {
         for(rspf_int32 col = frameRect.ul().x; col <= frameRect.lr().x; ++col)
         {
            rspfRpfFrameEntry tempEntry = m_frameEntryArray[row][col];
            if(tempEntry.exists())
            {
               result.push_back(rspfFrameEntryData(row,
                                                    col,
                                                    row*m_frame_height,
                                                    col*m_frame_width,
                                                    tempEntry));
            }
         }
      }
   }

   return result;
}

void rspfImageCacheBase::deleteAll()
{
   theOverview = 0;
}

bool rspfImageCacheBase::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   bool result = rspfImageHandler::saveState(kwl, prefix);

   return result;
}

bool rspfImageCacheBase::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   const char* MODULE = "rspfImageCacheBase::loadState";

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

rspf_uint32 rspfImageCacheBase::getImageTileWidth() const
{
   return 256;
}

rspf_uint32 rspfImageCacheBase::getImageTileHeight() const
{
   return 256;
}

void rspfImageCacheBase::establishDecimationFactors()
{
   theDecimationFactors.clear();
   
   // Just needed to set the first R level here, the base class can do the rest:
   rspfImageHandler::establishDecimationFactors();
}

rspfRefPtr<rspfImageGeometry> rspfImageCacheBase::getImageGeometry()
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

