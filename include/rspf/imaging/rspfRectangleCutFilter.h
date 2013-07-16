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
// $Id: rspfRectangleCutFilter.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfRectangleCutFilter_HEADER
#define rspfRectangleCutFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPF_DLL rspfRectangleCutFilter : public rspfImageSourceFilter
{
public:
   enum rspfRectangleCutType
   {
      RSPF_RECTANGLE_NULL_INSIDE  = 0,
      RSPF_RECTANGLE_NULL_OUTSIDE = 1
   };

   rspfRectangleCutFilter(rspfObject* owner,
                     rspfImageSource* inputSource=NULL);
   rspfRectangleCutFilter(rspfImageSource* inputSource=NULL);

   void setRectangle(const rspfIrect& rect);

   const rspfIrect&     getRectangle()const;
   rspfRectangleCutType getCutType()const;
   void                  setCutType(rspfRectangleCutType cutType);
   
   rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
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
   
protected:
   rspfIrect            theRectangle;
   rspfRectangleCutType theCutType;
   vector<rspfDpt>      theDecimationList;
TYPE_DATA
};

#endif /* #ifndef rspfRectangleCutFilter_HEADER */
