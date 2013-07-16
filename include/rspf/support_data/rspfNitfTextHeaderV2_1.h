//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#ifndef rspfNitfTextHeaderV2_1_HEADER
#define rspfNitfTextHeaderV2_1_HEADER
#include <rspf/support_data/rspfNitfTextHeader.h>

class RSPFDLLEXPORT rspfNitfTextHeaderV2_1 : public rspfNitfTextHeader
{
public:
   rspfNitfTextHeaderV2_1();
   virtual ~rspfNitfTextHeaderV2_1();
   
   virtual void parseStream(std::istream &in);
   virtual std::ostream& print(std::ostream &out)const;
   
   virtual void writeStream(std::ostream &out);

   void setSecurityClassification(const rspfString& value);
   void setSecurityClassificationSystem(const rspfString& value);
   void setCodewords(const rspfString& value);
   void setControlAndHandling(const rspfString& value);
   void setReleasingInstructions(const rspfString& value);
   void setDeclassificationType(const rspfString& value);
   
   static const rspfString TE_KW;
   static const rspfString TEXTID_KW;
   static const rspfString TXTALVL_KW;
   static const rspfString TXTDT_KW;
   static const rspfString TXTITL_KW;
   static const rspfString TSCLAS_KW;
   static const rspfString TSCLSY_KW;
   static const rspfString TSCODE_KW;
   static const rspfString TSCTLH_KW;
   static const rspfString TSREL_KW;
   static const rspfString TSDCTP_KW;
   static const rspfString TSDCDT_KW;
   static const rspfString TSDCXM_KW;
   static const rspfString TSDG_KW;
   static const rspfString TSDGDT_KW;
   static const rspfString TSCLTX_KW;
   static const rspfString TSCATP_KW;
   static const rspfString TSCAUT_KW;
   static const rspfString TSCRSN_KW;
   static const rspfString TSSRDT_KW;
   static const rspfString TSCTLN_KW;
   static const rspfString ENCRYP_KW;
   static const rspfString TXTFMT_KW;
   static const rspfString TXSHDL_KW;
   static const rspfString TXSOFL_KW;
   static const rspfString TXSHD_KW;

private:
   void clearFields();
   
   /*!
    * This is a required 2 byte fields and has
    * a value of TE.
    */
   char theFilePartType[3];
   
   /*!
    * This is a required 7 byte field.  Text Identifier
    */
   char theTextId[8];
   
   /*!
    * This is a required 3 byte field.
    */

   char theTextAttLevel[4];
   /*!
    * This is a required 14 byte field.  It has the
    * format of:
    *
    *  DDHHMMSSZMONYY
    *
    * DD   2 character day
    * HH   2 characters for the Hour
    * MM   2 characters for the minute
    * SS   2 characters for the seconds
    * Z    required
    * MON  3 characters of the month
    * YY   2 characters for the year
    */
   char theDataAndTime[15];

   /*!
    * is an optional 80 byte field
    */
   char theTextTitle[81];
   
   /*!
    * This is a required 1 byte field. Can have
    * of either:
    *
    * T   Top secret
    * S   Secret
    * C   Confidential
    * R   Restricted
    * U   Unclassified
    */
   char theTextSecurityClassification[2];
   
   /*!
    *  2 byte field Security Classification System
    */
   char theTextSecuritySystem[3];
   
   /*!
    *  11 byte field Text Codewords
    */
   char theTextCodewords[12];
   
   /*!
    *  2 byte field Text Control and Handling
    */
   char theTextControlAndHandling[3];
   
   /*!
    * This is an optional 20 byte field.
    */
   char theTextReleasingInstructions[21];
   
   /*!
    * optional 2 byte field Text Declassification Type
    */
   char theTextDeclassificationType[3];
   
   /*!
    * optional 8 byte field Text Declassification Date
    */
   char theTextDeclassificationDate[9];
   
   /*!
    * optional 4 byte field Text Declassification Exemption
    */
   char theTextDeclassificationExemption[5];

   /*!
    * optional 1 byte field Text Declassification Exemption
    */
   char theTextSecurityDowngrade[2];
   
   /*!
    * optional 8 byte field
    */
   char theTextSecurityDowngradeDate[9];
   
   /*!
    * 43 byte field providing additional information on text classification
    */
   char theTextClassificationText[44];
   
   /*!
    *	Optional 1 byte flag for type of classification athority
    */
   char theTextClassificationAthorityType[2];
   
   /*!
    *	Optional 40 byte flag for classification athority
    */
   char theTextClassificationAthority[41];
   
   /*!
    *	Optional 1 byte flag for classification athority
    */
   char theTextClassificationReason[2];
   
   /*!
    *	Optional 8 byte entry
    */
   char theTextSecuritySourceDate[9];

   /*!
    *	Optional 15 byte entry for security control number
    */
   char theTextSecurityControlNumber[16];
   
   /*!
    * This is the encription type and is a
    * required 1 byte field.  It can have values
    *
    * 0  Not encrypted
    * 1  Encrypted
    */
   char theTextEncyption[2];
   
   /*!
    * This is a required 3 byte field and will hold
    * either:
    *
    * MTF  indicates USMTF ( refer to JCS PUB 6-04
    *      for examples
    * STA  indicates NITF ASCII
    * OTH  indicates other or user defined
    */
   char theTextFormat[4];
   
   /*!
    * is a required 5 byute field.
    */
   char theExtSubheaderDataLength[6];
   
   /*!
    * is a conditional 3 byte field.  It exists if
    * theExtSubheaderDataLength is not 0
    */
   char theExtSubheaderOverflow[4];
   
   TYPE_DATA
};

#endif
