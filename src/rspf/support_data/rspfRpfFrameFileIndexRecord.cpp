//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfFrameFileIndexRecord.cpp 17501 2010-06-02 11:14:55Z dburken $

#include <cstring> /* for memset, memcpy */
#include <istream>
#include <ostream>

#include <rspf/support_data/rspfRpfFrameFileIndexRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

std::ostream& operator <<(std::ostream& out, const rspfRpfFrameFileIndexRecord& data)
{
   return data.print(out);
}

rspfRpfFrameFileIndexRecord::rspfRpfFrameFileIndexRecord()
   :
   m_boundaryRectRecordNumber(0),
   m_locationRowNumber(0),
   m_locationColumnNumber(0),
   m_pathnameRecordOffset(0)
{
   clearFields();
}

rspfRpfFrameFileIndexRecord::rspfRpfFrameFileIndexRecord(const rspfRpfFrameFileIndexRecord& obj)
   :
   m_boundaryRectRecordNumber(obj.m_boundaryRectRecordNumber),
   m_locationRowNumber(obj.m_locationRowNumber),
   m_locationColumnNumber(obj.m_locationColumnNumber),
   m_pathnameRecordOffset(obj.m_pathnameRecordOffset),
   m_securityClassification(obj.m_securityClassification)
{
   memcpy(m_filename, obj.m_filename, 13);
   memcpy(m_geographicLocation, obj.m_geographicLocation, 7);
   memcpy(m_fileSecurityCountryCode, obj.m_fileSecurityCountryCode, 3);
   memcpy(m_fileSecurityReleaseMarking, obj.m_fileSecurityReleaseMarking, 3);
}

const rspfRpfFrameFileIndexRecord& rspfRpfFrameFileIndexRecord::operator=(
   const rspfRpfFrameFileIndexRecord& rhs)
{
   if ( this != &rhs )
   {
      m_boundaryRectRecordNumber = rhs.m_boundaryRectRecordNumber;
      m_locationRowNumber = rhs.m_locationRowNumber;
      m_locationColumnNumber = rhs.m_locationColumnNumber;
      m_pathnameRecordOffset = rhs.m_pathnameRecordOffset;
      m_securityClassification = rhs.m_securityClassification;
      
      memcpy(m_filename, rhs.m_filename, 13);
      memcpy(m_geographicLocation, rhs.m_geographicLocation, 7);
      memcpy(m_fileSecurityCountryCode, rhs.m_fileSecurityCountryCode, 3);
      memcpy(m_fileSecurityReleaseMarking, rhs.m_fileSecurityReleaseMarking, 3);
   }
   return *this;
}

rspfErrorCode rspfRpfFrameFileIndexRecord::parseStream(
   std::istream& in, rspfByteOrder byteOrder)
{
   if(in)
   {
      rspfEndian anEndian;

      clearFields();
            
      in.read((char*)&m_boundaryRectRecordNumber, 2);
      in.read((char*)&m_locationRowNumber, 2);
      in.read((char*)&m_locationColumnNumber, 2);
      in.read((char*)&m_pathnameRecordOffset, 4);
      in.read((char*)m_filename, 12);
      in.read((char*)m_geographicLocation, 6);
      in.read((char*)&m_securityClassification, 1);
      in.read((char*)m_fileSecurityCountryCode, 2);
      in.read((char*)m_fileSecurityReleaseMarking, 2);

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(m_boundaryRectRecordNumber);
         anEndian.swap(m_locationRowNumber);
         anEndian.swap(m_locationColumnNumber);
         anEndian.swap(m_pathnameRecordOffset);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfFrameFileIndexRecord::writeStream(std::ostream& out)
{
   rspfEndian anEndian;
   
   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Always write out big endian.
      anEndian.swap(m_boundaryRectRecordNumber);
      anEndian.swap(m_locationRowNumber);
      anEndian.swap(m_locationColumnNumber);
      anEndian.swap(m_pathnameRecordOffset);
   }
   
   out.write((char*)&m_boundaryRectRecordNumber, 2);
   out.write((char*)&m_locationRowNumber, 2);
   out.write((char*)&m_locationColumnNumber, 2);
   out.write((char*)&m_pathnameRecordOffset, 4);
   out.write((char*)m_filename, 12);
   out.write((char*)m_geographicLocation, 6);
   out.write((char*)&m_securityClassification, 1);
   out.write((char*)m_fileSecurityCountryCode, 2);
   out.write((char*)m_fileSecurityReleaseMarking, 2);
   
   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Swap back to native.
      anEndian.swap(m_boundaryRectRecordNumber);
      anEndian.swap(m_locationRowNumber);
      anEndian.swap(m_locationColumnNumber);
      anEndian.swap(m_pathnameRecordOffset);
   }
}

std::ostream& rspfRpfFrameFileIndexRecord::print(std::ostream& out)const
{
   out << "BoundaryRectRecordNumber:      " << m_boundaryRectRecordNumber
       << "\nLocationRowNumber:             " << m_locationRowNumber
       << "\nLocationColumnNumber:          " << m_locationColumnNumber
       << "\nPathnameRecordOffset:          " << m_pathnameRecordOffset
       << "\nFilename:                      " << m_filename
       << "\nGeographicLocation:            " << m_geographicLocation
       << "\nSecurityClassification:        " << m_securityClassification
       << "\nFileSecurityCountryCode:       " << m_fileSecurityCountryCode
       << "\nFileSecurityReleaseMarking:    " << m_fileSecurityReleaseMarking
       << std::endl;
   return out;
}

rspf_uint16  rspfRpfFrameFileIndexRecord::getBoundaryRecNumber()const
{
   return m_boundaryRectRecordNumber;
}

rspf_uint16  rspfRpfFrameFileIndexRecord::getLocationRowNumber()const
{
   return m_locationRowNumber;
}

rspf_uint16  rspfRpfFrameFileIndexRecord::getLocationColNumber()const
{
   return m_locationColumnNumber;
}

rspf_uint32  rspfRpfFrameFileIndexRecord::getPathnameRecordOffset()const
{
   return m_pathnameRecordOffset;
}

rspfFilename rspfRpfFrameFileIndexRecord::getFilename()const
{
   return rspfFilename(m_filename);
}

void rspfRpfFrameFileIndexRecord::setBoundaryRecNumber(rspf_uint16 entry)
{
   m_boundaryRectRecordNumber = entry;
}

void rspfRpfFrameFileIndexRecord::setPathnameRecordOffset(rspf_uint32 offset)
{
   m_pathnameRecordOffset = offset;
}

void rspfRpfFrameFileIndexRecord::clearFields()
{
   m_boundaryRectRecordNumber = 0;
   m_locationRowNumber = 0;
   m_locationColumnNumber= 0;
   m_pathnameRecordOffset = 0;
   memset(m_filename, ' ', 12);
   memset(m_geographicLocation, ' ', 6);
   m_securityClassification = ' ';
   memset(m_fileSecurityCountryCode, ' ', 2);
   memset(m_fileSecurityReleaseMarking, ' ', 2);

   m_filename[12] = '\0';
   m_geographicLocation[6] = '\0';
   m_fileSecurityCountryCode[2] = '\0';
   m_fileSecurityReleaseMarking[2] = '\0';
}
