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
// $Id: rspfCastTileSourceFilter.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/imaging/rspfCastTileSourceFilter.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfStringProperty.h>

RTTI_DEF1( rspfCastTileSourceFilter,
           "rspfCastTileSourceFilter",
           rspfImageSourceFilter );

rspfCastTileSourceFilter::rspfCastTileSourceFilter(rspfImageSource* input,
						     rspfScalarType scalarType)
   :rspfImageSourceFilter(NULL,input),
    theTile(NULL),
    theOutputScalarType(scalarType)
{
}

rspfCastTileSourceFilter::~rspfCastTileSourceFilter()
{
}

rspfRefPtr<rspfImageData> rspfCastTileSourceFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> inputTile;
   
   if(theInputConnection)
   {
      inputTile = theInputConnection->getTile(tileRect, resLevel);
   }
   return applyCast(inputTile);
}

rspfScalarType rspfCastTileSourceFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      return theOutputScalarType;
   }
   else if(theInputConnection)
   {
      return theInputConnection->getOutputScalarType();
   }

   return RSPF_SCALAR_UNKNOWN;
}

double rspfCastTileSourceFilter::getNullPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      if(theInputConnection)
      {
         return theInputConnection->getNullPixelValue(band);
      }
   }
   return rspf::defaultNull(getOutputScalarType());
}

double rspfCastTileSourceFilter::getMinPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      if(theInputConnection)
      {
         return theInputConnection->getMinPixelValue(band);
      }
   }

   if(theInputConnection)
   {
      double tempMin    = theInputConnection->getMinPixelValue(band);
      
      double defaultMin = rspf::defaultMin(theOutputScalarType);
      double defaultMax = rspf::defaultMax(theOutputScalarType);

      if((tempMin >= defaultMin) && (tempMin <= defaultMax))
      {
         defaultMin = tempMin;
      }

      return defaultMin;
   }
   
   return rspfImageSource::getMinPixelValue(band);
}

double rspfCastTileSourceFilter::getMaxPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      if(theInputConnection)
      {
         return theInputConnection->getMaxPixelValue(band);
      }
   }
   if(theInputConnection)
   {
      double tempMax    = theInputConnection->getMaxPixelValue(band);
      
      double defaultMax = rspf::defaultMax(theOutputScalarType);
      double defaultMin = rspf::defaultMin(theOutputScalarType);

      if((tempMax >= defaultMin) && (tempMax <= defaultMax))
      {
         defaultMax = tempMax;
      }

      return defaultMax;
   }
   return rspfImageSource::getMaxPixelValue(band);
}

rspfRefPtr<rspfImageData> rspfCastTileSourceFilter::applyCast(
   rspfRefPtr<rspfImageData> inputTile)
{
   if(!inputTile.valid())
   {
      return inputTile;
   }
   
   if(!theTile.valid() ||
      (theTile->getNumberOfBands() != inputTile->getNumberOfBands()))
   {
      allocate(inputTile);
   }
   else
   {
      rspf_int32 tw =inputTile->getWidth(); 
      rspf_int32 th =inputTile->getHeight(); 
      rspf_int32 w = theTile->getWidth();
      rspf_int32 h = theTile->getHeight();
      theTile->setWidthHeight(tw, th);
      
      if((w*h != tw*th))
      {
         theTile->initialize();
      }
      else
      {
         theTile->makeBlank();
      }
      theTile->setOrigin(inputTile->getOrigin());
   }
   
   if( (inputTile->getDataObjectStatus()==RSPF_EMPTY) ||
       (inputTile->getDataObjectStatus()==RSPF_NULL))
   {
      return theTile;
   }
   
   switch(inputTile->getScalarType())
   {
      case RSPF_UINT8:
      {
         rspf_uint8** bands = new rspf_uint8*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<rspf_uint8*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_SINT8:
      {
         rspf_sint8** bands = new rspf_sint8*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<rspf_sint8*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         rspf_uint16** bands =
            new rspf_uint16*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<rspf_uint16*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_SINT16:
      {
         rspf_sint16** bands =
            new rspf_sint16*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<rspf_sint16*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_SINT32:
      {
         rspf_sint32** bands =
            new rspf_sint32*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<rspf_sint32*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_UINT32:
      {
         rspf_uint32** bands =
            new rspf_uint32*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<rspf_uint32*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_FLOAT32: 
      case RSPF_NORMALIZED_FLOAT:
      {
         float** bands = new float*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<float*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         double** bands = new double*[inputTile->getNumberOfBands()];
         
         for(rspf_uint32 band = 0;
             band < inputTile->getNumberOfBands();
             ++band)
         {
            bands[band] = static_cast<double*>(inputTile->getBuf(band));
         }
         castInputToOutput(bands,
                           inputTile->getNullPix(),
                           inputTile->getDataObjectStatus()==RSPF_PARTIAL);
         delete [] bands;
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         break;
      }
   }

//   theTile->setDataObjectStatus(inputTile->getDataObjectStatus());
   theTile->validate();
   return theTile;
}

void rspfCastTileSourceFilter::setOutputScalarType(rspfScalarType scalarType)
{
   rspfScalarType old_type = theOutputScalarType;
   theOutputScalarType = scalarType;

   if ( (theOutputScalarType != old_type))
   {
      // must reinitialize...
      theTile = NULL;
      initialize();
   }
}

void rspfCastTileSourceFilter::setOutputScalarType(rspfString scalarType)
{
   int scalar =
      rspfScalarTypeLut::instance()->getEntryNumber(scalarType.c_str());
   
   if (scalar != rspfLookUpTable::NOT_FOUND)
   {
      setOutputScalarType(static_cast<rspfScalarType>(scalar));
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfCastTileSourceFilter::setOutputScalarType WARN:"
         << "\nUnknown scalar type:  " << scalarType.c_str() << endl;
   }
}

rspfString rspfCastTileSourceFilter::getOutputScalarTypeString()const
{
   return rspfScalarTypeLut::instance()->getEntryString(theOutputScalarType);
}

void rspfCastTileSourceFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   if (theOutputScalarType == RSPF_SCALAR_UNKNOWN)
   {
      // default to RSPF_UCHAR
      theOutputScalarType = RSPF_UCHAR;
   }
}

bool rspfCastTileSourceFilter::saveState(rspfKeywordlist& kwl,
                                          const char* prefix) const
{
   rspfImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,
           rspfKeywordNames::SCALAR_TYPE_KW,
           rspfScalarTypeLut::instance()->getEntryString(theOutputScalarType),
           true);

   return true;
}

bool rspfCastTileSourceFilter::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   rspfImageSourceFilter::loadState(kwl, prefix);

   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfScalarRemapper::loadState WARN:\n"
         << " ERROR detected in keyword list!  State not loaded."
         << endl;
      return false;
   }

   int scalar = rspfScalarTypeLut::instance()->getEntryNumber(kwl, prefix);

   if (scalar != rspfLookUpTable::NOT_FOUND)
   {
      theOutputScalarType = static_cast<rspfScalarType>(scalar);
   }

   return true;
}

template<class inType>
void rspfCastTileSourceFilter::castInputToOutput(inType **inBuffer,
                                                  const double  *nullPix,
                                                  bool   inPartialFlag)
{
   switch(theTile->getScalarType())
   {
      case RSPF_UINT8:
      {
         rspf_uint8** bands = new rspf_uint8*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<rspf_uint8*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_SINT8:
      {
         rspf_sint8** bands = new rspf_sint8*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<rspf_sint8*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_SINT16:
      {
         rspf_sint16** bands = new rspf_sint16*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<rspf_sint16*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         rspf_uint16** bands = new rspf_uint16*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<rspf_uint16*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_UINT32:
      {
         rspf_uint32** bands = new rspf_uint32*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<rspf_uint32*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_SINT32:
      {
         rspf_sint32** bands = new rspf_sint32*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<rspf_sint32*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      
      case RSPF_FLOAT32: 
      case RSPF_NORMALIZED_FLOAT:
      {
         float** bands = new float*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<float*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         double** bands = new double*[theTile->getNumberOfBands()];

         for(rspf_uint32 band = 0; band < theTile->getNumberOfBands(); ++band)
         {
            bands[band] = static_cast<double*>(theTile->getBuf(band));
         }
      
         castInputToOutput(inBuffer,
                           nullPix,
                           inPartialFlag,
                           bands,
                           theTile->getNullPix(),
                           theTile->getNumberOfBands());
         delete [] bands;
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfCastTileSourceFilter::castInputToOutput WARN:\n"
            << "Scalar not set in rspfCastTileSourceFilter" << endl;
         break;
      }
   }
}

template<class inType, class outType>
void rspfCastTileSourceFilter::castInputToOutput(inType *inBuffer[],
                                                  const double  *nullInPix,
                                                  bool   inPartialFlag,
                                                  outType *outBuffer[],
                                                  const double  *nullOutPix,
                                                  rspf_uint32   numberOfBands)
{
   rspf_uint32 size = theTile->getWidth()*theTile->getHeight();
   if(!inPartialFlag)
   {
      for(rspf_uint32 band = 0; band < numberOfBands; ++band)
      {
         outType outMin = static_cast<outType>(theTile->getMinPix(band));
         outType outMax = static_cast<outType>(theTile->getMaxPix(band));
         
         for(rspf_uint32 offset = 0; offset < size; ++offset)
         {
            double temp = inBuffer[band][offset];
            if(temp < outMin) temp = outMin;
            if(temp > outMax) temp = outMax;
            outBuffer[band][offset] = static_cast<outType>(temp);
         }
      }
   }
   else
   {
      for(rspf_uint32 band = 0; band < numberOfBands; ++band)
      {
         inType np      = static_cast<inType>(nullInPix[band]);
	     outType outNp  = static_cast<outType>(nullOutPix[band]);
         outType outMin = static_cast<outType>(theTile->getMinPix(band));
         outType outMax = static_cast<outType>(theTile->getMaxPix(band));
         
         for(rspf_uint32 offset = 0; offset < size; ++offset)
         {
            if(inBuffer[band][offset] != np)
            {
               double temp = inBuffer[band][offset];
               if(temp < outMin) temp = outMin;
               if(temp > outMax) temp = outMax;
               outBuffer[band][offset] = static_cast<outType>(temp);
            }
	    else
	    {
		outBuffer[band][offset] = static_cast<outType>(outNp);
		
	    }
         }
      }
   }
}

void rspfCastTileSourceFilter::allocate(const rspfRefPtr<rspfImageData> inputTile)
{
   theTile = rspfImageDataFactory::instance()->create(
      this,
      getOutputScalarType(),
      inputTile->getNumberOfBands(),
      inputTile->getWidth(),
      inputTile->getHeight());
   theTile->initialize();
   theTile->setOrigin(inputTile->getOrigin());
}

void rspfCastTileSourceFilter::propertyEvent(rspfPropertyEvent& /* event */)
{
}

void rspfCastTileSourceFilter::refreshEvent(rspfRefreshEvent& /* event */)
{
}

void rspfCastTileSourceFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(property->getName() == rspfKeywordNames::SCALAR_TYPE_KW)
   {
      int scalar = rspfScalarTypeLut::instance()->getEntryNumber(property->valueToString());
      
      if (scalar != rspfLookUpTable::NOT_FOUND)
      {
         setOutputScalarType(static_cast<rspfScalarType>(scalar));
      }
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfCastTileSourceFilter::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> resultProperty;
   if(name == rspfKeywordNames::SCALAR_TYPE_KW)
   {
      rspfStringProperty* stringProperty = new rspfStringProperty(name,
                                                                    rspfScalarTypeLut::instance()->getEntryString(theOutputScalarType),
                                                                    false);

      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_UINT8));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_SINT8));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_UINT16));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_SINT16));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_UINT32));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_SINT32));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_FLOAT32));
      stringProperty->addConstraint(rspfScalarTypeLut::instance()->getEntryString(RSPF_FLOAT64));

      stringProperty->setCacheRefreshBit();
      resultProperty = stringProperty;
   }
   else
   {
      resultProperty = rspfImageSourceFilter::getProperty(name);
   }
   return resultProperty;
}

void rspfCastTileSourceFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back(rspfKeywordNames::SCALAR_TYPE_KW);
}
   
