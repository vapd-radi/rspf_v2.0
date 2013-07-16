//*******************************************************************
//
// LICENSE: LGPL  see top level LICENSE.txt
//
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfTagInformation.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <rspf/support_data/rspfNitfTagInformation.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/support_data/rspfNitfTagFactoryRegistry.h>
#include <rspf/support_data/rspfNitfUnknownTag.h>
#include <sstream>
#include <iomanip>
#include <cstring> // for memset

rspfNitfTagInformation::rspfNitfTagInformation(rspfRefPtr<rspfNitfRegisteredTag> tagData)
{
   clearFields();
   setTagData(tagData);
}

rspfNitfTagInformation::~rspfNitfTagInformation()
{
}

void rspfNitfTagInformation::parseStream(std::istream &in)
{
   if(in)
   {
      theTagOffset = in.tellg();
      in.read(theTagName, 6);
      in.read(theTagLength, 5);
      theTagDataOffset = in.tellg();

      theTagData = rspfNitfTagFactoryRegistry::instance()->create(getTagName());

      if (theTagData.valid())
      {
         if (theTagData->getClassName() == "rspfNitfUnknownTag")
         {
            // Unknown tag doesn't know his tag name yet.
            theTagData->setTagName( getTagName() );
         }

         //---
         // Tags with dynamic tag length construct with 0 length.
         // Set if 0.
         //---
         if ( theTagData->getTagLength() == 0 )
         {
            theTagData->setTagLength( getTagLength() );
         }
         // Sanity check fixed length in code with length from CEL field:
         else if ( theTagData->getTagLength() != getTagLength() )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfNitfTagInformation::parseStream WARNING!"
               << "\nCEL field length does not match fixed tag length for tag: "
               << theTagData->getTagName().c_str()
               << "\nCEL: " << getTagLength()
               << "\nTag: " << theTagData->getTagLength()
               << std::endl;
         }
                               
         theTagData->parseStream(in);
      }
      else
      {
         theTagData = (rspfNitfRegisteredTag*)NULL;
      }
   }
}

void rspfNitfTagInformation::writeStream(std::ostream &out)
{
   theTagOffset = out.tellp(); // Capture the offset.
   out.write(theTagName, 6);
   out.write(theTagLength, 5);
   if(theTagData.valid())
   {
      theTagDataOffset = out.tellp();
      theTagData->writeStream(out);
   }
}

rspf_uint32 rspfNitfTagInformation::getTotalTagLength()const
{
   return (getTagLength() + (rspf_uint32)11);
}

rspf_uint32 rspfNitfTagInformation::getTagLength()const
{
   return rspfString(theTagLength).toUInt32();
}

rspf_uint64 rspfNitfTagInformation::getTagOffset()const
{
   return theTagOffset;
}

rspf_uint64 rspfNitfTagInformation::getTagDataOffset()const
{
   return theTagDataOffset;
}

rspfString rspfNitfTagInformation::getTagName()const
{
   return rspfString(theTagName).trim();
}

std::ostream& rspfNitfTagInformation::print(std::ostream& out)const
{
   out << "theTagName:          " << theTagName
       << "\ntheTagLength:        " << theTagLength
       << "\ntheTagOffset:        " << theTagOffset
       << "\ntheTagDataOffset:    " << theTagDataOffset
       << std::endl;

   return out;
}

void rspfNitfTagInformation::clearFields()
{
   memset(theTagName, ' ', 6);
   memset(theTagLength, '0', 5);

   theTagName[6]    = '\0';
   theTagLength[5]  = '\0';
   
   theTagOffset     = 0;
   theTagDataOffset = 0;
}

void rspfNitfTagInformation::setTagName(const rspfString& tagName)
{
   memset(theTagName, ' ', 6);

   std::ostringstream out;

   out << std::setw(6)
       << std::setfill(' ')
       << tagName;
   memcpy(theTagName, out.str().c_str(), 6);
}

void rspfNitfTagInformation::setTagLength(rspf_uint32 tagLength)
{
   memset(theTagLength, '0', 5);
   
   if(tagLength > 99999)
   {
      tagLength = 99999;
   }

   std::ostringstream out;

   out << std::setw(5)
       << std::setfill('0')
       << tagLength;
   
   memcpy(theTagLength, out.str().c_str(), 5);
}

rspfRefPtr<rspfNitfRegisteredTag> rspfNitfTagInformation::getTagData()
{
   return theTagData;
}

const rspfRefPtr<rspfNitfRegisteredTag> rspfNitfTagInformation::getTagData()const
{
   return theTagData;
}

void rspfNitfTagInformation::setTagData(rspfRefPtr<rspfNitfRegisteredTag> tagData)
{
   theTagData = tagData;

   memset(theTagName, ' ', 6);
   memset(theTagLength, ' ', 5);
   
   if(theTagData.valid())
   {
      setTagName(theTagData->getRegisterTagName());
      setTagLength(theTagData->getSizeInBytes());
   }
}
