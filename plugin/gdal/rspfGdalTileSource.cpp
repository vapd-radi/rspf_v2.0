#include "rspfGdalTileSource.h"
#include "rspfGdalType.h"
#include "rspfOgcWktTranslator.h"
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTieGptSet.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfPolynomProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfRpcSolver.h>
#include <rspf/projection/rspfRpcProjection.h>
#include <rspf/support_data/rspfFgdcXmlDoc.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <sstream>
RTTI_DEF1(rspfGdalTileSource, "rspfGdalTileSource", rspfImageHandler)
static rspfOgcWktTranslator wktTranslator;
static rspfTrace traceDebug("rspfGdalTileSource:debug");
static const char DRIVER_SHORT_NAME_KW[] = "driver_short_name";
static const char PRESERVE_PALETTE_KW[]  = "preserve_palette";
using namespace rspf;
rspfGdalTileSource::rspfGdalTileSource()
   :
      rspfImageHandler(),
      theDataset(0),
      theTile(0),
      theSingleBandTile(0),
      theImageBound(),
      theMinPixValues(0),
      theMaxPixValues(0),
      theNullPixValues(0),
      theEntryNumberToRender(0),
      theSubDatasets(),
      theIsComplexFlag(false),
      theAlphaChannelFlag(false),
      m_preservePaletteIndexesFlag(false),
      m_outputBandList(0)
{
   getDefaults();
}
rspfGdalTileSource::~rspfGdalTileSource()
{
   close();
}
void rspfGdalTileSource::close()
{
   if(theDataset)
   {
      GDALClose(theDataset);
      theDataset = 0;
   }
   theTile = 0;
   theSingleBandTile = 0;
   if(theMinPixValues)
   {
      delete [] theMinPixValues;
      theMinPixValues = 0;
   }
   if(theMaxPixValues)
   {
      delete [] theMaxPixValues;
      theMaxPixValues = 0;
   }
   if(theNullPixValues)
   {
      delete [] theNullPixValues;
      theNullPixValues = 0;
   }
   m_preservePaletteIndexesFlag = false;
   theAlphaChannelFlag = false;
   theIsComplexFlag = false;
}
bool rspfGdalTileSource::open()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfGdalTileSource::open() DEBUG: entered ..."
         << std::endl;
   }
   if(isOpen())
   {
      close();
   }
   if (theSubDatasets.size() == 0)
   {
      if (theImageFile.size())
      {
         theDataset = GDALOpen(theImageFile.c_str(), GA_ReadOnly); 
         if( theDataset == 0 )
         {
            return false;
         }
      }
      else
      {
         return false;
      }
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfGdalTileSource::open DEBUG:"
            << "\nOpened image:  "
            << theImageFile.c_str()
            << std::endl;
      }
      GDALDriverH driverTmp = GDALGetDatasetDriver(theDataset);
      bool isNtif = false;
      if (driverTmp != 0)
      {
         rspfString driverNameTmp = rspfString(GDALGetDriverShortName(driverTmp));
         if (driverNameTmp == "NITF")
         {
            isNtif = true;
         }
      }
      
      char** papszMetadata = GDALGetMetadata( theDataset, "SUBDATASETS" );
      if( CSLCount(papszMetadata) > 0 )
      {
         theSubDatasets.clear();
         
         for( int i = 0; papszMetadata[i] != 0; ++i )
         {
            rspfString os = papszMetadata[i];
            if (os.contains("_NAME="))
            {
               if (isNtif)
               {
                  GDALDatasetH subDataset = GDALOpen(filterSubDatasetsString(os),
                     GA_ReadOnly);
                  if ( subDataset != 0 )
                  {
                     rspfString nitfIcatTag( GDALGetMetadataItem( subDataset, "NITF_ICAT", "" ) );
                     if ( nitfIcatTag.contains("CLOUD") == false )
                     {
                        theSubDatasets.push_back(filterSubDatasetsString(os));
                     }
                  }
                  GDALClose(subDataset);
               }
               else
               {
                  theSubDatasets.push_back(filterSubDatasetsString(os));
               }
            }
         }
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfGdalTileSource::open DEBUG:" << std::endl;
            for (rspf_uint32 idx = 0; idx < theSubDatasets.size(); ++idx)
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Sub_data_set[" << idx << "] "
                  << theSubDatasets[idx] << std::endl;
            }
         }
         close();
         
         theDataset = GDALOpen(theSubDatasets[theEntryNumberToRender].c_str(),
                               GA_ReadOnly);
         if (theDataset == 0)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN) << "Could not open sub dataset = " << theSubDatasets[theEntryNumberToRender] << "\n";
            }
            return false;
         }
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfGdalTileSource::open DEBUG:"
               << "\nOpened sub data set:  "
               << theSubDatasets[theEntryNumberToRender].c_str()
               << std::endl;
         }
         
      }  // End of has subsets block.
      
   }  // End of "if (theSubdatasets.size() == 0)"
   else
   {
      theDataset = GDALOpen(theSubDatasets[theEntryNumberToRender].c_str(),
                            GA_ReadOnly);
      if (theDataset == 0)
      {
         return false;
      }
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfGdalTileSource::open DEBUG:"
            << "\nOpened sub data set:  "
            << theSubDatasets[theEntryNumberToRender].c_str()
            << std::endl;
      }
   }
   theDriver = GDALGetDatasetDriver( theDataset );
   if(!theDriver) return false;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalTileSource::open Driver: "
         << GDALGetDriverShortName( theDriver )
         << "/" << GDALGetDriverLongName( theDriver ) << std::endl;
   }
   theGdtType = GDT_Byte;
   theOutputGdtType = GDT_Byte;
   if(getNumberOfInputBands() < 1 )
   {
      if(CSLCount(GDALGetMetadata(theDataset, "SUBDATASETS")))
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfGdalTileSource::open WARNING:"
            << "\nHas multiple sub datasets and need to set the data before"
            << " we can get to the bands" << std::endl;
      }
      
      close();
      rspfNotify(rspfNotifyLevel_WARN)
      << "rspfGdalTileSource::open WARNING:"
      << "\nNo band data present in rspfGdalTileSource::open" << std::endl;
      return false;
   }
   
   rspf_int32 i = 0;
   theGdtType  = GDALGetRasterDataType(GDALGetRasterBand( theDataset, 1 ));
   if(!isIndexed(1))
   {
      for(i = 0; i  < GDALGetRasterCount(theDataset); ++i)
      {
         if(theGdtType != GDALGetRasterDataType(GDALGetRasterBand( theDataset,
                                                                   i+1 )))
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfGdalTileSource::open WARNING"
               << "\nWe currently do not support different scalar type bands."
               << std::endl;
            close();
            return false;
         }
      }
   }
   theOutputGdtType = theGdtType;
   switch(theGdtType)
   {
      case GDT_CInt16:
      {
         theIsComplexFlag = true;
         break;
      }
      case GDT_CInt32:
      {
         theIsComplexFlag = true;
         break;
      }
      case GDT_CFloat32:
      {
         theIsComplexFlag = true;
         break;
      }
      case GDT_CFloat64:
      {
         theIsComplexFlag = true;
         break;
      }
      default:
      {
         theIsComplexFlag = false;
         break;
      }
   }
   if((rspfString(GDALGetDriverShortName( theDriver )) == "PNG")&&
      (getNumberOfInputBands() == 4))
   {
      theAlphaChannelFlag = true;
   }
   populateLut();
   computeMinMax();
   completeOpen();
   
   theTile = rspfImageDataFactory::instance()->create(this, this);
   theSingleBandTile = rspfImageDataFactory::instance()->create(this, getInputScalarType(), 1);
   if ( m_preservePaletteIndexesFlag )
   {
      theTile->setIndexedFlag(true);
      theSingleBandTile->setIndexedFlag(true);
   }
   theTile->initialize();
   theSingleBandTile->initialize();
   theGdalBuffer.resize(0);
   if(theIsComplexFlag)
   {
      theGdalBuffer.resize(theSingleBandTile->getSizePerBandInBytes()*2);
   }
   
   theImageBound = rspfIrect(0
                              ,0
                              ,GDALGetRasterXSize(theDataset)-1
                              ,GDALGetRasterYSize(theDataset)-1);
   
   if(traceDebug())
   {
      
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalTileSoruce::open\n"
         << "data type = " << theTile->getScalarType() << std::endl;
      
      for(rspf_uint32 i = 0; i < getNumberOfInputBands(); ++i)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "min pix   " << i << ":" << getMinPixelValue(i)
            << "\nmax pix   " << i << ":" << getMaxPixelValue(i)
            << "\nnull pix  " << i << ":" << getNullPixelValue(i) << std::endl;
      }
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "theTile:\n" << *theTile
         << "theSingleBandTile:\n" << *theSingleBandTile
         << std::endl;
   }
   
   return true;
}
rspfRefPtr<rspfImageData> rspfGdalTileSource::getTile(const  rspfIrect& tileRect,
                                                         rspf_uint32 resLevel)
{
   if (!isSourceEnabled() || !theDataset)
   {
      return rspfRefPtr<rspfImageData>();
   }
   rspfIrect imageBound = getBoundingRect(resLevel);
   if(!tileRect.intersects(imageBound))
   {
      theTile->setImageRectangle(tileRect);
      theTile->makeBlank();
      return theTile;
   }
   if(resLevel)
   {
      if ( m_preservePaletteIndexesFlag )
      {
         throw rspfException( std::string("rspfGdalTileSource::getTile ERROR: Accessing reduced resolution level with the preservePaletteIndexesFlag set!") );
      }
         
      if(theOverview.valid() && theOverview->isValidRLevel(resLevel))
      {
         rspfRefPtr<rspfImageData> tileData = theOverview->getTile(tileRect, resLevel);
         tileData->setScalarType(getOutputScalarType());
         return tileData;
      }
      
#if 0
      else if(GDALGetRasterCount(theDataset))
      {
         GDALRasterBandH band = GDALGetRasterBand(theDataset, 1);
         if(static_cast<int>(resLevel) > GDALGetOverviewCount(band))
         {
            return rspfRefPtr<rspfImageData>();
         }
      }
#endif
   }
   theTile->setImageRectangle(tileRect);
   rspfIrect clipRect   = tileRect.clipToRect(imageBound);
   theSingleBandTile->setImageRectangle(clipRect);
   if (tileRect.completely_within(clipRect) == false)
   {
      theTile->makeBlank();
   }
   theSingleBandTile->makeBlank();
   
   rspf_uint32 anOssimBandIndex = 0;
   rspf_uint32 aGdalBandIndex   = 1;
   rspf_uint32 rasterCount = GDALGetRasterCount(theDataset);
   if (m_outputBandList.size() > 0)
   {
      rasterCount = m_outputBandList.size();
   }
   
   rspf_uint32 outputBandIndex = 0;
   for(rspf_uint32 aBandIndex =1; aBandIndex <= rasterCount; ++aBandIndex)
   {
      if (m_outputBandList.size() > 0)
      {
         aGdalBandIndex = m_outputBandList[outputBandIndex] + 1;
         outputBandIndex++;
      }
      else
      {
         aGdalBandIndex = aBandIndex;
      }
      GDALRasterBandH aBand = resolveRasterBand( resLevel, aGdalBandIndex );
      if ( aBand )
      {
         bool bReadSuccess;
         if(!theIsComplexFlag)
         {
            bReadSuccess = (GDALRasterIO(aBand
                                         , GF_Read
                                         , clipRect.ul().x
                                         , clipRect.ul().y
                                         , clipRect.width()
                                         , clipRect.height()
                                         , theSingleBandTile->getBuf()
                                         , clipRect.width()
                                         , clipRect.height()
                                         , theOutputGdtType
                                         , 0 
                                         , 0 ) == CE_None) ? true : false;
            if ( bReadSuccess == true )
            {
               if(isIndexed(aGdalBandIndex))
               {
                  if(isIndexTo3Band(aGdalBandIndex))
                  {
                     if ( m_preservePaletteIndexesFlag )
                     {
                        theTile->loadBand((void*)theSingleBandTile->getBuf(),
                                          clipRect, anOssimBandIndex);
                        anOssimBandIndex += 1;
                     }
                     else
                     {
                        loadIndexTo3BandTile(clipRect, aGdalBandIndex, anOssimBandIndex);
                        anOssimBandIndex+=3;
                     }
                  }
                  else if(isIndexTo1Band(aGdalBandIndex))
                  { 
                     anOssimBandIndex += 1;
                  }
               }
               else
               {
                  if(theAlphaChannelFlag&&(aGdalBandIndex==rasterCount))
                  {
                     theTile->nullTileAlpha((rspf_uint8*)theSingleBandTile->getBuf(),
                                            theSingleBandTile->getImageRectangle(),
                                            clipRect,
                                            false);
                  }
                  else
                  {
                     theTile->loadBand((void*)theSingleBandTile->getBuf()
                                       , clipRect
                                       , anOssimBandIndex);
                     ++anOssimBandIndex;
                  }
               }
            }
            else
            {
               ++anOssimBandIndex;
            }
         } 
         else // matches if(!theIsComplexFlag){} 
         {
            bReadSuccess = (GDALRasterIO(aBand
                                         , GF_Read
                                         , clipRect.ul().x
                                         , clipRect.ul().y
                                         , clipRect.width()
                                         , clipRect.height()
                                         , &theGdalBuffer.front()
                                         , clipRect.width()
                                         , clipRect.height()
                                         , theOutputGdtType
                                         , 0
                                         , 0 ) == CE_None) ? true : false;
            if (  bReadSuccess == true )
            {
               rspf_uint32 byteSize = rspf::scalarSizeInBytes(theSingleBandTile->getScalarType());
               rspf_uint32 byteSize2 = byteSize*2;
               rspf_uint8* complexBufPtr = (rspf_uint8*)(&theGdalBuffer.front()); // start at first real part
               rspf_uint8* outBufPtr  = (rspf_uint8*)(theSingleBandTile->getBuf());
               rspf_uint32 idxMax = theSingleBandTile->getWidth()*theSingleBandTile->getHeight();
               rspf_uint32 idx = 0;
               for(idx = 0; idx < idxMax; ++idx)
               {
                  memcpy(outBufPtr, complexBufPtr, byteSize);
                  complexBufPtr += byteSize2;
                  outBufPtr     += byteSize;
               }
               theTile->loadBand((void*)theSingleBandTile->getBuf()
                                 , clipRect
                                 , anOssimBandIndex);
               ++anOssimBandIndex;
               complexBufPtr = (rspf_uint8*)(&theGdalBuffer.front()) + byteSize; // start at first imaginary part
               outBufPtr  = (rspf_uint8*)(theSingleBandTile->getBuf());
               for(idx = 0; idx < idxMax; ++idx)
               {
                  memcpy(outBufPtr, complexBufPtr, byteSize);
                  complexBufPtr += byteSize2;
                  outBufPtr     += byteSize;
               }
               theTile->loadBand((void*)theSingleBandTile->getBuf()
                                 , clipRect
                                 , anOssimBandIndex);
               ++anOssimBandIndex;
            }
            else
            {
               anOssimBandIndex += 2;
            }
         }
      }
   }
   theTile->validate();
   return theTile;
}
rspf_uint32 rspfGdalTileSource::getNumberOfInputBands() const
{
   if(isOpen())
   {
       if (m_outputBandList.size() > 0)
       {
          return m_outputBandList.size();
       }
      if(isIndexed(1))
      {
         return getIndexBandOutputNumber(1);
      }
      if(!theIsComplexFlag)
      {
         return GDALGetRasterCount(theDataset);
      }
      return GDALGetRasterCount(theDataset)*2;
   }
   return 0;
}
/*!
 * Returns the number of bands in a tile returned from this TileSource.
 * Note: we are supporting sources that can have multiple data objects.
 * If you want to know the scalar type of an object you can pass in the 
 */
rspf_uint32 rspfGdalTileSource::getNumberOfOutputBands() const
{
   rspf_uint32 result = 0;
   if( isIndexTo3Band() )
   {
      if ( m_preservePaletteIndexesFlag )
      {
         result = 1; // Passing palette indexes not expanded rgb values.
      }
      else
      {
         result = 3;
      }
   }
   else if (theAlphaChannelFlag)
   {
      result = 3;
   }
   else
   {
      result = getNumberOfInputBands();
   }
   return result;
}
rspf_uint32 rspfGdalTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   rspf_uint32 result = 0;
   if ( isOpen() && isValidRLevel(reduced_res_level) )
   {
      if(theOverview.valid() && theOverview->isValidRLevel(reduced_res_level))
      {
         result = theOverview->getNumberOfLines(reduced_res_level);
      }
      else
      {
         int x, y;
         getMaxSize(reduced_res_level, x, y);
         result = y;
      }
   }
   return result;
}
rspf_uint32 rspfGdalTileSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   rspf_uint32 result = 0;
   if ( isOpen() && isValidRLevel(reduced_res_level) )
   {
      if(theOverview.valid() && theOverview->isValidRLevel(reduced_res_level))
      {
         result = theOverview->getNumberOfLines(reduced_res_level);
      }
      else
      {
         int x, y;
         getMaxSize(reduced_res_level, x, y);
         result = x;
      }
   }
   return result;
}
rspfIrect rspfGdalTileSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   rspfIrect result;
   result.makeNan();
   
   int x, y;
   getMaxSize(reduced_res_level, x, y);
   if(x&&y)
   {
      return rspfIrect(0,
                        0,
                        x - 1,
                        y - 1);
   }
   if (result.hasNans())
   {
      return rspfImageHandler::getImageRectangle(reduced_res_level);
   }
   
   return result;
}
void rspfGdalTileSource::getDecimationFactor(rspf_uint32 resLevel,
                                              rspfDpt& result) const
{
   if (resLevel == 0)
   {
      result.x = 1.0;
      result.y = 1.0;
   }
   else
   {
      /* 
         ESH 02/2009 -- No longer assume powers of 2 reduction 
         in linear size from resLevel 0 (Tickets # 467,529).
      */
      rspf_int32 x  = getNumberOfLines(resLevel);
      rspf_int32 x0 = getNumberOfLines(0);
      if ( x > 0 && x0 > 0 )
      {
         result.x = ((double)x) / x0;
      }
      else
      {
         result.x = 1.0 / (1<<resLevel);
      }
      result.y = result.x;
   }
}
bool rspfGdalTileSource::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   kwl.add(prefix, 
	   "entry",
	   theEntryNumberToRender,
	   true);
   if (theEntryNumberToRender < theSubDatasets.size())
   {
      kwl.add(prefix,
              "entry_string",
              theSubDatasets[theEntryNumberToRender].c_str(),
              true);
   }
   
   bool result = rspfImageHandler::saveState(kwl, prefix);
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG:"
         << "\nrspfGdalTileSource::saveState keywordlist:\n"
         << kwl
         << std::endl;
   }
   return result;
}
bool rspfGdalTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   if (rspfImageHandler::loadState(kwl, prefix))
   {
      const char* lookup = kwl.find(prefix, "entry");
      if(lookup)
      {
         rspf_uint32 entry = rspfString(lookup).toUInt32();
         setCurrentEntry(entry);
         return isOpen();
      }
      lookup = kwl.find(prefix, PRESERVE_PALETTE_KW);
      if ( lookup )
      {
         setPreservePaletteIndexesFlag(rspfString(lookup).toBool());
      }
      return open();
   }
   return false;
}
rspfRefPtr<rspfImageGeometry> rspfGdalTileSource::getImageGeometry()
{
   if ( !theGeometry )
   {
      theGeometry = getExternalImageGeometry();
      if ( !theGeometry )
      {
         theGeometry = new rspfImageGeometry();
         if (theImageFile.before(":", 3).upcase() != "SDE") //No extend geo for SDE raster. Example of SDE raster file: SDE:SPATCDT025,5151,SDE,SDE,RAWWAR,SDE.FASTFOOD2
         {
            if(!rspfImageGeometryRegistry::instance()->extendGeometry( this ))
            {
               theGeometry = getInternalImageGeometry();
            }
         }
         else
         {
            theGeometry = getInternalImageGeometry();
         }
         
         if ( !theGeometry )
         {
            theGeometry = getExternalImageGeometryFromXml();
         }
      }
      initImageParameters( theGeometry.get() );
   }
  return theGeometry;
}
rspfRefPtr<rspfImageGeometry> rspfGdalTileSource::getExternalImageGeometryFromXml() const
{
  rspfRefPtr<rspfImageGeometry> geom = 0;
  rspfString fileBase = theImageFile.noExtension();
  rspfFilename xmlFile = rspfString(fileBase + ".xml");
  if (!xmlFile.exists())//try the xml file which includes the entire source file name
  {
    xmlFile = theImageFile + ".xml";
  }
  rspfFgdcXmlDoc* fgdcXmlDoc = new rspfFgdcXmlDoc;
  if ( fgdcXmlDoc->open(xmlFile) )
  {
     rspfRefPtr<rspfProjection> proj = fgdcXmlDoc->getProjection();
     if ( proj.valid() )
     {
        geom = new rspfImageGeometry;
        geom->setProjection( proj.get() );
     }
  }
  delete fgdcXmlDoc;
  fgdcXmlDoc = 0;
  
  return geom;
}
rspfRefPtr<rspfImageGeometry> rspfGdalTileSource::getInternalImageGeometry() const
{
   static const char MODULE[] = "rspfGdalTileSource::getImageGeometry";
   
   if( !isOpen())
   {
      return rspfRefPtr<rspfImageGeometry>();
   }
   
   rspfString driverName = theDriver ? GDALGetDriverShortName( theDriver ) : "";
   
   const char* wkt = GDALGetProjectionRef( theDataset );
   rspf_uint32 gcpCount = GDALGetGCPCount(theDataset);
   if(!rspfString(wkt).empty() && gcpCount < 4)
   {
      if(traceDebug())
      {
         CLOG << "WKT PROJECTION STRING = " << wkt << std::endl;
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "GDALGetGCPCount = " << gcpCount << std::endl;
      }
      
      if(gcpCount > 3)
      {
         rspfRefPtr<rspfProjection> projGcp = 0;
         wkt = GDALGetGCPProjection(theDataset);
         if (!rspfString(wkt).empty())
         {
            rspfKeywordlist kwl;
            
            if (wktTranslator.toOssimKwl(wkt, kwl))
            {
               projGcp = rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
            }
         }
         
         bool isGeo = true;
         if (projGcp.valid())
         {
            rspfRefPtr<rspfMapProjection> mapProj = PTR_CAST(rspfMapProjection, projGcp.get());
            if (mapProj.valid())
            {
               if (!mapProj->isGeographic())
               {
                  isGeo = false;
               }
            }
         }
         rspf_uint32 idx = 0;
         const GDAL_GCP* gcpList = GDALGetGCPs(theDataset);
         rspfTieGptSet tieSet;
         if(!gcpList) return rspfRefPtr<rspfImageGeometry>();
         
         for(idx = 0; idx < gcpCount; ++idx)
         {
            if ((gcpCount < 40) || (idx%10 == 0))
            {
               rspfDpt dpt(gcpList[idx].dfGCPPixel, gcpList[idx].dfGCPLine);
               rspfGpt gpt;
               if (isGeo == false)
               {
                  rspfDpt dptEastingNorthing(gcpList[idx].dfGCPX, gcpList[idx].dfGCPY);
                  gpt = projGcp->inverse(dptEastingNorthing);
                  gpt.hgt = gcpList[idx].dfGCPZ;
               }
               else
               {
                  gpt = rspfGpt(gcpList[idx].dfGCPY, gcpList[idx].dfGCPX, gcpList[idx].dfGCPZ);
               }
               
               tieSet.addTiePoint(new rspfTieGpt(gpt, dpt, .5));
               
               if (traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << "Added tie point for gcp[" << idx << "]: "
                     << dpt << ", " << gpt << std::endl;
               }
            }
         }
         
         rspfRefPtr<rspfBilinearProjection> bilinProj = new rspfBilinearProjection;
         bilinProj->optimizeFit(tieSet);
         
         rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();
         geom->setProjection(bilinProj.get());
         return geom;
      }
   }
   
   double geoTransform[6];
   bool transOk = GDALGetGeoTransform( theDataset, geoTransform ) == CE_None;
   if (!transOk)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << "DEBUG: GDALGetGeoTransform failed..." << std::endl;
      }
      return rspfRefPtr<rspfImageGeometry>();
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG: Geo transform = <" 
         << geoTransform[0] << ", " << geoTransform[1] << ", " << geoTransform[2] << ", "
         << geoTransform[3] << ", " << geoTransform[2] << ", " << geoTransform[5] << ">" 
         << std::endl;
   }
  
   rspfKeywordlist kwl;
   if ( !wktTranslator.toOssimKwl(wkt, kwl) )
   {
      if(transOk&&(driverName=="AAIGrid"))
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG:TESTING GEOGRAPHIC = <" 
               << geoTransform[0] << ", " << geoTransform[1] << ", " << geoTransform[2] << ", "
               << geoTransform[3] << ", " << geoTransform[2] << ", " << geoTransform[5] << ">" 
               << std::endl;
         }
         if((std::fabs(geoTransform[0]) < (180.0+FLT_EPSILON))&&
            (std::fabs(geoTransform[3]) < (90.0 + FLT_EPSILON)))
         {
            rspfTieGptSet tieSet;
            rspfIrect rect = getBoundingRect();
            if(rect.hasNans()) return rspfRefPtr<rspfImageGeometry>();
            rspfDpt ul = rect.ul();
            tieSet.addTiePoint(new rspfTieGpt(rspfGpt(geoTransform[3] + ul.x*geoTransform[4] + ul.y*geoTransform[5],
               geoTransform[0] + ul.x*geoTransform[1] + ul.y*geoTransform[2]), 
               ul, .5));
            rspfDpt ur = rect.ur();
            tieSet.addTiePoint(new rspfTieGpt(rspfGpt(geoTransform[3] + ur.x*geoTransform[4] + ur.y*geoTransform[5],
               geoTransform[0] + ur.x*geoTransform[1] + ur.y*geoTransform[2]), 
               ur, .5));
            rspfDpt lr = rect.lr();
            tieSet.addTiePoint(new rspfTieGpt(rspfGpt(geoTransform[3] + lr.x*geoTransform[4] + lr.y*geoTransform[5],
               geoTransform[0] + lr.x*geoTransform[1] + lr.y*geoTransform[2]), 
               lr, .5));
            rspfDpt ll = rect.ll();
            tieSet.addTiePoint(new rspfTieGpt(rspfGpt(geoTransform[3] + ll.x*geoTransform[4] + ll.y*geoTransform[5],
               geoTransform[0] + ll.x*geoTransform[1] + ll.y*geoTransform[2]), 
               ll, .5));
            rspfRefPtr<rspfBilinearProjection> bilinProj = new rspfBilinearProjection;
            bilinProj->optimizeFit(tieSet);
            rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();
            geom->setProjection(bilinProj.get());
            return geom;
         }
         else
         {
            return rspfRefPtr<rspfImageGeometry>();
         }
      }
      else
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << MODULE << "DEBUG:  wktTranslator.toOssimKwl failed..." << std::endl;
         }
         return rspfRefPtr<rspfImageGeometry>();
      }
   }
   const char* prefix = 0; // legacy code. KWL is used internally now so don't need prefix
   rspfString proj_type  = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   rspfString datum_type = kwl.find(prefix, rspfKeywordNames::DATUM_KW);
   rspfString units      = kwl.find(prefix, rspfKeywordNames::UNITS_KW);
   rspfDpt gsd(fabs(geoTransform[1]), fabs(geoTransform[5]));
   rspfUnitType savedUnitType = static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->getEntryNumber(units)); 
   rspfUnitType unitType = savedUnitType;
   if(unitType == RSPF_UNIT_UNKNOWN)
   {
      unitType = RSPF_METERS;
      units = "meters";
   }
   
   rspfDpt tie(geoTransform[0], geoTransform[3]);
   if(driverName == "AIG")
   {
      tie.x += fabs(gsd.x) / 2.0;
      tie.y -= fabs(gsd.y) / 2.0;
   }
   if(proj_type == "rspfLlxyProjection" ||
      proj_type == "rspfEquDistCylProjection")
   {
      if(savedUnitType == RSPF_UNIT_UNKNOWN)
      {
         unitType = RSPF_DEGREES;
         units = "degrees";
      }
   }
   kwl.add(prefix,
           rspfKeywordNames::PIXEL_SCALE_XY_KW,
           gsd.toString(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::PIXEL_SCALE_UNITS_KW,
           units,
           true);
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_XY_KW,
           tie.toString(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_UNITS_KW,
           units,
           true);
   std::stringstream mString;
   mString << rspfString::toString(geoTransform[1], 20) << " " << rspfString::toString(geoTransform[2], 20)
           << " " << 0 << " " << rspfString::toString(geoTransform[0], 20)
           << " " << rspfString::toString(geoTransform[4], 20) << " " << rspfString::toString(geoTransform[5], 20)
           << " " << 0 << " " << rspfString::toString(geoTransform[3], 20)
           << " " << 0 << " " << 0 << " " << 1 << " " << 0
           << " " << 0 << " " << 0 << " " << 0 << " " << 1;
   
   kwl.add(prefix, rspfKeywordNames::IMAGE_MODEL_TRANSFORM_MATRIX_KW, mString.str().c_str(), true);
   kwl.add(prefix, rspfKeywordNames::IMAGE_MODEL_TRANSFORM_UNIT_KW, units, true);
   
   if(driverName == "AIG" && datum_type.contains("OGB"))
   {
      rspfFilename prj_file = theImageFile.path() + "/prj.adf";
      
      if(prj_file.exists())
      {
         rspfKeywordlist prj_kwl(' ');
         prj_kwl.addFile(prj_file);
         
         rspfString proj = prj_kwl.find("Projection");
         
         if(proj.upcase().contains("GREATBRITAIN"))
         {
            kwl.add(prefix, rspfKeywordNames::TYPE_KW,
               "rspfBngProjection", true);
            rspfString datum  = prj_kwl.find("Datum");
            if(datum != "")
            {
               if(datum == "OGB_A")
                  datum = "OGB-A";
               else if(datum == "OGB_B")
                  datum = "OGB-B";
               else if(datum == "OGB_C")
                  datum = "OGB-C";
               else if(datum == "OGB_D")
                  datum = "OGB-D";
               else if(datum == "OGB_M")
                  datum = "OGB-M";
               else if(datum == "OGB_7")
                  datum = "OGB-7";
               kwl.add(prefix, rspfKeywordNames::DATUM_KW, 
                       datum, true);
            }
         }
      }
      else
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "MODULE DEBUG:"
               << "\nUnable to accurately support ArcGrid due to missing"
               << " prj.adf file." << std::endl;
         }
         return rspfRefPtr<rspfImageGeometry>();
      }
      
   } // End of "if(driverName == "AIG" && datum_type == "OSGB_1936")"
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE <<  " DEBUG:"
         << "\nwktTranslator keyword list:\n"
         << kwl
         << std::endl;
   }
   rspfProjection* proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
   rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();
   geom->setProjection(proj); // assumes ownership via ossiRefPtr mechanism
   return geom;
}
rspf_uint32 rspfGdalTileSource::getTileWidth() const
{
   return ( theTile.valid() ? theTile->getWidth() : 0 );
}
rspf_uint32 rspfGdalTileSource::getTileHeight() const
{
   return ( theTile.valid() ? theTile->getHeight() : 0 );
}
rspfScalarType rspfGdalTileSource::getOutputScalarType() const
{   
   rspfScalarType result = getInputScalarType();
   if ( theLut.valid() && !m_preservePaletteIndexesFlag )
   {
      result = RSPF_UINT8;
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalTileSource::getOutputScalarType debug:"
         << "\nOutput scalar: " << result << std::endl;
   }
   return result;
}
rspfScalarType rspfGdalTileSource::getInputScalarType() const
{   
   rspfScalarType result = RSPF_SCALAR_UNKNOWN;
   switch(theGdtType)
   {
      case GDT_Byte:
      {
         result = RSPF_UINT8;
         break;
      }
      case GDT_UInt16:
      {
         result = RSPF_USHORT16;
         break;
      }
      case GDT_Int16:
      {
         result = RSPF_SSHORT16;
         break;
      }
      case GDT_UInt32:
      {
         result = RSPF_UINT32;
         break;
         
      }
      case GDT_Int32:
      {
         rspf_int32 sizeType = GDALGetDataTypeSize(theGdtType)/8;
         if(sizeType == 2)
         {
            result = RSPF_SSHORT16;
            theGdtType = GDT_Int16;
         }
         else
         {
            result = RSPF_SINT32;
            theGdtType = GDT_Int32;
         }
         break;
      }
      case GDT_Float32:
      {
         result = RSPF_FLOAT;
         break;
      }
      case GDT_Float64:
      {
         result = RSPF_DOUBLE;
         break;
      }
      case GDT_CInt16:
      {
         result = RSPF_SINT16;
         break;
      }
      case  GDT_CInt32:
      {
         result = RSPF_SINT32;
         break;
      }
      case GDT_CFloat32:
      {
         result = RSPF_FLOAT32;
         break;
      }
      case GDT_CFloat64:
      {
         result = RSPF_FLOAT64;
         break;
      }
      default:
         break;
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalTileSource::getInputScalarType debug:"
         << "\nGDAL Type:    " << theGdtType
         << "\nInput scalar: " << result << std::endl;
   }
   return result;
}
double rspfGdalTileSource::getNullPixelValue(rspf_uint32 band)const
{
   double result = rspf::defaultNull(getOutputScalarType());
   if ( theLut.valid() )
   {
      rspf_int32 np = theLut->getNullPixelIndex();
      if ( np != -1 ) // -1 means the lut does not know a null value.
      {
         result = theLut->getNullPixelIndex();
      }
   }
   else if(theMetaData.getNumberOfBands())
   {
      result = rspfImageHandler::getNullPixelValue(band);
   }
   else if (theNullPixValues && (band < getNumberOfOutputBands()) )
   {
      result = theNullPixValues[band];
   }
   return result;
}
double rspfGdalTileSource::getMinPixelValue(rspf_uint32 band)const
{
   double result = rspf::defaultMin(getOutputScalarType());
   if (  theLut.valid() )
   {
      result = 0;
   }
   else if(theMetaData.getNumberOfBands())
   {
      result = rspfImageHandler::getMinPixelValue(band);
   }
   else if (theMinPixValues && (band < getNumberOfOutputBands()) )
   {
      result = theMinPixValues[band];
   }
   return result;
}
double rspfGdalTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   double result = rspf::defaultMax(getOutputScalarType());
   if ( m_preservePaletteIndexesFlag && theLut.valid() && theLut->getNumberOfEntries() )
   {
      result = theLut->getNumberOfEntries() - 1;
   }
   else if(theMetaData.getNumberOfBands())
   {
      result = rspfImageHandler::getMaxPixelValue(band);
   }
   else if ( theMaxPixValues && (band < getNumberOfOutputBands()) )
   {
      result = theMaxPixValues[band];
   }
   return result;
}
void rspfGdalTileSource::computeMinMax()
{
   rspf_uint32 bands = GDALGetRasterCount(theDataset);
   if(theMinPixValues)
   {
      delete [] theMinPixValues;
      theMinPixValues = 0;
   }
   if(theMaxPixValues)
   {
      delete [] theMaxPixValues;
      theMaxPixValues = 0;
   }
   if(theNullPixValues)
   {
      delete [] theNullPixValues;
      theNullPixValues = 0;
   }
   if(isIndexTo3Band())
   {
      int i = 0;
      theMinPixValues  = new double[3];
      theMaxPixValues  = new double[3];
      theNullPixValues = new double[3];
      for(i = 0; i < 3; ++i)
      {
         theMinPixValues[i] = 1;
         theMaxPixValues[i] = 255;
         theNullPixValues[i] = 0;
      }
   }
   else if(isIndexTo1Band())
   {
      theMinPixValues  = new double[1];
      theMaxPixValues  = new double[1];
      theNullPixValues = new double[1];
      
      *theNullPixValues = 0;
      *theMaxPixValues = 255;
      *theMinPixValues = 1;
   }
   else
   {
      if(!theMinPixValues && !theMaxPixValues&&bands)
      {
         theMinPixValues = new double[bands];
         theMaxPixValues = new double[bands];
         theNullPixValues = new double[bands];
      }
      for(rspf_int32 band = 0; band < (rspf_int32)bands; ++band)
      {
         GDALRasterBandH aBand=0;
         
         aBand = GDALGetRasterBand(theDataset, band+1);
         
         int minOk=1;
         int maxOk=1;
         int nullOk=1;
         
         if(aBand)
         {
	   if(hasMetaData())
           {
              theMinPixValues[band] = theMetaData.getMinPix(band);
              theMaxPixValues[band] = theMetaData.getMaxPix(band);
              theNullPixValues[band] = theMetaData.getNullPix(band);
           }
	   else 
           {
              rspfString driverName = theDriver ? GDALGetDriverShortName( theDriver ) : "";
              if ( driverName.contains("JP2KAK")   ||
                   driverName.contains("JPEG2000") ||
                   driverName.contains("NITF") )
              {
                 theMinPixValues[band] = rspf::defaultMin(getOutputScalarType());
                 theMaxPixValues[band] = rspf::defaultMax(getOutputScalarType());
                 theNullPixValues[band] = rspf::defaultNull(getOutputScalarType());
              }
              else
              {
                 theMinPixValues[band]  = GDALGetRasterMinimum(aBand, &minOk);
                 theMaxPixValues[band]  = GDALGetRasterMaximum(aBand, &maxOk);
                 theNullPixValues[band] = GDALGetRasterNoDataValue(aBand, &nullOk);
              }
              if((!nullOk)||(theNullPixValues[band] < rspf::defaultNull(getOutputScalarType())))
              {
                 theNullPixValues[band] = rspf::defaultNull(getOutputScalarType());
              }
           }
           if(!minOk||!maxOk)
           {
               
               theMinPixValues[band] = rspf::defaultMin(getOutputScalarType());
               theMaxPixValues[band] = rspf::defaultMax(getOutputScalarType());
               
            }
         }
         else
         {
            theMinPixValues[band] = rspf::defaultMin(getOutputScalarType());
            theMaxPixValues[band] = rspf::defaultMax(getOutputScalarType());
         }
      }
   }
}
rspf_uint32 rspfGdalTileSource::getNumberOfDecimationLevels() const
{
   rspf_uint32 result = 1; // Start with one.
   if (theOverview.valid())
   {
      result += theOverview->getNumberOfDecimationLevels();
   }
   return result;
#if 0
   if(theDataset&&!theOverview)
   {
      if(GDALGetRasterCount(theDataset))
      {
         GDALRasterBandH band = GDALGetRasterBand(theDataset, 1);
         if(GDALGetOverviewCount(band))
         {
            return (GDALGetOverviewCount(band) + 1);
         }
      }
   }
#endif
}
void rspfGdalTileSource::loadIndexTo3BandTile(const rspfIrect& clipRect,
                                               rspf_uint32 aGdalBandStart,
                                               rspf_uint32 anOssimBandStart)
{
   rspfScalarType inScalar  = getInputScalarType();
   rspfScalarType outScalar = getOutputScalarType();
   if ( ( inScalar == RSPF_UINT8 ) && ( outScalar == RSPF_UINT8 ) )
   {
      loadIndexTo3BandTileTemplate(rspf_uint8(0), // input type
                                   rspf_uint8(0), // output type
                                   clipRect,
                                   aGdalBandStart,
                                   anOssimBandStart);
   }
   else if ( ( inScalar == RSPF_UINT16 ) && ( outScalar == RSPF_UINT8 ) )
   {
      loadIndexTo3BandTileTemplate(rspf_uint16(0), // input type
                                   rspf_uint8(0),  // output type
                                   clipRect,
                                   aGdalBandStart,
                                   anOssimBandStart);
   }
   else if ( ( inScalar == RSPF_UINT16 ) && ( outScalar == RSPF_UINT16 ) )
   {
      loadIndexTo3BandTileTemplate(rspf_uint16(0), // input type
                                   rspf_uint16(0),  // output type
                                   clipRect,
                                   aGdalBandStart,
                                   anOssimBandStart);
   }
   else if ( ( inScalar == RSPF_SINT16 ) && ( outScalar == RSPF_SINT16 ) )
   {
      loadIndexTo3BandTileTemplate(rspf_sint16(0), // input type
                                   rspf_sint16(0),  // output type
                                   clipRect,
                                   aGdalBandStart,
                                   anOssimBandStart);
   }
   else if ( ( inScalar == RSPF_FLOAT32 ) && ( outScalar == RSPF_FLOAT32 ) )
   {
      loadIndexTo3BandTileTemplate(rspf_float32(0.0), // input type
                                   rspf_float32(0.0),  // output type
                                   clipRect,
                                   aGdalBandStart,
                                   anOssimBandStart);
   }
   else if ( ( inScalar == RSPF_FLOAT64 ) && ( outScalar == RSPF_FLOAT64 ) )
   {
      loadIndexTo3BandTileTemplate(rspf_float64(0.0), // input type
                                   rspf_float64(0.0),  // output type
                                   clipRect,
                                   aGdalBandStart,
                                   anOssimBandStart);
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << __FILE__ << __LINE__
         << " rspfGdalTileSource::loadIndexTo3BandTile WARNING!\n"
         << "Unsupported scalar types:\nInupt scalar: " << inScalar
         << "\nOutput scalar: " << outScalar
         << std::endl;
   }
}
template<class InputType, class OutputType>
void rspfGdalTileSource::loadIndexTo3BandTileTemplate(InputType /* in */,
                                                       OutputType /* out */,
                                                       const rspfIrect& clipRect,
                                                       rspf_uint32 aGdalBandStart,
                                                       rspf_uint32 anOssimBandStart)
{
   const InputType* s = reinterpret_cast<const InputType*>(theSingleBandTile->getBuf());
   GDALRasterBandH aBand=0;
   aBand = GDALGetRasterBand(theDataset, aGdalBandStart);
   GDALColorTableH table = GDALGetRasterColorTable(aBand);
   
   rspf_uint32 s_width = theSingleBandTile->getWidth();
   rspf_uint32 d_width = theTile->getWidth();
   rspfIrect src_rect  = theSingleBandTile->getImageRectangle();
   rspfIrect img_rect  = theTile->getImageRectangle();
   
   s += (clipRect.ul().y - src_rect.ul().y) * s_width +
   clipRect.ul().x - src_rect.ul().x;
   rspf_uint32 clipHeight = clipRect.height();
   rspf_uint32 clipWidth  = clipRect.width();
   OutputType* d[3];
   d[0]= static_cast<OutputType*>(theTile->getBuf(anOssimBandStart));
   d[1]= static_cast<OutputType*>(theTile->getBuf(anOssimBandStart + 1));
   d[2]= static_cast<OutputType*>(theTile->getBuf(anOssimBandStart + 2));
   
   OutputType np[3];
   np[0] = (OutputType)theTile->getNullPix(0);
   np[1] = (OutputType)theTile->getNullPix(1);
   np[2] = (OutputType)theTile->getNullPix(2);
   
   OutputType minp[3];
   minp[0] = (OutputType)theTile->getMinPix(0);
   minp[1] = (OutputType)theTile->getMinPix(1);
   minp[2] = (OutputType)theTile->getMinPix(2);
   
   rspf_uint32 offset = (clipRect.ul().y - img_rect.ul().y) * d_width +
   clipRect.ul().x  - img_rect.ul().x;
   d[0] += offset;
   d[1] += offset;
   d[2] += offset;
   
   GDALPaletteInterp interp = GDALGetPaletteInterpretation(table);
   for (rspf_uint32 line = 0; line < clipHeight; ++line)
   {
      for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
      {
         GDALColorEntry entry;
         if(GDALGetColorEntryAsRGB(table, s[sample], &entry))
         {
            if(interp == GPI_RGB)
            {
               if(!entry.c4)
               {
                  d[0][sample] = 0;
                  d[1][sample] = 0;
                  d[2][sample] = 0;
               }
               else
               {
#if 0 /* Code shut off to treat all indexes as valid. */
                  d[0][sample] = entry.c1==np[0]?minp[0]:entry.c1;
                  d[1][sample] = entry.c2==np[1]?minp[1]:entry.c2;
                  d[2][sample] = entry.c3==np[2]?minp[2]:entry.c3;
#endif
                  d[0][sample] = entry.c1;
                  d[1][sample] = entry.c2;
                  d[2][sample] = entry.c3;
                  
               }
            }
            else
            {
               d[0][sample] = entry.c1;
               d[1][sample] = entry.c2;
               d[2][sample] = entry.c3;
            }
         }
         else
         {
            d[0][sample] = 0;
            d[1][sample] = 0;
            d[2][sample] = 0;
         }
      }
      
      s += s_width;
      d[0] += d_width;
      d[1] += d_width;
      d[2] += d_width;
   }
   
}
bool rspfGdalTileSource::isIndexTo3Band(int bandNumber)const
{
   GDALRasterBandH band = GDALGetRasterBand(theDataset, bandNumber);
   if(GDALGetRasterColorInterpretation(band)==GCI_PaletteIndex)
   {
      GDALColorTableH table = GDALGetRasterColorTable(band);
      GDALPaletteInterp interp = GDALGetPaletteInterpretation(table);
      if( (interp == GPI_RGB) ||
          (interp == GPI_HLS)||
          (interp == GPI_CMYK))
      {
         return true;
      }
   }
   return false;
}
bool rspfGdalTileSource::isIndexTo1Band(int bandNumber)const
{
   GDALRasterBandH band = GDALGetRasterBand(theDataset, bandNumber);
   if(GDALGetRasterColorInterpretation(band)==GCI_PaletteIndex)
   {
      GDALColorTableH table = GDALGetRasterColorTable(band);
      GDALPaletteInterp interp = GDALGetPaletteInterpretation(table);
      if(interp == GPI_Gray)
      {
         return true;
      }
   }
   return false;
}
rspf_uint32 rspfGdalTileSource::getIndexBandOutputNumber(int bandNumber)const
{
   if(isIndexed(bandNumber))
   {
      GDALRasterBandH band = GDALGetRasterBand(theDataset, bandNumber);
      if(GDALGetRasterColorInterpretation(band)==GCI_PaletteIndex)
      {
         GDALColorTableH table = GDALGetRasterColorTable(band);
         GDALPaletteInterp interp = GDALGetPaletteInterpretation(table);
         switch(interp)
         {
            case GPI_CMYK:
            case GPI_RGB:
            case GPI_HLS:
            {
               return 3;
            }
            case GPI_Gray:
            {
               return 1;
            }
         }
      }
   }
   return 0;
}
bool rspfGdalTileSource::isIndexed(int aGdalBandNumber)const
{
   if(aGdalBandNumber <= GDALGetRasterCount(theDataset))
   {
      GDALRasterBandH band = GDALGetRasterBand(theDataset, aGdalBandNumber);
      if(!band) return false;
      if(GDALGetRasterColorInterpretation(band)==GCI_PaletteIndex)
      {
         return true;
      }
   }
   
   return false;
}
rspf_uint32 rspfGdalTileSource::getImageTileWidth() const
{
   return 128;
}
rspf_uint32 rspfGdalTileSource::getImageTileHeight() const
{
   return 128;
}
void rspfGdalTileSource::getMaxSize(rspf_uint32 resLevel,
                                     int& maxX,
                                     int& maxY)const
{
   int aGdalBandIndex = 0;
   maxX = 0;
   maxY = 0;
   
   if(theOverview.valid() && theOverview->isValidRLevel(resLevel))
   {
      rspfIrect rect = theOverview->getBoundingRect(resLevel);
      if(!rect.hasNans())
      {
         maxX = rect.width();
         maxY = rect.height();
      }
      return;
   }
   
   for(aGdalBandIndex=1;
       (int)aGdalBandIndex <= (int)GDALGetRasterCount(theDataset);
       ++aGdalBandIndex)
   {
      GDALRasterBandH aBand = resolveRasterBand(resLevel, aGdalBandIndex);
      if(aBand)
      {
         maxY = rspf::max<int>((int)GDALGetRasterBandYSize(aBand), maxY);
         maxX = rspf::max<int>((int)GDALGetRasterBandXSize(aBand), maxX);
      }
      else
      {
         break;
      }
   }
}
GDALRasterBandH rspfGdalTileSource::resolveRasterBand( rspf_uint32 resLevel,
                                                        int aGdalBandIndex ) const
{
   GDALRasterBandH aBand = GDALGetRasterBand( theDataset, aGdalBandIndex );
   if( resLevel > 0 )
   {
      int overviewIndex = resLevel-1;
      GDALRasterBandH overviewBand = GDALGetOverview( aBand, overviewIndex );
      if ( overviewBand )
      {
         aBand = overviewBand;
      }
   }
   return aBand;
}
rspfString rspfGdalTileSource::getShortName()const
{
   return rspfString("gdal");
}
rspfString rspfGdalTileSource::getLongName()const
{
   return rspfString("gdal reader");
}
rspfString rspfGdalTileSource::className()const
{
   return rspfString("rspfGdalTileSource");
}
bool rspfGdalTileSource::isOpen()const
{
   return (theDataset != 0);
}
rspf_uint32 rspfGdalTileSource::getCurrentEntry()const
{
   return theEntryNumberToRender;
}
bool rspfGdalTileSource::setCurrentEntry(rspf_uint32 entryIdx)
{
   if ( isOpen() && (theEntryNumberToRender == entryIdx) )
   {
      return true; // Nothing to do...
   }
   theDecimationFactors.clear();
   theGeometry = 0;
   theOverview = 0;
   theOverviewFile.clear();
   m_outputBandList.clear();
   theEntryNumberToRender = entryIdx;
   return open();
}
void rspfGdalTileSource::getEntryList(std::vector<rspf_uint32>& entryList) const
{
   entryList.clear();
   if (theSubDatasets.size())
   {
      for (rspf_uint32 i = 0; i < theSubDatasets.size(); ++i)
      {
         entryList.push_back(i);
      }
   }
   else
   {
      entryList.push_back(0);
   }
}
void rspfGdalTileSource:: getEntryStringList(std::vector<rspfString>& entryStringList) const
{
   if (theSubDatasets.size())
   {
      entryStringList = theSubDatasets;
   }
   else
   {
      rspfImageHandler::getEntryStringList(entryStringList);
   }
}
rspfString rspfGdalTileSource::filterSubDatasetsString(const rspfString& subString) const
{
   rspfString s;
   bool atStart = false;
   for (rspf_uint32 pos = 0; pos < subString.size(); ++pos)
   {
      if ( *(subString.begin()+pos) == '=')
      {
         atStart = true; // Start recording after this.
         continue;       // Skip the '='.
      }
      if (atStart)
      {
         s.push_back(*(subString.begin()+pos)); // Record the character.
      }
   }
   return s;
}
bool rspfGdalTileSource::isBlocked(int band)const
{
   rspfString driverName = theDriver ? GDALGetDriverShortName( theDriver ) : "";
   int xSize=0, ySize=0;
   GDALGetBlockSize(GDALGetRasterBand( theDataset, band+1 ),
                    &xSize,
                    &ySize);
   return ((xSize > 1)&&
           (ySize > 1));
}
void rspfGdalTileSource::populateLut()
{
   theLut = 0; // rspfRefPtr not a leak.
   
   if(isIndexed(1)&&theDataset)
   {
      GDALColorTableH aTable = GDALGetRasterColorTable(GDALGetRasterBand( theDataset, 1 ));
      GDALPaletteInterp interp = GDALGetPaletteInterpretation(aTable);
      if(aTable && ( (interp == GPI_Gray) || (interp == GPI_RGB)))
      {
         GDALColorEntry colorEntry;
         rspf_uint32 numberOfElements = GDALGetColorEntryCount(aTable);
         rspf_uint32 idx = 0;
         if(numberOfElements)
         {
            theLut = new rspfNBandLutDataObject(numberOfElements,
                                                 4,
                                                 RSPF_UINT8,
                                                 -1);
            bool nullSet = false;
            for(idx = 0; idx < numberOfElements; ++idx)
            {
               switch(interp)
               {
                  case GPI_RGB:
                  {
                     if(GDALGetColorEntryAsRGB(aTable, idx, &colorEntry))
                     {
                        (*theLut)[idx][0] = colorEntry.c1;
                        (*theLut)[idx][1] = colorEntry.c2;
                        (*theLut)[idx][2] = colorEntry.c3;
                        (*theLut)[idx][3] = colorEntry.c4;
                        if ( !nullSet )
                        {
                           if ( m_preservePaletteIndexesFlag )
                           {
                              if ( (*theLut)[idx][3] == 0 )
                              {
                                 theLut->setNullPixelIndex(idx);
                                 nullSet = true;
                              }
                           }
                           else
                           {
                              if ( ( (*theLut)[idx][0] == 0 ) &&
                                   ( (*theLut)[idx][1] == 0 ) &&
                                   ( (*theLut)[idx][2] == 0 ) )
                              {
                                 theLut->setNullPixelIndex(idx);
                                 nullSet = true;
                              }
                           }
                        }
                     }
                     else
                     {
                        (*theLut)[idx][0] = 0;
                        (*theLut)[idx][1] = 0;
                        (*theLut)[idx][2] = 0;
                        (*theLut)[idx][3] = 0;
                        if ( !nullSet )
                        {
                           if ( (*theLut)[idx][0] == 0 )
                           {
                              theLut->setNullPixelIndex(idx);
                           }
                        }
                     }
                     break;
                  }
                  case GPI_Gray:
                  {
                     const GDALColorEntry* constEntry =  GDALGetColorEntry(aTable, idx);
                     if(constEntry)
                     {
                        (*theLut)[idx][0] = constEntry->c1;
                     }
                     else
                     {
                        (*theLut)[idx][0] = 0;
                     }
                     break;
                  }
                  default:
                  {
                     break;
                  }
               }
            }
         }
      }
      rspf_uint32 rasterCount = GDALGetRasterCount(theDataset);
      for(rspf_uint32 aGdalBandIndex=1; aGdalBandIndex <= rasterCount; ++aGdalBandIndex)
      {
         GDALRasterBandH aBand = GDALGetRasterBand( theDataset, aGdalBandIndex );
         if (aBand)
         {
            GDALRasterAttributeTableH hRAT = GDALGetDefaultRAT(aBand);
            int colCount = GDALRATGetColumnCount(hRAT);
            for (rspf_int32 col = 0; col < colCount; col++)
            {
               const char* colName = GDALRATGetNameOfCol(hRAT, col);
               if (colName)
               {
                  if (strcmp(colName, "Class_Names") == 0)
                  {
                     std::vector<rspfString> entryLabels;
                     rspf_int32 rowCount = GDALRATGetRowCount(hRAT);
                     for (rspf_int32 row = 0; row < rowCount; row++)
                     {
                        const char* className = GDALRATGetValueAsString(hRAT, row, col);
                        rspfString entryLabel(className);
                        entryLabels.push_back(entryLabel);
                     }
                     theLut->setEntryLables(aGdalBandIndex-1, entryLabels);
                  }
               }
            }
         }
      }
   }
}
void rspfGdalTileSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if ( property->getName() == PRESERVE_PALETTE_KW )
   {
      rspfString s;
      property->valueToString(s);
      setPreservePaletteIndexesFlag(s.toBool());
   }
   else
   {
      rspfImageHandler::setProperty(property);
   }
}
rspfRefPtr<rspfProperty> rspfGdalTileSource::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;
   
   if( (name == DRIVER_SHORT_NAME_KW) && isOpen() )
   {
      rspfString driverName = GDALGetDriverShortName( theDriver );
      result = new rspfStringProperty(name, driverName);
   }
   else if ( (name == "imag") && isOpen() )
   {
      if (theDataset)
      {
         rspfString nitfImagTag( GDALGetMetadataItem( theDataset, "NITF_IMAG", "" ) );
         if (!nitfImagTag.empty())
         {
            result = new rspfStringProperty(name, nitfImagTag);
         }
      }
   }
   else if ( name == PRESERVE_PALETTE_KW )
   {
      result = new rspfBooleanProperty(name, m_preservePaletteIndexesFlag);
   }
   else
   {
     result = rspfImageHandler::getProperty(name);
   }
   return result;
}
void rspfGdalTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(DRIVER_SHORT_NAME_KW);
   propertyNames.push_back(PRESERVE_PALETTE_KW);
   rspfImageHandler::getPropertyNames(propertyNames);
}
bool rspfGdalTileSource::setOutputBandList(const vector<rspf_uint32>& outputBandList)
{
   m_outputBandList.clear();
   if (outputBandList.size())
   {
      rspf_uint32 bandCount = GDALGetRasterCount(theDataset);
      for (rspf_uint32 i = 0; i < outputBandList.size(); i++)
      {
         if (outputBandList[i] > bandCount)//check if it is a valid band
         {
            return false;
         }
      }
      m_outputBandList = outputBandList;  // Assign the new list.
      return true;
   }
   return false;
}
void rspfGdalTileSource::setPreservePaletteIndexesFlag(bool flag)
{
   bool stateChanged = (flag && !m_preservePaletteIndexesFlag);
   m_preservePaletteIndexesFlag = flag; 
   
   if ( isOpen() && stateChanged )
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      theTile->setIndexedFlag(true);
      theTile->initialize();
      theSingleBandTile = rspfImageDataFactory::instance()->create(this, this);
      theSingleBandTile->setIndexedFlag(true);
      theSingleBandTile->initialize();
      if ( m_preservePaletteIndexesFlag && theLut.valid() )
      {
         rspf_int32 nullIndex = theLut->getFirstNullAlphaIndex();
         if ( nullIndex > -1 ) // Returns -1 if not found.
         {
            theLut->setNullPixelIndex(nullIndex);
         }
      }
   }
}
bool rspfGdalTileSource::getPreservePaletteIndexesFlag() const
{
   return m_preservePaletteIndexesFlag;
}
bool rspfGdalTileSource::isIndexedData() const
{
   return m_preservePaletteIndexesFlag;
}
void rspfGdalTileSource::getDefaults()
{
   const char* lookup = rspfPreferences::instance()->findPreference(PRESERVE_PALETTE_KW);
   if (lookup)
   {
      setPreservePaletteIndexesFlag(rspfString(lookup).toBool());
   }
}
