//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
// 
// Description:  This class gives access to the Data Record Description
//               of a DTED Level 1 file.
//
// Notes:  Each elevation is a true value referenced to meas sea level
//         (MSL) datum recorded to the nearest meter.  The horizontal
//         position is referenced to precise longitude-latitiude
//         locations in terms of the current World Geodetic System
//         (WGS84) determined for each file by reference to the origin
//         at the southwest corner.  The elevations are evenly spaced
//         in latitude and longitude at the interval designated in the
//         User Header Label (UHL) in South to North profile sequence.
//
//********************************************************************
// $Id: rspfDtedRecord.cpp 20611 2012-02-27 12:21:51Z gpotts $

#include <iostream>

#include <rspf/support_data/rspfDtedRecord.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfNotifyContext.h>

static const rspf_uint16 DATA_NULL_VALUE = 0xffff; // -32767
static const rspf_uint16 DATA_MIN_VALUE  = 0xfffe; // -32766
static const rspf_uint16 DATA_MAX_VALUE  = 0x7fff; // +32767
static const rspf_uint16 DATA_VALUE_MASK = 0x7fff; // 0111 1111 1111 1111
static const rspf_uint16 DATA_SIGN_MASK  = 0x8000; // 1000 0000 0000 0000
static const rspf_uint16 DATA_RECOGNITION_SENTINEL = 0xAA;  // 170

//***
// Offsets from start of data record (one record per longitude line):
//***
static const int BLOCK_COUNT_OFFSET = 2;
static const int LON_INDEX_OFFSET   = 4;
static const int LAT_INDEX_OFFSET   = 6;
static const int ELEV_DATA_OFFSET   = 8;
static const int RECORD_HDR_LENGTH  = 12;
static const int BYTES_PER_POINT    = 2;

//**************************************************************************
// CONSTRUCTOR
//**************************************************************************
rspfDtedRecord::rspfDtedRecord(std::istream& in,
                                 rspf_int32 offset,
                                 rspf_int32 num_points)
   :
      theRecSen("170"),
      theDataBlockCount(0),
      theLonCount(0),
      theLatCount(0),
      theCheckSum(0),
      thePoints(new rspf_int32[num_points]),
      thePointsData(new rspf_uint16[num_points]),
      theComputedCheckSum(0),
      theNumPoints(num_points),
      theStartOffset(offset),
      theStopOffset(offset + RECORD_HDR_LENGTH + (num_points*BYTES_PER_POINT))
{
   // Verify we are at a cell record by checking the Recognition Sentinel.
   rspf_uint8 buf[1];
   in.seekg(theStartOffset, std::ios::beg);
   in.read((char*)buf, 1);
 
#if 0
   printf("\nBuf: %02X", (int)buf[0]);  // Display in HEX
#endif
   
   if(buf[0] != DATA_RECOGNITION_SENTINEL)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedRecord::rspfDtedRecord: reading DTED's data record at: " << theStartOffset
                                          << std::endl;
      return;
   }

   // Valid data record, so let's process on.
   parse(in);

   //***
   // Note:  The validateCheckSum method works; however, our in-house
   //        dted has bad stored check sums even though the post are good.
   //        So this in temporarily shut off.
   //***
   // Verify Check Sum for uncorrupted elevation data.
   if(validateCheckSum(in) == false)
   {
      //***
      // Note:  The validateCheckSum method works; however, our in-house
      //        dted has bad stored check sums even though the posts are good.
      //        So this in temporarily shut off.
      //***
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedRecord::rspfDtedRecord:"
         << "\nInvalid checksum in data record at: "
         << theStartOffset
         << "\nParsed Check Sum = " << theCheckSum
         << "\nComputed Check Sum = " << theComputedCheckSum
         << "\nDTED Elevation File is considered corrupted."
         << std::endl;
      return;
   }
}

rspfDtedRecord::~rspfDtedRecord()
{
   if (thePoints)
   {
      delete [] thePoints;
      thePoints = NULL;
   }
   if (thePointsData)
   {
      delete [] thePointsData;
      thePointsData = NULL;
   }
}

//**************************************************************************
// parse()
//**************************************************************************
void rspfDtedRecord::parse(std::istream& in)
{
   // DTED is stored in big endian byte order so swap the bytes if needed.
   bool swap_bytes = rspf::byteOrder() == RSPF_LITTLE_ENDIAN ? true : false;
   
   rspf_sint16 s;
   
   // parse data block count
   in.seekg(theStartOffset + BLOCK_COUNT_OFFSET, std::ios::beg);
   in.read((char*)&s, 2);
   theDataBlockCount =
      (swap_bytes ? ( ( (s & 0x00ff) << 8) | ( (s & 0xff00) >> 8) ) : s);
   
   // parse lon count
   in.seekg(theStartOffset + LON_INDEX_OFFSET, std::ios::beg);
   in.read((char*)&s, 2);
   theLonCount =
      (swap_bytes ? ( ( (s & 0x00ff) << 8) | ( (s & 0xff00) >> 8) ) : s);
   
   // parse lat count
   in.seekg(theStartOffset + LAT_INDEX_OFFSET, std::ios::beg);
   in.read((char*)&s, 2);
   theLatCount =
      (swap_bytes ? ( ( (s & 0x00ff) << 8) | ( (s & 0xff00) >> 8) ) : s);
   
   // Parse all elevation points.
   in.seekg(theStartOffset + ELEV_DATA_OFFSET, std::ios::beg);
   for(int i = 0; i < theNumPoints; ++i)
   {
      in.read((char*)&s, 2);
      s = (swap_bytes ? ( ( (s & 0x00ff) << 8) | ( (s & 0xff00) >> 8) ) : s);
      if (s & DATA_SIGN_MASK)
      {
         s = (s & DATA_VALUE_MASK) * -1;
      }
      thePoints[i] = static_cast<rspf_int32>(s);
      thePointsData[i] = s;
   }
}

//**************************************************************************
// validateCheckSum()
//**************************************************************************
bool rspfDtedRecord::validateCheckSum(std::istream& in)
{
   // DTED is stored in big endian byte order so swap the bytes if needed.
   bool swap_bytes = rspf::byteOrder() == RSPF_LITTLE_ENDIAN ? true : false;
   
   // Compute the check sum.
   in.seekg(theStartOffset, std::ios::beg);
   theComputedCheckSum = 0;
   rspf_int32 bytesToRead = (theNumPoints * 2) + ELEV_DATA_OFFSET;
   int i = 0;
   
   for(i = 0; i < bytesToRead; i++)
   {
      rspf_uint8 c;
      in.read((char*)&c, 1);
      theComputedCheckSum += static_cast<rspf_uint32>(c);
   }   

   // Read the stored check sum and swap the byte if needed.
   in.read((char*)&theCheckSum, 4); 
   if (swap_bytes)
   {
      rspfEndian swapper;
      swapper.swap(theCheckSum);
   }
   
   // Compare computed and parsed checksums.
   if(theCheckSum != theComputedCheckSum)
   {
      return false;
   }
   return true;
}

rspfString rspfDtedRecord::recognitionSentinel() const
{
   return theRecSen;
}

rspf_int32 rspfDtedRecord::dataBlockCount() const
{
   return theDataBlockCount;
}

rspf_int32 rspfDtedRecord::lonCount() const
{
   return theLonCount;
}

rspf_int32 rspfDtedRecord::latCount() const
{
   return theLatCount;
}

rspf_uint32 rspfDtedRecord::checkSum() const
{
   return theCheckSum;
}

rspf_uint32 rspfDtedRecord::computedCheckSum() const
{
   return theComputedCheckSum;
}

rspf_int32 rspfDtedRecord::numPoints() const
{
   return theNumPoints;
}

rspf_int32 rspfDtedRecord::getPoint(rspf_int32 i) const
{
   return thePoints[i];
}

rspf_uint16 rspfDtedRecord::getPointData(rspf_int32 i) const
{
   return thePointsData[i];
}

rspf_int32*  rspfDtedRecord::points() const
{
   return thePoints;
}

rspf_uint16* rspfDtedRecord::pointsData() const
{
   return thePointsData;
}

rspf_int32 rspfDtedRecord::startOffset() const
{
   return theStartOffset;
}

rspf_int32 rspfDtedRecord::stopOffset() const
{
   return theStopOffset;
}

//**************************************************************************
// operator <<
//**************************************************************************
std::ostream& operator<<( std::ostream& os, const rspfDtedRecord& rec)
{
   os << "\nDTED Record:"
      << "\n-------------------------------"
      << "\n Recognition Sentinel: " << rec.theRecSen
      << "\nData Block Count:      " << rec.theDataBlockCount
      << "\nLon Count:             " << rec.theLonCount
      << "\nLat Count:             " << rec.theLatCount
      << "\nNum Points:            " << rec.theNumPoints
      << "\nParsed Check Sum:      " << rec.theCheckSum
      << "\nComputed Check Sum     " << rec.theComputedCheckSum
      << "\nStart Offset:          " << rec.theStartOffset
      << "\nStop Offset:           " << rec.theStopOffset
      << std::endl;
   int i = 0;

   for(i = 0; i < rec.theNumPoints; i++)
   {
      os << "\nPoint[" << i << "]: " << rec.thePoints[i];
   }

   os << std::endl;

   return os;
}
