//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id: rspfGeoAnnotationSource.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfGeoAnnotationSource_HEADER
#define rspfGeoAnnotationSource_HEADER

#include <iostream>
#include <rspf/imaging/rspfAnnotationSource.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/base/rspfViewInterface.h>

class rspfGeoAnnotationObject;
class rspfProjection;

class RSPF_DLL rspfGeoAnnotationSource :
   public rspfAnnotationSource,
   public rspfViewInterface
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfGeoAnnotationSource& rhs);
   rspfGeoAnnotationSource(rspfImageGeometry* geom=NULL,
                            bool ownsProjectionFlag=false);

   rspfGeoAnnotationSource(rspfImageSource* inputSource,
                            rspfImageGeometry* geom=NULL,
                            bool ownsProjectionFlag=false);

   
   /*!
    * Will add an object to the list.
    * it will own the object added.  If the
    * object is not geographic 
    */
   virtual bool addObject(rspfAnnotationObject* anObject);
   virtual void computeBoundingRect();
   virtual void transformObjects(rspfImageGeometry* geom=0);
   virtual void setGeometry(rspfImageGeometry* projection);
   virtual bool setView(rspfObject* baseObject);
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
  
   virtual rspfObject*       getView();
   virtual const rspfObject* getView()const;

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
   virtual ~rspfGeoAnnotationSource();
   
   rspfRefPtr<rspfImageGeometry> m_geometry;
   
TYPE_DATA
};

#endif
