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
// $Id: rspfBandClipFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfBandClipFilter_HEADER
#define rspfBandClipFilter_HEADER
#include <vector>
using namespace std;

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfBandClipFilter : public rspfImageSourceFilter
{
public:
   /*!
    * Specifies the type of clipping.  The first will
    * just clip and 
    */
   enum rspfBandClipType
   {
      rspfBandClipType_NONE             = 0,
      rspfBandClipType_CLIP             = 1,
      rspfBandClipType_CLAMP            = 2,
      rspfBandClipType_LINEAR_STRETCH   = 3,
      rspfBandClipType_MEDIAN_STRETCH   = 4 
   };

   rspfBandClipFilter();
   rspfBandClipFilter(rspfImageSource*,
                        const vector<double>& minPix,
                        const vector<double>& maxPix,
                        rspfBandClipType clipType=rspfBandClipType_NONE);
   
   rspfBandClipFilter(rspfImageSource*,
                        double minPix,
                        double maxPix,
                        rspfBandClipType clipType=rspfBandClipType_NONE);


   void setClipType(rspfBandClipType clipType);

   rspfBandClipType getClipType()const;

   virtual rspf_uint32 getNumberOfValues()const;

   void setNumberOfValues(rspf_uint32 size);
   
   void setMinMaxPix(const vector<double>& minPix,
                     const vector<double>& maxPix);

   const std::vector<double>& getMinPixList()const;
   const std::vector<double>  getMaxPixList()const;

   double getMinPix(rspf_uint32 index)const;
   double getMaxPix(rspf_uint32 index)const;

   rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                       rspf_uint32 resLevel=0);
   
   virtual void initialize();
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = NULL);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = NULL)const;
protected:
   virtual ~rspfBandClipFilter();

   void runClip();
   void runClamp();
   void runLinearStretch();
   void runMedianStretch();

   std::vector<double> theMinPix; // normalized min
   std::vector<double> theMaxPix; // normalized max
   std::vector<double> theMedian; // normalized median.

   rspfBandClipType           theClipType;
   rspfRefPtr<rspfImageData> theTile;

TYPE_DATA
};

#endif
