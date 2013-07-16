//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationObject_HEADER
#define rspfAnnotationObject_HEADER
#include <iostream>

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfAnnotationObject : public rspfObject
{
public:
   rspfAnnotationObject(rspf_uint8 r=255,
                         rspf_uint8 g=255,
                         rspf_uint8 b=255,
                         rspf_uint8 thickness=1);
   virtual void applyScale(double x,
                           double y)=0;
   virtual void applyScale(const rspfDpt& scale);
   
   virtual std::ostream& print(std::ostream& out)const;

   virtual void draw(rspfRgbImage& anImage)const=0;
   virtual bool intersects(const rspfDrect& rect)const=0;

   /*!
    * Will allocate a new object that is clipped to the passed.
    * in rect.  Look at derived classes to see if the object is actually
    * clipped or not.  A new object is returned if it clips to the rect
    * else NULL should be returned it doesn't clip.
    */
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const=0;
   
   virtual void getBoundingRect(rspfDrect& rect)const=0;
   virtual rspfDrect getBoundingRect()const;

   virtual void computeBoundingRect()=0;
   
   virtual void setColor(rspf_uint8 r,
                         rspf_uint8 g,
                         rspf_uint8 b);

   void getColor(rspf_uint8 &r,
                 rspf_uint8 &g,
                 rspf_uint8 &b)const;

   rspf_uint8  getThickness()const;
   virtual void setThickness(rspf_uint8 thickness);
   
   /*!
    * This method allows you to pick an object.  It will
    * return true or false if the point falls somewhere
    * on an object.  Look at derived classes for further
    * information.
    */
   virtual bool isPointWithin(const rspfDpt& imagePoint) const;

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

   virtual rspfString getName() const;
   virtual void setName(const rspfString& name);

  rspf_uint32 getId()const;
  virtual rspf_uint32 setId();

  static rspf_uint32 getNextId();
  static void restIdCounter();

protected:
   virtual ~rspfAnnotationObject();
   rspf_uint8 theRed;
   rspf_uint8 theGreen;
   rspf_uint8 theBlue;
   rspf_uint8 theThickness;

   rspfString  theName;
   rspf_uint32 theId;

  static rspf_uint32 theIdCounter;
TYPE_DATA
};

#endif
