//*******************************************************************. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
//
#include <rspf/imaging/rspfMemoryImageSource.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfScalarTypeLut.h>
RTTI_DEF1(rspfMemoryImageSource, "rspfMemoryImageSource", rspfImageSource);

rspfMemoryImageSource::rspfMemoryImageSource()
   :rspfImageSource(0, 0, 1, true, false)
{
   m_boundingRect.makeNan();
}

void rspfMemoryImageSource::setImage(rspfRefPtr<rspfImageData> image)
{
   m_image = image.get();
	if(m_image.valid())
	{
		m_boundingRect = m_image->getImageRectangle();
	}
	else
	{
		m_boundingRect.makeNan();
	}
   m_result = 0;
}

void rspfMemoryImageSource::setImage(rspfScalarType scalarType,
                                      rspf_uint32 numberOfBands,
                                      rspf_uint32 width,
                                      rspf_uint32 height)
{
   m_image = new rspfImageData(0,
                                 scalarType,
                                 numberOfBands,
                                 width,
                                 height);
   m_image->initialize();
   
   m_boundingRect = m_image->getImageRectangle();
   m_result = 0;
}

void rspfMemoryImageSource::setRect(rspf_uint32 ulx,
                                     rspf_uint32 uly,
                                     rspf_uint32 width,
                                     rspf_uint32 height)
{
   if(m_image.valid())
   {
      m_image->setImageRectangle(rspfIrect(ulx, uly,
                                             ulx + (width-1),
                                             uly + (height-1)));
      m_boundingRect = m_image->getImageRectangle();
   }
}

rspf_uint32 rspfMemoryImageSource::getNumberOfInputBands() const
{
   if(m_image.valid())
   {
      return m_image->getNumberOfBands();
   }
   return 0;;
}

rspf_uint32 rspfMemoryImageSource::getNumberOfOutputBands() const
{
   return getNumberOfInputBands();
}

rspfScalarType rspfMemoryImageSource::getOutputScalarType() const
{
   if(m_image.valid())
   {
      return m_image->getScalarType();
   }
   
   return rspfImageSource::getOutputScalarType();
}

double rspfMemoryImageSource::getNullPixelValue(rspf_uint32 band)const
{
   if(m_image.valid())
   {
      if(band < getNumberOfInputBands())
      {
         return m_image->getNullPix(band);
      }
   }
   return rspfImageSource::getNullPixelValue(band);
}

double rspfMemoryImageSource::getMinPixelValue(rspf_uint32 band)const
{
   if(m_image.valid())
   {
      if(band < getNumberOfInputBands())
      {
         return m_image->getMinPix(band);
      }
   }
   return rspfImageSource::getMinPixelValue(band);
}

double rspfMemoryImageSource::getMaxPixelValue(rspf_uint32 band)const
{
   if(m_image.valid())
   {
      if(band < getNumberOfInputBands())
      {
         return m_image->getMaxPix(band);
      }
   }
   return rspfImageSource::getMaxPixelValue(band);
}

rspfIrect rspfMemoryImageSource::getBoundingRect(rspf_uint32 resLevel)const
{
   if(resLevel == 0)
   {
      if(m_image.valid())
      {
         return m_boundingRect;
      }
   }

   return rspfImageSource::getBoundingRect(resLevel);
}

rspfRefPtr<rspfImageData> rspfMemoryImageSource::getTile(const rspfIrect& rect,
                                                            rspf_uint32 /* resLevel */)
{
   if(!isSourceEnabled()||!m_image.valid()||m_boundingRect.hasNans()) return 0;
   if(!m_result.valid())
   {
      m_result = new rspfImageData(0, getOutputScalarType(), getNumberOfOutputBands(), rect.width(), rect.height());
      m_result->initialize();       
   }

   m_result->setImageRectangle(rect);
   m_result->makeBlank();
   rspfIrect clampRect = m_image->getImageRectangle().clipToRect(rect);
	
   m_result->loadTile(m_image->getBuf(),
	                    m_boundingRect,
	                    RSPF_BSQ);

	m_result->validate();
   return m_result;
}


bool rspfMemoryImageSource::canConnectMyInputTo(rspf_int32 /* myInputIndex */,
                                                 const rspfConnectableObject* /* object */)const
{
   return false;
}
   
void rspfMemoryImageSource::initialize()
{
   m_result = 0;
}

rspf_uint32 rspfMemoryImageSource::getNumberOfDecimationLevels() const
{
   return 1;
}

void rspfMemoryImageSource::getDecimationFactor(rspf_uint32 resLevel,
                                                 rspfDpt& result) const
{
   if (resLevel == 0)
   {
      result.x = 1.0;
      result.y = 1.0;
   }
   else
   {
      result.x = 1.0 / (1<<resLevel);
      result.y = result.x;
   }
}

void rspfMemoryImageSource::getDecimationFactors(std::vector<rspfDpt>& decimations)const
{
   decimations.push_back(rspfDpt(1.0,1.0));
}


bool rspfMemoryImageSource::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   
   rspfString imagePrefix = rspfString(prefix) + "image.";
   rspfString geomPrefix = rspfString(prefix) + "geom.";
   if(m_image.valid())
   {
      m_image->saveState(kwl, imagePrefix);
   }
   if(m_geometry.valid())
   {
      m_geometry->saveState(kwl, geomPrefix);
   }
   return rspfImageSource::saveState(kwl, prefix);
}

bool rspfMemoryImageSource::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool returnResult =  rspfImageSource::loadState(kwl, prefix);
   m_geometry = 0;
   if(returnResult)
   {
      rspfString imagePrefix = rspfString(prefix) + "image.";
      rspfString geomPrefix = rspfString(prefix) + "geom.";
      
      rspfString type = kwl.find(imagePrefix, "type");
      if(!type.empty())
      {
         m_image = new rspfImageData();
         returnResult = m_image->loadState(kwl, imagePrefix.c_str());
         m_image->initialize();
         m_boundingRect = m_image->getImageRectangle();
      }
      
      if(returnResult)
      {
         type = kwl.find(geomPrefix, "type");
         if(!type.empty())
         {
            m_geometry = new rspfImageGeometry();
            returnResult = m_geometry->loadState(kwl, geomPrefix.c_str());
         }
      }
   }
   m_result = 0;
   return returnResult;
}
