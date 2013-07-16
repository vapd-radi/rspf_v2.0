//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Rpf support class
//
//********************************************************************
// $Id: rspfRpfLocationSection.cpp 20324 2011-12-06 22:25:23Z dburken $

#include <rspf/support_data/rspfRpfLocationSection.h>
#include <rspf/base/rspfCommon.h> /* rspf::byteOrder() */
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/support_data/rspfRpfComponentIdLut.h>
#include <istream>
#include <ostream>
#include <iterator>

static const rspfTrace traceDebug("rspfRpfLocationSection:debug");

std::ostream& operator<<(std::ostream& out, const rspfRpfComponentLocationRecord& data)
{
   return data.print(out);
}

rspfRpfComponentLocationRecord::rspfRpfComponentLocationRecord()
   : m_componentId(0),
     m_componentLength(0),
     m_componentLocation(0)
{
}

rspfRpfComponentLocationRecord::rspfRpfComponentLocationRecord(
   const rspfRpfComponentLocationRecord& record)
   : m_componentId(record.m_componentId),
     m_componentLength(record.m_componentLength),
     m_componentLocation(record.m_componentLocation)
{
}

const rspfRpfComponentLocationRecord& rspfRpfComponentLocationRecord::operator=(
   const rspfRpfComponentLocationRecord& rhs)
{
   if (this != &rhs)
   {
      m_componentId       = rhs.m_componentId;
      m_componentLength   = rhs.m_componentLength;
      m_componentLocation = rhs.m_componentLocation;
   }
   return *this;
}

std::ostream& rspfRpfComponentLocationRecord::print(
   std::ostream& out, const std::string& prefix) const
{
   out << prefix << "ComponentId:             "
       << m_componentId << "\n"
       << prefix << "ComponentIdString:       "
       << rspfRpfComponentIdLut::instance()->getEntryString(m_componentId) << "\n"
       << prefix << "ComponentLength:         "
       << m_componentLength   << "\n"
       << prefix << "ComponentLocation:       "
       << m_componentLocation << "\n";
   return out;
}

std::ostream& operator <<(std::ostream& out, const rspfRpfLocationSection &data)
{
   return data.print(out);
}

rspfErrorCode rspfRpfComponentLocationRecord::parseStream(
   std::istream& in, rspfByteOrder byteOrder)
{
   if(in)
   {
      in.read((char*)&m_componentId, 2);
      in.read((char*)&m_componentLength, 4);
      in.read((char*)&m_componentLocation, 4);

      if( rspf::byteOrder() != byteOrder)
      {
         // swap to native
         rspfEndian anEndian;
         anEndian.swap(m_componentId);
         anEndian.swap(m_componentLength);
         anEndian.swap(m_componentLocation);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return  rspfErrorCodes::RSPF_OK;
}

void rspfRpfComponentLocationRecord::writeStream(std::ostream& out)
{
   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      rspfEndian endian;
      endian.swap(m_componentId);
      endian.swap(m_componentLength);
      endian.swap(m_componentLocation);
   }

   out.write((char*)&m_componentId, 2);
   out.write((char*)&m_componentLength, 4);
   out.write((char*)&m_componentLocation, 4);

   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Swap back to native byte order.
      rspfEndian endian;
      endian.swap(m_componentId);
      endian.swap(m_componentLength);
      endian.swap(m_componentLocation);
   }
}

rspfRpfLocationSection::rspfRpfLocationSection()
{
   clearFields();
}

rspfErrorCode rspfRpfLocationSection::parseStream(std::istream& in,
                                                    rspfByteOrder byteOrder)
{
   rspfErrorCode result = rspfErrorCodes::RSPF_OK;
   
   if(in)
   {
      clearFields();
      
      in.read((char*)&m_locationSectionLength, 2);
      in.read((char*)&m_locationTableOffset, 4);
      in.read((char*)&m_numberOfComponentLocationRecords, 2);
      in.read((char*)&m_locationRecordLength, 2);
      in.read((char*)&m_componentAggregateLength, 4);

      if( rspf::byteOrder() != byteOrder )
      {
         rspfEndian anEndian;
         anEndian.swap(m_locationSectionLength);
         anEndian.swap(m_locationTableOffset);
         anEndian.swap(m_numberOfComponentLocationRecords);
         anEndian.swap(m_locationRecordLength);
         anEndian.swap(m_componentAggregateLength);
      }
      
      if(traceDebug())
      {
         print(rspfNotify(rspfNotifyLevel_DEBUG));
         rspfNotify(rspfNotifyLevel_DEBUG) << std::endl;
      }

      m_componentLocationList.resize(m_numberOfComponentLocationRecords);
      for(rspf_uint32 index = 0;
          (index < m_componentLocationList.size())&&
             (result == rspfErrorCodes::RSPF_OK);
          ++index)
      {        
         result = m_componentLocationList[index].parseStream(in, byteOrder);
      }
   }
   else
   {
      result = rspfErrorCodes::RSPF_ERROR;
   }

   return result;
}


void rspfRpfLocationSection::writeStream(std::ostream& out)
{
   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Always write in big endian.
      rspfEndian endian;
      endian.swap(m_locationSectionLength);
      endian.swap(m_locationTableOffset);
      endian.swap(m_numberOfComponentLocationRecords);
      endian.swap(m_locationRecordLength);
      endian.swap(m_componentAggregateLength);
   }
   
   out.write((char*)&m_locationSectionLength, 2);
   out.write((char*)&m_locationTableOffset, 4);
   out.write((char*)&m_numberOfComponentLocationRecords, 2);
   out.write((char*)&m_locationRecordLength, 2);
   out.write((char*)&m_componentAggregateLength, 4);

   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Swap back to native byte order.
      rspfEndian endian;
      endian.swap(m_locationSectionLength);
      endian.swap(m_locationTableOffset);
      endian.swap(m_numberOfComponentLocationRecords);
      endian.swap(m_locationRecordLength);
      endian.swap(m_componentAggregateLength);
   }
   
   for(rspf_uint32 i = 0; i < m_componentLocationList.size(); ++i)
   {
      m_componentLocationList[i].writeStream(out);
   }
}

std::ostream& rspfRpfLocationSection::print(
   std::ostream& out, const std::string& prefix) const
{
   out << prefix << "LocationSectionLength:            "
       << m_locationSectionLength << "\n"
       << prefix << "LocationTableOffset:              "
       << m_locationTableOffset << "\n"
       << prefix << "NumberOfComponentLocationRecords: "
       << m_numberOfComponentLocationRecords << "\n"
       << prefix << "LocationRecordLength:             "
       << m_locationRecordLength << "\n"
       << prefix << "ComponentAggregateLength:         "
       << m_componentAggregateLength << "\n";
   
   if(m_numberOfComponentLocationRecords > 0)
   {
      std::vector<rspfRpfComponentLocationRecord>::const_iterator i =
         m_componentLocationList.begin();
      while (i != m_componentLocationList.end())
      {
         (*i).print(out, prefix);
         ++i;
      }
   }
   return out;
}

bool rspfRpfLocationSection::hasComponent(rspfRpfComponentId componentId)const
{
   rspfRpfComponentLocationRecord result;
   
   return getComponent(componentId, result);
}

bool rspfRpfLocationSection::getComponent(rspfRpfComponentId componentId,
                                           rspfRpfComponentLocationRecord &result)const
{
   std::vector<rspfRpfComponentLocationRecord>::const_iterator component =
      m_componentLocationList.begin();
   
   while(component != m_componentLocationList.end())
   {
      if((*component).m_componentId == static_cast<unsigned short>(componentId))
      {
         result = *component;

         return true;
      }      
      ++component;
   }
   
   return false;
}

void rspfRpfLocationSection::addComponentRecord(const rspfRpfComponentLocationRecord& record)
{
   m_componentLocationList.push_back(record);
}

void rspfRpfLocationSection::setLocationSectionLength(rspf_uint16 length)
{
   m_locationSectionLength = length;
}

void rspfRpfLocationSection::setLocationTableOffset(rspf_uint32 offset)
{
   m_locationTableOffset = offset;
}

void rspfRpfLocationSection::setNumberOfComponentLocationRecords(rspf_uint16 count)
{
   m_numberOfComponentLocationRecords = count;
}

void rspfRpfLocationSection::setLocationRecordLength(rspf_uint16 length)
{
   m_locationRecordLength = length;
}

void rspfRpfLocationSection::setComponentAggregateLength(rspf_uint32 length)
{
   m_componentAggregateLength = length;
}

void rspfRpfLocationSection::clearFields()
{
   m_locationSectionLength            = 0;
   m_locationTableOffset              = 0;
   m_numberOfComponentLocationRecords = 0;
   m_locationRecordLength             = 0;
   m_componentAggregateLength         = 0;

   m_componentLocationList.clear();
}

std::vector<rspfRpfComponentLocationRecord>& rspfRpfLocationSection::getLocationRecordList()
{
   return m_componentLocationList;
}
