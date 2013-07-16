//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class definition for rspfIrect.
// 
//*******************************************************************
//  $Id: rspfIrect.cpp 21560 2012-08-30 12:09:03Z gpotts $

#include <ostream>
#include <sstream>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeywordNames.h>

// nonstandard versions that use operator>, so they behave differently
// than std:::min/max and rspf::min/max.  kept here for now for that
// reason.
#ifndef MAX
#  define MAX(x,y) ((x)>(y)?(x):(y))
#  define MIN(x,y) ((x)>(y)?(y):(x))
#endif

rspfIrect::rspfIrect(const rspfDrect& rect)
   :
      theUlCorner(rect.ul()),
      theUrCorner(rect.ur()),
      theLrCorner(rect.lr()),
      theLlCorner(rect.ll()),
      theOrientMode(rect.orientMode())
{}

rspfIrect::rspfIrect(const std::vector<rspfIpt>& points,
                       rspfCoordSysOrientMode mode)
   :
      theOrientMode (mode)
{
   if(points.size())
   {
      rspf_uint32 index;
      rspf_int32  minx = points[0].x;
      rspf_int32  miny = points[0].y;
      rspf_int32  maxx = points[0].x;
      rspf_int32  maxy = points[0].y;
            
      // find the bounds
      for(index = 1; index < points.size();index++)
      {
         minx = rspf::min(minx, points[index].x);
         miny = rspf::min(miny, points[index].y);
         maxx = rspf::max(maxx, points[index].x);
         maxy = rspf::max(maxy, points[index].y);
         
      }
      if(theOrientMode == RSPF_LEFT_HANDED)
      {
         *this = rspfIrect(minx, miny, maxx, maxy, mode);
      }
      else
      {
         *this = rspfIrect(minx,maxy, maxx, miny, mode);
      }
   }
   else
   {
      makeNan();
   }
}

rspfIrect::rspfIrect(const rspfIpt& p1,
                       const rspfIpt& p2,
                       const rspfIpt& p3,
                       const rspfIpt& p4,
                       rspfCoordSysOrientMode mode)
   :theOrientMode(mode)
{
   if(p1.hasNans()||p2.hasNans()||p3.hasNans()||p4.hasNans())
   {
      makeNan();
   }
   else
   {
      rspf_int32 minx, miny;
      rspf_int32 maxx, maxy;
      
      minx = rspf::min( p1.x, rspf::min(p2.x, rspf::min(p3.x, p4.x)));
      miny = rspf::min( p1.y, rspf::min(p2.y, rspf::min(p3.y, p4.y)));
      maxx = rspf::max( p1.x, rspf::max(p2.x, rspf::max(p3.x, p4.x)));
      maxy = rspf::max( p1.y, rspf::max(p2.y, rspf::max(p3.y, p4.y)));
      
      if(theOrientMode == RSPF_LEFT_HANDED)
      {
         *this = rspfIrect(minx, miny, maxx, maxy, mode);
      }
      else
      {
         *this = rspfIrect(minx,maxy, maxx, miny, mode);
      }
   }
}

rspfIrect::~rspfIrect()
{
}

//*******************************************************************
//! Constructs an Irect surrounding the specified point, and of specified size.
//*******************************************************************
rspfIrect::rspfIrect(const rspfIpt& center, 
                       rspf_uint32    size_x, 
                       rspf_uint32    size_y,
                       rspfCoordSysOrientMode mode)
: theOrientMode (mode)
{
   rspf_int32 minx = center.x - size_x/2;
   rspf_int32 maxx = minx + size_x - 1;

   rspf_int32 miny = center.y - size_y/2;
   rspf_int32 maxy = miny + size_y - 1;

   if(mode == RSPF_LEFT_HANDED)
      *this = rspfIrect(minx, miny, maxx, maxy, mode);
   else
      *this = rspfIrect(minx,maxy, maxx, miny, mode);
}


//*************************************************************************************************
//! Guarantees that this rect will be at least w X h big. If smaller than specified, the 
//! corresponding side will be stretched equally in + and - direction to meet required size.
//! Returns TRUE if resizing occurred.
//*************************************************************************************************
bool rspfIrect::insureMinimumSize(const rspfIpt& width_height)
{
   rspfIpt ul (theUlCorner);
   rspfIpt lr (theLrCorner);
   bool resized = false;

   int dx = width_height.x - width();
   int dy = width_height.y - height();

   if (dx > 0)
   {
      dx = (int) ceil((double)dx/2.0);
      ul.x -= dx;
      lr.x += dx;
      resized = true;
   }

   if (dy > 0)
   {
      resized = true;
      if(theOrientMode == RSPF_LEFT_HANDED)
      {
         dy = (int) ceil((double)dy/2.0);
         ul.y -= dy;
         lr.y += dy;
      }
      else
      {
         dy = (int) ceil((double)dy/2.0);
         ul.y += dy;
         lr.y -= dy;
      }
   }

   if (resized)
      *this = rspfIrect(ul, lr, theOrientMode);

   return resized;
}

//*******************************************************************
// Public Method:
//*******************************************************************
bool rspfIrect::intersects(const rspfIrect& rect) const
{
   if(rect.hasNans() || hasNans())
   {
      return false;
   }
   if (theOrientMode != rect.theOrientMode)
      return false;
   
   rspf_int32  ulx = rspf::max(rect.ul().x,ul().x);
   rspf_int32  lrx = rspf::min(rect.lr().x,lr().x);
   rspf_int32  uly, lry;
   bool rtn=false;
   if (theOrientMode == RSPF_LEFT_HANDED)
   {
      uly  = rspf::max(rect.ul().y,ul().y);
      lry  = rspf::min(rect.lr().y,lr().y);
      rtn = ((ulx <= lrx) && (uly <= lry));
   }
   else
   {
      uly  = rspf::max(rect.ll().y,ll().y);
      lry  = rspf::min(rect.ur().y,ur().y);
      rtn = ((ulx <= lrx) && (uly <= lry));
   }
   
   return (rtn);
}

void rspfIrect::stretchToTileBoundary(const rspfIpt& tileWidthHeight)
{
   rspfIpt ul;
   rspfIpt lr;

   if(theOrientMode == RSPF_LEFT_HANDED)
   {
      ul.x = theUlCorner.x;
      if( (theUlCorner.x % tileWidthHeight.x) != 0)
      {
         ul.x = ((ul.x / tileWidthHeight.x))*tileWidthHeight.x;
         if(ul.x > theUlCorner.x)
         {
            ul.x -= tileWidthHeight.x;
         }
      }
      ul.y = theUlCorner.y;
      if( (theUlCorner.y % tileWidthHeight.y) != 0)
      {
         ul.y = ((ul.y / tileWidthHeight.y))*tileWidthHeight.y;
         if(ul.y > theUlCorner.y)
         {
            ul.y -= tileWidthHeight.y;
         }
      }
      rspf_int32 w = (theLrCorner.x - ul.x) + 1;
      rspf_int32 h = (theLrCorner.y - ul.y) + 1;
      
      rspf_int32 nw = (w / tileWidthHeight.x)*tileWidthHeight.x;
      rspf_int32 nh = (h / tileWidthHeight.y)*tileWidthHeight.y;
      
      if(w%tileWidthHeight.x)
      {
         nw += tileWidthHeight.x;
      }
      if(h%tileWidthHeight.y)
      {
         nh += tileWidthHeight.y;
      }
           
      lr.x = ul.x + (nw-1);
      lr.y = ul.y + (nh-1);
   }
   else
   {
      ul.x = theUlCorner.x;
      ul.y = theUlCorner.y;
      if( (theUlCorner.x%tileWidthHeight.x)!= 0)
      {
         ul.x = ((ul.x/ tileWidthHeight.x))*tileWidthHeight.x;
         if(ul.x > theUlCorner.x)
         {
            ul.x -= tileWidthHeight.x;
         }
      }
      if( (theUlCorner.y%tileWidthHeight.y)!=0 )
      {
         ul.y = ((ul.y / tileWidthHeight.y))*tileWidthHeight.y;
         if(ul.y < theUlCorner.y)
         {
            ul.y += tileWidthHeight.y;
         }
      }
      rspf_int32 w = theLrCorner.x - ul.x;
      if (w < 0)
      {
         w = -w;
      }
      w += 1;
      rspf_int32 h = theLrCorner.y - ul.y;
      if (h < 0)
      {
         h = -h;
      }
      h += 1;

      rspf_int32 nw = (w / tileWidthHeight.x)*tileWidthHeight.x;
      rspf_int32 nh = (h / tileWidthHeight.y)*tileWidthHeight.y;
      
      if(w%tileWidthHeight.x)
      {
         nw += tileWidthHeight.x;
      }
      if(h%tileWidthHeight.y)
      {
         nh += tileWidthHeight.y;
      }
           
      lr.x = ul.x + (nw-1);
      lr.y = ul.y - (nh-1);
  }

   *this = rspfIrect(ul, lr, theOrientMode);
}


const rspfIrect& rspfIrect::expand(const rspfIpt& padding)
{
   theUlCorner.x -= padding.x;
   theUrCorner.x += padding.x;
   theLrCorner.x += padding.x;
   theLlCorner.x -= padding.x;
   if(theOrientMode == RSPF_LEFT_HANDED)
   {
      theUlCorner.y -= padding.y;
      theUrCorner.y -= padding.y;
      theLrCorner.y += padding.y;
      theLlCorner.y += padding.y;
   }
   else
   {
      theUlCorner.y += padding.y;
      theUrCorner.y += padding.y;
      theLrCorner.y -= padding.y;
      theLlCorner.y -= padding.y;
   }
   
   return *this;
}

rspfString rspfIrect::toString()const
{
   rspfString result="(";
   
   if(theOrientMode == RSPF_LEFT_HANDED)
   {
      rspfIpt origin = ul();
      result += (rspfString::toString(origin.x) + ",");
      result += (rspfString::toString(origin.y) + ",");
      result += (rspfString::toString(width()) + ",");
      result += (rspfString::toString(height()) + ",");
      result += "LH";
   }
   else 
   {
      rspfIpt origin = ll();
      result += (rspfString::toString(origin.x) + ",");
      result += (rspfString::toString(origin.y) + ",");
      result += (rspfString::toString(width()) + ",");
      result += (rspfString::toString(height()) + ",");
      result += "RH";
   }

   result += ")";
   return result;
}

bool rspfIrect::toRect(const rspfString& rectString)
{
   bool result = false;
   makeNan();
   
   std::istringstream in(rectString);
   rspf::skipws(in);
   char charString[2];
   charString[1] = '\0';
   rspfString interior;
   if(in.peek() == '(')
   {
      in.ignore();
      while((in.peek() != ')')&&
            (in.peek() != '\n') &&
            in.good())
      {
         charString[0] = in.get();
         interior += charString;
      }
      if(in.peek() == ')')
      {
         result = true;
      }
   }
   if(result)
   {
      std::vector<rspfString> splitArray;
      interior.split(splitArray, ",");
      
      // assume left handed
      if(splitArray.size() >= 4)
      {
         rspf_int64 x = splitArray[0].toInt64();
         rspf_int64 y = splitArray[1].toInt64();
         rspf_int64 w = splitArray[2].toInt64();
         rspf_int64 h = splitArray[3].toInt64();
         rspfString orientation = "lh";
         if(splitArray.size() == 5)
         {
            orientation = splitArray[4].downcase();
         }
         if(orientation == "lh")
         {
            // origin upper left
            *this = rspfIrect(x,y,x + (w-1), y+h-1, RSPF_LEFT_HANDED);
         }
         else 
         {
            // origin lower left
            *this = rspfIrect(x,y+(h-1),x + (w-1), y, RSPF_RIGHT_HANDED);
         }
         
      }
      else
      {
         result = false;
      }

   }
   return result;
}

//*******************************************************************
// Public Method:
//*******************************************************************
bool rspfIrect::completely_within(const rspfIrect& rect) const
{
   if(rect.hasNans() || hasNans())
   {
      return false;
   }
   
   if (theOrientMode != rect.theOrientMode)
      return false;
   
   /*  --------------
       |     1      |
       | ---------- |
       | |        | |
       | |        | |
       | |   2    | |
       | |        | |
       | |        | |
       | ---------- |
       |            |
       --------------  */

   bool rtn = true;
   
   if (theUlCorner.x < rect.ul().x)
      rtn = false;
   
   else if (theLrCorner.x > rect.lr().x)
      rtn = false;
   
   else if (theOrientMode == RSPF_LEFT_HANDED)
   {
      if (theUlCorner.y < rect.ul().y)
         rtn = false;
   
      else if (theLrCorner.y > rect.lr().y)
         rtn = false;
   }
   
   else
   {
      if (theUlCorner.y > rect.ul().y)
         rtn = false;
   
      else if (theLrCorner.y < rect.lr().y)
         rtn = false;
   }

   return rtn;
}

//*******************************************************************
// Public Method:
//*******************************************************************
void rspfIrect::print(std::ostream& os) const
{
   os << toString();
}

//*******************************************************************
// friend function: operator<<
//*******************************************************************
std::ostream& operator<<(std::ostream& os, const rspfIrect& rect)
{
   rect.print(os);

   return os;
}

//*******************************************************************
//  Method: rspfIrect::clipToRect
//*******************************************************************
rspfIrect rspfIrect::clipToRect(const rspfIrect& rect)const
{   
   if (theOrientMode != rect.theOrientMode)
      return (*this);

   int x0 = MAX(rect.ul().x, ul().x);
   int x1 = MIN(rect.lr().x, lr().x);
   int y0, y1;

   if(!this->intersects(rect))
   {
      return rspfIrect(RSPF_INT_NAN,
         RSPF_INT_NAN,
         RSPF_INT_NAN,
         RSPF_INT_NAN);

   }
   if (theOrientMode == RSPF_LEFT_HANDED)
   {
      y0 = MAX(rect.ul().y, ul().y);
      y1 = MIN(rect.lr().y, lr().y);

      if( (x1 < x0) || (y1 < y0) )
         return rspfIrect(rspfIpt(0,0), rspfIpt(0,0), theOrientMode);
      else
         return rspfIrect(x0, y0, x1, y1, theOrientMode);
   }
   else
   {
      y1 = MIN(rect.ul().y,ul().y);
      y0 = MAX(rect.lr().y,lr().y);

      if((x1 < x0) || (y1 < y0))
         return rspfIrect(rspfIpt(0,0), rspfIpt(0,0), theOrientMode);
      else
         return rspfIrect(x0, y1, x1, y0, theOrientMode);
   }
}

//*******************************************************************
//  Returns the minimum bounding rect that includes this and arg rect.
//*******************************************************************
rspfIrect rspfIrect::combine(const rspfIrect& rect) const
{   
   // If any rect has NANs, it is assumed uninitialized, so assign the result to just the other
   if (hasNans()) 
      return rect;
   if(rect.hasNans())
      return *this;

   if (theOrientMode != rect.theOrientMode)
      return(*this);

   rspfIpt ulCombine;
   rspfIpt lrCombine;

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

   return rspfIrect(ulCombine, lrCombine, theOrientMode);
}

//*******************************************************************
// Inline Method: rspfIrect::operator=(const rspfDrect& rect)
//*******************************************************************
const rspfIrect& rspfIrect::operator=(const rspfDrect& rect)
{
   theUlCorner   = rect.ul();
   theUrCorner   = rect.ur();
   theLrCorner   = rect.lr();
   theLlCorner   = rect.ll();
   theOrientMode = rect.orientMode();
   
   return *this;
}

bool rspfIrect::saveState(rspfKeywordlist& kwl,
                           const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfIrect",
           true);

   kwl.add(prefix, "rect", toString());
 #if 0  
   if(hasNans())
   {
      kwl.add(prefix,
              "ul_x",
              "nan",
              true);
      kwl.add(prefix,
              "ul_y",
              "nan",
              true);
      kwl.add(prefix,
              "lr_x",
              "nan",
              true);
      kwl.add(prefix,
              "lr_y",
              "nan",
              true);
   }
   else
   {
      kwl.add(prefix,
              "ul_x",
              theUlCorner.x,
              true);
      kwl.add(prefix,
              "ul_y",
              theUlCorner.y,
              true);
      kwl.add(prefix,
              "lr_x",
              theLrCorner.x,
              true);
      kwl.add(prefix,
              "lr_y",
              theLrCorner.y,
              true);
   }
#endif
   return true;
}

bool rspfIrect::loadState(const rspfKeywordlist& kwl,
                           const char* prefix)
{
  makeNan();
  const char* ulx = kwl.find(prefix, "ul_x");
  const char* uly = kwl.find(prefix, "ul_y");
  const char* lrx = kwl.find(prefix, "lr_x");
  const char* lry = kwl.find(prefix, "lr_y");
  const char* rect = kwl.find(prefix, "rect");

  if(ulx&&uly&&lrx&&lry)
  {
    if( (rspfString(ulx).trim().upcase() != "NAN") &&
        (rspfString(uly).trim().upcase() != "NAN") &&
        (rspfString(lrx).trim().upcase() != "NAN") &&
        (rspfString(lry).trim().upcase() != "NAN"))
    {
      *this = rspfIrect(rspfString(ulx).toInt32(),
                         rspfString(uly).toInt32(),
                         rspfString(lrx).toInt32(),
                         rspfString(lry).toInt32());
    }
  }
  else if(rect)
  {
      toRect(rect);
  }
   
   return true;
}

void rspfIrect::getCenter(rspfDpt& center_point) const
{
   if (hasNans())
   {
      center_point.makeNan();
      return;
   }

   double d = (theUlCorner.x + theUrCorner.x + theLrCorner.x + theLlCorner.x);
   center_point.x = d / 4.0;

   d = (theUlCorner.y + theUrCorner.y + theLrCorner.y + theLlCorner.y);
   center_point.y = d / 4.0;
}
