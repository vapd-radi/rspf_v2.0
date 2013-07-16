//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfProjectionParameterTag.cpp 22013 2012-12-19 17:37:20Z dburken $
#include <rspf/support_data/rspfNitfProjectionParameterTag.h>
#include <sstream>
#include <iomanip>

rspfNitfProjectionParameterTag::rspfNitfProjectionParameterTag()
   :rspfNitfRegisteredTag(std::string("PRJPSB"), 0 )
{
   clearFields();
}

rspfNitfProjectionParameterTag::~rspfNitfProjectionParameterTag()
{
}

void rspfNitfProjectionParameterTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(theProjectionName, 80);
   in.read(theProjectionCode, 2);
   in.read(theNumberOfParameters, 1);

   rspf_uint32 numberOfParameters = rspfString(theNumberOfParameters).toUInt32();

   for(rspf_uint32 i = 0; i < numberOfParameters; ++i)
   {
      char param[15];

      in.read(param, 15);
      theProjectionParameters.push_back(rspfString(param,
                                                    param + 15));
   }
   in.read(theFalseXOrigin, 15);
   in.read(theFalseYOrigin, 15);

   // Set the base tag length.
   setTagLength( getSizeInBytes() );
}

void rspfNitfProjectionParameterTag::writeStream(std::ostream& out)
{
   out.write(theProjectionName, 80);
   out.write(theProjectionCode, 2);

   {
      std::ostringstream tempOut;

      tempOut << std::setw(1)
              << theProjectionParameters.size();
      theNumberOfParameters[0] = *(tempOut.str().c_str());
   }
   out.write(theNumberOfParameters, 1);
   rspf_uint32 numberOfParameters = rspfString(theNumberOfParameters).toUInt32();

   for(rspf_uint32 i = 0; i < numberOfParameters; ++i)
   {
      out.write(theProjectionParameters[i].c_str(), 15);
   }
   out.write(theFalseXOrigin, 15);
   out.write(theFalseYOrigin, 15);
}

rspf_uint32 rspfNitfProjectionParameterTag::getSizeInBytes()const
{
   return (113 + (rspf_uint32)theProjectionParameters.size()*15);
}

std::ostream& rspfNitfProjectionParameterTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getRegisterTagName();
   pfx += ".";

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:" << getRegisterTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "PRN:"   << theProjectionName << "\n"
       << pfx << std::setw(24) << "PCO:"   << theProjectionCode << "\n"
       << pfx << std::setw(24) << "NUM_PRJ:" << theNumberOfParameters << "\n";

   for (rspf_uint32 i = 0; i < theProjectionParameters.size(); ++i)
   {
      rspfString s = "PRJ";
      s += rspfString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << theProjectionParameters[i] << "\n";
   }

   out << pfx << std::setw(24) << "XOR:"   <<theFalseXOrigin << "\n"
       << pfx << std::setw(24) << "YOR:"   <<theFalseYOrigin << std::endl;
   
   return out;   
}
    
void rspfNitfProjectionParameterTag::clearFields()
{
   theProjectionParameters.clear();
   
   memset(theProjectionName, ' ', 80);
   memset(theProjectionCode, ' ', 2);
   memset(theNumberOfParameters, '0', 1);
   memset(theFalseXOrigin, '0', 15);
   memset(theFalseYOrigin, '0', 15);

   theProjectionName[80] = '\0';
   theProjectionCode[2] = '\0';
   theNumberOfParameters[1] = '\0';
   theFalseXOrigin[15] = '\0';
   theFalseYOrigin[15] = '\0';

   // Set the base tag length.
   setTagLength( 0 );
}

void rspfNitfProjectionParameterTag::clearParameters()
{
   theProjectionParameters.clear();
   theNumberOfParameters[0] = '0';
}

void rspfNitfProjectionParameterTag::addParameter(const rspfString& param)
{
   std::ostringstream out;

   out << std::setw(15)
       << std::setfill('0')
       << param.c_str();

   theProjectionParameters.push_back(out.str());
}

void rspfNitfProjectionParameterTag::addParameter(const rspf_float64& param)
{
   addParameter(rspfString::toString(param));
}

void rspfNitfProjectionParameterTag::setName(const rspfString& name)
{
   std::ostringstream out;

   out << std::setw(80)
       << std::setfill(' ')
       << name.c_str();

   memcpy(theProjectionName, out.str().c_str(), 80);
}

void rspfNitfProjectionParameterTag::setCode(const rspfString& code)
{
   std::ostringstream out;

   out << std::setw(2)
       << std::setfill(' ')
       << code.c_str();

   memcpy(theProjectionCode, out.str().c_str(), 2);
}

rspfString rspfNitfProjectionParameterTag::getName()const
{
   return rspfString(theProjectionName).trim();
}

rspfString rspfNitfProjectionParameterTag::getCode()const
{
   return rspfString(theProjectionCode).trim();
}

void rspfNitfProjectionParameterTag::setFalseX(double falseX)
{
   std::ostringstream out;
   out << std::setw(15)
       << std::setprecision(15)
       << std::setfill('0')
       << falseX;
   
   memcpy(theFalseXOrigin, out.str().c_str(), 15);
}

void rspfNitfProjectionParameterTag::setFalseY(double falseY)
{
   std::ostringstream out;
   out << std::setw(15)
       << std::setprecision(15)
       << std::setfill('0')
       << falseY;
   
   memcpy(theFalseYOrigin, out.str().c_str(), 15);
}

double rspfNitfProjectionParameterTag::getFalseX()const
{
   return rspfString(theFalseXOrigin).toDouble();
}

double rspfNitfProjectionParameterTag::getFalseY()const
{
   return rspfString(theFalseYOrigin).toDouble();
}
