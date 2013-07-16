//*******************************************************************
// Copyright (C) 2005 SANZ Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Kenneth Melero (kmelero@sanz.com)
//
// Description: This combiner is designed to "float" the maximum pixel value
//              of all inputs to the top of the mosaic output. 
//
//*************************************************************************
// $Id: rspfMaxMosaic.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfMaxMosaic_HEADER
#define rspfMaxMosaic_HEADER

#include <vector>
#include <rspf/imaging/rspfImageCombiner.h>


/**
 * An image mosaic is a simple combiner that will
 * just do a simple mosaic.  It just checks NULL pix values until it finds a
 * pixel that is not empty and copies it out to the output.  The list will
 * have same size tiles and have the same number of bands.
 */
class RSPFDLLEXPORT rspfMaxMosaic : public rspfImageCombiner
{
public:
   rspfMaxMosaic();
   rspfMaxMosaic(rspfConnectableObject::ConnectableObjectList& inputSources);
   
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

   virtual rspfString getShortName() const;
   virtual rspfString getLongName()  const;
   virtual rspfString getDescription()const;

protected:
   virtual ~rspfMaxMosaic();
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

#endif
