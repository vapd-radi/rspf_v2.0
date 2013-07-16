//*****************************************************************************
// FILE: rspfGridRemapper.cc
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfGridRemapper. This is
//   a spacially variant remapper that utilizes a grid for interpolating the
//   remap value given an image x, y. 
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGridRemapSource.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <rspf/imaging/rspfGridRemapSource.h>

RTTI_DEF1(rspfGridRemapSource,
          "rspfGridRemapSource",
          rspfImageSourceFilter);

#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfGridRemapEngineFactory.h>
#include <cstdio>
#include <fstream>


//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfGridRemapSource:exec");
static rspfTrace traceDebug ("rspfGridRemapSource:debug");

static const char* GRID_FILENAME_KW        = "grid_remap_file";
static const char* REMAP_ENGINE_KW         = "remap_engine";

//*****************************************************************************
//  DEFAULT CONSTRUCTOR #1: rspfGridRemapper()
//  
//*****************************************************************************
rspfGridRemapSource::rspfGridRemapSource()
   :
      rspfImageSourceFilter(),
      theGridFilename (0),
      theRemapEngine (0),
      theRemapIsLockedFlag(true),
      theGridIsFilled(false)
{
   disableSource();
}

//*****************************************************************************
//  CONSTRUCTOR #2: rspfGridRemapper(inputSource)
//  
//*****************************************************************************
rspfGridRemapSource::rspfGridRemapSource(rspfImageSource* inputSource,
                                           rspfGridRemapEngine* engine)
   :
      rspfImageSourceFilter(inputSource),
      theGridFilename (0),
      theRemapEngine ((rspfGridRemapEngine*)engine->dup()),
      theRemapIsLockedFlag(true),
      theGridIsFilled(false)
{
   disableSource();
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfGridRemapSource
//  
//*****************************************************************************
rspfGridRemapSource::~rspfGridRemapSource()
{
   deallocateGrids();
   theRemapEngine = 0;
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::initialize
//  
//*****************************************************************************
void rspfGridRemapSource::initialize(const rspfDrect& uv_rect,
                                      const rspfDpt&   grid_spacing)
{
   deallocateGrids();
   int num_params;
   
   if (theRemapEngine)
   {
      num_params = theRemapEngine->getNumberOfParams();
      
      for (int p=0; p<num_params; p++)
      {
         rspfDblGrid* grid = new rspfDblGrid(uv_rect, grid_spacing);
         theGrids.push_back(grid);
      }
      
      theGridFilename = "ogrs";
      theGridFilename += rspfString::toString(reinterpret_cast<rspf_int64>(this));
      theGridFilename += ".org";

      theGridIsFilled = false;
   }
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::getTile()
//  
//  Implementation of virtual method to return remapped tile.
//  
//*****************************************************************************
rspfRefPtr<rspfImageData> rspfGridRemapSource::getTile(
   const rspfIrect& rect, rspf_uint32 resLevel)
{
   if(!theInputConnection)
      return NULL;

   //***
   // Fetch tile from input source:
   //***
   rspfRefPtr<rspfImageData> tile = theInputConnection->getTile(rect,
                                                                  resLevel);

   //---
   // Bypass this filter if it is not initialized, or if input tile is bogus:
   //---
   if ((!isSourceEnabled()) || (!tile.valid()) || (!theRemapEngine) ||
       (tile->getDataObjectStatus()==RSPF_NULL) ||
       (tile->getDataObjectStatus()==RSPF_EMPTY))
   {
      return tile;
   }
   
   //***
   // Insure that the grid has been filled in case nodes were being randomly set
   //***
   if (!theGridIsFilled)
   {
      vector<rspfDblGrid*>::iterator grid = theGrids.begin();
      while (grid != theGrids.end())
      {
//         (*grid)->setInterpolationType(rspfDblGrid::BILINEAR);
         (*grid)->interpolateNullValuedNodes();
         grid++;
      }
      theGridIsFilled = true;
   }
   
   //***
   // hand off the actual remap to the ATB engine. This object knows how to
   // interpret the grids for the particular remap algorithm selected:
   //***
   theRemapEngine->remapTile(rect.ul(), this, tile);
   
   tile->validate();
   return tile;
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::loadState()
//  
//*****************************************************************************
bool rspfGridRemapSource::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   static const char MODULE[] = "rspfGridRemapSource::loadState()";
   if (traceExec())  CLOG << "entering..." << endl;
   
   bool successful = false;
   const char* value;

   //***
   // Reset object in preparation for reassignment:
   //***
   deallocateGrids();
   theRemapEngine = 0;
   
   //***
   // Read the remap engine type:
   //***
   value = kwl.find(prefix, REMAP_ENGINE_KW);
   theRemapEngine = rspfGridRemapEngineFactory::create(value);
   if (!theRemapEngine)
   {
//      CLOG << "ERROR: could not instantiate remap engine. Aborting..." << endl;
//      if (traceExec())  CLOG << "returning..." << endl;
//      return false;
   }
   
   //***
   // Read the grid filename and open input stream:
   //***
   theGridIsFilled = false;
   value = kwl.find(prefix, GRID_FILENAME_KW);
   if (value)
   {
      theGridFilename = value;
      ifstream is (theGridFilename.chars());
      
      //***
      // Create an input stream from the grid file to pass to the
      // corresponding grid:
      //***
      int num_grids = theRemapEngine->getNumberOfParams();
      for (int i=0; (i<num_grids) && successful; i++)
      {
         rspfDblGrid* grid = new rspfDblGrid;
         successful = grid->load(is);
         if (successful)
            theGrids.push_back(grid);
      }
      if (!successful)
      {
         CLOG << "ERROR: Encountered errorloading remap grids at file: "
              << theGridFilename << ". Remapper disabled." << endl;
         if (traceExec())  CLOG << "returning..." << endl;
         return false;
      }
      theGridIsFilled = true;
   }
   
   //***
   // Allow base class to parse list:
   //***
   bool rtn_stat = rspfImageSourceFilter::loadState(kwl, prefix);

   //***
   // Enable source only if KWL read was successful:
   //***
   if (theRemapEngine && successful && rtn_stat)
      enableSource();

   if (traceExec())  CLOG << "returning..." << endl;
   return rtn_stat;
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::saveState()
//  
//*****************************************************************************
bool rspfGridRemapSource::saveState(rspfKeywordlist& kwl,
                                     const char*       prefix) const
{
   static const char MODULE[] = "rspfGridRemapSource::saveState()";

   //***
   // No filename indicates that this remapper has not been initialized:
   //***
   if (!theGridFilename)
      return false;
   
   //***
   // Write the remap engine type:
   //***
   if (theRemapEngine)
      kwl.add(prefix, REMAP_ENGINE_KW, theRemapEngine->getClassName());

   //***
   // Write the filename to the KWL:
   //***
   if (!theGridFilename.empty())
   {
      kwl.add(prefix, GRID_FILENAME_KW, theGridFilename);

      //***
      // Loop over each remap component to write out the grid:
      //***
      ofstream os (theGridFilename.c_str());
      bool successful = true;
      int num_components = theRemapEngine->getNumberOfParams();
      for (int p=0; (p<num_components)&&successful; p++)
         successful = theGrids[p]->save(os, "Remap-Grid");

      if (!successful)
      {
         CLOG << "ERROR: Encountered saving remap grids to file: "
              << theGridFilename << ". State not properly saved."
              << endl;
         return false;
      }
   }
   
   return rspfImageSourceFilter::saveState(kwl, prefix);   
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::setGridNode()
//  
//  Sets a node of the member grid to the value specified.
//  
//*****************************************************************************
void rspfGridRemapSource::setGridNode(const rspfDpt& view_pt,
                                       const double*   value)
{
   int numGrids = (int)theGrids.size();
   for (int i=0; i<numGrids; i++)
      theGrids[i]->setNearestNode(view_pt, value[i]);

   theGridIsFilled = false;
   
   return;
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::getGrid()
//  
//*****************************************************************************
rspfDblGrid* rspfGridRemapSource::getGrid(unsigned int index)
{
   if (index >= theGrids.size())
      return 0;

   return theGrids[index];
}
   
//*****************************************************************************
//  METHOD: rspfGridRemapSource::deallocateGrids()
//  
//*****************************************************************************
void rspfGridRemapSource::deallocateGrids()
{
   static const char MODULE[] = "rspfGridRemapSource::deallocateMemory()";
   if (traceExec())  CLOG << "entering..." << endl;

   vector<rspfDblGrid*>::iterator grid = theGrids.begin();
   while (grid != theGrids.end())
   {
      delete *grid;
      grid++;
   }
   theGrids.clear();

   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfGridRemapSource::setRemapEngine()
//  
//*****************************************************************************
void rspfGridRemapSource::setRemapEngine(rspfGridRemapEngine* engine)
{

   theRemapEngine = engine;
   theGridIsFilled = false;
}

//*****************************************************************************
//  METHOD: rspfGridRemapSource::setGridFilename()
//  
//*****************************************************************************
void rspfGridRemapSource::setGridFilename(const rspfFilename& grid_filename)
{
   theGridFilename = grid_filename;
   //***
   // NOT COMPLETE###
   //***
}
