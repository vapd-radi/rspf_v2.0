//----------------------------------------------------------------------------
// File: rspfImageUtil.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: rspfImageUtil class definition
//
// Utility class for processing image recursively.  This is for doing things like:
// building overview, histograms, compute min/max, extract vertices.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/util/rspfImageUtil.h>
#include <rspf/base/rspfArgumentParser.h>
#include <rspf/base/rspfApplicationUsage.h>
#include <rspf/base/rspfCallback1.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/init/rspfInit.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfPropertyInterface.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryRegistry.h>
#include <rspf/init/rspfInit.h>
#include <rspf/plugin/rspfSharedPluginRegistry.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/support_data/rspfSupportFilesList.h>
#include <rspf/util/rspfFileWalker.h>

#include <iomanip>
#include <string>
#include <vector>

static std::string COPY_ALL_FLAG_KW           = "copy_all_flag";
static std::string CREATE_HISTOGRAM_KW        = "create_histogram";
static std::string CREATE_HISTOGRAM_FAST_KW   = "create_histogram_fast";
static std::string CREATE_HISTOGRAM_R0_KW     = "create_histogram_r0";
static std::string CREATE_OVERVIEWS_KW        = "create_overviews";
static std::string FALSE_KW                   = "false";
static std::string FILE_KW                    = "file";
static std::string INTERNAL_OVERVIEWS_FLAG_KW = "internal_overviews_flag";
static std::string OUTPUT_DIRECTORY_KW        = "output_directory";
static std::string OUTPUT_FILENAMES_KW        = "output_filenames";
static std::string OVERVIEW_STOP_DIM_KW       = "overview_stop_dimension";
static std::string OVERVIEW_TYPE_KW           = "overview_type";
static std::string READER_PROP_KW             = "reader_prop";
static std::string REBUILD_HISTOGRAM_KW       = "rebuild_histogram";
static std::string REBUILD_OVERVIEWS_KW       = "rebuild_overviews";
static std::string SCAN_MIN_MAX_KW            = "scan_for_min_max";
static std::string SCAN_MIN_MAX_NULL_KW       = "scan_for_min_max_null";
static std::string THREADS_KW                 = "threads";
static std::string TILE_SIZE_KW               = "tile_size";
static std::string TRUE_KW                    = "true";
static std::string WRITER_PROP_KW             = "writer_prop";

//---
// Call back class to register with rspfFileWalker for call to
// rspfImageUtil::processFile
//
// Placed here as it is unique to this class.
//---
class ProcessFileCB: public rspfCallback1<const rspfFilename&>
{
public:
   ProcessFileCB(
      rspfImageUtil* obj,
      void (rspfImageUtil::*func)(const rspfFilename&))
      :
      m_obj(obj),
      m_func(func)
   {}
      
   virtual void operator()(const rspfFilename& file) const
   {
      (m_obj->*m_func)(file);
   }

private:
   rspfImageUtil* m_obj;
   void (rspfImageUtil::*m_func)(const rspfFilename& file);
};


// Static trace for debugging.  Use -T rspfImageUtil to turn on.
static rspfTrace traceDebug = rspfTrace("rspfImageUtil:debug");

rspfImageUtil::rspfImageUtil()
   :
   m_kwl( new rspfKeywordlist() ),
   m_fileWalker(0),
   m_mutex(),
   m_errorStatus(0)
{
}

rspfImageUtil::~rspfImageUtil()
{
   if ( m_fileWalker )
   {
      delete m_fileWalker;
      m_fileWalker = 0;
   }
}

void rspfImageUtil::addArguments(rspfArgumentParser& ap)
{
   // Set the general usage:
   rspfApplicationUsage* au = ap.getApplicationUsage();
   rspfString usageString = ap.getApplicationName();
   usageString += " [options] <file-or-directory-to-walk>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption("-a or --include-fullres", "Copy full res dataset to overview file as well as building reduced res sets. Option only valid with tiff overview builder. Requires -o option.");

   au->addCommandLineOption("--compression-quality", "Compression quality for TIFF JPEG takes values from 0 to 100, where 100 is best.  For J2K plugin, numerically_lossless, visually_lossless, lossy");
   
   au->addCommandLineOption("--compression-type", "Compression type can be: deflate, jpeg, lzw, none or packbits");

   au->addCommandLineOption("--ch or --create-histogram", "Computes full histogram alongside overview.");
   
   au->addCommandLineOption("--chf or --create-histogram-fast", "Computes a histogram in fast mode which samples partial tiles.");

   au->addCommandLineOption("--create-histogram-r0", "Forces create-histogram code to compute a histogram using r0 instead of the starting resolution for the overview builder. Can require a separate pass of R0 layer if the base image has built in overviews.");

   au->addCommandLineOption("-d", "<output_directory> Write overview to output directory specified.");

   au->addCommandLineOption("--of or --output-files", "Output image files we can open, exluding overviews.");
   
   au->addCommandLineOption("-h", "Display this information");

   au->addCommandLineOption("-i or --internal-overviews", "Builds internal overviews. Requires -o option. Option only valid with tiff input image and tiff overview builder. WARNING: Modifies source image and cannot be undone!");
   
   au->addCommandLineOption("--list-entries", "Lists the entries within the image");

   au->addCommandLineOption("-o", "Creates overviews. (default=rspf_tiff_box)");

   au->addCommandLineOption("--ot", "<overview_type> Overview type. see list at bottom for valid types. (default=rspf_tiff_box)");

   au->addCommandLineOption("-r or --rebuild-overviews", "Rebuild overviews even if they are already present.");
   
   au->addCommandLineOption("--rebuild-histogram", "Rebuild histogram even if they are already present.");

   au->addCommandLineOption("--reader-prop", "Adds a property to send to the reader. format is name=value");

   au->addCommandLineOption("-s",  "Stop dimension for overviews.  This controls how \nmany layers will be built. If set to 64 then the builder will stop when height and width for current level are less than or equal to 64.  Note a default can be set in the rspf preferences file by setting the keyword \"overview_stop_dimension\".");

   au->addCommandLineOption("--tile-size", "<size> Defines the tile size for overview builder.  Tiff option only. Must be a multiple of 16. Size will be used in both x and y directions. Note a default can be set in your rspf preferences file by setting the key \"tile_size\".");

   au->addCommandLineOption("--threads", "<threads> The number of threads to use. (default=1) Note a default can be set in your rspf preferences file by setting the key \"rspf_threads\".");

   au->addCommandLineOption("--scanForMinMax", "Turns on min, max scanning when reading tiles. This option assumes the null is known.");

   au->addCommandLineOption("--scanForMinMaxNull", "Turns on min, max, null scanning when reading tiles.  This option tries to find a null value which is useful for float data.");

   au->addCommandLineOption("--writer-prop", "Adds a property to send to the writer. format is name=value");
   
} // void rspfImageUtil::addArguments(rspfArgumentParser& ap)

bool rspfImageUtil::initialize(rspfArgumentParser& ap)
{
   static const char M[] = "rspfImageUtil::initialize(rspfArgumentParser&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }

   bool result = true;

   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      usage(ap);

      // continue_after_init to false
      result = false;
   }
   else
   {
      //---
      // Start with clean options keyword list.
      //---
      m_kwl->clear();

      while ( 1 ) //  While forever loop...
      {
         // Used throughout below:
         std::string ts1;
         rspfArgumentParser::rspfParameter sp1(ts1);
         std::string ts2;
         rspfArgumentParser::rspfParameter sp2(ts2);

         if( ap.read("-a") || ap.read("--include-fullres") )
         {
            setCopyAllFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--compression-quality", sp1) )
         {
            if ( ts1.size() )
            {
               setCompressionQuality( ts1 );
            }
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--compression-type", sp1) )
         {
            if ( ts1.size() )
            {
               setCompressionType( ts1 );
            }
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("--ch") || ap.read("--create-histogram") )
         {
            setCreateHistogramFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("--chf") || ap.read("--create-histogram-fast") )
         {
            setCreateHistogramFastFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("--create-histogram-r0") )
         {
            setCreateHistogramR0Flag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("-d", sp1) )
         {
            setOutputDirectory( ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("-i") || ap.read("--internal-overviews") )
         {
            setInternalOverviewsFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--of") || ap.read("--output-files") )
         {
            setOutputFileNamesFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("-o") )
         {
            setCreateOverviewsFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }        
         
         if( ap.read("--ot", sp1) )
         {
            setOverviewType( ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("-r") || ap.read("--rebuild-overviews") )
         {
            setRebuildOverviewsFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--rebuild-histogram") )
         {
            setRebuildHistogramFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         while(ap.read("--reader-prop", sp1))
         {
            if (ts1.size())
            {
               std::string key = READER_PROP_KW;
               key += rspfString::toString( getNextReaderPropIndex() ).string();
               addOption( key, ts1 );
            }
         }
         if ( ap.argc() < 2 )
         {
            break;
         }
         
         if( ap.read("--scanForMinMax" ) )
         {
            setScanForMinMax( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--scanForMinMaxNull" ) )
         {
            setScanForMinMaxNull( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
         
         if( ap.read("-s", sp1) )
         {
            setOverviewStopDimension( ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if ( ap.read("-tile-size", sp1))
         {
            setTileSize( rspfString(ts1).toInt32() );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--threads", sp1) )
         {
            m_kwl->addPair( THREADS_KW, ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         while(ap.read("--writer-prop", sp1))
         {
            if (ts1.size())
            {
               std::string key = WRITER_PROP_KW;
               key += rspfString::toString( getNextWriterPropIndex() ).string();
               addOption( key, ts1 );
            }
         }
         if ( ap.argc() < 2 )
         {
            break;
         }

         // End of arg parsing.
         ap.reportRemainingOptionsAsUnrecognized();
         if ( ap.errors() )
         {
            ap.writeErrorMessages(rspfNotify(rspfNotifyLevel_NOTICE));
            std::string errMsg = "Unknown option...";
            throw rspfException(errMsg);
         }

         break; // Break from while forever.
         
      } // End while (forever) loop.

      if(ap.argc() > 1)
      {
         for (rspf_int32 i = 0; i < (ap.argc()-1); ++i)
         {
            rspfString kw = "file";
            kw += rspfString::toString(i);
            std::string value = ap[i+1];
            m_kwl->addPair(kw.string(), value, true);
         }
      }
      else
      {
         usage(ap);
         result = false;
      }

   } // not usage

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "m_kwl:\n" << *(m_kwl.get()) << "\n"
         << M << " exit result = " << (result?"true":"false")
         << "\n";
   }
   
   return result;
}

rspf_int32 rspfImageUtil::execute()
{
   static const char M[] = "rspfImageUtil::execute()";
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }

   if ( !m_fileWalker )
   {
      m_fileWalker = new rspfFileWalker();
   }
   
   // Get the number of "file*" keywords.
   rspf_uint32 fileCount = m_kwl->numberOf("file");

   if ( fileCount )
   {
      m_fileWalker->initializeDefaultFilterList();
      
      m_fileWalker->setNumberOfThreads( getNumberOfThreads() );

      // Must set this so we can stop recursion on directory based images.
      m_fileWalker->setWaitOnDirFlag( true );

      rspfCallback1<const rspfFilename&>* cb =
         new ProcessFileCB(this, &rspfImageUtil::processFile);

      m_fileWalker->registerProcessFileCallback(cb);
      
      // Wrap in try catch block as excptions can be thrown under the hood.
      try
      {
         // Get the list of files passed to us:
         std::vector<rspfFilename> files;
         rspf_uint32 processedFiles = 0;;
         rspf_uint32 i = 0;
         while ( processedFiles < fileCount )
         {
            rspfString kw = FILE_KW;
            kw += rspfString::toString(i);
            std::string lookup = m_kwl->findKey( kw.string() );
            if ( lookup.size() )
            {
               files.push_back( rspfFilename(lookup) );
               ++processedFiles;
            }
            
            ++i;
            if ( i > (fileCount + 100) ) break;
         }

         // Process the files:
         m_fileWalker->walk( files ); 
      }
      catch (const rspfException& e)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Caught exception: " << e.what() << endl;
         setErrorStatus( rspfErrorCodes::RSPF_ERROR );
      }
      
      // cleanup:
      if ( cb )
      {
         delete cb;
         cb = 0;
      }
      
   } // if ( fileCount )

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " exit status: " << m_errorStatus << std::endl;
   }
   
   // Zero is good, non zero is bad.
   return m_errorStatus; 
}

//---
// This method is called back by the rspfFileWalker::walk method for each file it finds that it
// deems can be processed.
//---
void rspfImageUtil::processFile(const rspfFilename& file)
{
   static const char M[] = "rspfImageUtil::processFile";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n" << "file: " << file << "\n";
   }

   rspfNotify(rspfNotifyLevel_NOTICE) << "Processing file: " << file << "\n";

   m_mutex.lock();
   rspfRefPtr<rspfImageHandler> ih =
      rspfImageHandlerRegistry::instance()->open(file, true, true);


   m_mutex.unlock();

   if ( ih.valid() && !ih->hasError() )
   {
      if ( isDirectoryBasedImage( ih.get() ) )
      {
         // Tell the walker not to recurse this directory.
         m_mutex.lock();
         m_fileWalker->setRecurseFlag(false);
         m_mutex.unlock();
      }

 
      // Set any reader props:
      rspfPropertyInterface* pi = dynamic_cast<rspfPropertyInterface*>(ih.get());
      if ( pi ) setProps(pi);

      bool consumedHistogramOptions  = false;
      bool consumedCmmOptionsOptions = false;
 
      if ( getOutputFileNamesFlag() )
      {
         // Simply output the file name of any images we can open:
         rspfNotify(rspfNotifyLevel_NOTICE) << ih->getFilename().expand(); 
      }
     
      if ( createOverviews() )
      {
         // Skip shape files...
         if ( ih->getClassName() != "rspfOgrGdalTileSource" )
         {
            createOverview(ih, consumedHistogramOptions, consumedCmmOptionsOptions);
         }
      }

      // Build stand alone histogram.  Note the overview sequencer may have computed for us.
      if ( hasHistogramOption() && !consumedHistogramOptions)
      {
         createHistogram( ih );
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN) << M << "\nCould not open: " << file << std::endl;
   }
   
   if(traceDebug())
   {
      // Since rspfFileWalker is threaded output the file so we know which job exited.
      rspfNotify(rspfNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   }
}

// Create overview for image:
void rspfImageUtil::createOverview(rspfRefPtr<rspfImageHandler>& ih,
                                    bool& consumedHistogramOptions,
                                    bool& /* consumedCmmOptions */)
{
   static const char M[] = "rspfImageUtil::createOverview #1";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }
    
   if ( ih.valid() )
   {
      // Get the entry list:
      std::vector<rspf_uint32> entryList;
      ih->getEntryList(entryList);
      
      bool useEntryIndex = false;
      if ( entryList.size() )
      {
         if ( (entryList.size() > 1) || (entryList[0] != 0) ) useEntryIndex = true;
      }

      // Create the overview builder:
      rspfString overviewType;
      getOverviewType( overviewType.string() );
      rspfRefPtr<rspfOverviewBuilderBase> ob =
         rspfOverviewBuilderFactoryRegistry::instance()->createBuilder(overviewType);
      if ( ob.valid() )
      {
         // Set up any overview builder options that don't involve histograms.
         rspfPropertyInterface* pi = dynamic_cast<rspfPropertyInterface*>( ob.get() );
         if ( pi ) setProps(pi);
         
         rspf_uint32 stopDimension = getOverviewStopDimension();
         if ( stopDimension ) ob->setOverviewStopDimension(stopDimension);

         ob->setScanForMinMax( scanForMinMax() );
         
         ob->setScanForMinMaxNull( scanForMinMaxNull() );
         
         for(rspf_uint32 idx = 0; idx < entryList.size(); ++idx)
         {
            createOverview(ih, ob, entryList[idx], useEntryIndex, consumedHistogramOptions);
         }
      }
      else
      {
        rspfNotify(rspfNotifyLevel_WARN)
           << "ERROR:\nCould not create builder for:  "<< overviewType << std::endl;
        outputOverviewWriterTypes();
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

// Create overview for entry:
void rspfImageUtil::createOverview(rspfRefPtr<rspfImageHandler>& ih,
                                    rspfRefPtr<rspfOverviewBuilderBase>& ob,
                                    rspf_uint32 entry,
                                    bool useEntryIndex,
                                    bool& consumedHistogramOptions)
{
   static const char M[] = "rspfImageUtil::createOverview #2";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }
   
   if ( ih.valid() && ob.valid() )
   {
      rspfFilename outputFile =
         ih->getFilenameWithThisExtension(rspfString(".ovr"), useEntryIndex);
      
      if ( rebuildOverviews() )
      {
         ih->closeOverview(); 
         if ( outputFile.exists() )
         {
            outputFile.remove();
         }
      }

      if ( getInternalOverviewsFlag() )
      {
         if ( ih->getClassName() == "rspfTiffTileSource")
         {
            //---
            // INTERNAL_OVERVIEWS_FLAG_KW is set to true:
            // Tiff reader can handle internal overviews.  Set the output file to
            // input file.  Do it after the above remove so that if there were
            // external overviews they will get removed.
            //---
            outputFile = ih->getFilename();
         }
         else 
         {
            rspfNotify(rspfNotifyLevel_NOTICE)
               << "Internal overviews not supported for reader type: "
               <<ih->getClassName()
               << "\nIgnoring option..."
               << endl;
         }
      }
         
      if (useEntryIndex)
      {
         ih->setCurrentEntry(entry);
         
         rspfNotify(rspfNotifyLevel_NOTICE) << "entry number: "<< entry << std::endl;
      }

      if ( hasRequiredOverview( ih, ob ) == false )
      {
         //---
         // Set create histogram code...
         //
         // Notes:
         // 1) Must put this logic after any removal of external overview file.
         // 
         // 2) Base file could have built in overviews, e.g. jp2 files.  So the sequensor could
         //    start at R6 even if there is no external overview file.
         //
         // 3) If user want the histogram from R0 the overview builder can do as long as
         //    rspfImageHandler::getNumberOfDecimationLevels returns 1.  If we are starting
         //    overview building at R6 then we must do the create histogram in a separate path.
         //---
         rspfHistogramMode histoMode = RSPF_HISTO_MODE_UNKNOWN;
         if ( createHistogram() ||
              ( createHistogramR0() && ( ih->getNumberOfDecimationLevels() == 1 ) ) )
         {
            histoMode = RSPF_HISTO_MODE_NORMAL;
         }
         else if ( createHistogramFast() )
         {
            histoMode = RSPF_HISTO_MODE_FAST;
         }
         
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "Histogram mode: " << histoMode << "\n";
         }
         
         if ( histoMode != RSPF_HISTO_MODE_UNKNOWN )
         {
            consumedHistogramOptions = true;
            ob->setHistogramMode(histoMode);
            
            rspfNotify(rspfNotifyLevel_NOTICE)
               << "Creating overviews with histogram for file: " << ih->getFilename() << std::endl;
         }
         else
         {
            if ( histoMode != RSPF_HISTO_MODE_UNKNOWN )
            {
               consumedHistogramOptions = false;  
               rspfNotify(rspfNotifyLevel_NOTICE)
                  << "Creating overviews for file: " << ih->getFilename() << std::endl;
            }
         }
         
         ob->setOutputFile(outputFile);
         ob->setInputSource(ih.get());
         
         // Create the overview for this entry in this file:
         if ( ob->execute() == false )
         {
            setErrorStatus( rspfErrorCodes::RSPF_ERROR );
            rspfNotify(rspfNotifyLevel_WARN)
               << "Error returned creating overviews for file: " << ih->getFilename() << std::endl;
         }
      }
      else
      {
         consumedHistogramOptions = false;
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "Image has required reduced resolution data sets." << std::endl;
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

bool rspfImageUtil::hasRequiredOverview( rspfRefPtr<rspfImageHandler>& ih,
                                          rspfRefPtr<rspfOverviewBuilderBase>& ob )
{
   bool result = false;
   if ( ih.valid() && ob.valid() && ( getCopyAllFlag() == false ) )
   {
      if ( ih->getClassName() == "rspfCcfTileSource" )
      {
         // CCF reader does not use external overviews.
         result = true;
      }
      else
      {
         // Note we always have one rset
         rspf_uint32 required = 1;
         
         rspf_uint32 startingResLevel      = ih->getNumberOfDecimationLevels();
         rspf_uint32 overviewStopDimension = ob->getOverviewStopDimension();
         
         rspf_uint32 largestImageDimension =
            ih->getNumberOfSamples(0) >
            ih->getNumberOfLines(0) ?
            ih->getNumberOfSamples(0) :
            ih->getNumberOfLines(0);
         
         while(largestImageDimension > overviewStopDimension)
         {
            largestImageDimension /= 2;
            ++required;
         }
         
         if ( startingResLevel >= required )
         {
            result = true;
         }
      }
   }
   return result;
}

void rspfImageUtil::createHistogram(rspfRefPtr<rspfImageHandler>& ih)
{
   static const char M[] = "rspfImageUtil::createHistogram #1";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }
   
   if ( ih.valid() )
   {
      // Get the entry list:
      std::vector<rspf_uint32> entryList;
      ih->getEntryList(entryList);
      
      bool useEntryIndex = false;
      if ( entryList.size() )
      {
         if ( (entryList.size() > 1) || (entryList[0] != 0) ) useEntryIndex = true;
      }

      for(rspf_uint32 idx = 0; idx < entryList.size(); ++idx)
      {
         createHistogram(ih, entryList[idx], useEntryIndex);
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

// Create histogram for entry:
void rspfImageUtil::createHistogram(rspfRefPtr<rspfImageHandler>& ih,
                                     rspf_uint32 entry,
                                     bool useEntryIndex)
{
   static const char M[] = "rspfImageUtil::createHistogram #2";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }
   
   if ( ih.valid() )
   {
      if (useEntryIndex)
      {
         ih->setCurrentEntry(entry);
         rspfNotify(rspfNotifyLevel_NOTICE) << "entry number: "<< entry << std::endl;
      }

      rspfFilename outputFile =
         ih->getFilenameWithThisExtension(rspfString(".his"), useEntryIndex);

      // Only build if needed:
      if ( (outputFile.exists() == false) || rebuildHistogram() )
      {
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "Computing histogram for file: " << ih->getFilename() << std::endl;

         // Check handler to see if it's filtering bands.
         std::vector<rspf_uint32> originalBandList(0);
         if ( ih->isBandSelector() )
         { 
            // Capture for finalize method.
            ih->getOutputBandList( originalBandList );
            
            // Set output list to input.
            ih->setOutputToInputBandList();
         }

         rspfRefPtr<rspfImageHistogramSource> histoSource = new rspfImageHistogramSource;
         rspfRefPtr<rspfHistogramWriter> writer = new rspfHistogramWriter;
         
         histoSource->setMaxNumberOfRLevels(1); // Currently hard coded...
         
#if 0 /* TODO tmp drb */
         if( !rspf::isnan(histoMin) )
         {
            histoSource->setMinValueOverride(histoMin);
         }
         
         if( !rspf::isnan(histoMax) )
         {
            histoSource->setMaxValueOverride(histoMax);
         }
         
         if(histoBins > 0)
         {
            histoSource->setNumberOfBinsOverride(histoBins);
         }
#endif
         
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "Histogram mode: " << getHistogramMode() << "\n";
         }
         
         // Connect histogram source to image handler.
         histoSource->setComputationMode( getHistogramMode() );
         histoSource->connectMyInputTo(0, ih.get() );
         histoSource->enableSource();
         
         // Connect writer to histogram source.
         writer->connectMyInputTo(0, histoSource.get());
         writer->setFilename(outputFile);
         theStdOutProgress.setFlushStreamFlag(true);
         writer->addListener(&theStdOutProgress);
         
         // Compute...
         writer->execute();
         
         writer=0;

         // Reset the band list.
         if ( ih->isBandSelector() && originalBandList.size() )
         {
            ih->setOutputBandList( originalBandList );
         }

      } // Matches: if ( (outputFile.exists() == false) || rebuildHistogram() )
      
   } // Matches: if ( ih.valid() )

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exited...\n";
   }

} // End: rspfImageUtil::createHistogram #2

void rspfImageUtil::usage(rspfArgumentParser& ap)
{
   // Add global usage options.
   rspfInit::instance()->addOptions(ap);
   
   // Set app name.
   ap.getApplicationUsage()->setApplicationName(ap.getApplicationName());

   // Add options.
   addArguments(ap);
   
   // Write usage.
   ap.getApplicationUsage()->write(rspfNotify(rspfNotifyLevel_INFO));

   outputOverviewWriterTypes();
   
   rspfNotify(rspfNotifyLevel_INFO)
      << "\nExample commands:\n"
      << "\n// A single image standard tiff overviews, histogram:\n"
      << "rspf-preproc -o --ch <file>\n"
      << "\n// A single image with j2k overviews(requires kakadu plugin), histogram:\n"
      << "rspf-preproc --ot rspf_kakadu_nitf_j2k --ch <file>\n"
      << "\n// j2k, histogram, 4 threads\n"
      << "\n// standard tiff overviews, full histogram, 4 threads:\n"
      << "rspf-preproc -r -o --ch --threads 4 <directory_to_walk>\n"
      << "\n// j2k, histogram (fast mode), 4 threads\n"
      << "rspf-preproc -r --ot rspf_kakadu_nitf_j2k --chf --threads 4 "
      << "<directory_to_walk>\n"
      << "\n// tiff, jpeg compression, histogram, 4 threads\n"
      << "rspf-preproc -r --ch --compression-quality 75 --compression-type "
      << "jpeg --threads 4 <directory_to_walk>\n"
      << "\nNOTES:\n"
      << "\n  --ch  equals --create-histogram"
      << "\n  --chf equals --create-histogram-fast"
      << std::endl;
}

// Private method:
void rspfImageUtil::outputOverviewWriterTypes() const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "\nValid overview types: " << std::endl;
   
   std::vector<rspfString> outputType;
   
   rspfOverviewBuilderFactoryRegistry::instance()->getTypeNameList(outputType);
   std::copy(outputType.begin(),
             outputType.end(),
             std::ostream_iterator<rspfString>(rspfNotify(rspfNotifyLevel_NOTICE), "\t\n"));
}

// Private method:
bool rspfImageUtil::isDirectoryBasedImage(const rspfImageHandler* ih) const
{
   bool result = false;
   if ( ih )
   {
      // Get the image handler name.
      rspfString imageHandlerName = ih->getClassName();
      if ( (imageHandlerName == "rspfAdrgTileSource") ||
           (imageHandlerName == "rspfCibCadrgTileSource") )  
      {
         result = true;
      }
   }
   return result;
}

void rspfImageUtil::setCreateOverviewsFlag( bool flag )
{
   addOption( CREATE_OVERVIEWS_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool rspfImageUtil::createOverviews() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( CREATE_OVERVIEWS_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setRebuildOverviewsFlag( bool flag )
{
   addOption( REBUILD_OVERVIEWS_KW, ( flag ? TRUE_KW : FALSE_KW ) );
   if ( flag )
   {
      setCreateOverviewsFlag( true ); // Turn on overview building.
   }
}

bool rspfImageUtil::rebuildOverviews() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( REBUILD_OVERVIEWS_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setRebuildHistogramFlag( bool flag )
{
   addOption( REBUILD_HISTOGRAM_KW, ( flag ? TRUE_KW : FALSE_KW ) );
   if ( flag )
   {
      setCreateHistogramFlag( true ); // Turn on histogram building.
   }
}

bool rspfImageUtil::rebuildHistogram() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( REBUILD_HISTOGRAM_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setScanForMinMax( bool flag )
{
   addOption( SCAN_MIN_MAX_KW, ( flag ? TRUE_KW : FALSE_KW ) ); 
}

bool rspfImageUtil::scanForMinMax() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( SCAN_MIN_MAX_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setScanForMinMaxNull( bool flag )
{
   addOption( SCAN_MIN_MAX_NULL_KW, ( flag ? TRUE_KW : FALSE_KW ) ); 
}

bool rspfImageUtil::scanForMinMaxNull() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( SCAN_MIN_MAX_NULL_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setCompressionQuality( const std::string& quality )
{
   if ( quality.size() )
   {
      std::string key = WRITER_PROP_KW;
      key += rspfString::toString( getNextWriterPropIndex() ).string();
      std::string value = rspfKeywordNames::COMPRESSION_QUALITY_KW;
      value += "=";
      value += quality;
      addOption( key, value );
   }
}

void rspfImageUtil::setCompressionType(const std::string& type)
{
   if ( type.size() )
   {
      std::string key = WRITER_PROP_KW;
      key += rspfString::toString( getNextWriterPropIndex() ).string();
      std::string value = rspfKeywordNames::COMPRESSION_TYPE_KW;
      value += "=";
      value += type;
      addOption( key, value );
   }   
}

void rspfImageUtil::setCopyAllFlag( bool flag )
{
   // Add this for hasRequiredOverview method.
   std::string key   = COPY_ALL_FLAG_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );

   // Add as a writer prop:
   key = WRITER_PROP_KW;
   key += rspfString::toString( getNextWriterPropIndex() ).string();
   value = COPY_ALL_FLAG_KW;
   value += "=";
   value += ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool rspfImageUtil::getCopyAllFlag() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( COPY_ALL_FLAG_KW );
   if ( lookup.size() )
   {
      rspfString os(lookup);
      result = os.toBool();
   }
   return result;
}

void rspfImageUtil::setInternalOverviewsFlag( bool flag )
{
   // Add this for hasRequiredOverview method.
   std::string key   = INTERNAL_OVERVIEWS_FLAG_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );

   // Add as a writer prop:
   key = WRITER_PROP_KW;
   key += rspfString::toString( getNextWriterPropIndex() ).string();
   value = INTERNAL_OVERVIEWS_FLAG_KW;
   value += "=";
   value += ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool rspfImageUtil::getInternalOverviewsFlag() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( INTERNAL_OVERVIEWS_FLAG_KW );
   if ( lookup.size() )
   {
      rspfString os(lookup);
      result = os.toBool();
   }
   return result;
}

void rspfImageUtil::setOutputFileNamesFlag( bool flag )
{
   std::string key   = OUTPUT_FILENAMES_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool rspfImageUtil::getOutputFileNamesFlag() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( OUTPUT_FILENAMES_KW );
   if ( lookup.size() )
   {
      rspfString os(lookup);
      result = os.toBool();
   }
   return result;
}

void rspfImageUtil::setOutputDirectory( const std::string& directory )
{
   std::string key = OUTPUT_DIRECTORY_KW;
   addOption( key, directory );
}
   
void rspfImageUtil::setOverviewType( const std::string& type )
{
   std::string key = OVERVIEW_TYPE_KW;
   addOption( key, type );
   setCreateOverviewsFlag( true ); // Assume caller wants overviews.
}

void rspfImageUtil::getOverviewType(std::string& type) const
{
   std::string lookup = m_kwl->findKey(OVERVIEW_TYPE_KW);
   if ( lookup.size() )
   {
      type = lookup;
   }
   else
   {
      type = "rspf_tiff_box"; // default
   }
}

void rspfImageUtil::setProps(rspfPropertyInterface* pi) const
{
   if ( pi )
   {
      rspfString baseKey;
      if ( dynamic_cast<rspfImageHandler*>(pi) )
      {
         baseKey = READER_PROP_KW;
      }
      else
      {
         baseKey = WRITER_PROP_KW;
      }
      
      rspf_uint32 propCount = m_kwl->numberOf( baseKey.c_str() );
      if ( propCount )
      {
         rspf_uint32 foundProps = 0;
         rspf_uint32 index = 0;
         
         // (propCount+100) is to allow for holes like reader_prop0, reader_prop2...
         while ( (foundProps < propCount) && (index < (propCount+100) ) ) 
         {
            rspfString key = baseKey;
            key += rspfString::toString(index);
            std::string lookup = m_kwl->findKey( key.string() );
            if ( lookup.size() )
            {
               rspfString value = lookup;
               std::vector<rspfString> v = value.split("=");
               if (v.size() == 2)
               {
                  rspfString propertyName  = v[0];
                  rspfString propertyValue = v[1];
                  rspfRefPtr<rspfProperty> p =
                     new rspfStringProperty(propertyName, propertyValue);
                  pi->setProperty( p );
               }
               ++foundProps;
            }
            ++index;
         }
      }
   }
}

void rspfImageUtil::setOverviewStopDimension( rspf_uint32 dimension )
{
   addOption( OVERVIEW_STOP_DIM_KW, dimension );
}

void rspfImageUtil::setOverviewStopDimension( const std::string& dimension )
{
   addOption( OVERVIEW_STOP_DIM_KW, dimension );
}

void rspfImageUtil::setTileSize( rspf_uint32 tileSize )
{
   if ((tileSize % 16) == 0)
   {
      addOption( TILE_SIZE_KW, tileSize );
   }
   else
   {
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "rspfImageUtil::setTileSize NOTICE:"
         << "\nTile width must be a multiple of 16!"
         << std::endl;
   }
}


rspf_uint32 rspfImageUtil::getOverviewStopDimension() const
{
   rspf_uint32 result = 0;
   std::string lookup = m_kwl->findKey( OVERVIEW_STOP_DIM_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toUInt32();
   }
   return result;
}

void rspfImageUtil::setCreateHistogramFlag( bool flag )
{
   addOption( CREATE_HISTOGRAM_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool rspfImageUtil::createHistogram() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( CREATE_HISTOGRAM_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setCreateHistogramFastFlag( bool flag )
{
   addOption( CREATE_HISTOGRAM_FAST_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool rspfImageUtil::createHistogramFast() const
{
   bool result = false;
   std::string lookup = m_kwl->findKey( CREATE_HISTOGRAM_FAST_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

void rspfImageUtil::setCreateHistogramR0Flag( bool flag )
{
   addOption( CREATE_HISTOGRAM_R0_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool rspfImageUtil::createHistogramR0() const
{   bool result = false;
   std::string lookup = m_kwl->findKey( CREATE_HISTOGRAM_R0_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toBool();
   }
   return result;
}

bool rspfImageUtil::hasHistogramOption() const
{
   return ( createHistogram() || createHistogramFast() || createHistogramR0() );
}

rspfHistogramMode rspfImageUtil::getHistogramMode() const
{
   rspfHistogramMode result = RSPF_HISTO_MODE_UNKNOWN;
   if ( createHistogram() || createHistogramR0() )
   {
      result = RSPF_HISTO_MODE_NORMAL;
   }
   else if ( createHistogramFast() )
   {
      result = RSPF_HISTO_MODE_FAST;
   }
   return result;
}

void rspfImageUtil::setNumberOfThreads( rspf_uint32 threads )
{
   addOption( THREADS_KW, threads );
}

void rspfImageUtil::setNumberOfThreads( const std::string& threads )
{
   addOption( THREADS_KW, threads );
}

rspf_uint32 rspfImageUtil::getNumberOfThreads() const
{
   rspf_uint32 result;
   std::string lookup = m_kwl->findKey( THREADS_KW );
   if ( lookup.size() )
   {
      result = rspfString(lookup).toUInt32();
   }
   else
   {
      result = rspf::getNumberOfThreads();
   }
   return result;
}

rspf_uint32 rspfImageUtil::getNextWriterPropIndex() const
{
   rspf_uint32 result = m_kwl->numberOf( WRITER_PROP_KW.c_str() );
   if ( result )
   {
      rspf_uint32 foundProps = 0;
      rspf_uint32 index = 0;

      //---
      // Loop until we find the last index used for WRITER_PROP_KW.
      // (result+100) is to allow for holes like writer_prop0, writer_prop2...
      //---
      while ( (foundProps < result) && (index < (result+100) ) ) 
      {
         rspfString key = WRITER_PROP_KW;
         key += rspfString::toString(index);
         std::string lookup = m_kwl->findKey( key.string() );
         if ( lookup.size() )
         {
            ++foundProps;
         }
         ++index;
      }
      result = index;
   }
   return result;
}

rspf_uint32 rspfImageUtil::getNextReaderPropIndex() const
{
   rspf_uint32 result = m_kwl->numberOf( READER_PROP_KW.c_str() );
   if ( result )
   {
      rspf_uint32 foundProps = 0;
      rspf_uint32 index = 0;
      
      //---
      // Loop until we find the last index used for WRITER_PROP_KW.
      // (result+100) is to allow for holes like reader_prop0, reader_prop2...
      //---
      while ( (foundProps < result) && (index < (result+100) ) ) 
      {
         rspfString key = READER_PROP_KW;
         key += rspfString::toString(index);
         std::string lookup = m_kwl->findKey( key.string() );
         if ( lookup.size() )
         {
            ++foundProps;
         }
         ++index;
      }
      result = index;
   }
   return result;
}

void rspfImageUtil::addOption( const std::string& key, rspf_uint32 value )
{
   addOption( key, rspfString::toString( value ).string() );
}

void rspfImageUtil::addOption(  const std::string& key, const std::string& value )
{
   m_mutex.lock();
   if ( m_kwl.valid() )
   {
      if ( key.size() && value.size() )
      {
         m_kwl->addPair( key, value );
      }
   }
   m_mutex.unlock();
}

void rspfImageUtil::setErrorStatus( rspf_int32 status )
{
   m_mutex.lock();
   m_errorStatus = status;
   m_mutex.unlock();
}
