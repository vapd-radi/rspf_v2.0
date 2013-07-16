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
//  $Id: rspfEdgeFilter.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfEdgeFilter_HEADER
#define rspfEdgeFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

/**
 * class rspfEdgeFilter
 *
 * This has default implementation for the certain filter types.  For the
 * different filter type please see method setFilterType.
 *
 *
 */
class rspfEdgeFilter : public rspfImageSourceFilter
{
public:
   rspfEdgeFilter(rspfObject* owner=NULL);
   rspfEdgeFilter(rspfImageSource* inputSource);
   rspfEdgeFilter(rspfObject* owner,
                    rspfImageSource* inputSource);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();
   virtual void getFilterTypeNames(std::vector<rspfString>& filterNames)const;
   virtual rspfString getFilterType()const;
   /**
    * The filter type can be one of the following strings.  It is
    * case insensitive for the compares.
    *
    *  Sobel
    *  Laplacian
    *  Prewitt
    *  
    */
   virtual void setFilterType(const rspfString& filterType);

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool saveState(rspfKeywordlist& kwl,const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,const char* prefix=0);
   
protected:
   rspfRefPtr<rspfImageData> theTile;
   rspfString                 theFilterType;
   
   void adjustRequestRect(rspfIrect& requestRect)const;
   
   template <class T>
   void runFilter(T dummyVariable,
                  rspfRefPtr<rspfImageData> inputData);
   template <class T>
   void runSobelFilter(T dummyVariable,
                       rspfRefPtr<rspfImageData> inputData);
   template <class T>
   void runPrewittFilter(T dummyVariable,
                         rspfRefPtr<rspfImageData> inputData);
   template <class T>
   void runLaplacianFilter(T dummyVariable,
                           rspfRefPtr<rspfImageData> inputData);
   template <class T>
   void runRobertsFilter(T dummyVariable,
                           rspfRefPtr<rspfImageData> inputData);
   template <class T>
   void runLocalMax8Filter(T dummyVariable,
                          rspfRefPtr<rspfImageData> inputData);
   
TYPE_DATA
};


#endif
