//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
#include <rspf/imaging/rspfImageStatisticsSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageSourceSequencer.h>

rspfImageStatisticsSource::rspfImageStatisticsSource()
      :rspfSource(0,
                   1,
                   0,
                   true,
                   false)
{
}

rspfImageStatisticsSource::~rspfImageStatisticsSource()
{
   
}

void rspfImageStatisticsSource::computeStatistics()
{
   rspfImageSource* anInterface = PTR_CAST(rspfImageSource,
                                                     getInput());

   if(anInterface&&isSourceEnabled())
   {
      clearStatistics();
      rspfScalarType outputScalarType = anInterface->getOutputScalarType();

      switch(outputScalarType)
      {
         case RSPF_UCHAR:
         {
            computeStatisticsTemplate((rspf_uint8)0);
            break;
         }
         case RSPF_USHORT16:
         case RSPF_USHORT11:
         {
            computeStatisticsTemplate((rspf_uint16)0);
            break;
         }
         case RSPF_SSHORT16:
         {
            computeStatisticsTemplate((rspf_sint16)0);
            break;
         }
         case RSPF_DOUBLE:
         case RSPF_NORMALIZED_DOUBLE:
         {
            computeStatisticsTemplate((rspf_float64)0);
            break;
         }
         case RSPF_FLOAT:
         case RSPF_NORMALIZED_FLOAT:
         {
            computeStatisticsTemplate((rspf_float32)0);
            break;
         }
		 default :
		 {
			 break;
		 }
      }
   }
}

bool rspfImageStatisticsSource::canConnectMyInputTo(rspf_int32 inputIndex,
                                                     const rspfConnectableObject* object)const
{
   return (PTR_CAST(rspfImageSource, object)&&(inputIndex < 1));
}

template <class T>
void rspfImageStatisticsSource::computeStatisticsTemplate(T /* dummyVariable */)
{
   rspfRefPtr<rspfImageSourceSequencer> sequencer = new rspfImageSourceSequencer;

   sequencer->connectMyInputTo(getInput());
   sequencer->setToStartOfSequence();
   rspf_uint32 bands = sequencer->getNumberOfOutputBands();

   if(bands)
   {
      setStatsSize(bands);
      rspfRefPtr<rspfImageData> dataObject;
      
      while( (dataObject=sequencer->getNextTile()).valid() )
      {
         rspf_uint32 bandIdx = 0;
         bands = dataObject->getNumberOfBands();
         rspfDataObjectStatus status = dataObject->getDataObjectStatus();
         if((status != RSPF_EMPTY)&&
            (dataObject->getBuf()))
         {
            rspf_uint32 offsetMax = dataObject->getWidth()*dataObject->getHeight();
            for(bandIdx = 0; bandIdx < bands; ++bandIdx)
            {
               rspf_float64 pixelCount = 0.0;
               rspf_uint32 offset = 0;
               T* dataPtr   = static_cast<T*>(dataObject->getBuf(bandIdx));
               T nullPixel = static_cast<T>(dataObject->getNullPix(bandIdx)); 
               for(offset = 0; offset < offsetMax; ++offset)
               {
                  if((*dataPtr) != nullPixel)
                  {
                     theMean[bandIdx] += *dataPtr;
                     if((*dataPtr) < theMin[bandIdx])
                     {
                        theMin[bandIdx] = (*dataPtr);
                     }
                     if((*dataPtr) > theMax[bandIdx])
                     {
                        theMax[bandIdx] = (*dataPtr);
                     }
                     ++pixelCount;
                  }
                  ++dataPtr;
               }
               if(pixelCount > 0)
               {
                  theMean[bandIdx] /= pixelCount;
               }
            }
         }
      }
   }
   
   sequencer->disconnect();
   sequencer = 0;
}

const std::vector<rspf_float64>& rspfImageStatisticsSource::getMean()const
{
   return theMean;
}

const std::vector<rspf_float64>& rspfImageStatisticsSource::getMin()const
{
   return theMin;
}

const std::vector<rspf_float64>& rspfImageStatisticsSource::getMax()const
{
   return theMax;
}

void rspfImageStatisticsSource::clearStatistics()
{
   theMean.clear();
   theMin.clear();
   theMax.clear();
}

void rspfImageStatisticsSource::setStatsSize(rspf_uint32 size)
{
   theMean.resize(size);
   theMin.resize(size);
   theMax.resize(size);

   std::fill(theMean.begin(),
             theMean.end(),
             (rspf_float64)0.0);
   std::fill(theMin.begin(),
             theMin.end(),
             RSPF_DEFAULT_MAX_PIX_DOUBLE);
   std::fill(theMax.begin(),
             theMax.end(),
             (rspf_float64)RSPF_DEFAULT_MIN_PIX_DOUBLE);
}
