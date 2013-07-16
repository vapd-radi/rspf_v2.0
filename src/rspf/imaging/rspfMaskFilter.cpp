//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Modified by: Elan Sharghi (1/20/2009)
// Description: Class definition for rspfMaskFilter.
//
//*************************************************************************
// $Id: rspfMaskFilter.cpp 20409 2011-12-22 16:57:05Z dburken $

#include <rspf/imaging/rspfMaskFilter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageDataFactory.h>

#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfStringProperty.h>

static const char * MASK_FILTER_MASK_TYPE_KW = "mask_type";

RTTI_DEF1(rspfMaskFilter, "rspfMaskFilter", rspfImageSource);

rspfMaskFilter::rspfMaskFilter(rspfObject* owner)
   :
   rspfImageSource(owner, 1, 1, true, false),
   theMaskType(RSPF_MASK_TYPE_SELECT),
   theTile(0)
{
}

rspfMaskFilter::~rspfMaskFilter()
{
}

//*************************************************************************************************
void rspfMaskFilter::setMaskSource(rspfImageSource* maskSource)
{
   theMaskSource = maskSource;
}

rspfRefPtr<rspfImageData> rspfMaskFilter::getTile(const rspfIrect& rect,
                                                     rspf_uint32 resLevel)
{
   rspfImageSource* imageSource = PTR_CAST(rspfImageSource, getInput(0));
   if (!imageSource || !theMaskSource.valid())
   {
      rspfRefPtr<rspfImageData>();
      return theTile;
   }

   rspfRefPtr<rspfImageData> imageSourceData;
   rspfRefPtr<rspfImageData> maskSourceData;
   
   imageSourceData = imageSource->getTile(rect, resLevel);
   if(!isSourceEnabled())
      return imageSourceData;
   
   if (!theTile.valid())
      allocate();

   maskSourceData = theMaskSource->getTile(rect, resLevel);
   if(!imageSourceData.valid() || !theTile.valid())
   {
      return rspfRefPtr<rspfImageData>();
   }
   
   theTile->setOrigin(rect.ul());
   if(theTile->getImageRectangle() != rect)
   {
      theTile->setImageRectangle(rect);
      theTile->initialize();
   }
   if(!imageSourceData.valid())
   {
      return theTile;
   }
   if(!maskSourceData.valid()) 
   {
      return imageSourceData;
   }
   
   if(imageSourceData->getDataObjectStatus() != RSPF_NULL)
   {
      return executeMaskFilter(imageSourceData, maskSourceData);  
   }
   
   return theTile;
}

bool rspfMaskFilter::canConnectMyInputTo(rspf_int32 /* index */,
                                          const rspfConnectableObject* object)const
{
   return (PTR_CAST(rspfImageSource, object)!= 0);
}

void rspfMaskFilter::initialize()
{
   if(getInput(0))
   {
      // Force an allocate on the next getTile.
      theTile = 0;
   }
}

void rspfMaskFilter::allocate()
{
   if(getInput())
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      theTile->initialize();
   }
}

rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilter(
   rspfRefPtr<rspfImageData> imageSourceData,
   rspfRefPtr<rspfImageData> maskSourceData)
{
   if(!theTile)
   {
      theTile = (rspfImageData*)imageSourceData->dup();
      if(!theTile->getBuf())
      {
         theTile->initialize();
      }
   }
   else
   {
      rspf_uint32 tw = theTile->getWidth();
      rspf_uint32 th = theTile->getHeight();
      rspf_uint32 dw = imageSourceData->getWidth();
      rspf_uint32 dh = imageSourceData->getHeight();
      
      theTile->setWidthHeight(imageSourceData->getWidth(),
			      imageSourceData->getHeight());
      theTile->setOrigin(imageSourceData->getOrigin());
      if((tw*th) != (dw*dh))
      {
         theTile->initialize();
      }
      theTile->setDataObjectStatus(imageSourceData->getDataObjectStatus());
   }
   theTile->loadTile(imageSourceData.get());
   theTile->setDataObjectStatus(imageSourceData->getDataObjectStatus());
   switch(theMaskType)
   {
      case RSPF_MASK_TYPE_SELECT:
      case RSPF_MASK_TYPE_SELECT_CLAMP_MIN:
      {
         return executeMaskFilterSelect(theTile, maskSourceData);
      }
      case RSPF_MASK_TYPE_INVERT:
      {
         return executeMaskFilterInvertSelect(theTile, maskSourceData);
      }
      case RSPF_MASK_TYPE_WEIGHTED:
      {
         return executeMaskFilterWeighted(theTile, maskSourceData);
      }
      case RSPF_MASK_TYPE_BINARY:
      case RSPF_MASK_TYPE_BINARY_INVERSE:
      {
         return executeMaskFilterBinary(theTile, maskSourceData);
      }
   }
   
   return imageSourceData;
}

rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterSelect(
   rspfRefPtr<rspfImageData> imageSourceData,
   rspfRefPtr<rspfImageData> maskSourceData)
{
   if(maskSourceData->getScalarType() != RSPF_UCHAR)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterSelect WARNING: Only uchar masks are supported" << endl;
      return imageSourceData;
   }

   switch(imageSourceData->getScalarType())
   {
   case RSPF_UCHAR:
   {
      executeMaskFilterSelection((rspf_uint8)0,
                                 (rspf_uint8)0,
                                 imageSourceData,
                                 maskSourceData);
      break;
   }
   case RSPF_USHORT11:
   case RSPF_USHORT16:
   {
      executeMaskFilterSelection((rspf_uint16)0,
                                 (rspf_uint8)0,
                                 imageSourceData,
                                 maskSourceData);
      break;
   }
   case RSPF_SSHORT16:
   {
      executeMaskFilterSelection((rspf_sint16)0,
                                 (rspf_uint8)0,
                                 imageSourceData,
                                 maskSourceData);
      break;
   }
   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
   {
      executeMaskFilterSelection((float)0,
                                 (rspf_uint8)0,
                                 imageSourceData,
                                 maskSourceData);
      break;
   }
   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
   {
      executeMaskFilterSelection((double)0,
                                 (rspf_uint8)0,
                                 imageSourceData,
                                 maskSourceData);
      break;
   }
   default:
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterSelect WARNING: Unknown scalar type" << endl;
      break;
   }   
   }

   return theTile;
}

rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterInvertSelect(rspfRefPtr<rspfImageData> imageSourceData,
                                                                           rspfRefPtr<rspfImageData> maskSourceData)
{
   if(maskSourceData->getScalarType() != RSPF_UCHAR)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterSelect WARNING: Only uchar masks are supported" << endl;
      return imageSourceData;
   }

   switch(imageSourceData->getScalarType())
   {
   case RSPF_UCHAR:
   {
      executeMaskFilterInvertSelection((rspf_uint8)0,
                                       (rspf_uint8)0,
                                       imageSourceData,
                                       maskSourceData);
      break;
   }
   case RSPF_USHORT11:
   case RSPF_USHORT16:
   {
      executeMaskFilterInvertSelection((rspf_uint16)0,
                                       (rspf_uint8)0,
                                       imageSourceData,
                                       maskSourceData);
      break;
   }
   case RSPF_SSHORT16:
   {
      executeMaskFilterInvertSelection((rspf_sint16)0,
                                       (rspf_uint8)0,
                                       imageSourceData,
                                       maskSourceData);
      break;
   }
   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
   {
      executeMaskFilterInvertSelection((float)0,
                                       (rspf_uint8)0,
                                       imageSourceData,
                                       maskSourceData);
      break;
   }
   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
   {
      executeMaskFilterInvertSelection((double)0,
                                       (rspf_uint8)0,
                                       imageSourceData,
                                       maskSourceData);
      break;
   }
   default:
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterSelect WARNING: Unknown scalar type" << endl;
      break;
   }   
   }

   return theTile;
}

rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterWeighted(rspfRefPtr<rspfImageData> imageSourceData,
                                                                       rspfRefPtr<rspfImageData> maskSourceData)
{
   if(maskSourceData->getScalarType() != RSPF_UCHAR)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterSelect WARNING: Only uchar masks are supported" << endl;
      return imageSourceData;
   }

   switch(imageSourceData->getScalarType())
   {
   case RSPF_UCHAR:
   {
      executeMaskFilterWeighted((rspf_uint8)0,
                                (rspf_uint8)0,
                                imageSourceData,
                                maskSourceData);
      break;
   }
   case RSPF_USHORT11:
   case RSPF_USHORT16:
   {
      executeMaskFilterWeighted((rspf_uint16)0,
                                (rspf_uint8)0,
                                imageSourceData,
                                maskSourceData);
      break;
   }
   case RSPF_SSHORT16:
   {
      executeMaskFilterWeighted((rspf_sint16)0,
                                (rspf_uint8)0,
                                imageSourceData,
                                maskSourceData);
      break;
   }
   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
   {
      executeMaskFilterWeighted((float)0,
                                (rspf_uint8)0,
                                imageSourceData,
                                maskSourceData);
      break;
   }
   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
   {
      executeMaskFilterWeighted((double)0,
                                (rspf_uint8)0,
                                imageSourceData,
                                maskSourceData);
      break;
   }
   default:
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterSelect WARNING: Unknown scalar type" << endl;
      break;
   }   
   }

   return theTile;
}

rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterBinary(
   rspfRefPtr<rspfImageData> imageSourceData, rspfRefPtr<rspfImageData> maskSourceData)
{
   if(maskSourceData->getScalarType() != RSPF_UCHAR)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterBinary WARNING: Only uchar masks are supported" << endl;
      return imageSourceData;
   }
   
   switch(imageSourceData->getScalarType())
   {
      case RSPF_UCHAR:
      {
         executeMaskFilterBinarySelection((rspf_uint8)0,
                                          (rspf_uint8)0,
                                          imageSourceData,
                                          maskSourceData);
         break;
      }
      case RSPF_USHORT11:
      case RSPF_USHORT16:
      {
         executeMaskFilterBinarySelection((rspf_uint16)0,
                                          (rspf_uint8)0,
                                          imageSourceData,
                                          maskSourceData);
         break;
      }
      case RSPF_SSHORT16:
      {
         executeMaskFilterBinarySelection((rspf_sint16)0,
                                          (rspf_uint8)0,
                                          imageSourceData,
                                          maskSourceData);
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         executeMaskFilterBinarySelection((float)0,
                                          (rspf_uint8)0,
                                          imageSourceData,
                                          maskSourceData);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         executeMaskFilterBinarySelection((double)0,
                                          (rspf_uint8)0,
                                          imageSourceData,
                                          maskSourceData);
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN) << "rspfMaskFilter::executeMaskFilterBinary WARNING: Unknown scalar type" << endl;
         break;
      }
   }

   return theTile;
}

template <class inputT, class maskT>
rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterSelection(
   inputT /* dummyInput */,
   maskT  /* dummyMask */,
   rspfRefPtr<rspfImageData> imageSourceData,
   rspfRefPtr<rspfImageData> maskSourceData)
{
   rspfDataObjectStatus maskDataStatus  = maskSourceData->getDataObjectStatus();
   rspfDataObjectStatus inputDataStatus = imageSourceData->getDataObjectStatus();

   // First just check if mask is full, which means pass the image source along untouched:
   if (maskDataStatus == RSPF_FULL)
   {
      theTile = imageSourceData;
      return theTile;
   }

   // Then check for a total mask (all mask values are 0) and return empty tile if so:
   if( (maskDataStatus == RSPF_NULL) || (maskDataStatus == RSPF_EMPTY))
   {
      theTile->makeBlank();
      return theTile;
   }

   // Finally check for blank source tile and return it if so:
   if( (inputDataStatus == RSPF_NULL) || (inputDataStatus == RSPF_EMPTY))
   {
      theTile->makeBlank();
      return theTile;
   }

   rspf_uint32 maskBands  = maskSourceData->getNumberOfBands();
   rspf_uint32 inputBands = imageSourceData->getNumberOfBands();
   if(maskBands&&inputBands)
   {
      rspf_uint32 maxOffset = theTile->getWidth()*theTile->getHeight();
      for(rspf_uint32 band = 0; band < inputBands; ++band)
      {
         maskT*  bufMask = static_cast<maskT*> ( maskSourceData->getBuf() );
         inputT* bufIn   = static_cast<inputT*>( imageSourceData->getBuf(band) );
         inputT* bufOut  = static_cast<inputT*>( theTile->getBuf(band) );
         inputT  nullPix = static_cast<inputT> ( theTile->getNullPix(band) );
         inputT  minPix  = static_cast<inputT> ( theTile->getMinPix(band) );
         rspf_uint32 offset = 0;
         for(offset = 0; offset < maxOffset; ++offset)
         {
            if(*bufMask)
            {
               if ( theMaskType == RSPF_MASK_TYPE_SELECT )
               {
                  *bufOut = *bufIn;
               }
               else
               {
                  //---
                  // RSPF_MASK_TYPE_SELECT_CLAMP_MIN
                  // Use input pixel clamping any nulls to min.
                  //---
                  *bufOut = *bufIn != nullPix ? *bufIn : minPix;
               }
            }
            else
            {
               *bufOut = nullPix;
            }
            ++bufOut;
            ++bufIn;
            ++bufMask;
         }
      }
      theTile->validate();
   }
   
   return theTile;
}

template <class inputT, class maskT>
rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterInvertSelection(
   inputT /* dummyInput */,
   maskT  /* dummyMask */,
   rspfRefPtr<rspfImageData> imageSourceData,
   rspfRefPtr<rspfImageData> maskSourceData)
{
   rspfDataObjectStatus maskDataStatus  = maskSourceData->getDataObjectStatus();
   rspfDataObjectStatus inputDataStatus = imageSourceData->getDataObjectStatus();

   // First just check if mask is empty, which means pass the image source along untouched:
   if( (maskDataStatus == RSPF_NULL) || (maskDataStatus == RSPF_EMPTY))
   {
      theTile = imageSourceData;
      return theTile;
   }

   // Then check for a total mask (all mask values are 1) and return empty tile if so:
   if (maskDataStatus == RSPF_FULL)
   {
      theTile->makeBlank();
      return theTile;
   }

   // Finally check for blank source tile and return it if so:
   if( (inputDataStatus == RSPF_NULL) || (inputDataStatus == RSPF_EMPTY))
   {
      theTile->makeBlank();
      return theTile;
   }

   rspf_uint32 maskBands  = maskSourceData->getNumberOfBands();
   rspf_uint32 inputBands = imageSourceData->getNumberOfBands();
   if(maskBands&&inputBands)
   {
      rspf_uint32 maxOffset = theTile->getWidth()*theTile->getHeight();
      for(rspf_uint32 band = 0; band < inputBands; ++band)
      {
         maskT*    bufMask = (maskT*)maskSourceData->getBuf();
         inputT* bufIn     = (inputT*)imageSourceData->getBuf(band);
         inputT* bufOut    = (inputT*)theTile->getBuf(band);
         inputT  np        = (inputT)theTile->getNullPix(band);
         rspf_uint32 offset = 0;
         for(offset = 0; offset < maxOffset; ++offset)
         {
            if(!*bufMask)
            {
               *bufOut = *bufIn;
            }
            else
            {
               *bufOut = np;
            }
            ++bufOut;
            ++bufIn;
            ++bufMask;
         }
      }
      theTile->validate();
   }
   
   return theTile;
}

template <class inputT, class maskT>
rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterWeighted(
   inputT /* dummyInput */,
   maskT  /* dummyMask */,
   rspfRefPtr<rspfImageData> imageSourceData,
   rspfRefPtr<rspfImageData> maskSourceData)
{
   rspfDataObjectStatus maskDataStatus  = maskSourceData->getDataObjectStatus();
   rspfDataObjectStatus inputDataStatus = imageSourceData->getDataObjectStatus();

   if( (maskDataStatus == RSPF_NULL)||
       (maskDataStatus == RSPF_EMPTY))
   {
      theTile->makeBlank();
      return theTile;
   }
   if( (inputDataStatus == RSPF_NULL)||
       (inputDataStatus == RSPF_EMPTY))
   {
      return theTile;
   }
   
   rspf_uint32 maskBands  = maskSourceData->getNumberOfBands();
   rspf_uint32 inputBands = imageSourceData->getNumberOfBands();
   if(maskBands&&inputBands)
   {
      rspf_uint32 maxOffset = theTile->getWidth()*theTile->getHeight();
      for(rspf_uint32 band = 0; band < inputBands; ++band)
      {
         maskT*    bufMask = (maskT*)maskSourceData->getBuf();
         inputT* bufIn     = (inputT*)imageSourceData->getBuf(band);
         inputT* bufOut    = (inputT*)theTile->getBuf(band);
         inputT  np        = (inputT)theTile->getNullPix(band);
         inputT  minp      = (inputT)theTile->getMinPix(band);
         rspf_uint32 offset = 0;

         if(inputDataStatus == RSPF_PARTIAL)
         {
            for(offset = 0; offset < maxOffset; ++offset)
            {
               if(*bufIn != np)
               {
                  *bufOut = (inputT)((*bufIn)*((double)(*bufMask)/255.0));
                  if((*bufOut != np)&&
                     (*bufOut < minp))
                  {
                     *bufOut = minp;
                  }
               }
               else
               {
                  *bufOut = np;
               }
               ++bufOut;
               ++bufIn;
               ++bufMask;
            }
         }
         else
         {
            for(offset = 0; offset < maxOffset; ++offset)
            {
               *bufOut = (inputT)((*bufIn)*((double)(*bufMask)/255.0));
               if((*bufOut != np)&&
                  (*bufOut < minp))
               {
                  *bufOut = minp;
               }
               ++bufOut;
               ++bufIn;
               ++bufMask;
            }
         }
      }
      theTile->validate();
   }
   
   return theTile;
}

template <class inputT, class maskT>
rspfRefPtr<rspfImageData> rspfMaskFilter::executeMaskFilterBinarySelection(
   inputT /* dummyInput */,
   maskT  /* dummyMask  */ ,
   rspfRefPtr<rspfImageData> imageSourceData,
   rspfRefPtr<rspfImageData> maskSourceData)
{
   rspf_uint32 maskBands  = maskSourceData->getNumberOfBands();
   rspf_uint32 inputBands = imageSourceData->getNumberOfBands();
   if(maskBands&&inputBands)
   {
      rspf_uint32 maxOffset = theTile->getWidth()*theTile->getHeight();
      for(rspf_uint32 band = 0; band < inputBands; ++band)
      {
         maskT*  bufMask   = (maskT*)maskSourceData->getBuf();
         inputT* bufOut    = (inputT*)theTile->getBuf(band);
         const inputT  NP  = (inputT)theTile->getNullPix(band);
         const inputT  MP  = (inputT)theTile->getMaxPix(band);
         rspf_uint32 offset = 0;
         for(offset = 0; offset < maxOffset; ++offset)
         {
            if (theMaskType == RSPF_MASK_TYPE_BINARY_INVERSE)
            {
               if(*bufMask)      // if mask == 1
                  *bufOut = MP;  // set to max pix
               else              // if mask == 0
                  *bufOut = NP;  // set to null pix
            }
            else
            {
               if(*bufMask)      // if mask == 1
                  *bufOut = NP;  // set to null pix
               else              // if mask == 0
                  *bufOut = MP;  // set to max pix
            }
            ++bufOut;
            ++bufMask;
         }
      }
      theTile->validate();
   }
   
   return theTile;
}

void rspfMaskFilter::setMaskType(rspfFileSelectionMaskType type)
{
   theMaskType = type;
}

void rspfMaskFilter::setMaskType(const rspfString& type)
{
   if(type != "")
   {
      rspfString maskType = type;
      maskType.downcase();

      if(maskType == "select")
      {
         theMaskType = RSPF_MASK_TYPE_SELECT;
      }
      else if(maskType == "invert")
      {
         theMaskType = RSPF_MASK_TYPE_INVERT;
      }
      else if(maskType == "weighted")
      {
         theMaskType = RSPF_MASK_TYPE_WEIGHTED;
      }
      else if(maskType == "binary")
      {
         theMaskType = RSPF_MASK_TYPE_BINARY;
      }
      else if(maskType == "binary_inverse")
      {
         theMaskType = RSPF_MASK_TYPE_BINARY_INVERSE;
      }
      else if(maskType == "select_clamp_min")
      {
         theMaskType = RSPF_MASK_TYPE_SELECT_CLAMP_MIN;
      } 
   }
}

rspfMaskFilter::rspfFileSelectionMaskType rspfMaskFilter::getMaskType()const
{
   return theMaskType;
}

rspfString rspfMaskFilter::getMaskTypeString() const
{
   rspfString maskType;
   
   switch(theMaskType)
   {
      case RSPF_MASK_TYPE_SELECT:
      {
         maskType = "select";
         break;
      }
      case RSPF_MASK_TYPE_INVERT:
      {
         maskType = "invert";
         break;
      }
      case RSPF_MASK_TYPE_WEIGHTED:
      {
         maskType = "weighted";
         break;
      }
      case RSPF_MASK_TYPE_BINARY:
      {
         maskType = "binary";
         break;
      }
      case RSPF_MASK_TYPE_BINARY_INVERSE:
      {
         maskType = "binary_inverse";
         break;
      }
      case RSPF_MASK_TYPE_SELECT_CLAMP_MIN:
      {
         maskType = "select_clamp_min";
         break;
      }
   }

   return maskType;
}

bool rspfMaskFilter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   bool result = rspfImageSource::loadState(kwl, prefix);
   
   theInputListIsFixedFlag  = true;
   theOutputListIsFixedFlag = false;
   setNumberOfInputs(2);

   const char* lookup = kwl.find(prefix, MASK_FILTER_MASK_TYPE_KW);
   if (lookup)
   {
      setMaskType(rspfString(lookup));
   }

   return result;
}

bool rspfMaskFilter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   kwl.add(prefix,
           MASK_FILTER_MASK_TYPE_KW,
           getMaskTypeString().c_str(),
           true);
   
   return rspfImageSource::saveState(kwl, prefix);
}

void rspfMaskFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if( property.valid() )
   {
      if(property->getName() == MASK_FILTER_MASK_TYPE_KW)
      {
         setMaskType(property->valueToString());
      }
      else
      {
         rspfImageSource::setProperty(property);
      }
   }
}

rspfRefPtr<rspfProperty> rspfMaskFilter::getProperty(
   const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;
   
   if(name == MASK_FILTER_MASK_TYPE_KW)
   {
      std::vector<rspfString> constraintList;
      constraintList.push_back(rspfString("select"));
      constraintList.push_back(rspfString("invert"));
      constraintList.push_back(rspfString("weighted"));
      
      result = new rspfStringProperty(MASK_FILTER_MASK_TYPE_KW,
                                       getMaskTypeString(),
                                       false,
                                       constraintList);
   }
   else
   {
      result = rspfImageSource::getProperty(name);
   }
   return result;
}

void rspfMaskFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSource::getPropertyNames(propertyNames);
   propertyNames.push_back(MASK_FILTER_MASK_TYPE_KW);
}

rspf_uint32 rspfMaskFilter::getNumberOfInputBands() const
{
   rspfImageSource* img_source = PTR_CAST(rspfImageSource, getInput());
   if (img_source)
      return img_source->getNumberOfInputBands();
   return 0;
}

