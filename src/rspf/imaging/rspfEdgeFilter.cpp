//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfEdgeFilter.cpp 19956 2011-08-16 00:36:25Z gpotts $
#include <rspf/imaging/rspfEdgeFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfStringProperty.h>

RTTI_DEF1(rspfEdgeFilter, "rspfEdgeFilter", rspfImageSourceFilter);

#define PROP_EDGE_FILTER "Edge type"

rspfEdgeFilter::rspfEdgeFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theTile(NULL),
    theFilterType("Sobel")
{
}

rspfEdgeFilter::rspfEdgeFilter(rspfImageSource* inputSource)
   :rspfImageSourceFilter(inputSource),
    theTile(NULL),
    theFilterType("Sobel")
{
}

rspfEdgeFilter::rspfEdgeFilter(rspfObject* owner,
                                   rspfImageSource* inputSource)
   :rspfImageSourceFilter(owner, inputSource),
    theTile(NULL),
    theFilterType("Sobel")
{
}

rspfRefPtr<rspfImageData> rspfEdgeFilter::getTile(const rspfIrect& rect,
                                                     rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(rect, resLevel);
   }

   // expand the rect out to cver the 3x3 horizontal and vertical
   // kernel.
   //
   rspfIrect requestRect = rect;

   adjustRequestRect(requestRect);
   
   rspfRefPtr<rspfImageData> inputData =
      rspfImageSourceFilter::getTile(requestRect, resLevel);

   if(!inputData.valid() || (!inputData->getBuf()))
   {
      return inputData;
   }

   if(!theTile.valid()) initialize();
   if(!theTile.valid()) return theTile;
      
   theTile->setImageRectangleAndBands(rect, inputData->getNumberOfBands());
   
   switch(theTile->getScalarType())
   {
   case RSPF_UCHAR:
   {
      runFilter((rspf_uint8)0,
                inputData);
      break;
   }
   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
   {
      runFilter((rspf_float32)0,
                inputData);
      break;
   }
   case RSPF_USHORT16:
   case RSPF_USHORT11:
   {
      runFilter((rspf_uint16)0,
                inputData);
      break;
   }
   case RSPF_SSHORT16:
   {
      runFilter((rspf_sint16)0,
                inputData);
      break;
   }
   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
   {
      runFilter((rspf_float64)0,
                inputData);
      break;
   }
   default:
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfEdgeFilter::getTile WARN: Scalar type = " << theTile->getScalarType()
                                         << " Not supported by rspfEdgeFilter" << std::endl;
      break;
   }
   }

   return theTile;
}

void rspfEdgeFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   theTile = NULL;

   if(!isSourceEnabled())
   {
      return;
   }
   
   theTile = rspfImageDataFactory::instance()->create(this, this);
   if(theTile.valid())
   {
      theTile->initialize();
   }

}


void rspfEdgeFilter::getFilterTypeNames(
   std::vector<rspfString>& filterNames)const
{
   filterNames.push_back("Laplacian");
   filterNames.push_back("Prewitt");
   filterNames.push_back("Roberts");
   filterNames.push_back("Sobel");
   filterNames.push_back("LocalMax8");
}

rspfString rspfEdgeFilter::getFilterType()const
{
   return theFilterType;
}

void rspfEdgeFilter::setFilterType(const rspfString& filterType)
{
   rspfString tempFilterType = filterType;
   tempFilterType = tempFilterType.downcase();
   
   if(tempFilterType.contains("sob"))
   {
      theFilterType = "Sobel";
   }
   else if(tempFilterType.contains("lap"))
   {
      theFilterType = "Laplacian";
   }
   else if(tempFilterType.contains("prew"))
   {
      theFilterType = "Prewitt";
   }
   else if(tempFilterType.contains("rob"))
   {
      theFilterType = "Roberts";
   }
   else if(tempFilterType.contains("localmax"))
   {
      theFilterType = "LocalMax8";
   }
   else
   {
      theFilterType = "Sobel";
   }
}

void rspfEdgeFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;
   
   if(property->getName() == PROP_EDGE_FILTER)
   {
      theFilterType = property->valueToString();
   }
   else 
   {
      rspfImageSourceFilter::setProperty(property.get());
   }

}

rspfRefPtr<rspfProperty> rspfEdgeFilter::getProperty(const rspfString& name)const
{
   if(name == PROP_EDGE_FILTER)
   {
      std::vector<rspfString> filterNames;
      
      getFilterTypeNames(filterNames);
      rspfStringProperty* stringProp = new rspfStringProperty(PROP_EDGE_FILTER,
								theFilterType,
								false,
								filterNames);
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();

      return stringProp;
   }

   return rspfImageSourceFilter::getProperty(name);
}

void rspfEdgeFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back(PROP_EDGE_FILTER);
}


void rspfEdgeFilter::adjustRequestRect(rspfIrect& requestRect)const
{
   rspfString filterType = theFilterType;
   filterType = filterType.downcase();
   rspfIrect rect = requestRect;
   if(filterType.contains("sob")||
      filterType.contains("lap")||
      filterType.contains("pre")||
      filterType.contains("localmax"))
   {
      requestRect = rspfIrect(rect.ul().x - 1,
                               rect.ul().y - 1,
                               rect.lr().x + 1,
                               rect.lr().y + 1);
   }
   else if(filterType.contains("rob"))
   {
      requestRect = rspfIrect(rect.ul().x,
                               rect.ul().y,
                               rect.lr().x + 1,
                               rect.lr().y + 1);
   }
   else
   {
      requestRect = rspfIrect(rect.ul().x - 1,
                               rect.ul().y - 1,
                               rect.lr().x + 1,
                               rect.lr().y + 1);
   }
}

template <class T>
void rspfEdgeFilter::runFilter(T dummyVariable,
                                rspfRefPtr<rspfImageData> inputData)
{
   rspfString filterType = theFilterType;
   filterType = filterType.downcase();
   
   if(filterType.contains("sobel"))
   {
      runSobelFilter(dummyVariable, inputData);
   }
   else if(filterType.contains("lap"))
   {
      runLaplacianFilter(dummyVariable, inputData);
   }
   else if(filterType.contains("pre"))
   {
      runPrewittFilter(dummyVariable, inputData);
   }
   else if(filterType.contains("rob"))
   {
      runRobertsFilter(dummyVariable, inputData);
   }
   else if(filterType.contains("localmax"))
   {
      runLocalMax8Filter(dummyVariable, inputData);
   }
   else 
   {
      theTile->makeBlank();
   }
}

template <class T>
void rspfEdgeFilter::runSobelFilter(T /* dummyVariable */,
                                     rspfRefPtr<rspfImageData> inputData)
{
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
   double horizontalValue = 0.0;
   double verticalValue = 0.0;
   double value = 0.0;
   // rspf_uint32 valueIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_int32 rowIncrement  = inputData->getWidth();
   rspf_int32 rowIncrement2 = 2*inputData->getWidth();
   
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(theTile->getNullPix(bandIdx));
      T minP       = static_cast<T>(theTile->getMinPix(bandIdx));
      T maxP       = static_cast<T>(theTile->getMaxPix(bandIdx));

      if(inputBuf&&outputBuf)
      {
         for(y = 0; y < height; ++y)
         {
            for(x = 0; x < width; ++x)
            {
               if( (*(inputBuf + rowIncrement + 1) != np))
               {
                  horizontalValue = ((double)inputBuf[0] - (double)inputBuf[rowIncrement2]) +
                                    ((double)(inputBuf[1]*2.0) - (double)( inputBuf[rowIncrement2+1]*2.0)) +
                                    ((double)(inputBuf[2]) - (double)(inputBuf[rowIncrement2+2]));

                   verticalValue   = ((double)(inputBuf[2]) + (double)inputBuf[rowIncrement+2]*2.0 + (double)inputBuf[rowIncrement2+2]) -
                                     (double)(inputBuf[0] + 2.0*(double)inputBuf[rowIncrement] + (double)inputBuf[rowIncrement2]);
                  
                  value = sqrt(horizontalValue*horizontalValue +  verticalValue*verticalValue);

                  if((value == np) ||
                     (value < minP))
                  {
                     *outputBuf = (static_cast<T>(minP));
                  }
                  else if(value > maxP)
                  {
                     *outputBuf = (static_cast<T>(maxP));
                  }
                  else
                  {
                     *outputBuf = (static_cast<T>(value));
                  }
                  
               }
               else
               {
                  *outputBuf = np;
               }
               ++outputBuf;
               ++inputBuf;
            }
            inputBuf+=2;
         }
      }
   }
   theTile->validate();
}

template <class T>
void rspfEdgeFilter::runPrewittFilter(T /* dummyVariable */,
                                       rspfRefPtr<rspfImageData> inputData)
{
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
   double horizontalValue = 0.0;
   double verticalValue = 0.0;
   double value = 0.0;
   // rspf_uint32 valueIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_int32 rowIncrement  = inputData->getWidth();
   rspf_int32 rowIncrement2 = 2*inputData->getWidth();
   
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(theTile->getNullPix(bandIdx));
      T minP       = static_cast<T>(theTile->getMinPix(bandIdx));
      T maxP       = static_cast<T>(theTile->getMaxPix(bandIdx));

      if(inputBuf&&outputBuf)
      {
         for(y = 0; y < height; ++y)
         {
            for(x = 0; x < width; ++x)
            {
               if( (*(inputBuf + rowIncrement + 1) != np))
               {
                  horizontalValue = ((double)inputBuf[0] - (double)inputBuf[rowIncrement2]) +
                                    ((double)(inputBuf[1]) - (double)( inputBuf[rowIncrement2+1])) +
                                    ((double)(inputBuf[2]) - (double)(inputBuf[rowIncrement2+2]));

                   verticalValue   = ((double)(inputBuf[2]) + (double)inputBuf[rowIncrement+2] + (double)inputBuf[rowIncrement2+2]) -
                                     ((double)inputBuf[0] + (double)inputBuf[rowIncrement] + (double)inputBuf[rowIncrement2]);
                  
                  value = sqrt(horizontalValue*horizontalValue +  verticalValue*verticalValue);

                  if((value == np) ||
                     (value < minP))
                  {
                     *outputBuf = (static_cast<T>(minP));
                  }
                  else if(value > maxP)
                  {
                     *outputBuf = (static_cast<T>(maxP));
                  }
                  else
                  {
                     *outputBuf = (static_cast<T>(value));
                  }
                  
               }
               else
               {
                  *outputBuf = np;
               }
               ++outputBuf;
               ++inputBuf;
            }
            inputBuf+=2;
         }
      }
   }
   theTile->validate();
}

template <class T>
void rspfEdgeFilter::runRobertsFilter(T /* dummyVariable */,
                                       rspfRefPtr<rspfImageData> inputData)
{
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
   double v1 = 0.0;
   double v2 = 0.0;
   double value = 0.0;
   // rspf_uint32 valueIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_int32 rowIncrement  = inputData->getWidth();
   
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(theTile->getNullPix(bandIdx));
      T minP       = static_cast<T>(theTile->getMinPix(bandIdx));
      T maxP       = static_cast<T>(theTile->getMaxPix(bandIdx));

      if(inputBuf&&outputBuf)
      {
         for(y = 0; y < height; ++y)
         {
            for(x = 0; x < width; ++x)
            {
               if( (*inputBuf) != np)
               {
                  v1 = (double)inputBuf[0] - (double)(inputBuf[rowIncrement+1]);

                  v2   = (double)inputBuf[1] - (double)inputBuf[rowIncrement];
                  
                  value = sqrt(v1*v1 +  v2*v2);

                  if((value == np) ||
                     (value < minP))
                  {
                     *outputBuf = (static_cast<T>(minP));
                  }
                  else if(value > maxP)
                  {
                     *outputBuf = (static_cast<T>(maxP));
                  }
                  else
                  {
                     *outputBuf = (static_cast<T>(value));
                  }
                  
               }
               else
               {
                  *outputBuf = np;
               }
               ++outputBuf;
               ++inputBuf;
            }
            ++inputBuf;
         }
      }
   }
   theTile->validate();
}

template <class T>
void rspfEdgeFilter::runLaplacianFilter(T /* dummyVariable */,
                                         rspfRefPtr<rspfImageData> inputData)
{
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
   // double horizontalValue = 0.0;
   // double verticalValue = 0.0;
   double value = 0.0;
   // rspf_uint32 valueIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_int32 rowIncrement  = inputData->getWidth();
   rspf_int32 rowIncrement2 = 2*inputData->getWidth();
   
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(theTile->getNullPix(bandIdx));
      T minP       = static_cast<T>(theTile->getMinPix(bandIdx));
      T maxP       = static_cast<T>(theTile->getMaxPix(bandIdx));

      if(inputBuf&&outputBuf)
      {
         for(y = 0; y < height; ++y)
         {
            for(x = 0; x < width; ++x)
            {
               if( (*(inputBuf + rowIncrement + 1) != np))
               {
                  
                  value = fabs(((double)inputBuf[rowIncrement + 1]*4.0) -
                               ((double)inputBuf[1] + (double)inputBuf[rowIncrement] + (double)inputBuf[rowIncrement + 2] + (double)inputBuf[rowIncrement2+1]));

                  if((value == np) ||
                     (value < minP))
                  {
                     *outputBuf = (static_cast<T>(minP));
                  }
                  else if(value > maxP)
                  {
                     *outputBuf = (static_cast<T>(maxP));
                  }
                  else
                  {
                     *outputBuf = (static_cast<T>(value));
                  }
                  
               }
               else
               {
                  *outputBuf = np;
               }
               ++outputBuf;
               ++inputBuf;
            }
            inputBuf+=2;
         }
      }
   }
   theTile->validate();
}

template <class T>
void rspfEdgeFilter::runLocalMax8Filter(T /* dummyVariable */,
                                         rspfRefPtr<rspfImageData> inputData)
{
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
  
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_int32 rowIncrement  = inputData->getWidth();
   rspf_int32 rowIncrement2 = 2*inputData->getWidth(); 
         
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      //inputBuf has a 1 pixel edge compared to outputBuf
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(inputData->getNullPix(bandIdx)); //changed to input Null            

      if(inputBuf&&outputBuf)
      {
         //one pass: maybe faster if changed to two passes
         T* outB;
         T* inB;
         
      	outB = outputBuf;         
         inB  = inputBuf;         
         for(y = 0; y < height; ++y)
         {
            for(x = 0; x < width; ++x)
            {
               if (inB[1+rowIncrement] != np)
               {
                  *outB = max<T>(
                           max<T>(
                            max<T>(inB[0],inB[1]),
                            max<T>(inB[2],inB[rowIncrement])),
                           max<T>(
                            max<T>(inB[rowIncrement+2],inB[rowIncrement2]),
                            max<T>(inB[rowIncrement2+1],inB[rowIncrement2+2])
                            ));
               }
               else
               {
                  *outB = np;
               }
               ++outB;
               ++inB;
            }
            inB+=2; //go to next line, jump due to edge
         }       
      }      
   }
   theTile->validate();
}

bool rspfEdgeFilter::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{   
   kwl.add(prefix,
           PROP_EDGE_FILTER,
           getFilterType(),
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfEdgeFilter::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* value = kwl.find(prefix,
                                PROP_EDGE_FILTER );
   if(value)
   {
      setFilterType(rspfString(value));
   } else {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfEdgeFilter::loadState WARN: no filter type found" << std::endl;
   }
         
   return rspfImageSourceFilter::loadState(kwl, prefix);
}
