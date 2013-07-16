//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageMosaic.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfImageMosaic_HEADER
#define rspfImageMosaic_HEADER
#include <vector>
using namespace std;

#include <rspf/imaging/rspfImageCombiner.h>


/**
 * An image mosaic is a simple combiner that will
 * just do a simple mosaic.  It just checks NULL pix values until it finds a
 * pixel that is not empty and copies it out to the output.  The list will
 * have same size tiles and have the same number of bands.
 */
class RSPFDLLEXPORT rspfImageMosaic : public rspfImageCombiner
{
public:
   rspfImageMosaic();
   rspfImageMosaic(rspfConnectableObject::ConnectableObjectList& inputSources);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);
   
   virtual void initialize();
   
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

protected:
   virtual ~rspfImageMosaic();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   rspfRefPtr<rspfImageData> theTile;

   template <class T> rspfRefPtr<rspfImageData> combine(
      T, // dummy template variable not used
      const rspfIrect& tileRect,
      rspf_uint32 resLevel=0);
   template <class T> rspfRefPtr<rspfImageData> combineNorm(
      T, // dummy template variable not used
      const rspfIrect& tileRect,
      rspf_uint32 resLevel=0);

TYPE_DATA
};

#endif /* #ifndef rspfImageMosaic_HEADER */
