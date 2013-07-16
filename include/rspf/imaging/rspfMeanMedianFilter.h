//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfMeanMedianFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfMeanMedianFilter_HEADER
#define rspfMeanMedianFilter_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

/*!
 * class rspfMeanMedianFilter
 *
 * Allows you to change between a median or mean filter.  You can
 * also specify a window size which the median or mean is computed and
 * the center pixel is replaced.
 *
 */
class RSPF_DLL rspfMeanMedianFilter : public rspfImageSourceFilter
{
public:

   enum rspfMeanMedianFilterType
   {
      /** Applies filter to any non-null center pixel. */
      RSPF_MEDIAN                  = 0,

      /** Applies filter to all pixels including null center pixels. */
      RSPF_MEDIAN_FILL_NULLS       = 1,

      /** Applies filter to only null center pixels. */
      RSPF_MEDIAN_NULL_CENTER_ONLY = 2,

      /** Applies filter to any non-null center pixel. */
      RSPF_MEAN                    = 3,

      /* Applies filter to all pixels including null center pixels. */
      RSPF_MEAN_FILL_NULLS         = 4,

      /** Applies filter to only null center pixels. */
      RSPF_MEAN_NULL_CENTER_ONLY   = 5
   };

   rspfMeanMedianFilter(rspfObject* owner=NULL);

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();

   void setWindowSize(rspf_uint32 windowSize);
   rspf_uint32 getWindowSize()const;

   /**
    * @param flag Set "theAutoGrowRectFlag".  This only affects filter types
    * that set nulls.  Will have a growing affect on the edges.
    */
   void setAutoGrowRectFlag(bool flag);
   bool getAutoGrowRectFlag() const;

   void setFilterType(rspfMeanMedianFilterType type);
   void setFilterType(const rspfString& type);
   rspfString getFilterTypeString() const;
   void getFilterTypeList(std::vector<rspfString>& list) const;
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
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

   /**
    * @return Returns the bounding rectangle which is the input bounding
    * rectangle with any expansion (from autogrow) added in.
    */
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

protected:
   virtual ~rspfMeanMedianFilter();
   
   rspfRefPtr<rspfImageData> theTile;
   rspfMeanMedianFilterType   theFilterType;
   rspf_uint32                theWindowSize;

   /** Used by applyMean and applyMedian for "fill null" modes. */
   bool theEnableFillNullFlag;

   /**
    * If true rectangle is expanded by half filter in each direction if the
    * theFilterType fills nulls.
    */
   bool theAutoGrowRectFlag;

   void applyFilter(rspfRefPtr<rspfImageData>& input);

   template <class T>
      void applyMean(T dummyVariable,
                     rspfRefPtr<rspfImageData>& inputData);
   template <class T>
      void applyMeanNullCenterOnly(T dummyVariable,
                                   rspfRefPtr<rspfImageData>& inputData);

   template <class T>
      void applyMedian(T dummyVariable,
                       rspfRefPtr<rspfImageData>& inputData);
   template <class T>
      void applyMedianNullCenterOnly(T dummyVariable,
                                     rspfRefPtr<rspfImageData>& inputData);
TYPE_DATA
};

#endif
