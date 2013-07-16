//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id: rspfImageHandlerFactory.cpp 22230 2013-04-12 16:34:05Z dburken $

#include <rspf/imaging/rspfImageHandlerFactory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfRegExp.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfAdrgTileSource.h>
#include <rspf/imaging/rspfCcfTileSource.h>
#include <rspf/imaging/rspfCibCadrgTileSource.h>
#include <rspf/imaging/rspfDoqqTileSource.h>
#include <rspf/imaging/rspfDtedTileSource.h>
#include <rspf/imaging/rspfEnviTileSource.h>
#include <rspf/imaging/rspfNitfTileSource.h>
// #include <rspf/imaging/rspfPdfReader.h>
#include <rspf/imaging/rspfQuickbirdNitfTileSource.h>
#include <rspf/imaging/rspfSrtmTileSource.h>
#include <rspf/imaging/rspfQuickbirdTiffTileSource.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfUsgsDemTileSource.h>
#include <rspf/imaging/rspfLandsatTileSource.h>
#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/imaging/rspfERSTileSource.h>
#include <rspf/imaging/rspfVpfTileSource.h>
#include <rspf/imaging/rspfJpegTileSource.h>
#include <rspf/imaging/rspfRpfCacheTileSource.h>
#include <rspf/imaging/rspfImageCacheTileSource.h>
#include <rspf/imaging/rspfQbTileFilesHandler.h>
#include <rspf/imaging/rspfBitMaskTileSource.h>
#include <rspf/imaging/rspfBandSeparateHandler.h>
#include <rspf/parallel/rspfImageHandlerMtAdaptor.h>


static const rspfTrace traceDebug("rspfImageHandlerFactory:debug");

RTTI_DEF1(rspfImageHandlerFactory, "rspfImageHandlerFactory", rspfImageHandlerFactoryBase);

rspfImageHandlerFactory* rspfImageHandlerFactory::theInstance = 0;
rspfImageHandlerFactory::~rspfImageHandlerFactory()
{
   theInstance = (rspfImageHandlerFactory*)0;
}

rspfImageHandlerFactory* rspfImageHandlerFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageHandlerFactory;

      // let's turn off tiff error reporting
      TIFFSetErrorHandler(0);
      TIFFSetWarningHandler(0);
   }

   return theInstance;
}

rspfImageHandler* rspfImageHandlerFactory::open(const rspfFilename& fileName,
                                                  bool openOverview) const
{
   static const char* M = "rspfImageHandlerFactory::open(filename) -- ";
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG) << M <<" Entering..." << std::endl;

   rspfFilename copyFilename = fileName;
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "Attempting to open file: " << fileName.c_str()
         << "\nopen overview flag: " << (openOverview?"true":"false") << "\n";
   }

   rspfRefPtr<rspfImageHandler> result = 0;
   while (true)
   {
      // Check for empty file.
      copyFilename.trim();
      if (copyFilename.empty()) break;

      // for all of our imagehandlers the filename must exist.
      // if we have any imagehandlers that require an encoded string and is contrlled in this factory then
      // we need to move this.
      if (!copyFilename.exists())  break;

      rspfString ext = copyFilename.ext().downcase();
      if(ext == "gz")
         copyFilename = copyFilename.setExtension("");

      // Try opening from extension logic first (this is faster than instantiating each type).
//      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying via extension...";
//      result = openFromExtension(copyFilename);
//      if (result.valid())  break;

      //---
      // If here do it the brute force way by going down the list of available
      // readers...
      //---
      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying RPF Cache Image...\n";
      result = new rspfRpfCacheTileSource;
      result->setOpenOverviewFlag(openOverview);
      if (result->open(copyFilename)) break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying RPF Image Cache Image...\n";
      result = new rspfImageCacheTileSource;
      result->setOpenOverviewFlag(openOverview);
      if (result->open(copyFilename)) break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying JPEG...\n";
      result = new rspfJpegTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename)) break;

      // this must be checked first before the TIFF handler
      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying Quickbird TIFF...\n";
      result = new rspfQuickbirdTiffTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename)) break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying TIFF...\n";
      result = new rspfTiffTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying CIB/CADRG...\n";
      result = new rspfCibCadrgTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying DOQQ...\n";
      result = new rspfDoqqTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying DTED...\n";
      result = new rspfDtedTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      // this must be checked first before the NITF raw handler
      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying Quickbird Nitf...\n";
      result = new rspfQuickbirdNitfTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying NITF...\n";
      result = new rspfNitfTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying USGS Dem...\n";
      result = new rspfUsgsDemTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying Landsat...\n";
      result = new rspfLandsatTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying VPF...\n";
      result = new rspfVpfTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying ERS...\n";
      result = new rspfERSTileSource;
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      //---
      // The srtm and general raser readers were picking up j2k overviews because the
      // matching base file has an "omd" file that the raster reader can load
      // so added extension check.  (drb - 20100709)
      //---
      if (copyFilename.ext() != "ovr")
      {
         // Note:  ENVI should be in front of general raster..
         if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying ENVI...\n";
         result = new rspfEnviTileSource;
         result->setOpenOverviewFlag(openOverview);         
         if (result->open(copyFilename))  break;

         // Note:  SRTM should be in front of general raster..
         if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying SRTM...\n";
         result = new rspfSrtmTileSource;
         result->setOpenOverviewFlag(openOverview);         
         if (result->open(copyFilename))  break;

         if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying General Raster...\n";
         result = new rspfGeneralRasterTileSource; 
         result->setOpenOverviewFlag(openOverview);        
         if (result->open(copyFilename))  break;
      }

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying ADRG...\n";
      result = new rspfAdrgTileSource();
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying rspfQbTileFilesHandler...\n";
      result = new rspfQbTileFilesHandler();
      result->setOpenOverviewFlag(openOverview);
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying Bit Mask...\n";
      result = new rspfBitMaskTileSource();
      result->setOpenOverviewFlag(openOverview);
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying band-separated files...\n";
      result = new rspfBandSeparateHandler();
      result->setOpenOverviewFlag(openOverview);      
      if (result->open(copyFilename))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying CCF...\n";
      result->setOpenOverviewFlag(openOverview);      
      result = new rspfCcfTileSource();
      if (result->open(copyFilename))  break;

      result = 0;
      break;
   }

   if (traceDebug())
   {
      if (result.valid())
         rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "   SUCCESS" << std::endl;
      else
         rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "   Open FAILED" << std::endl;
   }

   return result.release();
}

rspfImageHandler* rspfImageHandlerFactory::open(const rspfKeywordlist& kwl,
                                                  const char* prefix)const
{
   static const char* M = "rspfImageHandlerFactory::open(kwl,prefix) -- ";
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG) << M <<" entered..." << std::endl;

   rspfRefPtr<rspfImageHandler> result = 0;
   while (true)
   {
      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying adrg...\n";
      result = new rspfAdrgTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying ccf...\n";
      result  = new rspfCcfTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M << "trying CIB/CADRG...\n";
      result  = new rspfCibCadrgTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "trying RpfCache...\n";
      result  = new rspfRpfCacheTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "trying Rpf ImageCache...\n";
      result  = new rspfImageCacheTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M << "trying DOQQ...\n";
      result  = new rspfDoqqTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying DTED...\n";
      result  = new rspfDtedTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying jpeg...\n";
      result  = new rspfJpegTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying Quickbird NITF...\n";
      result = new rspfQuickbirdNitfTileSource;
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M << "trying Nitf...\n";
      result  = new rspfNitfTileSource();
      if (result->loadState(kwl, prefix))  break;

      // Must be before tiff...
      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying Quickbird TIFF...\n";
      result = new rspfQuickbirdTiffTileSource;
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying tiff...\n";
      result  = new rspfTiffTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying USGS Dem...\n";
      result  = new rspfUsgsDemTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying Landsat...\n";
      result  = new rspfLandsatTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying VPF...\n";
      result = new rspfVpfTileSource;
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying ERS...\n";
      result = new rspfERSTileSource;
      if (result->loadState(kwl, prefix))  break;

      // Note:  ENVI should be in front of general raster...
      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying ENVI...\n"<< std::endl;
      result  = new rspfEnviTileSource();
      if (result->loadState(kwl, prefix))  break;

      // Note:  SRTM should be in front of general raster...
      if(traceDebug())  rspfNotify(rspfNotifyLevel_DEBUG) << M<< "trying SRTM...\n"<< std::endl;
      result  = new rspfSrtmTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG) << M << "trying General Raster...\n"<< std::endl;
      result  = new rspfGeneralRasterTileSource();
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"trying rspfQbTileFilesHandler...\n"<<std::endl;
      result = new rspfQbTileFilesHandler;
      if (result->loadState(kwl, prefix))  break;

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"trying rspfQbTileFilesHandler...\n"<<std::endl;
      result = new rspfQbTileFilesHandler;
      if (result->loadState(kwl, prefix))  break;

      if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "Trying band-separated files...\n";
      result = new rspfBandSeparateHandler();
      if (result->loadState(kwl, prefix))  break;

      result = 0;
      break;
   }

   if (traceDebug())
   {
      if (result.valid())
         rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "   SUCCESS" << std::endl;
      else
         rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "   Open FAILED" << std::endl;
   }

   return result.release();
}

rspfRefPtr<rspfImageHandler> rspfImageHandlerFactory::openOverview(
   const rspfFilename& file ) const
{
   rspfRefPtr<rspfImageHandler> result = 0;
   if ( file.size() )
   {
      result = new rspfTiffTileSource;
      
      result->setOpenOverviewFlag( false ); // Always false.

      if ( result->open( file ) == false )
      {
         result = 0;
      }
   }
   return result;
}

#if 0
rspfImageHandler* rspfImageHandlerFactory::openFromExtension(const rspfFilename& fileName) const
{
   
   static const char* M = "rspfImageHandlerFactory::openFromExtension() -- ";
   if(traceDebug()) 
      rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Attempting to open <"<<fileName<<">"<<std::endl;

   rspfString ext = fileName.ext().downcase();
   rspfRefPtr<rspfImageHandler> result = 0;
   
   
   while (true)
   {
      // OVR can be combined with "tif" once we get rid of rspfQuickbirdTiffTileSource
      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying OVR...\n";
      if (ext == "ovr")
      {
         result = new rspfTiffTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying TIF or TIFF...\n";
      if ( (ext == "tif") || (ext == "tiff") )
      {
         // this must be checked first before the TIFF handler
         result = new rspfQuickbirdTiffTileSource;
         if(result->open(fileName)) break;
         result = new rspfTiffTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying NTF or NITF...\n";
      if ( (ext == "ntf") || (ext == "nitf") )
      {
         // this must be checked first before the NITF raw handler
         result = new rspfQuickbirdNitfTileSource;
         if(result->open(fileName)) break;
         result = new rspfNitfTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying RPF...\n";
      if ( (fileName == "rpf"))
      {
         result = new rspfRpfCacheTileSource;
         if(result->open(fileName)) break;

         result = new rspfImagefCacheTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying TOC...\n";
      if ( (fileName == "a.toc") || (ext == "toc"))
      {
         result = new rspfCibCadrgTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying JPG or JPEG...\n";
      if ( (ext == "jpg") || (ext == "jpeg") )
      {
         result = new rspfJpegTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying DOQ or DOQQ...\n";
      if ( (ext == "doq") || (ext == "doqq") )
      {
         result = new rspfDoqqTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying DTn...\n";
      if ( (ext == "dt2") || (ext == "dt1") || (ext == "dt3") ||
         (ext == "dt4") || (ext == "dt5") || (ext == "dt0") )
      {
         result = new rspfDtedTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying HGT...\n";
      if (ext == "hgt")
      {
         result = new rspfSrtmTileSource;
         if(result->open(fileName)) break;
      }  

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying HRI,HSI\n";
      if ( (ext == "hri") || (ext == "hsi") )
      {
         result = new rspfEnviTileSource;
         if(result->open(fileName)) break;
      }  

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying DEM...\n";
      if (ext == "dem")
      {
         result = new rspfUsgsDemTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying FST...\n";
      if (ext == "fst")
      {
         result = new rspfLandsatTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying RAS or RAW...\n";
      if ( (ext == "ras") || (ext == "raw") )
      {
         result = new rspfGeneralRasterTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying IMG...\n";
      if (ext == "img")
      {
         result  = new rspfAdrgTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying CCF...\n";
      if (ext == "ccf")
      {
         result = new rspfCcfTileSource;
         if(result->open(fileName)) break;
      }

      if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying TIL...\n";
      if (ext == "til")
      {
         result = new rspfQbTileFilesHandler;
         if(result->open(fileName)) break;
      }

      result = 0;
      break;
   }
   if (traceDebug())
   {
      if (result.valid())
         rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "   SUCCESS" << std::endl;
      else
         rspfNotify(rspfNotifyLevel_DEBUG)<<M<< "   Open FAILED" << std::endl;
   }
   return result.release();
}
#endif
rspfObject* rspfImageHandlerFactory::createObject(const rspfString& typeName)const
{
   if(STATIC_TYPE_NAME(rspfAdrgTileSource) == typeName)
   {
      return new rspfAdrgTileSource();
   }
   if(STATIC_TYPE_NAME(rspfCcfTileSource) == typeName)
   {
      return new rspfCcfTileSource();
   }
   if(STATIC_TYPE_NAME(rspfCibCadrgTileSource) == typeName)
   {
      return new rspfCibCadrgTileSource();
   }
   if(STATIC_TYPE_NAME(rspfRpfCacheTileSource) == typeName)
   {
     return new rspfRpfCacheTileSource();
   }
   if(STATIC_TYPE_NAME(rspfImageCacheTileSource) == typeName)
   {
      return new rspfImageCacheTileSource();
   }
   if(STATIC_TYPE_NAME(rspfDoqqTileSource) == typeName)
   {
      return new rspfDoqqTileSource();
   }
   if(STATIC_TYPE_NAME(rspfDtedTileSource) == typeName)
   {
      return new rspfDtedTileSource();
   }
   if(STATIC_TYPE_NAME(rspfEnviTileSource) == typeName)
   {
      return new rspfEnviTileSource();
   }
   if(STATIC_TYPE_NAME(rspfJpegTileSource) == typeName)
   {
      return new rspfJpegTileSource();
   }
   if(STATIC_TYPE_NAME(rspfNitfTileSource) == typeName)
   {
      return new rspfNitfTileSource();
   }
   if(STATIC_TYPE_NAME(rspfTiffTileSource) == typeName)
   {
      return new rspfTiffTileSource();
   }
   if(STATIC_TYPE_NAME(rspfUsgsDemTileSource) == typeName)
   {
      return new rspfUsgsDemTileSource();
   }
   if(STATIC_TYPE_NAME(rspfLandsatTileSource) == typeName)
   {
      return new rspfLandsatTileSource();
   }
   if(STATIC_TYPE_NAME(rspfERSTileSource) == typeName)
   {
      return new rspfERSTileSource();
   }
   if(STATIC_TYPE_NAME(rspfSrtmTileSource) == typeName)
   {
      return new rspfSrtmTileSource();
   }
   if(STATIC_TYPE_NAME(rspfGeneralRasterTileSource) == typeName)
   {
      return new rspfGeneralRasterTileSource();
   }
   if(STATIC_TYPE_NAME(rspfQbTileFilesHandler) == typeName)
   {
      return new rspfQbTileFilesHandler();
   }
   if(STATIC_TYPE_NAME(rspfBitMaskTileSource) == typeName)
   {
      return new rspfBitMaskTileSource();
   }
   if(STATIC_TYPE_NAME(rspfBandSeparateHandler) == typeName)
   {
      return new rspfBandSeparateHandler();
   }
   if(STATIC_TYPE_NAME(rspfImageHandlerMtAdaptor) == typeName)
   {
      return new rspfImageHandlerMtAdaptor();
   }
   
   return (rspfObject*)0;
}

void rspfImageHandlerFactory::getSupportedExtensions(rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const
{
   extensionList.push_back("img");
   extensionList.push_back("ccf"); 
   extensionList.push_back("toc");
   extensionList.push_back("tif");
   extensionList.push_back("tiff");
   extensionList.push_back("doq");
   extensionList.push_back("doqq");
   extensionList.push_back("dt0");
   extensionList.push_back("dt1");
   extensionList.push_back("dt2");
   extensionList.push_back("dt3");
   extensionList.push_back("jpg");
   extensionList.push_back("jpeg");
   extensionList.push_back("dem");
   extensionList.push_back("fst");
   extensionList.push_back("hdr");
   extensionList.push_back("ras");
   extensionList.push_back("hgt");
   extensionList.push_back("nsf");
   extensionList.push_back("nitf");
   extensionList.push_back("ntf");
   extensionList.push_back("til");
   extensionList.push_back("mask");
   extensionList.push_back("txt");
}

void rspfImageHandlerFactory::getImageHandlersBySuffix(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& ext)const
{
   static const char* M = "rspfImageHandlerFactory::getImageHandlersBySuffix() -- ";
   // OVR can be combined with "tif" once we get rid of rspfQuickbirdTiffTileSource
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Trying OVR...\n";
   rspfString testExt = ext.downcase();
   if (testExt == "ovr")
   {
      result.push_back(new rspfTiffTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing TIF or TIFF...\n";
   if ( (testExt == "tif") || (testExt == "tiff") )
   {
      // this must be checked first before the TIFF handler
      result.push_back(new rspfQuickbirdTiffTileSource);
      result.push_back(new rspfTiffTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing NTF or NITF...\n";
   if ( (testExt == "ntf") || (testExt == "nitf") )
   {
      // this must be checked first before the NITF raw handler
      result.push_back(new rspfQuickbirdNitfTileSource);
      result.push_back(new rspfNitfTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing RPF...\n";
   if ( (testExt == "rpf"))
   {
      result.push_back(new rspfRpfCacheTileSource);
      result.push_back(new rspfImageCacheTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing TOC...\n";
   if ( testExt == "toc")
   {
      result.push_back(new rspfCibCadrgTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing JPG or JPEG...\n";
   if ( (testExt == "jpg") || (testExt == "jpeg") )
   {
      result.push_back(new rspfJpegTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing DOQ or DOQQ...\n";
   if ( (testExt == "doq") || (testExt == "doqq") )
   {
      result.push_back(new rspfDoqqTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing DTn...\n";
   rspfString regExpStr = "dt[0-9]";
   rspfRegExp regExp(regExpStr);
   if(regExp.find(testExt))
   {
      result.push_back(new rspfDtedTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing HGT...\n";
   if (testExt == "hgt")
   {
      result.push_back(new rspfSrtmTileSource);
      return;
   }  
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing HRI,HSI...\n";
   if ( (testExt == "hri") || (testExt == "hsi") )
   {
      result.push_back(new rspfEnviTileSource);
      return;
   }  
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing DEM...\n";
   if (testExt == "dem")
   {
      result.push_back(new rspfUsgsDemTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing FST...\n";
   if (testExt == "fst")
   {
      result.push_back(new rspfLandsatTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing RAS or RAW or General Raster...\n";
   if ( (testExt == "ras") || (testExt == "raw") || (testExt == "bil"))
   {
      result.push_back(new rspfGeneralRasterTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing IMG...\n";
   if (testExt == "img")
   {
      result.push_back(new rspfAdrgTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing CCF...\n";
   if (testExt == "ccf")
   {
      result.push_back(new rspfCcfTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing TIL...\n";
   if (testExt == "til")
   {
      result.push_back(new rspfQbTileFilesHandler);
      return;
   }

   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing MASK...\n";
   if (testExt == "mask")
   {
      result.push_back(new rspfBitMaskTileSource);
      return;
   }
   
   if(traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)<<M<<"Testing TXT...\n";
   if (testExt == "txt")
   {
      result.push_back(new rspfBandSeparateHandler);
      return;
   }
}

void rspfImageHandlerFactory::getImageHandlersByMimeType(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& mimeType)const
{
   rspfString test(mimeType.begin(), mimeType.begin()+6);
   if(test == "image/")
   {
      rspfString mimeTypeTest(mimeType.begin() + 6, mimeType.end());
      getImageHandlersBySuffix(result, mimeTypeTest);
      if(mimeTypeTest == "dted")
      {
         result.push_back(new rspfDtedTileSource);
      }
   }
}

rspfObject* rspfImageHandlerFactory::createObject(const rspfKeywordlist& kwl,
                                                    const char* prefix)const
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageHandlerFactory::createObject(kwl, prefix) DEBUG:"
         << " entering ..." << std::endl;
   }
   rspfObject* result = (rspfObject*)0;
   const char* type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);

   if(type)
   {
      if (rspfString(type).trim() == STATIC_TYPE_NAME(rspfImageHandler))
      {
         const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);

         if (lookup)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "BEBUG: filename " << lookup << std::endl;
            }
            // Call the open that takes a filename...
            result = this->open(kwl, prefix);//rspfFilename(lookup));
         }
      }
      else
      {
         result = createObject(rspfString(type));
         if(result)
         {
            result->loadState(kwl, prefix);
         }
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageHandlerFactory::createObject(kwl, prefix) DEBUG: returning result ..." << std::endl;
   }
   return result;
}

void rspfImageHandlerFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfAdrgTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfCcfTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfCibCadrgTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfEnviTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfRpfCacheTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageCacheTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfDoqqTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfDtedTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfJpegTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfNitfTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfTiffTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfUsgsDemTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfLandsatTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfERSTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfSrtmTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfGeneralRasterTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfQuickbirdNitfTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfQuickbirdTiffTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfQbTileFilesHandler));
   typeList.push_back(STATIC_TYPE_NAME(rspfBitMaskTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageHandlerMtAdaptor));
}
