//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
// 
// Description:  This class gives access to the User Header Label
//               (UHL) of a DTED Level 1 file.
//
//********************************************************************
// $Id: rspfDtedUhl.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <rspf/support_data/rspfDtedUhl.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfProperty.h>


//**************************************************************************
// CONSTRUCTOR
//**************************************************************************
rspfDtedUhl::rspfDtedUhl(const rspfFilename& dted_file, rspf_int32 offset)
   :
      theRecSen(),
      theField2(),
      theLonOrigin(),
      theLatOrigin(),
      theLonInterval(),
      theLatInterval(),
      theAbsoluteLE(),
      theSecurityCode(),
      theNumLonLines(),
      theNumLatPoints(),
      theMultipleAccuracy(),
      theStartOffset(0),
      theStopOffset(0)
{
   if(!dted_file.empty())
   {
      // Check to see that dted file exists.
      if(!dted_file.exists())
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedUhl::rspfDtedUhl: The DTED file does not exist: " << dted_file << std::endl;
         return;
      }
      
      // Check to see that the dted file is readable.
      if(!dted_file.isReadable())
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedUhl::rspfDtedUhl: The DTED file is not readable --> " << dted_file << std::endl;
         return;
      }
      
      std::ifstream in(dted_file.c_str());
      if(!in)
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfDtedUhl::rspfDtedUhl: Error opening the DTED file: " << dted_file << std::endl;
         
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
rspfDtedUhl::rspfDtedUhl(std::istream& in)
   :
      theRecSen(),
      theLonOrigin(),
      theLatOrigin(),
      theLonInterval(),
      theLatInterval(),
      theAbsoluteLE(),
      theSecurityCode(),
      theNumLonLines(),
      theNumLatPoints(),
      theMultipleAccuracy(),
      theStartOffset(0),
      theStopOffset(0)
{
   parse(in);
}

//**************************************************************************
// rspfDtedUhl::parse()
//**************************************************************************
void rspfDtedUhl::parse(std::istream& in)
{
   clearErrorStatus();
   theStartOffset = in.tellg();
   theStopOffset  = theStartOffset;
   // Parse theRecSen
   in.read(theRecSen, FIELD1_SIZE);
   theRecSen[FIELD1_SIZE] = '\0';

   if(!(strncmp(theRecSen, "UHL", 3) == 0))
   {
      // Not a user header label.
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;  
      in.seekg(theStartOffset);
      return;
   }  
   
   // Parse Field 2
   in.read(theField2, FIELD2_SIZE);
   theField2[FIELD2_SIZE] = '\0';
   
   // Parse theLonOrigin
   in.read(theLonOrigin, FIELD3_SIZE);
   theLonOrigin[FIELD3_SIZE] = '\0';
   
   // Parse theLatOrigin
   in.read(theLatOrigin, FIELD4_SIZE);
   theLatOrigin[FIELD4_SIZE] = '\0';
   
   // Parse theLonInterval
   in.read(theLonInterval, FIELD5_SIZE);
   theLonInterval[FIELD5_SIZE] = '\0';
   
   // Parse theLatInterval
   in.read(theLatInterval, FIELD6_SIZE);
   theLatInterval[FIELD6_SIZE] = '\0';
   
   // Parse theAbsoluteLE
   in.read(theAbsoluteLE, FIELD7_SIZE);
   theAbsoluteLE[FIELD7_SIZE] = '\0';
   
   // Parse theSecurityCode
   in.read(theSecurityCode, FIELD8_SIZE);
   theSecurityCode[FIELD8_SIZE] = '\0';
   
   // Parse Field 9
   in.read(theField9, FIELD9_SIZE);
   theField9[FIELD9_SIZE] = '\0';
   
   // Parse theNumLonLines
   in.read(theNumLonLines, FIELD10_SIZE);
   theNumLonLines[FIELD10_SIZE] = '\0';
   
   // Parse theNumLatPoints
   in.read(theNumLatPoints, FIELD11_SIZE);
   theNumLatPoints[FIELD11_SIZE] = '\0';
   
   // Parse theMultipleAccuracy
   in.read(theMultipleAccuracy, FIELD12_SIZE);
   theMultipleAccuracy[FIELD12_SIZE] = '\0';

   // Field 13 not parsed as it's unused.
   in.ignore(FIELD13_SIZE);
   // Set the stop offset.
   theStopOffset = theStartOffset + UHL_LENGTH;
}

rspfRefPtr<rspfProperty> rspfDtedUhl::getProperty(
   const rspfString& /* name */) const
{
   rspfRefPtr<rspfProperty> result = 0;
   return result;
}

void rspfDtedUhl::getPropertyNames(
   std::vector<rspfString>& propertyNames) const
{
   propertyNames.push_back(rspfString("dted_uhl_record"));
}

rspfString rspfDtedUhl::recognitionSentinel() const
{
   return theRecSen;
}

double rspfDtedUhl::lonOrigin() const
{
   return degreesFromString(theLonOrigin);
}

double rspfDtedUhl::latOrigin() const
{
   return degreesFromString(theLatOrigin);
}

double rspfDtedUhl::lonInterval()      const
{
   return spacingFromString(theLonInterval);
}

double rspfDtedUhl::latInterval()      const
{
   return spacingFromString(theLatInterval);
}
  
double rspfDtedUhl::degreesFromString(const char* str) const
{
   // Parse the string:  DDDMMMSSH
   if (!str)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedUhl::degreesFromString: "
         << "Null pointer passed to method!" << std::endl;
      return 0.0;
   }
   
   if (strlen(str) < 8)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedUhl::decimalDegreesFromString:"
         << "String not big enough!" << std::endl;
      return 0.0;
   }

   double d = ((str[0]-'0')*100 + (str[1]-'0')*10 + (str[2]-'0') +
               (str[3]-'0')/6.0 + (str[4]-'0')/60.0 +
               (str[5]-'0')/360.0 + (str[6]-'0')/3600.0);
   
   if ( (str[7] == 'S') || (str[7] == 's') ||
        (str[7] == 'W') || (str[7] == 'w') )
   {
      d *= -1.0;
   }
   
   return d;
}
 
double rspfDtedUhl::spacingFromString(const char* str) const
{
   // Parse the string: SSSS (tenths of a second)
   if (!str)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedUhl::decimalDegreesFromString: "
         << "Null pointer passed to method!" << std::endl;
      return 0.0;
   }

   return atof(str) / 36000.0;  // return 10ths of second as decimal degrees.
}

double rspfDtedUhl::absoluteLE() const
{
   return (theAbsoluteLE ? atoi(theAbsoluteLE) : 0);
}

rspfString rspfDtedUhl::securityCode() const
{
   return theSecurityCode;
}

rspf_int32 rspfDtedUhl::numLonLines() const
{
   return (theNumLonLines ? atoi(theNumLonLines) : 0);
}

rspf_int32 rspfDtedUhl::numLatPoints() const
{
   return (theNumLatPoints ? atoi(theNumLatPoints) : 0);
}

rspf_int32 rspfDtedUhl::mulitpleAccuracy() const
{
   return (theMultipleAccuracy ? atoi(theMultipleAccuracy) : 0);
}

rspf_int32 rspfDtedUhl::startOffset() const
{
   return theStartOffset;
}

rspf_int32 rspfDtedUhl::stopOffset() const
{
   return theStopOffset;
}

//**************************************************************************
// operator <<
//**************************************************************************
std::ostream& operator<<( std::ostream& out, const rspfDtedUhl& uhl)
{
   std::string prefix;
   return uhl.print(out, prefix);
}

std::ostream& rspfDtedUhl::print(std::ostream& out,
                                  const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += "uhl.";
   
   out << setiosflags(ios::left)
       << pfx << setw(28) << "recognition_sentinel:" << theRecSen << "\n"
       << pfx << setw(28) << "lon_origin:" << theLonOrigin << "\n"
       << pfx << setw(28) << "lat_origin:" << theLatOrigin << "\n"
       << pfx << setw(28) << "lon_interval:" << theLonInterval << "\n"
       << pfx << setw(28) << "lat_interval:" << theLatInterval << "\n"
       << pfx << setw(28) << "absolute_le:" << theAbsoluteLE << "\n"
       << pfx << setw(28) << "security_code:" << theSecurityCode << "\n"
       << pfx << setw(28) << "number_of_lat_points:" << theNumLatPoints << "\n"
       << pfx << setw(28) << "number_of_lon_lines:" << theNumLonLines << "\n"
       << pfx << setw(28) << "multiple_accuracy:"
       << theMultipleAccuracy << "\n"
       << pfx << setw(28) << "start_offset:" << theStartOffset << "\n"
       << pfx << setw(28) << "stop_offset:" << theStopOffset
       << std::endl;
   
   return out;
}

rspfDtedUhl::rspfDtedUhl(const rspfDtedUhl& /* source */)
{
}

const rspfDtedUhl& rspfDtedUhl::operator=(const rspfDtedUhl& rhs)
{
   return rhs;
}

