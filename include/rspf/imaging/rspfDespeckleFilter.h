//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*******************************************************************
//  $Id: rspfDespeckleFilter.h 2644 2011-05-26 15:20:11Z oscar.kramer $
#ifndef rspfDespeckleFilter_HEADER
#define rspfDespeckleFilter_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfDespeckleFilter : public rspfImageSourceFilter
{
public:
   rspfDespeckleFilter();
   rspfDespeckleFilter(rspfImageSource* inputSource, rspf_uint32 filter_radius=1);
   
   virtual void setRadius(rspf_uint32 radius) { theFilterRadius = (rspf_int32) radius; }
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=NULL)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=NULL);

   rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect, rspf_uint32 resLevel=0);
   
   virtual void initialize();
   
   /* ------------------- PROPERTY INTERFACE -------------------- */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /* ------------------ PROPERTY INTERFACE END ------------------- */

protected:
   virtual ~rspfDespeckleFilter();
   template<class T> void despeckle(T dummyVariable, rspfRefPtr<rspfImageData> inputTile);

   void allocate();
   
   rspfRefPtr<rspfImageData> theTile;
   rspf_int32                theFilterRadius;
   
TYPE_DATA
};

#endif /* #ifndef rspfDespeckleFilter_HEADER */
