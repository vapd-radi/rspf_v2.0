//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNBandLutDataObject.cpp 19732 2011-06-06 22:24:54Z dburken $

#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfNBandLutDataObject.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfNotifyContext.h>

#include <iostream>
#include <sstream>

RTTI_DEF1(rspfNBandLutDataObject, "rspfNBandLutDataObject", rspfObject);

std::ostream& operator <<(std::ostream& out,
                          const rspfNBandLutDataObject& lut)
{
   if(lut.theLut)
   {
      out << rspfKeywordNames::NUMBER_ENTRIES_KW << ": "
          << lut.theNumberOfEntries << std::endl;
      out << rspfKeywordNames::NUMBER_BANDS_KW   << ": "
          << lut.theNumberOfBands << std::endl;
      out << "null_pixel_index: " << lut.theNullPixelIndex << std::endl;
      for(rspf_uint32 idx = 0; idx < lut.theNumberOfEntries; ++idx)
      {
         const rspfNBandLutDataObject::LUT_ENTRY_TYPE *bandPtr =lut[idx];
         rspf_uint32 bandIdx = 0;
         out << rspfKeywordNames::ENTRY_KW << idx << ": ";
         for(bandIdx = 0; bandIdx < lut.theNumberOfBands; ++bandIdx)
         {
            out << bandPtr[bandIdx] << " ";
         }
         if(idx < (lut.theNumberOfEntries-1))
         {
            out << std::endl;
         }
      }
   }
   return out;
}

rspfNBandLutDataObject::rspfNBandLutDataObject(rspf_uint32 numberOfEntries,
                                                 rspf_uint32 numberOfBands,
                                                 rspfScalarType bandScalarType,
                                                 rspf_int32 nullPixelIndex)
   :theLut(0),
    theNumberOfEntries(0),
    theNumberOfBands(0),
    theBandScalarType(bandScalarType),
    theNullPixelIndex(nullPixelIndex)
{
   create(numberOfEntries, numberOfBands);
}

rspfNBandLutDataObject::rspfNBandLutDataObject(const rspfNBandLutDataObject& lut)
   :theLut(0),
    theNumberOfEntries(lut.theNumberOfEntries),
    theNumberOfBands(lut.theNumberOfBands),
    theBandScalarType(lut.theBandScalarType),
    theNullPixelIndex(lut.theNullPixelIndex),
    m_entryLabels(lut.m_entryLabels)
{
   *this = lut;
}

rspfNBandLutDataObject::~rspfNBandLutDataObject()
{
   if(theLut)
   {
      delete [] theLut;
      theLut = 0;
   }
   theNumberOfEntries = 0;
   theNumberOfBands   = 0;
   m_entryLabels.clear();
}

void rspfNBandLutDataObject::create(rspf_uint32 numberOfEntries,
                                     rspf_uint32 numberOfBands)
{
   if(theLut)
   {
      delete [] theLut;
      theLut = 0;
   }
   if(numberOfEntries&&numberOfBands)
   {
      theLut = new rspfNBandLutDataObject::LUT_ENTRY_TYPE[numberOfEntries*numberOfBands];
      theNumberOfEntries = numberOfEntries;
      theNumberOfBands   = numberOfBands;
   }
   else
   {
      theNumberOfEntries = 0;
      theNumberOfBands   = 0;
   }
}

rspf_uint32 rspfNBandLutDataObject::findIndex(
   rspfNBandLutDataObject::LUT_ENTRY_TYPE* bandValues)const
{
   return findIndex(bandValues, theNumberOfBands);
}

rspf_uint32 rspfNBandLutDataObject::findIndex(
   rspfNBandLutDataObject::LUT_ENTRY_TYPE* bandValues, rspf_uint32 size)const
{
   rspf_uint32  result = 0;

   if ( (theNumberOfEntries > 0) && (size <= theNumberOfBands) )
   {
      rspf_float64 distance = 1.0/DBL_EPSILON; 
      rspf_uint32 idx = 0;
      rspf_uint32 bandIdx = 0;
      rspfNBandLutDataObject::LUT_ENTRY_TYPE* lutPtr = theLut;

      for(idx = 0; idx < theNumberOfEntries; ++idx,lutPtr+=theNumberOfBands)
      {
         rspf_float64 sumSquare = 0.0;
         
         for(bandIdx = 0; bandIdx < size; ++bandIdx)
         {
            rspf_int64 delta = lutPtr[bandIdx] - bandValues[bandIdx];
            sumSquare += (delta*delta);
         }
         if((rspfNBandLutDataObject::LUT_ENTRY_TYPE)sumSquare == 0)
         {
            return idx;
         }
         else if( sumSquare < distance)
         {
            result = idx;
            distance = sumSquare;
         }
      }
   }

   return result;
}

void rspfNBandLutDataObject::clearLut()
{
   if(theLut)
   {
      memset(theLut, '\0', theNumberOfEntries*theNumberOfBands*sizeof(rspfNBandLutDataObject::LUT_ENTRY_TYPE));
   }
}

void rspfNBandLutDataObject::getMinMax(rspf_uint32 band,
                                        rspfNBandLutDataObject::LUT_ENTRY_TYPE& minValue,
                                        rspfNBandLutDataObject::LUT_ENTRY_TYPE& maxValue)
{
   minValue = 0;
   maxValue = 0;
   rspf_uint32 idx = 0;
   LUT_ENTRY_TYPE *bandPtr = theLut+band;
   if((band < theNumberOfBands)&&
      (theNumberOfEntries > 0))
   {
      minValue = theLut[band];
      maxValue = theLut[band];
      
      for(idx = 0; idx < theNumberOfEntries; ++idx,bandPtr+=theNumberOfBands)
      {
         if((rspf_int32)idx != theNullPixelIndex)
         {
            if(bandPtr[band] < minValue)
            {
               minValue = bandPtr[band];
            }
            if(bandPtr[band] > maxValue)
            {
               maxValue = bandPtr[band];
            }
         }
      }
   }
}

rspf_int32 rspfNBandLutDataObject::getFirstNullAlphaIndex() const
{
   rspf_int32 result = -1;
   if ( (theNumberOfBands == 4) &&  (theNumberOfEntries > 0) )
   {
      rspf_uint32 idx = 0;
      LUT_ENTRY_TYPE* bandPtr = theLut+3; // Index to the first alpha channel.
      for ( idx = 0; idx < theNumberOfEntries; ++idx, bandPtr+=theNumberOfBands )
      {
         if ( *bandPtr == 0 )
         {
            result = *bandPtr;
            break;
         }
      }
   }
   return result;
}

const rspfNBandLutDataObject& rspfNBandLutDataObject::operator =(const rspfNBandLutDataObject& lut)
{
   if(theNumberOfEntries != lut.theNumberOfEntries)
   {
      delete [] theLut;
      theLut = 0;
   }

   theNullPixelIndex = lut.theNullPixelIndex;
   theBandScalarType = lut.theBandScalarType;
   create(lut.theNumberOfEntries,
          lut.theNumberOfBands);

   if(theLut)
   {
      memcpy(theLut, lut.theLut, (theNumberOfEntries*theNumberOfBands)*sizeof(rspfNBandLutDataObject::LUT_ENTRY_TYPE));
   }

   return *this;
}

bool rspfNBandLutDataObject::operator ==(const rspfNBandLutDataObject& lut)const
{
   if(theNumberOfEntries != lut.theNumberOfEntries)
   {
      return false;
   }

   if(!theLut && !lut.theLut) return true;
   if(theNullPixelIndex != lut.theNullPixelIndex) return false;
   if(theBandScalarType != lut.theBandScalarType) return false;
   
   if(theLut&&lut.theLut)
   {
      return (memcmp(theLut, lut.theLut, theNumberOfEntries*theNumberOfBands*sizeof(rspfNBandLutDataObject::LUT_ENTRY_TYPE)) == 0);
   }
   return false;
}

bool rspfNBandLutDataObject::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           getClassName(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_ENTRIES_KW,
           rspfString::toString(theNumberOfEntries).c_str(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_BANDS_KW,
           theNumberOfBands,
           true);
   kwl.add(prefix,
           rspfKeywordNames::NULL_VALUE_KW,
           theNullPixelIndex,
           true);
   kwl.add(prefix,
           rspfKeywordNames::SCALAR_TYPE_KW,
           rspfScalarTypeLut::instance()->getEntryString(theBandScalarType),
           true);
          
   rspfNBandLutDataObject::LUT_ENTRY_TYPE* lutPtr = theLut;
   for(rspf_uint32 idx = 0; idx < theNumberOfEntries; ++idx, lutPtr+=theNumberOfBands)
   {
      rspfString newPrefix = rspfKeywordNames::ENTRY_KW;
      newPrefix += rspfString::toString(idx);
      std::ostringstream ostr;
      rspf_uint32 bandIdx = 0;
      for(bandIdx = 0; bandIdx < theNumberOfBands; ++bandIdx)
      {
         ostr << lutPtr[bandIdx]
              << " ";
      }
      kwl.add(prefix,
              newPrefix,
              ostr.str().c_str(),
              true);
   }

   return true;
}

bool rspfNBandLutDataObject::open(const rspfFilename& lutFile)
{
   rspfKeywordlist kwl;
   kwl.addFile(lutFile);
   
   return loadState(kwl);
}

bool rspfNBandLutDataObject::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   const char* nullPixelIndex = kwl.find(prefix, rspfKeywordNames::NULL_VALUE_KW);
   const char* lutFile = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   rspfKeywordlist fileLut;
   const rspfKeywordlist* tempKwl = &kwl;
   rspfString tempPrefix = prefix;

   // check to see if we should open an external file
   // if so point the fileLut to the one that we use
   if(lutFile)
   {
      rspfFilename filename(lutFile);
      if(filename.exists())
      {
         fileLut.addFile(filename.c_str());
         tempKwl = &fileLut;
         tempPrefix = "";
      }
   }

   if(nullPixelIndex)
   {
      theNullPixelIndex = rspfString(nullPixelIndex).toInt32();
   }
   else
   {
      theNullPixelIndex = -1;
   }
   int scalar = rspfScalarTypeLut::instance()->getEntryNumber(kwl, prefix);

   if (scalar != rspfLookUpTable::NOT_FOUND)
   {
      theBandScalarType = static_cast<rspfScalarType>(scalar);
   }
   const char* numberOfBands   = tempKwl->find(tempPrefix, rspfKeywordNames::NUMBER_BANDS_KW);
   const char* numberOfEntries = tempKwl->find(tempPrefix, rspfKeywordNames::NUMBER_ENTRIES_KW);

   if (numberOfBands && numberOfEntries)
   {
      create(rspfString(numberOfEntries).toUInt32(),
             rspfString(numberOfBands).toUInt32());
   }
   
   clearLut();
   rspfNBandLutDataObject::LUT_ENTRY_TYPE* lutPtr = theLut;

   if(lutPtr)
   {
      rspf_uint32 entryIdx;
      rspf_uint32 bandIdx;
      for(entryIdx = 0; entryIdx < theNumberOfEntries; ++entryIdx)
      {
         rspfString newPrefix = "entry";
         newPrefix += rspfString::toString(entryIdx);
         rspfString v = tempKwl->find(tempPrefix, newPrefix.c_str());
         v = v.trim();
         if(v != "")
         {
            std::istringstream istr(v);
            rspfString lutValue;
            for(bandIdx = 0; bandIdx < theNumberOfBands; ++bandIdx)
            {
               if(!istr.fail())
               {
                  istr >> lutValue;
                  lutPtr[bandIdx] = lutValue.toInt32();
               }
            }
         }
         lutPtr += theNumberOfBands;
      }
   }
   
   return true;
}

std::vector<rspfString> rspfNBandLutDataObject::getEntryLabels(rspf_uint32 band)
{
   std::map<rspf_uint32, std::vector<rspfString> >::iterator it = m_entryLabels.find(band);
   if (it != m_entryLabels.end())
   {
      return it->second;
   }
   return std::vector<rspfString>();
}

void rspfNBandLutDataObject::setEntryLables(rspf_uint32 band, std::vector<rspfString> entryLabels)
{
   m_entryLabels[band] = entryLabels;
}
