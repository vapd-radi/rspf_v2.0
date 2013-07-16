//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for rspfBandSelector.
// 
//*******************************************************************
//  $Id: rspfBandSelector.cpp 22230 2013-04-12 16:34:05Z dburken $

#include <rspf/imaging/rspfBandSelector.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfStringProperty.h>
#include <iostream>
#include <algorithm>

static rspfTrace traceDebug("rspfBandSelector:debug");

RTTI_DEF1(rspfBandSelector,"rspfBandSelector", rspfImageSourceFilter)

rspfBandSelector::rspfBandSelector()
   :
      rspfImageSourceFilter(),
      theTile(0),
      theOutputBandList(0),
      theWithinRangeFlag(rspfBandSelectorWithinRangeFlagState_NOT_SET),
      thePassThroughFlag(false)

{
//   theEnableFlag = false; // Start off disabled.
   theEnableFlag = true; 
}

rspfBandSelector::~rspfBandSelector()
{
   theTile = 0;
}

rspfRefPtr<rspfImageData> rspfBandSelector::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if (!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }

   // Get the tile from the source.
   rspfRefPtr<rspfImageData> t = theInputConnection->getTile(tileRect, resLevel);

   if (!isSourceEnabled())
   {
      return t;  // This tile source bypassed, return the input tile source.
   }

//   if (theOrderedCorrectlyFlag)
//   {
//      return t; // Input band order same as output band order.
//   }

   if(!theTile.valid()) // First time through, might not be initialized...
   {
      allocate();
      if (!theTile.valid())
      {
         // Should never happen...
         return t; // initialize failed.
      }
   }

   theTile->setImageRectangle(tileRect);
   if(theWithinRangeFlag == rspfBandSelectorWithinRangeFlagState_NOT_SET)
   {
      theWithinRangeFlag = ((outputBandsWithinInputRange() == true) ?
                            rspfBandSelectorWithinRangeFlagState_IN_RANGE:
                            rspfBandSelectorWithinRangeFlagState_OUT_OF_RANGE);
   }
   if(theWithinRangeFlag == rspfBandSelectorWithinRangeFlagState_OUT_OF_RANGE)
   {
      theTile->makeBlank();
      return theTile;
   }

   if ( !t.valid() ||
        (t->getDataObjectStatus() == RSPF_EMPTY) ||
        (t->getDataObjectStatus() == RSPF_NULL))
   {
      //---
      // Since we're enabled, we must return our tile not "t" so the
      // correct number of bands goes through the chain.
      //---
      theTile->makeBlank();
      return theTile;
   }

   // Copy selected bands to our tile.
   for (rspf_uint32 i=0; i<theOutputBandList.size(); i++)
   {
      theTile->assignBand(t.get(), theOutputBandList[i], i);
   }
   
   theTile->validate();

   return theTile;
}

void rspfBandSelector::setOutputBandList( const vector<rspf_uint32>& outputBandList )
{
   if (outputBandList.size())
   {
      theOutputBandList = outputBandList;  // Assign the new list.

      bool setBands = false;
      rspfRefPtr<rspfImageHandler> ih = getBandSelectableImageHandler();
      if ( ih.valid() )
      {
         // Our input is a single image chain that can do band selection.
         if ( ih->setOutputBandList( outputBandList ) )
         {
            thePassThroughFlag = true;
            setBands = true;
            theTile = 0; // Don't need.
         }
      }

      if ( setBands == false )
      {
         if ( theTile.valid() && ( theTile->getNumberOfBands() != outputBandList.size() ) )
         {
            theTile = 0;       // Force an allocate call next getTile.
         }
         theWithinRangeFlag = rspfBandSelectorWithinRangeFlagState_NOT_SET;
         checkPassThrough();
         //theOrderedCorrectlyFlag = isOrderedCorrectly();
      }
   }
}

rspf_uint32 rspfBandSelector::getNumberOfOutputBands() const
{
   rspf_uint32 bands;
   
   if(isSourceEnabled())
   {
      bands = static_cast<rspf_uint32>(theOutputBandList.size());
   }
   else
   {
      //---
      // Note:
      // This returns theInputConnection->getNumberOfOutputBands() which is our
      // input.  Calling rspfBandSelector::getNumberOfInputBands() will produce
      // an error if we are bypassed due to a band selectable image handler.
      //---
      bands = rspfImageSourceFilter::getNumberOfInputBands();
   }

   return bands;
}

rspf_uint32 rspfBandSelector::getNumberOfInputBands() const
{
   rspf_uint32 bands;

   // See if we have a single image chain with band selectable image handler.
   rspfRefPtr<rspfImageHandler> ih = getBandSelectableImageHandler();
   if ( ih.valid() )
   {
      bands = ih->getNumberOfInputBands();
   }
   else
   {
      bands = rspfImageSourceFilter::getNumberOfInputBands();
   }
   
   return bands;
}

void rspfBandSelector::initialize()
{
   // Base class will recapture "theInputConnection".
   rspfImageSourceFilter::initialize();
   
   theWithinRangeFlag =  rspfBandSelectorWithinRangeFlagState_NOT_SET;

   if(theInputConnection)
   {
      if ( !theOutputBandList.size() ) 
      {
         // First time through set the output band list to input.
         theInputConnection->getOutputBandList(theOutputBandList);
      }

      // See if we have a single image chain with band selectable image handler.
      rspfRefPtr<rspfImageHandler> ih = getBandSelectableImageHandler();
      if ( ih.valid() )
      {
         if ( theOutputBandList.size() )
         {
            ih->setOutputBandList( theOutputBandList );
         }
         thePassThroughFlag = true;
      }
      else
      {
         checkPassThrough();
      }
      
      if ( isSourceEnabled() )
      {
         // theOrderedCorrectlyFlag = isOrderedCorrectly();
         
         if ( theTile.valid() )
         {
            //---
            // Check for:
            // - ordered correctly
            // - band change
            // - scalar change
            //---
            if( ( theTile->getNumberOfBands() != theOutputBandList.size() ) ||
                ( theTile->getScalarType() !=
                  theInputConnection->getOutputScalarType() ) )
            {
               theTile = 0; // Don't need it.
            }
         }
      }
   }
   else // No input connection.
   {
      thePassThroughFlag = true;
   }

   if ( !isSourceEnabled() )
   {
      theTile = 0;
   }
}

void rspfBandSelector::allocate()
{
   //initialize(); // Update the connection.
   //theOrderedCorrectlyFlag = isOrderedCorrectly();
   theTile = rspfImageDataFactory::instance()->create(this, this);
   theTile->initialize();
}

bool rspfBandSelector::isSourceEnabled()const
{
   bool result = rspfImageSourceFilter::isSourceEnabled();
   if(result)
   {
      // if I am not marked to pass information on through then enable me
      result = !thePassThroughFlag;
   }
   
   return result;
}

double rspfBandSelector::getMinPixelValue(rspf_uint32 band)const
{
   if(theInputConnection)
   {
      if (isSourceEnabled())
      {
         if(band < theOutputBandList.size())
         {
            return theInputConnection->
               getMinPixelValue(theOutputBandList[band]);
         }
         else
         {
            return theInputConnection->getMinPixelValue(band);
         }
      }
      else
      {
         return theInputConnection->getMinPixelValue(band);
      }
   }
   return 0;
}

double rspfBandSelector::getNullPixelValue(rspf_uint32 band)const
{
   if(theInputConnection)
   {
      if (isSourceEnabled())
      {
         if(band < theOutputBandList.size())
         {
            return theInputConnection->
               getNullPixelValue(theOutputBandList[band]);
         }
         else
         {
            return theInputConnection->getNullPixelValue(band);
         }
      }
      else
      {
         return theInputConnection->getNullPixelValue(band);
      }
   }
   
   return 0;
}

double rspfBandSelector::getMaxPixelValue(rspf_uint32 band)const
{
   if(theInputConnection)
   {
      if (isSourceEnabled())
      {
         if(band < theOutputBandList.size())
         {
            return theInputConnection->
               getMaxPixelValue(theOutputBandList[band]);
         }
         else
         {
            return theInputConnection->getMaxPixelValue(band);
         }
      }
      else
      {
        return theInputConnection->getMaxPixelValue(band);
      }
   }
   return 1.0/DBL_EPSILON;
}

bool rspfBandSelector::saveState(rspfKeywordlist& kwl,
                                  const char* prefix)const
{
   rspfString temp;

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_OUTPUT_BANDS_KW,
           static_cast<int>(theOutputBandList.size()),
           true);
   
   rspfString bandsString;
   rspf::toSimpleStringList(bandsString,
                             theOutputBandList);
   kwl.add(prefix,
           rspfKeywordNames::BANDS_KW,
           bandsString,
           true);
/*   
   for(rspf_uint32 counter = 0; counter < theOutputBandList.size();counter++)
   {
      temp  = rspfKeywordNames::BAND_KW;
      temp += rspfString::toString(counter+1);
      
      kwl.add(prefix,
              temp.c_str(),
              rspfString::toString(theOutputBandList[counter]+1).c_str());
   }
*/   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfBandSelector::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   rspfImageSourceFilter::loadState(kwl, prefix);

   theOutputBandList.clear();
   
   rspfString copyPrefix = prefix;
   
   rspfString bands = kwl.find(prefix, rspfKeywordNames::BANDS_KW);
   if(!bands.empty())
   {
      rspf::toSimpleVector(theOutputBandList, bands);
   }
   else
   {
      rspfString regExpression =  rspfString("^(") + copyPrefix + "band[0-9]+)";
      
      vector<rspfString> keys = kwl.getSubstringKeyList( regExpression );
      long numberOfBands = (long)keys.size();
      rspf_uint32 offset = (rspf_uint32)(copyPrefix+"band").size();
      std::vector<int>::size_type idx = 0;
      std::vector<int> numberList(numberOfBands);
      for(idx = 0; idx < keys.size();++idx)
      {
         rspfString numberStr(keys[idx].begin() + offset,
                               keys[idx].end());
         numberList[idx] = numberStr.toInt();
      }
      std::sort(numberList.begin(), numberList.end());
      for(idx=0;idx < numberList.size();++idx)
      {
         const char* bandValue =
         kwl.find(copyPrefix,
                  ("band"+rspfString::toString(numberList[idx])).c_str());
         theOutputBandList.push_back( rspfString(bandValue).toLong()-1);
      }
   }

   initialize();
   
   return true;
}

void rspfBandSelector::checkPassThrough()
{
   thePassThroughFlag = ((theInputConnection == 0)||!outputBandsWithinInputRange());
   
   // check if marked with improper bands
   if(thePassThroughFlag) return;
   
   if(theInputConnection)
   {
      std::vector<rspf_uint32> inputList;
      theInputConnection->getOutputBandList(inputList);

      if ( inputList.size() == theOutputBandList.size() )
      {
         const std::vector<rspf_uint32>::size_type SIZE =
            theOutputBandList.size();
      
         std::vector<rspf_uint32>::size_type i = 0;
         while (i < SIZE)
         {
            if ( inputList[i] != theOutputBandList[i] )
            {
               break;
            }
            ++i;
         }
         if (i == SIZE)
         {
            thePassThroughFlag = true;
         }
      }
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
         << "rspfBandSelector::isOrderedCorrectly() ERROR:"
         << "Method called prior to initialization!\n";
      }
   }

}

bool rspfBandSelector::outputBandsWithinInputRange() const
{
   bool result = false;
   
   if(theInputConnection)
   {
      result = true;
      const rspf_uint32 HIGHEST_BAND = getNumberOfInputBands() - 1;
      const rspf_uint32 OUTPUT_BANDS = (rspf_uint32)theOutputBandList.size();
      for (rspf_uint32 i=0; i<OUTPUT_BANDS; ++i)
      {
         if (theOutputBandList[i] > HIGHEST_BAND)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
               << "rspfBandSelector::outputBandsWithinInputRange() ERROR:"
               << "Output band greater than highest input band. "
               << theOutputBandList[i] << " > " << HIGHEST_BAND << "."
               << std::endl;
            }
            result = false;
            break;
         }
      }
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
         << "rspfBandSelector::outputBandsWithinInputRange() ERROR:"
         << "Method called prior to initialization!" << std::endl;
      }
   }
   return result;
}

void rspfBandSelector::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   if ( isSourceEnabled()&&theOutputBandList.size() )
   {
      bandList = theOutputBandList;
   }
   else if (theInputConnection)
   {
      theInputConnection->getOutputBandList(bandList);
   }
   else
   {
      bandList.clear();
   }
}

rspfString rspfBandSelector::getLongName()const
{
   return rspfString("Band Selector, maps an input band to the output band.");
}

rspfString rspfBandSelector::getShortName()const
{
   return rspfString("Band Selector");
}

void rspfBandSelector::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if(property->getName() == "bandSelection")
   {
     rspfString str = property->valueToString();
     std::vector<rspfString> str_vec;
     std::vector<rspf_uint32> int_vec;
     
     str.split( str_vec, " " );
     
     for ( rspf_uint32 i = 0; i < str_vec.size(); ++i )
     {
        if(!str_vec[i].empty())
        {
           int_vec.push_back( str_vec[i].toUInt32() );
        }
     }
     setOutputBandList( int_vec );
   }
   else if(property->getName() == "bands")
   {
      std::vector<rspf_uint32> selection;
      if(rspf::toSimpleVector(selection, property->valueToString()))
      {
         theOutputBandList = selection;
      }
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfBandSelector::getProperty(const rspfString& name)const
{
   if(name == "bandSelection")
   {
      std::vector<rspf_uint32> bands;
	  
	  getOutputBandList( bands );
	  	  
      std::vector<rspfString> bandNames;

      for(rspf_uint32 i = 0; i < bands.size(); i++)
      {
         bandNames.push_back( rspfString::toString( bands[i] ) );
      }
	  
	  rspfString str;
	  
	  str.join( bandNames, " " );
	  
      rspfStringProperty* stringProp = new rspfStringProperty(name, str);
									
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }
   else if(name == "bands")
   {
      rspfString bandsString;
      rspf::toSimpleStringList(bandsString,
                                theOutputBandList);
      rspfStringProperty* stringProp = new rspfStringProperty(name, bandsString);
      
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfBandSelector::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("bands");
}

rspfRefPtr<rspfImageHandler> rspfBandSelector::getBandSelectableImageHandler() const
{
   rspfRefPtr<rspfImageHandler> ih = 0;

   if ( theInputConnection )
   {
      rspfTypeNameVisitor visitor(rspfString("rspfImageHandler"),
                                   true,
                                   rspfVisitor::VISIT_CHILDREN|rspfVisitor::VISIT_INPUTS);
      
      theInputConnection->accept(visitor);

      // If there are multiple image handlers, e.g. a mosaic do not uses.
      if ( visitor.getObjects().size() == 1 )
      {
         ih = visitor.getObjectAs<rspfImageHandler>( 0 );
         if ( ih.valid() )
         {
            if ( ih->isBandSelector() == false )
            {
               ih = 0;
            }
         }
      }
      
   } // Matches: if ( theInputConnection )
   return ih;
   
} // End: rspfBandSelector::getBandSelectableImageHandler()
