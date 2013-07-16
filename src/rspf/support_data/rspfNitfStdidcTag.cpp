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
// $Id: rspfNitfStdidcTag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <iostream>
#include <iomanip>
#include <rspf/support_data/rspfNitfStdidcTag.h>
#include <rspf/base/rspfStringProperty.h>

RTTI_DEF1(rspfNitfStdidcTag, "rspfNitfStdidcTag", rspfNitfRegisteredTag);
static const rspfString ACQDATE_KW = "ACQDATE";
static const rspfString MISSION_KW = "MISSION";
static const rspfString PASS_KW = "PASS";
static const rspfString OPNUM_KW = "OPNUM";
static const rspfString STARTSEGMENT_KW = "STARTSEGMENT";
static const rspfString REPRONUM_KW = "REPRONUM";
static const rspfString REPLAYREGEN_KW = "REPLAYREGEN";
static const rspfString BLANKFILL_KW = "BLANKFILL";
static const rspfString STARTCOLUMN_KW = "STARTCOLUMN";
static const rspfString STARTROW_KW = "STARTROW";
static const rspfString ENDSEGMENT_KW = "ENDSEGMENT";
static const rspfString ENDCOLUMN_KW = "ENDCOLUMN";
static const rspfString ENDROW_KW = "ENDROW";
static const rspfString COUNTRY_KW = "COUNTRY";
static const rspfString WAC_KW = "WAC";
static const rspfString LOCATION_KW = "LOCATION";


rspfNitfStdidcTag::rspfNitfStdidcTag()
   : rspfNitfRegisteredTag(std::string("STDIDC"), 89)
{
   clearFields();
}

rspfNitfStdidcTag::~rspfNitfStdidcTag()
{
}

void rspfNitfStdidcTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(theAcqDate,      ACQ_DATE_SIZE);
   in.read(theMission,      MISSION_SIZE);
   in.read(thePass,         PASS_SIZE);
   in.read(theOpNum,        OP_NUM_SIZE);
   in.read(theStartSegment, START_SEGMENT_SIZE);
   in.read(theReproNum,     REPRO_NUM_SIZE);
   in.read(theReplayRegen,  REPLAY_REGEN_SIZE);
   in.read(theBlankFill,    BLANK_FILL_SIZE);
   in.read(theStartColumn,  START_COLUMN_SIZE);
   in.read(theStartRow,     START_ROW_SIZE);
   in.read(theEndSegment,   END_SEGMENT_SIZE);
   in.read(theEndColumn,    END_COLUMN_SIZE);
   in.read(theEndRow,       END_ROW_SIZE);
   in.read(theCountry,      COUNTRY_SIZE);
   in.read(theWac,          WAC_SIZE);
   in.read(theLocation,     LOCATION_SIZE);
   in.read(theField17,      FIELD17_SIZE);
   in.read(theField18,      FIELD18_SIZE);
}

void rspfNitfStdidcTag::writeStream(std::ostream& out)
{
   out.write(theAcqDate,      ACQ_DATE_SIZE);
   out.write(theMission,      MISSION_SIZE);
   out.write(thePass,         PASS_SIZE);
   out.write(theOpNum,        OP_NUM_SIZE);
   out.write(theStartSegment, START_SEGMENT_SIZE);
   out.write(theReproNum,     REPRO_NUM_SIZE);
   out.write(theReplayRegen,  REPLAY_REGEN_SIZE);
   out.write(theBlankFill,    BLANK_FILL_SIZE);
   out.write(theStartColumn,  START_COLUMN_SIZE);
   out.write(theStartRow,     START_ROW_SIZE);
   out.write(theEndSegment,   END_SEGMENT_SIZE);
   out.write(theEndColumn,    END_COLUMN_SIZE);
   out.write(theEndRow,       END_ROW_SIZE);
   out.write(theCountry,      COUNTRY_SIZE);
   out.write(theWac,          WAC_SIZE);
   out.write(theLocation,     LOCATION_SIZE);
   out.write(theField17,      FIELD17_SIZE);
   out.write(theField18,      FIELD18_SIZE);
}

void rspfNitfStdidcTag::clearFields()
{
   memset(theAcqDate,      ' ', ACQ_DATE_SIZE);
   memset(theMission,      ' ', MISSION_SIZE);
   memset(thePass,         ' ', PASS_SIZE);
   memset(theOpNum,        ' ', OP_NUM_SIZE);
   memset(theStartSegment, ' ', START_SEGMENT_SIZE);
   memset(theReproNum,     ' ', REPRO_NUM_SIZE);
   memset(theReplayRegen,  ' ', REPLAY_REGEN_SIZE);
   memset(theBlankFill,    ' ', BLANK_FILL_SIZE);
   memset(theStartColumn,  ' ', START_COLUMN_SIZE);
   memset(theStartRow,     ' ', START_ROW_SIZE);
   memset(theEndSegment,   ' ', END_SEGMENT_SIZE);
   memset(theEndColumn,    ' ', END_COLUMN_SIZE);
   memset(theEndRow,       ' ', END_ROW_SIZE);
   memset(theCountry,      ' ', COUNTRY_SIZE);
   memset(theWac,          ' ', WAC_SIZE);
   memset(theLocation,     ' ', LOCATION_SIZE);
   memset(theField17,      ' ', FIELD17_SIZE);
   memset(theField18,      ' ', FIELD18_SIZE);

   theAcqDate[ACQ_DATE_SIZE]           = '\0';
   theMission[MISSION_SIZE]            = '\0';
   thePass[PASS_SIZE]                  = '\0';
   theOpNum[OP_NUM_SIZE]               = '\0';
   theStartSegment[START_SEGMENT_SIZE] = '\0';
   theReproNum[REPRO_NUM_SIZE]         = '\0';
   theReplayRegen[REPLAY_REGEN_SIZE]   = '\0';
   theBlankFill[BLANK_FILL_SIZE]       = '\0';
   theStartColumn[START_COLUMN_SIZE]   = '\0';
   theStartRow[START_ROW_SIZE]         = '\0';
   theEndSegment[END_SEGMENT_SIZE]     = '\0';
   theEndColumn[END_COLUMN_SIZE]       = '\0';
   theEndRow[END_ROW_SIZE]             = '\0';
   theCountry[COUNTRY_SIZE]            = '\0';
   theWac[WAC_SIZE]                    = '\0';
   theLocation[LOCATION_SIZE]          = '\0';
   theField17[FIELD17_SIZE]            = '\0';
   theField18[FIELD18_SIZE]            = '\0';
}

rspfString rspfNitfStdidcTag::getAcqDate()const
{
  return rspfString(theAcqDate);
}

void rspfNitfStdidcTag::setAcqDate(rspfString acqDate)
{
   memset(theAcqDate, ' ', ACQ_DATE_SIZE);
   memcpy(theAcqDate, acqDate.c_str(), std::min((size_t)ACQ_DATE_SIZE, acqDate.length()));
}

rspfString rspfNitfStdidcTag::getMission()const
{
  return rspfString(theMission);
}

void rspfNitfStdidcTag::setMission(rspfString mission)
{
   memset(theMission, ' ', MISSION_SIZE);
   memcpy(theMission, mission.c_str(), std::min((size_t)MISSION_SIZE, mission.length()));
}

rspfString rspfNitfStdidcTag::getPass()const
{
  return rspfString(thePass);
}

void rspfNitfStdidcTag::setPass(rspfString pass)
{
   memset(thePass, ' ', PASS_SIZE);
   memcpy(thePass, pass.c_str(), std::min((size_t)PASS_SIZE, pass.length()));
}

rspfString rspfNitfStdidcTag::getOpNum()const
{
  return rspfString(theOpNum);
}

void rspfNitfStdidcTag::setOpNum(rspfString opNum) 
{
   memset(theOpNum, ' ', OP_NUM_SIZE);
   memcpy(theOpNum, opNum.c_str(), std::min((size_t)OP_NUM_SIZE, opNum.length()));
}

rspfString rspfNitfStdidcTag::getStartSegment()const
{
  return rspfString(theStartSegment);
}

void rspfNitfStdidcTag::setStartSegment(rspfString startSegment) 
{
   memset(theStartSegment, ' ', START_SEGMENT_SIZE);
   memcpy(theStartSegment, startSegment.c_str(), std::min((size_t)START_SEGMENT_SIZE, startSegment.length()));
}

rspfString rspfNitfStdidcTag::getReproNum()const
{
  return rspfString(theReproNum);
}

void rspfNitfStdidcTag::setReproNum(rspfString reproNum) 
{
   memset(theReproNum, ' ', REPRO_NUM_SIZE);
   memcpy(theReproNum, reproNum.c_str(), std::min((size_t)REPRO_NUM_SIZE, reproNum.length()));
}

rspfString rspfNitfStdidcTag::getReplayRegen()const
{
  return rspfString(theReplayRegen);
}

void rspfNitfStdidcTag::setReplayRegen(rspfString replayRegen) 
{
   memset(theReplayRegen, ' ', REPLAY_REGEN_SIZE);
   memcpy(theReplayRegen, replayRegen.c_str(), std::min((size_t)REPLAY_REGEN_SIZE, replayRegen.length()));
}

rspfString rspfNitfStdidcTag::getBlankFill()const
{
  return rspfString(theBlankFill);
}

void rspfNitfStdidcTag::setBlankFill(rspfString blankFill) 
{
   memset(theBlankFill, ' ', BLANK_FILL_SIZE);
   memcpy(theBlankFill, blankFill.c_str(), std::min((size_t)BLANK_FILL_SIZE, blankFill.length()));
}

rspfString rspfNitfStdidcTag::getStartColumn()const
{
  return rspfString(theStartColumn);
}

void rspfNitfStdidcTag::setStartColumn(rspfString startColumn) 
{
   memset(theStartColumn, ' ', START_COLUMN_SIZE);
   memcpy(theStartColumn, startColumn.c_str(), std::min((size_t)START_COLUMN_SIZE, startColumn.length()));
}

rspfString rspfNitfStdidcTag::getStartRow()const
{
  return rspfString(theStartRow);
}

void rspfNitfStdidcTag::setStartRow(rspfString startRow) 
{
   memset(theStartRow, ' ', START_ROW_SIZE);
   memcpy(theStartRow, startRow.c_str(), std::min((size_t)START_ROW_SIZE, startRow.length()));
}

rspfString rspfNitfStdidcTag::getEndSegment()const
{
  return rspfString(theEndSegment);
}

void rspfNitfStdidcTag::setEndSegment(rspfString endSegment) 
{
   memset(theEndSegment, ' ', END_SEGMENT_SIZE);
   memcpy(theEndSegment, endSegment.c_str(), std::min((size_t)END_SEGMENT_SIZE, endSegment.length()));
}

rspfString rspfNitfStdidcTag::getEndColumn()const
{
  return rspfString(theEndColumn);
}

void rspfNitfStdidcTag::setEndColumn(rspfString endColumn) 
{
   memset(theEndColumn, ' ', END_COLUMN_SIZE);
   memcpy(theEndColumn, endColumn.c_str(), std::min((size_t)END_COLUMN_SIZE, endColumn.length()));
}

rspfString rspfNitfStdidcTag::getEndRow()const
{
  return rspfString(theEndRow);
}

void rspfNitfStdidcTag::setEndRow(rspfString endRow) 
{
   memset(theEndRow, ' ', END_ROW_SIZE);
   memcpy(theEndRow, endRow.c_str(), std::min((size_t)END_ROW_SIZE, endRow.length()));
}

rspfString rspfNitfStdidcTag::getCountry()const
{
  return rspfString(theCountry);
}

void rspfNitfStdidcTag::setCountry(rspfString country) 
{
   memset(theCountry, ' ', COUNTRY_SIZE);
   memcpy(theCountry, country.c_str(), std::min((size_t)COUNTRY_SIZE, country.length()));
}

rspfString rspfNitfStdidcTag::getWac()const
{
  return rspfString(theWac);
}

void rspfNitfStdidcTag::setWac(rspfString wac) 
{
   memset(theWac, ' ', WAC_SIZE);
   memcpy(theWac, wac.c_str(), std::min((size_t)WAC_SIZE, wac.length()));
}

rspfString rspfNitfStdidcTag::getLocation()const
{
  return rspfString(theLocation);
}

void rspfNitfStdidcTag::setLocation(rspfString location) 
{
   memset(theLocation, ' ', LOCATION_SIZE);
   memcpy(theLocation, location.c_str(), std::min((size_t)LOCATION_SIZE, location.length()));
}

rspfString rspfNitfStdidcTag::getField17()const
{
  return rspfString(theField17);
}

void rspfNitfStdidcTag::setField17(rspfString field17) 
{
   memset(theField17, ' ', FIELD17_SIZE);
   memcpy(theField17, field17.c_str(), std::min((size_t)FIELD17_SIZE, field17.length()));
}

rspfString rspfNitfStdidcTag::getField18()const
{
  return rspfString(theField18);
}

void rspfNitfStdidcTag::setField18(rspfString field18) 
{
   memset(theField18, ' ', FIELD18_SIZE);
   memcpy(theField18, field18.c_str(), std::min((size_t)FIELD18_SIZE, field18.length()));
}

std::ostream& rspfNitfStdidcTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "ACQDATE:"      << theAcqDate << "\n"
       << pfx << std::setw(24) << "MISSION:"      << theMission << "\n"
       << pfx << std::setw(24) << "PASS:"         << thePass << "\n"
       << pfx << std::setw(24) << "OPNUM:"        << theOpNum  << "\n"
       << pfx << std::setw(24) << "STARTSEGMENT:" << theStartSegment << "\n"
       << pfx << std::setw(24) << "REPRONUM:"     << theReproNum << "\n"
       << pfx << std::setw(24) << "REPLAYREGEN:"  << theReplayRegen << "\n"
       << pfx << std::setw(24) << "STARTCOLUMN:"  << theStartColumn << "\n"
       << pfx << std::setw(24) << "STARTROW:"     << theStartRow << "\n"
       << pfx << std::setw(24) << "ENDSEGMENT:"   << theEndSegment << "\n"
       << pfx << std::setw(24) << "ENDCOLUMN:"    << theEndColumn << "\n"
       << pfx << std::setw(24) << "ENDROW:"       << theEndRow << "\n"
       << pfx << std::setw(24) << "COUNTRY:"      << theCountry << "\n"
       << pfx << std::setw(24) << "WAC:"          << theWac << "\n"
       << pfx << std::setw(24) << "LOCATION:"     << theLocation << "\n";

   return out;
}

void rspfNitfStdidcTag::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfNitfRegisteredTag::setProperty(property);
}

rspfRefPtr<rspfProperty> rspfNitfStdidcTag::getProperty(const rspfString& name)const
{
   rspfProperty* result = 0;
   
   if(name == ACQDATE_KW)
   {
      result = new rspfStringProperty(name, theAcqDate);
   }
   else if(name == MISSION_KW)
   {
      result = new rspfStringProperty(name, theMission);
   }
   else if(name == PASS_KW)
   {
      result = new rspfStringProperty(name, thePass);
   }
   else if(name == OPNUM_KW)
   {
      result = new rspfStringProperty(name, theOpNum);
   }
   else if(name == STARTSEGMENT_KW)
   {
      result = new rspfStringProperty(name, theStartSegment);
   }
   else if(name == REPRONUM_KW)
   {
      result = new rspfStringProperty(name, theReproNum);
   }
   else if(name == REPLAYREGEN_KW)
   {
      result = new rspfStringProperty(name, theReplayRegen);
   }
   else if(name == BLANKFILL_KW)
   {
      result = new rspfStringProperty(name, theBlankFill);
   }
   else if(name == STARTCOLUMN_KW)
   {
      result = new rspfStringProperty(name, theStartColumn);
   }
   else if(name == STARTROW_KW)
   {
      result = new rspfStringProperty(name, theStartRow);
   }
   else if(name == ENDSEGMENT_KW)
   {
      result = new rspfStringProperty(name, theEndSegment);
   }
   else if(name == ENDCOLUMN_KW)
   {
      result = new rspfStringProperty(name, theEndColumn);
   }
   else if(name == ENDROW_KW)
   {
      result = new rspfStringProperty(name, theEndRow);
   }
   else if(name == COUNTRY_KW)
   {
      result = new rspfStringProperty(name,theCountry );
   }
   else if(name == WAC_KW)
   {
      result = new rspfStringProperty(name, theWac);
   }
   else if(name == LOCATION_KW)
   {
      result = new rspfStringProperty(name, theLocation);
   }
   else
   {
      return rspfNitfRegisteredTag::getProperty(name);
   }

   return result;
}

void rspfNitfStdidcTag::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfNitfRegisteredTag::getPropertyNames(propertyNames);

   propertyNames.push_back(ACQDATE_KW);
   propertyNames.push_back(MISSION_KW);
   propertyNames.push_back(PASS_KW);
   propertyNames.push_back(OPNUM_KW);
   propertyNames.push_back(STARTSEGMENT_KW);
   propertyNames.push_back(REPRONUM_KW);
   propertyNames.push_back(REPLAYREGEN_KW);
   propertyNames.push_back(BLANKFILL_KW);
   propertyNames.push_back(STARTCOLUMN_KW);
   propertyNames.push_back(STARTROW_KW);
   propertyNames.push_back(ENDSEGMENT_KW);
   propertyNames.push_back(ENDCOLUMN_KW);
   propertyNames.push_back(ENDROW_KW);
   propertyNames.push_back(COUNTRY_KW);
   propertyNames.push_back(WAC_KW);
   propertyNames.push_back(LOCATION_KW);
   
}

