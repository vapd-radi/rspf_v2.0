//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfIrect.
// Container class for four integer points representing a rectangle.
//
//*******************************************************************
//  $Id: rspfIrect.h 22197 2013-03-12 02:00:55Z dburken $

#ifndef rspfIrect_HEADER
#define rspfIrect_HEADER 1
#include <iosfwd>
#include <vector>

#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfCommon.h>

//***
// NOTE:  A word on corner points...
//
// There is the concept of "pixel is area" and "pixel is point".
// - Pixel is area means the (x,y) pixel coordinate refers to the upper left
//   corner of the pixel, NOT the center of the pixel.
// - Pixel is point means the (x,y) pixel coordinate refers to the center
//   of the pixel, NOT the upper left corner.
//
// For the uniformity purposes, all pixel points  should be in the
// "pixel is point" form; therefore, the (x,y) point should represent the
// CENTER of the pixel.
//***


//***
// Forward class declarations.
//***
class rspfDrect;
class rspfKeywordlist;

//*******************************************************************
// CLASS:  rspfIrect
//*******************************************************************

class RSPFDLLEXPORT rspfIrect
{
public:
   enum
   {
      UPPER_LEFT  = 1,
      LOWER_LEFT  = 2,
      LOWER_RIGHT = 4,
      UPPER_RIGHT = 8
   };

   rspfIrect()
      :
         theUlCorner(0, 0),
         theUrCorner(0, 0),
         theLrCorner(0, 0),
         theLlCorner(0, 0),
         theOrientMode(RSPF_LEFT_HANDED)
      {}

   rspfIrect(rspfIpt ul_corner,
              rspfIpt lr_corner,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED)
      :
         theUlCorner(ul_corner),
         theUrCorner(lr_corner.x, ul_corner.y),
         theLrCorner(lr_corner),
         theLlCorner(ul_corner.x, lr_corner.y),
         theOrientMode(mode)
      {}

   rspfIrect(rspf_int32 ul_corner_x,
              rspf_int32 ul_corner_y,
              rspf_int32 lr_corner_x,
              rspf_int32 lr_corner_y,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED)
      :
         theUlCorner(ul_corner_x, ul_corner_y),
         theUrCorner(lr_corner_x, ul_corner_y),
         theLrCorner(lr_corner_x, lr_corner_y),
         theLlCorner(ul_corner_x, lr_corner_y),
         theOrientMode(mode)
      {}
   
   rspfIrect(const rspfIrect& rect)
      :
         theUlCorner(rect.ul()),
         theUrCorner(rect.ur()),
         theLrCorner(rect.lr()),
         theLlCorner(rect.ll()),
         theOrientMode(rect.orientMode())
      {}

   rspfIrect(const rspfDrect& rect);

   /*!
    * Must compute a bounding rect given these image
    * points.
    */
   rspfIrect(const std::vector<rspfIpt>& points,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);

   rspfIrect(const rspfIpt& p1,
              const rspfIpt& p2,
              const rspfIpt& p3,
              const rspfIpt& p4,
              rspfCoordSysOrientMode=RSPF_LEFT_HANDED);

   //! Constructs an Irect surrounding the specified point, and of specified size.
   rspfIrect(const rspfIpt& center, 
              rspf_uint32    size_x, 
              rspf_uint32    size_y,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);

   /** destructor */
   ~rspfIrect();
   
   inline const rspfIrect& operator=  (const rspfIrect& rect);
   const rspfIrect&        operator=  (const rspfDrect& rect);
   inline bool              operator!= (const rspfIrect& rect) const;
   inline bool              operator== (const rspfIrect& rect) const;

   friend rspfIrect operator*(double scalar, const rspfIrect& rect)
      {
         return rspfIrect((int)floor(rect.theUlCorner.x*scalar),
                           (int)floor(rect.theUlCorner.y*scalar),
                           (int)ceil(rect.theUlCorner.x*scalar+rect.width()*scalar-1),
                           (int)ceil(rect.theUlCorner.y*scalar+rect.height()*scalar-1),
                           rect.theOrientMode);
      }

   const rspfIrect& operator*=(rspf_int32 scalar)
   {
      *this = rspfIrect((theUlCorner.x*scalar),
                         (theUlCorner.y*scalar),
                         (theUlCorner.x*scalar+width()*scalar-1),
                         (theUlCorner.y*scalar+height()*scalar-1),
                         theOrientMode);
      return *this;
   }
   
   rspfIrect operator*(rspf_int32 scalar)const
   {
      return rspfIrect((theUlCorner.x*scalar),
                        (theUlCorner.y*scalar),
                        (theUlCorner.x*scalar+width()*scalar-1),
                        (theUlCorner.y*scalar+height()*scalar-1),
                        theOrientMode);
   }
   
   const rspfIrect& operator *=(double scalar)
      {
         *this = rspfIrect((int)floor(theUlCorner.x*scalar),
                            (int)floor(theUlCorner.y*scalar),
                            (int)ceil(theUlCorner.x*scalar+width()*scalar-1),
                            (int)ceil(theUlCorner.y*scalar+height()*scalar-1),
                            theOrientMode);
         return *this;
      }
   
   rspfIrect operator *(double scalar)const
      {
         return rspfIrect((int)floor(theUlCorner.x*scalar),
                           (int)floor(theUlCorner.y*scalar),
                           (int)ceil(theUlCorner.x*scalar+width()*scalar-1),
                           (int)ceil(theUlCorner.y*scalar+height()*scalar-1),
                           theOrientMode);
      }

   const rspfIrect& operator *=(const rspfDpt& scalar)
      {
         *this = rspfIrect((int)floor(theUlCorner.x*scalar.x),
                            (int)floor(theUlCorner.y*scalar.y),
                            (int)ceil(theUlCorner.x*scalar.x+width()*scalar.x-1),
                            (int)ceil(theUlCorner.y*scalar.y+height()*scalar.y-1),
                            theOrientMode);
         return *this;
      }
   
   rspfIrect operator *(const rspfDpt& scalar)const
      {
         return  rspfIrect((int)floor(theUlCorner.x*scalar.x),
                            (int)floor(theUlCorner.y*scalar.y),
                            (int)ceil(theUlCorner.x*scalar.x+width()*scalar.x-1),
                            (int)ceil(theUlCorner.y*scalar.y+height()*scalar.y-1),
                            theOrientMode);
      }
   const rspfIrect& operator +=(const rspfIpt& shift)
      {
         *this = rspfIrect(theUlCorner.x+shift.x,
                            theUlCorner.y+shift.y,
                            theLrCorner.x+shift.x,
                            theLrCorner.y+shift.y,
                            theOrientMode);
         return *this;
      }
   
   const rspfIrect& operator -=(const rspfIpt& shift)
      {
         *this = rspfIrect(theUlCorner.x-shift.x,
                            theUlCorner.y-shift.y,
                            theLrCorner.x-shift.x,
                            theLrCorner.y-shift.y,
                            theOrientMode);
         return *this;
      }
   
   rspfIrect operator +(const rspfIpt& shift)const
      {
         return rspfIrect(theUlCorner.x+shift.x,
                           theUlCorner.y+shift.y,
                           theLrCorner.x+shift.x,
                           theLrCorner.y+shift.y,
                           theOrientMode);
      }

   rspfIrect operator -(const rspfIpt& shift)const
      {
         return rspfIrect(theUlCorner.x-shift.x,
                           theUlCorner.y-shift.y,
                           theLrCorner.x-shift.x,
                           theLrCorner.y-shift.y,
                           theOrientMode);
      }


   const rspfIrect& operator +=(const rspfDpt& shift)
      {
         *this = rspfIrect((int)floor(theUlCorner.x+shift.x),
                            (int)floor(theUlCorner.y+shift.y),
                            (int)ceil(theUlCorner.x+shift.x+width()-1),
                            (int)ceil(theUlCorner.y+shift.y+height()-1),
                            theOrientMode);
         return *this;
      }
   
   const rspfIrect& operator -=(const rspfDpt& shift)
      {
         *this = rspfIrect((int)floor(theUlCorner.x-shift.x),
                            (int)floor(theUlCorner.y-shift.y),
                            (int)ceil(theUlCorner.x-shift.x+width()-1),
                            (int)ceil(theUlCorner.y-shift.y+height()-1),
                            theOrientMode);
         return *this;
      }
   
   rspfIrect operator +(const rspfDpt& shift)const
      {
         return rspfIrect((int)floor(theUlCorner.x+shift.x),
                           (int)floor(theUlCorner.y+shift.y),
                           (int)ceil(theUlCorner.x+shift.x+width()-1),
                           (int)ceil(theUlCorner.y+shift.y+height()-1),
                           theOrientMode);
      }

   rspfIrect operator -(const rspfDpt& shift)const
      {
         return rspfIrect((int)floor(theUlCorner.x-shift.x),
                           (int)floor(theUlCorner.y-shift.y),
                           (int)ceil(theUlCorner.x-shift.x+width()-1),
                           (int)ceil(theUlCorner.y-shift.y+height()-1),
                           theOrientMode);
         return *this;
      }

   
   const rspfIpt& ul() const { return theUlCorner; }
   const rspfIpt& ur() const { return theUrCorner; }
   const rspfIpt& lr() const { return theLrCorner; }
   const rspfIpt& ll() const { return theLlCorner; }

   const rspfIrect& changeOrientationMode(rspfCoordSysOrientMode mode)
   {
      // if we are already in the orientation then return
      //
      if(mode == theOrientMode) return *this;
      if(mode == RSPF_LEFT_HANDED)
      {
         // we must be right handed so change to left handed
         *this = rspfIrect(theUlCorner.x,
                            theLlCorner.y,
                            theLrCorner.x,
                            theUlCorner.y,
                            RSPF_LEFT_HANDED);
      }
      else
      {
         // we must be left handed so change to RIGHT handed
         *this = rspfIrect(theUlCorner.x,
                            theLlCorner.y,
                            theLrCorner.x,
                            theUlCorner.y,
                            RSPF_RIGHT_HANDED);
      }
      theOrientMode = mode;
      
      return *this;
   }
   void getBounds(rspf_int32& minx, rspf_int32& miny,
                  rspf_int32& maxx, rspf_int32& maxy)const
      {
         minx = theUlCorner.x;
         maxx = theLrCorner.x;
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            miny = theUlCorner.y;
            maxy = theLrCorner.y;
         }
         else
         {
            maxy = theUlCorner.y;
            miny = theLrCorner.y;
         }
      }

   /*!
    * Initializes center_point with center of the rectangle.
    * Makes center_point nan if this rectangle has nans.
    */
   void getCenter(rspfDpt& center_point) const;
   
   void makeNan()
   {
      theUlCorner.makeNan();
      theLlCorner.makeNan();
      theLrCorner.makeNan();
      theUrCorner.makeNan();
   }
   
   bool hasNans()const{ return (theUlCorner.hasNans() ||
                                theLlCorner.hasNans() ||
                                theLrCorner.hasNans() ||
                                theUrCorner.hasNans());}
   
   bool isNan()const{ return (theUlCorner.isNan() &&
                              theLlCorner.isNan() &&
                              theLrCorner.isNan() &&
                              theUrCorner.isNan());}

   //***
   // This class supports both left and right-handed coordinate systems. For
   // both, the positive x-axis extends to the "right".
   //***
   rspfCoordSysOrientMode orientMode() const { return theOrientMode; }
   void setOrientMode(rspfCoordSysOrientMode mode) { theOrientMode = mode; }

   void stretchToTileBoundary(const rspfIpt& tileWidthHeight);

   /**
    * @return rspfString representing rspfIrect.
    *
    * Format:  ( 30, -90, 512, 512, [LH|RH] )
    *            -x- -y-  -w-  -h-   -Right or left handed-
    *
    * where:
    *     x and y are origins either upper left if LEFT HANDED (LH) or
    *                                lower left if RIGHT HANDED (RH)
    *     w and h are width and height respectively
    *     The last value is LH or RH to indicate LeftHanded or RightHanded
    *    
    */
   rspfString toString()const;
   
   
   /**
    * expected Format:  form 1: ( 30, -90, 512, 512, [LH|RH] )
    *                            -x- -y-  -w-  -h-   -Right or left handed-
    * 
    *                   form 2: ( 30, -90, 512, 512)
    *                            -x- -y-  -w-  -h-
    *
    * NOTE: Form 2 assumes Left handed were x,y is origin upper left and y positive down.
    *
    * This method starts by doing a "makeNan" on rect. 
    *
    * @param rectString String to initialize from.
    * @return true or false to indicate successful parsing.
    */
   bool toRect(const rspfString& rectString);
   

   const rspfIrect& expand(const rspfIpt& padding);

   //! Guarantees that this rect will be at least w X h big. If smaller than specified, the 
   //! corresponding side will be stretched equally in + and - direction to meet required size.
   //! Returns TRUE if resizing occurred.
   bool insureMinimumSize(const rspfIpt& width_height);

   rspf_uint32 area()const
      {
         return width()*height();
      }

   /*!
    * Sets the upper left corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_ul(const rspfIpt& pt);

   /*!
    * Sets the upper right corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_ur(const rspfIpt& pt);

   /*!
    * Sets the lower right corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_lr(const rspfIpt& pt);

   /*!
    * Sets the lower left corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_ll(const rspfIpt& pt);

   /*!
    * Sets the upper left x.  Adjusts the remaining corners accordingly.
    */
   inline void set_ulx(rspf_int32 x);

   /*!
    * Sets the upper left y.  Adjusts the remaining corners accordingly.
    */
   inline void set_uly(rspf_int32 y);

   /*!
    * Sets the upper right x.  Adjusts the remaining corners accordingly.
    */
   inline void set_urx(rspf_int32 x);

   /*!
    * Sets the upper right y.  Adjusts the remaining corners accordingly.
    */
   inline void set_ury(rspf_int32 y);

   /*!
    * Sets the lower right x.  Adjusts the remaining corners accordingly.
    */
   inline void set_lrx(rspf_int32 x);

   /*!
    * Sets the lower right y.  Adjusts the remaining corners accordingly.
    */
   inline void set_lry(rspf_int32 y);

   /*!
    * Sets the lower left x.  Adjusts the remaining corners accordingly.
    */
   inline void set_llx(rspf_int32 x);

   /*!
    * Sets the lower left y.  Adjusts the remaining corners accordingly.
    */
   inline void set_lly(rspf_int32 y);

   /*!
    * Returns true if "pt" falls within rectangle.  Fall on an edge is also
    * considered to be within.
    */
   inline bool pointWithin(const rspfIpt& pt) const;

   /*!
    * Returns true if any portion of an input rectangle "rect" intersects
    * "this" rectangle.  
    */
   bool intersects(const rspfIrect& rect) const;

   /*!
    * Returns true if "this" rectangle is contained completely within
    * the input rectangular "rect".
    */
   bool completely_within(const rspfIrect& rect) const;

   rspfCoordSysOrientMode orientationMode()const{return theOrientMode;}
   /*!
    * Returns the height of the rectangle.
    */
   rspf_uint32 height() const
   {
      rspf_int32 h = theLlCorner.y - theUlCorner.y;
      if (h < 0)
      {
         h = -h;
      }
      return static_cast<rspf_uint32>( h + 1 );
   }

   /*!
    * Returns the width of a rectangle.
    */
   rspf_uint32 width()  const
   {
      rspf_int32 w = theLrCorner.x - theLlCorner.x;
      if (w < 0)
      {
         w = -w;
      }
      return static_cast<rspf_uint32>( w + 1 );
   }

   rspfIpt size() const { return rspfIpt(width(), height()); }
 
   rspfIrect clipToRect(const rspfIrect& rect)const;

   inline rspfIpt midPoint()const;

   void print(std::ostream& os) const;

   rspfIrect combine(const rspfIrect& rect)const;
   
   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfIrect& rect);

   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
private:

   //***
   // Private data members representing the rectangle corners.
   //***
   rspfIpt theUlCorner;
   rspfIpt theUrCorner;
   rspfIpt theLrCorner;
   rspfIpt theLlCorner;

   rspfCoordSysOrientMode  theOrientMode;
};

//*******************************************************************
// Inline Method: rspfIrect::operator=(rspfIrect)
//*******************************************************************
inline const rspfIrect& rspfIrect::operator=(const rspfIrect& rect)
{
   if (this != &rect)
   {
      theUlCorner   = rect.ul();
      theUrCorner   = rect.ur();
      theLrCorner   = rect.lr();
      theLlCorner   = rect.ll();
      theOrientMode = rect.theOrientMode;

      if(rect.hasNans())
      {
         makeNan();
      }
   }

   return *this;
}

//*******************************************************************
// Inline Method: rspfIrect::operator!=
//*******************************************************************
inline bool rspfIrect::operator!=(const rspfIrect& rect) const
{
   return ( (theUlCorner   != rect.ul()) ||
            (theUrCorner   != rect.ur()) ||
            (theLrCorner   != rect.lr()) ||
            (theLlCorner   != rect.ll()) ||
            (theOrientMode != rect.theOrientMode));
}

//*******************************************************************
// Inline Method: rspfIrect::operator==
//*******************************************************************
inline bool rspfIrect::operator==(const rspfIrect& rect) const
{
   return ( (theUlCorner   == rect.ul()) &&
            (theUrCorner   == rect.ur()) &&
            (theLrCorner   == rect.lr()) &&
            (theLlCorner   == rect.ll()) &&
            (theOrientMode == rect.theOrientMode) );
}

//*******************************************************************
// Inline Method: rspfIrect::set_ul
//*******************************************************************
inline void rspfIrect::set_ul(const rspfIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      theUlCorner   = pt;
      theUrCorner.y = pt.y;
      theLlCorner.x = pt.x;
   }
}

//*******************************************************************
// Inline Method: rspfIrect::set_ur
//*******************************************************************
inline void rspfIrect::set_ur(const rspfIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      theUrCorner   = pt;
      theUlCorner.y = pt.y;
      theLrCorner.x = pt.x;
   }
}

//*******************************************************************
// Inline Method: rspfIrect::set_lr
//*******************************************************************
inline void rspfIrect::set_lr(const rspfIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      theLrCorner   = pt;
      theUrCorner.x = pt.x;
      theLlCorner.y = pt.y;
   }
}

//*******************************************************************
// Inline Method: rspfIrect::set_ll
//*******************************************************************
inline void rspfIrect::set_ll(const rspfIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      theLlCorner   = pt;
      theUlCorner.x = pt.x;
      theLrCorner.y = pt.y;
   }
}

//*******************************************************************
// Inline Method: rspfIrect::set_ulx
//*******************************************************************
inline void rspfIrect::set_ulx(rspf_int32 x)
{
   theUlCorner.x = x;
   theLlCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfIrect::set_uly
//*******************************************************************
inline void rspfIrect::set_uly(rspf_int32 y)
{
   theUlCorner.y = y;
   theUrCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfIrect::set_urx
//*******************************************************************
inline void rspfIrect::set_urx(rspf_int32 x)
{
   theUrCorner.x = x;
   theLrCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfIrect::set_ury
//*******************************************************************
inline void rspfIrect::set_ury(rspf_int32 y)
{
   theUrCorner.y = y;
   theUlCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfIrect::set_lrx
//*******************************************************************
inline void rspfIrect::set_lrx(rspf_int32 x)
{
   theLrCorner.x = x;
   theUrCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfIrect::set_lry
//*******************************************************************
inline void rspfIrect::set_lry(rspf_int32 y)
{
   theLrCorner.y = y;
   theLlCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfIrect::set_llx
//*******************************************************************
inline void rspfIrect::set_llx(rspf_int32 x)
{
   theLlCorner.x = x;
   theUlCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfIrect::set_lly
//*******************************************************************
inline void rspfIrect::set_lly(rspf_int32 y)
{
   theLlCorner.y = y;
   theLrCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfIrect::pointWithin
//*******************************************************************
inline bool rspfIrect::pointWithin(const rspfIpt& pt) const
{
   if(hasNans())
   {
      return false;
   }
   if (theOrientMode == RSPF_LEFT_HANDED)
      return ((pt.x >= ul().x) &&
              (pt.x <= ur().x) &&
              (pt.y >= ul().y) &&
              (pt.y <= ll().y));
   else
      return ((pt.x >= ul().x) &&
              (pt.x <= ur().x) &&
              (pt.y <= ul().y) &&
              (pt.y >= ll().y));
}

//*******************************************************************
// Inline Method: rspfIrect::midPoint
//*******************************************************************
inline rspfIpt rspfIrect::midPoint()const
{
   if(hasNans())
   {
      return rspfIpt(RSPF_INT_NAN, RSPF_INT_NAN);
   }
   double x = (ul().x + ur().x + ll().x + lr().x) * 0.25;
   double y = (ul().y + ur().y + ll().y + lr().y) * 0.25;
   
   return rspfIpt(rspf::round<int>(x),
                   rspf::round<int>(y));
}

#endif
