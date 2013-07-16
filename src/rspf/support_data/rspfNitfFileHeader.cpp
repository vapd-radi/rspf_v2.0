//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfFileHeader.cpp 19043 2011-03-10 15:24:24Z dburken $
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/base/rspfContainerProperty.h>
#include <iostream>
#include <sstream>

static const char* TAGS_KW = "tags";

RTTI_DEF2(rspfNitfFileHeader, "rspfNitfFileHeader", rspfObject, rspfPropertyInterface);

std::ostream& rspfNitfImageOffsetInformation::print(std::ostream& out) const
{
   return out << "theImageHeaderOffset:          "
              << theImageHeaderOffset << std::endl
              << "theImageDataOffset:            "
              << theImageDataOffset;
}

rspfNitfFileHeader::rspfNitfFileHeader()
{
}

rspfNitfFileHeader::~rspfNitfFileHeader()
{
}


bool rspfNitfFileHeader::getTag(rspfNitfTagInformation& tagInfo,
                                 const rspfString& tagName)const
{
   if(theTagList.size())
   {
      for(rspf_uint32 i = 0; i < theTagList.size(); ++i)
      {
         if(theTagList[i].getTagName() == tagName)
         {
            tagInfo = theTagList[i];
            return true;
         }
      }
   }

   return false;
}

bool  rspfNitfFileHeader::hasImages()const
{
   return (getNumberOfImages() > 0);
}

bool  rspfNitfFileHeader::hasSymbols()const
{
   return (getNumberOfSymbols() > 0);
}

bool  rspfNitfFileHeader::hasGraphics()const
{
   return (getNumberOfGraphics() > 0);
}

bool  rspfNitfFileHeader::hasText()const
{
   return (getNumberOfTextSegments() > 0);
}

bool  rspfNitfFileHeader::hasLabels()const
{
   return (getNumberOfLabels() > 0);
}
bool rspfNitfFileHeader::hasDataExtSegments()const
{
   return (getNumberOfDataExtSegments() > 0);
}

void rspfNitfFileHeader::removeTag(const rspfString& tagName)
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theTagList.size(); ++idx)
   {
      if(theTagList[idx].getTagName() == tagName)
      {
         theTagList.erase(theTagList.begin() + idx);
         return;
      }
   }
}
void rspfNitfFileHeader::addTag(const rspfNitfTagInformation& tag)
{
   removeTag(tag.getTagName());
   theTagList.push_back(tag);
}

bool rspfNitfFileHeader::getTagInformation(rspfNitfTagInformation& tag,
                                            int idx) const
{
   bool result = false;
   
   if(static_cast<rspf_uint32>(idx) < theTagList.size())
   {
      tag = theTagList[idx];
      result = true;
   }
   
   return result;
}

int rspfNitfFileHeader::getNumberOfTags()const
{
   return (int)theTagList.size();
}

rspf_uint32 rspfNitfFileHeader::getTotalTagLength()const
{
   rspf_uint32 tagLength = 0;
   
   for(rspf_uint32 i = 0; i < theTagList.size(); ++i)
   {
      tagLength += theTagList[i].getTotalTagLength();
   }
   
   return tagLength;
}


void rspfNitfFileHeader::setProperty(rspfRefPtr<rspfProperty> /* property */)
{
}

rspfRefPtr<rspfProperty> rspfNitfFileHeader::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;

   if(name == TAGS_KW)
   {
      rspf_uint32 idxMax = (rspf_uint32)theTagList.size();
      if(idxMax > 0)
      {
         rspfContainerProperty* containerProperty = new rspfContainerProperty;
         containerProperty->setName(name);
         rspf_uint32 idx = 0;

         result = containerProperty;
         for(idx = 0; idx < idxMax; ++idx)
         {
            rspfNitfTagInformation tagInfo;
            getTagInformation(tagInfo, idx);
            
            const rspfRefPtr<rspfNitfRegisteredTag> tagData = tagInfo.getTagData();
            if(tagData.valid())
            {
               rspfContainerProperty* containerPropertyTag = new rspfContainerProperty;
               containerPropertyTag->setName(tagInfo.getTagName());
               std::vector<rspfRefPtr<rspfProperty> > propertyList;
               tagData->getPropertyList(propertyList);
               
               containerPropertyTag->addChildren(propertyList);
               containerProperty->addChild(containerPropertyTag);
            }
            else
            {
               containerProperty->addStringProperty(tagInfo.getTagName(), "", true);
            }
         }
      }
   }
   
   return result;
}

void rspfNitfFileHeader::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(TAGS_KW);
}

bool rspfNitfFileHeader::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   rspfObject::saveState(kwl, prefix);
   
   bool result = true;
   rspfString tagsPrefix = prefix;
   for(rspf_uint32 i = 0; i < theTagList.size(); ++i)
   {
      rspfRefPtr<rspfNitfRegisteredTag> tag = theTagList[i].getTagData();
      if (tag.valid())
      {
         // we eventually need to do our own prefix for this object and not let the calling object do any prefix definitions
//         rspfString newPrefix = tagsPrefix + tag->getRegisterTagName() + ".";

         tag->saveState(kwl, tagsPrefix);
      }
   }
   
   return result;
}

std::ostream& rspfNitfFileHeader::print(std::ostream& out,
                                         const std::string& prefix ) const
{
   return printTags(out, prefix);
}

std::ostream& rspfNitfFileHeader::printTags(std::ostream& out,
                                             const std::string& prefix) const
{
   for(rspf_uint32 i = 0; i < theTagList.size(); ++i)
   {
      rspfRefPtr<rspfNitfRegisteredTag> tag = theTagList[i].getTagData();
      if (tag.valid())
      {
         tag->print(out, prefix);
      }
   }

   return out;
}
