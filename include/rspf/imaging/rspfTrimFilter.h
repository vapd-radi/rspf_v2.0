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
// $Id: rspfTrimFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfTrimFilter_HEADER
#define rspfTrimFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfPolyCutter.h>

class rspfTrimFilter : public rspfImageSourceFilter
{
public:
   rspfTrimFilter();

   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
   virtual void getValidImageVertices(vector<rspfIpt>& validVertices,
                                      rspfVertexOrdering ordering=RSPF_CLOCKWISE_ORDER,
                                      rspf_uint32 resLevel=0)const;

   rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                       rspf_uint32 resLevel=0);

   virtual void initialize();
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   

   virtual rspfRefPtr<rspfProperty> getProperty(
      const rspfString& name)const;

   /**
    * Set property.
    *
    * @param property Property to set if property->getName() matches a
    * property name of this object.
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);

   /**
    * Adds this objects properties to the list.
    *
    * @param propertyNames Array to add to.
    *
    * @note This method does not clear propertyNames prior to adding it's
    * names.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

protected:
   virtual ~rspfTrimFilter();
   double theLeftPercent;
   double theRightPercent;
   double theTopPercent;
   double theBottomPercent;
   std::vector<rspfIpt>  theValidVertices;
   rspfRefPtr<rspfPolyCutter> theCutter;
   
TYPE_DATA
};

#endif /* #ifndef rspfTrimFilter_HEADER */
