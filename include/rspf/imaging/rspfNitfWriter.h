//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNitfWriter.h 18674 2011-01-11 16:24:12Z dburken $
#ifndef rspfNitfWriter_HEADER
#define rspfNitfWriter_HEADER

#include <iosfwd>
#include <rspf/imaging/rspfNitfWriterBase.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/base/rspfRgbLutDataObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_1.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_1.h>
#include <rspf/support_data/rspfNitfTextHeaderV2_0.h>
#include <rspf/support_data/rspfNitfTextHeaderV2_1.h>

class rspfProjection;

class RSPF_DLL rspfNitfWriter : public rspfNitfWriterBase
{
public:
   rspfNitfWriter(const rspfFilename& filename=rspfFilename(""),
                   rspfImageSource* inputSource = (rspfImageSource*)NULL);
   virtual ~rspfNitfWriter();
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
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;

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

   virtual bool addTextToNitf(std::string& inputText);

   /** @brief Gets the block size. */
   virtual void getTileSize(rspfIpt& size) const;

   /**
    * Sets the nitf output block size.  Must be divisible by 16.
    *
    * Note:  This is not called setBlockSize just because there was already
    * a virtual setTileSize.
    *
    * @param tileSize Block size.
    */
   virtual void setTileSize(const rspfIpt& tileSize);

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

   /** Currently disabled... */
   // virtual void addStandardTags();

   std::ofstream*                        m_outputStream;
   rspfRefPtr<rspfNitfFileHeaderV2_1>  m_fileHeader;
   rspfRefPtr<rspfNitfImageHeaderV2_1> m_imageHeader;
   rspfRefPtr<rspfNitfTextHeaderV2_1>  m_textHeader;
   std::string                           m_textEntry;
   rspfIpt                              m_blockSize;

TYPE_DATA   
};

#endif /* #ifndef rspfNitfWriter_HEADER */
