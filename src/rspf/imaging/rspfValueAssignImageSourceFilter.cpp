//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfValueAssignImageSourceFilter.cpp 15833 2009-10-29 01:41:53Z eshirschorn $
#include <rspf/imaging/rspfValueAssignImageSourceFilter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfValueAssignImageSourceFilter,
          "rspfValueAssignImageSourceFilter",
          rspfImageSourceFilter);

rspfValueAssignImageSourceFilter::rspfValueAssignImageSourceFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theAssignType(rspfValueAssignType_GROUP),
    theTile(NULL)
{
}

rspfValueAssignImageSourceFilter::~rspfValueAssignImageSourceFilter()
{
}

rspfRefPtr<rspfImageData> rspfValueAssignImageSourceFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;  // This filter requires an input.
   }
   
   rspfRefPtr<rspfImageData> inputTile =
      theInputConnection->getTile(tileRect, resLevel);
   
   if(!isSourceEnabled() || !inputTile.valid())
   {
      return inputTile;
   }

   if(!theTile.valid())
   {
      allocate(); // First time through...
   }
   
   if (!theTile.valid()) // throw exeption...
   {
      return inputTile;
   }

   // Set the origin, resize if needed of the output tile.
   theTile->setImageRectangle(tileRect);

   if( inputTile->getDataObjectStatus() == RSPF_NULL ||
       inputTile->getDataObjectStatus() == RSPF_EMPTY )
   {
      theTile->makeBlank();
      return theTile;
   }

   if(!inputTile->getBuf())
   {
      theTile->makeBlank();
   }
   else
   {
      theTile->loadTile(inputTile.get());
   }

   switch(inputTile->getScalarType())
   {
      case RSPF_UCHAR:
      {
         executeAssign(static_cast<rspf_uint8>(0),
                       theTile);
         break;
      }
      case RSPF_FLOAT: 
      case RSPF_NORMALIZED_FLOAT:
      {
         executeAssign(static_cast<float>(0),
                       theTile);
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         executeAssign(static_cast<rspf_uint16>(0),
                       theTile);
         break;
      }
      case RSPF_SSHORT16:
      {
         executeAssign(static_cast<rspf_sint16>(0),
                       theTile);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         executeAssign(static_cast<double>(0),
                       theTile);
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN) << "rspfValueAssignImageSourceFilter::getTile WARN: Scalar type = " << theTile->getScalarType()
                                            << " Not supported!" << std::endl;
         break;
      }
   }

   theTile->validate();
   
   return theTile;
}

void rspfValueAssignImageSourceFilter::setInputOutputValues(const vector<double>& inputValues,
                                                             const vector<double>& outputValues)
{
   theInputValueArray  = inputValues;
   theOutputValueArray = outputValues;

   validateArrays();
}

void rspfValueAssignImageSourceFilter::validateArrays()
{
   if(theOutputValueArray.size() != theInputValueArray.size())
   {
      rspf_uint32 index = std::min((rspf_uint32)theOutputValueArray.size(),
                                    (rspf_uint32)theInputValueArray.size());
      
      vector<double> copyVector(theOutputValueArray.begin(),
                                theOutputValueArray.begin() + index);

      theOutputValueArray = copyVector;

      for(rspf_uint32 index2 = index; index < theInputValueArray.size(); ++index)
      {
         theOutputValueArray.push_back(theInputValueArray[index2]);
      }
   }
}

template <class T> void rspfValueAssignImageSourceFilter::executeAssign(
   T,
   rspfRefPtr<rspfImageData>& data)
{
   if(!theInputValueArray.size()) return;
   
   if(theAssignType == rspfValueAssignType_SEPARATE)
   {
      executeAssignSeparate(static_cast<T>(0), data);
   }
   else
   {
      executeAssignGroup(static_cast<T>(0), data);      
   }
}

template <class T> void rspfValueAssignImageSourceFilter::executeAssignSeparate(
   T,
   rspfRefPtr<rspfImageData>& data)
{
   rspf_uint32 numberOfBands = std::min((rspf_uint32)data->getNumberOfBands(),
                                         (rspf_uint32)theInputValueArray.size());
   rspf_uint32 maxOffset     = data->getWidth()*data->getHeight();
   
   for(rspf_uint32 band = 0; band<numberOfBands; ++band)
   {
      T* buf      = static_cast<T*>(data->getBuf(band));
      T  inValue  = static_cast<T>(theInputValueArray[band]);
      T  outValue = static_cast<T>(theOutputValueArray[band]);
      for(rspf_uint32 count = 0; count < maxOffset; ++count)
      {
         if(*buf == inValue)
         {
            *buf = outValue;
         }
         ++buf;
      }
   }
}

template <class T> void rspfValueAssignImageSourceFilter::executeAssignGroup(
   T,
   rspfRefPtr<rspfImageData>& data)
{
   rspf_uint32 numberOfBands = std::min((rspf_uint32)data->getNumberOfBands(),
                                         (rspf_uint32)theInputValueArray.size());
   rspf_uint32 maxOffset     = data->getWidth()*data->getHeight();
   rspf_uint32 band = 0;
   bool equalFlag = false;

   T** bufArray = new T*[data->getNumberOfBands()];
   for(band = 0; band < data->getNumberOfBands(); ++band)
   {
      bufArray[band] = static_cast<T*>(data->getBuf(band));
   }
   
   for(rspf_uint32 offset = 0; offset < maxOffset; ++offset)
   {
      equalFlag = true;
      
      for(band = 0; band<numberOfBands; ++band)
      {
         if(bufArray[band][offset] != theInputValueArray[band])
         {
            equalFlag = false;
         }
      }
      if(equalFlag)
      {
         for(band = 0; band<numberOfBands; ++band)
         {
            bufArray[band][offset] = static_cast<T>(theOutputValueArray[band]);
         }
      }
   }

   delete [] bufArray;
}

void rspfValueAssignImageSourceFilter::initialize()
{
   // Base class will recapture "theInputConnection".
   rspfImageSourceFilter::initialize();
}

void rspfValueAssignImageSourceFilter::allocate()
{   
   theTile = NULL;

   if(theInputConnection)
   {
      rspfImageDataFactory* idf = rspfImageDataFactory::instance();
      
      theTile = idf->create(this,
                            this);
      
      theTile->initialize();
   }
}

bool rspfValueAssignImageSourceFilter::saveState(rspfKeywordlist& kwl,
                                                  const char* prefix)const
{
   rspf_uint32 index = 0;
   for(index =0; index < theInputValueArray.size(); ++index)
   {
      rspfString in  = ("input_band" + rspfString::toString(index));
      rspfString out = ("output_band" + rspfString::toString(index));

      kwl.add(prefix,
              in.c_str(),
              theInputValueArray[index],
              true);
      kwl.add(prefix,
              out.c_str(),
              theOutputValueArray[index],
              true);
   }
   if(theAssignType ==  rspfValueAssignType_SEPARATE)
   {
      kwl.add(prefix,
              "assign_type",
              "separate",
              true);
   }
   else
   {
      kwl.add(prefix,
              "assign_type",
              "group",
              true);
   }
           
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

/*!
 * Method to the load (recreate) the state of an object from a keyword
 * list.  Return true if ok or false on error.
 */
bool rspfValueAssignImageSourceFilter::loadState(const rspfKeywordlist& kwl,
                                                  const char* prefix)
{
   
   rspfString inRegExpression =  rspfString("^(") +
                                  rspfString(prefix) +
                                  "input_band[0-9]+)";
   rspfString outRegExpression =  rspfString("^(") +
                                   rspfString(prefix) +
                                   "output_band[0-9]+)";
   rspf_uint32 numberOfInputs  = kwl.getNumberOfSubstringKeys(inRegExpression);
   rspf_uint32 numberOfOutputs = kwl.getNumberOfSubstringKeys(outRegExpression);
   rspf_uint32 index = 0;
   rspf_uint32 currentIndex = 0;
   
   theInputValueArray.clear();
   theOutputValueArray.clear();

   while(index < numberOfInputs)
   {
      const char* value = kwl.find(prefix,
                                   (rspfString("input_band") +
                                    rspfString::toString(currentIndex)).c_str()
                                   );
      if(value)
      {
         theInputValueArray.push_back(rspfString(value).toDouble());
         ++index;
      }
      ++currentIndex;
   }
   index = 0;
   currentIndex = 0;
   while(index < numberOfOutputs)
   {
      const char* value = kwl.find(prefix,
                                   (rspfString("output_band") +
                                    rspfString::toString(currentIndex)).c_str()
                                   );
      if(value)
      {
         theOutputValueArray.push_back(rspfString(value).toDouble());
         ++index;
      }
      ++currentIndex;
   }
   validateArrays();
   
   const char* assignType = kwl.find(prefix, "assign_type");
   if(assignType)
   {
      if(rspfString(assignType).trim().downcase() == "separate")
      {
         theAssignType = rspfValueAssignType_SEPARATE;
      }
      else
      {
         theAssignType = rspfValueAssignType_GROUP;
      }
   }
   else
   {
      theAssignType = rspfValueAssignType_GROUP;
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

const vector<double>& rspfValueAssignImageSourceFilter::getInputValues()const
{
   return theInputValueArray;
}
const vector<double>& rspfValueAssignImageSourceFilter::getOutputValues()const
{
   return theOutputValueArray;
}

rspfValueAssignImageSourceFilter::rspfValueAssignType rspfValueAssignImageSourceFilter::getValueAssignType()const
{
   return theAssignType;
}
void rspfValueAssignImageSourceFilter::setValueAssignType(rspfValueAssignType type)
{
   theAssignType = type;
}
