//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for drect.
// Container class for four double points representing a rectangle.
// 
//*******************************************************************
//  $Id: rspfDrect.h 22197 2013-03-12 02:00:55Z dburken $

#ifndef rspfDrect_HEADER
#define rspfDrect_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfFpt.h>

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
class rspfIrect;
class rspfPolygon;
class rspfKeywordlist;

//*******************************************************************
// CLASS:  rspfDrect
//*******************************************************************

class RSPFDLLEXPORT rspfDrect
{
public:
   enum
   {
      UPPER_LEFT  = 1,
      LOWER_LEFT  = 2,
      LOWER_RIGHT = 4,
      UPPER_RIGHT = 8
   };

   rspfDrect()
      :
         theUlCorner(0.0, 0.0),
         theUrCorner(0.0, 0.0),
         theLrCorner(0.0, 0.0),
         theLlCorner(0.0, 0.0),
         theOrientMode(RSPF_LEFT_HANDED)
      {}

   rspfDrect(const rspfDpt&   ul_corner,
              const rspfDpt&   lr_corner,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED)
      :
         theUlCorner(ul_corner),
         theUrCorner(lr_corner.x, ul_corner.y),
         theLrCorner(lr_corner),
         theLlCorner(ul_corner.x, lr_corner.y),
         theOrientMode(mode)
      {
      }

   rspfDrect(const double& ul_corner_x,
              const double& ul_corner_y,
              const double& lr_corner_x,
              const double& lr_corner_y,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED)
      :
         theUlCorner(ul_corner_x, ul_corner_y),
         theUrCorner(lr_corner_x, ul_corner_y),
         theLrCorner(lr_corner_x, lr_corner_y),
         theLlCorner(ul_corner_x, lr_corner_y),
         theOrientMode(mode)
      {}
   
   rspfDrect(const rspfDrect& rect)
      :
         theUlCorner(rect.ul()),
         theUrCorner(rect.ur()),
         theLrCorner(rect.lr()),
         theLlCorner(rect.ll()),
         theOrientMode(rect.orientMode())
      {}

   rspfDrect(const rspfIrect& rect);

   /*!
    * Must compute a bounding rect given a collection of points or polygon:
    */
   rspfDrect(const std::vector<rspfDpt>& points,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);
   rspfDrect(const rspfDpt& p1,
              const rspfDpt& p2,
              const rspfDpt& p3,
              const rspfDpt& p4,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);
   rspfDrect(const rspfPolygon& polygon, 
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);

   /** destructor */
   ~rspfDrect();
   
   //! Constructs an Drect surrounding the specified point, and of specified size.
   rspfDrect(const rspfDpt& center, 
              const double&   size_x, 
              const double&   size_y,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);

   inline const rspfDrect& operator=  (const rspfDrect& rect);
   const rspfDrect&        operator=  (const rspfIrect& rect);
   inline bool              operator!= (const rspfDrect& rect) const;
   inline bool              operator== (const rspfDrect& rect) const;

   friend rspfDrect operator*(double scalar, const rspfDrect& rect)
   {
      rspfDpt ul(rect.theUlCorner.x*scalar,
                  rect.theUlCorner.y*scalar);
      
      if(rect.theOrientMode == RSPF_LEFT_HANDED)
      {
         return rspfDrect(ul.x,
                           ul.y,
                           ul.x+rect.width()*scalar-1,
                           ul.y+rect.height()*scalar-1,
                           rect.theOrientMode);
      }
      return rspfDrect(ul.x,
                        ul.y,
                        ul.x+rect.width()*scalar-1,
                        ul.y-(rect.height()*scalar-1),
                        rect.theOrientMode);
      
   }
   
   const rspfDrect& operator *=(double scalar)
      {
          rspfDpt ul(theUlCorner.x*scalar,
                      theUlCorner.y*scalar);

          if(theOrientMode == RSPF_LEFT_HANDED)
          {
             *this = rspfDrect(ul.x,
                                ul.y,
                                (ul.x+width()*scalar-1),
                                (ul.y+height()*scalar-1),
                                theOrientMode);
          }
          else
          {
             *this = rspfDrect(ul.x,
                                ul.y,
                                (ul.x+width()*scalar-1),
                                (ul.y-(height()*scalar-1)),
                                theOrientMode);
                                
          }
         return *this;
      }
   
   rspfDrect operator *(double scalar)const
      {
         rspfDpt ul((theUlCorner.x*scalar),
                     (theUlCorner.y*scalar));

         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            return rspfDrect(ul.x,
                              ul.y,
                              (ul.x+width()*scalar-1),
                              (ul.y+height()*scalar-1),
                              theOrientMode);
         }

         return rspfDrect(ul.x,
                           ul.y,
                           (ul.x+width()*scalar-1),
                           ul.y-(height()*scalar-1),
                           theOrientMode);
      }
   const rspfDrect& operator *=(const rspfDpt& scalar)
      {
         rspfDpt ul((theUlCorner.x*scalar.x),
                     (theUlCorner.y*scalar.y));
         
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            *this = rspfDrect(ul.x,
                               ul.y,
                               (ul.x+width()*scalar.x - 1),
                               (ul.y+height()*scalar.y - 1),
                               theOrientMode);
         }
         else
         {
            *this = rspfDrect(ul.x,
                               ul.y,
                               (ul.x+width()*scalar.x - 1),
                               (ul.y-(height()*scalar.y - 1)),
                               theOrientMode);            
         }
         return *this;
      }
   
   rspfDrect operator *(const rspfDpt& scalar)const
      {
         rspfDpt ul((theUlCorner.x*scalar.x),
                     (theUlCorner.y*scalar.y));
         
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            return rspfDrect(ul.x,
                              ul.y,
                              (ul.x+width()*scalar.x-1),
                              (ul.y+height()*scalar.y-1),
                              theOrientMode);
         }
         return rspfDrect(ul.x,
                           ul.y,
                           (ul.x+width()*scalar.x-1),
                           (ul.y-(height()*scalar.y-1)),
                           theOrientMode);
      }
   const rspfDrect& operator +=(const rspfDpt& shift)
      {
         rspfDpt ul((theUlCorner.x+shift.x),
                     (theUlCorner.y+shift.y));
                     
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            *this = rspfDrect(ul.x,
                               ul.y,
                               ul.x+width()-1,
                               ul.y+height()-1,
                               theOrientMode);
         }
         else
         {
            *this = rspfDrect(ul.x,
                               ul.y,
                               ul.x+width()-1,
                               ul.y-(height()-1),
                               theOrientMode);            
         }
         return *this;
      }
   
   const rspfDrect& operator -=(const rspfDpt& shift)
      {
         rspfDpt ul((theUlCorner.x-shift.x),
                     (theUlCorner.y-shift.y));
         
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            *this = rspfDrect(ul.x,
                               ul.y,
                               ul.x+width()-1,
                               ul.y+height()-1,
                               theOrientMode);
         }
         else
         {
            *this = rspfDrect(ul.x,
                               ul.y,
                               ul.x+width()-1,
                               ul.y-(height()-1),
                               theOrientMode);
         }
         return *this;
      }
   
   rspfDrect operator +(const rspfDpt& shift)const
      {
         rspfDpt ul((theUlCorner.x+shift.x),
                     (theUlCorner.y+shift.y));
                     
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            return rspfDrect(ul.x,
                              ul.y,
                              ul.x+width()-1,
                              ul.y+height()-1,
                              theOrientMode);
         }
         else
         {
            return rspfDrect(ul.x,
                              ul.y,
                              ul.x+width()-1,
                              ul.y-(height()-1),
                              theOrientMode);
         }
      }  

   rspfDrect operator -(const rspfDpt& shift)const
      {
         rspfIpt ul(rspf::round<int>(theUlCorner.x-shift.x),
                     rspf::round<int>(theUlCorner.y-shift.y));
                     
         if(theOrientMode == RSPF_LEFT_HANDED)
         {
            return rspfDrect(ul.x,
                              ul.y,
                              ul.x+width()-1,
                              ul.y+height()-1,
                              theOrientMode);
         }
         else
         {
            return rspfDrect(ul.x,
                              ul.y,
                              ul.x+width()-1,
                              ul.y-(height()-1),
                              theOrientMode);            
         }
      }  

   const rspfDpt& ul() const { return theUlCorner; }
   const rspfDpt& ur() const { return theUrCorner; }
   const rspfDpt& lr() const { return theLrCorner; }
   const rspfDpt& ll() const { return theLlCorner; }

   const rspfDrect& changeOrientationMode(rspfCoordSysOrientMode mode)
   {
      // if we are already in the orientation then return
      //
      if(mode == theOrientMode) return *this;
      if(mode == RSPF_LEFT_HANDED)
      {
         // we must be right handed so change to left handed
         *this = rspfDrect(theUlCorner.x,
                            theLlCorner.y,
                            theLrCorner.x,
                            theUlCorner.y,
                            RSPF_LEFT_HANDED);
      }
      else
      {
         // we must be left handed so change to RIGHT handed
         *this = rspfDrect(theUlCorner.x,
                            theLlCorner.y,
                            theLrCorner.x,
                            theUlCorner.y,
                            RSPF_RIGHT_HANDED);
      }
      theOrientMode = mode;
      
      return *this;
   }
   
   void getBounds(double& minx, double& miny,
                  double& maxx, double& maxy)const
      {
         minx = theUlCorner.x;
         maxx = theUrCorner.x;
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
   
   bool isNan()const{ return (theUlCorner.hasNans() &&
                              theLlCorner.hasNans() &&
                              theLrCorner.hasNans() &&
                              theUrCorner.hasNans());}

   double area()const
      {
         return width()*height();
      }
   //***
   // This class supports both left and right-handed coordinate systems. For
   // both, the positive x-axis extends to the "right".
   //***
   rspfCoordSysOrientMode orientMode() const { return theOrientMode; }
   void setOrientMode(rspfCoordSysOrientMode mode) { theOrientMode = mode; }
   
   /*!
    * Sets the upper left corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_ul(const rspfDpt& pt);

   /*!
    * Sets the upper right corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_ur(const rspfDpt& pt);

   /*!
    * Sets the lower right corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_lr(const rspfDpt& pt);

   /*!
    * Sets the lower left corner to "pt".  Adjusts the remaining corners
    * accordingly.
    */
   inline void set_ll(const rspfDpt& pt);

   /*!
    * Sets the upper left x.  Adjusts the remaining corners accordingly.
    */
   inline void set_ulx(rspf_float64 x);

   /*!
    * Sets the upper left y.  Adjusts the remaining corners accordingly.
    */
   inline void set_uly(rspf_float64 y);

   /*!
    * Sets the upper right x.  Adjusts the remaining corners accordingly.
    */
   inline void set_urx(rspf_float64 x);

   /*!
    * Sets the upper right y.  Adjusts the remaining corners accordingly.
    */
   inline void set_ury(rspf_float64 y);

   /*!
    * Sets the lower right x.  Adjusts the remaining corners accordingly.
    */
   inline void set_lrx(rspf_float64 x);

   /*!
    * Sets the lower right y.  Adjusts the remaining corners accordingly.
    */
   inline void set_lry(rspf_float64 y);

   /*!
    * Sets the lower left x.  Adjusts the remaining corners accordingly.
    */
   inline void set_llx(rspf_float64 x);

   /*!
    * Sets the lower left y.  Adjusts the remaining corners accordingly.
    */
   inline void set_lly(rspf_float64 y);

   /*!
    * METHOD: initBoundingRect(points)
    * Initializes this rect to the bounding rect containing all points in the
    * collection passed in.
    */
   void initBoundingRect(const std::vector<rspfDpt>& points);

   /*!
    * Returns true if "pt" falls within rectangle.  Fall on an edge is also
    * considered to be within.  The edge is expanded by epsilon value so any value
    * within epsilon is inside
    */
   bool pointWithin(const rspfDpt& pt, double epsilon=0.0) const;

   /*!
    * Returns true if "pt" falls within rectangle.  Fall on an edge is also
    * considered to be within.
    */
   bool pointWithin(const rspfFpt& pt, double epsilon=0.0) const;

   /*!
    * Returns true if any portion of an input rectangle "rect" intersects
    * "this" rectangle.  
    */
   bool intersects(const rspfDrect& rect) const;

   /*!
    * Returns true if "this" rectangle is contained completely within the
    * input rectangle "rect".
    */
   bool completely_within(const rspfDrect& rect) const;

   rspfCoordSysOrientMode orientationMode()const{return theOrientMode;}
   /*!
    * Returns the height of a rectangle.
    */
   rspf_float64 height() const { return fabs(theLlCorner.y - theUlCorner.y) + 1.0; }

   /*!
    * Returns the width of a rectangle.
    */
   rspf_float64 width()  const { return fabs(theLrCorner.x - theLlCorner.x) + 1.0; }

   rspfDpt size() const { return rspfDpt(width(), height()); }

   /*!
    * Stretches this rectangle out to integer boundaries.
    */
   void stretchOut();

   /*!
    * Will stretch the rect to the passed in tiled boundary.
    */
   void stretchToTileBoundary(const rspfDpt& widthHeight);

   const rspfDrect& expand(const rspfDpt& padding);

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

   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);

   /*!
    * Will subdivide this rect into four partitions.
    */
   void splitToQuad(rspfDrect& ulRect,
                    rspfDrect& urRect,
                    rspfDrect& lrRect,
                    rspfDrect& llRect);
   
   /*!
   * Finds the point on the rect boundary that is closest to the arg_point. Closest is defined as
   * the minimum perpendicular distance.
   */
   rspfDpt findClosestEdgePointTo(const rspfDpt& arg_point) const;
   
   rspfDrect clipToRect(const rspfDrect& rect)const;

   inline rspfDpt midPoint()const;

   void print(std::ostream& os) const;

   rspfDrect combine(const rspfDrect& rect)const;
   
   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfDrect& rect);

   bool clip(rspfDpt &p1,
             rspfDpt  &p2)const;

   static long getCode(const rspfDpt& aPoint,
                       const rspfDrect& clipRect);
private:

   enum rspfCohenSutherlandClipCodes
   {
      NONE   = 0,
      LEFT   = 1,
      RIGHT  = 2,
      BOTTOM = 4,
      TOP    = 8
   };
   
   //***
   // Private data members representing the rectangle corners.
   //***
   rspfDpt theUlCorner;
   rspfDpt theUrCorner;
   rspfDpt theLrCorner;
   rspfDpt theLlCorner;

   rspfCoordSysOrientMode  theOrientMode;
};

//*******************************************************************
// Inline Method: rspfDrect::operator=(rspfDrect)
//*******************************************************************
inline const rspfDrect& rspfDrect::operator=(const rspfDrect& rect)
{
   if (this != &rect)
   {
      theUlCorner   = rect.theUlCorner;
      theUrCorner   = rect.theUrCorner;
      theLrCorner   = rect.theLrCorner;
      theLlCorner   = rect.theLlCorner;
      theOrientMode = rect.theOrientMode;
   }

   return *this;
}

//*******************************************************************
// Inline Method: rspfDrect::operator!=
//*******************************************************************
inline bool rspfDrect::operator!=(const rspfDrect& rect) const
{
   return ( (theUlCorner   != rect.theUlCorner) ||
            (theUrCorner   != rect.theUrCorner) ||
            (theLrCorner   != rect.theLrCorner) ||
            (theLlCorner   != rect.theLlCorner) ||
            (theOrientMode != rect.theOrientMode));
}

//*******************************************************************
// Inline Method: rspfDrect::operator==
//*******************************************************************
inline bool rspfDrect::operator==(const rspfDrect& rect) const
{
   return ( (theUlCorner   == rect.theUlCorner) &&
            (theUrCorner   == rect.theUrCorner) &&
            (theLrCorner   == rect.theLrCorner) &&
            (theLlCorner   == rect.theLlCorner) &&
            (theOrientMode == rect.theOrientMode));
}

//*******************************************************************
// Inline Method: rspfDrect::set_ul
//*******************************************************************
inline void rspfDrect::set_ul(const rspfDpt& pt)
{
   theUlCorner   = pt;
   theUrCorner.y = pt.y;
   theLlCorner.x = pt.x;
}

//*******************************************************************
// Inline Method: rspfDrect::set_ur
//*******************************************************************
inline void rspfDrect::set_ur(const rspfDpt& pt)
{
   theUrCorner   = pt;
   theUlCorner.y = pt.y;
   theLrCorner.x = pt.x;
}

//*******************************************************************
// Inline Method: rspfDrect::set_lr
//*******************************************************************
inline void rspfDrect::set_lr(const rspfDpt& pt)
{
   theLrCorner   = pt;
   theUrCorner.x = pt.x;
   theLlCorner.y = pt.y;
}

//*******************************************************************
// Inline Method: rspfDrect::set_ll
//*******************************************************************
inline void rspfDrect::set_ll(const rspfDpt& pt)
{
   theLlCorner   = pt;
   theUlCorner.x = pt.x;
   theLrCorner.y = pt.y;
}

//*******************************************************************
// Inline Method: rspfDrect::set_ulx
//*******************************************************************
inline void rspfDrect::set_ulx(rspf_float64 x)
{
   theUlCorner.x = x;
   theLlCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfDrect::set_uly
//*******************************************************************
inline void rspfDrect::set_uly(rspf_float64 y)
{
   theUlCorner.y = y;
   theUrCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfDrect::set_urx
//*******************************************************************
inline void rspfDrect::set_urx(rspf_float64 x)
{
   theUrCorner.x = x;
   theLrCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfDrect::set_ury
//*******************************************************************
inline void rspfDrect::set_ury(rspf_float64 y)
{
   theUrCorner.y = y;
   theUlCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfDrect::set_lrx
//*******************************************************************
inline void rspfDrect::set_lrx(rspf_float64 x)
{
   theLrCorner.x = x;
   theUrCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfDrect::set_lry
//*******************************************************************
inline void rspfDrect::set_lry(rspf_float64 y)
{
   theLrCorner.y = y;
   theLlCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfDrect::set_llx
//*******************************************************************
inline void rspfDrect::set_llx(rspf_float64 x)
{
   theLlCorner.x = x;
   theUlCorner.x = x;
}

//*******************************************************************
// Inline Method: rspfDrect::set_lly
//*******************************************************************
inline void rspfDrect::set_lly(rspf_float64 y)
{
   theLlCorner.y = y;
   theLrCorner.y = y;
}

//*******************************************************************
// Inline Method: rspfDrect::pointWithin(const rspfDpt& pt) 
//*******************************************************************
inline bool rspfDrect::pointWithin(const rspfDpt& pt, double epsilon) const
{
   if (theOrientMode == RSPF_LEFT_HANDED)
   {
      return ((pt.x >= (ul().x-epsilon)) &&
              (pt.x <= (ur().x+epsilon)) &&
              (pt.y >= (ul().y-epsilon)) &&
              (pt.y <= (ll().y+epsilon)));
   }
   return ((pt.x >= (ul().x-epsilon)) &&
           (pt.x <= (ur().x+epsilon)) &&
           (pt.y <= (ul().y+epsilon)) &&
           (pt.y >= (ll().y-epsilon)));
}

//*******************************************************************
// Inline Method: rspfDrect::pointWithin(const rspfFpt& pt)
//*******************************************************************
inline bool rspfDrect::pointWithin(const rspfFpt& pt, double epsilon) const
{
   if (theOrientMode == RSPF_LEFT_HANDED)
   {
      return ((pt.x >= (ul().x-epsilon)) &&
              (pt.x <= (ur().x+epsilon)) &&
              (pt.y >= (ul().y-epsilon)) &&
              (pt.y <= (ll().y+epsilon)));
   }
   return ((pt.x >= (ul().x-epsilon)) &&
           (pt.x <= (ur().x+epsilon)) &&
           (pt.y <= (ul().y+epsilon)) &&
           (pt.y >= (ll().y-epsilon)));
}

//*******************************************************************
// Inline Method: rspfDrect::midPoint()
//*******************************************************************
inline rspfDpt rspfDrect::midPoint()const
{
   return rspfDpt( (ul().x + ur().x + ll().x + lr().x)*.25,
                    (ul().y + ur().y + ll().y + lr().y)*.25);
}

//*******************************************************************
// Inline Method: rspfDrect::combine(const rspfDrect& rect)
//*******************************************************************
inline rspfDrect rspfDrect::combine(const rspfDrect& rect)const
{
   if(rect.hasNans() || hasNans())
   {
      rspfDrect result;

      result.makeNan();

      return result;
   }
   if (theOrientMode != rect.theOrientMode)
      return(*this);
   
   rspfDpt ulCombine;
   rspfDpt lrCombine;

   if(theOrientMode == RSPF_LEFT_HANDED)
   {
      ulCombine.x = ((ul().x <= rect.ul().x)?ul().x:rect.ul().x);
      ulCombine.y = ((ul().y <= rect.ul().y)?ul().y:rect.ul().y);
      lrCombine.x = ((lr().x >= rect.lr().x)?lr().x:rect.lr().x);
      lrCombine.y = ((lr().y >= rect.lr().y)?lr().y:rect.lr().y);
   }
   else
   {
      ulCombine.x = ((ul().x <= rect.ul().x)?ul().x:rect.ul().x);
      ulCombine.y = ((ul().y >= rect.ul().y)?ul().y:rect.ul().y);
      lrCombine.x = ((lr().x >= rect.lr().x)?lr().x:rect.lr().x);
      lrCombine.y = ((lr().y <= rect.lr().y)?lr().y:rect.lr().y);
   }

   return rspfDrect(ulCombine, lrCombine, theOrientMode);
}
#endif
