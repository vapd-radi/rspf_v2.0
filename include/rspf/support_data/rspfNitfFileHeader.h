//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfFileHeader.h 18413 2010-11-11 19:56:22Z gpotts $
#ifndef rspfNitfFileHeader_HEADER
#define rspfNitfFileHeader_HEADER

#include <iosfwd>
#include <vector>
#include <iterator>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfPropertyInterface.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/support_data/rspfNitfTagInformation.h>

class rspfNitfImageHeader;
class rspfNitfSymbolHeader;
class rspfNitfTextHeader;
class rspfNitfLabelHeader;
class rspfNitfDataExtensionSegment;
// These structures are just data holders and will
// not be used directly by outside users.  This is for internal use
//
class RSPF_DLL rspfNitfImageOffsetInformation : public rspfObject
{
public:
   virtual std::ostream& print(std::ostream& out) const;

   rspfNitfImageOffsetInformation(rspf_uint64 headerOffset,
                                   rspf_uint64 dataOffset)
      :theImageHeaderOffset(headerOffset),
       theImageDataOffset(dataOffset)
      {
      }
   
   rspf_uint64 theImageHeaderOffset;
   rspf_uint64 theImageDataOffset;
};


class RSPFDLLEXPORT rspfNitfSymbolOffsetInformation : public rspfObject
{
public:
   rspfNitfSymbolOffsetInformation(rspf_uint64 headerOffset,
                                    rspf_uint64 dataOffset)
      :theSymbolHeaderOffset(headerOffset),
       theSymbolDataOffset(dataOffset)
      {
      }
   
   rspf_uint64 theSymbolHeaderOffset;
   rspf_uint64 theSymbolDataOffset;
};

class RSPFDLLEXPORT rspfNitfLabelOffsetInformation : public rspfObject
{
public:
   rspfNitfLabelOffsetInformation(rspf_uint64 headerOffset,
                                   rspf_uint64 dataOffset)
      :theLabelHeaderOffset(headerOffset),
       theLabelDataOffset(dataOffset)
      {
      }
   
   rspf_uint64 theLabelHeaderOffset;
   rspf_uint64 theLabelDataOffset;
};

class RSPFDLLEXPORT rspfNitfTextOffsetInformation : public rspfObject
{
public:
   rspfNitfTextOffsetInformation(rspf_uint64 headerOffset,
                                  rspf_uint64 dataOffset)
      :theTextHeaderOffset(headerOffset),
       theTextDataOffset(dataOffset)
      {
      }
   
   rspf_uint64 theTextHeaderOffset;
   rspf_uint64 theTextDataOffset;
   
};

class RSPFDLLEXPORT rspfNitfDataExtSegOffsetInformation : public rspfObject
{
public:
   rspfNitfDataExtSegOffsetInformation(rspf_uint64 headerOffset,
                                        rspf_uint64 dataOffset)
      :theDataExtSegHeaderOffset(headerOffset),
       theDataExtSegDataOffset(dataOffset)
      {
      }
   
   rspf_uint64 theDataExtSegHeaderOffset;
   rspf_uint64 theDataExtSegDataOffset;   
};

class RSPF_DLL rspfNitfFileHeader : public rspfObject,
                                      public rspfPropertyInterface
{
public:
   friend class rspfNitfFile;

   rspfNitfFileHeader();
   virtual ~rspfNitfFileHeader();

   virtual void parseStream(std::istream &in)= 0;
   virtual void writeStream(std::ostream &out)=0;
   
   virtual bool isEncrypted()const=0;
   virtual rspf_int32 getNumberOfImages()const=0;
   virtual rspf_int32 getNumberOfLabels()const=0;
   virtual rspf_int32 getNumberOfSymbols()const=0;
   virtual rspf_int32 getNumberOfGraphics()const=0;
   virtual rspf_int32 getNumberOfTextSegments()const=0;
   virtual rspf_int32 getNumberOfDataExtSegments()const=0;
   virtual rspf_int32 getHeaderSize()const=0;
   
   void addTag(const rspfNitfTagInformation& tag);
   virtual void removeTag(const rspfString& tagName);
   virtual bool  getTagInformation(rspfNitfTagInformation& tag,
                                   int idx)const;

   virtual int getNumberOfTags()const;

   virtual bool getTag(rspfNitfTagInformation& tagInfo,
                       const rspfString& tagName)const;
   
   virtual rspf_int64 getFileSize()const=0;
   virtual const char* getVersion()const=0;
   virtual const char* getDateTime()const=0;
   virtual rspfDrect getImageRect()const=0;
   virtual rspfString getSecurityClassification()const=0;
   
   bool hasImages()const;
   bool hasSymbols()const;
   bool hasGraphics()const;
   bool hasText()const;
   bool hasLabels()const;
   bool hasDataExtSegments()const;

   virtual rspfNitfImageHeader* getNewImageHeader(rspf_uint32 imageNumber,
                                                   std::istream& in)const=0;
   virtual rspfNitfSymbolHeader* getNewSymbolHeader(rspf_uint32 symbolNumber,
                                                    std::istream& in)const=0;
   virtual rspfNitfLabelHeader* getNewLabelHeader(rspf_uint32 labelNumber,
                                                   std::istream& in)const=0;
   virtual rspfNitfTextHeader* getNewTextHeader(rspf_uint32 textNumber,
                                                std::istream& in)const=0;
   virtual rspfNitfDataExtensionSegment* getNewDataExtensionSegment(
      rspf_uint32 dataExtNumber, std::istream& in)const=0;
   
   virtual rspfNitfImageHeader*    allocateImageHeader()const=0;
   virtual rspfNitfSymbolHeader*   allocateSymbolHeader()const=0;
   virtual rspfNitfLabelHeader*    allocateLabelHeader()const=0;
   virtual rspfNitfTextHeader*     allocateTextHeader()const=0;
   virtual rspfNitfDataExtensionSegment *allocateDataExtSegment()const=0;

   virtual rspf_uint32 getTotalTagLength()const;

   /** @brief Sets file length (FL) field. */
   virtual void setFileLength(rspf_uint64 fileLength) = 0;

   /** @brief Sets header length (HL) field. */
   virtual void setHeaderLength(rspf_uint64 headerLength) = 0;

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
   /**
    * @brief print tags method that outputs a key/value type format adding
    * prefix to keys.
    */
   virtual std::ostream& printTags(
      std::ostream& out,
      const std::string& prefix=std::string()) const;
   
   virtual bool saveState(rspfKeywordlist& kwl, const rspfString& prefix="")const;
   
protected:
   std::vector<rspfNitfTagInformation> theTagList;
   
TYPE_DATA
};

#endif
