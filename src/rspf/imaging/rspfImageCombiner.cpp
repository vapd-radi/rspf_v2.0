//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageCombiner.cpp 20382 2011-12-15 14:24:57Z dburken $
#include <rspf/imaging/rspfImageCombiner.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfTrace.h>

using namespace std;

RTTI_DEF2(rspfImageCombiner, "rspfImageCombiner", rspfImageSource, rspfConnectableObjectListener)
static rspfTrace traceDebug ("rspfImageCombiner:debug");

rspfImageCombiner::rspfImageCombiner()
   :rspfImageSource(NULL,
                     0,
                     0,
                     false,
                     false),
    theLargestNumberOfInputBands(0),
    theInputToPassThrough(0),
    theHasDifferentInputs(false),
    theNormTile(NULL),
    theCurrentIndex(0)
{
	theComputeFullResBoundsFlag = true;
   // until something is set we will just set the blank tile
   // to a 1 band unsigned char type
   addListener((rspfConnectableObjectListener*)this);
}
rspfImageCombiner::rspfImageCombiner(rspfObject* owner,
                                       int numberOfInputs,
                                       int numberOfOutputs,
                                       bool inputListIsFixedFlag,
                                       bool outputListIsFixedFlag)
   :rspfImageSource(owner,
                     numberOfInputs,
                     numberOfOutputs,
                     inputListIsFixedFlag,
                     outputListIsFixedFlag),
    theLargestNumberOfInputBands(0),
    theInputToPassThrough(0),
    theHasDifferentInputs(false),
    theNormTile(NULL),
    theCurrentIndex(0)
{
   addListener((rspfConnectableObjectListener*)this);
   theComputeFullResBoundsFlag = true;
}

rspfImageCombiner::rspfImageCombiner(rspfConnectableObject::ConnectableObjectList& inputSources)
   :rspfImageSource(NULL,
                     (rspf_uint32)inputSources.size(),
                     0,
                     false,
                     false),
                     theLargestNumberOfInputBands(0),
                     theInputToPassThrough(0),
                     theHasDifferentInputs(false),
                     theNormTile(NULL),
                     theCurrentIndex(0)
{
	theComputeFullResBoundsFlag = true;
   for(rspf_uint32 index = 0; index < inputSources.size(); ++index)
   {
      connectMyInputTo(index, inputSources[index].get());
   }
   addListener((rspfConnectableObjectListener*)this);
   initialize();
}

rspfImageCombiner::~rspfImageCombiner()
{
   removeListener((rspfConnectableObjectListener*)this);
}

rspfIrect rspfImageCombiner::getBoundingRect(rspf_uint32 resLevel)const
{
   static const char* MODULE = "rspfImageCombiner::getBoundingRect";
   rspfIrect result;
   if(theComputeFullResBoundsFlag)
   {
      precomputeBounds();
   }
   
   double scale = 1.0/std::pow(2.0, (double)resLevel);
   rspfDpt scalar(scale, scale);
   result.makeNan();
   
   rspf_uint32 inputIndex = 0;
   rspfImageSource* interface;
   for(inputIndex = 0;inputIndex < getNumberOfInputs(); ++inputIndex)
   {
      interface = PTR_CAST(rspfImageSource, getInput(inputIndex));
      if(interface)
      {
         rspfIrect rect  = theFullResBounds[inputIndex];
         if(result.hasNans())
         {
            result = rect;
         }
         else if(!rect.hasNans())
         {
            if(traceDebug())
            {
               CLOG << "rect " << inputIndex << " = " << result << endl;
            }
            rect = rect*scale;
            result = result.combine(rect);
         }
      }
   }
   if(traceDebug())
   {
      CLOG << "resulting bounding rect =  " << result << endl;
   }
   
   return result;
}

rspf_uint32 rspfImageCombiner::getNumberOfInputBands() const
{
   return theLargestNumberOfInputBands;
}

rspfScalarType rspfImageCombiner::getOutputScalarType() const
{
   if(getInput(theInputToPassThrough))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(theInputToPassThrough));
      if(temp)
      {
         return temp->getOutputScalarType();
      }
   }

   return RSPF_SCALAR_UNKNOWN;
}

rspf_uint32 rspfImageCombiner::getTileWidth()const
{
   if(getInput(theInputToPassThrough))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(theInputToPassThrough));
      if(temp)
      {
         return temp->getTileWidth();
      }     
   }
   
   return 1;
}

rspf_uint32 rspfImageCombiner::getTileHeight()const
{
   if(getInput(theInputToPassThrough))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(theInputToPassThrough));
      if(temp)
      {
         return temp->getTileHeight();
      }     
   }
   
   return 1;
}

double rspfImageCombiner::getNullPixelValue(rspf_uint32 band)const
{
   if(getInput(theInputToPassThrough))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(theInputToPassThrough));
      if(temp)
      {
         rspf_uint32 bands = temp->getNumberOfOutputBands();
         if(band < bands)
         {
            return temp->getNullPixelValue(band);
         }
         else
         {
            return temp->getNullPixelValue(bands-1);
         }
      }     
   }
   
   return 0.0;
}

double rspfImageCombiner::getMinPixelValue(rspf_uint32 band)const
{
   const char* MODULE = "rspfImageCombiner::getMinPixelValue";

   if(!getNumberOfInputs())
   {
      return rspf::nan();
   }
   double result = 1.0/DBL_EPSILON;
   
   for(rspf_uint32 index = 0; index < getNumberOfInputs();++index)
   {
      rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(index));
      if(input)
      {
         rspf_uint32 bands = input->getNumberOfOutputBands();
         double temp = 0;
         if(band < bands)
         {
            temp = input->getMinPixelValue(band);
         }
         else
         {
            temp = input->getMinPixelValue(0);
         }
         
         if(temp < result)
         {
            result = temp;
         }
      }
   }
   if(traceDebug())
   {
      CLOG << "min pixel returned is = " << result;
   }

   return result;
}

double rspfImageCombiner::getMaxPixelValue(rspf_uint32 band)const
{
   const char* MODULE = "rspfImageCombiner::getMaxPixelValue";

   if(!getNumberOfInputs())
   {
      return rspf::nan();
   }
   double result = -1.0/DBL_EPSILON;
   
   for(rspf_uint32 idx = 0; idx < getNumberOfInputs();++idx)
   {
      rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(idx));
      if(input)
      {
         rspf_uint32 bands = input->getNumberOfOutputBands();
         double temp = 0;
         if(band < bands)
         {
            temp = input->getMaxPixelValue(band);
         }
         else
         {
            temp = input->getMaxPixelValue(0);
         }
         if(temp > result)
         {
            result = temp;
         }
      }
   }

   if(traceDebug())
   {
      CLOG << "max pixel returned is = " << result;
   }
   
   return result;
}

void rspfImageCombiner::initialize()
{
   
   theLargestNumberOfInputBands = 0;
   theComputeFullResBoundsFlag = true;

   // now find the largest number of bands
   //
   rspf_uint32 size = getNumberOfInputs();
   rspfScalarType scalarType = RSPF_SCALAR_UNKNOWN;
   rspf_uint32 scalarSizeInBytes = 0;
   theInputToPassThrough = 0;
   theHasDifferentInputs= false;
   if(size > 0)
   {
      for(rspf_uint32 idx = 0; idx < size; ++idx)
      {
         rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(idx));
         if(temp)
         {
            rspf_uint32 numberOfBands = temp->getNumberOfOutputBands();
            if(numberOfBands > theLargestNumberOfInputBands)
            {
               theLargestNumberOfInputBands = numberOfBands;
            }
            rspfScalarType current = temp->getOutputScalarType();
            if(current != scalarType)
            {
               if(scalarType == RSPF_SCALAR_UNKNOWN)
               {
                  scalarType = current;
                  scalarSizeInBytes = rspf::scalarSizeInBytes(scalarType);
                  theInputToPassThrough = idx;
               }
               else
               {
                  theHasDifferentInputs = true;
                  if(scalarSizeInBytes < rspf::scalarSizeInBytes(current))
                  {
                     scalarType        = current;
                     scalarSizeInBytes = rspf::scalarSizeInBytes(current);
                     theInputToPassThrough = idx;
                  }
               }
            }
         }
      }
   }
}

bool rspfImageCombiner::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool result = rspfImageSource::loadState(kwl, prefix);

   return result;
}

void rspfImageCombiner::updateRects()
{
	theComputeFullResBoundsFlag = true;
}

// rspfRefPtr<rspfImageData> rspfImageCombiner::getNextTile(rspf_uint32& returnedIdx,
//                                                             rspf_uint32 idx,
//                                                             const rspfIpt& tileRect,
//                                                             rspf_uint32 resLevel)
// {
//    theCurrentIndex  = idx;
//    return getNextTile(returnedIdx, origin, resLevel);
// }

// rspfRefPtr<rspfImageData> rspfImageCombiner::getNextTile(rspf_uint32& returnedIdx,
//                                                             rspf_uint32 idx,
//                                                             const rspfIpt& origin,
//                                                             rspf_uint32 resLevel)
// {
//    theCurrentIndex  = idx;
//    return getNextTile(returnedIdx, origin, resLevel);
// }

rspfRefPtr<rspfImageData> rspfImageCombiner::getNextTile(rspf_uint32& returnedIdx,
                                                            const rspf_uint32 startIdx,
                                                            const rspfIrect& tileRect,
                                                            rspf_uint32 resLevel)
{
   theCurrentIndex = startIdx;   
   return getNextTile(returnedIdx, tileRect, resLevel);
}

// rspfRefPtr<rspfImageData> rspfImageCombiner::getNextTile(rspf_uint32& returnedIdx,
//                                                             const rspfIpt& origin,
//                                                             rspf_uint32 resLevel)
// {
//    rspf_int32 w = getTileWidth();
//    rspf_int32 h = getTileHeight();
   
//    return getNextTile(returnedIndex,
//                       rspfIrect(origin.x,
//                                  origin.y,
//                                  origin.x + w-1,
//                                  origin.y + h-1),
//                       resLevel);
// }

rspfRefPtr<rspfImageData> rspfImageCombiner::getNextTile(rspf_uint32& returnedIdx,
                                                            const rspfIrect& tileRect,
                                                            rspf_uint32 resLevel)
{
   rspf_uint32 size = getNumberOfInputs();
   if ( theCurrentIndex >= size)
   {
      return 0;
   }
   
   if(theComputeFullResBoundsFlag)
   {
      precomputeBounds();
   }
   
   rspfImageSource* temp = 0;
   rspfRefPtr<rspfImageData> result = 0;
   rspfDataObjectStatus status = RSPF_NULL;

   double scale = 1.0/std::pow(2.0, (double)resLevel);
   rspfDpt scalar(scale, scale);

   while( (theCurrentIndex<size) && !result)
   {
      rspfIrect rect = theFullResBounds[theCurrentIndex];
      if(!rect.hasNans())
      {
         rect = rect * scalar;
         temp = PTR_CAST(rspfImageSource,
                         getInput(theCurrentIndex));
         
         if(rect.intersects(tileRect)&&temp)
         {
            result = temp->getTile(tileRect, resLevel);
            status = (result.valid() ?
                      result->getDataObjectStatus():RSPF_NULL);
            if((status == RSPF_NULL)||
               (status == RSPF_EMPTY))
            {
               result = 0;
            }
         }
         else
         {
            status = RSPF_NULL;
            result = 0;
         }
      }
      else
      {
         status = RSPF_NULL;
         result = NULL;
      }
      
      // Go to next source.
      ++theCurrentIndex;
   }
   returnedIdx = theCurrentIndex;
   if(result.valid())
   {
      --returnedIdx;
   }

   return result;
}


// rspfRefPtr<rspfImageData> rspfImageCombiner::getNextNormTile(rspf_uint32& returnedIdx,
//                                                                 rspf_uint32 index,
//                                                                 const rspfIpt& origin,
//                                                                 rspf_uint32 resLevel)
// {
//    theCurrentIndex = index;
//    return getNextNormTile(returnedIdx, origin, resLevel);
// }

rspfRefPtr<rspfImageData> rspfImageCombiner::getNextNormTile(rspf_uint32& returnedIdx,
                                                                const rspf_uint32 startIdx,
                                                                const rspfIrect& tileRect,
                                                                rspf_uint32 resLevel)
{
   theCurrentIndex = startIdx;
   return getNextNormTile(returnedIdx, tileRect, resLevel);
}

// rspfRefPtr<rspfImageData> rspfImageCombiner::getNextNormTile(rspf_uint32& returnedIdx,
//                                                                 const rspfIpt& origin,
//                                                                 rspf_uint32 resLevel)
// {
//    rspf_int32 w = getTileWidth();
//    rspf_int32 h = getTileHeight();
   
//    return getNextNormTile(returnedIdx,
//                           rspfIrect(origin.x,
//                                      origin.y,
//                                      origin.x + w-1,
//                                      origin.y + h-1),
//                           resLevel);
// }


rspfRefPtr<rspfImageData> rspfImageCombiner::getNextNormTile(rspf_uint32& returnedIdx,
                                                                const rspfIrect& tileRect,
                                                                rspf_uint32 resLevel)
{
   rspf_uint32 size = getNumberOfInputs();

   if(theCurrentIndex >= size)
   {
      return 0;
   }
   
   if(!theNormTile)
   {
      theNormTile = new rspfImageData(this,
                                       RSPF_NORMALIZED_FLOAT);
      theNormTile->initialize();
   }

   rspfRefPtr<rspfImageData> result = getNextTile(returnedIdx, tileRect, resLevel);

   if(result.valid())
   {
      theNormTile->setImageRectangleAndBands(result->getImageRectangle(),
                                             result->getNumberOfBands());
      result->copyTileToNormalizedBuffer((float*)theNormTile->getBuf());

      theNormTile->setDataObjectStatus(result->getDataObjectStatus());
      result = theNormTile;
   }
   
   return result;
}

rspf_uint32 rspfImageCombiner::getNumberOfOverlappingImages(const rspfIrect& rect,
                                                              rspf_uint32 resLevel)const
{
   if(theComputeFullResBoundsFlag)
   {
      precomputeBounds();
   }
   double scale = 1.0/std::pow(2.0, (double)resLevel);
   rspfDpt scalar(scale, scale);
   rspf_uint32 result = 0;
   rspf_uint32 maxIndex = getNumberOfInputs();
   for(rspf_uint32 inputIndex = 0; inputIndex < maxIndex; ++inputIndex)
   {
      if(!theFullResBounds[inputIndex].hasNans())
      {
         rspfIrect boundingRect = theFullResBounds[inputIndex] * scalar;
         if(rect.intersects(boundingRect))
         {
            ++result;
         }
      }
   }
   
   return result;
}

void rspfImageCombiner::getOverlappingImages(std::vector<rspf_uint32>& result,
					      const rspfIrect& rect,
                                              rspf_uint32 resLevel)const
{
   if(theComputeFullResBoundsFlag)
   {
      precomputeBounds();
   }
   double scale = 1.0/std::pow(2.0, (double)resLevel);
   rspfDpt scalar(scale, scale);
   
   rspf_uint32 inputIndex;
   rspfIrect boundingRect;
   for(inputIndex = 0; inputIndex < getNumberOfInputs(); ++inputIndex)
   {
      if(!theFullResBounds[inputIndex].hasNans())
      {
         boundingRect = theFullResBounds[inputIndex]*scalar;
         if(rect.intersects(boundingRect))
         {
            result.push_back(inputIndex);
         }
      }
   }
}

void rspfImageCombiner::connectInputEvent(rspfConnectionEvent& /* event */)
{
   initialize();
}

void rspfImageCombiner::disconnectInputEvent(rspfConnectionEvent& /* event */)
{ 
   initialize();
}

void rspfImageCombiner::propertyEvent(rspfPropertyEvent& /* event */)
{
   initialize();
}

void rspfImageCombiner::refreshEvent(rspfRefreshEvent& /* event */)
{
   initialize();
}

bool rspfImageCombiner::hasDifferentInputs()const
{
   return theHasDifferentInputs;
}

bool rspfImageCombiner::saveState(rspfKeywordlist& kwl,
                                   const char* prefix) const
{
   return rspfImageSource::saveState(kwl, prefix);
}

bool rspfImageCombiner::canConnectMyInputTo(rspf_int32 /* inputIndex */,
                                             const rspfConnectableObject* object)const
{
   return (object&& PTR_CAST(rspfImageSource, object));
}

void rspfImageCombiner::precomputeBounds()const
{
   
   rspf_uint32 inputSize = getNumberOfInputs();

   if(inputSize)
   {
      rspfImageSource* tempInterface=0;
      if(theFullResBounds.size() != inputSize)
      {
         theFullResBounds.resize(inputSize);
      }
      for(rspf_uint32 inputIndex = 0; inputIndex < inputSize; ++inputIndex)
      {
         tempInterface = PTR_CAST(rspfImageSource, getInput(inputIndex));
         if(tempInterface)
         {
            theFullResBounds[inputIndex] = tempInterface->getBoundingRect();
         }
         else
         {
            theFullResBounds[inputIndex].makeNan();
         }
      }
      theComputeFullResBoundsFlag = false;
   }
   else
   {
      theFullResBounds.clear();
   }
}
