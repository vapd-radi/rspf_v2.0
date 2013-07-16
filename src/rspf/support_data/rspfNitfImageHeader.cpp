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
// $Id: rspfNitfImageHeader.cpp 18443 2010-11-22 23:42:39Z gpotts $

#include <cmath> /* for fmod */
#include <iomanip>
#include <sstream>

#include <rspf/support_data/rspfNitfImageHeader.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF2(rspfNitfImageHeader,
          "rspfNitfImageHeader",
          rspfObject,
          rspfPropertyInterface)
static const char* TAGS_KW = "tags";
rspfNitfImageHeader::rspfNitfImageHeader()
{
}

rspfNitfImageHeader::~rspfNitfImageHeader()
{
}

bool rspfNitfImageHeader::getTagInformation(rspfNitfTagInformation& tagInfo,
                                             const rspfString& tagName)const
{
   if(theTagList.size())
   {
      for(rspf_uint32 idx = 0; idx < theTagList.size(); ++idx)
      {
         if(theTagList[idx].getTagName() == tagName)
         {
            tagInfo = theTagList[idx];
            return true;
         }
      }
   }
   
   return false;
}

bool rspfNitfImageHeader::getTagInformation(rspfNitfTagInformation& tag,
                                             rspf_uint32 idx)const
{
   bool result = false;
   
   if(idx < theTagList.size())
   {
      tag = theTagList[idx];
      result = true;
   }
   
   return result;
}

rspfRefPtr<rspfNitfRegisteredTag> rspfNitfImageHeader::getTagData(
   const rspfString& tagName)
{
   if(theTagList.size())
   {
      for(rspf_uint32 idx = 0; idx < theTagList.size(); ++idx)
      {
         if(theTagList[idx].getTagName() == tagName)
         {
            return theTagList[idx].getTagData();
         }
      }
   }

   return rspfRefPtr<rspfNitfRegisteredTag>();
}
   
const rspfRefPtr<rspfNitfRegisteredTag> rspfNitfImageHeader::getTagData(
   const rspfString& tagName) const
{
   if(theTagList.size())
   {
      for(rspf_uint32 idx = 0; idx < theTagList.size(); ++idx)
      {
         if(theTagList[idx].getTagName() == tagName)
         {
            return theTagList[idx].getTagData();
         }
      }
   }
   
   return rspfRefPtr<rspfNitfRegisteredTag>();
}

rspf_uint32 rspfNitfImageHeader::getNumberOfTags()const
{
   return static_cast<rspf_uint32>(theTagList.size());
}

void rspfNitfImageHeader::addTag(const rspfNitfTagInformation& tag)
{
   removeTag(tag.getTagName());
   theTagList.push_back(tag);
}

void rspfNitfImageHeader::removeTag(const rspfString& tagName)
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

bool rspfNitfImageHeader::isSameAs(const rspfNitfImageHeader* hdr) const
{
   if (!hdr) return false;
   
   return ( (isCompressed() == hdr->isCompressed()) &&
            (getNumberOfRows() == hdr->getNumberOfRows()) &&
            (getNumberOfBands() == hdr->getNumberOfBands()) &&
            (getNumberOfCols() == hdr->getNumberOfCols()) &&
            (getNumberOfBlocksPerRow() == hdr->getNumberOfBlocksPerRow()) &&
            (getNumberOfBlocksPerCol() == hdr->getNumberOfBlocksPerCol()) &&
            (getNumberOfPixelsPerBlockHoriz() ==
             hdr->getNumberOfPixelsPerBlockHoriz()) &&
            (getNumberOfPixelsPerBlockVert() ==
             hdr->getNumberOfPixelsPerBlockVert()) &&
            (getBitsPerPixelPerBand() == hdr->getBitsPerPixelPerBand()) &&
            (getImageRect() == hdr->getImageRect()) &&
            (getIMode() == hdr->getIMode()) &&
            (getCoordinateSystem() == hdr->getCoordinateSystem()) &&
            (getGeographicLocation() == hdr->getGeographicLocation()) );
}


void rspfNitfImageHeader::setProperty(rspfRefPtr<rspfProperty> /* property */)
{
}

rspfRefPtr<rspfProperty> rspfNitfImageHeader::getProperty(const rspfString& name)const
{
   rspfProperty* result = 0;

   if(name == TAGS_KW)
   {
      rspf_uint32 idxMax = getNumberOfTags();
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

void rspfNitfImageHeader::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(TAGS_KW);
  
}

rspf_uint32 rspfNitfImageHeader::getTotalTagLength()const
{
   rspf_uint32 tagLength = 0;
   
   for(rspf_uint32 i = 0; i < theTagList.size(); ++i)
   {
      tagLength += theTagList[i].getTotalTagLength();
   }

   return tagLength;
}

std::ostream& rspfNitfImageHeader::printTags(std::ostream& out,
                                              const std::string& prefix)const
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

void rspfNitfImageHeader::getMetadata(rspfKeywordlist& kwl,
                                       const char* prefix) const
{
   kwl.add(prefix, "source",
           getImageSource().c_str(),
           false);
   kwl.add(prefix,
           "image_date",
           getAcquisitionDateMonthDayYear().c_str(),
           false);
   kwl.add(prefix,
           "image_title",
           getTitle().c_str(),
           false);
}

bool rspfNitfImageHeader::hasLut() const
{
   bool result = false;

   const rspf_uint32 BANDS = static_cast<rspf_uint32>(getNumberOfBands());
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const rspfRefPtr<rspfNitfImageBand> imgBand = getBandInformation(band);
      if(imgBand.valid())
      {
         rspf_uint32 luts = imgBand->getNumberOfLuts();
         if(luts > 0)
         {
            if(imgBand->getLut(0).valid())
            {
               result = true;
               break;
            }
         }
      }
   }
   
   return result;
}

rspfRefPtr<rspfNBandLutDataObject> rspfNitfImageHeader::createLut(
   rspf_uint32 bandIdx)const
{
   rspfRefPtr<rspfNBandLutDataObject> result;
   
   if(bandIdx < (rspf_uint32)getNumberOfBands())
   {
      const rspfRefPtr<rspfNitfImageBand> band = getBandInformation(bandIdx);
      if(band.valid())
      {
         rspf_uint32 bands = band->getNumberOfLuts();
         if(bands > 0)
         {
            if(band->getLut(0).valid())
            {
               rspf_uint32 entries = band->getLut(0)->getNumberOfEntries();
               
               result = new rspfNBandLutDataObject();
               result->create(entries, band->getNumberOfLuts());
               rspf_uint32 bIdx;
               rspf_uint32 entryIdx;

               for(bIdx = 0; bIdx < bands; ++bIdx)
               {
                  const rspfRefPtr<rspfNitfImageLut> lut = band->getLut(bIdx);
                  if(lut.valid())
                  {
                     for(entryIdx = 0; entryIdx < entries; ++entryIdx)
                     {
                        (*result)[entryIdx][bIdx] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(lut->getValue(entryIdx));
                     }
                  }
               }
            }
         }
      }
   }

   return result;
}


void rspfNitfImageHeader::checkForGeographicTiePointTruncation(
   const rspfDpt& tie) const
{
   // One arc second in decimal degrees.
   const rspf_float64 ARC_SECOND = 1.0/3600.0;

   // Very small number.
   const rspf_float64 FUDGE_FACTOR = 0.000000001;

   // Remainder portion of latitude divided by an arc second.
   rspf_float64 y = std::fmod(tie.y, ARC_SECOND);

   // Remainder portion of longitue divided by an arc second.
   rspf_float64 x = std::fmod(tie.x, ARC_SECOND);

   if ( (std::fabs(y) > FUDGE_FACTOR) || (std::fabs(x) > FUDGE_FACTOR) )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfImageHeader WARNING:\n"
         << "Tie point will be truncated in image header:  "
         << tie
         << std::endl;
   }
}

void rspfNitfImageHeader::getDecimationFactor(rspf_float64& result) const
{
   //---
   // Look for string like:
   // 
   // "/2" = 1/2
   // "/4  = 1/4
   // ...
   // "/16 = 1/16
   // 
   // If it is full resolution it should be "1.0"
   //
   // or
   //
   // "0.5" which is the same as "/2"
   // "0.25" which is the same as "/4"
   //---
   rspfString os = getImageMagnification();

   // Spec says to fill with spaces so strip them.
   os.trim(rspfString(" "));

   if (os.size())
   {
      if ( os.contains("/") )
      {
         os = os.after("/");
         result = os.toFloat64();
         if (result)
         {
            result = 1.0 / result;
         }
      }
      else
      {
         result = os.toFloat64();
      }
   }
   else
   {
      result = rspf::nan();
   }
}


bool rspfNitfImageHeader::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   if(!rspfObject::saveState(kwl, prefix)) return false;
   rspfString tagsPrefix = prefix;
   
   for(rspf_uint32 i = 0; i < theTagList.size(); ++i)
   {
      rspfRefPtr<rspfNitfRegisteredTag> tag = theTagList[i].getTagData();
      if (tag.valid())
      {
         if(!tag->saveState(kwl, tagsPrefix))
         {
            return false;
         }
      }
   }
   return true;
}
