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
// $Id: rspfHistogramThreshholdFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfHistogramThreshholdFilter_HEADER
#define rspfHistogramThreshholdFilter_HEADER
#include <rspf/imaging/rspfImageSourceHistogramFilter.h>

class rspfHistogramThreshholdFilter : public rspfImageSourceHistogramFilter
{
public:
   rspfHistogramThreshholdFilter();
   rspfHistogramThreshholdFilter(double minValuePercent,
                                  double maxValuePercent,
                                  rspfImageSource* inputSource,
                                  rspfMultiResLevelHistogram* histogram);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   virtual void setMinValuePercent(double minValue);
   virtual void setMaxValuePercent(double maxValue);
   
   virtual double getMinValuePercent()const;
   virtual double getMaxValuePercent()const;

   virtual void initialize();
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
protected:
   virtual ~rspfHistogramThreshholdFilter();
   
   double theMinValuePercent;
   double theMaxValuePercent;
   template <class T>
   rspfRefPtr<rspfImageData> runThreshholdStretchAlgorithm(
      T dummyVariable,
      rspfRefPtr<rspfImageData>& tile);
   
TYPE_DATA
};
#endif
