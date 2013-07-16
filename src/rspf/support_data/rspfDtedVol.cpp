//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
// 
// Description:  This class gives access to the Volume Header Label
//               (VOL) of a DTED Level 1 file.
//
//********************************************************************
// $Id: rspfDtedVol.cpp 17501 2010-06-02 11:14:55Z dburken $

#include <iostream>
#include <fstream>
#include <string>

#include <rspf/support_data/rspfDtedVol.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfProperty.h>

//**************************************************************************
// CONSTRUCTOR
//**************************************************************************
rspfDtedVol::rspfDtedVol(const rspfFilename& dted_file,
                           rspf_int32 offset)
   :
      theStartOffset(0),
      theStopOffset(0)
{
   if(!dted_file.empty())
   {
      // Check to see that dted file exists.
      if(!dted_file.exists())
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedVol::rspfDtedVol"
         << "\nThe DTED file does not exist: " << dted_file << std::endl;
         return;
      }
      
      // Check to see that the dted file is readable.
      if(!dted_file.isReadable())
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedVol::rspfDtedVol"
         << "\nThe DTED file is not readable: " << dted_file << std::endl;
         return;
      }
      
      // Open the dted file for reading.
      std::ifstream in(dted_file.c_str());
      if(!in)
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedVol::rspfDtedVol"
         << "\nUnable to open the DTED file: " << dted_file << std::endl;
         return;
      }
      in.seekg(offset);
      parse(in);
      
      in.close();
   }
}

//**************************************************************************
// CONSTRUCTOR
//**************************************************************************
rspfDtedVol::rspfDtedVol(std::istream& in)
   :
      theStartOffset(0),
      theStopOffset(0)
{
   parse(in);
}

void rspfDtedVol::parse(std::istream& in)
{
   clearErrorStatus();
   theStartOffset = in.tellg();
   theStopOffset  = theStartOffset;
   // Parse theRecSen
   in.read(theRecSen, FIELD1_SIZE);
   theRecSen[FIELD1_SIZE] = '\0';
   if(!(strncmp(theRecSen, "VOL", 3) == 0))
   {
      // Not a volume header label.
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      in.seekg(theStartOffset);
      return;
   }   

   // Parse Field 2
   in.read(theField2, FIELD2_SIZE);
   theField2[FIELD2_SIZE] = '\0';
   
   // Parse theReelNumber
   in.read(theReelNumber, FIELD3_SIZE);
   theReelNumber[FIELD3_SIZE] = '\0';
   
   // Parse Field 4
   in.read(theField4, FIELD4_SIZE);
   theField4[FIELD4_SIZE] = '\0';
   
   // Parse Field 5
   in.read(theField5, FIELD5_SIZE);
   theField5[FIELD5_SIZE] = '\0';
   
   // Parse theAccountNumber
   in.read(theAccountNumber, FIELD6_SIZE);
   theAccountNumber[FIELD6_SIZE] = '\0';
   
   // Parse Field 7
   in.read(theField7, FIELD7_SIZE);
   theField7[FIELD7_SIZE] = '\0';

   // Parse Field 8
   in.read(theField8, FIELD8_SIZE);
   theField8[FIELD8_SIZE] = '\0';

   // Set the stop offset.
   theStopOffset = theStartOffset + VOL_LENGTH;
}

rspfRefPtr<rspfProperty> rspfDtedVol::getProperty(
   const rspfString& /* name */) const
{
   rspfRefPtr<rspfProperty> result = 0;
   return result;
}

void rspfDtedVol::getPropertyNames(
   std::vector<rspfString>& propertyNames) const
{
   propertyNames.push_back(rspfString("dted_vol_record"));
}

rspfString rspfDtedVol::getRecognitionSentinel() const
{
   return rspfString(theRecSen);
}

rspfString rspfDtedVol::getReelNumber() const
{
   return rspfString(theReelNumber);
}

rspfString rspfDtedVol::getAccountNumber() const
{
   return rspfString(theAccountNumber);
}

rspf_int32 rspfDtedVol::startOffset() const
{
   return theStartOffset;
}

rspf_int32 rspfDtedVol::stopOffset() const
{
   return theStopOffset;
}

std::ostream& rspfDtedVol::print(std::ostream& out,
                                  const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += "vol.";
   
   out << pfx << "recognition_sentinel: " << theRecSen << "\n"
       << pfx << "field2:                " << theField2 << "\n"
       << pfx << "reel_number:           " << theReelNumber << "\n"
       << pfx << "field4:                " << theField4 << "\n"
       << pfx << "field5:                " << theField5 << "\n"
       << pfx << "account_number:        " << theAccountNumber << "\n"
       << pfx << "field7:                " << theField7 << "\n"
       << pfx << "field8:                " << theField8 << "\n"
       << std::endl;
   return out;
}

//**************************************************************************
// operator <<
//**************************************************************************
std::ostream& operator<<( std::ostream& out, const rspfDtedVol& vol)
{
   std::string prefix;
   return vol.print(out, prefix);
}

rspfDtedVol::rspfDtedVol(const rspfDtedVol& /* source */)
{
}

const rspfDtedVol& rspfDtedVol::operator=(const rspfDtedVol& rhs)
{
   return rhs;
}

