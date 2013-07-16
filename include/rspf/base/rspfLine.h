//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//*******************************************************************
//  $Id: rspfLine.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfLine_HEADER
#define rspfLine_HEADER
#include <rspf/base/rspfDpt.h>
#include <iosfwd>

class RSPFDLLEXPORT rspfLine
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                               const rspfLine& rhs);
   rspfLine(const rspfDpt& p1=rspfDpt(0,0),
             const rspfDpt& p2=rspfDpt(0,0))
      :theP1(p1),
       theP2(p2)
      {
      }

   
   rspfDpt getVector()const
      {
         return (theP2-theP1);
      }

   /*!
    * Computes the following equation:
    *
    * Note: this object will be line a and the passed
    *       in object will be line b;
    *       and
    *       P1 and P2 coorespond to this object and
    *       P3 and P4 will coorespond to the passed in object.
    *
    * Now find point formed at the intersection of line a and b:
    *
    * Pa = P1 + ua ( P2 - P1 ) 
    * Pb = P3 + ub ( P4 - P3 )
    *const rspfDpt&   ul_corner,
              const rspfDpt&   lr_corner,
              rspfCoordSysOrientMode mode=RSPF_LEFT_HANDED);
    * x1 + ua (x2 - x1) = x3 + ub (x4 - x3) 
    * and 
    * y1 + ua (y2 - y1) = y3 + ub (y4 - y3)
    *
    * Solve:
    *  ua = ((x4-x3)(y1-y3) - (y4-y3)(x1-x3))/
    *       ((y4-y3)(x2-x1) - (x4-x3)(y2-y1))
    *  ub = ((x2-x1)(y1-y3) - (y2-y1)(x1-x3))/
    *       ((y4-y3)(x2-x1) - (x4-x3)(y2-y1))
    * substitute:
    *
    * x = x1 + ua (x2 - x1) 
    * y = y1 + ua (y2 - y1)
    *
    */
   rspfDpt intersectInfinite(const rspfLine& line)const;

   rspfDpt intersectSegment(const rspfLine& line)const;

   rspfDpt midPoint()const;
   double length()const;
   rspfDpt normal()const;
   /*!
    * Will return true if the point is on the line.
    */
   bool isPointWithin(const rspfDpt& point, double delta=FLT_EPSILON)const;

   bool isPointOnInfiniteLine(const rspfDpt& point, double delta=FLT_EPSILON)const;

   rspfDpt   theP1;
   rspfDpt   theP2;
};

#endif
