//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationObject_HEADER
#define rspfGeoAnnotationObject_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/imaging/rspfImageGeometry.h>

class rspfImageProjectionModel;

class RSPF_DLL rspfGeoAnnotationObject : public rspfAnnotationObject
{
public:
   rspfGeoAnnotationObject(unsigned char r=255,
                            unsigned char g=255,
                            unsigned char b=255,
                            long thickness=1);

   rspfGeoAnnotationObject(const rspfGeoAnnotationObject& rhs);

   
   /*!
    * Add the projection interface to all
    * geographically defined objects.
    */
   virtual void transform(rspfImageGeometry* projection)=0;

   /**
    * Saves the current state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

protected:
   virtual ~rspfGeoAnnotationObject();

TYPE_DATA
};

#endif
