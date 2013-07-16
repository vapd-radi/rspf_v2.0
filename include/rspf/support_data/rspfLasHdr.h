//----------------------------------------------------------------------------
//
// File: rspfLasHdr.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Container class for LAS LIDAR format public header block.
//
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfLasHdr_HEADER
#define rspfLasHdr_HEADER 1

#include <rspf/base/rspfConstants.h>

#include <istream>
#include <ostream>

class rspfKeywordlist;

class RSPF_DLL rspfLasHdr
{
public:
   
   /* @brief default constructor */
   rspfLasHdr();

   /* @brief copy constructor */
   rspfLasHdr(const rspfLasHdr& hdr);

   /* @brief assignment operator= */
   const rspfLasHdr& operator=(const rspfLasHdr& copy_this);

   /* destructor */
   ~rspfLasHdr();

   /**
    * @brief Checks for LASF in first four bytes.
    * @return true if first four bytes are LASF.
    */
   bool checkSignature(std::istream& in) const;   

   /**
    * @brief Method to initialize from input stream.
    * @param in Stream to read from.
    * @note The first four bytes are not read in so stream should be
    * positioned at the 5th byte.
    */
   void readStream(std::istream& in);

   /** @brief Writes header to stream. */
   void writeStream(std::ostream& out);

   /**
    * @brief print method.
    * @return std::ostream&
    */
   std::ostream& print(std::ostream& out) const;

   /** @brief Convenience operator<< method. */
   friend RSPF_DLL std::ostream& operator<<(std::ostream& out, const rspfLasHdr& hdr);
   
   void getKeywordlist(rspfKeywordlist& kwl) const;

   /** @return True if bit 0 of m_globalEncoding is set, false if not. */
   bool getGpsTimeTypeBit() const;

   /** @return True if bit 1 of m_globalEncoding is set, false if not. */
   bool getWaveforDataPacketsInternalBit() const;

   /** @return True if bit 2 of m_globalEncoding is set, false if not. */
   bool getWaveforDataPacketsExternalBit() const;
   
   /** @return True if bit 3 of m_globalEncoding is set, false if not. */
   bool getReturnsSyntheticallyGeneratedBit() const;
   
   /**
    * @return The project ID GUID data, e.g.: 55b44da7-7c23-4f86-a54ec39e8f1d1ea1
    */
   std::string getProjectIdGuid() const;

   /** @return The major and minor version, e.g.: 1.2 */
   std::string getVersion() const;

   /** @return The system identifier string. */
   std::string  getSystemIndentifier() const;

   /** @return The generating software string. */
   std::string  getGeneratingSoftware() const;

   /** @return Size of this header in bytes. */
   rspf_uint16 getHeaderSize() const;

   /** @return Offset to point data. */
   rspf_uint32 getOffsetToPointData() const;

   /** @return The number fo variable length records. */
   rspf_uint32 getNumberOfVlrs() const;

   /** @return Point data format ID */
   rspf_uint8 getPointDataFormatId() const;

   /** @return The number of total points. */
   rspf_uint32 getNumberOfPoints() const;

   /**
    * @brief Gets number of points for entry where entry is synonymous returns.
    * @return The number of points for entries 0 through 4.
    */
   rspf_uint32 getNumberOfPoints(rspf_uint32 entry) const;

   const rspf_float64& getScaleFactorX() const;
   const rspf_float64& getScaleFactorY() const;
   const rspf_float64& getScaleFactorZ() const;
   const rspf_float64& getOffsetX() const;
   const rspf_float64& getOffsetY() const;
   const rspf_float64& getOffsetZ() const;
   const rspf_float64& getMinX() const;
   const rspf_float64& getMinY() const;
   const rspf_float64& getMinZ() const;
   const rspf_float64& getMaxX() const;
   const rspf_float64& getMaxY() const;
   const rspf_float64& getMaxZ() const;
   
   
private:

   /** @brief Performs a swap if system byte order is not little endian. */
   void swap();

   char          m_fileSignature[4];
   rspf_uint16  m_fileSourceId;
   rspf_uint16  m_globalEncoding;
   rspf_uint32  m_projectIdGuidData1;
   rspf_uint16  m_projectIdGuidData2;
   rspf_uint16  m_projectIdGuidData3;
   rspf_uint8   m_projectIdGuidData4[8];
   rspf_uint8   m_versionMajor;
   rspf_uint8   m_versionMinor;
   char          m_systemIndentifier[32];
   char          m_generatingSoftware[32];
   rspf_uint16  m_fileCreationDay;
   rspf_uint16  m_fileCreateionYear;
   rspf_uint16  m_headerSize;
   rspf_uint32  m_offsetToPointData;
   rspf_uint32  m_numberOfVariableLengthRecords;
   rspf_uint8   m_pointDataFormatId;
   rspf_uint16  m_pointDataRecordLength;
   rspf_uint32  m_numberOfPointRecords;
   rspf_uint32  m_numberOfPointsReturn1;
   rspf_uint32  m_numberOfPointsReturn2;
   rspf_uint32  m_numberOfPointsReturn3;
   rspf_uint32  m_numberOfPointsReturn4;
   rspf_uint32  m_numberOfPointsReturn5;
   rspf_float64 m_xScaleFactor;
   rspf_float64 m_yScaleFactor;
   rspf_float64 m_zScaleFactor;
   rspf_float64 m_xOffset;
   rspf_float64 m_yOffset;
   rspf_float64 m_zOffset;
   rspf_float64 m_maxX;
   rspf_float64 m_minX;
   rspf_float64 m_maxY;
   rspf_float64 m_minY;
   rspf_float64 m_maxZ;
   rspf_float64 m_minZ;
   rspf_uint64  m_startOfWaveformData;
};



#endif /* End of "#ifndef rspfLasHdr_HEADER" */
