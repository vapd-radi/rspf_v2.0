//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageSource.cpp 21637 2012-09-06 21:17:57Z dburken $

#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfKeywordNames.h>

RTTI_DEF1(rspfImageSource,
          "rspfImageSource" ,
          rspfSource)

rspfImageSource::rspfImageSource(rspfObject* owner)
   :rspfSource(owner, 0,0,false,false)
{
}

rspfImageSource::rspfImageSource(rspfObject* owner,
                                   rspf_uint32 inputListSize,
                                   rspf_uint32 outputListSize,
                                   bool inputListIsFixedFlag,
                                   bool outputListIsFixedFlag)
   :
      rspfSource(owner, inputListSize, outputListSize,
                  inputListIsFixedFlag, outputListIsFixedFlag)
{
}

rspfImageSource::~rspfImageSource()
{
}

rspfRefPtr<rspfImageData> rspfImageSource::getTile(const rspfIpt& origin,
                                                      rspf_uint32 resLevel)
{
   rspfIrect tileRect(origin.x,
                       origin.y,
                       origin.x + getTileWidth()  - 1,
                       origin.y + getTileHeight() - 1);
   
   return getTile(tileRect, resLevel);
}

rspfRefPtr<rspfImageData> rspfImageSource::getTile(const rspfIrect& rect,
                                                      rspf_uint32 resLevel)
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                      getInput(0));
   if(inter)
   {
      return inter->getTile(rect, resLevel);
   }
   return NULL;
}

bool rspfImageSource::getTile(rspfImageData* result, rspf_uint32 resLevel)
{
   bool status = true;
   
   if (result)
   {
      result->ref();
      
      rspfIrect tileRect = result->getImageRectangle();
      
      rspfRefPtr<rspfImageData> id = getTile(tileRect, resLevel);
      if (id.valid())
      {
         *result = *(id.get());
      }
      else
      {
         status = false;
      }
      result->unref();
   }

   return status;
}

void rspfImageSource::getDecimationFactor(rspf_uint32 resLevel,
                                           rspfDpt& result)const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      inter->getDecimationFactor(resLevel, result);
   }
}

void rspfImageSource::getDecimationFactors(std::vector<rspfDpt>& decimations)const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      inter->getDecimationFactors(decimations);
   }
}

rspf_uint32 rspfImageSource::getNumberOfDecimationLevels() const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      return inter->getNumberOfDecimationLevels();
   }
   return 0;
}

rspfScalarType rspfImageSource::getOutputScalarType() const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      return inter->getOutputScalarType();
   }
   
   return RSPF_SCALAR_UNKNOWN;
}

rspf_uint32 rspfImageSource::getTileWidth()  const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      return inter->getTileWidth();
   }
   rspfIpt tileSize;
   rspf::defaultTileSize(tileSize);
   
   return tileSize.x;
}

rspf_uint32 rspfImageSource::getTileHeight() const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
   if(inter)
   {
      return inter->getTileHeight();
   }
   
   rspfIpt tileSize;
   rspf::defaultTileSize(tileSize);
   
   return tileSize.y;
}

rspfIrect rspfImageSource::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
   if(inter)
   {
      return inter->getBoundingRect(resLevel);
   }
   rspfIrect rect;
   rect.makeNan();
   return rect;
}

void rspfImageSource::getBoundingRect(rspfIrect& rect, rspf_uint32 resLevel) const
{
   rect = getBoundingRect( resLevel );
}

bool rspfImageSource::saveState(rspfKeywordlist& kwl,
                                 const char* prefix)const
{
   return rspfSource::saveState(kwl, prefix);
}

bool rspfImageSource::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   return rspfSource::loadState(kwl, prefix);
}

//*****************************************************************************
//  METHOD: rspfImageSource::getValidImageVertices()
//*****************************************************************************
void rspfImageSource::getValidImageVertices(std::vector<rspfIpt>& validVertices,
                                             rspfVertexOrdering ordering,
                                             rspf_uint32 resLevel)const
{
  rspfImageSource* inter = PTR_CAST(rspfImageSource,
					      getInput(0));
  if(inter)
    {
      return inter->getValidImageVertices(validVertices,
					  ordering,
                                          resLevel);
    }
   rspfIrect boundingRect = getBoundingRect(resLevel);
   validVertices.clear();
   
   if(ordering == RSPF_CLOCKWISE_ORDER)
   {
      validVertices.push_back(boundingRect.ul());
      validVertices.push_back(boundingRect.ur());
      validVertices.push_back(boundingRect.lr());
      validVertices.push_back(boundingRect.ll());
   }
   else
   {
      validVertices.push_back(boundingRect.ul());
      validVertices.push_back(boundingRect.ll());
      validVertices.push_back(boundingRect.lr());
      validVertices.push_back(boundingRect.ur());
   }
}

double rspfImageSource::getNullPixelValue(rspf_uint32 band)const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      return inter->getNullPixelValue(band);
   }
   
   return rspf::defaultNull(getOutputScalarType());
}

double rspfImageSource::getMinPixelValue(rspf_uint32 band)const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      return inter->getMinPixelValue(band);
   }
   return rspf::defaultMin(getOutputScalarType());
}

double rspfImageSource::getMaxPixelValue(rspf_uint32 band)const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(inter)
   {
      return inter->getMaxPixelValue(band);
   }
   
   return rspf::defaultMax(getOutputScalarType());
}

//**************************************************************************************************
// Default implementation returns the image geometry object associated with the first input source 
// (if any) connected to this source, or NULL.
//**************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfImageSource::getImageGeometry()
{
   rspfRefPtr<rspfImageGeometry> result = 0;
   if ( getInput(0) )
   {
      rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
      if( inter )
      {
         result = inter->getImageGeometry();
      }
   }
   return result;
}

//**************************************************************************************************
//! Default implementation sets geometry of the first input to the geometry specified.
//**************************************************************************************************
void rspfImageSource::setImageGeometry(const rspfImageGeometry* geom)
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
   if (inter)
   {
      inter->setImageGeometry(geom);
   }
}

void rspfImageSource::saveImageGeometry() const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
   if (inter)
   {
      inter->saveImageGeometry();
   }
}

void rspfImageSource::saveImageGeometry(const rspfFilename& geometry_file) const
{
   rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
   if (inter)
   {
      inter->saveImageGeometry(geometry_file);
   }
}

void rspfImageSource::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   const rspf_uint32 INPUT_BANDS = getNumberOfInputBands();
   if ( INPUT_BANDS )
   {
      bandList.resize( INPUT_BANDS );
      for ( rspf_uint32 band = 0; band < INPUT_BANDS; ++band )
      {
         bandList[band] = band;
      }
   }
   else
   {
      bandList.clear();
   }
}

rspf_uint32 rspfImageSource::getNumberOfOutputBands() const
{
   return getNumberOfInputBands();
}

rspfRefPtr<rspfProperty> rspfImageSource::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = rspfSource::getProperty(name);

   rspfString tempName = name;
   if(result.valid())
   {
      if(tempName.downcase() == rspfKeywordNames::ENABLED_KW)
      {
         result->clearChangeType();
         
         // we will at least say its a radiometric change
         //
         result->setFullRefreshBit();
      }
   }

   return result;
}

void rspfImageSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfSource::setProperty(property);
}

void rspfImageSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfSource::getPropertyNames(propertyNames);
}

bool rspfImageSource::isIndexedData() const
{
   bool result = false;
   rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput(0));
   if (inter)
   {
      result = inter->isIndexedData();
   }
   return result;
}

// Protected to hide from use...
rspfImageSource::rspfImageSource (const rspfImageSource& /* rhs */)
   :rspfSource() 
{}

// Protected to hide from use...
const rspfImageSource& rspfImageSource::operator=(const rspfImageSource&)
{
   return *this;
}
