//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: This class extends the stl's string class.
//
//********************************************************************
// $Id: rspfRpfHeader.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <cstring>
#include <iostream>
#include <fstream>

#include <rspf/support_data/rspfRpfHeader.h>
#include <rspf/base/rspfCommon.h> /* rspf::byteOrder() */
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/support_data/rspfNitfCommon.h>
#include <rspf/support_data/rspfRpfLocationSection.h>
#include <rspf/support_data/rspfRpfCoverageSection.h>
#include <rspf/support_data/rspfRpfBoundaryRectSectionSubheader.h>
#include <rspf/support_data/rspfRpfBoundaryRectTable.h>
#include <rspf/support_data/rspfRpfFrameFileIndexSectionSubheader.h>
#include <rspf/support_data/rspfRpfFrameFileIndexSubsection.h>
#include <rspf/support_data/rspfRpfImageDescriptionSubheader.h>
#include <rspf/support_data/rspfRpfMaskSubsection.h>
#include <rspf/support_data/rspfRpfAttributeSectionSubheader.h>
#include <rspf/support_data/rspfRpfImageDisplayParameterSubheader.h>
#include <rspf/support_data/rspfRpfCompressionSectionSubheader.h>
#include <rspf/support_data/rspfRpfCompressionSection.h>
#include <rspf/support_data/rspfRpfColorGrayscaleSubheader.h>

#include <rspf/base/rspfTrace.h>

 // Static trace for debugging
static rspfTrace traceDebug("rspfRpfHeader:debug");

// Keywords:
static const rspfString HEADER_SECTION_LENGTH_KW    = "HeaderSectionLength";
static const rspfString NEW_REP_UP_INDICATOR_KW     = "NewRepUpIndicator";
static const rspfString GOV_SPEC_NUMBER_KW          = "GovSpecNumber";
static const rspfString GOV_SPEC_DATE_KW            = "GovSpecDate";
static const rspfString SECURITY_CLASSIFICATION_KW  = "SecurityClassification";
static const rspfString COUNTRY_CODE_KW             = "CountryCode";
static const rspfString SECURITY_RELEASE_MARKING_KW = "SecurityReleaseMarking";


rspfRpfHeader::rspfRpfHeader()
   :
   rspfNitfRegisteredTag(std::string("RPFHDR"), 48),
   m_littleBigEndianIndicator(0x00),
   m_headerSectionLength(48),
   m_fileName(),
   m_newRepUpIndicator(0),
   m_govSpecNumber(),
   m_govSpecDate(),
   m_securityClassification(),
   m_countryCode(),
   m_securityReleaseMarking(),
   m_locSectionLoc(0),
   m_locationSection(new rspfRpfLocationSection)
{
   memset(m_fileName, ' ' , 12);
   memset(m_govSpecNumber, ' ', 15);
   memset(m_govSpecDate, ' ', 8);
   memset(m_securityClassification, ' ', 1);
   memset(m_countryCode, ' ', 2);
   memset(m_securityReleaseMarking, ' ', 2);
   
   m_fileName[12] = '\0';
   m_govSpecNumber[15] = '\0';
   m_govSpecDate[8] = '\0';
   m_securityClassification[1] = '\0';
   m_countryCode[2] = '\0';
   m_securityReleaseMarking[2] = '\0';
}

rspfRpfHeader::rspfRpfHeader(const rspfRpfHeader& obj)
   :
   rspfNitfRegisteredTag(std::string("RPFHDR"), 48),
   m_littleBigEndianIndicator(obj.m_littleBigEndianIndicator),
   m_headerSectionLength(obj.m_headerSectionLength),
   m_fileName(),
   m_newRepUpIndicator(obj.m_newRepUpIndicator),
   m_govSpecNumber(),
   m_govSpecDate(),
   m_securityClassification(),
   m_countryCode(),
   m_securityReleaseMarking(),
   m_locSectionLoc(obj.m_locSectionLoc),
   m_locationSection( new rspfRpfLocationSection( *(obj.m_locationSection) ) )
{
   memcpy(m_fileName, obj.m_fileName, 13);
   memcpy(m_govSpecNumber, obj.m_govSpecNumber, 16);
   memcpy(m_govSpecDate, obj.m_govSpecDate, 9);
   memcpy(m_securityClassification, obj.m_securityClassification, 2);
   memcpy(m_countryCode, obj.m_countryCode, 3);
   memcpy(m_securityReleaseMarking, obj.m_securityReleaseMarking, 3);
}

const rspfRpfHeader& rspfRpfHeader::operator=(const rspfRpfHeader& rhs)
{
   if ( this != &rhs )
   {
      m_littleBigEndianIndicator = rhs.m_littleBigEndianIndicator;
      m_headerSectionLength = rhs.m_headerSectionLength;
      memcpy(m_fileName, rhs.m_fileName, 13);
      m_newRepUpIndicator = rhs.m_newRepUpIndicator;
      memcpy(m_govSpecNumber, rhs.m_govSpecNumber, 16);
      memcpy(m_govSpecDate, rhs.m_govSpecDate, 9);   
      memcpy(m_securityClassification, rhs.m_securityClassification, 2);
      memcpy(m_countryCode, rhs.m_countryCode, 3);
      memcpy(m_securityReleaseMarking, rhs.m_securityReleaseMarking, 3);
      m_locSectionLoc = rhs.m_locSectionLoc;

      m_locationSection = new rspfRpfLocationSection( *(rhs.m_locationSection) );
   }
   return *this;
}

rspfRpfHeader::~rspfRpfHeader()
{
   if(m_locationSection)
   {
      delete m_locationSection;
      m_locationSection = 0;
   }
}

void rspfRpfHeader::parseStream(std::istream& in)
{
   if(in)
   {
      in.read((char*)&m_littleBigEndianIndicator, 1);

      in.read((char*)&m_headerSectionLength, 2);
      in.read((char*)m_fileName, 12);
      in.read((char*)&m_newRepUpIndicator, 1);
      in.read((char*)m_govSpecNumber, 15);
      in.read((char*)m_govSpecDate, 8);
      in.read((char*)m_securityClassification, 1);
      in.read((char*)m_countryCode, 2);
      in.read((char*)m_securityReleaseMarking, 2);
      in.read((char*)&m_locSectionLoc, 4);
      
      m_fileName[12] = '\0';
      m_govSpecNumber[15] = '\0';
      m_govSpecDate[8] = '\0';
      m_securityClassification[1] = '\0';
      m_countryCode[2] = '\0';
      m_securityReleaseMarking[2] = '\0';
      
      //---
      // From spec:  MIL-PRF-89038CARDG m_littleBigEndianIndicator shall
      // be 0x00 for all data denoting big endian storage.  We will test
      // anyway just in case...
      //---
      rspfByteOrder dataByteOrder = getByteOrder();

      if( rspf::byteOrder() != dataByteOrder )
      {
         rspfEndian anEndian;
         anEndian.swap(m_headerSectionLength);
         anEndian.swap(m_locSectionLoc);
      }

      std::streamoff saveGet = in.tellg();
      in.seekg(m_locSectionLoc, ios::beg);
      m_locationSection->parseStream(in, dataByteOrder);
      in.seekg(saveGet, ios::beg);
   }
}

void rspfRpfHeader::writeStream(std::ostream& out)
{
   // Always write in big endian.
   if (m_littleBigEndianIndicator != 0x00)
   {
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "rspfRpfHeader::writeStream writing in big endian even though"
         << " the m_littleBigEndianIndicator is set to little endian."
         << std::endl;
      m_littleBigEndianIndicator = 0x00;
   }
   
   rspfByteOrder dataByteOrder = getByteOrder();

   if( rspf::byteOrder() != dataByteOrder )
   {
      rspfEndian anEndian;
      anEndian.swap(m_headerSectionLength);
      anEndian.swap(m_locSectionLoc);
   }

   out.write((char*)&m_littleBigEndianIndicator, 1);
   
   out.write((char*)&m_headerSectionLength, 2);
   out.write((char*)m_fileName, 12);
   out.write((char*)&m_newRepUpIndicator, 1);
   out.write((char*)m_govSpecNumber, 15);
   out.write((char*)m_govSpecDate, 8);
   out.write((char*)m_securityClassification, 1);
   out.write((char*)m_countryCode, 2);
   out.write((char*)m_securityReleaseMarking, 2);
   out.write((char*)&m_locSectionLoc, 4);

   if( rspf::byteOrder() != dataByteOrder )
   {
      // Must swap things back or we will seek to a bad location.
      rspfEndian anEndian;
      anEndian.swap(m_headerSectionLength);
      anEndian.swap(m_locSectionLoc);
   }

   if (m_locSectionLoc) // May or may not be set.
   {
      std::streampos pos = out.tellp();
      out.seekp(m_locSectionLoc, ios::beg);
      m_locationSection->writeStream(out);
      out.seekp(pos);
   }
}

std::ostream& rspfRpfHeader::print(std::ostream& out, const std::string& prefix) const
{
   out << prefix << "byte_order:               "
       << (m_littleBigEndianIndicator==0x00?"big_endian\n":"little_endian\n")
       << prefix << HEADER_SECTION_LENGTH_KW << ":      "
       << m_headerSectionLength << "\n"
       << prefix << rspfKeywordNames::FILENAME_KW << ":                 "
       << m_fileName << "\n"
       << prefix << NEW_REP_UP_INDICATOR_KW << ":        "
       << int(m_newRepUpIndicator) << "\n"       
       << prefix << GOV_SPEC_NUMBER_KW << ":            "
       << m_govSpecNumber << "\n"
       << prefix << GOV_SPEC_DATE_KW << ":              "
       << m_govSpecDate << "\n"
       << prefix << SECURITY_CLASSIFICATION_KW << ":   "
       << m_securityClassification << "\n"
       << prefix << COUNTRY_CODE_KW << ":              "
       << m_countryCode   << "\n"           
       << prefix << SECURITY_RELEASE_MARKING_KW << ":   "
       << m_securityReleaseMarking << "\n";

   if ( traceDebug() )
   {
      out << prefix << "LocSectionLoc:            "
          << m_locSectionLoc << "\n";
      if (m_locationSection)
      {
         m_locationSection->print(out, prefix);
      }
   }

   return out;
}

rspfByteOrder rspfRpfHeader::getByteOrder()const
{
   return ( (m_littleBigEndianIndicator==0x00) ? RSPF_BIG_ENDIAN : RSPF_LITTLE_ENDIAN);
}

const rspfRpfLocationSection* rspfRpfHeader::getLocationSection() const
{
   return m_locationSection;
}

rspfRpfLocationSection* rspfRpfHeader::getLocationSection()
{
   return m_locationSection;
}

bool rspfRpfHeader::hasComponent(rspfRpfComponentId componentId)const
{
   if(m_locationSection)
   {
      return m_locationSection->hasComponent(componentId);
   }
   
   return false;
}

rspfString rspfRpfHeader::getSecurityClassification()const
{
   return m_securityClassification;
}

rspfRpfCoverageSection* rspfRpfHeader::getNewCoverageSection(std::istream& in)const
{
   rspfRpfCoverageSection* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_COVERAGE_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfCoverageSection;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            result->parseStream(in, getByteOrder());
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   return result;
}

rspfRpfMaskSubsection*  rspfRpfHeader::getNewMaskSubsection(std::istream& in)const
{
   rspfRpfMaskSubsection* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_MASK_SUBSECTION,
                                          component))
      {
         result = new rspfRpfMaskSubsection;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;   
}


rspfRpfAttributeSectionSubheader* rspfRpfHeader::getNewAttributeSectionSubheader(std::istream& in)const
{
   rspfRpfAttributeSectionSubheader* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_ATTRIBUTE_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfAttributeSectionSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;
}

rspfRpfColorGrayscaleSubheader* rspfRpfHeader::getNewColorGrayscaleSubheader(std::istream& in)const
{
   rspfRpfColorGrayscaleSubheader* result = 0;
   
   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_COLOR_GRAYSCALE_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfColorGrayscaleSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }      
   }

   return result;
}

rspfRpfCompressionSection* rspfRpfHeader::getNewCompressionSection(std::istream& in)const
{   
   rspfRpfCompressionSection* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_COMPRESSION_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfCompressionSection;
         
         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;   
}

rspfRpfCompressionSectionSubheader* rspfRpfHeader::getNewCompressionSectionSubheader(std::istream& in)const
{
   rspfRpfCompressionSectionSubheader* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_COMPRESSION_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfCompressionSectionSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;   
}

   

rspfRpfImageDisplayParameterSubheader* rspfRpfHeader::getNewImageDisplayParameterSubheader(std::istream& in)const
{
   rspfRpfImageDisplayParameterSubheader* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_IMAGE_DISPLAY_PARAMETERS_SUBHEADER,
                                          component))
      {
         result = new rspfRpfImageDisplayParameterSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;
}

rspfRpfImageDescriptionSubheader* rspfRpfHeader::getNewImageDescriptionSubheader(std::istream& in)const
{
   rspfRpfImageDescriptionSubheader* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_IMAGE_DESCRIPTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfImageDescriptionSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;
}


rspfRpfBoundaryRectTable* rspfRpfHeader::getNewBoundaryRectTable(std::istream &in)const
{
   rspfRpfBoundaryRectTable* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      rspfRpfBoundaryRectSectionSubheader* tempSubheader = getNewBoundaryRectSectSubheader(in);

      if(tempSubheader)
      {
         if(m_locationSection->getComponent(RSPF_RPF_BOUNDARY_RECT_TABLE,
                                             component))
         {
            result = new rspfRpfBoundaryRectTable;
            
            result->setNumberOfEntries(tempSubheader->getNumberOfEntries());
            in.seekg(component.m_componentLocation, ios::beg);
            if(in)
            {
               if(result->parseStream(in, getByteOrder()) !=
                  rspfErrorCodes::RSPF_OK)
               {
                  delete result;
                  result = 0;
               }
            }
            else
            {
               delete result;
               result = 0;
            }
         }

         delete tempSubheader;
         tempSubheader = 0;
      }
   }   

   return result;
}

rspfRpfBoundaryRectSectionSubheader* rspfRpfHeader::getNewBoundaryRectSectSubheader(std::istream &in)const
{
   rspfRpfBoundaryRectSectionSubheader* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;
      
      if(m_locationSection->getComponent(RSPF_RPF_BOUNDARY_RECT_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfBoundaryRectSectionSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }   

   return result;
   
}

rspfRpfFrameFileIndexSectionSubheader* rspfRpfHeader::getNewFrameFileIndexSectionSubheader(std::istream &in)const
{
   rspfRpfFrameFileIndexSectionSubheader* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;

      if(m_locationSection->getComponent(RSPF_RPF_FRAME_FILE_INDEX_SECTION_SUBHEADER,
                                          component))
      {
         result = new rspfRpfFrameFileIndexSectionSubheader;

         in.seekg(component.m_componentLocation, ios::beg);
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
   }   

   return result;   
}

rspfRpfFrameFileIndexSubsection* rspfRpfHeader::getNewFileIndexSubsection(std::istream& in)const
{
   rspfRpfFrameFileIndexSubsection* result = 0;

   if(in&&m_locationSection)
   {
      rspfRpfComponentLocationRecord component;
      rspfRpfFrameFileIndexSectionSubheader* tempSubheader = getNewFrameFileIndexSectionSubheader(in);

      if(m_locationSection->getComponent(RSPF_RPF_FRAME_FILE_INDEX_SUBSECTION,
                                          component))
      {
         result = new rspfRpfFrameFileIndexSubsection;

         result->setNumberOfFileIndexRecords(tempSubheader->getNumberOfIndexRecords());
         result->setNumberOfPathnames(tempSubheader->getNumberOfPathnameRecords());
         in.seekg(component.m_componentLocation, ios::beg);
         if(in)
         {
            if(result->parseStream(in, getByteOrder()) !=
               rspfErrorCodes::RSPF_OK)
            {
               delete result;
               result = 0;
            }
         }
         else
         {
            delete result;
            result = 0;
         }
      }
      if(tempSubheader)
      {
         delete tempSubheader;
         tempSubheader = 0;
      }
   }   

   return result;      
}

rspfRpfCompressionSection* rspfRpfHeader::getNewCompressionSection(const rspfFilename& file)const
{
   ifstream in(file.c_str(), ios::in|ios::binary);

   return getNewCompressionSection(in);
}

rspfRpfCoverageSection* rspfRpfHeader::getNewCoverageSection(const rspfFilename& file)const
{
   ifstream in(file.c_str(), ios::in|ios::binary);
   
   return getNewCoverageSection(in);
}

rspfRpfBoundaryRectTable* rspfRpfHeader::getNewBoundaryRectTable(const rspfFilename &file)const
{
   ifstream in(file.c_str(), ios::in|ios::binary);
   
   return getNewBoundaryRectTable(in);
}

rspfRpfBoundaryRectSectionSubheader* rspfRpfHeader::getNewBoundaryRectSectSubheader(const rspfFilename &file)const
{
   ifstream in(file.c_str(), ios::in|ios::binary);

   return getNewBoundaryRectSectSubheader(in);
}

rspfRpfFrameFileIndexSectionSubheader* rspfRpfHeader::getNewFrameFileIndexSectionSubheader(const rspfFilename &file)const
{
   ifstream in(file.c_str(), ios::in|ios::binary);

   return getNewFrameFileIndexSectionSubheader(in);
}

rspfRpfFrameFileIndexSubsection* rspfRpfHeader::getNewFileIndexSubsection(const rspfFilename& file)const
{
   ifstream in(file.c_str(), ios::in|ios::binary);

   return getNewFileIndexSubsection(in);
}

rspfString rspfRpfHeader::getDate()const
{
   return m_govSpecDate;
}

rspf_uint32 rspfRpfHeader::getLocationSectionLocation() const
{
   return m_locSectionLoc;
}

void rspfRpfHeader::setFilename(const rspfString& file)
{
   rspfNitfCommon::setField(m_fileName, file, 12);
}

void rspfRpfHeader::setNewRepUpIndicator(const rspfString& s)
{
   if (s.size())
   {
      // Range check maybe??? (drb)
      m_newRepUpIndicator = static_cast<rspf_uint8>(*s.begin());
   }
}

void rspfRpfHeader::setGovSpecNumber(const rspfString& s)
{
   rspfNitfCommon::setField(m_govSpecNumber, s, 15); 
}

void rspfRpfHeader::setGovSpecDate(const rspfString& s)
{
   rspfNitfCommon::setField(m_govSpecDate, s, 8); 
}

void rspfRpfHeader::setSecurityClassification(const rspfString& s)
{
   rspfNitfCommon::setField(m_securityClassification, s, 1);
}

void rspfRpfHeader::setCountryCode(const rspfString& s)
{
   rspfNitfCommon::setField(m_countryCode, s, 2); 
}

void rspfRpfHeader::setSecurityReleaseMarking(const rspfString& s)
{
   rspfNitfCommon::setField(m_securityReleaseMarking, s, 2);
}

void rspfRpfHeader::setLocationSectionPos(std::streamoff off)
{
   m_locSectionLoc = static_cast<rspf_uint32>(off);
}

bool rspfRpfHeader::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   const char* lookup = 0;
   rspfString s;

   lookup = kwl.find(prefix, NEW_REP_UP_INDICATOR_KW);
   if (lookup)
   {
      s = lookup;
      setNewRepUpIndicator(s);
   }

   lookup = kwl.find(prefix, GOV_SPEC_NUMBER_KW);
   if (lookup)
   {
      s = lookup;
      setGovSpecNumber(s);
   }

   lookup = kwl.find(prefix, GOV_SPEC_DATE_KW);
   if (lookup)
   {
      s = lookup;
      setGovSpecDate(s);
   }

   lookup = kwl.find(prefix, SECURITY_CLASSIFICATION_KW);
   if (lookup)
   {
      s = lookup;
      setSecurityClassification(s);
   }

   lookup = kwl.find(prefix, COUNTRY_CODE_KW);
   if (lookup)
   {
      s = lookup;
      setCountryCode(s);
   }

   lookup = kwl.find(prefix, SECURITY_RELEASE_MARKING_KW);
   if (lookup)
   {
      s = lookup;
      setSecurityReleaseMarking(s);
   }

   return true;
}
