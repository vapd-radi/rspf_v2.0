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
// $Id: rspfBandAverageFilter.cpp 17206 2010-04-25 23:20:40Z dburken $
#include <rspf/imaging/rspfBandAverageFilter.h>

// used for constructing and an rspfImageData object
#include <rspf/imaging/rspfImageDataFactory.h>

// used for error reporting and for general info reporting such as warnings
#include <rspf/base/rspfErrorContext.h>

#include <sstream>
#include <iterator>

static const char* WEIGHTS_KW = "weights";

RTTI_DEF1(rspfBandAverageFilter,
          "rspfBandAverageFilter",
          rspfImageSourceFilter);

rspfBandAverageFilter::rspfBandAverageFilter()
   :rspfImageSourceFilter(),
    theTile(NULL)
{
}

rspfBandAverageFilter::rspfBandAverageFilter(rspfImageSource* input,
                                               const std::vector<double>& weights)
   :rspfImageSourceFilter(input),
    theTile(NULL),
    theWeights(weights)
{
}

rspfBandAverageFilter::~rspfBandAverageFilter()
{
}

rspf_uint32 rspfBandAverageFilter::getNumberOfOutputBands()const
{
   if(isSourceEnabled())
   {
      return 1;
   }
   
   return rspfImageSourceFilter::getNumberOfOutputBands();
}

rspfRefPtr<rspfImageData> rspfBandAverageFilter::getTile(const rspfIrect& rect, rspf_uint32 resLevel)
{
   // first we will check to see if we have an input connection
   //
   if(!theInputConnection)
   {
      // this will probably be null since we aren't initialized.
      // we really need to creae a default blank tile in RSPF that is returned
      return theTile;
   }

   rspfRefPtr<rspfImageData> inputData =
      theInputConnection->getTile(rect, resLevel);

   if(!inputData.valid())
   {
      return inputData;
   }
   
   // If we are disabled or if the data object is empty or NULL or
   // not enough bands we don't need to run our algorithm so return the input
   // tile.
   if(!isSourceEnabled() ||
      (inputData->getDataObjectStatus() == RSPF_NULL)||
      (inputData->getDataObjectStatus() == RSPF_EMPTY)||
      (inputData->getNumberOfBands() < 2))
   {
      return inputData;
   }

   // check to see if we are initialized
   if(!theTile.valid())
   {
      initialize();
   }
   
   theTile->setImageRectangle(rect);

   // now lets set up the template method to operate in native type and run
   // the algorithm
   //
   switch(inputData->getScalarType())
   {
   case RSPF_UCHAR:
   {
      averageInput((rspf_uint8)0, // setup template variable
                   inputData);
      break;
   }
   case RSPF_USHORT11:
   case RSPF_USHORT16:
   {
      averageInput((rspf_uint16)0, // setup template variable
                   inputData);
      break;
   }
   case RSPF_SSHORT16:
   {
      averageInput((rspf_sint16)0, // setup template variable
                   inputData);
      break;
   }
   case RSPF_NORMALIZED_FLOAT:
   case RSPF_FLOAT:
   {
      averageInput((float)0, // setup template variable
                   inputData);
      break;
   }
   case RSPF_NORMALIZED_DOUBLE:
   case RSPF_DOUBLE:
   {
      averageInput((double)0, // setup template variable
                   inputData);
      break;
   }
   default:
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "Unsupported scalar type in file %d at line %d",
                    __FILE__,
                    __LINE__);

      
      theTile->makeBlank();
      break;
   }
   }
   
   theTile->validate();

   
   return theTile;
}



void rspfBandAverageFilter::initialize()
{
   rspfImageSourceFilter::initialize();
   
   // theInputConnection is defined in rspfImageSourceFilter
   // It is automatically set when an input is connected to this
   // object
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);

      // now initialize the tile
      theTile->initialize();
      checkWeights();
   }
}

double rspfBandAverageFilter::getNullPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getNullPixelValue(band);
   }
   // lets use the first band's null value as our null
   return rspfImageSourceFilter::getMinPixelValue(0);
}

double rspfBandAverageFilter::getMinPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled()||!theInputConnection)
   {
      return rspfImageSourceFilter::getMinPixelValue(band);
   }

   // loop through each band and find the min pix value
   rspf_uint32 bandIndex = 0;
   rspf_uint32 inputBands = theInputConnection->getNumberOfOutputBands();
   double minValue = rspf::defaultMax(RSPF_DOUBLE);
   for(bandIndex = 0; bandIndex < inputBands; ++bandIndex)
   {
      double minPix = theInputConnection->getMinPixelValue(bandIndex);

      if(minPix < minValue)
      {
         minValue = minPix;
      }
   }

   return minValue;
}

double rspfBandAverageFilter::getMaxPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getMaxPixelValue(band);
   }
   
   // loop through each band and find the max pix value
   rspf_uint32 bandIndex = 0;
   rspf_uint32 inputBands = theInputConnection->getNumberOfOutputBands();
   double maxValue = rspf::defaultMin(RSPF_DOUBLE);
   for(bandIndex = 0; bandIndex < inputBands; ++bandIndex)
   {
      double maxPix = theInputConnection->getMaxPixelValue(bandIndex);

      if(maxPix > maxValue)
      {
         maxValue = maxPix;
      }
   }
   
   return maxValue;
}

void rspfBandAverageFilter::checkWeights()
{
   // we have not been initialzed yet
   //
   if(!theTile)
   {
      return;
   }

   // Check to see if the weights array is the same size as the input
   // list.  If not then resize it and populate them with the same weight
   // value.  It does not matter if they sum to one since we will perform
   // a weighted average in the execution of the algorithm
   //
   if(theWeights.size() != theInputConnection->getNumberOfOutputBands())
   {
      theWeights.resize(theInputConnection->getNumberOfOutputBands());  

      std::fill(theWeights.begin(), theWeights.end(), 1);
   }
}


template<class T> void rspfBandAverageFilter::averageInput(
   T /* dummyVariable */, // used only for template type, value ignored
   rspfRefPtr<rspfImageData> inputDataObject)
{
   // since we have already checked the input for empty or
   // null we will now execute the algorithm.  We will make
   //
   // 
   std::vector<T*> inputBands(inputDataObject->getNumberOfBands());
   std::vector<double> nullValues(inputDataObject->getNumberOfBands());
   rspf_uint32 i = 0;

   // for efficiency we will copy the band pointers and
   // null values so we don't have extra function calls
   // on a per band basis
   //
   for(i = 0; i < inputDataObject->getNumberOfBands(); ++i)
   {
      inputBands[i] = static_cast<T*>(inputDataObject->getBuf(i));
      nullValues[i] = inputDataObject->getNullPix(i);
   }

   // store the output buffer pointer
   T* outputBuf = static_cast<T*>(theTile->getBuf(0));

   // setup index values and boundary values
   rspf_uint32 upperBound = theTile->getWidth()*theTile->getHeight();
   rspf_uint32 bandIndex = 0;
   rspf_uint32 numberOfBands = inputDataObject->getNumberOfBands();
   double outputNullPix = theTile->getNullPix(0);
   
   for(i = 0; i < upperBound; ++i)
   {
      double value=0.0;
      double sumWeights=0.0;

      // loop over each band only using the valid pixel data
      // in the weighted average.
      //
      for(bandIndex = 0; bandIndex < numberOfBands; ++bandIndex)
      {
         // if valid then muliply the value by the weight and add it to
         // current pixel value
         if(*inputBands[bandIndex] != nullValues[bandIndex])
         {
            value += theWeights[bandIndex]*((double)*inputBands[bandIndex]);
            sumWeights += theWeights[bandIndex];
         }
         // increment to the next pixel for the next time around
         ++inputBands[bandIndex];
      }

      // check to see if we had any valid data.  If we did the weight
      // should be greater than 0
      if(sumWeights != 0.0)
      {
         value /= sumWeights;
      }
      else
      {
         // else we set the pixel to the output null value
         value = outputNullPix;
      }
      // store the result in the outputBuffer.
      *outputBuf = static_cast<T>(value);

      // advance the output buffer to the next pixel value
      ++outputBuf;
   }
}


bool rspfBandAverageFilter::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   // get the value of the stored keyword
   rspfString weightString = kwl.find(prefix, WEIGHTS_KW);
   weightString = weightString.trim();
   

   theWeights.clear();
   if(weightString != "")
   {
      // split the string list into an array of strings
      std::vector<rspfString> weightList = weightString.split(" ");

      // resize the weights to the size of the weight list
      //
      theWeights.resize(weightList.size());
      rspf_uint32 i = 0;

      // now store the weights to the array
      for(i = 0; i < theWeights.size(); ++i)
      {
         theWeights[i] = weightList[i].toDouble();
      }
   }

   // call base class to continue the load state
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfBandAverageFilter::saveState(rspfKeywordlist& kwl,
                                       const char* prefix)const
{
   std::stringstream out;

   // copy the weights  to a memory stream separated by space
   std::copy(theWeights.begin(),
             theWeights.end(),
             ostream_iterator<double>(out, " "));

   rspfString weightString = out.str();
   weightString = weightString.trim();

   
   kwl.add(prefix, // prefix to help uniquely id or attributes
           WEIGHTS_KW, // the keyword to identity our attribute
           weightString.c_str(),     // the value 
           true); // overwrite if already in the keywordlist
   
   // call base classes save to allow us to pass the information up
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

double rspfBandAverageFilter::getWeight(rspf_uint32 band)const
{
   if(band < theWeights.size())
   {
      return theWeights[band];
   }
   
   return 0.0;
}

void rspfBandAverageFilter::setWeight(rspf_uint32 band, double weight)
{
   if(band < theWeights.size())
   {
      theWeights[band] = fabs(weight);
   }
}
