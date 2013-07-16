//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Contains class definition for rspfDrect.
//*******************************************************************
//  $Id: rspfDrect.cpp 21560 2012-08-30 12:09:03Z gpotts $

#include <iostream>
#include <sstream>

#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>

// XXX not replaced with std::max since the test is backward here
//     and will give a different answer in the case of nan.
#define d_MAX(a,b)      (((a)>(b)) ? a : b)

static int
clip_1d (double *x0, 
	 double *y0, 
	 double *x1, 
	 double *y1, 
	 double maxdim)
{
   double m;			/* gradient of line */
   if (*x0 < 0)
   {				/* start of line is left of window */
      if (*x1 < 0)		/* as is the end, so the line never cuts the window */
         return 0;
      m = (*y1 - *y0) / (double) (*x1 - *x0);	/* calculate the slope of the line */
      /* adjust x0 to be on the left boundary (ie to be zero), and y0 to match */
      *y0 -= m * *x0;
      *x0 = 0;
      /* now, perhaps, adjust the far end of the line as well */
      if (*x1 > maxdim)
      {
         *y1 += m * (maxdim - *x1);
         *x1 = maxdim;
      }
      return 1;
   }
   if (*x0 > maxdim)
   {				/* start of line is right of window -
				   complement of above */
      if (*x1 > maxdim)		/* as is the end, so the line misses the window */
         return 0;
      m = (*y1 - *y0) / (double) (*x1 - *x0);	/* calculate the slope of the line */
      *y0 += m * (maxdim - *x0);	/* adjust so point is on the right
					   boundary */
      *x0 = maxdim;
      /* now, perhaps, adjust the end of the line */
      if (*x1 < 0)
      {
         *y1 -= m * *x1;
         *x1 = 0;
      }
      return 1;
   }
   /* the final case - the start of the line is inside the window */
   if (*x1 > maxdim)
   {				/* other end is outside to the right */
      m = (*y1 - *y0) / (double) (*x1 - *x0);	/* calculate the slope of the line */
      *y1 += m * (maxdim - *x1);
      *x1 = maxdim;
      return 1;
   }
   if (*x1 < 0)
   {				/* other end is outside to the left */
      m = (*y1 - *y0) / (double) (*x1 - *x0);	/* calculate the slope of the line */
      *y1 -= m * *x1;
      *x1 = 0;
      return 1;
   }
   /* only get here if both points are inside the window */
   return 1;
}

//*******************************************************************
// Public Constructor: rspfDrect
//
//*******************************************************************
rspfDrect::rspfDrect(const rspfIrect& rect)
   :
      theUlCorner(rect.ul()),
      theUrCorner(rect.ur()),
      theLrCorner(rect.lr()),
      theLlCorner(rect.ll()),
      theOrientMode(rect.orientMode())
{
   if(rect.isNan())
   {
      makeNan();
   }
}


//*****************************************************************************
//  CONSTRUCTOR: rspfDrect(const vector<rspfDpt>& points)
//  
//*****************************************************************************
rspfDrect::rspfDrect(const rspfPolygon& polygon,
                       rspfCoordSysOrientMode mode)
   :
      theOrientMode (mode)
{
   std::vector<rspfDpt> vertices;
   rspfDpt point;
   int index = 0;
   while (polygon.vertex(index, point))
   {
      vertices.push_back(point);
      index++;
   }

   initBoundingRect(vertices);
}

//*****************************************************************************
//  CONSTRUCTOR: rspfDrect(const vector<rspfDpt>& points)
//  
//*****************************************************************************
rspfDrect::rspfDrect(const std::vector<rspfDpt>& points,
                       rspfCoordSysOrientMode mode)
   :
      theOrientMode (mode)
{
  if(points.size())
   {
      unsigned long index;
      double minx, miny;
      double maxx, maxy;
      
      minx = points[0].x;
      miny = points[0].y;
      maxx = points[0].x;
      maxy = points[0].y;
            
      // find the bounds
      for(index = 1; index < points.size();index++)
      {
         
         minx = std::min(minx, points[index].x);
         miny = std::min(miny, points[index].y);
         maxx = std::max(maxx, points[index].x);
         maxy = std::max(maxy, points[index].y);
         
      }
      if(theOrientMode == RSPF_LEFT_HANDED)
      {
         *this = rspfDrect(minx, miny, maxx, maxy, mode);
      }
      else
      {
         *this = rspfDrect(minx,maxy, maxx, miny, mode);
      }
   }
   else
   {
      makeNan();
   }
}
rspfDrect::rspfDrect(const rspfDpt& p1,
                       const rspfDpt& p2,
                       const rspfDpt& p3,
                       const rspfDpt& p4,
                       rspfCoordSysOrientMode mode)
: theOrientMode(mode)
{
   if(p1.hasNans()||p2.hasNans()||p3.hasNans()||p4.hasNans())
   {
      makeNan();
   }
   else
   {
      double minx, miny;
      double maxx, maxy;
      
      minx = std::min( p1.x, std::min(p2.x, std::min(p3.x, p4.x)));
      miny = std::min( p1.y, std::min(p2.y, std::min(p3.y, p4.y)));
      maxx = std::max( p1.x, std::max(p2.x, std::max(p3.x, p4.x)));
      maxy = std::max( p1.y, std::max(p2.y, std::max(p3.y, p4.y)));
      
      if(mode == RSPF_LEFT_HANDED)
      {
         *this = rspfDrect(minx, miny, maxx, maxy, mode);
      }
      else
      {            
         *this = rspfDrect(minx,maxy, maxx, miny, mode);
      }
   }
}


//*******************************************************************
//! Constructs an rspfDrect surrounding the specified point, and of specified size.
//*******************************************************************
rspfDrect::rspfDrect(const rspfDpt& center, 
                       const double&   size_x, 
                       const double&   size_y,
                       rspfCoordSysOrientMode mode)
: theOrientMode(mode)
{
   double dx = fabs(size_x);
   double dy = fabs(size_y);

   double minx = center.x - dx/2.0;
   double maxx = minx + dx;

   double miny = center.y - dy/2.0;
   double maxy = miny + dy;

   if(mode == RSPF_LEFT_HANDED)
      *this = rspfDrect(minx, miny, maxx, maxy, mode);
   else
      *this = rspfDrect(minx,maxy, maxx, miny, mode);
}

rspfDrect::~rspfDrect()
{
}

void rspfDrect::initBoundingRect(const std::vector<rspfDpt>& points)
{
   unsigned long index;

   // initialize everyone to the first point
   if(points.size() > 0)
   {
      theUlCorner.x = points[0].x;
      theUlCorner.y = points[0].y;
      theLrCorner.x = theUlCorner.x;
      theLrCorner.y = theUlCorner.y;
   }
   
   // find the bounds
   for(index = 1; index < points.size();index++)
   {
      // find left most
      if(points[index].x < theUlCorner.x)
         theUlCorner.x = points[index].x;

      // find right most
      else if(points[index].x > theLrCorner.x)
         theLrCorner.x = points[index].x;

      if (theOrientMode == RSPF_LEFT_HANDED)
      {
         //find top most
         if(points[index].y < theUlCorner.y)
            theUlCorner.y = points[index].y;

         // find bottom most
         else if(points[index].y > theLrCorner.y)
            theLrCorner.y = points[index].y;
      }

      else // right handed coord system
      {
         if(points[index].y > theUlCorner.y)
            theUlCorner.y = points[index].y;

         // find bottom most
         else if(points[index].y < theLrCorner.y)
            theLrCorner.y = points[index].y;
      }
   }

   // now set the other points for the rect.
   theUrCorner.x = theLrCorner.x;
   theUrCorner.y = theUlCorner.y;
   theLlCorner.x = theUlCorner.x;
   theLlCorner.y = theLrCorner.y;
}

//*******************************************************************
// Public Method:
//*******************************************************************
bool rspfDrect::intersects(const rspfDrect& rect) const
{
   if(rect.hasNans() || hasNans())
   {
      return false;
   }
   if (theOrientMode != rect.theOrientMode)
      return false;
   
   rspf_float64  ulx = rspf::max(rect.ul().x,ul().x);
   rspf_float64  lrx = rspf::min(rect.lr().x,lr().x);
   rspf_float64  uly, lry;
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

//*******************************************************************
// Public Method: rspfDrect::completely_within
//*******************************************************************
bool rspfDrect::completely_within(const rspfDrect& rect) const
{
   if(hasNans() || rect.hasNans())
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
// Public Method: rspfDrect::stretchOut
//*******************************************************************
void rspfDrect::stretchOut()
{
   set_ulx(floor(theUlCorner.x));
   set_lrx(ceil(theLrCorner.x));

   if (theOrientMode == RSPF_LEFT_HANDED)
   {
      set_uly(floor(theUlCorner.y));
      set_lry(ceil(theLrCorner.y));
   }
   else
   {
      set_uly(ceil(theUlCorner.y));
      set_lry(floor(theLrCorner.y));
   }
}

void rspfDrect::stretchToTileBoundary(const rspfDpt& widthHeight)
{
   rspfDpt ul;
   rspfDpt lr;
   rspf_int32 evenDivision=0;

   if(theOrientMode == RSPF_LEFT_HANDED)
   {
      ul.x = theUlCorner.x;
      if( fmod(theUlCorner.x, widthHeight.x) != 0)
      {
         ul.x = ((long)(ul.x / widthHeight.x))*widthHeight.x;
         if(ul.x > theUlCorner.x)
         {
            ul.x -= widthHeight.x;
         }
      }
      ul.y = theUlCorner.y;
      if( fmod(theUlCorner.y, widthHeight.y) != 0)
      {
         ul.y = ((long)(ul.y / widthHeight.y))*widthHeight.y;
         if(ul.y > theUlCorner.y)
         {
            ul.y -= widthHeight.y;
         }
      }
      
      evenDivision = fmod(theLrCorner.x, widthHeight.x) == 0;
      lr.x = theLrCorner.x;
      if(!evenDivision)
      {
         lr.x = ((long)((lr.x)/widthHeight.x)) * widthHeight.x;
         if(lr.x < theLrCorner.x)
         {
            lr.x += widthHeight.x;
         }
      }

      evenDivision = fmod(theLrCorner.y, widthHeight.y) == 0;
      lr.y = theLrCorner.y;
      if(!evenDivision)
      {
         lr.y = ((long)(lr.y/widthHeight.y)) * widthHeight.y;
         if(lr.y < theLrCorner.y)
         {
            lr.y += widthHeight.y;
         }
      }
   }
   else
   {
      ul.x = theUlCorner.x;
      ul.y = theUlCorner.y;
      if( !rspf::almostEqual(fmod(theUlCorner.x, widthHeight.x), 0.0))
      {
         ul.x = ((long)(ul.x/ widthHeight.x))*widthHeight.x;
         if(ul.x > theUlCorner.x)
         {
            ul.x -= widthHeight.x;
         }
      }
      if( !rspf::almostEqual((double)fmod(theUlCorner.y, widthHeight.y), 0.0) )
      {
         ul.y = ((long)(ul.y / widthHeight.y))*widthHeight.y;
         if(ul.y < theUlCorner.y)
         {
            ul.y += widthHeight.y;
         }
      }
      
      evenDivision = rspf::almostEqual( fmod(theLrCorner.x, widthHeight.x), 0.0);
      lr.x = theLrCorner.x;
      if(!evenDivision)
      {
         lr.x = ((long)(lr.x/widthHeight.x)) * widthHeight.x;
         if(lr.x < theLrCorner.x)
         {
            lr.x += widthHeight.x;
         }
      }

      evenDivision = rspf::almostEqual(fmod(theLrCorner.y, widthHeight.y), 0.0);
      lr.y = theLrCorner.y;
      if(!evenDivision)
      {
         lr.y = ((long)(lr.y/widthHeight.y)) * widthHeight.y;

         if(lr.y > theLrCorner.y)
         {
            lr.y -= widthHeight.y;
         }
      }
  }

   *this = rspfDrect(ul, lr, theOrientMode);
}

const rspfDrect& rspfDrect::expand(const rspfDpt& padding)
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
rspfString rspfDrect::toString()const
{
   rspfString result="(";
   
   if(theOrientMode == RSPF_LEFT_HANDED)
   {
      rspfDpt origin = ul();
      result += (rspfString::toString(origin.x) + ",");
      result += (rspfString::toString(origin.y) + ",");
      result += (rspfString::toString(width()) + ",");
      result += (rspfString::toString(height()) + ",");
      result += "LH";
   }
   else 
   {
      rspfDpt origin = ll();
      result += (rspfString::toString(origin.x) + ",");
      result += (rspfString::toString(origin.y) + ",");
      result += (rspfString::toString(width()) + ",");
      result += (rspfString::toString(height()) + ",");
      result += "RH";
   }
   
   result += ")";
   return result;
}

bool rspfDrect::toRect(const rspfString& rectString)
{
   bool result = false;
   
   
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
         rspf_float64 x = splitArray[0].toDouble();
         rspf_float64 y = splitArray[1].toDouble();
         rspf_float64 w = splitArray[2].toDouble();
         rspf_float64 h = splitArray[3].toDouble();
         rspfString orientation = "lh";
         if(splitArray.size() == 5)
         {
            orientation = splitArray[4].downcase();
         }
         if(orientation == "lh")
         {
            // origin upper left
            *this = rspfDrect(x,y,x + (w-1), y+h-1, RSPF_LEFT_HANDED);
         }
         else 
         {
            // origin lower left so construct and make an upper left
            *this = rspfDrect(x,y+(h-1),x + (w-1), y, RSPF_RIGHT_HANDED);
         }
      }
      else
      {
         result = false;
      }
      
   }
   return result;
}

bool rspfDrect::saveState(rspfKeywordlist& kwl,
                           const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfDrect",
           true);

   kwl.add(prefix, "rect", toString());

   return true;
}

bool rspfDrect::loadState(const rspfKeywordlist& kwl,
                           const char* prefix)
{
  const char* rect = kwl.find(prefix, "rect");
  makeNan();

  if(rect)
  {
      toRect(rect);
  }
   
   return true;
}

//*******************************************************************
// Public Method: rspfDrect::print
//*******************************************************************
void rspfDrect::print(std::ostream& os) const
{
   os << toString();
}

//*******************************************************************
// friend function: operator<<
//*******************************************************************
std::ostream& operator<<(std::ostream& os, const rspfDrect& rect)
{
   rect.print(os);

   return os;
}

//*******************************************************************
// Public Method: rspfDrect::clip
//*******************************************************************
bool rspfDrect::clip(rspfDpt &p1, rspfDpt &p2)const
{
   if(p1.isNan() || p2.isNan())
   {
      return false;
   }
   rspfDpt shift(-theUlCorner.x,
		  -theUlCorner.y);

   rspfDpt tempShiftP1 = p1+shift;
   rspfDpt tempShiftP2 = p2+shift;
   double maxW = width()-1;
   double maxH = height()-1;
   if (clip_1d (&tempShiftP1.x, &tempShiftP1.y, 
                &tempShiftP2.x, &tempShiftP2.y, 
                maxW) == 0)
   {
      return false;
   }
   if(clip_1d (&tempShiftP1.y, 
               &tempShiftP1.x, 
               &tempShiftP2.y, 
               &tempShiftP2.x, maxH) == 0)
   {
      return false;
   }
   p1 = tempShiftP1-shift;
   p2 = tempShiftP2-shift;
   return true;
}

//*******************************************************************
// Public Method: rspfDrect::getCode
//*******************************************************************
long rspfDrect::getCode(const rspfDpt& aPoint,
                         const rspfDrect& clipRect)
{
   long result=NONE; // initialize to inside rect
   
   if( (aPoint.x > clipRect.lr().x) )
      result |= RIGHT;
   else if( (aPoint.x < clipRect.ul().x) )
      result |= LEFT;

   if (clipRect.theOrientMode == RSPF_LEFT_HANDED)
   {
      if( (aPoint.y < clipRect.ul().y) )
         result |= TOP;
      else if( (aPoint.y > clipRect.lr().y) )
         result |= BOTTOM;
   }
   else
   {
      if( (aPoint.y > clipRect.ul().y) )
         result |= TOP;
      else if( (aPoint.y < clipRect.lr().y) )
         result |= BOTTOM;
   }
      
   return result;
}


void rspfDrect::splitToQuad(rspfDrect& ulRect,
                             rspfDrect& urRect,
                             rspfDrect& lrRect,
                             rspfDrect& llRect)
{
   rspfDpt ulPt  = this->ul();
   rspfDpt urPt  = this->ur();
   rspfDpt lrPt  = this->lr();
   rspfDpt llPt  = this->ll();
   rspfIpt midPt = this->midPoint();
   
   ulRect = rspfDrect(ulPt.x,
                       ulPt.y,
                       midPt.x,
                       midPt.y,
                       theOrientMode);
   
   urRect = rspfDrect(midPt.x,
                       ulPt.y,
                       urPt.x,
                       midPt.y,
                       theOrientMode);
   
   if(theOrientMode  == RSPF_LEFT_HANDED)
   {
      lrRect = rspfDrect(midPt.x,
                          midPt.y,
                          lrPt.x,
                          theOrientMode);
      llRect = rspfDrect(ulPt.x,
                          midPt.y,
                          midPt.x,
                          llPt.y,
                          theOrientMode);
   }
   else
   {       
      lrRect = rspfDrect(midPt.x,
                          midPt.y,
                          lrPt.x,
                          theOrientMode);
      llRect = rspfDrect(ulPt.x,
                          midPt.y,
                          midPt.x,
                          llPt.y,
                          theOrientMode);       
   }
   
}

//*******************************************************************
// Public Method: rspfDrect::clipToRect
//*******************************************************************
rspfDrect rspfDrect::clipToRect(const rspfDrect& rect)const
{
   rspfDrect result;
   result.makeNan();
   if(rect.hasNans() || hasNans())
   {

      return result;
   }
   
   if (theOrientMode != rect.theOrientMode)
      return (*this);

   double x0 = rspf::max(rect.ul().x, ul().x);
   double x1 = rspf::min(rect.lr().x, lr().x);
   double y0, y1;

   if (theOrientMode == RSPF_LEFT_HANDED)
   {
      y0 = rspf::max(rect.ll().y, ll().y);
      y1 = rspf::min(rect.ur().y, ur().y);

      if( (x1 < x0) || (y1 < y0) )
         return result;
      else
         result = rspfDrect(x0, y0, x1, y1, theOrientMode);
   }
   else
   {
      y0 = rspf::max(rect.ll().y,ll().y);
      y1 = rspf::min(rect.ur().y,ur().y);
      if((x0 <= x1) && (y0 <= y1))
      {
         result = rspfDrect(x0, y1, x1, y0, theOrientMode);
      }
   }
   return result;
}

const rspfDrect& rspfDrect::operator=(const rspfIrect& rect)
{
   if(rect.isNan())
   {
      makeNan();
   }
   else
   {
      theUlCorner   = rect.ul();
      theUrCorner   = rect.ur();
      theLrCorner   = rect.lr();
      theLlCorner   = rect.ll();
      theOrientMode = rect.orientMode();
   }
   
   return *this;
}

//*************************************************************************************************
// Finds the point on the rect boundary that is closest to the arg_point. Closest is defined as
// the minimum perpendicular distance.
//*************************************************************************************************
rspfDpt rspfDrect::findClosestEdgePointTo(const rspfDpt& arg_point) const
{
   double dXleft  = theUlCorner.x - arg_point.x;
   double dXright = theLrCorner.x - arg_point.x;
   double dYupper = theUlCorner.y - arg_point.y;
   double dYlower = theLrCorner.y - arg_point.y;

   rspfDpt edge_point (theLrCorner);

   if (dXleft*dXright < 0.0)
      edge_point.x = arg_point.x;
   else if (fabs(dXleft) < fabs(dXright))
      edge_point.x = theUlCorner.x;

   if (dYupper*dYlower < 0.0)
      edge_point.y = arg_point.y;
   else if (fabs(dYupper) < fabs(dYlower))
      edge_point.y = theUlCorner.y;

   return edge_point;
}

