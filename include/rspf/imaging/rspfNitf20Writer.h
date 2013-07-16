//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNitfWriter.h 9256 2006-07-14 15:28:19Z dburken $
#ifndef rspfNitf20Writer_HEADER
#define rspfNitf20Writer_HEADER

#include <iosfwd>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/imaging/rspfNitfWriterBase.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/base/rspfRgbLutDataObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_0.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_0.h>

class rspfProjection;

class RSPF_DLL rspfNitf20Writer : public rspfNitfWriterBase
{
public:
   rspfNitf20Writer(const rspfFilename& filename=rspfFilename(""),
                   rspfImageSource* inputSource = (rspfImageSource*)NULL);
   virtual ~rspfNitf20Writer();
   virtual bool isOpen()const;
   virtual bool open();
   virtual void close();
   
   /**
    * void getImageTypeList(std::vector<rspfString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * This writer has the following types:
    * nitf_block_band_separate
    * nitf_block_band_sequential
    *
    * @param imageTypeList stl::vector<rspfString> list to append to.
    */   
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(
      const rspfString& name)const;

   /**
    * @param propertyNames Array to populate with property names.
    *
    * @note The following names are handled:
    *  file_header
    *  image_header
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   void addRegisteredTag(rspfRefPtr<rspfNitfRegisteredTag> registeredTag);

   /**
    * Saves the state of the writer to kwl with prefix then calls
    * base class rspfImageFileWriter::saveState
    *
    * @param kwl Keyword list to save to.
    *
    * @param prefix Usually something like: "object2."

    * @return true on success, false on failure.
    */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0) const;

   /**
    * Initializes the state of the writer from kwl with prefix then calls
    * base class rspfImageFileWriter::loadState
    *
    * @param kwl Keyword list to initialize from.
    *
    * @param prefix Usually something like: "object2."

    * @return true on success, false on failure.
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

protected:
   
   /**
    * @return true on success false on error.
    */
   virtual bool writeFile();
   
   /**
    * write out block band separate
    * 
    * @return true on success and false on error
    */
   virtual bool writeBlockBandSeparate();
   
   /**
    * Outputs in band sequential format.  Band 1 is followed by band
    * 2, ... etc.
    */
   virtual bool writeBlockBandSequential();

   void addTags();

   std::ofstream* theOutputStream;
   
   rspfRefPtr<rspfNitfFileHeaderV2_0>  theFileHeader;
   rspfRefPtr<rspfNitfImageHeaderV2_0> theImageHeader;

   /** If true user wants to set RPC00B tag. (DEFAULT = false) */
   bool theEnableRpcbTagFlag;

   /**
    * If true user wants to set BLOCKA tag. (DEFAULT = true)
    * Currently only valid for map projected images.
    */
   bool theEnableBlockaTagFlag;

   /**
    *
    * If true this will enable searching the input connnection for another NITF handler and 
    * bring the fields to this writers output fields and will maintin as many field values as possible
    *
    */
   bool theCopyFieldsFlag;
   
TYPE_DATA   
};

#endif /* #ifndef rspfNitfWriter_HEADER */
