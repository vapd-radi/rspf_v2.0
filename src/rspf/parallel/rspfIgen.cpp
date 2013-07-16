//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: implementation for image generator
//
//*************************************************************************
// $Id: rspfIgen.cpp 21850 2012-10-21 20:09:55Z dburken $

#include <rspf/rspfConfig.h> /* To pick up define RSPF_HAS_MPI. */

#if RSPF_HAS_MPI
#  include <mpi.h>
#  include <rspf/parallel/rspfImageMpiMWriterSequenceConnection.h>
#  include <rspf/parallel/rspfImageMpiSWriterSequenceConnection.h>
#endif

#include <rspf/parallel/rspfIgen.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfRectangleCutFilter.h>
#include <rspf/imaging/rspfGeoPolyCutter.h>
#include <rspf/imaging/rspfTiffWriter.h>
#include <rspf/imaging/rspfTilingRect.h>
#include <rspf/imaging/rspfTilingPoly.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/parallel/rspfMpi.h>
#include <rspf/parallel/rspfMultiThreadSequencer.h>
#include <rspf/parallel/rspfMtDebug.h> //### For debug/performance eval
#include <iterator>
#include <sstream>

static rspfTrace traceDebug(rspfString("rspfIgen:debug"));
static rspfTrace traceLog(rspfString("rspfIgen:log"));

rspfIgen::rspfIgen()
:
theContainer(new rspfConnectableContainer()),
theProductProjection(0),
theProductChain(0),
theTiling(new rspfTiling),
theOutputRect(),
theBuildThumbnailFlag(false),
theThumbnailSize(0, 0),
theNumberOfTilesToBuffer(2),
theKwl(),
theTilingEnabled(false),
theProgressFlag(true),
theStdoutFlag(false),
theThreadCount(9999) // Default no threading
{
   theOutputRect.makeNan();
}

rspfIgen::~rspfIgen()
{
   theProductProjection = 0;
   theTiling = 0;
   theContainer->disconnect();
   theContainer->deleteAllChildren();
   theContainer = 0;
}

void rspfIgen::initializeAttributes()
{
   theBuildThumbnailFlag  = false;
   theThumbnailSize = rspfIpt(0,0);
   theTilingEnabled = false;

   if(rspfMpi::instance()->getRank() != 0)
   {
      rspfPreferences::instance()->addPreferences(theKwl,
         "preferences.",
         true);
   }

   const char* lookup = theKwl.find("igen.output_progress");
   if (lookup)
   {
      rspfString os = lookup;
      theProgressFlag = os.toBool();
   }

   const char* thumbnailStr = theKwl.find("igen.thumbnail");
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "Thumbnail string = " << rspfString(thumbnailStr) << std::endl;
   }
   if(thumbnailStr)
   {
      theBuildThumbnailFlag= rspfString(thumbnailStr).toBool();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "Generate thumbnail attribute is set to "
            << theBuildThumbnailFlag << std::endl;
      }
      if(theBuildThumbnailFlag)
      {
         const char* resStr = theKwl.find("igen.thumbnail_res");
         if(resStr)
         {
            theThumbnailSize = rspfIpt(0,0);
            std::istringstream in(resStr);
            rspfString x,y;

            in >> x >> y;

            rspf_int32 ix = x.toInt32();
            rspf_int32 iy = y.toInt32();

            if (ix > 0)
            {
               theThumbnailSize.x = ix;
            }
            else
            {
               theThumbnailSize.x = 128;
            }

            if (iy > 0)
            {
               theThumbnailSize.y = iy;
            }
            else
            {
               theThumbnailSize.y = theThumbnailSize.x;
            }
         }
         else
         {
            theThumbnailSize = rspfIpt(128, 128);
         }
      }
   }
   const char* numberOfSlaveTileBuffersStr = theKwl.find("igen.slave_tile_buffers");
   if(numberOfSlaveTileBuffersStr)
   {
      theNumberOfTilesToBuffer = rspfString(numberOfSlaveTileBuffersStr).toLong();
   }

   const char* tilingKw = theKwl.find("igen.tiling.type");
   if(tilingKw)
   {
      theTilingEnabled = true;
      if(!theTiling->loadState(theKwl, "igen.tiling."))
      {
         theTilingEnabled = false;
      }
   }
}

void rspfIgen::slaveSetup()
{
#if RSPF_HAS_MPI
   int stringSize;
   MPI_Status status;
   int numberOfTimes = 0;

   memset((void *)&status, 0, sizeof(MPI_Status));


   // we first need to receive the size of the keyword list to load
   MPI_Recv(&stringSize,
      1,
      MPI_INT,
      0,    // source
      0,    // tag
      MPI_COMM_WORLD,
      &status);

   if(status.MPI_ERROR != MPI_SUCCESS)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "SLAVE = "
         << rspfMpi::instance()->getRank()
         << "Had errors receiving!!!!" << std::endl;
      return;
   }

   char* buf = new char[stringSize+1];

   numberOfTimes = 0;

   memset((void *)&status, 0, sizeof(MPI_Status));

   // now lets get the keywordlist as a string so we can load it up
   MPI_Recv(buf,
      stringSize,
      MPI_CHAR,
      0, // source
      0, // tag
      MPI_COMM_WORLD,
      &status);

   if(status.MPI_ERROR != MPI_SUCCESS)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "SLAVE = " << rspfMpi::instance()->getRank()
         << "Had errors receiving in rspfIgen::slaveCreate(!!!!"
         << std::endl;
      return;
   }
   buf[stringSize] = '\0';

   if(status.MPI_ERROR != MPI_SUCCESS)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "SLAVE = " << rspfMpi::instance()->getRank()
         << "Had errors receiving!!!!" << std::endl;
      return;
   }

   // now lets convert the received keywordlist into an actual
   // rspfKeywordlist by using the parsStream method.
   std::ostringstream kwlStream;

   kwlStream << buf << ends;

   istringstream kwlInStream(kwlStream.str());
   theKwl.clear();
   theKwl.parseStream(kwlInStream);
   initializeAttributes();
   delete [] buf;
   buf = 0;

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "****************** KWL ************************" << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << theKwl << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "**************** END KWL ************************" << std::endl;

   }
   loadProductSpec();
#endif
}

void rspfIgen::initialize(const rspfKeywordlist& kwl)
{
#if RSPF_HAS_MPI
   if(rspfMpi::instance()->getNumberOfProcessors() > 0)
   {
      if(rspfMpi::instance()->getRank() != 0)
      {
         slaveSetup();
         return;
      }
   }
#endif
   theKwl = kwl;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "The igen kewyord list  ==== \n" << theKwl << std::endl;
   }
   rspfKeywordlist kwlPrefs = rspfPreferences::instance()->preferencesKWL();

   kwlPrefs.addPrefixToAll("preferences.");
   theKwl.add(kwlPrefs);

   initializeAttributes();

   // now stream it to all slave processors
   //
#if RSPF_HAS_MPI
   if(rspfMpi::instance()->getNumberOfProcessors() > 0)
   {
      std::ostringstream outputKeywordlist;

      theKwl.writeToStream(outputKeywordlist);
      rspfString kwlString = outputKeywordlist.str();
      rspf_uint32 size = kwlString.size();

      for(long processor = 1;
         processor < rspfMpi::instance()->getNumberOfProcessors();
         ++processor)
      {
         // let's send the keywordlist argument.
         // This is two steps.  We send a message to
         // indicate the size and then we send the
         // string.
         //
         MPI_Send(&size,
            1,
            MPI_INT,
            processor,
            0,
            MPI_COMM_WORLD);

         MPI_Send((void*)kwlString.c_str(),
            size,
            MPI_CHAR,
            processor,
            0,
            MPI_COMM_WORLD);
      }
   }
#endif
   loadProductSpec();

}

bool rspfIgen::loadProductSpec()
{
   const char* MODULE = "rspfIgen::loadProductSpec";
   if(traceDebug())  CLOG << "entered..." << std::endl;

   // Clear out the overall container and initialize it with spec in KWL:
   theContainer->deleteAllChildren();
   theContainer->loadState(theKwl);

   // There should be a product chain defined in the container:
   // rspfConnectableObject* obj = 
   //   theContainer->findFirstObjectOfType(STATIC_TYPE_NAME(rspfImageChain), false);
   // theProductChain = PTR_CAST(rspfImageChain, obj);

   rspfTypeNameVisitor visitor( rspfString("rspfImageChain"),
                                 true, // firstofTypeFlag
                                 (rspfVisitor::VISIT_INPUTS |
                                  rspfVisitor::VISIT_CHILDREN) );
   theContainer->accept( visitor );
   theProductChain = visitor.getObjectAs<rspfImageChain>(0);
   
   if (!theProductChain.valid())
   {
      // Search for a connectable container specified that contains the entire product chain:
      // rspfConnectableObject* obj2 = 
      //    theContainer->findFirstObjectOfType(STATIC_TYPE_NAME(rspfImageFileWriter), true);
      // rspfImageFileWriter* writer = PTR_CAST(rspfImageFileWriter, obj2);
      visitor.reset();
      visitor.setTypeName( rspfString( "rspfImageFileWriter" ) );
      theContainer->accept( visitor );
      rspfRefPtr<rspfImageFileWriter> writer = visitor.getObjectAs<rspfImageFileWriter>(0);
      if ( writer.valid() )
      {
         theProductChain = dynamic_cast<rspfImageChain*>( writer->getInput() );
      }

      if (!theProductChain.valid())
      {
         rspfNotify(rspfNotifyLevel_FATAL) << MODULE 
            << " -- No processing chain defined for generating product." << std::endl;
         return false; 
      }
   }

   // The output projection is specified separately in the KWL:
   rspfString prefix = "product.projection.";
   theProductProjection = dynamic_cast<rspfMapProjection*>( 
      rspfProjectionFactoryRegistry::instance()->createProjection(theKwl, prefix) );

   const char* lookup = theKwl.find("igen.write_to_stdout");
   if (lookup && rspfString(lookup).toBool())
   {
      theStdoutFlag = true;
   }

   return true;
}

//*************************************************************************************************
//! Writes the output product image. Throws an rspfException if error encountered.
//*************************************************************************************************
void rspfIgen::outputProduct()
{
   // Verify that all vitals have been initialized:
   if (!theProductChain.valid())
   {
      std::string err = "rspfIgen::outputProduct() ERROR:  No product processing chain has yet"
         " been established. Nothing to output!";
      throw(rspfException(err));
   }
   if (!theProductProjection.valid())
   {
      std::string err = "rspfIgen::outputProduct() ERROR:  No product projection has yet"
         " been established. Nothing to output!";
      throw(rspfException(err));
   }

   // Update the chain with the product view specified:
   setView();
   initializeChain();

   // if it's a thumbnail then adjust the GSD and reset the view proj to the chain.
   if(theBuildThumbnailFlag)
      initThumbnailProjection();

   rspfRefPtr<rspfImageSourceSequencer> sequencer = 0;

#if RSPF_HAS_MPI
   // only allocate the slave connection if the number of processors is larger than 1
   if(rspfMpi::instance()->getNumberOfProcessors() > 1)
   {
      if(rspfMpi::instance()->getRank()!=0)
         sequencer = new rspfImageMpiSWriterSequenceConnection(0, theNumberOfTilesToBuffer);
      else
         sequencer = new rspfImageMpiMWriterSequenceConnection();
   }
#endif

   // we will just load a serial connection if MPI is not supported.
   // Threading?
   if (!sequencer.valid() && (theThreadCount != 9999))
      sequencer = new rspfMultiThreadSequencer(0, theThreadCount);

   if (!sequencer.valid())
      sequencer = new rspfImageSourceSequencer(0);


   // Look for the first writer (should be the only writer) in our list of objects:

   // rspfRefPtr<rspfImageFileWriter> writer  = 0;
   // rspfConnectableObject::ConnectableObjectList imageWriters =
   //    theContainer->findAllObjectsOfType(STATIC_TYPE_INFO(rspfImageFileWriter), false);

   rspfTypeNameVisitor visitor( rspfString("rspfImageFileWriter"),
                                 true, // firstofTypeFlag
                                 (rspfVisitor::VISIT_INPUTS |
                                  rspfVisitor::VISIT_CHILDREN) );
   theContainer->accept( visitor );
   rspfRefPtr<rspfImageFileWriter> writer = visitor.getObjectAs<rspfImageFileWriter>(0);
   
   if ( !writer.valid() )
   {
      sequencer = 0;
      std::string err = "rspfIgen::outputProduct() ERROR:  No image writer object was found in "
         " processing chain.";
      throw(rspfException(err));
   }

   // writer = PTR_CAST(rspfImageFileWriter, imageWriters[0].get());
   writer->changeSequencer(sequencer.get());
   writer->connectMyInputTo(theProductChain.get());
   
   // Check for writing to standard output flag. Not all writers support this so check and 
   // throw an error if not supported.
   if (theStdoutFlag)
   {
      if ( writer->setOutputStream(std::cout) == false )
      {
         std::string err = "ERROR:  The write to standard out flag is set; however, writer does "
            "not support going to standard out. Bummer...";
         throw(rspfException(err));
      }
   }
   
   writer->initialize();

   if ( theBuildThumbnailFlag )
   {
      //---
      // Use theOutputRect as it has been clamped to be within the requested thumbnail size.
      // 
      // Relying of the bounding rectangle of the scaled product chain has given us off by
      // one rectangles, i.e., a width of 513 instead of 512.
      // 
      // NOTE: This must be called after the writer->initialize() as
      // rspfImageFileWriter::initialize incorrectly resets theAreaOfInterest
      // back to the bounding rect.
      //---
      writer->setAreaOfInterest( rspfIrect(theOutputRect) );
   }

   // If multi-file tiled output is not desired perform simple output, handle special:
   if(theTilingEnabled && theProductProjection.valid())
   {
      theTiling->initialize(*(theProductProjection.get()), theOutputRect);

      rspfRectangleCutFilter* cut = NULL;
      rspfTilingPoly* tilingPoly = dynamic_cast<rspfTilingPoly*>( theTiling.get() );
      if (tilingPoly == NULL)
      {
         cut = new rspfRectangleCutFilter;
         theProductChain->addFirst(cut);
      }
      
      rspfFilename tempFile = writer->getFilename();
      if(!tempFile.isDir())
         tempFile = tempFile.path();

      rspfString tileName;
      rspfIrect clipRect;

      // 'next' method modifies the mapProj which is the same instance as theProductProjection,
      // so this data member is modified here, then later accessed by setView:
      while(theTiling->next(theProductProjection, clipRect, tileName))
      {
         if (cut && tilingPoly == NULL)//use rspfTiling or rspfTilingRect
         {
            setView();
            cut->setRectangle(clipRect);
         }
         else //otherwise use rspfTilingPoly
         {
            if (tilingPoly != NULL)
            {
               if (!tilingPoly->isFeatureBoundingIntersect())//if clip rect does not intersect with output rect, do nothing
               {
                  continue;
               }
               if (tilingPoly->useMbr())//if use_mbr flag is set to true, all pixels within the MBR will be preserved
               {
                  if (cut == NULL)
                  {
                      cut = new rspfRectangleCutFilter;
                      theProductChain->addFirst(cut);
                  }
                  setView();
                  cut->setRectangle(clipRect);
               }
               else
               {
                  if ( tilingPoly->hasExteriorCut() )
                  {
                     theProductChain->addFirst( tilingPoly->getExteriorCut().get() );
                  }
                  if ( tilingPoly->hasInteriorCut() )
                  {
                     theProductChain->addFirst( tilingPoly->getInteriorCut().get() );
                  }
               }
            }
         }
         
         initializeChain();
         writer->disconnect();
         writer->connectMyInputTo(theProductChain.get());
         writer->setFilename(tempFile.dirCat(tileName));
         writer->initialize();

         if (!writeToFile(writer.get()))
            break;
      }
   }
   else
   {
      // No multi-file tiling, just conventional write to single file:
      writeToFile(writer.get());
   }

   //########## DEBUG CODE FOR TIMING MULTI THREAD LOCKS ##############
   if (sequencer.valid() && (theThreadCount != 9999))
   {
      rspfMultiThreadSequencer* mts = dynamic_cast<rspfMultiThreadSequencer*>(sequencer.get());
      if (mts != NULL)
      {

         double jgtt = mts->d_jobGetTileT;
         rspf_uint32 num_threads = mts->getNumberOfThreads();
         double jgttpj = jgtt/num_threads;
         cout<<setprecision(3)<<endl;
         cout << "Multi-threading metrics ---"<<endl;
         cout << "   Number of threads:      " << num_threads<< endl;
         cout << "   Max cache used:         "<< mts->d_maxCacheUsed << endl;
         cout << "   Cache emptied count:    "<< rspfString::toString(mts->d_cacheEmptyCount) << endl;
         cout << "   Time waiting on jobs:   "<<mts->d_idleTime2<<" s"<<endl;
         cout << "   Time waiting on cache:  "<<mts->d_idleTime5<<" s"<<endl;
         cout << "   Handler getTile T:      "<<mts->handlerGetTileT()<<" s"<<endl;
         cout << "   Job getTile T:          "<<jgtt<<" s"<<endl;
         cout << "   Average getTile T/job:  "<<jgttpj<<" s\n"<<endl;
      }
   }
   //##################################################################
}

//*************************************************************************************************
//! Consolidates job of actually writing to the output file.
//*************************************************************************************************
bool rspfIgen::writeToFile(rspfImageFileWriter* writer)
{
   rspfStdOutProgress* prog = 0;
   if ( (rspfMpi::instance()->getRank() == 0) && theProgressFlag)
   {
      // Add a listener to master.
      prog = new rspfStdOutProgress(0, true);
      writer->addListener(prog);
   }

   if (traceLog() && (rspfMpi::instance()->getRank() == 0))
   {
      rspfFilename logFile = writer->getFilename();
      logFile.setExtension(rspfString("log"));

      rspfRefPtr<rspfConnectableContainer> container = new rspfConnectableContainer;
      writer->fillContainer(*container.get());
      rspfKeywordlist logKwl;
      container->saveState(logKwl);
      logKwl.write(logFile.c_str());
   }

   try
   {
      writer->execute();
   }

   // Catch internal exceptions:
   catch(const rspfException& e)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "rspfIgen::outputProduct ERROR:\n"
         << "Caught exception!\n"
         << e.what()
         << std::endl;
      return false;
   }
   catch(...)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "rspfIgen::outputProduct ERROR:\n"
         << "Unknown exception caught!\n"
         << std::endl;
      return false;
   }

   if (prog)
   {
      writer->removeListener(prog);
      delete prog;
      prog = 0;
   }

   return true;
}

//*************************************************************************************************
//! Initializes all clients of the view projection to the current product projection.
//*************************************************************************************************
void rspfIgen::setView()
{
   if( theProductChain.valid() && theProductProjection.valid() )
   {
      // Find all view clients in the chain, and notify them of the new view:
#if 0
      rspfConnectableObject::ConnectableObjectList clientList;
      theProductChain->findAllInputsOfType(clientList, STATIC_TYPE_INFO(rspfViewInterface), true, true);
      for(rspf_uint32 i = 0; i < clientList.size();++i)
      {
         rspfViewInterface* viewClient = dynamic_cast<rspfViewInterface*>( clientList[i].get() );
         if (viewClient)
            viewClient->setView(theProductProjection->dup());
      }
#endif
      
      rspfTypeNameVisitor visitor( rspfString("rspfViewInterface"),
                                    false, // firstofTypeFlag
                                    (rspfVisitor::VISIT_INPUTS|
                                     rspfVisitor::VISIT_CHILDREN) );
      theProductChain->accept( visitor );
      for( rspf_uint32 i = 0; i < visitor.getObjects().size(); ++i )
      {
         rspfViewInterface* viewClient = visitor.getObjectAs<rspfViewInterface>( i );
         if (viewClient)
         {
            viewClient->setView( theProductProjection->dup() );
         }
      }

      // Force recompute of bounding rect:
      initializeChain();
   }
}

//*************************************************************************************************
//! Modifies the production chain to output redused-resolution thumbnail image.
//*************************************************************************************************
void rspfIgen::initThumbnailProjection()
{
   double thumb_size = rspf::max(theThumbnailSize.x, theThumbnailSize.y);
   rspfMapProjection* mapProj = dynamic_cast<rspfMapProjection*>(theProductProjection.get());

   if(mapProj && !theOutputRect.hasNans())
   {
      double xScale = theOutputRect.width()  / thumb_size;
      double yScale = theOutputRect.height() / thumb_size;
      double scale = rspf::max(xScale, yScale);
      mapProj->applyScale(rspfDpt(scale, scale), true);
   }

   // Need to change the view in the product chain:
   setView();

   // Clamp output rectangle to thumbnail bounds.
   rspfDpt ul = theOutputRect.ul();
   rspfDpt lr = theOutputRect.lr();
   if ( (lr.x - ul.x + 1) > thumb_size)
   {
      lr.x = ul.x + thumb_size - 1;
   }
   if ( (lr.y - ul.y + 1) > thumb_size )
   {
      lr.y = ul.y + thumb_size - 1;
   }
   theOutputRect = rspfDrect(ul, lr);
}

//*************************************************************************************************
// This method is called after a change is made to the product chain. It recomputes the bounding
// rectangle.
//*************************************************************************************************
void rspfIgen::initializeChain()
{
   // Force initialization of the chain to recompute parameters:
   theProductChain->initialize();
   theOutputRect = theProductChain->getBoundingRect();
   
   if(!theOutputRect.hasNans())
   {
      // Stretch the rectangle out to integer boundaries.
      theOutputRect.stretchOut();

      // Communicate the new product size to the view's geometry object. This is a total HACK that 
      // external code needs to worry about setting this. Something is wrong with this picture 
      // (OLK 02/11)
      rspfImageGeometry* geom = theProductChain->getImageGeometry().get();
      if (geom)
         geom->setImageSize(rspfIpt(theOutputRect.size()));
   }
}
