//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageHistogramSource.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSourceSequencer.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfImageHistogramSource:debug");

  RTTI_DEF3(rspfImageHistogramSource, "rspfImageHistogramSource", rspfHistogramSource, rspfConnectableObjectListener, rspfProcessInterface);

rspfImageHistogramSource::rspfImageHistogramSource(rspfObject* owner)
   :rspfHistogramSource(owner,
                         1,     // one input
                         0,     // no outputs
                         true,  // input list is fixed
                         false),// output can still grow though
    theHistogramRecomputeFlag(true),
    theMaxNumberOfResLevels(1),
    theComputationMode(RSPF_HISTO_MODE_NORMAL),
    theNumberOfTilesToUseInFastMode(100)
{
   theAreaOfInterest.makeNan();
   addListener((rspfConnectableObjectListener*)this);
	
   theMinValueOverride     = rspf::nan();
   theMaxValueOverride     = rspf::nan();
   theNumberOfBinsOverride = -1;
}

rspfImageHistogramSource::~rspfImageHistogramSource()
{
   removeListener((rspfConnectableObjectListener*)this);
}

rspfObject* rspfImageHistogramSource::getObject()
{
   return this;
}

const rspfObject* rspfImageHistogramSource::getObject()const
{
   return this;
}

void rspfImageHistogramSource::setAreaOfInterest(const rspfIrect& rect)
{
   if(rect != theAreaOfInterest)
   {
      theHistogramRecomputeFlag = true;
   }
   theAreaOfInterest = rect;
}

rspfIrect rspfImageHistogramSource::getAreaOfInterest()const
{
   return theAreaOfInterest;
}

void rspfImageHistogramSource::getAreaOfInterest(rspfIrect& rect)const
{
   rect = theAreaOfInterest;
}

rspf_uint32 rspfImageHistogramSource::getMaxNumberOfRLevels()const
{
   return theMaxNumberOfResLevels;
}

void rspfImageHistogramSource::setMaxNumberOfRLevels(rspf_uint32 number)
{
   if(number != theMaxNumberOfResLevels)
   {
      theHistogramRecomputeFlag = true;
   }
   theMaxNumberOfResLevels = number;
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfImageHistogramSource::getHistogram(const rspfIrect& rect)
{
   if((theAreaOfInterest != rect)||
      (theAreaOfInterest.hasNans()))
   {
      theAreaOfInterest = rect;
      theHistogramRecomputeFlag = true;
   }
	
   return getHistogram();
}

bool rspfImageHistogramSource::execute()
{
   if(!isSourceEnabled())
   {
      return theHistogram.valid();
   }
   
   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_EXECUTING);
   if(theHistogramRecomputeFlag)
   {
      if(theAreaOfInterest.hasNans())
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource, getInput(0));
         if(interface)
         {
            theAreaOfInterest = interface->getBoundingRect();
         }
      }
      switch(theComputationMode)
      {
         case RSPF_HISTO_MODE_FAST:
         {
            computeFastModeHistogram();
            break;
         }
         case RSPF_HISTO_MODE_NORMAL:
         default:
         {
            computeNormalModeHistogram();
            break;
         }
      }
   }
   
   if (needsAborting())
   {
      setProcessStatus(rspfProcessInterface::PROCESS_STATUS_ABORTED);
      theHistogramRecomputeFlag = false;
   }
   else
   {
      setProcessStatus(rspfProcessInterface::PROCESS_STATUS_NOT_EXECUTING);
   }
   
   return true;
}

bool rspfImageHistogramSource::canConnectMyInputTo(rspf_int32 myInputIndex,
                                                    const rspfConnectableObject* object)const
{
   return ((myInputIndex==0)&&PTR_CAST(rspfImageSource,
                                       object));
}

void rspfImageHistogramSource::setNumberOfBinsOverride(rspf_int32 numberOfBinsOverride)
{
   theNumberOfBinsOverride = numberOfBinsOverride;
}

void rspfImageHistogramSource::setMinValueOverride(rspf_float32 minValueOverride)
{
   theMinValueOverride = minValueOverride;
}
void rspfImageHistogramSource::setMaxValueOverride(rspf_float32 maxValueOverride)
{
   theMaxValueOverride = maxValueOverride;
}

rspfHistogramMode rspfImageHistogramSource::getComputationMode()const
{
   return theComputationMode;
}

void rspfImageHistogramSource::setComputationMode(rspfHistogramMode mode)
{
   theComputationMode = mode;
}

void rspfImageHistogramSource::propertyEvent(rspfPropertyEvent& /* event */)
{
   theHistogramRecomputeFlag = true;
}

void rspfImageHistogramSource::connectInputEvent(rspfConnectionEvent& /* event */)
{
   theHistogramRecomputeFlag = true;
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfImageHistogramSource::getHistogram()
{
   execute();
   return theHistogram;
}

void rspfImageHistogramSource::getBinInformation(rspf_uint32& numberOfBins,
                                                  rspf_float64& minValue,
                                                  rspf_float64& maxValue)const
{
   numberOfBins = 0;
   minValue     = 0;
   maxValue     = 0;
	
   rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(0));
   if(input)
   {
      switch(input->getOutputScalarType())
      {
         case RSPF_UINT8:
         {
            minValue     = 0;
            maxValue     = RSPF_DEFAULT_MAX_PIX_UCHAR;
            numberOfBins = 256;
				
            break;
         }
         case RSPF_USHORT11:
         {
            minValue     = 0;
            maxValue     = RSPF_DEFAULT_MAX_PIX_UINT11;
            numberOfBins = RSPF_DEFAULT_MAX_PIX_UINT11 + 1;
				
            break;
         }
         case RSPF_UINT16:
         case RSPF_UINT32:
         {
            minValue     = 0;
            maxValue     = RSPF_DEFAULT_MAX_PIX_UINT16;
            numberOfBins = RSPF_DEFAULT_MAX_PIX_UINT16 + 1;
				
            break;
         }
         case RSPF_SINT16:
         case RSPF_SINT32:
         case RSPF_FLOAT32:
         case RSPF_FLOAT64:
         {
            minValue     = RSPF_DEFAULT_MIN_PIX_SINT16;
            maxValue     = RSPF_DEFAULT_MAX_PIX_SINT16;
            numberOfBins = (RSPF_DEFAULT_MAX_PIX_SINT16-RSPF_DEFAULT_MIN_PIX_SINT16) + 1;
				
            break;
         }
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_NORMALIZED_DOUBLE:
         {
            minValue     = 0;
            maxValue     = 1.0;
            numberOfBins = RSPF_DEFAULT_MAX_PIX_UINT16+1;
            break;
         }
         default:
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "Unsupported scalar type in rspfImageHistogramSource::computeHistogram()" << endl;
            }
            return;
         }
      }
   }
	
   if(rspf::isnan(theMinValueOverride) == false)
   {
      minValue = (float)theMinValueOverride;
   }
   if(rspf::isnan(theMaxValueOverride) == false)
   {
      maxValue = (float)theMaxValueOverride;
   }
   if(theNumberOfBinsOverride > 0)
   {
      numberOfBins = theNumberOfBinsOverride;
   }
}

void rspfImageHistogramSource::computeNormalModeHistogram()
{
   // ref ptr, not a leak.
   theHistogram = new rspfMultiResLevelHistogram;
	
   rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(0));
   if(!input)
   {
      setPercentComplete(100.0);
      return;
   }
   
   if(getInput(0))
   {
      // sum up all tiles needing processing.  We will use the sequncer.
      //      rspf_uint32 numberOfResLevels = input->getNumberOfDecimationLevels();
      rspf_uint32 index = 0;
      double tileCount   = 0.0;
      double totalTiles  = 0.0;
      rspf_uint32 numberOfBands = input->getNumberOfOutputBands();
      rspf_uint32 numberOfBins  = 0;
      rspf_float64 minValue     = 0;
      rspf_float64 maxValue     = 0;
      getBinInformation(numberOfBins, minValue, maxValue);
		
      rspfRefPtr<rspfImageSourceSequencer> sequencer = new rspfImageSourceSequencer;
      sequencer->connectMyInputTo(0, getInput(0));
      sequencer->initialize();
      
      vector<rspfDpt> decimationFactors;
      input->getDecimationFactors(decimationFactors);
      if ( !decimationFactors.size() )
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageHistogramSource::computeNormalModeHistogram WARNING:"
            << "\nNo decimation factors from input.  returning..." << std::endl;
         return;
      }
      
      rspf_uint32 resLevelsToCompute = rspf::min((rspf_uint32)theMaxNumberOfResLevels,
                                                   (rspf_uint32)decimationFactors.size());
         
      if( decimationFactors.size() < theMaxNumberOfResLevels)
      {
         rspfNotify(rspfNotifyLevel_WARN) << "Number Decimations is smaller than the request number of r-levels defaulting to the smallest of the 2 numbers" << endl;
      }

      theHistogram->create(resLevelsToCompute);
      for(index = 0; index < resLevelsToCompute; ++index)
      {
         sequencer->setAreaOfInterest(theAreaOfInterest*decimationFactors[index]);
         
         totalTiles += sequencer->getNumberOfTiles();
      }
      
      
      if(numberOfBins > 0)
      {
         setPercentComplete(0.0);
         for(index = 0;
             (index < resLevelsToCompute);
             ++index)
         {
            // Check for abort request.
            if (needsAborting())
            {
               setPercentComplete(100);
               break;
            }
            
            //sequencer->setAreaOfInterest(input->getBoundingRect(index));
            sequencer->setAreaOfInterest(theAreaOfInterest*decimationFactors[index]);
            
            sequencer->setToStartOfSequence();
            theHistogram->getMultiBandHistogram(index)->create(numberOfBands,
                                                               numberOfBins,
                                                               minValue,
                                                               maxValue);
            
            rspfRefPtr<rspfImageData> data = sequencer->getNextTile(index);
            ++tileCount;
            setPercentComplete((100.0*(tileCount/totalTiles)));
            
            rspf_uint32 resLevelTotalTiles = sequencer->getNumberOfTiles();
            for (rspf_uint32 resLevelTileCount = 0;
                 resLevelTileCount < resLevelTotalTiles;
                 ++resLevelTileCount)
            {
               if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus() != RSPF_EMPTY))
               {
                  data->populateHistogram(theHistogram->getMultiBandHistogram(index));
               }
               
               // Check for abort request.
               if (needsAborting())
               {
                  setPercentComplete(100);
                  break;
               }
               
               
               data = sequencer->getNextTile(index);
               ++tileCount;
               setPercentComplete((100.0*(tileCount/totalTiles)));
            }
         }
      }
      sequencer->disconnect();
      sequencer = 0;
   }
}

void rspfImageHistogramSource::computeFastModeHistogram()
{
   // We will only compute a full res histogram in fast mode.  and will only do a MAX of 100 tiles.
   //
   rspf_uint32 resLevelsToCompute = 1;
	
   // ref ptr, not a leak.
   theHistogram = new rspfMultiResLevelHistogram;
   theHistogram->create(resLevelsToCompute);
   rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(0));
   if(!input)
   {
      setPercentComplete(100.0);
      return;
   }
   // sum up all tiles needing processing.  We will use the sequencer.
   //      rspf_uint32 numberOfResLevels = input->getNumberOfDecimationLevels();
   double tileCount   = 0.0;
   double totalTiles  = 0.0;
   rspf_uint32 numberOfBands = input->getNumberOfOutputBands();
   rspf_uint32 numberOfBins  = 0;
   rspf_float64 minValue     = 0;
   rspf_float64 maxValue     = 0;
   getBinInformation(numberOfBins, minValue, maxValue);
	
   rspfIrect tileBoundary = theAreaOfInterest;
   rspfIpt tileSize(rspf::max((rspf_uint32)input->getTileWidth(), (rspf_uint32)64),
                     rspf::max((rspf_uint32)input->getTileHeight(), (rspf_uint32)64));
	
   tileBoundary.stretchToTileBoundary(tileSize);
   rspf_uint32 tilesWide = (tileBoundary.width()/tileSize.x);
   rspf_uint32 tilesHigh = (tileBoundary.height()/tileSize.y);
   totalTiles = tilesWide*tilesHigh;
   
   if(totalTiles > theNumberOfTilesToUseInFastMode)
   {
      rspf_uint32 testTiles = (rspf_uint32) (std::sqrt((double)theNumberOfTilesToUseInFastMode)+.5);
      tilesWide = testTiles>tilesWide?tilesWide:testTiles;
      tilesHigh = testTiles>tilesHigh?tilesHigh:testTiles;
   }
   if(numberOfBins > 0)
   {
      rspfIpt origin = theAreaOfInterest.ul();
		
      rspf_uint32 widthWithExcess  = (rspf_uint32)(((rspf_float64)tileBoundary.width()/(tilesWide*tileSize.x))*tileSize.x);
      rspf_uint32 heightWithExcess = ((rspf_uint32)((rspf_float64)tileBoundary.height()/(tilesHigh*tileSize.y))*tileSize.y);
      theHistogram->getMultiBandHistogram(0)->create(numberOfBands,
                                                     numberOfBins,
                                                     minValue,
                                                     maxValue);
		
      rspf_uint32 x = 0;
      rspf_uint32 y = 0;
      tileCount = 0;
      totalTiles = tilesWide*tilesHigh;
      for(y = 0; y < tilesHigh; ++y)
      {
         for(x = 0; x < tilesWide; ++x)
         {
            rspfIpt ul(origin.x + (x*widthWithExcess),
                        origin.y + (y*heightWithExcess));
            rspfIrect tileRect(ul.x, ul.y, ul.x + tileSize.x-1, ul.y + tileSize.y-1);
            rspfRefPtr<rspfImageData> data = input->getTile(tileRect);
				
            if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus() != RSPF_EMPTY))
            {
               data->populateHistogram(theHistogram->getMultiBandHistogram(0));
            }
            ++tileCount;
            setPercentComplete((100.0*(tileCount/totalTiles)));
         }
      }
   }
}

bool rspfImageHistogramSource::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   rspfHistogramSource::loadState(kwl, prefix);  
   setNumberOfInputs(2);
   rspfString rect = kwl.find(prefix, "rect");

   if(!rect.empty())
   {
      loadState(kwl, prefix);
   }
   else 
   {
      rspfString newPrefix = rspfString(prefix) + "area_of_interest.";
      theAreaOfInterest.loadState(kwl, newPrefix);
   }
   
   rspfString mode = kwl.find(prefix, "mode");
   mode = mode.downcase();
   if(mode == "normal")
   {
      theComputationMode = RSPF_HISTO_MODE_NORMAL;
   }
   else if(mode == "fast")
   {
      theComputationMode = RSPF_HISTO_MODE_FAST;
   }
   if(getNumberOfInputs()!=1)
   {
      setNumberOfInputs(1);
   }
   
   rspfString numberOfTiles = kwl.find(prefix, "number_of_tiles");
   if(!numberOfTiles.empty())
   {
      theNumberOfTilesToUseInFastMode = numberOfTiles.toUInt32();
   }
   theInputListIsFixedFlag = true;
   theOutputListIsFixedFlag = false;
	
   return true;
}

bool rspfImageHistogramSource::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   bool result = rspfHistogramSource::saveState(kwl, prefix);
   if(result)
   {
      rspfString newPrefix = rspfString(prefix) + "area_of_interest.";
      theAreaOfInterest.saveState(kwl, newPrefix);
   }
   return result;
}
