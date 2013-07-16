//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationSource.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfAnnotationSource.h>
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/imaging/rspfAnnotationObjectFactory.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/base/rspfKeywordlist.h>

RTTI_DEF1(rspfAnnotationSource,
          "rspfAnnotationSource",
          rspfImageSourceFilter)
   
rspfAnnotationSource::rspfAnnotationSource(rspfImageSource* inputSource)
   :
      rspfImageSourceFilter(inputSource),
      theRectangle(0, 0, 0, 0),
      theNumberOfBands(1),
      theImage(0),
      theTile(0),
      theAnnotationObjectList()
{
   theRectangle.makeNan();
}

rspfAnnotationSource::~rspfAnnotationSource()
{
   deleteAll();
   destroy();
}

void rspfAnnotationSource::destroy()
{
   // theImage is an rspfRefPtr<rspfRgbImage> so this is not a leak.
   theImage = 0;

   // theTile is an rspfRefPtr<rspfImageData> so this is not a leak.
   theTile = 0; 
}

void rspfAnnotationSource::allocate(const rspfIrect& rect)
{
   if (!theImage)
   {
      theImage = new rspfRgbImage();
   }

   if (!theTile)
   {
      theTile = new rspfU8ImageData( this,
                                      theNumberOfBands,
                                      rect.width(),
                                      rect.height() );
      theTile->initialize();
   }
}

rspfScalarType rspfAnnotationSource::getOutputScalarType() const
{
   if(theInputConnection && !isSourceEnabled())
   {
      return theInputConnection->getOutputScalarType();
   }
   return RSPF_UINT8;
}

rspf_uint32 rspfAnnotationSource::getNumberOfOutputBands()const
{
   if(theInputConnection && !isSourceEnabled())
   {
      return theInputConnection->getNumberOfOutputBands();
   }
   return theNumberOfBands;
}


void rspfAnnotationSource::initialize()
{
   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   rspfImageSourceFilter::initialize();

   if (theInputConnection)
   {
      computeBoundingRect();

      // This will call destroy on band count change.
      setNumberOfBands(theInputConnection->getNumberOfOutputBands());
   }
   else
   {
      destroy();
   }
}

rspfIrect rspfAnnotationSource::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfIrect result;
   result.makeNan();
   
   if(theInputConnection)
   {
      rspfIrect temp = theInputConnection->getBoundingRect(resLevel);
      result = temp;
   }
   
   if(result.hasNans())
   {
      result = theRectangle;
   }
   else if(!theRectangle.hasNans())
   {
      result.combine(theRectangle);
   }
      
   return result;
}

void rspfAnnotationSource::setNumberOfBands(rspf_uint32 bands)
{
   theNumberOfBands = bands;
   if (theTile.get())
   {
      if ( theNumberOfBands != theTile->getNumberOfBands() )
      {
         //---
         // This will wide things slick and force an allocate()
         // call on first getTile().
         //---
         destroy();
      }
   }
}

rspfRefPtr<rspfImageData> rspfAnnotationSource::getTile(
   const rspfIrect& tile_rect, rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> inputTile = 0;

   //---
   // NOTE:
   // This source is written to be used with or without an input connection.
   // So any call to inputTile should be preceeded by: if (inputTile.valid())"
   //---
   if(theInputConnection)
   {
      // Fetch tile from pointer from the input source.
      inputTile = theInputConnection->getTile(tile_rect, resLevel);
   }
   
   // Check for remap bypass:
   if ( !theEnableFlag )
   {
      return inputTile;
   }

   // Check for first time through or reallocation force by band change.
   if ( !theTile )
   {
      allocate(tile_rect);
   }

   // Allocation failed!
   if(!theTile)
   {
      return inputTile;
   }

   //---
   // Set the image rectangle and bands.  This will set the origin.
   // 
   // NOTE:  We do this before the "theTile->makeBlank()" call for efficiency
   // since this will force a "rspfImageData::initialize()", which performs
   // a "makeBlank" if a resize is needed due to tile rectangle or number
   // of band changes.
   //---
   theTile->setImageRectangleAndBands( tile_rect, theNumberOfBands );

   //---
   // Start with a blank tile.
   //
   // NOTE: This will not do anything if already blank.
   //---
   theTile->makeBlank();
   
   if(inputTile.valid() &&
      inputTile->getBuf()&&
      (inputTile->getDataObjectStatus()!=RSPF_EMPTY))
   {
      //---
      // Copy the input tile to the output tile performing scalar remap if
      // needed.
      //---
      theTile->loadTile(inputTile.get());
   }

   // Annotate the output tile.
   drawAnnotations(theTile);

   theTile->validate();
      
   return theTile;
}

bool rspfAnnotationSource::addObject(rspfAnnotationObject* anObject)
{
   if(anObject)
   {
      theAnnotationObjectList.push_back(anObject);
      return true;
   }

   return false;
}

bool rspfAnnotationSource::deleteObject(rspfAnnotationObject* anObject)
{
   if(anObject)
   {
      AnnotationObjectListType::iterator current =
         theAnnotationObjectList.begin();
      while(current != theAnnotationObjectList.end())
      {
         if(*current == anObject)
         {
            theAnnotationObjectList.erase(current);
            return true;
         }
         ++current;
      }
   }

   return false;
}

bool rspfAnnotationSource::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   // Save the state of all annotation objects we have.
   AnnotationObjectListType::const_iterator obj =
      theAnnotationObjectList.begin();
   rspf_uint32 objIdx = 0;
   while (obj < theAnnotationObjectList.end())
   {
      rspfString newPrefix = prefix;
      
      newPrefix += (rspfString("object") +
                    rspfString::toString(objIdx) +
                    rspfString("."));
      (*obj)->saveState(kwl, newPrefix.c_str());
      ++objIdx;
      ++obj;
   }
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfAnnotationSource::loadState(const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   rspf_uint32 index = 0;
   rspfString copyPrefix = prefix;
   rspf_uint32 result = kwl.getNumberOfSubstringKeys(copyPrefix +
                                                      "object[0-9]+\\.");
   
   rspf_uint32 numberOfMatches = 0;
   const rspf_uint32 MAX_INDEX = result + 100;
   
   while(numberOfMatches < result)
   {
      rspfString newPrefix = copyPrefix;
      newPrefix += rspfString("object");
      newPrefix += rspfString::toString(index);
      newPrefix += rspfString(".");

      const char* lookup = kwl.find(newPrefix.c_str(), "type");
      if (lookup)
      {
         ++numberOfMatches;
         
         rspfRefPtr<rspfAnnotationObject> obj = 0;
         obj = rspfAnnotationObjectFactory::instance()->
            create(kwl, newPrefix.c_str());
         if (obj.valid())
         {
            if(!addObject(obj.get()))
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfGeoAnnotationSource::loadState\n"
                  << "Object " << obj->getClassName()
                  << " is not a geographic object" << endl;
               obj = 0;
           }
         }
      }

      ++index;
      
      if (index > MAX_INDEX) // Avoid infinite loop...
      {
         break;
      }
   }

   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfAnnotationSource::computeBoundingRect()
{   
   theRectangle.makeNan();

   if(theAnnotationObjectList.size()>0)
   {
      rspfDrect rect;
      theAnnotationObjectList[0]->computeBoundingRect();
      
      theAnnotationObjectList[0]->getBoundingRect(theRectangle);
      
      AnnotationObjectListType::iterator object =
         (theAnnotationObjectList.begin()+1);
      while(object != theAnnotationObjectList.end())
      {
         (*object)->computeBoundingRect();
         (*object)->getBoundingRect(rect);
         if(theRectangle.hasNans())
         {
            theRectangle = rect;
         }
         else if(!rect.hasNans())
         {  
            theRectangle = theRectangle.combine(rect);
         }
         ++object;
      }
   }
}

rspfAnnotationSource::AnnotationObjectListType rspfAnnotationSource::pickObjects(
   const rspfDpt& imagePoint)
{
   AnnotationObjectListType result;
   AnnotationObjectListType::iterator currentObject;

   currentObject = theAnnotationObjectList.begin();

   while(currentObject != theAnnotationObjectList.end())
   {
      if((*currentObject)->isPointWithin(imagePoint))
      {
         result.push_back(*currentObject);
      }
      
      ++currentObject;
   }
   return result;
}

rspfAnnotationSource::AnnotationObjectListType rspfAnnotationSource::pickObjects(
   const rspfDrect& imageRect)
{
   AnnotationObjectListType result;
   AnnotationObjectListType::iterator currentObject;

   currentObject = theAnnotationObjectList.begin();

   while(currentObject != theAnnotationObjectList.end())
   {
      rspfRefPtr<rspfAnnotationObject> current = (*currentObject);
      if(current->isPointWithin(imageRect.ul()))
      {
         result.push_back(*currentObject);
      }
      else if(current->isPointWithin(imageRect.ll()))
      {
         result.push_back(*currentObject);
      }
      else if(current->isPointWithin(imageRect.lr()))
      {
         result.push_back(*currentObject);
      }
      else if(current->isPointWithin(imageRect.ur()))
      {
         result.push_back(*currentObject);
      }
      
      ++currentObject;
   }
   return result;
}

void rspfAnnotationSource::deleteAll()
{
   AnnotationObjectListType::iterator obj;

   theAnnotationObjectList.clear();
}

void rspfAnnotationSource::drawAnnotations(rspfRefPtr<rspfImageData> tile)
{
   theImage->setCurrentImageData(tile);

   if(theImage->getImageData().valid())
   {
      AnnotationObjectListType::iterator object =
         theAnnotationObjectList.begin();
      while(object != theAnnotationObjectList.end())
      {
         if((*object).valid())
         {
            (*object)->draw(*theImage);
         }
         ++object;
      }      
   }
}

const rspfAnnotationSource::AnnotationObjectListType&
rspfAnnotationSource::getObjectList()const
{
   return theAnnotationObjectList;
}

rspfAnnotationSource::AnnotationObjectListType& rspfAnnotationSource::getObjectList()
{
   return theAnnotationObjectList;
}
