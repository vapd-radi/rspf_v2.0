#include <rspfOgrGdalTileSource.h>
RTTI_DEF3(rspfOgrGdalTileSource,
          "rspfOgrGdalTileSource",
          rspfImageHandler,
          rspfViewInterface,
          rspfEsriShapeFileInterface);
rspfOgrGdalTileSource::rspfOgrGdalTileSource()
   :
      rspfImageHandler(),
      rspfViewInterface(),
      rspfEsriShapeFileInterface()
{
   theObject = this;
   theAnnotationSource = new rspfGdalOgrVectorAnnotation();
}
rspfOgrGdalTileSource::~rspfOgrGdalTileSource()
{
   rspfViewInterface::theObject = 0;
   close();
   
}
bool rspfOgrGdalTileSource::open()
{
   return theAnnotationSource->open(theImageFile);
}
bool rspfOgrGdalTileSource::saveState(rspfKeywordlist& kwl,
                                       const char* prefix) const
{
   if(theAnnotationSource.valid())
   {
      theAnnotationSource->saveState(kwl, prefix);
   }
   return rspfImageHandler::saveState(kwl, prefix);
}
bool rspfOgrGdalTileSource::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   if (rspfImageHandler::loadState(kwl, prefix))
   {
      if (open())
      {
         if(theAnnotationSource.valid())
         {
            theAnnotationSource->loadState(kwl, prefix);
         }
         return true;
      }
   }
   return false;
}
rspf_uint32 rspfOgrGdalTileSource::getImageTileWidth() const
{
   return 0;
}
rspf_uint32 rspfOgrGdalTileSource::getImageTileHeight() const
{
   return 0;
}
rspfRefPtr<rspfImageGeometry> rspfOgrGdalTileSource::getInternalImageGeometry() const
{
   theGeometry = theAnnotationSource->getImageGeometry();
   return theGeometry;  
}
void rspfOgrGdalTileSource::close()
{
   theAnnotationSource->close();
}
rspfRefPtr<rspfImageData> rspfOgrGdalTileSource::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   return theAnnotationSource->getTile(tileRect, resLevel);
}
rspf_uint32 rspfOgrGdalTileSource::getNumberOfInputBands() const
{
   return theAnnotationSource->getNumberOfOutputBands();
}
rspf_uint32 rspfOgrGdalTileSource::getNumberOfOutputBands() const
{
   return theAnnotationSource->getNumberOfOutputBands();
}
rspf_uint32 rspfOgrGdalTileSource::getNumberOfLines(rspf_uint32 /* reduced_res_level */ ) const
{
   rspfIrect theBounds = theAnnotationSource->getBoundingRect();
   
   if(theBounds.hasNans())
   {
      return theBounds.ul().x;
   }
   
   return theBounds.height();
}
rspf_uint32 rspfOgrGdalTileSource::getNumberOfSamples(rspf_uint32 /* reduced_res_level */ ) const
{
   rspfIrect theBounds = theAnnotationSource->getBoundingRect();
   
   if(theBounds.hasNans())
   {
      return theBounds.ul().x;
   }
   
   return theBounds.height();
}
rspf_uint32 rspfOgrGdalTileSource::getNumberOfDecimationLevels() const
{
   return 32;
}
rspfIrect rspfOgrGdalTileSource::getImageRectangle(rspf_uint32 /* reduced_res_level */ ) const
{
   return theAnnotationSource->getBoundingRect();
}
rspfScalarType rspfOgrGdalTileSource::getOutputScalarType() const
{
   return theAnnotationSource->getOutputScalarType();
}
rspf_uint32 rspfOgrGdalTileSource::getTileWidth() const
{
   return theAnnotationSource->getTileWidth();         
}
rspf_uint32 rspfOgrGdalTileSource::getTileHeight() const
{
   return theAnnotationSource->getTileHeight();
}
bool rspfOgrGdalTileSource::isOpen()const
{
   return theAnnotationSource->isOpen();
}
   
double rspfOgrGdalTileSource::getNullPixelValue(rspf_uint32 band)const
{
   return theAnnotationSource->getNullPixelValue(band);
}
double rspfOgrGdalTileSource::getMinPixelValue(rspf_uint32 band)const
{
   return theAnnotationSource->getMinPixelValue(band);
}
double rspfOgrGdalTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   return theAnnotationSource->getMaxPixelValue(band);
}
rspfObject* rspfOgrGdalTileSource::getView()
{
   return theAnnotationSource->getView();
}
const rspfObject* rspfOgrGdalTileSource::getView()const
{
   return theAnnotationSource->getView();
}
bool rspfOgrGdalTileSource::setView(rspfObject*  baseObject)
{
   return theAnnotationSource->setView(baseObject);
}
void rspfOgrGdalTileSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(theAnnotationSource.valid())
   {
      theAnnotationSource->setProperty(property);
   }
}
rspfRefPtr<rspfProperty> rspfOgrGdalTileSource::getProperty(const rspfString& name)const
{
   if(theAnnotationSource.valid())
   {
      return theAnnotationSource->getProperty(name);
   }
   
   return 0;
}
void rspfOgrGdalTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   if(theAnnotationSource.valid())
   {
      theAnnotationSource->getPropertyNames(propertyNames); 
   }
}
std::multimap<long, rspfAnnotationObject*> rspfOgrGdalTileSource::getFeatureTable()
{
  if(theAnnotationSource.valid())
  {
    return theAnnotationSource->getFeatureTable();
  }
  return std::multimap<long, rspfAnnotationObject*>();
}
void rspfOgrGdalTileSource::setQuery(const rspfString& query)
{
  if (theAnnotationSource.valid())
  {
    theAnnotationSource->setQuery(query);
  }
}
void rspfOgrGdalTileSource::setGeometryBuffer(rspf_float64 distance, rspfUnitType type)
{
   if (theAnnotationSource.valid())
   {
      theAnnotationSource->setGeometryBuffer(distance, type);
   }
}
bool rspfOgrGdalTileSource::setCurrentEntry(rspf_uint32 entryIdx)
{
   if (theAnnotationSource.valid())
   {
      return theAnnotationSource->setCurrentEntry(entryIdx);
   }
   return false;
}
