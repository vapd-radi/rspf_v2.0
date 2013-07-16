//*****************************************************************************
// FILE: rspfDtedHandler.cc
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains implementation of class rspfDtedHandler. This class derives from
//   rspfElevHandler. It is responsible for loading an individual DTED cell
//   from disk. This elevation files are memory mapped.
//
//*****************************************************************************
// $Id: rspfDtedHandler.cpp 21214 2012-07-03 16:20:11Z dburken $

#include <cstdlib>
#include <cstring> /* for memcpy */
#include <rspf/elevation/rspfDtedHandler.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfKeywordlist.h>

RTTI_DEF1(rspfDtedHandler, "rspfDtedHandler" , rspfElevCellHandler)

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfDtedHandler:exec");
static rspfTrace traceDebug ("rspfDtedHandler:debug");

static const char ENABLE_STATS_KW[] = "elevation.compute_statistics.enabled";


rspfDtedHandler::rspfDtedHandler(const rspfFilename& dted_file, bool memoryMapFlag)
   :
      rspfElevCellHandler(dted_file),
      m_fileStr(),
      m_numLonLines(0),
      m_numLatPoints(0),
      m_dtedRecordSizeInBytes(0),
      m_edition(),
      m_productLevel(),
      m_compilationDate(),
      m_offsetToFirstDataRecord(0),
      m_latSpacing(0.0),
      m_lonSpacing(0.0),
      m_swCornerPost(),
      m_swapBytesFlag(false)
{

   static const char MODULE[] = "rspfDtedHandler (Filename) Constructor";
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG " << MODULE <<": entering..." << std::endl;
   }
   
   m_swapBytesFlag = rspf::byteOrder() == RSPF_LITTLE_ENDIAN ? true : false;
   //---
   //  Open the dted file for reading.
   //---
   if (!open(dted_file, memoryMapFlag))
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL " << MODULE << ": "
            << "\nCould not open file:  " << dted_file.c_str()
            << "\nReturning..." << std::endl;
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG " << MODULE << ": returning with error..." << std::endl;
      }
      return;
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG " << MODULE <<": Loading dted file: " << dted_file
            << std::endl;
      }
   }

   // DTED is stored in big endian.
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG " << MODULE << ": returning..." << std::endl;
   }
}

rspfDtedHandler::~rspfDtedHandler()
{
   close();
}

bool rspfDtedHandler::getAccuracyInfo(rspfElevationAccuracyInfo& info,
                                       const rspfGpt& /* gpt */ ) const
{
  info.m_confidenceLevel = .9;
  info.m_absoluteLE = m_acc.absLE();
  info.m_absoluteCE = m_acc.absCE();
  info.m_relativeLE = m_acc.relLE();
  info.m_relativeCE = m_acc.relCE();

  info.m_surfaceName = m_dsi.productLevel();

  return info.hasValidAbsoluteError();
}

double rspfDtedHandler::getHeightAboveMSL(const rspfGpt& gpt)
{
   if(m_fileStr.is_open())
   {
      return getHeightAboveMSL(gpt, true);
   }
   else if(m_memoryMap.size())
   {
      return getHeightAboveMSL(gpt, false);
   }
   
   return rspf::nan();
}

bool rspfDtedHandler::open(const rspfFilename& file, bool memoryMapFlag)
{
   static const char* MODULE = "rspfDtedHandler::open";
   close();
   theFilename = file;
   m_fileStr.clear();

   m_fileStr.open(file.c_str(), 
                  std::ios::in | std::ios::binary);
   if(!m_fileStr.good())
   {
      return false;
   }
   m_numLonLines = 0;
   m_numLatPoints = 0;
   m_dtedRecordSizeInBytes = 0;
   
   // Attempt to parse.
   m_vol.parse(m_fileStr);
   m_hdr.parse(m_fileStr);
   m_uhl.parse(m_fileStr);
   m_dsi.parse(m_fileStr);
   m_acc.parse(m_fileStr);

   //***
   // Check for errors.  Must have uhl, dsi and acc records.  vol and hdr
   // are for magnetic tape only; hence, may or may not be there.
   //***
   if (m_uhl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR ||
       m_dsi.getErrorStatus() == rspfErrorCodes::RSPF_ERROR ||
       m_acc.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG " << MODULE << ": "
         << "\nError parsing file:  " << file.c_str()
         << "\nPossibly not a dted file."
         << std::endl;
      }
      
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      close();
      return false;
   }
   if(memoryMapFlag)
   {
      m_fileStr.seekg(0);
      m_fileStr.clear();
      m_memoryMap.resize(theFilename.fileSize());
      m_fileStr.read((char*)(&m_memoryMap.front()), (std::streamsize)m_memoryMap.size());
      m_fileStr.close();
   }
   
   m_numLonLines  = m_uhl.numLonLines();
   m_numLatPoints = m_uhl.numLatPoints();
   m_latSpacing   = m_uhl.latInterval();
   m_lonSpacing   = m_uhl.lonInterval();
   m_dtedRecordSizeInBytes = m_numLatPoints*2+rspfDtedRecord::DATA_LENGTH;
   
   m_edition  = m_dsi.edition();
   m_productLevel = m_dsi.productLevel();
   m_compilationDate = m_dsi.compilationDate();
   
   m_offsetToFirstDataRecord = m_acc.stopOffset();
   
#if 0 /* Serious debug only... */
   std::cout << m_numLonLines
             << "\t" << m_numLatPoints
             << "\t" << m_lonSpacing
             << "\t" << m_latSpacing
             << "\t" << m_dtedRecordSizeInBytes
             << "\t" << theFilename.fileSize()
             << "\t" << file
             << "\t" << m_offsetToFirstDataRecord
             << std::endl;
#endif

   //***
   //  initialize the bounding rectangle:
   //***
   double south_boundary = m_uhl.latOrigin();
   double west_boundary  = m_uhl.lonOrigin();
   double north_boundary = south_boundary + m_latSpacing*(m_numLatPoints-1);
   double east_boundary  = west_boundary  + m_lonSpacing*(m_numLonLines-1);
   
   // For rspfElevCellHandler::pointHasCoverage method.
   theGroundRect = rspfGrect(rspfGpt(north_boundary, west_boundary, 0.0),
                              rspfGpt(south_boundary, east_boundary, 0.0));
   
   m_swCornerPost.lat = south_boundary;
   m_swCornerPost.lon = west_boundary;
   
   //***
   //  Determine the mean spacing:
   //***
   double center_lat = (south_boundary + north_boundary)/2.0;
   theMeanSpacing = (m_latSpacing + m_lonSpacing*rspf::cosd(center_lat))
                     * rspfGpt().metersPerDegree().x / 2.0;
   
   //  Initialize the accuracy values:
   theAbsLE90 = m_acc.absLE();
   theAbsCE90 = m_acc.absCE();
   
   // Set the base class null height value.
   theNullHeightValue = -32767.0;

   //---
   // Commented out as this writes an un-needed file.  (drb 20100611)
   // Get the statistics.
   // gatherStatistics();
   //---

   return true;
}

double rspfDtedHandler::getHeightAboveMSL(const rspfGpt& gpt, bool readFromFile)
{
   // Establish the grid indexes:
   double xi = (gpt.lon - m_swCornerPost.lon) / m_lonSpacing;
   double yi = (gpt.lat - m_swCornerPost.lat) / m_latSpacing;

   // Check for right edge.
   int x0 = static_cast<int>(xi);
   int y0 = static_cast<int>(yi);

   if(x0 == (m_numLonLines-1))
   {
      --x0; // Move over one post.
   }
   
   // Check for top edge.
   //    if (gpt.lat == theGroundRect.ul().lat)
   if(y0 == (m_numLatPoints-1))
   {
      --y0; // Move down one post.
   }

   // Do some error checking.
   if ( xi < 0.0 || yi < 0.0 ||
        x0 > (m_numLonLines  - 2.0) ||
        y0 > (m_numLatPoints - 2.0) )
   {
      return rspf::nan();
   }

   //***
   // Grab the four points from the dted cell needed.  These are big endian,
   // signed magnitude shorts so they must be interpreted accordingly.
   //***
   int offset = m_offsetToFirstDataRecord + x0 * m_dtedRecordSizeInBytes +
                y0 * 2 + DATA_RECORD_OFFSET_TO_POST;

   /// read the posts from the DTED file.
   DtedHeight postData;
   //
   if ( readFromFile )
   {
     readPostsFromFile( postData, offset );
   }
   else
   {
     rspf_uint8* buf = &m_memoryMap.front();
     {
       rspf_uint16 us;

       memcpy(&us, buf+offset, POST_SIZE);
       postData.m_posts[0].m_height = convertSignedMagnitude(us);
       memcpy(&us, buf+offset+POST_SIZE, POST_SIZE);
       postData.m_posts[1].m_height = convertSignedMagnitude(us);

       // Move over to the next column.
       offset += m_dtedRecordSizeInBytes;
       memcpy(&us, buf+offset, POST_SIZE);
       postData.m_posts[2].m_height = convertSignedMagnitude(us);
       memcpy(&us, buf+offset+POST_SIZE, POST_SIZE);
       postData.m_posts[3].m_height = convertSignedMagnitude(us);
     }
   }
   // Perform bilinear interpolation:
   double wx1 = xi  - x0;
   double wy1 = yi  - y0;
   double wx0 = 1.0 - wx1;
   double wy0 = 1.0 - wy1;
   
   postData.m_posts[0].m_weight = wx0*wy0;
   postData.m_posts[1].m_weight = wx0*wy1;
   postData.m_posts[2].m_weight = wx1*wy0;
   postData.m_posts[3].m_weight = wx1*wy1;

#if 0 /* Serious debug only... */
   postData.debug();
#endif

   return postData.calcHeight();
}

void rspfDtedHandler::readPostsFromFile( DtedHeight &postData, int offset)
{

  OpenThreads::ScopedLock <OpenThreads::Mutex> lock( m_fileStrMutex );
  rspf_sint16 ss;
  rspf_uint16 us;
  int postCount = 0;
  // read the posts in blocks 2x2.
  for ( int column = 0; column < NUM_POSTS_PER_BLOCK ; ++column )
  {
    m_fileStr.seekg( offset, std::ios::beg );
    for ( int row = 0; row < NUM_POSTS_PER_BLOCK ; ++row )
    {
      if ( !m_fileStr.eof() )
      {
        us = 0;
        m_fileStr.read( ( char* ) &us, POST_SIZE );
        // check the read was ok
        if ( m_fileStr.good() )
        {
          postData.m_posts[postCount].m_status = true;
        }
        else
        {
          // reset the goodbit
          m_fileStr.clear();
        }
        ss = convertSignedMagnitude( us );
        postData.m_posts[postCount].m_height = ss;
      }
      postCount++;
    }
    offset += m_dtedRecordSizeInBytes;
  }
}

double rspfDtedHandler::getPostValue(const rspfIpt& gridPt) const
{
   static const char MODULE[] = "rspfDtedHandler::getPostValue";
   
   // Do some error checking.
   if ( gridPt.x < 0.0 || gridPt.y < 0.0 ||
        gridPt.x > (m_numLonLines  - 1) ||
        gridPt.y > (m_numLatPoints - 1) )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "WARNING " << MODULE << ": No intersection..." << std::endl;
      }
      return rspf::nan();
   }

   if (!isOpen())
   {
      return rspf::nan();
   }

   int offset =
      m_offsetToFirstDataRecord + gridPt.x * m_dtedRecordSizeInBytes +
      gridPt.y * 2 + DATA_RECORD_OFFSET_TO_POST;
   
   // Put the file pointer at the start of the first elevation post.
   m_fileStr.seekg(offset, std::ios::beg);

   rspf_uint16 us;

   // Get the post.
   m_fileStr.read((char*)&us, POST_SIZE);
   
   return double(convertSignedMagnitude(us));
}

void rspfDtedHandler::gatherStatistics()
{
   //***
   // Check to see if there is a statistics file already.  If so; do a lookup
   // for the min and max values.
   //***
   rspfFilename stats_file = theFilename;//theFilename.path();
   stats_file.setExtension("statistics");
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfDtedHandler::gatherStatistics(): Looking for "
         << stats_file << " statistics file..." << std::endl;
   }

   rspfKeywordlist kwl;
   const char* min_str = NULL;
   const char* max_str = NULL;

   if (stats_file.exists())
   {
      if (kwl.addFile(stats_file))
      {
         // Look for the min_pixel_value keyword.
         min_str = kwl.find(rspfKeywordNames::MIN_VALUE_KW);
         max_str = kwl.find(rspfKeywordNames::MAX_VALUE_KW);
      }
   }

   if (min_str && max_str)
   {
      theMinHeightAboveMSL = atoi(min_str);
      theMaxHeightAboveMSL = atoi(max_str);
   }
   else if (theComputeStatsFlag&&!m_memoryMap.size())  // Scan the cell and gather the statistics...
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "NOTICE rspfDtedHandler::gatherStatistics():"
            << " scanning for min/max"
            << "\nThis may take a while..." << std::endl;
      }
      // Start off with the min and max pegged.
      theMinHeightAboveMSL =  32767;
      theMaxHeightAboveMSL = -32767;
      
      // Put the file pointer at the start of the first elevation post.
      m_fileStr.seekg(m_offsetToFirstDataRecord, std::ios::beg);
      
      //---
      // Loop through all records and scan for lowest min and highest max.
      // Each record contains a row of latitude points for a given longitude.
      // There are eight bytes in front of the post and four checksum bytes at
      // the end so ignore them.
      //---
      for (rspf_int32 i=0; i<m_numLonLines; ++i)  // longitude direction
      {
         m_fileStr.seekg(DATA_RECORD_OFFSET_TO_POST, std::ios::cur);
         
         for (rspf_int32 j=0; j<m_numLatPoints; ++j) // latitude direction
         {
            rspf_uint16 us;
            rspf_sint16 ss;
            m_fileStr.read((char*)&us, POST_SIZE);
            ss = convertSignedMagnitude(us);
            if (ss < theMinHeightAboveMSL && ss != NULL_POST)
            {
               theMinHeightAboveMSL = ss;
            }
            if (ss > theMaxHeightAboveMSL)
            {
               theMaxHeightAboveMSL = ss;
            }
         }
         
         m_fileStr.seekg(DATA_RECORD_CHECKSUM_SIZE, std::ios::cur);
      }
      
      // Add the stats to the keyword list.
      kwl.add(rspfKeywordNames::MIN_VALUE_KW, theMinHeightAboveMSL);
      kwl.add(rspfKeywordNames::MAX_VALUE_KW, theMaxHeightAboveMSL);
      
      // Write out the statistics file.
      kwl.write(stats_file.c_str());
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfDtedHandler::gatherStatistics:"
         << "\ntheMinHeightAboveMSL:  " << theMinHeightAboveMSL
         << "\ntheMaxHeightAboveMSL:  " << theMaxHeightAboveMSL
         << std::endl;
   }
}

rspfIpt rspfDtedHandler::getSizeOfElevCell() const
{
   return rspfIpt(m_numLonLines, m_numLatPoints);
}

rspfString  rspfDtedHandler::edition() const
{
   return m_edition;
}

rspfString  rspfDtedHandler::productLevel() const
{
   return m_productLevel;
}

rspfString  rspfDtedHandler::compilationDate() const
{
   return m_compilationDate;
}

const rspfDtedHandler& rspfDtedHandler::operator=(
   const rspfDtedHandler& rhs)
{
   return rhs;
}

rspfDtedHandler::rspfDtedHandler(const rspfDtedHandler&)
   :
   rspfElevCellHandler()
{}


/// DtedPost methods
rspfDtedHandler::DtedPost::~DtedPost(){}

/// DtedHeight methods
rspfDtedHandler::DtedHeight::DtedHeight() {}

rspfDtedHandler::DtedHeight::~DtedHeight(){}

double rspfDtedHandler::DtedHeight::calcHeight()
{
  double sum_weights = 0;
  double sum_posts = 0;

  for ( int i = 0; i < TOTAL_POSTS; ++i )
  {
    if ( m_posts[i].m_height == NULL_POST || !m_posts[i].m_status )
    {
      m_posts[i].m_weight = 0.0;
    }
  }

  for ( int i = 0; i < TOTAL_POSTS; ++i )
  {
    sum_weights += m_posts[i].m_weight;
    sum_posts += m_posts[i].m_height * m_posts[i].m_weight;
  }
  if ( sum_weights )
  {
    return sum_posts / sum_weights;
  }
  return rspf::nan();
}

void rspfDtedHandler::DtedHeight::debug()
{
  cout << "\np00:  " << m_posts[0].m_height
       << "\np01:  " << m_posts[1].m_height
       << "\np10:  " << m_posts[2].m_height
       << "\np11:  " << m_posts[3].m_height
       << "\nw00:  " << m_posts[0].m_weight
       << "\nw01:  " << m_posts[1].m_weight
       << "\nw10:  " << m_posts[2].m_weight
       << "\nw11:  " << m_posts[3].m_weight
       << "\ns00:  " << m_posts[0].m_status
       << "\ns01:  " << m_posts[1].m_status
       << "\ns10:  " << m_posts[2].m_status
       << "\ns11:  " << m_posts[3].m_status
       << std::endl;
}

