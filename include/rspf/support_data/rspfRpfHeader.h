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
// $Id: rspfRpfHeader.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef rspfRpfHeader_HEADER
#define rspfRpfHeader_HEADER 1

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/support_data/rspfNitfRegisteredTag.h>
#include <rspf/support_data/rspfRpfConstants.h>

class rspfRpfLocationSection;
class rspfRpfCoverageSection;
class rspfRpfBoundaryRectTable;
class rspfRpfBoundaryRectSectionSubheader;
class rspfRpfFrameFileIndexSectionSubheader;
class rspfRpfFrameFileIndexSubsection;
class rspfRpfColorTableIndexSectionSubheader;
class rspfRpfImageDescriptionSubheader;
class rspfRpfMaskSubsection;
class rspfRpfAttributeSectionSubheader;
class rspfRpfImageDisplayParameterSubheader;
class rspfRpfCompressionSection;
class rspfRpfCompressionSectionSubheader;
class rspfRpfColorGrayscaleSubheader;

class rspfRpfHeader : public rspfNitfRegisteredTag
{
public:

   /** default constructor */
   rspfRpfHeader();

   /** copy constructor */
   rspfRpfHeader(const rspfRpfHeader& obj);

   /** assignment operator */
   const rspfRpfHeader& operator=(const rspfRpfHeader& rhs);

   virtual ~rspfRpfHeader();

   /**
    * @brief Parse method.
    *
    * @param in Stream to parse.
    */
   virtual void parseStream(std::istream& in);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   virtual void writeStream(std::ostream& out);
   
   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;

   virtual rspfByteOrder getByteOrder()const;
   
   const rspfRpfLocationSection* getLocationSection() const;

   rspfRpfLocationSection* getLocationSection();

   bool hasComponent(rspfRpfComponentId componentId)const;
   rspfString getSecurityClassification()const;
   
   /*!
    * This will return a new coverage section.  It will return NULL if
    * one does not exist.
    */
   rspfRpfCoverageSection*
      getNewCoverageSection(const rspfFilename& file)const;
   rspfRpfCoverageSection*
      getNewCoverageSection(std::istream& in)const;

   rspfRpfColorGrayscaleSubheader*
      getNewColorGrayscaleSubheader(std::istream& in)const;
   rspfRpfCompressionSection*
      getNewCompressionSection(std::istream& in)const;
   rspfRpfCompressionSectionSubheader*
      getNewCompressionSectionSubheader(std::istream& in)const;
   rspfRpfImageDisplayParameterSubheader*
      getNewImageDisplayParameterSubheader(std::istream& in)const;
   rspfRpfAttributeSectionSubheader*
      getNewAttributeSectionSubheader(std::istream& in)const;
   rspfRpfImageDescriptionSubheader*
      getNewImageDescriptionSubheader(std::istream& in)const;
   rspfRpfMaskSubsection*
      getNewMaskSubsection(std::istream& in)const;
   rspfRpfBoundaryRectSectionSubheader*
      getNewBoundaryRectSectSubheader(std::istream& in)const;
   rspfRpfBoundaryRectTable*
      getNewBoundaryRectTable(std::istream& in)const;
   rspfRpfFrameFileIndexSectionSubheader*
      getNewFrameFileIndexSectionSubheader(std::istream& in)const;
   rspfRpfFrameFileIndexSubsection*
      getNewFileIndexSubsection(std::istream& in)const;
   rspfRpfColorTableIndexSectionSubheader*
      getNewColorTableIndexSectionSubheader(std::istream& in)const;

   rspfRpfCompressionSection*
      getNewCompressionSection(const rspfFilename& file)const;
   rspfRpfBoundaryRectSectionSubheader*
      getNewBoundaryRectSectSubheader(const rspfFilename& file)const;
   rspfRpfBoundaryRectTable*
      getNewBoundaryRectTable(const rspfFilename& file)const;
   rspfRpfFrameFileIndexSectionSubheader*
      getNewFrameFileIndexSectionSubheader(const rspfFilename& file)const;
   rspfRpfFrameFileIndexSubsection*
      getNewFileIndexSubsection(const rspfFilename& file)const;
   rspfRpfColorTableIndexSectionSubheader*
      getNewColorTableIndexSectionSubheader(const rspfFilename& file)const;

   rspfString getDate()const;

   /** @brief returns the byte position of the location section. */
   rspf_uint32 getLocationSectionLocation() const;
   
   void setFilename(const rspfString& file);
   void setNewRepUpIndicator(const rspfString& s);
   void setGovSpecNumber(const rspfString& s);
   void setGovSpecDate(const rspfString& s);
   void setSecurityClassification(const rspfString& s);
   void setCountryCode(const rspfString& s);
   void setSecurityReleaseMarking(const rspfString& s);
   void setLocationSectionPos(std::streamoff off);

   /*!
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

private:

   /** 0x00 = big, 0xff = little */
   rspf_uint8  m_littleBigEndianIndicator; // 1 byte bool
   rspf_uint16 m_headerSectionLength;      // 2 bytes uint
   char         m_fileName[13];             // 12 byte ascii
   rspf_uint8  m_newRepUpIndicator;        // 1 byte unsigned int
   char         m_govSpecNumber[16];        // 15 byte ascii
   char         m_govSpecDate[9];           // 8 byte ascii
   /**
    * 1 byte field that can have the values
    *
    * U   Unclassified
    * R   Restricted
    * C   Confidential
    * S   Secret
    * T   Top Secret
    */ 
   char         m_securityClassification[2]; // 1 byte ascii
   char         m_countryCode[3];            // 2 byte ascii
   char         m_securityReleaseMarking[3]; // 2 byte ascii
   rspf_uint32 m_locSectionLoc;             // 4 byte unsigned int
   
   rspfRpfLocationSection* m_locationSection;
};

#endif
 
