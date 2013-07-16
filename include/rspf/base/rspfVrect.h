//*******************************************************************
//
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for vrect.
// Container class for four double points representing a rectangle
// where y is up
// 
//*******************************************************************
//  $Id: rspfVrect.h 11955 2007-10-31 16:10:22Z gpotts $
#ifndef rspfVrect_HEADER
#define rspfVrect_HEADER
#include <iostream>

#include <rspf/base/rspfDpt.h>

class RSPFDLLEXPORT rspfVrect
{
public:
   rspfVrect()
      :theUlCorner(0.0, 0.0),
       theLlCorner(0.0, 0.0),
       theLrCorner(0.0, 0.0),
       theUrCorner(0.0, 0.0)
      {}
   rspfVrect(const rspfDpt &ul,
         const rspfDpt &lr)
      :theUlCorner(ul),
       theLlCorner(ul.x, lr.y),
       theLrCorner(lr),
       theUrCorner(lr.x, ul.y)
      {}
   rspfVrect(double ul_corner_x,
         double ul_corner_y,
         double lr_corner_x,
         double lr_corner_y)
      :
         theUlCorner(ul_corner_x, ul_corner_y),
         theLlCorner(ul_corner_x, lr_corner_y),
         theLrCorner(lr_corner_x, lr_corner_y),
         theUrCorner(lr_corner_x, ul_corner_y)
      {}
   rspfVrect(const rspfVrect& rect)
      :
         theUlCorner(rect.ul()),
         theLlCorner(rect.ll()),
         theLrCorner(rect.lr()),
         theUrCorner(rect.ur())
      {}

   ~rspfVrect();

   const rspfVrect& operator=  (const rspfVrect& rect);
   bool         operator!= (const rspfVrect& rect) const;
   bool         operator== (const rspfVrect& rect) const;
   
   const rspfDpt &ul() const { return theUlCorner; }
   const rspfDpt &ur() const { return theUrCorner; }
   const rspfDpt &lr() const { return theLrCorner; }
   const rspfDpt &ll() const { return theLlCorner; }
   /*!
    * Returns the height of a rectangle.
    */
   rspf_float64 height() const
      { return fabs(theLlCorner.y - theUlCorner.y) + 1.0; }

   /*!
    * Returns the width of a rectangle.
    */
   rspf_float64 width()  const
      { return fabs(theLrCorner.x - theLlCorner.x) + 1.0; }
   
   inline rspfDpt midPoint()const;
   void print(std::ostream& os) const;
   
   rspfVrect combine(const rspfVrect& rect)const;
   inline rspfVrect clipToRect(const rspfVrect& rect)const;

   friend std::ostream& operator<<(std::ostream& os, const rspfVrect& rect);

private:
   rspfDpt theUlCorner;
   rspfDpt theLlCorner;
   rspfDpt theLrCorner;
   rspfDpt theUrCorner;
};

inline const rspfVrect& rspfVrect::operator=(const rspfVrect& rect)
{
   if (this != &rect)
   {
      theUlCorner = rect.ul();
      theUrCorner = rect.ur();
      theLrCorner = rect.lr();
      theLlCorner = rect.ll();
   }

   return *this;
}

inline bool rspfVrect::operator!=(const rspfVrect& rect) const
{
   return ( (theUlCorner != rect.ul()) ||
            (theUrCorner != rect.ur()) ||
            (theLrCorner != rect.lr()) ||
            (theLlCorner != rect.ll()) );
}

inline bool rspfVrect::operator==(const rspfVrect& rect) const
{
   return ( (theUlCorner == rect.ul()) &&
            (theUrCorner == rect.ur()) &&
            (theLrCorner == rect.lr()) &&
            (theLlCorner == rect.ll()) );
}


inline rspfVrect rspfVrect::clipToRect(const rspfVrect& rect)const
{
    rspf_float64     ulx, uly, lrx, lry;

    // XXX not replaced with std::max or rspf::max since the test is backward 
    //     here and will give a different answer in the case of nan.
    #define d_MAX(a,b)      (((a)>(b)) ? a : b)

    ulx = d_MAX(rect.ul().x,ul().x);
    uly = d_MAX(rect.ul().y,ul().y);

    #undef d_MAX

    lrx = std::min(rect.lr().x,lr().x);
    lry = std::min(rect.lr().y,lr().y);

    if( lrx <= ulx || lry <= uly )
        return rspfVrect(rspfDpt(0,0),rspfDpt(0,0));
    else
       return rspfVrect(ulx,uly,lrx,lry);
}

inline rspfDpt rspfVrect::midPoint()const
{
   return rspfDpt( (ul().x + ur().x + ll().x + lr().x)*.25,
               (ul().y + ur().y + ll().y + lr().y)*.25);
}

inline rspfVrect rspfVrect::combine(const rspfVrect& rect)const
{
   rspfDpt ulCombine;
   rspfDpt lrCombine;
   
   ulCombine.x = ((ul().x <= rect.ul().x)?ul().x:rect.ul().x);
   ulCombine.y = ((ul().y >= rect.ul().y)?ul().y:rect.ul().y);
   lrCombine.x = ((lr().x >= rect.lr().x)?lr().x:rect.lr().x);
   lrCombine.y = ((lr().y <= rect.lr().y)?lr().y:rect.lr().y);

   return rspfVrect(ulCombine, lrCombine);
}

#endif
