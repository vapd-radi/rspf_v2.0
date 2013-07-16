//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description: 
//
// $Id: rspfVpfTileSource.cpp 17932 2010-08-19 20:34:35Z dburken $
//----------------------------------------------------------------------------
#include <rspf/imaging/rspfVpfTileSource.h>

RTTI_DEF2(rspfVpfTileSource, "rspfVpfTileSource", rspfImageHandler, rspfViewInterface);

rspfVpfTileSource::rspfVpfTileSource()
   :
      rspfViewInterface()
{
   theObject = this;
   m_AnnotationSource = new rspfVpfAnnotationSource;
   m_AnnotationSource->setNumberOfBands(3);
}

rspfVpfTileSource::~rspfVpfTileSource()
{
   m_AnnotationSource = 0;
}

bool rspfVpfTileSource::saveState(rspfKeywordlist& kwl,
				   const char* prefix)const
{
   return m_AnnotationSource->saveState(kwl, prefix);
}

bool rspfVpfTileSource::loadState(const rspfKeywordlist& kwl,
				   const char* prefix)
{
   if (rspfImageHandler::loadState(kwl, prefix))
   {
      return m_AnnotationSource->loadState(kwl, prefix);
   }

   return false;
}

void rspfVpfTileSource::close()
{
   m_AnnotationSource->close();
}

bool rspfVpfTileSource::open()
{
   return m_AnnotationSource->open(theImageFile);
}

rspfRefPtr<rspfImageData> rspfVpfTileSource::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   return m_AnnotationSource->getTile(tileRect, resLevel);
}

rspf_uint32 rspfVpfTileSource::getNumberOfInputBands() const
{
   return m_AnnotationSource->getNumberOfOutputBands();
}

rspf_uint32 rspfVpfTileSource::getNumberOfOutputBands() const
{
   return m_AnnotationSource->getNumberOfOutputBands();
}

rspf_uint32 rspfVpfTileSource::getNumberOfLines(rspf_uint32 /* reduced_res_level */) const
{
   rspfIrect theBounds = m_AnnotationSource->getBoundingRect();
   
   if(theBounds.hasNans())
   {
      return theBounds.ul().x;
   }
   
   return theBounds.height();
}

rspf_uint32 rspfVpfTileSource::getNumberOfSamples(rspf_uint32 /* reduced_res_level */) const
{
   rspfIrect theBounds = m_AnnotationSource->getBoundingRect();
   
   if(theBounds.hasNans())
   {
      return theBounds.ul().x;
   }
   
   return theBounds.height();
}

rspf_uint32 rspfVpfTileSource::getNumberOfDecimationLevels() const
{
   return 32;
}

rspfIrect rspfVpfTileSource::getImageRectangle(rspf_uint32 /* reduced_res_level */) const
{
   return m_AnnotationSource->getBoundingRect();
}

//**************************************************************************************************
// Returns the image geometry object associated with this tile source or NULL if non defined.
//**************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfVpfTileSource::getImageGeometry()
{
   if (theGeometry.valid()) return theGeometry;
   
   rspfRefPtr<rspfImageGeometry> annotGeom = m_AnnotationSource->getImageGeometry();
   if ( annotGeom.valid() )
   {
      // Copy the annotation source's geometry as our own:
      theGeometry = new rspfImageGeometry(*annotGeom);

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
      
      return theGeometry;
   }
   return rspfRefPtr<rspfImageGeometry>();
}

rspfScalarType rspfVpfTileSource::getOutputScalarType() const
{
   return m_AnnotationSource->getOutputScalarType();
}

rspf_uint32 rspfVpfTileSource::getTileWidth() const
{
   return m_AnnotationSource->getTileWidth();         
}

rspf_uint32 rspfVpfTileSource::getTileHeight() const
{
   return m_AnnotationSource->getTileHeight();
}

rspf_uint32 rspfVpfTileSource::getImageTileWidth() const
{
   return 0;
}

rspf_uint32 rspfVpfTileSource::getImageTileHeight() const
{
   return 0;
}

bool rspfVpfTileSource::isOpen()const
{
   return m_AnnotationSource->isOpen();
}
   
double rspfVpfTileSource::getNullPixelValue(rspf_uint32 band)const
{
   return m_AnnotationSource->getNullPixelValue(band);
}

double rspfVpfTileSource::getMinPixelValue(rspf_uint32 band)const
{
   return m_AnnotationSource->getMinPixelValue(band);
}
      
double rspfVpfTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   return m_AnnotationSource->getMaxPixelValue(band);
}

rspfObject* rspfVpfTileSource::getView()
{
   return m_AnnotationSource->getView();
}

const rspfObject* rspfVpfTileSource::getView()const
{
   return m_AnnotationSource->getView();
}
   
bool rspfVpfTileSource::setView(rspfObject*  baseObject)
{
   return m_AnnotationSource->setView(baseObject);
}

void rspfVpfTileSource::getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& featureList)
{
   m_AnnotationSource->getAllFeatures(featureList);
}

void rspfVpfTileSource::setAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& featureList)
{
   m_AnnotationSource->setAllFeatures(featureList);
}

void rspfVpfTileSource::transformObjects()
{
   m_AnnotationSource->transformObjects();
}

void rspfVpfTileSource::computeBoundingRect()
{
   m_AnnotationSource->computeBoundingRect();
}
