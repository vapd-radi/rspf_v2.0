//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class definition for rspfScalarRemapper.
// This class is used to remap image data from one scalar type to another.
//
//*******************************************************************
//  $Id: rspfScalarRemapper.cpp 22135 2013-02-02 16:27:24Z dburken $

#include <iostream>

#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfPropertyEvent.h>
#include <rspf/base/rspfRefreshEvent.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfStringProperty.h>

RTTI_DEF1(rspfScalarRemapper,
          "rspfScalarRemapper",
          rspfImageSourceFilter)

static const rspfTrace traceDebug("rspfScalarRemapper:debug");
   
rspfScalarRemapper::rspfScalarRemapper()
   :
      rspfImageSourceFilter(),
      theNormBuf(NULL),
      theTile(NULL),
      theOutputScalarType(RSPF_UINT8),
      theByPassFlag(false)
{
}

rspfScalarRemapper::rspfScalarRemapper(rspfImageSource* inputSource,
                                         rspfScalarType outputScalarType)
   :
      rspfImageSourceFilter(inputSource),
      theNormBuf(NULL),
      theTile(NULL),
      theOutputScalarType(outputScalarType),
      theByPassFlag(false)
{
   if(inputSource)
   {
      if (inputSource->getOutputScalarType() == outputScalarType)
      {
         // Disable this filter simply return the inputSource's data.
         theByPassFlag = true;
         
         // Nothing else to do.
      }
      else
      {
         theByPassFlag = false;
      }
   }
   else
   {
      theByPassFlag = true;
   }
}

rspfScalarRemapper::~rspfScalarRemapper()
{
   destroy();
}

void rspfScalarRemapper::destroy()
{
   if (theNormBuf)
   {
      delete [] theNormBuf;
      theNormBuf = NULL;
   }
   theTile      = NULL;
}

rspfRefPtr<rspfImageData> rspfScalarRemapper::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }

   // Fetch tile from pointer from the input source.
   rspfRefPtr<rspfImageData> inputTile =
      theInputConnection->getTile(tileRect, resLevel);

   // Check for remap bypass:
   if ( !isSourceEnabled()||theByPassFlag )
   {
      return inputTile;
   }

   // Check for first time through.
   if ( !theTile.valid() )
   {
      allocate();
      
      if ( !theTile.valid() )
      {
         // This can happen if input/output scalars are the same.
         return inputTile;
      }
   }

   // Capture the size prior to a possible resize.
   rspf_uint32 oldSize = theTile->getSize();

   // Set the origin,bands of the output tile.
   theTile->setImageRectangle(tileRect);

   rspf_uint32 newSize = theTile->getSize();

   // Check for size change before possible return.
   if(newSize != oldSize)
   {
      if(theNormBuf)
      {
         //---
         // Delete the current buffer since it is the wrong size.
         // 
         // NOTE:
         // We won't reallocate it yet since we could return without using it.
         // It will be checked prior to using later.
         //---
         delete [] theNormBuf;
         theNormBuf = NULL;
      }
   }
   
   if ( !inputTile.valid() ||
        (inputTile->getDataObjectStatus() == RSPF_NULL) ||
        (inputTile->getDataObjectStatus() == RSPF_EMPTY) )
   {
      //---
      // Since the filter is enabled, return theTile which is of the
      // correct scalar type.
      //---
      theTile->makeBlank();
      return theTile;
   }

   if (!theNormBuf) // First time through or size changed and was deleted...
   {
      theNormBuf = new double[newSize];
      memset(theNormBuf, '\0', newSize);
   }

   if (inputTile->getScalarType() == theOutputScalarType)
   {
      // Scalar types already the same.  Nothing to do...
      return inputTile;
   }
   
   switch(inputTile->getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         // Un-normalize and copy the buffer to the destination tile.
         theTile->copyNormalizedBufferToTile(
            static_cast<double*>( inputTile->getBuf() ) );
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         // Un-normalize and copy the buffer to the destination tile.
         theTile->copyNormalizedBufferToTile(
            static_cast<float*>( inputTile->getBuf() ) );
         break;
      }
      default:
      {
         //---
         // NOTE: stretchMinMax commented out as it was incorrectly not resetting
         // the tile's min/max data members; hence, messing up the downstream copy
         // to normalized buffer. (drb 02 Feb. 2013)
         // Special case.  Stretch assuming caller want to view this data.
         //---
         // inputTile->stretchMinMax();

         // Normalize and copy the source tile to a buffer.
         inputTile->copyTileToNormalizedBuffer(theNormBuf);
         
         // Un-normalize and copy the buffer to the destination tile.
         theTile->copyNormalizedBufferToTile(theNormBuf);

         break;
      }
   }
   
   theTile->validate();
   
   return theTile;
}

rspfScalarType rspfScalarRemapper::getOutputScalarType() const
{
   if(isSourceEnabled()&&!theByPassFlag)
   {
      return theOutputScalarType;
   }

   return rspfImageSourceFilter::getOutputScalarType();
}

void rspfScalarRemapper::setOutputScalarType(rspfScalarType scalarType)
{
   if (scalarType == RSPF_SCALAR_UNKNOWN)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
         << "rspfScalarRemapper::setOutputScalarType WARN:\n"
         << "RSPF_SCALAR_UNKNOWN passed to method.  No action taken..."
         << std::endl;
      }
      return;
   }
   
   if (theInputConnection)
   {
      if ( scalarType == theInputConnection->getOutputScalarType() )
      {
         // Input same as output, nothing for us to do...
         theByPassFlag = true;
      }
      else // Types not equal...
      {
         theByPassFlag = false;
         destroy();
      }
   }
   else // No input source, disable.
   {
      theByPassFlag = true;
   }

   theOutputScalarType = scalarType;
}

void rspfScalarRemapper::setOutputScalarType(rspfString scalarType)
{
   int scalar =
      rspfScalarTypeLut::instance()->getEntryNumber(scalarType.c_str());
   
   if (scalar != rspfLookUpTable::NOT_FOUND)
   {
      setOutputScalarType(static_cast<rspfScalarType>(scalar));
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
         << "rspfScalarRemapper ERROR:"
         << "\nUnknown scalar type:  " << scalarType.c_str() << std::endl;
      }
   }
}

void rspfScalarRemapper::initialize()
{
   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   rspfImageSourceFilter::initialize();

   if (theInputConnection)
   {
      // Set the bypass flag accordingly...
      if ( theInputConnection->getOutputScalarType() == 
           theOutputScalarType )
      {
         theByPassFlag = true;
      }
      else
      {
         theByPassFlag = false;
      }
      
      if (theTile.valid())
      {
         //---
         // Check for:
         // - bypass
         // - disabled(!enabled)
         // - scalar change
         // - band count change
         //---
         if ( theByPassFlag ||
              !theEnableFlag ||
              ( theInputConnection->getOutputScalarType() !=
                theOutputScalarType ) ||
              ( theInputConnection->getNumberOfOutputBands() !=
                theTile->getNumberOfBands() ) )
         {
            destroy();  // Reallocated first unbypassed getTile.
         }
      }
   }
}

void rspfScalarRemapper::allocate()
{
   destroy();

   if(!theInputConnection) // Nothing to do here.
   {
      setInitializedFlag(false);
      theByPassFlag = true;
      return;
   }
   
   if (theOutputScalarType == RSPF_SCALAR_UNKNOWN)
   {
      // default to RSPF_UINT8
      theOutputScalarType = RSPF_UINT8;
   }
   
   if(theInputConnection &&
      (getOutputScalarType() != theInputConnection->getOutputScalarType())&&
      (theInputConnection->getOutputScalarType() != RSPF_SCALAR_UNKNOWN)&&
      (getOutputScalarType() != RSPF_SCALAR_UNKNOWN))
   {
      theByPassFlag = false;
      
      theTile = rspfImageDataFactory::instance()->create(this, this);

      // Initialize the tile.
      theTile->initialize();
      
      // Set the base class flags to be initialized and enabled.
      setInitializedFlag(true);
      
   } // End of "if(theInputConnection->isConnected()..."
   else
   {
      // Set to not initialized and disabled.
      setInitializedFlag(false);
      theByPassFlag = true;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfScalarRemapper::allocate() DEBUG"
         << "\ninput scalar:  " << theInputConnection->getOutputScalarType()
         << "\noutput scalar: " << getOutputScalarType()
         << "\nenabled:  " << (isSourceEnabled()?"true":"false")
         << std::endl;
   }
}

void rspfScalarRemapper::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if(property->getName() == "Output scalar type")
   {
      theOutputScalarType = rspfScalarTypeLut::instance()->
         getScalarTypeFromString(property->valueToString());
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfScalarRemapper::getProperty(const rspfString& name)const
{
   if(name == "Output scalar type")
   {
      std::vector<rspfString> scalarNames;

      rspf_int32 tableSize = (rspf_int32)rspfScalarTypeLut::instance()->
         getTableSize();
      rspf_int32 idx;

      for(idx = 0; idx < tableSize; ++idx)
      {
         scalarNames.push_back(rspfScalarTypeLut::instance()->
                               getEntryString(idx));
      }
      rspfStringProperty* stringProp =
         new rspfStringProperty("Output scalar type",
                                 rspfScalarTypeLut::instance()->getEntryString((rspf_int32)theOutputScalarType),
                                 false,
                                 scalarNames);
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }

   return rspfImageSourceFilter::getProperty(name);
}

void rspfScalarRemapper::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("Output scalar type");
}

bool rspfScalarRemapper::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   rspfImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,
           rspfKeywordNames::SCALAR_TYPE_KW,
           rspfScalarTypeLut::instance()->getEntryString(theOutputScalarType),
           true);

   return true;
}

bool rspfScalarRemapper::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   rspfImageSourceFilter::loadState(kwl, prefix);

   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfScalarRemapper::loadState\n"
            << " ERROR detected in keyword list!  State not loaded."
            << std::endl;
      }
      return false;
   }

   int scalar = rspfScalarTypeLut::instance()->getEntryNumber(kwl, prefix);

   if (scalar != rspfLookUpTable::NOT_FOUND)
   {
      setOutputScalarType(static_cast<rspfScalarType>(scalar));
   }

   return true;
}

rspfString rspfScalarRemapper::getOutputScalarTypeString() const
{
   return rspfScalarTypeLut::instance()->getEntryString(theOutputScalarType);
}

void rspfScalarRemapper::propertyEvent(rspfPropertyEvent& event)
{
   // if my properties have changed then just initialize
   //
   if(event.getObject() == this)
   {
      initialize();
   }
   else // if an input property has changed just check to see if the number
   {    // of bands has changed
      
      if(!theTile)
      {
         initialize();
      }
      else
      {
         int b = theInputConnection->getNumberOfOutputBands();
         if((int)theTile->getNumberOfBands() != b)
         {
            initialize();
         }
      }
   }
}

void rspfScalarRemapper::refreshEvent(rspfRefreshEvent& event)
{
   // if my properties have changed then just initialize
   if(event.getObject() == this)
   {
      initialize();
   }
   else // if an input property has changed just check to see if the number
   {    // of bands has changed
      
      if(!theTile)
      {
         initialize();
      }
      else
      {
         int b = theInputConnection->getNumberOfOutputBands();
         if((int)theTile->getNumberOfBands() != b)
         {
            initialize();
         }
      }
   }
}

double rspfScalarRemapper::getNullPixelValue(rspf_uint32 band) const
{
   if(!isSourceEnabled()||theByPassFlag)
   {
      if(theInputConnection)
      {
         return theInputConnection->getNullPixelValue(band);
      }
   }
   else if(theTile.valid())
   {
      if (band < theTile->getNumberOfBands())
      {
         return theTile->getNullPix(band);
      }
   }
   
   return rspf::defaultNull(theOutputScalarType);
}

double rspfScalarRemapper::getMinPixelValue(rspf_uint32 band) const
{
   if(!isSourceEnabled()||theByPassFlag)
   {
      if(theInputConnection)
      {
         return theInputConnection->getMinPixelValue(band);
      }
   }
   else if(theTile.valid())
   {
      if (band < theTile->getNumberOfBands())
      {
         return theTile->getMinPix(band);
      }
   }
   
   return rspf::defaultMin(theOutputScalarType);
}

double rspfScalarRemapper::getMaxPixelValue(rspf_uint32 band) const
{
   if(!isSourceEnabled()||theByPassFlag)
   {
      if(theInputConnection)
      {
         return theInputConnection->getMaxPixelValue(band);
      }
   }
   else if(theTile.valid())
   {
      if (band < theTile->getNumberOfBands())
      {
         return theTile->getMaxPix(band);
      }
   }
   
   return rspf::defaultMax(theOutputScalarType);
}

rspfString rspfScalarRemapper::getLongName()const
{
   return rspfString("Scalar Remapper, filters between different scalar types.");
}

rspfString rspfScalarRemapper::getShortName()const
{
   return rspfString("Scalar Remapper");
}

