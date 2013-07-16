//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfBoundaryRectRecord.cpp 17815 2010-08-03 13:23:14Z dburken $

#include <cstring> /* for memset/memcpy */
#include <iomanip>
#include <istream>
#include <ostream>
#include <rspf/support_data/rspfRpfBoundaryRectRecord.h>
#include <rspf/base/rspfCommon.h> /* rspf::byteOrder() */
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

std::ostream& operator <<(std::ostream& out, const rspfRpfBoundaryRectRecord& data)
{
   return data.print( out, std::string() );
}

rspfRpfBoundaryRectRecord::rspfRpfBoundaryRectRecord()
   :
   m_zone(),
   m_coverage(),
   m_numberOfFramesNorthSouth(0),
   m_numberOfFramesEastWest(0)
{
   clearFields();
}

rspfRpfBoundaryRectRecord::rspfRpfBoundaryRectRecord(const rspfRpfBoundaryRectRecord& obj)
   :
   m_zone(obj.m_zone),
   m_coverage(obj.m_coverage),
   m_numberOfFramesNorthSouth(obj.m_numberOfFramesNorthSouth),
   m_numberOfFramesEastWest(obj.m_numberOfFramesEastWest)
{
   memcpy(m_productDataType, obj.m_productDataType, 6);
   memcpy(m_compressionRatio, obj.m_compressionRatio, 6);
   memcpy(m_scale, obj.m_scale, 13);
   memcpy(m_producer, obj.m_producer, 6);
}

const rspfRpfBoundaryRectRecord& rspfRpfBoundaryRectRecord::operator=(
   const rspfRpfBoundaryRectRecord& rhs)
{
   if ( this != & rhs )
   {
      memcpy(m_productDataType, rhs.m_productDataType, 6);
      memcpy(m_compressionRatio, rhs.m_compressionRatio, 6);
      memcpy(m_scale, rhs.m_scale, 13);
      memcpy(m_producer, rhs.m_producer, 6);
      m_coverage = rhs.m_coverage;
      m_numberOfFramesNorthSouth = rhs.m_numberOfFramesNorthSouth;
      m_numberOfFramesEastWest = rhs.m_numberOfFramesEastWest;
   }
   return *this;
}

rspfRpfBoundaryRectRecord::~rspfRpfBoundaryRectRecord()
{
}

rspfErrorCode rspfRpfBoundaryRectRecord::parseStream(std::istream& in, rspfByteOrder byteOrder)
{
   if(in)
   {
      clearFields();
      
      in.read((char*)&m_productDataType, 5);
      in.read((char*)&m_compressionRatio, 5);
      in.read((char*)&m_scale, 12);
      in.read((char*)&m_zone, 1);
      in.read((char*)&m_producer, 5);

      rspfString tmpScale(m_scale);
      tmpScale.trim();
      if (!tmpScale.empty())
      {
        if (tmpScale.beforePos(2) != "1:")
        {
          if (tmpScale.afterPos(tmpScale.size()-2)!="M" && 
            tmpScale.afterPos(tmpScale.size()-2)!="K")
          {
            int tmpScaleValue = tmpScale.toInt();
            if (tmpScaleValue > 0)
            {
              tmpScale = rspfString("1:" + tmpScale);
              memset(m_scale, ' ', 12);
              memcpy(m_scale, tmpScale.c_str(), 12);
            }
          }
        }
      }
      
      m_coverage.parseStream(in, byteOrder);
      
      in.read((char*)&m_numberOfFramesNorthSouth, 4);
      in.read((char*)&m_numberOfFramesEastWest, 4);

      if( rspf::byteOrder() != byteOrder )
      {
         rspfEndian anEndian;
         anEndian.swap(m_numberOfFramesNorthSouth);
         anEndian.swap(m_numberOfFramesEastWest);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfBoundaryRectRecord::writeStream(std::ostream& out)
{
   rspfEndian anEndian;
   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Always write out big endian.
      anEndian.swap(m_numberOfFramesNorthSouth);
      anEndian.swap(m_numberOfFramesEastWest);
   }
   
   out.write((char*)&m_productDataType, 5);
   out.write((char*)&m_compressionRatio, 5);
   out.write((char*)&m_scale, 12);
   out.write((char*)&m_zone, 1);
   out.write((char*)&m_producer, 5);
   
   m_coverage.writeStream(out);
   
   out.write((char*)&m_numberOfFramesNorthSouth, 4);
   out.write((char*)&m_numberOfFramesEastWest, 4);
   
   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Swap back to native byte order.
      anEndian.swap(m_numberOfFramesNorthSouth);
      anEndian.swap(m_numberOfFramesEastWest);
   }
}

void rspfRpfBoundaryRectRecord::clearFields()
{
   memset(m_productDataType, ' ', 5);
   memset(m_compressionRatio, ' ', 5);
   memset(m_scale, ' ', 12);
   memset(m_producer, ' ', 5);
   m_numberOfFramesNorthSouth = 0;
   m_numberOfFramesEastWest = 0;
   m_coverage.clearFields();

   m_productDataType[5] = '\0';
   m_compressionRatio[5] = '\0';
   m_scale[12] = '\0';
   m_producer[5] = '\0';
   m_zone = ' ';
}

void rspfRpfBoundaryRectRecord::setCoverage(const rspfRpfCoverageSection& coverage)
{
   m_coverage = coverage;
}

std::ostream& rspfRpfBoundaryRectRecord::print(std::ostream& out, rspfString prefix) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();
   
   out << std::setiosflags(std::ios_base::fixed)
       << std::setprecision(12)
       << prefix << "ProductDataType: "
       << m_productDataType << "\n"
       << prefix << "CompressionRatio: "
       << m_compressionRatio << "\n"
       << prefix << "Scale: "
       << m_scale << "\n"
       << prefix << "Zone: "
       << m_zone << "\n"
       << prefix << "Producer: "
       << m_producer << "\n";
   
   m_coverage.print(out, prefix);
   
   out << prefix << "NumberOfFramesNorthSouth: "
       << m_numberOfFramesNorthSouth << "\n"
       << prefix << "NumberOfFramesEastWest: "
       << m_numberOfFramesEastWest
       << "\n";

   // Reset flags.
   out.setf(f);

   return out;
}
