//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationMultiEllipseObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationMultiEllipseObject_HEADER
#define rspfAnnotationMultiEllipseObject_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <vector>

class RSPFDLLEXPORT rspfAnnotationMultiEllipseObject : public rspfAnnotationObject
{
public:
   rspfAnnotationMultiEllipseObject(const std::vector<rspfDpt>& pointList,
                                     const rspfDpt& widthHeight = rspfDpt(1,1),
                                     bool enableFill = false,
                                     unsigned char r = 255,
                                     unsigned char g = 255,
                                     unsigned char b = 255,
                                     long thickness = 1);

   rspfAnnotationMultiEllipseObject(const rspfDpt& widthHeight = rspfDpt(1,1),
                                     bool enableFill = false,
                                     unsigned char r = 255,
                                     unsigned char g = 255,
                                     unsigned char b = 255,
                                     long thickness = 1);

   rspfAnnotationMultiEllipseObject(const rspfAnnotationMultiEllipseObject& rhs);

   rspfObject* dup()const;


   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual void applyScale(double x, double y);
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();
  /*!
   */
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   
   void setFillFlag(bool flag);

   void resize(rspf_uint32 size);

   virtual void setWidthHeight(const rspfDpt& widthHeight);

   rspfDpt& operator[](int i);
   
   const rspfDpt& operator[](int i)const;
   
protected:
   virtual ~rspfAnnotationMultiEllipseObject();
   std::vector<rspfDpt>  thePointList;
   rspfDpt               theWidthHeight;
   bool                   theFillFlag;
   rspfDrect             theBoundingRect;
};

#endif
