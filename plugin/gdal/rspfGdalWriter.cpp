#include "rspfGdalWriter.h"
#include "rspfOgcWktTranslator.h"
#include "rspfGdalTiledDataset.h"
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfXmlAttribute.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/vpfutil/set.h>
#include <cmath>
#include <iterator>
#include <sstream>
using namespace std;
static int CPL_STDCALL gdalProgressFunc(double percentComplete,
                            const char* msg,
                            void* data)
{
   rspfGdalWriter* writer = (rspfGdalWriter*)data;
   
   rspfProcessProgressEvent event(writer,
                                   percentComplete*100.0,
                                   msg,
                                   false);
   
   writer->fireEvent(event);
   
   return !writer->needsAborting();
}
static rspfTrace traceDebug(rspfString("rspfGdalWriter:debug"));
RTTI_DEF1(rspfGdalWriter, "rspfGdalWriter", rspfImageFileWriter);
static rspfOgcWktTranslator translator;
rspfGdalWriter::rspfGdalWriter()
   :rspfImageFileWriter(),
    theDriverName(""),
    theDriver((GDALDriverH)0),
    theDataset((GDALDatasetH)0),
    theJpeg2000TileSize(),
    theDriverOptionValues(),
    theGdalDriverOptions(0),
    theGdalOverviewType(rspfGdalOverviewType_NONE),
    theColorLutFlag(false),
    theColorLut(0),
    theLutFilename(),
    theNBandToIndexFilter(0)
{ 
}
rspfGdalWriter::~rspfGdalWriter()
{
   deleteGdalDriverOptions();
   close();
   theDataset = 0;
   theDriverName = "";
}
bool rspfGdalWriter::isOpen()const
{
   return (theDriver != 0);
}
bool rspfGdalWriter::open()
{
   theDriverName = convertToDriverName(theOutputImageType);
   theDriver = GDALGetDriverByName(theDriverName.c_str());
   if(theDriver)
   {
      return true;
   }
   return false;
}
void rspfGdalWriter::close()
{
   if(theDataset)
   {
      GDALClose(theDataset);
      theDataset = 0;
   }
}
void  rspfGdalWriter::setOutputImageType(const rspfString& type)
{
   rspfImageFileWriter::setOutputImageType(type);
   theDriverOptionValues.clear();
}
void rspfGdalWriter::setLut(const rspfNBandLutDataObject& lut)
{
  theColorLutFlag = true;
  theColorLut = (rspfNBandLutDataObject*)lut.dup();
}
/*!
 * saves the state of the object.
 */
bool rspfGdalWriter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalWriter::saveState entered ..."
         << "\nprefix:         " << prefix << std::endl;
   }
   rspfString rrdOption = CPLGetConfigOption("HFA_USE_RRD","");
   if(rrdOption != "")
   {
      kwl.add(prefix,
              "HFA_USE_RRD",
              rrdOption,
              true);
   }
   kwl.add(prefix,
           "gdal_overview_type",
           gdalOverviewTypeToString(),
           true);
   kwl.add(prefix, theDriverOptionValues);
   kwl.add(prefix,
     "color_lut_flag",
     (rspf_uint32)theColorLutFlag,
     true);
   if(theColorLutFlag)
   {
     if(theLutFilename != "")
     {
       kwl.add(prefix,
         "lut_filename",
         theLutFilename.c_str(),
         true);
     }
     else
     {
        if ( theColorLut.valid() )
        {
           rspfString newPrefix = rspfString(prefix) + "lut.";
           theColorLut->saveState(kwl, newPrefix.c_str());
        }
     }
   }
   
   return rspfImageFileWriter::saveState(kwl, prefix);
}
/*!
 * Method to the load (recreate) the state of an object from a keyword
 * list.  Return true if ok or false on error.
 */
bool rspfGdalWriter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   rspfImageFileWriter::loadState(kwl, prefix);
   rspfString regExpression =  (rspfString("^") +
                                 rspfString(prefix) +
                                 "property[0-9]+");
   const char* hfa_use_rrd = kwl.find(prefix, "HFA_USE_RRD");
   if(hfa_use_rrd)
   {
      CPLSetConfigOption("HFA_USE_RRD", hfa_use_rrd);
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalWriter::loadState entered ..."
         << "\nprefix:         " << (prefix?prefix:"null")
         << "\nregExpression:  " << regExpression
         << "\nkwl:" << kwl << std::endl;
   }
   const char* overviewType = kwl.find(prefix, "gdal_overview_type");
   if(overviewType)
   {
      theGdalOverviewType = gdalOverviewTypeFromString(rspfString(overviewType));
   }
   rspfString newPrefix = rspfString(prefix) + "lut.";
   const char* colorLutFlag = kwl.find(prefix, "color_lut_flag");
   if(colorLutFlag)
   {
     theColorLutFlag = rspfString(colorLutFlag).toBool();
   }
   else
   {
     theColorLutFlag = false;
   }
   theLutFilename = rspfFilename(kwl.find(prefix, "lut_filename"));
   theLutFilename = rspfFilename(theLutFilename.trim());
   if ( theColorLut.valid() == false ) theColorLut = new rspfNBandLutDataObject();
   if(theLutFilename != "")
   {
     theColorLut->open(theLutFilename);
   }
   else
   {
     theColorLut->loadState(kwl, newPrefix.c_str());
   }
   
   vector<rspfString> keys = kwl.getSubstringKeyList( regExpression );
   theDriverOptionValues.clear();
   
   deleteGdalDriverOptions();
   kwl.extractKeysThatMatch(theDriverOptionValues,
                            regExpression);
   if(prefix)
   {
      theDriverOptionValues.stripPrefixFromAll(prefix);
   }
#if 0
   if(keys.size()>0)
   {
      rspf_uint32 i = 0;
      theGdalDriverOptions = new char*[keys.size()+1];
      for(i = 0 ; i < keys.size(); ++i)
      {
         rspfString name    = kwl.find(keys[i] +".name");
         rspfString value   = kwl.find(keys[i] +".value");
         rspfString combine = name + "=" + value;
         
         theGdalDriverOptions[i] = new char[combine.size() + 1];
         strcpy(theGdalDriverOptions[i], combine.c_str());
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "\nkey:  " << keys[i]
               << "\nname:  " << name
               << "\nvalue: " << value
               << "\ngdal option:  " << combine
               << std::endl;
         }
      }
      theGdalDriverOptions[keys.size()] = 0;
   }
   else
   {
      const char* compressionQuality =
         kwl.find(prefix, rspfKeywordNames::COMPRESSION_QUALITY_KW);
      if(compressionQuality)
      {
         theGdalDriverOptions = new char*[2];
         rspfString combine  = "QUALITY=" + rspfString(compressionQuality);
         theGdalDriverOptions[0] = new char[combine.size() + 1];
         strcpy(theGdalDriverOptions[0], combine.c_str());
         theGdalDriverOptions[1] = 0;
      }
   }
#endif
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalWriter::loadState exiting..." << std::endl;
   }
   return true;
}
void rspfGdalWriter::getSupportedWriters(vector<rspfString>& /* writers */)
{
}
bool rspfGdalWriter::isLutEnabled()const
{
  return (theColorLutFlag);
}
void rspfGdalWriter::checkColorLut()
{
   bool needColorLut = false;
   bool needLoop = true;
   rspfRefPtr<rspfNBandLutDataObject> colorLut = 0;
   rspfConnectableObject::ConnectableObjectList imageInputs = theInputConnection->getInputList();
   if (imageInputs.size() > 0)
   {
      for (rspf_uint32 i = 0; i < imageInputs.size(); i++)
      {
         if (needLoop == false)
         {
            break;
         }
         rspfImageChain* source = PTR_CAST(rspfImageChain, imageInputs[i].get());
         if (source)
         {
            rspfConnectableObject::ConnectableObjectList imageChains = source->getInputList();
            for (rspf_uint32 j = 0; j < imageChains.size(); j++)
            {
               if (needLoop == false)
               {
                  break;
               }
               rspfImageChain* imageChain = PTR_CAST(rspfImageChain, imageChains[j].get());
               if (imageChain)
               {
                  rspfConnectableObject::ConnectableObjectList imageHandlers =
                     imageChain->findAllObjectsOfType(STATIC_TYPE_INFO(rspfImageHandler), false);
                  for (rspf_uint32 h= 0; h < imageHandlers.size(); h++)
                  {
                     rspfImageHandler* handler =
                        PTR_CAST(rspfImageHandler, imageHandlers[h].get());
                     if (handler)
                     {
                        if (handler->hasLut() != 0) //
                        {
                           colorLut = handler->getLut();
                           needColorLut = true;
                        }
                        else //if not all handlers have color luts, ignore the color lut.
                        {
                           needColorLut = false;
                           needLoop = false;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
   if (needColorLut && colorLut != 0)
   {
      setLut(*colorLut.get());
   }
}
bool rspfGdalWriter::writeFile()
{
   bool result = true; // On failure set to false.
   
   const char *MODULE = "rspfGdalWriter::writeFile()";
   if(traceDebug())
   {
      CLOG << "entered..." << std::endl;
   }
      
   if(theDataset)
   {
      close();
   }
   checkColorLut();
   if( isLutEnabled() && !isInputDataIndexed() )
   {
      theNBandToIndexFilter = new rspfNBandToIndexFilter;
      theNBandToIndexFilter->connectMyInputTo(0, theInputConnection->getInput());
      theNBandToIndexFilter->setLut(*theColorLut.get());
      theNBandToIndexFilter->initialize();
      theInputConnection->disconnect();
      theInputConnection->connectMyInputTo(0, theNBandToIndexFilter.get());
      theInputConnection->initialize();
   }
   else
   {
      theNBandToIndexFilter = 0;
   }
   rspfRefPtr<rspfProperty> blockSize =
      getProperty(theDriverName+rspfString("_BLOCKSIZE"));
   if(blockSize.valid())
   {
      rspf_uint32 size = blockSize->valueToString().toUInt32();
      rspfIpt tileSize;
      if(size > 0)
      {
         tileSize = rspfIpt(size, size);
         theInputConnection->setTileSize(tileSize);
      }
      else
      {
         rspfIpt defaultSize = theInputConnection->getTileSize();
         if(defaultSize.y != defaultSize.x)
         {
            defaultSize.y = defaultSize.x;
            theInputConnection->setTileSize(defaultSize);
         }
         blockSize->setValue(rspfString::toString(defaultSize.x));
         setProperty(blockSize);
         theInputConnection->setTileSize(defaultSize);
      }
   }
   allocateGdalDriverOptions();
   if(!theInputConnection->isMaster()) // MPI slave process...
   {
      theInputConnection->slaveProcessTiles();
      if(theNBandToIndexFilter.valid())
      {
        theInputConnection->connectMyInputTo(0, theNBandToIndexFilter->getInput());
        theNBandToIndexFilter = 0;
      }
      result = true;
   }
   else // MPI master process.
   {
      open();
      
      if(isOpen())
      {
         GDALDataType gdalType = getGdalDataType(theInputConnection->
                                                 getOutputScalarType());
         rspf_uint32 bandCount = theInputConnection->getNumberOfOutputBands();
				 
         theDataset = GDALCreate( theDriver ,
                                  theFilename.c_str(),
                                  (int)theAreaOfInterest.width(),
                                  (int)theAreaOfInterest.height(),
                                  (int)bandCount,
                                  gdalType,
                                  theGdalDriverOptions);
         int nRasterBands = GDALGetRasterCount( theDataset );
         if(theDataset && nRasterBands>0)
         {
            writeProjectionInfo(theDataset);
            if (theColorLutFlag)
            {
              writeColorMap(nRasterBands);
            }
            
            theInputConnection->setToStartOfSequence();
            rspfRefPtr<rspfImageData> currentTile =
               theInputConnection->getNextTile();
            rspfRefPtr<rspfImageData> outputTile =
               (rspfImageData*)currentTile->dup();
            outputTile->initialize();
            if (theDriver)
            {
               rspfString driverName = GDALGetDriverShortName(theDriver);
               if ( (driverName != "HFA") && (driverName != "JP2MrSID") && 
                    (driverName != "JP2KAK") && (driverName != "JPEG2000") )
               {
                  for(rspf_uint32 band = 0;
                      band < currentTile->getNumberOfBands();
                      ++band)
                  {
                     GDALRasterBandH aBand =
                        GDALGetRasterBand(theDataset, band+1);
                     if(aBand)
                     {
                        GDALSetRasterNoDataValue(
                           aBand, theInputConnection->getNullPixelValue(band));
                     }
                  }
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_DEBUG)
                        << "GDALSetRasterNoDataValue called for driver: "
                        << driverName << std::endl;
                  }
               }
               else 
               {
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_DEBUG)
                        << "GDALSetRasterNoDataValue not called for driver: "
                        << driverName << std::endl;
                  }
               }
               rspf_uint32 numberOfBands = currentTile->getNumberOfBands();
               for( rspf_uint32 idx = 0; idx < numberOfBands; ++idx )
               {
                  GDALRasterBandH aBand =
                     GDALGetRasterBand(theDataset, idx+1);
                  if(aBand)
                  {
                     GDALSetMetadataItem( aBand, "AREA_OR_POINT", "Area", 0 );
                     double minPix = (double)currentTile->getMinPix( idx );
                     double maxPix = (double)currentTile->getMaxPix( idx );
                     if ( maxPix > 0.0 && minPix < maxPix )
                     {
                        char szValue[128];
                        sprintf( szValue, "%.14g", minPix );
                        GDALSetMetadataItem( aBand, "STATISTICS_MINIMUM", szValue, 0 );
                        sprintf( szValue, "%.14g", maxPix );
                        GDALSetMetadataItem( aBand, "STATISTICS_MAXIMUM", szValue, 0 );
                     }
                  }
               }
            }
   
            rspf_uint32 numberOfTiles =
               (theInputConnection->getNumberOfTiles()*
                theInputConnection->getNumberOfOutputBands());
   
            rspf_uint32 tileNumber = 0;
            while(currentTile.valid()&&(!needsAborting()))
            {
               rspfIrect clipRect =
                  currentTile->getImageRectangle().clipToRect(theAreaOfInterest);
               outputTile->setImageRectangle(clipRect);
               outputTile->loadTile(currentTile.get());
               rspfIpt offset = clipRect.ul() - theAreaOfInterest.ul();
      
               for(rspf_uint32 band = 0;
                   ((band < (currentTile->getNumberOfBands()))&&
                    (!needsAborting()));
                   ++band)
               {
                  GDALRasterBandH aBand=0;
                  aBand = GDALGetRasterBand(theDataset, band+1);
                  if(aBand)
                  {
                     GDALRasterIO( aBand,
                        GF_Write,
                        offset.x,
                        offset.y,
                        clipRect.width(),
                        clipRect.height(),
                        outputTile->getBuf(band),
                        outputTile->getWidth(),
                        outputTile->getHeight(),
                        gdalType,
                        0,
                        0);
                  }
                  ++tileNumber;
                  rspfProcessProgressEvent event(this,
                                                  ((double)tileNumber/
                                                   (double)numberOfTiles)*100.0,
                                                  "",
                                                  false);
                  
                  fireEvent(event);
               }
               currentTile = theInputConnection->getNextTile();
               
            }
            
            if(theDataset)
            {
               close();
               if((theGdalOverviewType != rspfGdalOverviewType_NONE)&&
                  (!needsAborting()))
               {
                  theDataset= GDALOpen( theFilename.c_str(), GA_Update );
                  
                  if( theDataset == 0 )
                  {
                     theDataset = GDALOpen( theFilename.c_str(), GA_ReadOnly );
                  }
                  buildGdalOverviews();
                  close();
               }
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfGdalWriter::writeFile unable to create dataset"
                  << std::endl;
               result = false;
            }
            
         } // End of "if (theDataset)"
         else
         {
            result = writeBlockFile();
         }
         
      } 
      else // open failed...
      {
         result = false;
         if(traceDebug())
         {
            CLOG << "Unable to open file for writing. Exiting ..."
                 << std::endl;
         }
      }
      if (result)
      {
         postProcessOutput();
      }
   } // End of "else // MPI master process."
   if(theNBandToIndexFilter.valid())
   {
     theInputConnection->connectMyInputTo(0, theNBandToIndexFilter->getInput());
     theNBandToIndexFilter = 0;
   }
   return result;
   
} // End of: rspfGdalWriter::writeFile
bool rspfGdalWriter::writeBlockFile()
{
   theInputConnection->setAreaOfInterest(theAreaOfInterest);
   theInputConnection->setToStartOfSequence();
   
   MEMTiledDataset* tiledDataset = new MEMTiledDataset(theInputConnection.get());
   if (theDriver)
   {
      rspfString driverName = GDALGetDriverShortName(theDriver);
      if ( (driverName == "HFA") || (driverName == "JP2MrSID") || 
           (driverName == "JP2KAK") || (driverName == "JPEG2000") )
      {
         tiledDataset->setNoDataValueFlag(false);
      }
   }
   rspfString driverName="null";
   if (theDriver)
   {
      driverName = GDALGetDriverShortName(theDriver);
   }
   writeProjectionInfo(tiledDataset);
   theDataset = GDALCreateCopy( theDriver,
                                theFilename.c_str(),
                                tiledDataset ,
                                true,
                                theGdalDriverOptions,
                                (GDALProgressFunc)&gdalProgressFunc,
                                this);
   
   if(theDataset&&!needsAborting())
   {
      if(theGdalOverviewType != rspfGdalOverviewType_NONE)
      {
         buildGdalOverviews();
      }
      close();
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalWriter::writeBlockFile(): Unable to create dataset for file " << theFilename << std::endl;
      }
      return false;
   }
   if( theDataset && rspfString(GDALGetDriverShortName(theDataset)) == rspfString("PNG") )
   {
      setWriteExternalGeometryFlag(true);
   }
   return true;
}
void rspfGdalWriter::writeColorMap(int bands)
{
   if ( theDataset && theColorLut.valid() )
   {
      bool hasAlpha = (theColorLut->getNumberOfBands() == 4);
      for (int band = 0; band < bands; band++)
      {
         GDALRasterBandH aBand=0;
         aBand = GDALGetRasterBand(theDataset, band+1);
         if(aBand)
         {
            bool hasEntryLabel = false;
            rspf_uint32 entryNum = theColorLut->getNumberOfEntries();
            std::vector<rspfString> entryLabels = theColorLut->getEntryLabels(band);
            if (entryLabels.size() == entryNum)
            {
               hasEntryLabel = true;
            }
            
            GDALColorTable* gdalColorTable = new GDALColorTable();
            for(rspf_uint32 entryIndex = 0; entryIndex < entryNum; entryIndex++)
            {
               GDALColorEntry colorEntry;
               colorEntry.c1 = (*theColorLut)[entryIndex][0];
               colorEntry.c2 = (*theColorLut)[entryIndex][1];
               colorEntry.c3 = (*theColorLut)[entryIndex][2];
               colorEntry.c4 = hasAlpha ? (*theColorLut)[entryIndex][3] : 255;
#if 0
               if (hasEntryLabel)
               {
                  char* labelName = const_cast<char*>(entryLabels[entryIndex].c_str());
                  colorEntry.poLabel = labelName;
               }
               else
               {
                  colorEntry.poLabel = NULL;
               }
#endif
               
               gdalColorTable->SetColorEntry(entryIndex, &colorEntry);
            }
            
            GDALSetRasterColorTable(aBand, gdalColorTable);
            delete gdalColorTable;
         }
      }
   }
}
void rspfGdalWriter::writeProjectionInfo(GDALDatasetH dataset)
{
   rspfRefPtr<rspfImageGeometry> geom = theInputConnection->getImageGeometry();
   if(geom.valid())
   {
      const rspfMapProjection* mapProj =
         PTR_CAST(rspfMapProjection, geom->getProjection());
      if (mapProj)
      {
         rspf_uint32 pcs_code = mapProj->getPcsCode();
         if (pcs_code == 0)
            pcs_code = 32767;
         double geoTransform[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
         rspfDpt tiePoint;
         rspfDpt scale;
         if (mapProj->isGeographic())
         {
            rspfGpt gpt;
            mapProj->lineSampleToWorld(theAreaOfInterest.ul(), gpt);
            tiePoint = gpt;
            scale.x =  mapProj->getDecimalDegreesPerPixel().x;
            scale.y = -mapProj->getDecimalDegreesPerPixel().y;
         }
         else
         {
            mapProj->lineSampleToEastingNorthing(theAreaOfInterest.ul(),
                                                 tiePoint);
            scale.x =  mapProj->getMetersPerPixel().x;
            scale.y = -mapProj->getMetersPerPixel().y;
         }
         
         geoTransform[1] = scale.x;
         geoTransform[5] = scale.y;
         geoTransform[0] = tiePoint.x;
         geoTransform[3] = tiePoint.y;
         geoTransform[0] -= fabs(scale.x) / 2.0;
         geoTransform[3] += fabs(scale.y) / 2.0;
         rspfString wktString = "";
         if( mapProj->getProjectionName() == rspfString("rspfBngProjection") )
         {
            wktString = "PROJCS[\"OSGB 1936 / British National Grid\",GEOGCS[\"OSGB 1936\","
               "DATUM[\"OSGB_1936\",SPHEROID[\"Airy 1830\",6377563.396,299.3249646]],PRIMEM[\""
               "Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],"
               "PARAMETER[\"latitude_of_origin\",49],PARAMETER[\"central_meridian\",-2],PARAMETER[\""
               "scale_factor\",0.999601272],PARAMETER[\"false_easting\",400000],PARAMETER[\"false_"
               "northing\",-100000],UNIT[\"meter\",1]]";
         }
         else if (mapProj->getProjectionName() == rspfString("rspfAlbersProjection") )
         {
            wktString = "PROJCS[\"Albers Conical Equal Area\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\","
               "SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],"
               "TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,"
               "AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],"
               "AUTHORITY[\"EPSG\",\"4269\"]],PROJECTION[\"Albers_Conic_Equal_Area\"],PARAMETER[\"standard_parallel_1\",29.5],"
               "PARAMETER[\"standard_parallel_2\",45.5],PARAMETER[\"latitude_of_center\",23],PARAMETER[\"longitude_of_center\",-96],"
               "PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"meters\",1]]";
         }
         else
         {
            rspfKeywordlist kwl;
            mapProj->saveState(kwl);
            wktString = translator.fromOssimKwl(kwl);
         }
         GDALSetProjection(dataset, wktString.c_str());
         GDALSetGeoTransform(dataset, geoTransform);
      } // matches: if (mapProj)
   } // matches: if(geom.valid())
}
void rspfGdalWriter::setOptions(rspfKeywordlist& /* kwl */,
                                 const char* prefix)
{
   rspfString regExpression = rspfString("^(") +
                               rspfString(prefix) +
                               "option[0-9]+.)";
}
GDALDataType rspfGdalWriter::getGdalDataType(rspfScalarType scalar)
{
   GDALDataType dataType = GDT_Unknown;
   
   switch(scalar)
   {
      case RSPF_UCHAR:
      {
         dataType = GDT_Byte;
         break;
      }
      case RSPF_USHORT11:
      case RSPF_USHORT16:
      {
         dataType = GDT_UInt16;
         break;
      }
      case RSPF_SSHORT16:
      {
         dataType = GDT_Int16;
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         dataType = GDT_Float32;
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         dataType = GDT_Float64;
         break;
      }
      default:
         break;
   }
   return dataType;
}
void rspfGdalWriter::allocateGdalDriverOptions()
{
   deleteGdalDriverOptions();
   rspfString regExpression =  (rspfString("^")+
                                 "property[0-9]+");
   vector<rspfString> keys = theDriverOptionValues.getSubstringKeyList( regExpression );
   if(keys.size())
   {
      rspf_uint32 i = 0;
      theGdalDriverOptions = new char*[keys.size()+1];
      for(i = 0 ; i < keys.size(); ++i)
      {
         rspfString name    = theDriverOptionValues.find(keys[i] +".name");
         rspfString value   = theDriverOptionValues.find(keys[i] +".value");
         if (theInputConnection.valid() && (GDALGetDriverByName("JP2KAK")) && (name == "QUALITY"))
         {
            if (theInputConnection->getOutputScalarType() != RSPF_UINT8)
            {
               double d = value.toDouble();
               if (d >= 99.5)
               {
                  if (traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_DEBUG)
                        << "DEBUG:"
                        << "\nLossless quality not valid for non 8 bit data"
                        << "\nResetting to 99.0"
                        << std::endl;
                  }
                  value = "99.0";
               }
            }
         }
         if ( (GDALGetDriverByName("HFA")) &&
              (name == "FORCETOPESTRING") )
         {
            value = "TRUE";
         }
         rspfString combine = name + "=" + value;
         
         theGdalDriverOptions[i] = new char[combine.size() + 1];
		   strcpy(theGdalDriverOptions[i], combine.c_str());
         
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "\nkey:  " << keys[i]
               << "\nname:  " << name
               << "\nvalue: " << value
               << "\ngdal option:  " << combine
               << std::endl;
         }
      }
      theGdalDriverOptions[keys.size()] = 0;
   }
}
void rspfGdalWriter::appendGdalOption(const rspfString& name,
                                       const rspfString& value,
                                       bool replaceFlag)
{
   rspfString regExpression =  (rspfString("^")+
                                 "property[0-9]+");
   vector<rspfString> keys = theDriverOptionValues.getSubstringKeyList( regExpression );
   if(keys.size())
   {
      rspf_uint32 i = 0;
      for(i = 0 ; i < keys.size(); ++i)
      {
         rspfString tempName    = theDriverOptionValues.find(keys[i] +".name");
         if(tempName == name)
         {
            if(replaceFlag)
            {
               theDriverOptionValues.add(keys[i]+".value",
                                         value,
                                         true);
            }
            
            return;
         }
      }
   }
   rspfString prefix = "property" + rspfString::toString((rspf_uint32)keys.size()) + ".";
   
   theDriverOptionValues.add(prefix,
                             "name",
                             name,
                             true);
   theDriverOptionValues.add(prefix,
                             "value",
                             value,
                             true);
}
void rspfGdalWriter::deleteGdalDriverOptions()
{
   int i = 0;
   if(theGdalDriverOptions)
   {
      char* currentOption = theGdalDriverOptions[i];
      while(currentOption)
      {
         ++i;
         delete [] currentOption;
         currentOption = theGdalDriverOptions[i];
      }
      delete [] theGdalDriverOptions;
      theGdalDriverOptions = 0;
   }
}
rspfString rspfGdalWriter::convertToDriverName(const rspfString& imageTypeName)const
{
   rspfString strippedName = imageTypeName;
   strippedName = strippedName.substitute("gdal_", "");
   if(GDALGetDriverByName(strippedName.c_str()))
   {
      return strippedName;
   }
   if(imageTypeName == "image/jp2")
   {
      if(GDALGetDriverByName("JP2KAK"))
      {
         return "JP2KAK";
      }
      else if(GDALGetDriverByName("JPEG2000"))
      {
         return "JPEG2000";
      }
      else if(GDALGetDriverByName("JP2MrSID"))
      {
         return "JP2MrSID";
      }
   }
   if(imageTypeName == "image/png")
   {
      return "PNG";
   }
   if(imageTypeName == "image/wbmp")
   {
	   return "BMP";
   }
   if(imageTypeName == "image/bmp")
   {
	   return "BMP";
   }
   if(imageTypeName == "image/jpeg")
   {
      return "JPEG";
   }
   if(imageTypeName == "image/gif")
   {
      return "GIF";
   }
   
   if( imageTypeName == "gdal_imagine_hfa")
   {
      return "HFA";
   }
   if(imageTypeName == "gdal_nitf_rgb_band_separate")
   {
      return "NITF";
   }
   if(imageTypeName == "gdal_jpeg2000")
   {
      if(GDALGetDriverByName("JP2KAK"))
      {
         return "JP2KAK";
      }
      else if(GDALGetDriverByName("JPEG2000"))
      {
         return "JPEG2000";
      }
      else if(GDALGetDriverByName("JP2MrSID"))
      {
         return "JP2MrSID";
      }
   }                                       
   if(imageTypeName == "gdal_arc_info_aig")
   {
      return "AIG";
   }
   if(imageTypeName == "gdal_arc_info_gio")
   {
      return "GIO";
   }
   if(imageTypeName == "gdal_arc_info_ascii_grid")
   {
      return "AAIGrid";
   }
   if(imageTypeName == "gdal_mrsid")
   {
      return "MrSID";
   }
   if(imageTypeName == "gdal_jp2mrsid")
   {
      return "JP2MrSID";
   }
   if(imageTypeName == "gdal_png")
   {
      return "PNG";
   }
   if(imageTypeName == "gdal_jpeg")
   {
      return "JPEG";
   }
      
   if(imageTypeName == "gdal_gif")
   {
      return "GIF";
   }
   if(imageTypeName == "gdal_dted")
   {
      return "DTED";
   }
   if(imageTypeName == "gdal_usgsdem")
   {
      return "USGSDEM";
   }
   if(imageTypeName == "gdal_bmp")
   {
      return "BMP";
   }
   if(imageTypeName == "gdal_raw_envi")
   {
      return "ENVI";
   }
   if(imageTypeName == "gdal_raw_esri")
   {
      return "EHdr";
   }
   if(imageTypeName == "gdal_raw_pci")
   {
      return "PAux";
   }
   if(imageTypeName == "gdal_pcidsk")
   {
      return "PCIDSK";
   }
   if(imageTypeName == "gdal_sdts")
   {
      return "SDTS";
   }
   if(imageTypeName == "gdal_jp2ecw")
   {
      return "JP2ECW";
   }
   if(imageTypeName == "gdal_ecw")
   {
      return "ECW";
   }
   return imageTypeName;
}
void rspfGdalWriter::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   int driverCount = GDALGetDriverCount();
   int idx = 0;
   for(idx = 0; idx < driverCount; ++idx)
   {
      GDALDriverH driver =  GDALGetDriver(idx);
      
      if(driver)
      {
         const char* metaData = GDALGetMetadataItem(driver, GDAL_DCAP_CREATE, 0);
         if(metaData)
         {
            imageTypeList.push_back("gdal_" + rspfString(GDALGetDriverShortName(driver)));
         }
         else
         {
            metaData = GDALGetMetadataItem(driver, GDAL_DCAP_CREATECOPY, 0);
            if(metaData)
            {
               imageTypeList.push_back("gdal_" + rspfString(GDALGetDriverShortName(driver)));
            }
         }
      }
   }
}
bool rspfGdalWriter::hasImageType(const rspfString& imageType) const
{
   if((imageType == "gdal_imagine_hfa") ||
      (imageType == "gdal_nitf_rgb_band_separate") ||
      (imageType == "gdal_jpeg2000") ||
      (imageType == "gdal_jp2ecw") ||
      (imageType == "gdal_arc_info_aig") ||
      (imageType == "gdal_arc_info_gio") ||
      (imageType == "gdal_arc_info_ascii_grid") ||
      (imageType == "gdal_mrsid") ||
      (imageType == "image/mrsid") ||
      (imageType == "gdal_jp2mrsid") ||
      (imageType == "image/jp2mrsid") ||
      (imageType == "gdal_png") ||
      (imageType == "image/png") ||
      (imageType == "image/bmp") ||
      (imageType == "image/wbmp") ||
      (imageType == "gdal_jpeg") ||
      (imageType == "image/jpeg") ||
      (imageType == "gdal_gif") ||
      (imageType == "image/gif") ||
      (imageType == "gdal_dted") ||
      (imageType == "image/dted") ||
      (imageType == "gdal_bmp") ||
      (imageType == "gdal_xpm") ||
      (imageType == "image/x-xpixmap") ||
      (imageType == "gdal_raw_envi") ||
      (imageType == "gdal_raw_esri") ||
      (imageType == "gdal_raw_esri") ||
      (imageType == "gdal_pcidsk") ||
      (imageType == "gdal_sdts"))
   {
      return true;
   }
   
   return rspfImageFileWriter::hasImageType(imageType);
}
rspfString rspfGdalWriter::getExtension() const
{
   rspfString imgTypeStr(theOutputImageType);
   imgTypeStr.downcase();
   if(imgTypeStr == "image/jp2")
   {
      return rspfString("jp2");
   }
   if(imgTypeStr == "image/png")
   {
      return rspfString("png");
   }
   if(imgTypeStr == "image/wbmp")
   {
      return rspfString("bmp");
   }
   if(imgTypeStr == "image/bmp")
   {
      return rspfString("bmp");
   }
   if(imgTypeStr == "image/jpeg")
   {
      return rspfString("jpg");
   }
   if(imgTypeStr == "image/gif")
   {
      return rspfString("gif");
   }
   if( imgTypeStr == "gdal_imagine_hfa")
   {
      return rspfString("hfa");
   }
   if(imgTypeStr == "gdal_nitf_rgb_band_separate")
   {
      return rspfString("ntf");
   }
   if(imgTypeStr == "gdal_jpeg2000")
   {
      return rspfString("jp2");
   }                                       
   if(imgTypeStr == "gdal_arc_info_aig")
   {
      return rspfString("aig");
   }
   if(imgTypeStr == "gdal_arc_info_gio")
   {
      return rspfString("gio");
   }
   if(imgTypeStr == "gdal_arc_info_ascii_grid")
   {
      return rspfString("aig");
   }
   if(imgTypeStr == "gdal_mrsid")
   {
      return rspfString("sid");
   }
   if(imgTypeStr == "gdal_jp2mrsid")
   {
      return rspfString("jp2");
   }
   if(imgTypeStr == "gdal_png")
   {
      return rspfString("png");
   }
   if(imgTypeStr == "gdal_jpeg")
   {
      return rspfString("jpg");
   }
   if(imgTypeStr == "gdal_gif")
   {
      return rspfString("gif");
   }
   if(imgTypeStr == "gdal_dted")
   {
      return rspfString("dte");
   }
   if(imgTypeStr == "gdal_usgsdem")
   {
      return rspfString("dem");
   }
   if(imgTypeStr == "gdal_bmp")
   {
      return rspfString("bmp");
   }
   if(imgTypeStr == "gdal_raw_envi")
   {
      return rspfString("raw");
   }
   if(imgTypeStr == "gdal_raw_esri")
   {
      return rspfString("raw");
   }
   if(imgTypeStr == "gdal_raw_pci")
   {
      return rspfString("raw");
   }
   if(imgTypeStr == "gdal_pcidsk")
   {
      return rspfString("pix");
   }
   if(imgTypeStr == "gdal_sdts")
   {
      return rspfString("sdt");
   }
   if(imgTypeStr == "gdal_jp2ecw")
   {
      return rspfString("jp2");
   }
   if(imgTypeStr == "gdal_ecw")
   {
      return rspfString("ecw");
   }
   return rspfString("ext");
}
void rspfGdalWriter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
	 rspfString propName = property->getName();
	 if (propName.downcase() == "format")
	 {
		 propName = propName.downcase();
	 }
   if(!validProperty(propName))
   {
      rspfImageFileWriter::setProperty(property);
      return;
   }
   rspfString driverName = convertToDriverName(theOutputImageType);
   rspfString name = property->getName();
   if(name == "HFA_USE_RRD")
   {
      bool value = property->valueToString().toBool();
      CPLSetConfigOption("HFA_USE_RRD", value?"YES":"NO");
   }
   else if(name == "gdal_overview_type")
   {
      theGdalOverviewType = gdalOverviewTypeFromString(property->valueToString());
   }
	 else if (name.downcase() == "format")
	 {
		 storeProperty(name,
			 property->valueToString());
	 }
   else if(name == "lut_file")
   {
     theLutFilename = rspfFilename(property->valueToString());
     theColorLut->open(theLutFilename);
   }
   else if(name == "color_lut_flag")
   {
     theColorLutFlag = property->valueToString().toBool();
   }
   else
   {
      name = name.substitute(driverName+"_","");
   
      storeProperty(name,
                    property->valueToString());
   }
}
rspfRefPtr<rspfProperty> rspfGdalWriter::getProperty(const rspfString& name)const
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalWriter::getProperty: entered..." << std::endl;
   }
   if(!validProperty(name))
   {
      return rspfImageFileWriter::getProperty(name);
   }
   if(name == "HFA_USE_RRD")
   {
      return  new rspfBooleanProperty(name, rspfString(CPLGetConfigOption(name.c_str(),"NO")).toBool());
   }
   else if(name == "gdal_overview_type")
   {
      rspfStringProperty* prop =  new rspfStringProperty(name, gdalOverviewTypeToString(), false);
      prop->addConstraint("none");
      prop->addConstraint("nearest");
      prop->addConstraint("average");
      
      return prop;
   }
   
   const rspfRefPtr<rspfXmlNode>  node = getGdalOptions();
   rspfString driverName = convertToDriverName(theOutputImageType);
   GDALDriverH driver =  GDALGetDriverByName(driverName.c_str());
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "Driver name = " << driverName << "\n"
                                          << "name        = " << name << std::endl;
   }
   bool storedValueFlag=false;
   rspfString storedValue;
   if(node.valid()&&driver&&name.contains(driverName))
   {
      rspfString storedName = name;
      storedName = storedName.substitute(driverName+"_","");
      storedValueFlag = getStoredPropertyValue(storedName,
                                               storedValue);
      vector<rspfRefPtr<rspfXmlNode> > nodelist;
      node->findChildNodes("Option", nodelist);
      rspf_uint32 idx = 0;
      for(idx = 0; idx < nodelist.size(); ++idx)
      {
         rspfRefPtr<rspfXmlAttribute> nameAttribute = nodelist[idx]->findAttribute("name");
         if(nameAttribute.valid())
         {
            if((driverName+"_"+nameAttribute->getValue())== name)
            {
               rspfRefPtr<rspfXmlAttribute> type = nodelist[idx]->findAttribute("type");
               
               vector<rspfRefPtr<rspfXmlNode> > valuelist;
               
               if(type.valid())
               {
                  rspfRefPtr<rspfXmlAttribute> descriptionAttribute = nodelist[idx]->findAttribute("description");
                  nodelist[idx]->findChildNodes("Value",
                                       valuelist);
                  rspfString description;
                  rspfString typeValue = type->getValue();
                  typeValue = typeValue.downcase();
                  if(descriptionAttribute.valid())
                  {
                     description = descriptionAttribute->getValue();
                  }
                  if((typeValue == "int")||
                     (typeValue == "integer"))
                  {
                     rspfNumericProperty* prop = new rspfNumericProperty(name,
                                                                           storedValue);
                     prop->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
                     prop->setDescription(description);
                     return prop;
                  }
                  else if(typeValue == "unsigned int")
                  {
                     rspfNumericProperty* prop = new rspfNumericProperty(name,
                                                                           storedValue);
                     prop->setNumericType(rspfNumericProperty::rspfNumericPropertyType_UINT);
                     prop->setDescription(description);
                     return prop;
                  }
                  else if(typeValue == "float")
                  {
                     rspfNumericProperty* prop = new rspfNumericProperty(name,
                                                                           storedValue);
                     prop->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT32);
                     prop->setDescription(description);
                     return prop;
                  }
                  else if(typeValue == "boolean")
                  {
                     rspfBooleanProperty* prop = new rspfBooleanProperty(name,
                                                                           storedValue.toBool());
                     prop->setDescription(description);
                     return prop;
                  }
                  else if((typeValue == "string-select")||
                          (typeValue == "string"))
                  {
                     rspf_uint32 idx2 = 0;
                     rspfString defaultValue = "";
                     
                     if(storedValueFlag)
                     {
                        defaultValue = storedValue;
                     }
                     else if(valuelist.size())
                     {
                        defaultValue = valuelist[0]->getText();
                     }
                     rspfStringProperty* prop = new rspfStringProperty(name,
                                                                         defaultValue,
                                                                         false);
                     for(idx2 = 0; idx2 < valuelist.size();++idx2)
                     {
                        prop->addConstraint(valuelist[idx2]->getText());
                     }
                     prop->setDescription(description);
                     return prop;
                  }
                  else
                  {
                     if(traceDebug())
                     {
                        rspfNotify(rspfNotifyLevel_INFO) << "************************TYPE VALUE = " << typeValue << " NOT HANDLED************"<< std::endl;
                        rspfNotify(rspfNotifyLevel_INFO) << "************************DEFAULTING TO TEXT ************" << std::endl;
                     }
                     rspf_uint32 idx2 = 0;
                     rspfString defaultValue = "";
                     
                     if(storedValueFlag)
                     {
                        defaultValue = storedValue;
                     }
                     else if(valuelist.size())
                     {
                        defaultValue = valuelist[0]->getText();
                     }
                     rspfStringProperty* prop = new rspfStringProperty(name,
                                                                         defaultValue,
                                                                         false);
                     for(idx2 = 0; idx2 < valuelist.size();++idx2)
                     {
                        prop->addConstraint(valuelist[idx2]->getText());
                     }
                     prop->setDescription(description);
                     return prop;
                  }
               }
            }
         }
      }
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalWriter::getProperty: entered..." << std::endl;
   }
   return 0;
}
void rspfGdalWriter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalWriter::getPropertyNames: entered......." << std::endl;
   }
   rspfImageFileWriter::getPropertyNames(propertyNames);
   propertyNames.push_back("gdal_overview_type");
   propertyNames.push_back("HFA_USE_RRD");
   getGdalPropertyNames(propertyNames);
}
void rspfGdalWriter::getGdalPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfString driverName = convertToDriverName(theOutputImageType);
   GDALDriverH driver =  GDALGetDriverByName(driverName.c_str());
   if(driver)
   {
      const rspfRefPtr<rspfXmlNode>  node = getGdalOptions();
      
      if(node.valid())
      {
         vector<rspfRefPtr<rspfXmlNode> > nodelist;
         node->findChildNodes("Option", nodelist);
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "GDAL XML PROPERTY LIST" << std::endl
                                                << node.get() << std::endl;
         }
         rspf_uint32 idx = 0;
         for(idx= 0; idx < nodelist.size(); ++idx)
         {
            rspfRefPtr<rspfXmlAttribute> name = nodelist[idx]->findAttribute("name");
            if(name.valid())
            {
               if(traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG) << "Adding property = " << name->getValue() << std::endl;
               }
               propertyNames.push_back(driverName + "_" + name->getValue());
            }
         }
      }         
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalWriter::getPropertyNames: leaving......." << std::endl;
   }
   
}
const rspfRefPtr<rspfXmlNode> rspfGdalWriter::getGdalOptions()const
{
   rspfString driverName = convertToDriverName(theOutputImageType);
   GDALDriverH driver =  GDALGetDriverByName(driverName.c_str());
   if(driver)
   {
      const char* metaData = GDALGetMetadataItem(driver, GDAL_DMD_CREATIONOPTIONLIST, 0);
      if(metaData)
      {
         istringstream in(metaData);
         rspfRefPtr<rspfXmlNode>  node = new rspfXmlNode(in);
         vector<rspfRefPtr<rspfXmlNode> > nodelist;
         node->findChildNodes("Option", nodelist);
         if(nodelist.size())
         {
            return node;
         }
      }
   }
   return 0;
}
void rspfGdalWriter::storeProperty(const rspfString& name,
                                    const rspfString& value)
{
   rspfString regExpression =  (rspfString("^") +
                                 "property[0-9]+");
   vector<rspfString> keys = theDriverOptionValues.getSubstringKeyList( regExpression );
   rspf_uint32 idx = 0;
   for(idx = 0; idx < keys.size(); ++idx)
   {
      rspfString storedName    = theDriverOptionValues.find(keys[idx] +".name");
      if(storedName == name)
      {
         theDriverOptionValues.add(keys[idx] +".value",
                                   value,
                                   true);
         return;
      }
   }
   rspfString prefix = (rspfString("property") +
                         rspfString::toString(theDriverOptionValues.getSize()/2) +
                         ".");
   theDriverOptionValues.add(prefix.c_str(),
                             "name",
                             name,
                             true);
   theDriverOptionValues.add(prefix.c_str(),
                             "value",
                             value,
                             true);
}
bool rspfGdalWriter::getStoredPropertyValue(const rspfString& name,
                                             rspfString& value)const
{
   rspfString regExpression =  (rspfString("^") +
                                 "property[0-9]+");
   vector<rspfString> keys = theDriverOptionValues.getSubstringKeyList( regExpression );
   rspf_uint32 idx = 0;
   for(idx = 0; idx < keys.size(); ++idx)
   {
      rspfString storedName    = theDriverOptionValues.find(keys[idx] +".name");
      if(storedName == name)
      {
         value   = rspfString(theDriverOptionValues.find(keys[idx] +".value"));
         return true;
      }
   }
   
   return false;
}
bool rspfGdalWriter::validProperty(const rspfString& name)const
{
   std::vector<rspfString> propertyNames;
   if((name == "gdal_overview_type")||
      (name == "HFA_USE_RRD") || name == "format")
   {
      return true;
   }
   getGdalPropertyNames(propertyNames);
   rspf_uint32 idx = 0;
   for(idx = 0; idx < propertyNames.size(); ++idx)
   {
      if(name == propertyNames[idx])
      {
         return true;
      }
   }
   return false;
}
rspfString rspfGdalWriter::gdalOverviewTypeToString()const
{
   switch(theGdalOverviewType)
   {
      case rspfGdalOverviewType_NONE:
      {
         return "none";
      }
      case rspfGdalOverviewType_NEAREST:
      {
         return "nearest";
      }
      case rspfGdalOverviewType_AVERAGE:
      {
         return "average";
      }
   }
   return "none";
}
rspfGdalWriter::rspfGdalOverviewType rspfGdalWriter::gdalOverviewTypeFromString(const rspfString& typeString)const
{
   rspfString tempType = typeString;
   tempType = tempType.downcase();
   tempType = tempType.trim();
   if(tempType == "nearest")
   {
      return rspfGdalOverviewType_NEAREST;
   }
   else if(tempType == "average")
   {
      return rspfGdalOverviewType_AVERAGE;
   }
   return rspfGdalOverviewType_NONE;
}
void rspfGdalWriter::buildGdalOverviews()
{
  if(!theDataset)
  {
    return;
  }
  rspfIrect bounds = theInputConnection->getBoundingRect();
  
  rspf_uint32 minValue = rspf::min((rspf_uint32)bounds.width(),
				   (rspf_uint32)bounds.height());
  
  int nLevels = static_cast<int>(std::log((double)minValue)/std::log(2.0));
  int idx = 0;
  if(nLevels)
    {
      rspf_int32* levelsPtr = new rspf_int32[nLevels];
      *levelsPtr = 2;
      for(idx = 1; idx < nLevels;++idx)
	{
	  levelsPtr[idx] = levelsPtr[idx-1]*2;
	}
      if(GDALBuildOverviews(theDataset,
			    gdalOverviewTypeToString().c_str(),
			    nLevels,
			    levelsPtr,
			    0,
			    0,
			    GDALTermProgress,
			    0)!=CE_None)
	{
	  rspfNotify(rspfNotifyLevel_WARN) << "rspfGdalWriter::buildGdalOverviews():  Overview building failed" 
					     << std::endl;
	}
      delete [] levelsPtr;
    }
}
void rspfGdalWriter::postProcessOutput() const
{
   if (theDriver)
   {
      rspfString driverName = GDALGetDriverShortName(theDriver);
      if ( (driverName == "JP2MrSID") || (driverName != "JP2KAK") ||
           (driverName == "JPEG2000") )
      {
         rspfFilename auxXmlFile = theFilename;
         auxXmlFile += ".aux.xml";
         if ( auxXmlFile.exists() )
         {
            if ( auxXmlFile.remove() )
            {
               rspfNotify(rspfNotifyLevel_NOTICE)
                  << "rspfGdalWriter::postProcessOutput NOTICE:"
                  << "\nFile removed:  " << auxXmlFile << std::endl;
            }
         }
      }
   }
}
bool rspfGdalWriter::isInputDataIndexed()
{
   bool result = false;
   if ( theInputConnection.valid() )
   {
      result = theInputConnection->isIndexedData();
   }
   return result;
}
