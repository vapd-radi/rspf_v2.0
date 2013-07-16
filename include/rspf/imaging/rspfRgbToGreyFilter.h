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
// $Id: rspfRgbToGreyFilter.h 20197 2011-11-03 13:22:02Z dburken $
#ifndef rspfRgbToGreyFilter_HEADER
#define rspfRgbToGreyFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPFDLLEXPORT rspfRgbToGreyFilter : public rspfImageSourceFilter
{
public:
   rspfRgbToGreyFilter(rspfObject* owner=NULL);
   rspfRgbToGreyFilter(rspfImageSource* inputSource,
                        double c1 = 1.0/3.0,
                        double c2 = 1.0/3.0,
                        double c3 = 1.0/3.0);
   rspfRgbToGreyFilter(rspfObject* owner,
                        rspfImageSource* inputSource,
                        double c1 = 1.0/3.0,
                        double c2 = 1.0/3.0,
                        double c3 = 1.0/3.0);
   
   virtual rspfString getShortName()const;
   virtual rspfString getLongName()const;
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   virtual void initialize();
   
   virtual rspfScalarType getOutputScalarType() const;
   
   rspf_uint32 getNumberOfOutputBands() const;
 
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

protected:
   virtual ~rspfRgbToGreyFilter();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   rspfRefPtr<rspfImageData> theBlankTile;
   rspfRefPtr<rspfImageData> theTile;
   void runUcharTransformation(rspfRefPtr<rspfImageData>& tile);
   double theC1;
   double theC2;
   double theC3;

TYPE_DATA
};

#endif /* #ifndef rspfRgbToGreyFilter_HEADER */
