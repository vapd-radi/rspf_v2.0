#include <set>
#include <rspfHdfReader.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id";
#endif
static rspfTrace traceDebug("rspfHdfReader:debug");
RTTI_DEF1_INST(rspfHdfReader, "rspfHdfReader", rspfImageHandler)
bool doubleEquals(rspf_float64 left, rspf_float64 right, rspf_float64 epsilon) 
{
  return (fabs(left - right) < epsilon);
}
rspfHdfReader::rspfHdfReader()
  : rspfImageHandler(),
    m_gdalTileSource(0),
    m_entryFileList(),
    m_numberOfBands(0),
    m_scalarType(RSPF_SCALAR_UNKNOWN),
    m_currentEntryRender(0),
    m_tile(0)
{
}
rspfHdfReader::~rspfHdfReader()
{
   close();
   rspfImageHandler::close();
}
rspfString rspfHdfReader::getShortName()const
{
   return rspfString("rspf_hdf_reader");
}
rspfString rspfHdfReader::getLongName()const
{
   return rspfString("rspf hdf reader");
}
rspfString rspfHdfReader::getClassName()const
{
   return rspfString("rspfHdfReader");
}
rspf_uint32 rspfHdfReader::getNumberOfLines(
   rspf_uint32 resLevel) const
{
   rspf_uint32 result = 0;
   if (resLevel == 0)
   {
      if (m_gdalTileSource.valid())
      {
         result = m_gdalTileSource->getNumberOfLines(resLevel);
      }
   }
   else if (theOverview.valid())
   {
      result = theOverview->getNumberOfSamples(resLevel);
   }
   return result;
}
rspf_uint32 rspfHdfReader::getNumberOfSamples(
   rspf_uint32 resLevel) const
{
   rspf_uint32 result = 0;
   if (resLevel == 0)
   {
      if (m_gdalTileSource.valid())
      {
         result = m_gdalTileSource->getNumberOfSamples(resLevel);
      }
   }
   else if (theOverview.valid())
   {
      result = theOverview->getNumberOfSamples(resLevel);
   }
   return result;
}
bool rspfHdfReader::open()
{
   static const char MODULE[] = "rspfHdfReader::open";
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered...\n"
         << "image: " << theImageFile << "\n";
   }
   bool result = false;
   if (!isSupportedExtension())
   {
      return false;
   }
   if (m_entryFileList.size() == 0)
   {
      if(isOpen())
      {
         close();
      }
      m_gdalTileSource = new rspfGdalTileSource;
      m_gdalTileSource->setFilename(theImageFile);
      if ( m_gdalTileSource->open() == false )
      {
         m_gdalTileSource = 0;
         return false;
      }
      std::vector<rspfString> entryStringList;
      if (m_gdalTileSource != 0)
      {
         m_gdalTileSource->getEntryStringList(entryStringList);
      }
      
      if (entryStringList.size() > 0)
      {
         for (rspf_uint32 i = 0; i < entryStringList.size(); i++)
         {
            m_entryFileList.push_back(i);
         }
      }
      else
      {
        result = false;
      }
   }
      
   if (m_currentEntryRender < m_entryFileList.size())
   {
      
      m_gdalTileSource->setCurrentEntry(m_currentEntryRender);
      m_numberOfBands = m_gdalTileSource->getNumberOfInputBands();
      m_tile = rspfImageDataFactory::instance()->create(this, this);
      m_tile->initialize();
      
      completeOpen();
      result = true;
   }
   else
   {
      result = false;
   }
   
   if (result == false)
   {
      close();
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (result?"true":"false\n")
         << std::endl;
   }
   
   return result;
}
bool rspfHdfReader::isSDSDataset(rspfString fileName)
{
  std::vector<rspfString> fileList = fileName.split(":");
  if (fileList.size() > 0)
  {
    rspfString  subName = fileList[0];
    if (subName.contains("_SDS"))
    {
      return true;
    }
  }
  return false;
}
bool rspfHdfReader::isOpen()const
{
   return m_gdalTileSource.get();
}
void rspfHdfReader::close()
{
   m_tile = 0;
   if (m_gdalTileSource.valid())
   {
      m_gdalTileSource = 0;
   }
   m_entryFileList.clear();
}
rspfRefPtr<rspfImageData> rspfHdfReader::getTile(const rspfIrect& tile_rect, 
                                                    rspf_uint32 resLevel)
{
   if (m_tile.valid())
   {
      m_tile->setImageRectangle(tile_rect);
      
      if ( getTile( m_tile.get(), resLevel ) == false )
      {
         if (m_tile->getDataObjectStatus() != RSPF_NULL)
         {
            m_tile->makeBlank();
         }
      }
   }
   return m_tile;
}
bool rspfHdfReader::getTile(rspfImageData* result,
                             rspf_uint32 resLevel)
{
   bool status = false;
   if( isOpen() && isSourceEnabled() && isValidRLevel(resLevel) &&
       result && (result->getNumberOfBands() == getNumberOfOutputBands()) )
   {
      result->ref();  // Increment ref count.
      status = getOverviewTile(resLevel, result);
      if (!status) // Did not get an overview tile.
      {
         status = true;
         rspfIrect tile_rect = result->getImageRectangle();     
         if (getImageRectangle().intersects(tile_rect))
         {
            rspfIrect clipRect = tile_rect.clipToRect(getImageRectangle());
            if (tile_rect.completely_within(clipRect) == false)
            {
               result->makeBlank();
            }
            if (m_gdalTileSource.valid())
            {
               rspfRefPtr<rspfImageData> imageData =
                  m_gdalTileSource->getTile(tile_rect, resLevel);
               result->loadTile(imageData->getBuf(), tile_rect, clipRect, RSPF_BSQ);
            }
     
         }
         else // No intersection...
         {
            result->makeBlank();
         }
      }
      result->validate();
      result->unref();  // Decrement ref count.
   }
   return status;
}
bool rspfHdfReader::isSupportedExtension()
{
   rspfString ext = theImageFile.ext();
   ext.downcase();
   if ( ext == "hdf" || ext == "h4" || ext == "hdf4" || 
     ext == "he4" || ext == "hdf5" || ext == "he5" || ext == "h5" || 
     ext == "l1r")
   {
      return true;
   }
   return false;
}
rspf_uint32 rspfHdfReader::getNumberOfInputBands() const
{
   return m_numberOfBands;
}
rspf_uint32 rspfHdfReader::getNumberOfOutputBands()const
{
   return m_numberOfBands;
}
rspf_uint32 rspfHdfReader::getImageTileWidth() const
{
   rspf_uint32 result = 128;
   if (m_gdalTileSource.valid())
   {
      result = m_gdalTileSource->getImageTileWidth();
   }
   return result;
}
rspf_uint32 rspfHdfReader::getImageTileHeight() const
{
   rspf_uint32 result = 128;
   if (m_gdalTileSource.valid())
   {
      result = m_gdalTileSource->getImageTileHeight();
   }
   return result;
}
rspfScalarType rspfHdfReader::getOutputScalarType() const
{
   if (m_gdalTileSource.valid())
   {
      return m_gdalTileSource->getOutputScalarType(); 
   }
   return m_scalarType;
}
rspfRefPtr<rspfImageGeometry> rspfHdfReader::getImageGeometry()
{
   if ( !theGeometry )
   {
      if ( m_gdalTileSource.valid() )
      {
         theGeometry = m_gdalTileSource->getImageGeometry();
      }
   }
   return theGeometry;
}
bool rspfHdfReader::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool result = false;
   if ( rspfImageHandler::loadState(kwl, prefix) )
   {
      result = open();
   }
   return result;
}
bool rspfHdfReader::setCurrentEntry(rspf_uint32 entryIdx)
{
   if (m_currentEntryRender == entryIdx)
   {
      return true; // Nothing to do...
   }
   theDecimationFactors.clear();
   theGeometry = 0;
   theOverview = 0;
   theOverviewFile.clear();
   m_currentEntryRender = entryIdx;
   return open();
}
rspf_uint32 rspfHdfReader::getNumberOfEntries()const
{
   return m_entryFileList.size();
}
void rspfHdfReader::getEntryList(std::vector<rspf_uint32>& entryList) const
{
   entryList.clear();
   for (rspf_uint32 i = 0; i < m_entryFileList.size(); i++)
   {
      entryList.push_back(m_entryFileList[i]);
   }
}
rspfString rspfHdfReader::getEntryString(rspf_uint32 entryId) const 
{
   if (m_gdalTileSource.valid())
   {
      std::vector<rspfString> entryStringList;
      m_gdalTileSource->getEntryStringList(entryStringList);
      if (entryId < entryStringList.size())
      {
         return entryStringList[entryId];
      }
   }
   return "";
}
rspfString rspfHdfReader::getDriverName()
{
   rspfString result = "";
   if (m_gdalTileSource.valid())
   {
      GDALDriverH driver = m_gdalTileSource->getDriver();
      if (driver)
      {
         result = GDALGetDriverShortName(driver);
      }
   }
   return result;
}
bool rspfHdfReader::setOutputBandList(const vector<rspf_uint32>& outputBandList)
{
   if (outputBandList.size() && m_gdalTileSource != 0)
   {
      m_gdalTileSource->setOutputBandList(outputBandList);
      open();
      return true;
   }
   return false;
}
