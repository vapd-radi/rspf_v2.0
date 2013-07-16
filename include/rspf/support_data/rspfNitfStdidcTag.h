//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author:  Walt Bunch
// 
// Description:   NITF support data class for STDIDC - Standard ID extension.
//
// See:  STDI-000_v2.1 Table 7-3 for detailed description.
// 
//********************************************************************
// $Id: rspfNitfStdidcTag.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef rspfNitfStdidcTag_HEADER
#define rspfNitfStdidcTag_HEADER 1
#include <rspf/support_data/rspfNitfRegisteredTag.h>

class RSPF_DLL rspfNitfStdidcTag : public rspfNitfRegisteredTag
{
public:

   enum
   {
      ACQ_DATE_SIZE         = 14,
      MISSION_SIZE          = 14,
      PASS_SIZE             = 2,
      OP_NUM_SIZE           = 3,
      START_SEGMENT_SIZE    = 2,
      REPRO_NUM_SIZE        = 2,
      REPLAY_REGEN_SIZE     = 3,
      BLANK_FILL_SIZE       = 1,
      START_COLUMN_SIZE     = 3,
      START_ROW_SIZE        = 5,
      END_SEGMENT_SIZE      = 2,
      END_COLUMN_SIZE       = 3,
      END_ROW_SIZE          = 5,
      COUNTRY_SIZE          = 2,
      WAC_SIZE              = 4,
      LOCATION_SIZE         = 11,
      FIELD17_SIZE          = 5,
      FIELD18_SIZE          = 8
      //                   -----
      //                     89 bytes
   };
      
   rspfNitfStdidcTag();
   virtual ~rspfNitfStdidcTag();

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual void clearFields();

  // The set methods below taking rspfString args will truncate and
  // pad with spaces, as necessary, to match enumed size
   void setAcqDate(rspfString acqDate);
   rspfString getAcqDate()const;
   void setMission(rspfString mission);
   rspfString getMission()const;
   void setPass(rspfString pass);
   rspfString getPass()const;
   void setOpNum(rspfString opNum);
   rspfString getOpNum()const;
   void setStartSegment(rspfString startSegment);
   rspfString getStartSegment()const;
   void setReproNum(rspfString reproNum);
   rspfString getReproNum()const;
   void setReplayRegen(rspfString replayRegen);
   rspfString getReplayRegen()const;
   void setBlankFill(rspfString blankFill);
   rspfString getBlankFill()const;
   void setStartColumn(rspfString startColumn);
   rspfString getStartColumn()const;
   void setStartRow(rspfString startRow);
   rspfString getStartRow()const;
   void setEndSegment(rspfString endSegment);
   rspfString getEndSegment()const;
   void setEndColumn(rspfString endColumn);
   rspfString getEndColumn()const;
   void setEndRow(rspfString endRow);
   rspfString getEndRow()const;
   void setCountry(rspfString country);
   rspfString getCountry()const;
   void setWac(rspfString wac);
   rspfString getWac()const;
   void setLocation(rspfString location);
   rspfString getLocation()const;
   void setField17(rspfString field17);
   rspfString getField17()const;
   void setField18(rspfString field18);
   rspfString getField18()const;

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;


protected:

   /**
    * FIELD: ACQDATE
    */
   char theAcqDate[ACQ_DATE_SIZE+1];
   
   /**
    * FIELD: MISSION
    */
   char theMission[MISSION_SIZE+1];
   
   /**
    * FIELD: PASS
    */
   char thePass[PASS_SIZE+1];
   
   /**
    * FIELD: OPNUM
    */
   char theOpNum[OP_NUM_SIZE+1];
   
   /**
    * FIELD: STARTSEGMENT
    */
   char theStartSegment[START_SEGMENT_SIZE+1];
   
   /**
    * FIELD: REPRONUM
    */
   char theReproNum[REPRO_NUM_SIZE+1];
   
   /**
    * FIELD: REPLAYREGEN
    */
   char theReplayRegen[REPLAY_REGEN_SIZE+1];
   
   /**
    * FIELD: BLANKFILL
    */
   char theBlankFill[BLANK_FILL_SIZE+1];
   
   /**
    * FIELD: STARTCOLUMN
    */
   char theStartColumn[START_COLUMN_SIZE+1];
   
   /**
    * FIELD: STARTROW
    */
   char theStartRow[START_ROW_SIZE+1];
   
   /**
    * FIELD: ENDSEGMENT
    */
   char theEndSegment[END_SEGMENT_SIZE+1];
   
   /**
    * FIELD: ENDCOLUMN
    */
   char theEndColumn[END_COLUMN_SIZE+1];
   
   /**
    * FIELD: ENDROW
    */
   char theEndRow[END_ROW_SIZE+1];
   
   /**
    * FIELD: COUNTRY
    */
   char theCountry[COUNTRY_SIZE+1];
   
   /**
    * FIELD: WAC
    */
   char theWac[WAC_SIZE+1];
   
   /**
    * FIELD: LOCATION
    */
   char theLocation[LOCATION_SIZE+1];
   
   /**
    * FIELD: FIELD17
    */
   char theField17[FIELD17_SIZE+1];
   
   /**
    * FIELD: FIELD18
    */
   char theField18[FIELD18_SIZE+1];

TYPE_DATA   
};

#endif
