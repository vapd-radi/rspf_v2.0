//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: implementation for base fusion
//
//*************************************************************************
// $Id: rspfFusionCombiner.cpp 13312 2008-07-27 01:26:52Z gpotts $

#include <rspf/imaging/rspfFusionCombiner.h>
#include <rspf/imaging/rspfImageDataFactory.h>
RTTI_DEF1(rspfFusionCombiner, "rspfFusionCombiner", rspfImageCombiner);

rspfFusionCombiner::rspfFusionCombiner()
  :rspfImageCombiner(NULL, 2, 0, true, false),
   theTile(NULL),
   theNormTile(NULL),
   theNormIntensity(NULL),
   theInputConnection(NULL),
   theIntensityConnection(NULL)
{
}

rspfFusionCombiner::rspfFusionCombiner(rspfObject* owner)
  :rspfImageCombiner(owner, 2, 0, true, false),
   theTile(NULL),
   theNormTile(NULL),
   theNormIntensity(NULL),
   theInputConnection(NULL),
   theIntensityConnection(NULL)
{}

rspfFusionCombiner::~rspfFusionCombiner()
{
   theInputConnection     = NULL;
   theIntensityConnection = NULL;
}

rspfIrect rspfFusionCombiner::getBoundingRect(rspf_uint32 resLevel) const
{
  rspfIrect result;
  rspfIrect colorRect;
  result.makeNan();
  
  if(theIntensityConnection)
  {
     result = theIntensityConnection->getBoundingRect(resLevel);
  }
  if(theInputConnection)
  {
     colorRect = theInputConnection->getBoundingRect(resLevel);
     if(theIntensityConnection)
     {
        result = result.clipToRect(colorRect);
     }
     else
     {
        result = colorRect;
     }
  }
  
  return result;
}

bool rspfFusionCombiner::canConnectMyInputTo(rspf_int32 inputIndex,
					      const rspfConnectableObject* object)const
{
  return ((inputIndex<2)&&
	  (PTR_CAST(rspfImageSource, object)||!object));
}

rspfScalarType rspfFusionCombiner::getOutputScalarType() const
{
   if(theInputConnection)
   {
      return theInputConnection->getOutputScalarType();
   }

   return rspfImageCombiner::getOutputScalarType();
}


rspfRefPtr<rspfImageData> rspfFusionCombiner::getNormIntensity(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if(theIntensityConnection)
   {
      rspfRefPtr<rspfImageData> data =
         theIntensityConnection->getTile(rect, resLevel);
      if(data.valid() && data->getBuf())
      {
         if(!theNormIntensity.valid())
         {
            theNormIntensity = new rspfImageData(this,
                                                  RSPF_NORMALIZED_FLOAT,
                                                  data->getNumberOfBands(),
                                                  rect.width(),
                                                  rect.height());
            theNormIntensity->initialize();
         }

         theNormIntensity->setImageRectangleAndBands(rect,
                                                     data->getNumberOfBands());

         data->copyTileToNormalizedBuffer((float*)theNormIntensity->getBuf());
         theNormIntensity->setDataObjectStatus(data->getDataObjectStatus());
         return theNormIntensity;
      }
   }
   return rspfRefPtr<rspfImageData>();
}

rspfRefPtr<rspfImageData> rspfFusionCombiner::getNormTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageData> data = theInputConnection->getTile(rect,
                                                                     resLevel);
      if(data.valid() && data->getBuf())
      {
         // make sure the tile result is updated if changed.
         //
         if((data->getNumberOfBands() != theTile->getNumberOfBands())||
            (data->getScalarType()    != theTile->getScalarType()))
         {
            theTile = rspfImageDataFactory::instance()->create(this,
                                                                theInputConnection);
            theTile->initialize();
         }

         if(!theNormTile.valid())
         {
            theNormTile = new rspfImageData(this,
                                             RSPF_NORMALIZED_FLOAT,
                                             data->getNumberOfBands(),
                                             rect.width(),
                                             rect.height());
            theNormTile->initialize();
         }
         else if(theNormTile->getNumberOfBands() != data->getNumberOfBands())
         {
            theNormTile = new rspfImageData(this,
                                             RSPF_NORMALIZED_FLOAT,
                                             data->getNumberOfBands(),
                                             rect.width(),
                                             rect.height());
            theNormTile->initialize();
            
         }

         theNormTile->setImageRectangleAndBands(rect,
                                                data->getNumberOfBands());

         data->copyTileToNormalizedBuffer((float*)theNormTile->getBuf());
         theNormTile->setDataObjectStatus(data->getDataObjectStatus());
         return theNormTile;
      }
   }
   return rspfRefPtr<rspfImageData>();
}

void rspfFusionCombiner::initialize()
{
   rspfImageCombiner::initialize();
   
   theInputConnection     = PTR_CAST(rspfImageSource, getInput(0));
   theIntensityConnection = PTR_CAST(rspfImageSource, getInput(1));

   if(getInput(0)&&getInput(1))
   {
      rspfImageSource* temp  = PTR_CAST(rspfImageSource,
                                                  getInput(0));
      rspfImageSource* temp2 = PTR_CAST(rspfImageSource,
                                                  getInput(1));

      if(temp&&temp2)
      {
         if((temp->getNumberOfOutputBands()==1)&&
            (temp2->getNumberOfOutputBands()!=1))
         {
            theIntensityConnection = PTR_CAST(rspfImageSource,
                                              getInput(0));
            theInputConnection     = PTR_CAST(rspfImageSource,
                                              getInput(1));
         }
      }
   }
   
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this,
                                                          theInputConnection);
      theTile->initialize();
   }
}
