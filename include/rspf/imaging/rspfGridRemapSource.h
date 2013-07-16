//*****************************************************************************
// FILE: rspfGridRemapSource.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class rspfGridRemapSource. This is
//   a spacially variant remapper that utilizes a grid for interpolating the
//   remap value given an image x, y. 
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGridRemapSource.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfGridRemapSource_HEADER
#define rspfGridRemapSource_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>
#include <vector>
#include <rspf/imaging/rspfGridRemapEngine.h>
#include <rspf/base/rspfFilename.h>

class rspfDblGrid;
class rspfDrect;
class rspfDpt;
class rspfImageData;

/*!****************************************************************************
 *
 * CLASS:  rspfGridRemapSource
 *
 *****************************************************************************/
class rspfGridRemapSource : public rspfImageSourceFilter
{
public:
   /*!
    * Constructors: 
    */
   rspfGridRemapSource();
   rspfGridRemapSource(rspfImageSource* inputSource,
                        rspfGridRemapEngine* engine);

   /*!rspfAtbPointSource
    * Initializes the remap engine. This object implements specific algorithms
    * for interpreting the grids owned by this remapper. It actually performs
    * the remapping.
    */
   void setRemapEngine(rspfGridRemapEngine* engine);
   
   /*!
    * Provides access to the remap engine:
    */
   rspfGridRemapEngine* getRemapEngine() { return theRemapEngine; }
   
   /*!
    * Permits initializing the remap grid after construction. 
    */
   virtual void initialize(const rspfDrect& uv_rect,
			   const rspfDpt&   grid_spacing);

  /*!
   * Sets a node of the member grid to the value specified. The grid is flagged
   * as unfilled.
   */
  void setGridNode(const rspfDpt& view_pt,
		   const double*    value);
  
   /*!
    * Implementation of virtual method to return remapped tile.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const  rspfIrect& origin,
                                   rspf_uint32 resLevel=0);

   /*!
    * Restore the state of the object from a keywordlist.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /*!
    * Save the state of the object to a keywordlist.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /*!
    * Locks the image to prevent grid adjustment. The remapper continues to
    * remap given the current grid (if enabled), but all target values computed
    * are such that no change in the grids will occur in an adjsutment. If more
    * than one overlapping image is locked, the target will may be influenced
    * by all locked images, but the remap grids will not be updated.
    */
   void lock() { theRemapIsLockedFlag = true; }

   /*!
    * Unlocks the grids for adjustment.
    */
   void unlock() { theRemapIsLockedFlag = false; }
   
   /*!
    * Returns the status of the lock flag.
    */
   bool isLocked() const { return theRemapIsLockedFlag; }

   /*!
    * Provides access to individual grids (pointer) given an index. Null pointer
    * returned if index out of range.
    */
   rspfDblGrid* getGrid(unsigned int index);

   /*!
    * Returns grid filename (may be empty)
    */
   const rspfFilename& getGridFilename() const { return theGridFilename; }

   /*!
    * Sets the grid filename and initiates read.
    */
   void setGridFilename(const rspfFilename& grid_filename);
   
protected:
   virtual ~rspfGridRemapSource();

   /*!
    * Deallocates grid memory.
    */
   void deallocateGrids();
   
   vector<rspfDblGrid*> theGrids;
   rspfFilename         theGridFilename;
   rspfGridRemapEngine* theRemapEngine;
   bool                  theRemapIsLockedFlag;
   bool                  theGridIsFilled;
   
TYPE_DATA
};

#endif
