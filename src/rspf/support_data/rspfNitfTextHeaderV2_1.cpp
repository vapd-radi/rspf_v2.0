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

#include <rspf/support_data/rspfNitfTextHeaderV2_1.h>
#include <rspf/support_data/rspfNitfCommon.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfString.h>
#include <cstring> // for memset
#include <istream>

RTTI_DEF1(rspfNitfTextHeaderV2_1, "rspfNitfTextHeaderV2_1", rspfNitfTextHeader)
   

const rspfString rspfNitfTextHeaderV2_1::TE_KW = "te";
const rspfString rspfNitfTextHeaderV2_1::TEXTID_KW = "textid";
const rspfString rspfNitfTextHeaderV2_1::TXTALVL_KW = "txtalvl";
const rspfString rspfNitfTextHeaderV2_1::TXTDT_KW = "txtdt";
const rspfString rspfNitfTextHeaderV2_1::TXTITL_KW = "txtitl";
const rspfString rspfNitfTextHeaderV2_1::TSCLAS_KW = "tsclas";
const rspfString rspfNitfTextHeaderV2_1::TSCLSY_KW = "tsclsy";
const rspfString rspfNitfTextHeaderV2_1::TSCODE_KW = "tscode";
const rspfString rspfNitfTextHeaderV2_1::TSCTLH_KW = "tsctlh";
const rspfString rspfNitfTextHeaderV2_1::TSREL_KW = "tsrel";
const rspfString rspfNitfTextHeaderV2_1::TSDCTP_KW = "tsdctp";
const rspfString rspfNitfTextHeaderV2_1::TSDCDT_KW = "tsdcdt";
const rspfString rspfNitfTextHeaderV2_1::TSDCXM_KW = "tsdcxm";
const rspfString rspfNitfTextHeaderV2_1::TSDG_KW = "tsdg";
const rspfString rspfNitfTextHeaderV2_1::TSDGDT_KW = "tsdgdt";
const rspfString rspfNitfTextHeaderV2_1::TSCLTX_KW = "tscltx";
const rspfString rspfNitfTextHeaderV2_1::TSCATP_KW = "tscatp";
const rspfString rspfNitfTextHeaderV2_1::TSCAUT_KW = "tscaut";
const rspfString rspfNitfTextHeaderV2_1::TSCRSN_KW = "tscrsn";
const rspfString rspfNitfTextHeaderV2_1::TSSRDT_KW = "tssrdt";
const rspfString rspfNitfTextHeaderV2_1::TSCTLN_KW = "tsctln";
const rspfString rspfNitfTextHeaderV2_1::ENCRYP_KW = "encryp";
const rspfString rspfNitfTextHeaderV2_1::TXTFMT_KW = "txtfmt";
const rspfString rspfNitfTextHeaderV2_1::TXSHDL_KW = "txshdl";
const rspfString rspfNitfTextHeaderV2_1::TXSOFL_KW = "txsofl";
const rspfString rspfNitfTextHeaderV2_1::TXSHD_KW = "txshd";


rspfNitfTextHeaderV2_1::rspfNitfTextHeaderV2_1()
{
   clearFields();
}

rspfNitfTextHeaderV2_1::~rspfNitfTextHeaderV2_1()
{
}

void rspfNitfTextHeaderV2_1::parseStream(std::istream &in)
{
   if(in)
   {
      clearFields();
      
      in.read(theFilePartType, 2);
      in.read(theTextId, 7);
      in.read(theTextAttLevel, 3);
      in.read(theDataAndTime, 14);
      in.read(theTextTitle, 80);
      in.read(theTextSecurityClassification, 1);
      // If text class is T,S,C, or R, need to have TSCLSY populated with valid code
      in.read(theTextSecuritySystem, 2);
      in.read(theTextCodewords, 11);
      in.read(theTextControlAndHandling, 2);
      in.read(theTextReleasingInstructions, 20);
      in.read(theTextDeclassificationType, 2);
      in.read(theTextDeclassificationDate, 8);
      in.read(theTextDeclassificationExemption, 4);
      in.read(theTextSecurityDowngrade, 2);
      in.read(theTextSecurityDowngradeDate, 8);
      in.read(theTextClassificationText, 43);        // TSCLTX
      in.read(theTextClassificationAthorityType, 1); // TSCATP
      in.read(theTextClassificationAthority, 40);    // TSCAUT
      in.read(theTextClassificationReason, 1);       // TSCRSN
      in.read(theTextSecuritySourceDate, 8);         // TSSRDT
      in.read(theTextSecurityControlNumber, 15);     // TSCTLN

      in.read(theTextEncyption, 1);
      in.read(theTextFormat, 3);
      in.read(theExtSubheaderDataLength, 5);
      long dataLength = rspfString(theExtSubheaderDataLength).toLong();
      if(dataLength > 0)
      {
         in.read(theExtSubheaderOverflow, 3);

         // ignore the data for now
         in.ignore(dataLength - 3);
      }
   }
}

std::ostream& rspfNitfTextHeaderV2_1::print(std::ostream &out)const
{
   out << "theFilePartType:                     " << theFilePartType
       << "\ntheTextId:                         " << theTextId
       << "\ntheTextAttLevel:                   " << theTextAttLevel
       << "\ntheDataAndTime:                    " << theDataAndTime
       << "\ntheTextTitle:                      " << theTextTitle
       << "\ntheTextSecurityClassification:     " << theTextSecurityClassification
       << "\ntheTextSecuritySystem:             " << theTextSecuritySystem
       << "\ntheTextCodewords:                  " << theTextCodewords
       << "\ntheTextControlAndHandling:         " << theTextControlAndHandling
       << "\ntheTextReleasingInstructions:      " << theTextReleasingInstructions
       << "\ntheTextDeclassificationType:       " << theTextDeclassificationType
       << "\ntheTextDeclassificationDate:       " << theTextDeclassificationDate
       << "\ntheTextDeclassificationExemption:  " << theTextDeclassificationExemption
       << "\ntheTextSecurityDowngrade:          " << theTextSecurityDowngrade
       << "\ntheTextSecurityDowngradeDate:      " << theTextSecurityDowngradeDate
       << "\ntheTextClassificationText:         " << theTextClassificationText
       << "\ntheTextClassificationAthorityType: " << theTextClassificationAthorityType
       << "\ntheTextClassificationAthority:     " << theTextClassificationAthority
       << "\ntheTextClassificationReason:       " << theTextClassificationReason
       << "\ntheTextSecuritySourceDate:         " << theTextSecuritySourceDate
       << "\ntheTextSecurityControlNumber:      " << theTextSecurityControlNumber
       << "\ntheTextEncyption:                  " << theTextEncyption
       << "\ntheTextFormat:                     " << theTextFormat
       << "\ntheExtSubheaderDataLength:         " << theExtSubheaderDataLength
       << "\ntheExtSubheaderOverflow:           " << theExtSubheaderOverflow
       << std::endl;
   return out;
}

void rspfNitfTextHeaderV2_1::clearFields()
{
   memset(theFilePartType,' ', 2);
   memset(theTextId,' ', 7);
   memset(theTextAttLevel,' ', 3);
   memset(theDataAndTime,' ', 14);
   memset(theTextTitle,' ', 80);
   memset(theTextSecurityClassification, ' ',1);
   // If text class is T,S,C, or R, need to have TSCLSY populated with valid code
   memset(theTextSecuritySystem, ' ', 2);
   memset(theTextCodewords, ' ',11);
   memset(theTextControlAndHandling,' ', 2);
   memset(theTextReleasingInstructions,' ', 20);
   memset(theTextDeclassificationType,' ', 2);
   memset(theTextDeclassificationDate, ' ',8);
   memset(theTextDeclassificationExemption,' ', 4);
   memset(theTextSecurityDowngrade, ' ',2);
   memset(theTextSecurityDowngradeDate, ' ',8);
   memset(theTextClassificationText, ' ',43);        // TSCLTX
   memset(theTextClassificationAthorityType, ' ',1); // TSCATP
   memset(theTextClassificationAthority, ' ',40);    // TSCAUT
   memset(theTextClassificationReason, ' ',1);       //TSCRSN
   memset(theTextSecuritySourceDate,' ', 8);         //TSSRDT
   memset(theTextSecurityControlNumber, ' ',15);     // TSCTLN
   memset(theTextEncyption, ' ',1);
   memset(theTextFormat, ' ',3);
   memset(theExtSubheaderDataLength, ' ',5);
   memset(theExtSubheaderOverflow, ' ',3);

   theFilePartType[2] = '\0';
   theTextId[7] = '\0';
   theTextAttLevel[3] = '\0';
   theDataAndTime[14] = '\0';
   theTextTitle[80] = '\0';
   theTextSecurityClassification[1] = '\0';
   theTextSecuritySystem[2] = '\0';
   theTextCodewords[11] = '\0';
   theTextControlAndHandling[2] = '\0';
   theTextReleasingInstructions[20] = '\0';
   theTextDeclassificationType[2] = '\0';
   theTextDeclassificationDate[8] = '\0';
   theTextDeclassificationExemption[4] = '\0';
   theTextSecurityDowngrade[1] = '\0';
   theTextSecurityDowngradeDate[8] = '\0';;
   theTextClassificationText[43] = '\0';;			
   theTextClassificationAthorityType[1] = '\0';
   theTextClassificationAthority[40] = '\0';
   theTextClassificationReason[1] = '\0';
   theTextSecuritySourceDate[8] = '\0';
   theTextSecurityControlNumber[15] = '\0';
   theTextEncyption[1] = '\0';
   theTextFormat[3] = '\0';
   theExtSubheaderDataLength[5] = '\0';
   theExtSubheaderOverflow[3] = '\0';

}


void rspfNitfTextHeaderV2_1::writeStream(std::ostream &out)
{
   out.write(theFilePartType, 2);
   out.write(theTextId, 7);
   out.write(theTextAttLevel, 3);
   out.write(theDataAndTime, 14);
   out.write(theTextTitle, 80);
   out.write(theTextSecurityClassification, 1);
   // If text class is T,S,C, or R, need to have TSCLSY populated with valid code
   out.write(theTextSecuritySystem, 2);
   out.write(theTextCodewords, 11);
   out.write(theTextControlAndHandling, 2);
   out.write(theTextReleasingInstructions, 20);
   out.write(theTextDeclassificationType, 2);
   out.write(theTextDeclassificationDate, 8);
   out.write(theTextDeclassificationExemption, 4);
   out.write(theTextSecurityDowngrade, 2);
   out.write(theTextSecurityDowngradeDate, 8);
   out.write(theTextClassificationText, 43);        // TSCLTX
   out.write(theTextClassificationAthorityType, 1); // TSCATP
   out.write(theTextClassificationAthority, 40);    // TSCAUT
   out.write(theTextClassificationReason, 1);       // TSCRSN
   out.write(theTextSecuritySourceDate, 8);         // TSSRDT
   out.write(theTextSecurityControlNumber, 15);     // TSCTLN
   out.write(theTextEncyption, 1);
   out.write(theTextFormat, 3);
   out.write(theExtSubheaderDataLength, 5);
   long dataLength = rspfString(theExtSubheaderDataLength).toLong();
   if(dataLength > 0)
   {
      out.write(theExtSubheaderOverflow, 3);
   }
}


/*
void rspfNitfTextHeaderV2_1::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(ISCLSY_KW);
   propertyNames.push_back(ISCODE_KW);
   propertyNames.push_back(ISCTLH_KW);
   propertyNames.push_back(ISREL_KW);
   propertyNames.push_back(ISDCTP_KW);
   propertyNames.push_back(ISDCDT_KW);
   propertyNames.push_back(ISDCXM_KW);
   propertyNames.push_back(ISDG_KW);
   propertyNames.push_back(ISDGDT_KW);
   propertyNames.push_back(ISCLTX_KW);
   propertyNames.push_back(ISCATP_KW);
   propertyNames.push_back(ISCAUT_KW);
   propertyNames.push_back(ISCRSN_KW);
   propertyNames.push_back(ISSRDT_KW);
   propertyNames.push_back(ISCTLN_KW);
   propertyNames.push_back(XBANDS_KW);
}
*/

void rspfNitfTextHeaderV2_1::setSecurityClassification(const rspfString& value)
{
   rspfNitfCommon::setField(theTextSecurityClassification, value, 1);
}

void rspfNitfTextHeaderV2_1::setSecurityClassificationSystem(const rspfString& value)
{
   rspfNitfCommon::setField(theTextSecuritySystem, value, 2);
}

void rspfNitfTextHeaderV2_1::setCodewords(const rspfString& value)
{
   rspfNitfCommon::setField(theTextCodewords, value, 11);
}

void rspfNitfTextHeaderV2_1::setControlAndHandling(const rspfString& value)
{
   rspfNitfCommon::setField(theTextControlAndHandling, value, 2);
}

void rspfNitfTextHeaderV2_1::setReleasingInstructions(const rspfString& value)
{
   rspfNitfCommon::setField(theTextReleasingInstructions, value, 20);
}

void rspfNitfTextHeaderV2_1::setDeclassificationType(const rspfString& value)
{
   rspfNitfCommon::setField(theTextDeclassificationType, value, 2);
}
