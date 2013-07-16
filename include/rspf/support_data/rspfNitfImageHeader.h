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
// $Id: rspfNitfImageHeader.h 18413 2010-11-11 19:56:22Z gpotts $
#ifndef rspfNitfImageHeader_HEADER
#define rspfNitfImageHeader_HEADER

#include <vector>

#include <rspf/base/rspfNBandLutDataObject.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfPropertyInterface.h>
#include <rspf/support_data/rspfNitfTagInformation.h>
#include <rspf/support_data/rspfNitfCompressionHeader.h>
#include <rspf/support_data/rspfNitfImageBand.h>

class rspfString;
class rspfDpt;
class rspfIpt;
class rspfIrect;

class RSPFDLLEXPORT rspfNitfImageHeader : public rspfObject,
                                            public rspfPropertyInterface
{
public:
   rspfNitfImageHeader();
   virtual ~rspfNitfImageHeader();

   virtual bool getTagInformation(rspfNitfTagInformation& tagInfo,
                                  rspf_uint32 idx) const;
   
   virtual bool getTagInformation(rspfNitfTagInformation& tagInfo,
                                  const rspfString& tagName) const;

   rspfRefPtr<rspfNitfRegisteredTag> getTagData(const rspfString& tagName);
   
   const rspfRefPtr<rspfNitfRegisteredTag> getTagData(
      const rspfString& tagName) const;
   
   virtual rspf_uint32 getNumberOfTags()const;

   virtual void addTag(const rspfNitfTagInformation& tag);
   virtual void removeTag(const rspfString& tagName);
   
   virtual void parseStream(std::istream &in)= 0;
   virtual void writeStream(std::ostream &out)= 0;

   virtual bool isCompressed()const=0;
   virtual bool isEncrypted()const=0;

   virtual rspfString getCompressionCode()const=0;
   virtual rspfString getCompressionRateCode()const=0;
   virtual rspf_uint32 getDisplayLevel()const=0;
   virtual rspf_uint32 getAttachmentLevel()const=0;
   
   virtual rspf_int32 getNumberOfRows()const=0;
   virtual rspf_int32 getNumberOfBands()const=0;
   virtual rspf_int32 getNumberOfCols()const=0;
   virtual rspf_int32 getNumberOfBlocksPerRow()const=0;
   virtual rspf_int32 getNumberOfBlocksPerCol()const=0;
   virtual rspf_int32 getNumberOfPixelsPerBlockHoriz()const=0;
   virtual rspf_int32 getNumberOfPixelsPerBlockVert()const=0;
   virtual rspfString getImageId()const=0;
   virtual rspfString getTitle()const=0;
   virtual rspf_int32 getActualBitsPerPixelPerBand()const=0;
   virtual rspf_int32 getBitsPerPixelPerBand()const=0;
   virtual rspfIrect  getImageRect()const=0;
   /**
    * Blocks might not be the same size as the significant rows and cols used
    * by the getImageRect.
    */
   virtual rspfIrect   getBlockImageRect()const=0;
   virtual rspfString  getIMode()const=0;
   virtual rspf_uint64 getDataLocation()const=0;

   /** @return The IDATIM field unparsed. */
   virtual rspfString  getImageDateAndTime() const=0;
   
   virtual rspfString  getAcquisitionDateMonthDayYear(rspf_uint8 separationChar='-')const=0;
   virtual rspfString  getSecurityClassification()const=0;
   virtual rspfString  getCategory()const=0;
   virtual rspfString  getImageSource()const=0;
   virtual rspfString  getRepresentation()const=0;
   virtual rspfString  getCoordinateSystem()const=0;
   virtual rspfString  getGeographicLocation()const=0;
   virtual rspfString  getPixelValueType()const=0;

   /** @return IMAG field */
   virtual rspfString  getImageMagnification()const=0;

   /**
    * @brief Gets the ILOC field as an rspfIpt.
    * @param loc Initialized with the field.
    * @note: Do not confuse with "data location".
    */
   virtual void getImageLocation(rspfIpt& loc)const=0;

   /**
    * @brief Convenience method to get the decimation factor as a double from
    * the string returned be the getImageMagnification() method.
    *
    * @param result Initialized with the decimation factor as a double.
    * Will return an rspf::nan() if something bad happens like the string is
    * empty so callers should check result with rspf::isnan(result).
    */
   virtual void getDecimationFactor(rspf_float64& result) const;
   
   virtual bool hasBlockMaskRecords()const=0;
   virtual bool hasPadPixelMaskRecords()const=0;
   virtual bool hasTransparentCode()const=0;
   virtual rspf_uint32 getTransparentCode()const=0;
   virtual rspf_uint32 getBlockMaskRecordOffset(rspf_uint32 blockNumber,
                                                 rspf_uint32 bandNumber)const=0;
   
   virtual rspf_uint32 getPadPixelMaskRecordOffset(rspf_uint32 blockNumber,
                                                    rspf_uint32 bandNumber)const=0;
   virtual const rspfRefPtr<rspfNitfCompressionHeader> getCompressionHeader()const=0;
   virtual const rspfRefPtr<rspfNitfImageBand> getBandInformation(rspf_uint32 idx)const=0;
   bool hasLut()const;
   rspfRefPtr<rspfNBandLutDataObject> createLut(rspf_uint32 bandIdx)const;
   virtual bool isSameAs(const rspfNitfImageHeader* hdr) const;
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual rspf_uint32 getTotalTagLength()const;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    */
   virtual std::ostream& printTags(
      std::ostream& out, const std::string& prefix=std::string()) const;
   
   /**
    * @brief Populates keyword list with metadata.
    * expample:  country_code: US
    * 
    * @param kwl Keywordlist to populate with metadata.
    *
    * @param prefix Optional prefix to put in front of key, like: "image1.".
    */
   virtual void getMetadata(rspfKeywordlist& kwl,
                            const char* prefix=0) const;

   /**
    * @brief pure virtual print method that outputs a key/value type format
    * adding prefix to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const = 0;

   virtual bool saveState(rspfKeywordlist& kwl, const rspfString& prefix="")const;
protected:

   /**
    * Method to check tie point to see if it will be truncated in field
    * IGEOLO which has only arc second resolution.
    * 
    * Spits out warning if tie does not evenly divide by an arc second.
    *
    * @param tie Tie point to check.
    */
   void checkForGeographicTiePointTruncation(const rspfDpt& tie) const;
   
   std::vector<rspfNitfTagInformation> theTagList;

TYPE_DATA
};

#endif
