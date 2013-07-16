
#include <fstream>
#include <iostream> //TBR DEBUG
using namespace std;

#include "rspfTieGenerator.h"
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>

static rspfTrace traceDebug("rspfTieGenerator:debug");

RTTI_DEF2(rspfTieGenerator, "rspfTieGenerator",
          rspfOutputSource, rspfProcessInterface);

rspfTieGenerator::rspfTieGenerator(rspfImageSource* inputSource)
      :
      rspfOutputSource(NULL, // owner
                        1,
                        0,
                        true,
                        true),
      rspfProcessInterface(),
      theAreaOfInterest(),
      theFilename(rspfFilename::NIL),
      theFileStream(),
      theStoreFlag(false)
{
   connectMyInputTo(0, inputSource);
   theAreaOfInterest.makeNan();
}

rspfTieGenerator::~rspfTieGenerator()
{
}

bool rspfTieGenerator::execute()
{
   bool status = true;
   static const char MODULE[] = "rspfTieGenerator::execute";

   if (traceDebug()) CLOG << " Entered..." << endl;
   
   rspfImageSource* src = reinterpret_cast<rspfImageSource*>(getInput(0));
   if (theAreaOfInterest.isNan())
   {
      // ChipMatch will provide correct ROI
      theAreaOfInterest = src->getBoundingRect(0);
   }

   //open stream
   open();

   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_EXECUTING);
   
   //write out projection info
   if (theFilename != rspfFilename::NIL)
   {
      rspfRefPtr<rspfImageGeometry> geom = src->getImageGeometry();
      rspfKeywordlist mpk;
      if(geom.valid())
      {
         geom->saveState(mpk);
      }
      theFileStream<<mpk.toString(); //writeToStream(theFileStream);
   }

   //do the actual work there
   if (status) status = getAllFeatures();  
   
   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_NOT_EXECUTING);

   if (traceDebug()) CLOG << " Exited..." << endl;

   //close stream
   close();

   return status;
}

bool rspfTieGenerator::getAllFeatures()
{
   static const char MODULE[] = "rspfTieGenerator::getAllFeatures";

   if (traceDebug()) CLOG << " Entered..." << endl;

   rspfChipMatch* src = reinterpret_cast<rspfChipMatch*>(getInput(0));
   if (!src)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "WARN rspfTieGenerator::scanForEdges():"
         << "\nInput source is not a rspfImageChip.  Returning..." << std::endl;
      return false;
   }

   // Some constants needed throughout...
   const rspf_int32 TILE_HEIGHT    = src->getTileHeight();
   const rspf_int32 TILE_WIDTH     = src->getTileWidth();
   const rspf_int32 START_LINE = theAreaOfInterest.ul().y;
   const rspf_int32 STOP_LINE  = theAreaOfInterest.lr().y;
   const rspf_int32 START_SAMP = theAreaOfInterest.ul().x;
   const rspf_int32 STOP_SAMP  = theAreaOfInterest.lr().x;

   // For percent complete status.
   rspf_int32 tilerows=(STOP_LINE-START_LINE+TILE_HEIGHT) / TILE_HEIGHT; //ceil : (stop-start+1+size-1)/size
   rspf_int32 tilecols=(STOP_SAMP-START_SAMP+TILE_WIDTH) / TILE_WIDTH;
   double total_tiles = ((double)tilerows)*tilecols;
   double tiles_processed = 0.0;

   // Set the status message to be "scanning source for edges..."
   rspfNotify(rspfNotifyLevel_INFO) << "Getting tie points..." << std::endl;
   
   // Start off with a percent complete at 0...
   setPercentComplete(0.0);

   // loop through all tiles
   // need to use a sequencer for parallelism in the future TBD
   theTiePoints.clear();
   rspf_int32 line=START_LINE;
   rspf_int32 i,j;

   //loong
   //int grid_rows = 10;
   //int grid_cols = 10;
   //int gridsize_row = tilerows / grid_rows;
   //int gridsize_col = tilecols / grid_cols;
   //int row_step = 5;
   //int col_step = 5;
   //int nblock_row = tilerows / row_step;
   //int nblock_col = tilecols / col_step;
   //int total_blocks = nblock_row * nblock_col;
   //for (i=0;(i<nblock_row)&&!needsAborting();++i)
   //{
	  // rspf_int32 samp=START_SAMP;
	  // for (j=0;(j<nblock_col)&&!needsAborting();++j )
	  // {
		 //  // Get the tie points
		 //  //TBC : can we go further than source edges with edge tiles?? TBD
		 //  const vector<rspfTDpt>& tp = src->getFeatures(rspfIrect(rspfIpt(samp, line),rspfIpt(samp+TILE_WIDTH-1,line+TILE_HEIGHT-1)));

		 //  if (theFilename != rspfFilename::NIL)
		 //  {
			//   //write on stream
			//   writeTiePoints(tp);
		 //  }

		 //  if (getStoreFlag())
		 //  {
			//   //store them : insert at the end (constant time) //TBD : conditional store
			//   theTiePoints.insert(theTiePoints.end(),tp.begin(),tp.end());
		 //  }

		 //  samp+=TILE_WIDTH * row_step;
		 //  // Set the percent complete.
		 //  tiles_processed += 1.0;
		 //  //setPercentComplete(tiles_processed/total_tiles*100.0);
		 //  setPercentComplete(tiles_processed/total_blocks*100.0);

		 //  //DEBUG TBR TBC
		 //  // std::cout<<"p="<<tiles_processed/total_tiles*100.0<<std::endl;

	  // }
	  // line+=TILE_HEIGHT * col_step;
   //}

   // loong commentted
   for (i=0;(i<tilerows)&&!needsAborting();++i)
   {
      rspf_int32 samp=START_SAMP;
      for (j=0;(j<tilecols)&&!needsAborting();++j )
      {
         // Get the tie points
         //TBC : can we go further than source edges with edge tiles?? TBD
         const vector<rspfTDpt>& tp = src->getFeatures(rspfIrect(rspfIpt(samp, line),rspfIpt(samp+TILE_WIDTH-1,line+TILE_HEIGHT-1)));

         if (theFilename != rspfFilename::NIL)
         {
            //write on stream
            writeTiePoints(tp);
         }
         
         if (getStoreFlag())
         {
            //store them : insert at the end (constant time) //TBD : conditional store
            theTiePoints.insert(theTiePoints.end(),tp.begin(),tp.end());
         }

         samp+=TILE_WIDTH;
         // Set the percent complete.
         tiles_processed += 1.0;
         setPercentComplete(tiles_processed/total_tiles*100.0);

//DEBUG TBR TBC
// std::cout<<"p="<<tiles_processed/total_tiles*100.0<<std::endl;

      }
      line+=TILE_HEIGHT;
   }

   setPercentComplete(100.0);
   if (traceDebug()) CLOG << " Exited." << endl;
   return true;
}

void rspfTieGenerator::writeTiePoints(const vector<rspfTDpt>& tp)
{
   for (vector<rspfTDpt>::const_iterator it=tp.begin();it!=tp.end();++it)
   {
      it->printTab(theFileStream);
      theFileStream<<endl;
   }
}

void rspfTieGenerator::setOutputName(const rspfString& filename)
{
   rspfOutputSource::setOutputName(filename);

   if (isOpen()) close();
   
   if (filename != "")
   {
      theFilename = filename;
   }
}

void rspfTieGenerator::setAreaOfInterest(const rspfIrect& rect)
{
   theAreaOfInterest = rect;
}

bool rspfTieGenerator::isOpen()const
{
   return const_cast<ofstream*>(&theFileStream)->is_open();
}

bool rspfTieGenerator::open()
{
   if(isOpen())
   {
      close();
   }

   if (theFilename == rspfFilename::NIL)
   {
      return false;
   }
   
   theFileStream.open(theFilename.c_str());

   return theFileStream.good();
}

void rspfTieGenerator::close()
{
   if (isOpen()) theFileStream.close();
}
