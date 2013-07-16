//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Ken Melero
// 
// Description:  This class gives access to the File Header Label
//               (HDR) of a DTED Level 1 file.
//
//********************************************************************
// $Id: rspfDtedHdr.cpp 17501 2010-06-02 11:14:55Z dburken $

#include <iostream>
#include <fstream>
#include <string>

#include <rspf/support_data/rspfDtedHdr.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfProperty.h>

//**************************************************************************
// CONSTRUCTOR
//**************************************************************************
rspfDtedHdr::rspfDtedHdr(const rspfFilename& dted_file, rspf_int32 offset)
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
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedHdr::rspfDtedHdr: The DTED file does not exist: " << dted_file << std::endl;
         return;
      }
      
      // Check to see that the dted file is readable.
      if(!dted_file.isReadable())
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedHdr::rspfDtedHdr: The DTED file is not readable: " << dted_file << std::endl;
         return;
      }
      
      std::ifstream in(dted_file.c_str());
      // Open the dted file for reading.
      if(!in)
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedHdr::rspfDtedHdr: Error opening the DTED file: " << dted_file << std::endl;
         return;
      }
      in.seekg(offset);
      // Continue parsing all the record fields.
      parse(in);
      
      in.close();
   }
}

//**************************************************************************
// CONSTRUCTOR
//**************************************************************************
rspfDtedHdr::rspfDtedHdr(std::istream& in)
   :
      theStartOffset(0),
      theStopOffset(0)
{
   parse(in);
}

//**************************************************************************
// rspfDtedHdr::parse()
//**************************************************************************
void rspfDtedHdr::parse(std::istream& in)
{
   clearErrorStatus();
   theStartOffset = in.tellg();
   theStopOffset  = theStartOffset;
   // Parse theRecSen
   in.read(theRecSen, FIELD1_SIZE);
   theRecSen[FIELD1_SIZE] = '\0';

   if(!(strncmp(theRecSen, "HDR", 3) == 0))
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      in.seekg(theStartOffset);
      return;
   }
   
   // Parse Field 2
   in.read(theField2, FIELD2_SIZE);
   theField2[FIELD2_SIZE] = '\0';
   
   // Parse theFilename
   in.read(theFilename, FIELD3_SIZE);
   theFilename[FIELD3_SIZE] = '\0';
   
   // Parse Field 4
   in.read(theField4, FIELD4_SIZE);
   theField4[FIELD4_SIZE] = '\0';
   
   // Parse Field 5
   in.read(theField5, FIELD5_SIZE);
   theField5[FIELD5_SIZE] = '\0';
   
   // Parse Field 6
   in.read(theField6, FIELD6_SIZE);
   theField6[FIELD6_SIZE] = '\0';
   
   // Parse Field 7
   in.read(theVersion, FIELD7_SIZE);
   theVersion[FIELD7_SIZE] = '\0';
   
   // Parse theCreationDate
   in.read(theCreationDate, FIELD8_SIZE);
   theCreationDate[FIELD8_SIZE] = '\0';
   
   // Parse Field 9
   in.read(theField9, FIELD9_SIZE);
   theField9[FIELD9_SIZE] = '\0';   
   
   // Parse Field 10
   in.read(theField10, FIELD10_SIZE);
   theField10[FIELD10_SIZE] = '\0';   
   
   // Parse Field 11
   in.read(theField11, FIELD11_SIZE);
   theField11[FIELD11_SIZE] = '\0';   
   
   // Parse Field 12
   in.read(theField12, FIELD12_SIZE);
   theField12[FIELD12_SIZE] = '\0';   
   
   // Parse Field 13
   in.read(theField13, FIELD13_SIZE);
   theField13[FIELD13_SIZE] = '\0';   
   
   // Parse Field 14
   in.read(theField14, FIELD14_SIZE);
   theField14[FIELD14_SIZE] = '\0';

   // Set the stop offset.
   theStopOffset = theStartOffset + HDR_LENGTH;
}

rspfRefPtr<rspfProperty> rspfDtedHdr::getProperty(
   const rspfString& /* name */) const
{
   rspfRefPtr<rspfProperty> result = 0;
   return result;
}

void rspfDtedHdr::getPropertyNames(
   std::vector<rspfString>& propertyNames) const
{
   propertyNames.push_back(rspfString("dted_hdr_record"));
}


//**************************************************************************
// operator <<
//**************************************************************************
std::ostream& operator<<( std::ostream& os, const rspfDtedHdr& hdr)
{
   std::string prefix;
   return hdr.print(os, prefix);
}

std::ostream& rspfDtedHdr::print(std::ostream& out,
                                  const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += "hdr.";
   
   out << pfx << "recognition_sentinel:  " << theRecSen << "\n"
       << pfx << "field2:                " << theField2 << "\n"
       << pfx << "filename:              " << theFilename << "\n"
       << pfx << "field4:                " << theField4 << "\n"
       << pfx << "field5:                " << theField5 << "\n"
       << pfx << "field6:                " << theField6 << "\n"
       << pfx << "version:               " << theVersion << "\n"
       << pfx << "creation_date:         " << theCreationDate << "\n"
       << pfx << "field9:                " << theField9 << "\n"
       << pfx << "field10:               " << theField10 << "\n"
       << pfx << "field11:               " << theField11 << "\n"
       << pfx << "field12:               " << theField12 << "\n"
       << pfx << "field13:               " << theField13 << "\n"
       << pfx << "field14:               " << theField14 << "\n"
       << std::endl;
   
   return out;
}

rspfString rspfDtedHdr::recognitionSentinel() const
{
   return theRecSen;
}

rspfString rspfDtedHdr::fileName() const
{
   return theFilename;
}
rspfString rspfDtedHdr::version() const
{ return theVersion;
}

rspfString rspfDtedHdr::creationDate() const
{
   return theCreationDate;
}

rspf_int32 rspfDtedHdr::startOffset() const
{
   return theStartOffset;
}

rspf_int32 rspfDtedHdr::stopOffset() const
{
   return theStopOffset;
}

rspfDtedHdr::rspfDtedHdr(const rspfDtedHdr& /* source */)
{}

const rspfDtedHdr& rspfDtedHdr::operator=(const rspfDtedHdr& rhs)
{
   return rhs;
}
