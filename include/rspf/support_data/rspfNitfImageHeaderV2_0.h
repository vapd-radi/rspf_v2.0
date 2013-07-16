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
// $Id: rspfNitfImageHeaderV2_0.h 18413 2010-11-11 19:56:22Z gpotts $
#ifndef rspfNitfImageHeaderV2_0_HEADER
#define rspfNitfImageHeaderV2_0_HEADER
#include <rspf/support_data/rspfNitfImageHeaderV2_X.h>
#include <rspf/support_data/rspfNitfImageBandV2_0.h>
#include <iterator>

class RSPFDLLEXPORT rspfNitfImageHeaderV2_0 : public rspfNitfImageHeaderV2_X
{
public:
   rspfNitfImageHeaderV2_0();
   virtual ~rspfNitfImageHeaderV2_0();

   virtual void parseStream(std::istream &in);
   virtual void writeStream(std::ostream &out);
   
   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   virtual bool        isCompressed()const;
   virtual bool        isEncrypted()const;

   virtual rspf_int32  getNumberOfBands()const;
   virtual rspf_int32  getNumberOfRows()const;
   virtual rspf_int32  getNumberOfCols()const;
   virtual rspf_int32  getNumberOfBlocksPerRow()const;
   virtual rspf_int32  getNumberOfBlocksPerCol()const;
   virtual rspf_int32  getNumberOfPixelsPerBlockHoriz()const;
   virtual rspf_int32  getNumberOfPixelsPerBlockVert()const;
   virtual rspfString  getImageId()const;
   virtual rspf_int32  getBitsPerPixelPerBand()const;
   virtual rspf_int32  getActualBitsPerPixelPerBand()const;
   virtual rspfString  getIMode()const;
   virtual rspfString  getSecurityClassification()const;

   /** @return The IDATIM field unparsed. */
   virtual rspfString  getImageDateAndTime() const;
   virtual rspfString  getAcquisitionDateMonthDayYear(
      rspf_uint8 separationChar='-')const;
   virtual rspfString  getCategory()const;
   virtual rspfString  getImageSource()const;
   virtual rspfString  getRepresentation()const;
   virtual rspfString  getCoordinateSystem()const;

   virtual rspfString  getPixelValueType()const;
   virtual bool         hasBlockMaskRecords()const;
   virtual bool         hasPadPixelMaskRecords()const;
   virtual bool         hasTransparentCode()const;
   virtual rspf_uint32 getTransparentCode()const;
   virtual rspf_uint32 getBlockMaskRecordOffset(rspf_uint32 blockNumber,
                                                 rspf_uint32 bandNumber)const;
   virtual rspf_uint32 getPadPixelMaskRecordOffset(rspf_uint32 blockNumber,
                                                    rspf_uint32 bandNumber)const;
   
   virtual const rspfRefPtr<rspfNitfCompressionHeader> getCompressionHeader()const;
   virtual const rspfRefPtr<rspfNitfImageBand> getBandInformation(rspf_uint32 idx)const;

   virtual void setNumberOfBands(rspf_uint32 nbands);
   virtual void setBandInfo(rspf_uint32 idx,
                            const rspfNitfImageBandV2_0& info);
   virtual void setNumberOfRows(rspf_uint32 rows);
   virtual void setNumberOfCols(rspf_uint32 cols);
   virtual bool saveState(rspfKeywordlist& kwl, const rspfString& prefix="")const;
   
   static const rspfString ISCODE_KW;
   static const rspfString ISCTLH_KW;
   static const rspfString ISREL_KW;
   static const rspfString ISCAUT_KW;
   static const rspfString CTLN_KW;
   static const rspfString ISDWNG_KW;
   static const rspfString ISDEVT_KW;
   
TYPE_DATA
private:
   //
   
   void clearFields();

   /*!
    * FIELD ISCODE:
    * is a 40 byte field
    */
   char theCodewords[41];

   /*!
    * FIELD ISCTLH:
    * is a 40 byte field
    */
   char theControlAndHandling[41];

   /*!
    * FIELD ISREL:
    * Is a 40 byte field.
    */
   char theReleasingInstructions[41];

   /*!
    * FIELD ISCAUT:
    * Is a 20 byte field
    */
   char theClassificationAuthority[21];

   /*!
    * FIELD CTLN:
    */
   char theSecurityControlNumber[21];

   /*!
    * FIELD ISDWNG:
    * Is a 6 byte field with form:
    *
    *  YYMMDD.  Year month day
    *
    * 999999 indicates that he Originating
    *        agencies determination is required
    *        (OADR)
    * 999998 downgrade event will specify at what
    *        point and time the declassification
    *        or downgrading is to take place.
    */
   char theSecurityDowngrade[7];

   /*!
    * FIELD ISDEVT:
    * This is a conditional field and is present
    * if the Security downgrade is 999998.
    */
   char theDowngradingEvent[41];

   /**
    * Will contain also the field NBANDS and we will have to recreate the prefix for
    * 
    * Container for repetitive fields:
    * IREPBAND
    * ISUBCAT
    * IFC
    * IMFLT
    * NLUTS
    * NELUT1
    * LUTD
    */ 
   std::vector<rspfRefPtr<rspfNitfImageBandV2_0> > theImageBands;
};

#endif
