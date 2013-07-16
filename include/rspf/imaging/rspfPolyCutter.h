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
// $Id: rspfPolyCutter.h 19907 2011-08-05 19:55:46Z dburken $
#ifndef rspfPolyCutter_HEADER
#define rspfPolyCutter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/imaging/rspfImageDataHelper.h>
#include <vector>

//class rspfPolyArea2d;
class rspfImageData;

class RSPFDLLEXPORT rspfPolyCutter : public rspfImageSourceFilter
{
public:
   enum rspfPolyCutterCutType
   {
      RSPF_POLY_NULL_INSIDE  = 0,
      RSPF_POLY_NULL_OUTSIDE = 1
   };
   
   rspfPolyCutter();
    rspfPolyCutter(rspfImageSource* inputSource,
                    const rspfPolygon& polygon);

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

   virtual void initialize();
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual void setPolygon(const vector<rspfDpt>& polygon,
                           rspf_uint32 i = 0);
   
   virtual void setPolygon(const vector<rspfIpt>& polygon,
                           rspf_uint32 i = 0);
   
   virtual void addPolygon(const vector<rspfIpt>& polygon);
   virtual void addPolygon(const vector<rspfDpt>& polygon);
   virtual void addPolygon(const rspfPolygon& polygon);
   
   virtual void setNumberOfPolygons(rspf_uint32 count);
   
   virtual std::vector<rspfPolygon>& getPolygonList();

   virtual const std::vector<rspfPolygon>& getPolygonList()const;
   
   void setCutType(rspfPolyCutterCutType cutType);
   
   rspfPolyCutterCutType getCutType()const;

   void clear();

   const rspfIrect& getRectangle() const;

   void setRectangle(const rspfIrect& rect);
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
protected:
   virtual ~rspfPolyCutter();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   void computeBoundingRect();

   rspfRefPtr<rspfImageData> theTile;

   /*!
    * Will hold a pre-computed bounding rect of the
    * polygon data.
    */
   rspfIrect theBoundingRect;

   std::vector<rspfPolygon> thePolygonList;

   /*!
    * theDefault fill will be outside.
    */
   rspfPolyCutterCutType theCutType;
   rspfImageDataHelper theHelper;
   bool m_boundingOverwrite;

TYPE_DATA  
};
#endif /* #ifndef rspfPolyCutter_HEADER */
