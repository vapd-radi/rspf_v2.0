//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfDataExtensionSegmentV2_0.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfNitfDataExtensionSegmentV2_0_HEADER
#define rspfNitfDataExtensionSegmentV2_0_HEADER
#include <rspf/support_data/rspfNitfDataExtensionSegment.h>
#include <rspf/support_data/rspfNitfTagInformation.h>

struct rspfNitfDataExtIdentSecurityChunkV2_0
{
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfNitfDataExtIdentSecurityChunkV2_0& data);
   char theFilePartType[3]; // 2 byte alpha
   char theUniqueDataExtTypeId[26]; // 25 byte alpha numeric
   char theVersionDataFieldDef[3]; // 2 byte numeric
   char theDataExtSecurityClass[2]; // 1 byte alpha
   char theDataExtCodewords[41];    // 40 byte alpha numeric
   char theDataExtControlAndHand[41]; // 40 byte alpha numeric
   char theDataExtReleasingInst[41]; // 40 byte alpha numeric
   char theDataExtClassAuthority[21]; // 20 byte alph num
   char theDataExtSecurityConNum[21]; // 20 byte alpha num
   char theDataExtSecuityDowngrade[7]; // 6 byte alpha num
   char theDataExtDowngradingEvent[41]; // 40 byte alpha num
};

class RSPFDLLEXPORT rspfNitfDataExtensionSegmentV2_0 : public rspfNitfDataExtensionSegment
{
public:
   rspfNitfDataExtensionSegmentV2_0();
   virtual void parseStream(std::istream &in);
   virtual std::ostream& print(std::ostream& out)const;
   virtual const rspfNitfTagInformation&  getTagInformation()const
      {
         return theTag;
      }
protected:
   virtual ~rspfNitfDataExtensionSegmentV2_0();

private:
   void clearFields();
   
   rspfNitfDataExtIdentSecurityChunkV2_0 theIdentSecurityChunk;
   
   /*!
    * This is a 6 byte field
    */
   char theOverflowedHeaderType[7];

   /*!
    * This is a 3 byte field
    */
   char theDataItemOverflowed[4];

   /*!
    * This is a 4 byte field.
    */
   char theLengthOfUserDefinedSubheaderFields[5];
   
   rspfNitfTagInformation                theTag;

TYPE_DATA
};

#endif
