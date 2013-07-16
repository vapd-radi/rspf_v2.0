//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfIndexToRgbLutFilter.cpp 17206 2010-04-25 23:20:40Z dburken $
#include <rspf/imaging/rspfIndexToRgbLutFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/base/rspfStringProperty.h>

static const rspfTrace traceDebug("rspfIndexToRgbLutFilter:debug");

RTTI_DEF1(rspfIndexToRgbLutFilter, "rspfIndexToRgbLutFilter", rspfImageSourceFilter);

static const char* MIN_VALUE_KW          = "min_value";
static const char* MAX_VALUE_KW          = "max_value";
static const char* INTERPOLATION_TYPE_KW = "interpolation_type";

rspfIndexToRgbLutFilter::rspfIndexToRgbLutFilter()
   :rspfImageSourceFilter(),
    theLut(new rspfRgbLutDataObject()),
    theMinValue(rspf::nan()),
    theMaxValue(rspf::nan()),
    theMinMaxDeltaLength(rspf::nan()),
    theMinValueOverride(false),
    theMaxValueOverride(false),
    theInterpolationType(rspfIndexToRgbLutFilter_NEAREST),
    theTile(NULL),
    theLutFile("")
{
}

rspfIndexToRgbLutFilter::rspfIndexToRgbLutFilter(rspfImageSource* inputSource,
                                                 const rspfRgbLutDataObject& lut,
                                                 double minValue,
                                                 double maxValue,
                                                 rspfIndexToRgbLutFilterInterpolationType interpolationType)
   :rspfImageSourceFilter(inputSource),
    theLut((rspfRgbLutDataObject*)lut.dup()),
    theMinValue(minValue),
    theMaxValue(maxValue),
    theMinValueOverride(false),
    theMaxValueOverride(false),
    theInterpolationType(interpolationType),
    theTile(NULL),
    theLutFile("")
{ 
   if(theMinValue > theMaxValue)
   {
      double temp = theMinValue;
      theMinValue = theMaxValue;
      theMaxValue = temp;
   }
   theMinMaxDeltaLength = theMaxValue - theMinValue;
   
}

rspfIndexToRgbLutFilter::~rspfIndexToRgbLutFilter()
{
}

rspfRefPtr<rspfImageData> rspfIndexToRgbLutFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }
   rspfRefPtr<rspfImageData> tile = theInputConnection->getTile(tileRect,
                                                                  resLevel);
   if(!tile)
   {
      return 0;
   }
   
   if(!theTile)
   {
      allocate();
      if (!theTile)
      {
         return theTile;
      }
   }

   theTile->setImageRectangle(tileRect);

   theTile->setDataObjectStatus(RSPF_FULL);
   theTile->makeBlank();

   if(tile->getBuf()&&
      tile->getDataObjectStatus() != RSPF_EMPTY)
   {
      return convertInputTile(tile);
   }

   return theTile;
   
//    if ( theTile->getDataObjectStatus() != RSPF_EMPTY )
//    {
//       theTile->makeBlank();
//    }

}

rspfRefPtr<rspfImageData> rspfIndexToRgbLutFilter::convertInputTile(const rspfRefPtr<rspfImageData>& tile)
{
   rspf_uint8* outBuf[3];
   outBuf[0] = (rspf_uint8*)(theTile->getBuf(0));
   outBuf[1] = (rspf_uint8*)(theTile->getBuf(1));
   outBuf[2] = (rspf_uint8*)(theTile->getBuf(2));
   long numberOfEntries = (long)theLut->getNumberOfEntries();

   if(!numberOfEntries)
   {
      return rspfRefPtr<rspfImageData>();
   }
   
   long maxLength = tile->getWidth()*tile->getHeight();
   rspfRgbVector color;

   switch(tile->getScalarType())
   {
      case RSPF_SSHORT16:
      {      
         rspf_sint16* buf = (rspf_sint16*)(tile->getBuf());

         for(long index = 0; index < maxLength; ++index)
         {
            if(!tile->isNull(index))
            {
               double colorIndex;
               normalizeValue(*buf, colorIndex);            
               getColorNormIndex(colorIndex, color);
               outBuf[0][index]  = color.getR();
               outBuf[1][index]  = color.getG();
               outBuf[2][index]  = color.getB();
            }
            else
            {
               outBuf[0][index] = 0;
               outBuf[1][index] = 0;
               outBuf[2][index] = 0;
            }
            ++buf;
         }
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {      
         float* buf = (float*)(tile->getBuf());
       
         for(long index = 0; index < maxLength; ++index)
	 {
            if(!tile->isNull(index))
            {
	       double colorIndex;
	       normalizeValue(*buf, colorIndex);            
	       getColorNormIndex(colorIndex, color);
	       outBuf[0][index]  = color.getR();
	       outBuf[1][index]  = color.getG();
	       outBuf[2][index]  = color.getB();
            }
            else
            {
	       outBuf[0][index] = 0;
	       outBuf[1][index] = 0;
	       outBuf[2][index] = 0;
            }
            ++buf;
	 }
         break;
      }
      case RSPF_UCHAR:
      {
         rspf_uint8* buf = (rspf_uint8*)(tile->getBuf());

         for(long index = 0; index < maxLength; ++index)
         {
            if(!tile->isNull(index))
            {
               double colorIndex;
               normalizeValue(*buf, colorIndex);
               getColorNormIndex(colorIndex, color);
	    
               outBuf[0][index]  = color.getR();
               outBuf[1][index]  = color.getG();
               outBuf[2][index]  = color.getB();
            }
            else
            {
               outBuf[0][index] = 0;
               outBuf[1][index] = 0;
               outBuf[2][index] = 0;
            }
            ++buf;
         }
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         rspf_uint16* buf = (rspf_uint16*)(tile->getBuf());

         for(long index = 0; index < maxLength; ++index)
         {
            if(!tile->isNull(index))
            {
               double colorIndex;
               normalizeValue(*buf, colorIndex);            
               getColorNormIndex(colorIndex, color);
               outBuf[0][index]  = color.getR();
               outBuf[1][index]  = color.getG();
               outBuf[2][index]  = color.getB();
            }
            else
            {
               outBuf[0][index] = 0;
               outBuf[1][index] = 0;
               outBuf[2][index] = 0;
            }
            ++buf;
         }
         break;
      }
      default:
         break;
   }
   theTile->validate();
   return theTile;
}

void rspfIndexToRgbLutFilter::allocate()
{
   if(!theInputConnection) return;

   theTile = rspfImageDataFactory::instance()->create(this, 3, this);
   if(theTile.valid())
   {
      theTile->initialize();
   }
}

void rspfIndexToRgbLutFilter::initialize()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfIndexToRgbLutFilter::initialize() DEBUG: Entered .... " << std::endl;
   }
   rspfImageSourceFilter::initialize();
   
   theTile      = NULL;
   if(!theInputConnection)
   {
      return;
   }
   if(!theMinValueOverride)
   {
      theMinValue = theInputConnection->getMinPixelValue(0);
   }
   if(!theMaxValueOverride)
   {
      theMaxValue = theInputConnection->getMaxPixelValue(0);
   }
   if(theMinValue > theMaxValue)
   {
      swap(theMinValue, theMaxValue);
   }
   theMinMaxDeltaLength = theMaxValue - theMinValue;

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "theInputConnection Pointer = " << theInputConnection
         << "\nrspfIndexToRgbLutFilter::initialize() DEBUG: Leaving .... "
         << std::endl;
   }

}

void rspfIndexToRgbLutFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(property.valid())
   {
      rspfString value = property->valueToString();
      value = value.trim();
      
      if(property->getName() == "Lut file")
      {
         setLut(rspfFilename(property->valueToString()));
      }
      else if(property->getName() == "Interpolation type")
      {
         value=value.downcase();
         if(value == "linear")
         {
            setInterpolationType(rspfIndexToRgbLutFilter_LINEAR);
         }
         else
         {
            setInterpolationType(rspfIndexToRgbLutFilter_NEAREST);
         }
      }
      else
      {
         rspfImageSourceFilter::setProperty(property);
      }
   }
}

rspfRefPtr<rspfProperty> rspfIndexToRgbLutFilter::getProperty(const rspfString& name)const
{
   if(name == "Lut file")
   {
      rspfFilenameProperty* filenameProperty = new rspfFilenameProperty(name, theLutFile);
      if(filenameProperty)
      {
         filenameProperty->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
         filenameProperty->clearChangeType();
         filenameProperty->setCacheRefreshBit();
         filenameProperty->setReadOnlyFlag(false);

         return filenameProperty;
      }
   }
   else if(name == "Interpolation type")
   {
      std::vector<rspfString> options;
      options.push_back("nearest");
      options.push_back("linear");
      rspfString value = "nearest";
      if(theInterpolationType == rspfIndexToRgbLutFilter_LINEAR)
      {
         value = "linear";
      }
      
      rspfStringProperty* stringProperty = new rspfStringProperty(name,
                                                                    value,
                                                                    false,
                                                                    options);
      
      stringProperty->clearChangeType();
      stringProperty->setCacheRefreshBit();
      stringProperty->setReadOnlyFlag(false);

      return stringProperty;
   }

   return rspfImageSourceFilter::getProperty(name);
}

void rspfIndexToRgbLutFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("Lut file");
   propertyNames.push_back("Interpolation type");
}

void rspfIndexToRgbLutFilter::normalizeValue(double value,
                                              double& normalizedValue)
{
   // map to normalized elevation within our range of values
   normalizedValue = (value - theMinValue)/theMinMaxDeltaLength;
   if(normalizedValue > 1) normalizedValue = 1;
   if(normalizedValue < 0) normalizedValue = 0;   
}

void rspfIndexToRgbLutFilter::getColorNormIndex(double index,
                                                 rspfRgbVector& result)
{
   long numberOfEntries = theLut->getNumberOfEntries();
   index*=numberOfEntries;
   if(theInterpolationType == rspfIndexToRgbLutFilter_LINEAR)
   {
      int lutIndex = (long)index;
      
      double lutT   = index - lutIndex;
      
      int lutIndex2 = lutIndex+1;
      if(lutIndex2>=numberOfEntries) lutIndex2 = numberOfEntries-1;
      
      result = (*theLut)[lutIndex]*(1.0-lutT) +
               (*theLut)[lutIndex2]*(lutT);
      
   }
   else
   {
      int i = rspf::round<int>(index);
      i = i < 0?0:i;
      i = i >numberOfEntries?numberOfEntries:i;
      result = (*theLut)[i];
   }
}

void rspfIndexToRgbLutFilter::getColor(double index,
                                        rspfRgbVector& result)
{
   if(theInterpolationType == rspfIndexToRgbLutFilter_LINEAR)
   {
      long numberOfEntries = theLut->getNumberOfEntries();
      int lutIndex = (int)index;
      
      double lutT   = index - lutIndex;
      int lutIndex2 = lutIndex+1;
      
      if(lutIndex2>=numberOfEntries) lutIndex2 = numberOfEntries-1;
      
      result = (*theLut)[lutIndex]*(1.0-lutT) +
               (*theLut)[lutIndex2]*(lutT);
   }
   else
   {
      result = (*theLut)[rspf::round<int>(index)];
   }
}

bool rspfIndexToRgbLutFilter::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{
   kwl.add(prefix,
           MIN_VALUE_KW,
           theMinValue,
           true);
   
   kwl.add(prefix,
           MAX_VALUE_KW,
           theMaxValue,
           true);
   
   rspfString interpolationType = "nearest";
   switch(theInterpolationType)
   {
      case rspfIndexToRgbLutFilter_LINEAR:
      {
         interpolationType = "linear";
      }
      default:
         break;
   }
   kwl.add(prefix,
           INTERPOLATION_TYPE_KW,
           interpolationType.c_str(),
           true);
   
   rspfString newPrefix = rspfString(prefix)+ "lut.";
   if(theLutFile != "")
   {
      kwl.add(newPrefix.c_str(), "lut_file", theLutFile.c_str(), true);
      rspfKeywordlist kwl2;
      theLut->saveState(kwl2);
   }
   else
   {
      theLut->saveState(kwl, newPrefix.c_str());
   }

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfIndexToRgbLutFilter::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
   const char* minValue = kwl.find(prefix,
                                   MIN_VALUE_KW);
   const char* maxValue = kwl.find(prefix,
                                   MAX_VALUE_KW);

   const char* interpolationType =  kwl.find(prefix,
                                             INTERPOLATION_TYPE_KW);
   
   theInterpolationType = rspfIndexToRgbLutFilter_NEAREST;
   if(interpolationType)
   {
      rspfString interp = rspfString(interpolationType).trim().upcase();
      if(interp == "LINEAR")
      {
         theInterpolationType = rspfIndexToRgbLutFilter_LINEAR;
      }
   }
   if(minValue)
   {
      theMinValue = rspfString(minValue).toDouble();
      theMinValueOverride = true;
      
   }
   else
   {
      theMinValueOverride = false;
   }
   
   if(maxValue)
   {
      theMaxValue = rspfString(maxValue).toDouble();
      theMaxValueOverride = true;
   }
   else
   {
      theMaxValueOverride = false;
   }
   if( (rspf::isnan(theMinValue) == false) &&
       (rspf::isnan(theMaxValue) == false) )
   {
      if(theMinValue > theMaxValue)
      {
         double temp = theMinValue;
         theMinValue = theMaxValue;
         theMaxValue = temp;
      }
      theMinMaxDeltaLength = theMaxValue - theMinValue;
   }
   else
   {
      theMinMaxDeltaLength = rspf::nan();
   }
   rspfString newPrefix = rspfString(prefix)+ "lut.";

   const char* file = kwl.find(newPrefix.c_str(), "lut_file");
   if(file)
   {
      theLutFile = file;
   }
   else
   {
      theLutFile = "";
   }
   theLut->loadState(kwl, newPrefix.c_str());

   bool result = rspfImageSourceFilter::loadState(kwl, prefix);

   return result;
}

rspf_uint32 rspfIndexToRgbLutFilter::getNumberOfOutputBands() const
{
   if(isSourceEnabled())
   {
      return 3;
   }
   return rspfImageSourceFilter::getNumberOfOutputBands();
}

rspfScalarType rspfIndexToRgbLutFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      return RSPF_UCHAR;
   }

   return rspfImageSourceFilter::getOutputScalarType();
}

void rspfIndexToRgbLutFilter::setLut(rspfRgbLutDataObject& lut)
{
   theLut = new rspfRgbLutDataObject(lut);
}

void rspfIndexToRgbLutFilter::setLut(const rspfFilename& file)
{
   theLutFile = file;
   if(file.exists())
   {
      rspfKeywordlist kwl(file.c_str());
      theLut->loadState(kwl);
   }
}

rspfFilename rspfIndexToRgbLutFilter::getLutFile()const
{
   return theLutFile;
}

double rspfIndexToRgbLutFilter::getMinValue()const
{
   return theMinValue;
}

double rspfIndexToRgbLutFilter::getMaxValue()const
{
   return theMaxValue;
}

void rspfIndexToRgbLutFilter::setMinValue(double value)
{
   theMinValue = value;
}

void rspfIndexToRgbLutFilter::setMaxValue(double value)
{
   theMaxValue = value;
}

double rspfIndexToRgbLutFilter::getNullPixelValue()const
{
   return 0.0;
}

double rspfIndexToRgbLutFilter::getMinPixelValue(rspf_uint32 /* band */)const
{
   return 1.0;
}

double rspfIndexToRgbLutFilter::getMaxPixelValue(rspf_uint32 /* band */)const
{
   return 255.0;
}

rspfIndexToRgbLutFilter::rspfIndexToRgbLutFilterInterpolationType rspfIndexToRgbLutFilter::getInterpolationType()const
{
   return theInterpolationType;
}

void rspfIndexToRgbLutFilter::setInterpolationType(rspfIndexToRgbLutFilterInterpolationType type)
{
   theInterpolationType = type;
}
