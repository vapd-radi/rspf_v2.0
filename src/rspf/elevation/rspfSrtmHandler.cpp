//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Shuttle Radar Topography Mission (SRTM) elevation source.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmHandler.cpp 19444 2011-04-25 18:20:59Z dburken $

#include <rspf/elevation/rspfSrtmHandler.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfStreamFactoryRegistry.h>

RTTI_DEF1(rspfSrtmHandler, "rspfSrtmHandler" , rspfElevCellHandler)

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceDebug ("rspfSrtmElevSource:debug");

rspfSrtmHandler::rspfSrtmHandler()
   :
      rspfElevCellHandler(),
      m_streamOpen(false),
      m_numberOfLines(0),
      m_numberOfSamples(0),
      m_srtmRecordSizeInBytes(0),
      m_latSpacing(0.0),
      m_lonSpacing(0.0),
      m_nwCornerPost(),
      m_swapper(0)
{
}

rspfSrtmHandler::~rspfSrtmHandler()
{
   if (m_swapper)
   {
      delete m_swapper;
      m_swapper = 0;
   }
   close();
}

double rspfSrtmHandler::getHeightAboveMSL(const rspfGpt& gpt)
{
   if(!isOpen()) return rspf::nan();
   if(!m_memoryMap.empty())
   {
      switch(m_scalarType)
      {
         case RSPF_SINT16:
         {
            return getHeightAboveMSLMemoryTemplate((rspf_sint16)0, gpt);
         }
         case RSPF_FLOAT32:
         {
            return getHeightAboveMSLMemoryTemplate((rspf_float32)0, gpt);
         }
         default:
         {
            break;
         }
      }
   }
   else
   {
      switch(m_scalarType)
      {
         case RSPF_SINT16:
         {
            return getHeightAboveMSLFileTemplate((rspf_sint16)0, gpt);
         }
         case RSPF_FLOAT32:
         {
            return getHeightAboveMSLFileTemplate((rspf_float32)0, gpt);
         }
         default:
         {
            break;
         }
      }
   }

   return rspf::nan();
}

template <class T>
double rspfSrtmHandler::getHeightAboveMSLFileTemplate(T /* dummy */, const rspfGpt& gpt)
{
   // Establish the grid indexes:
   double xi = (gpt.lon - m_nwCornerPost.lon) / m_lonSpacing;
   double yi = (m_nwCornerPost.lat - gpt.lat) / m_latSpacing;

   int x0 = static_cast<int>(xi);
   int y0 = static_cast<int>(yi);

   if(x0 == (m_numberOfSamples-1))
   {
      --x0; // Move over one post.
   }
   
   // Check for top edge.
   //    if (gpt.lat == theGroundRect.ul().lat)
   if(y0 == (m_numberOfLines-1))
   {
      --y0; // Move down one post.
   }


   // Do some error checking.
   if ( xi < 0.0 || yi < 0.0 ||
        x0 > (m_numberOfSamples  - 2.0) ||
        y0 > (m_numberOfLines    - 2.0) )
   {
      return rspf::nan();
   }

   T p[4];
   
   double p00;
   double p01;
   double p10;
   double p11;

   //---
   // Grab the four points from the srtm cell needed.
   //---
   std::streampos offset = y0 * m_srtmRecordSizeInBytes + x0 * sizeof(T);

   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_fileStrMutex);

      // lets a seek cur
      // 
      // Put the file pointer at the start of the first elevation post.
      m_fileStr.seekg(offset, std::ios::beg);
      // Get the first post.
      m_fileStr.read((char*)p, sizeof(T));
      
      // Get the second post.
      m_fileStr.read((char*)(p+1), sizeof(T));
      
      // Move over to the next column.
      offset += m_srtmRecordSizeInBytes;
      
      m_fileStr.seekg(offset, std::ios::beg);
      
      // Get the third post.
      m_fileStr.read((char*)(p+2), sizeof(T));
      
      // Get the fourth post.
      m_fileStr.read((char*)(p+3), sizeof(T));
      
      if(m_fileStr.fail())
      {
         return rspf::nan();
      }
   }
   if (m_swapper)
   {
      m_swapper->swap((T*)p, 4);
   }

   p00 = p[0];
   p01 = p[1];
   p10 = p[2];
   p11 = p[3];
   
   // Perform bilinear interpolation:

   double xt0 = xi - x0;
   double yt0 = yi - y0;
   double xt1 = 1-xt0;
   double yt1 = 1-yt0;

   double w00 = xt1*yt1;
   double w01 = xt0*yt1;
   double w10 = xt1*yt0;
   double w11 = xt0*yt0;

   //***
   // Test for null posts and set the corresponding weights to 0:
   //***
   if (p00 == theNullHeightValue)
      w00 = 0.0;
   if (p01 == theNullHeightValue)
      w01 = 0.0;
   if (p10 == theNullHeightValue)
      w10 = 0.0;
   if (p11 == theNullHeightValue)
      w11 = 0.0;

#if 0 /* Serious debug only... */
   cout << "\np00:  " << p00
        << "\np01:  " << p01
        << "\np10:  " << p10
        << "\np11:  " << p11
        << "\nw00:  " << w00
        << "\nw01:  " << w01
        << "\nw10:  " << w10
        << "\nw11:  " << w11
        << endl;
#endif

   double sum_weights = w00 + w01 + w10 + w11;

   if (sum_weights)
   {
      return (p00*w00 + p01*w01 + p10*w10 + p11*w11) / sum_weights;
   }

   return rspf::nan();
}


template <class T>
double rspfSrtmHandler::getHeightAboveMSLMemoryTemplate(T /* dummy */,
                                                         const rspfGpt& gpt)
{
   // Establish the grid indexes:
   double xi = (gpt.lon - m_nwCornerPost.lon) / m_lonSpacing;
   double yi = (m_nwCornerPost.lat - gpt.lat) / m_latSpacing;
   
   int x0 = static_cast<int>(xi);
   int y0 = static_cast<int>(yi);
   
   if(x0 == (m_numberOfSamples-1))
   {
      --x0; // Move over one post.
   }
   
   // Check for top edge.
   //    if (gpt.lat == theGroundRect.ul().lat)
   if(y0 == (m_numberOfLines-1))
   {
      --y0; // Move down one post.
   }
#if 0
   // Check for right edge.
   if (rspf::almostEqual(gpt.lon,theGroundRect.lr().lon))
   {
      --x0; // Move over one post to the left.
   }
   
   // Check for bottom edge.
   if (rspf::almostEqual(gpt.lat,theGroundRect.lr().lat))
   {
      --y0; // Move up one post.
   }
#endif
   // Do some error checking.
   if ( xi < 0.0 || yi < 0.0 ||
       x0 > (m_numberOfSamples  - 2.0) ||
       y0 > (m_numberOfLines    - 2.0) )
   {
      //       std::cout << "rspfSrtmHandler::getHeightAboveMSLTemplate: leaving 1" << std::endl;
      return rspf::nan();
   }

   // Grab the four points from the srtm cell needed.
   rspf_uint64 offset = y0 * m_srtmRecordSizeInBytes + x0 * sizeof(T);
   rspf_uint64 offset2 =offset+m_srtmRecordSizeInBytes;
   T v00 = *(reinterpret_cast<T*> (&m_memoryMap[offset]));
   T v01 = *(reinterpret_cast<T*> (&m_memoryMap[offset + sizeof(T)]));
   T v10 = *(reinterpret_cast<T*> (&m_memoryMap[offset2]));
   T v11 = *(reinterpret_cast<T*> (&m_memoryMap[offset2 + sizeof(T)]));
   if (m_swapper)
   {
      m_swapper->swap(v00);
      m_swapper->swap(v01);
      m_swapper->swap(v10);
      m_swapper->swap(v11);
   }
   double p00 = v00;
   double p01 = v01;
   double p10 = v10;
   double p11 = v11;
   
   double xt0 = xi - x0;
   double yt0 = yi - y0;
   double xt1 = 1-xt0;
   double yt1 = 1-yt0;
   
   double w00 = xt1*yt1;
   double w01 = xt0*yt1;
   double w10 = xt1*yt0;
   double w11 = xt0*yt0;
   
   //***
   // Test for null posts and set the corresponding weights to 0:
   //***
   if (p00 == theNullHeightValue)
      w00 = 0.0;
   if (p01 == theNullHeightValue)
      w01 = 0.0;
   if (p10 == theNullHeightValue)
      w10 = 0.0;
   if (p11 == theNullHeightValue)
      w11 = 0.0;
   
#if 0 /* Serious debug only... */
   cout << "\np00:  " << p00
   << "\np01:  " << p01
   << "\np10:  " << p10
   << "\np11:  " << p11
   << "\nw00:  " << w00
   << "\nw01:  " << w01
   << "\nw10:  " << w10
   << "\nw11:  " << w11
   << endl;
#endif
   
   double sum_weights = w00 + w01 + w10 + w11;
   
   if (sum_weights)
   {
      return (p00*w00 + p01*w01 + p10*w10 + p11*w11) / sum_weights;
   }
   
   return rspf::nan();
}

double rspfSrtmHandler::getPostValue(const rspfIpt& /* gridPt */) const
{
   rspfNotify(rspfNotifyLevel_WARN)
      << "rspfSrtmHandler::getPostValue(const rspfIpt& gridPt):  NEED TO IMPLEMENT TO NEW INTERFACE\n";
   
   return theNullHeightValue;
#if 0
   static const char MODULE[] = "rspfSrtmHandler::getPostValue";
   
   if(!theFileStr.valid()) return rspf::nan();
   
   // Do some error checking.
   if ( gridPt.x < 0.0 || gridPt.y < 0.0 ||
        gridPt.x > (theNumberOfSamples  - 1) ||
        gridPt.y > (theNumberOfLines - 1) )
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

   std::streampos offset = gridPt.y * theSrtmRecordSizeInBytes + gridPt.x * 2;

   // Put the file pointer at the start of the first elevation post.
   m_fileStr.seekg(offset, std::ios::beg);

   rspf_sint16 p;

   // Get the post.
   m_fileStr.read((char*)&p, 2);
   if (theSwapper)
   {
      theSwapper->swap(p);
   }
   return p;
#endif
}

rspfIpt rspfSrtmHandler::getSizeOfElevCell() const
{
   return rspfIpt(m_numberOfSamples, m_numberOfLines);
}

const rspfSrtmHandler&
rspfSrtmHandler::operator=(const rspfSrtmHandler& rhs)
{
   m_supportData = rhs.m_supportData;
   m_streamOpen = rhs.m_streamOpen,
   m_numberOfLines = rhs.m_numberOfLines;
   m_numberOfSamples = rhs.m_numberOfSamples;
   m_srtmRecordSizeInBytes = rhs.m_srtmRecordSizeInBytes;
   m_latSpacing = rhs.m_latSpacing;
   m_lonSpacing = rhs.m_lonSpacing;
   m_nwCornerPost = rhs.m_nwCornerPost;
   m_swapper = rhs.m_swapper?new rspfEndian:0;
   m_scalarType = rhs.m_scalarType;
   m_memoryMap = rhs.m_memoryMap;
   
   rspfElevCellHandler::operator = (rhs);
   return *this;
}

rspfSrtmHandler::rspfSrtmHandler(const rspfSrtmHandler& src)
:rspfElevCellHandler(src),
m_supportData(src.m_supportData),
m_streamOpen(src.m_streamOpen),
m_numberOfLines(src.m_numberOfLines),
m_numberOfSamples(src.m_numberOfSamples),
m_srtmRecordSizeInBytes(src.m_srtmRecordSizeInBytes),
m_latSpacing(src.m_latSpacing),
m_lonSpacing(src.m_lonSpacing),
m_nwCornerPost(src.m_nwCornerPost),
m_swapper(src.m_swapper?new rspfEndian:0),
m_scalarType(src.m_scalarType),
m_memoryMap(src.m_memoryMap)
{
   if(m_memoryMap.empty()&&src.isOpen())
   {
      m_fileStr.open(src.getFilename().c_str(),
                     std::ios::binary|std::ios::in);
   }
}

bool rspfSrtmHandler::isOpen()const
{
   if(!m_memoryMap.empty()) return true;
   
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_fileStrMutex);
   return m_streamOpen;
   // return (m_fileStr.is_open());
}


bool rspfSrtmHandler::open(const rspfFilename& file, bool memoryMapFlag)
{
   theFilename = file;
   clearErrorStatus();
   if (!m_supportData.setFilename(file, false) )
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      return false;
   }
   if(m_swapper) {delete m_swapper;m_swapper=0;}
   m_scalarType = m_supportData.getScalarType();
   // SRTM is stored in big endian.
   if ( rspf::byteOrder() == RSPF_LITTLE_ENDIAN)
   {
      m_swapper = new rspfEndian();
   }
   m_streamOpen = false;
   m_numberOfLines         = m_supportData.getNumberOfLines();
   m_numberOfSamples       = m_supportData.getNumberOfSamples();
   m_srtmRecordSizeInBytes = m_numberOfSamples * rspf::scalarSizeInBytes(m_scalarType);
   m_latSpacing            = m_supportData.getLatitudeSpacing();
   m_lonSpacing            = m_supportData.getLongitudeSpacing();
   m_nwCornerPost.lon      = m_supportData.getSouthwestLongitude();
   m_nwCornerPost.lat      = m_supportData.getSouthwestLatitude()+1.0;
   m_scalarType            = m_supportData.getScalarType();
   theGroundRect = rspfGrect(rspfGpt(m_nwCornerPost.lat,
                                       m_nwCornerPost.lon,
                                       0.0),
                              rspfGpt(m_nwCornerPost.lat-1.0,
                                       m_nwCornerPost.lon+1.0,
                                       0.0));
   theMeanSpacing = (m_latSpacing*rspfGpt().metersPerDegree().y);
   
   // Set the base class null height value.
   theNullHeightValue = -32768.0;

   m_fileStr.clear();
   m_fileStr.open(theFilename.c_str(), std::ios::in | std::ios::binary);
   if(!m_fileStr)
   {
      return false;
   }
   
   if(memoryMapFlag)
   {
      m_memoryMap.resize(theFilename.fileSize());
      m_fileStr.read((char*)&m_memoryMap.front(), (streamsize)m_memoryMap.size());
      m_fileStr.close();
   }
   m_streamOpen = true;
   // Capture the stream state for non-const is_open on old compiler.
   
   return m_streamOpen;
#if 0
   // theMinHeightAboveMSL;
   // theMaxHeightAboveMSL;
   if(theFileStr.valid())
   {
      if(!m_fileStr.fail())
      {
         return true;
      }
   }
   theFileStr = 0;

   
   theFileStr = rspfStreamFactoryRegistry::instance()->createNewIFStream(theFilename,
                                                                          std::ios::in | std::ios::binary);   

   if(theFileStr.valid())
   {
      return (!m_fileStr.fail());
   }
   return false;
#endif
}

void rspfSrtmHandler::close()
{
   m_fileStr.close();
   m_memoryMap.clear();
   m_streamOpen = false;
}
