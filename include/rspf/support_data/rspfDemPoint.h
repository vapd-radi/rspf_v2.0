//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Ken Melero (kmelero@imagelinks.com)
//         Orginally written by Jamie Moyers (jmoyers@geeks.com)
//         Adapted from the package KDEM.
// Description: This class defines a DEM point.
//
//********************************************************************
// $Id: rspfDemPoint.h 17195 2010-04-23 17:32:18Z dburken $

#ifndef rspfDemPoint_HEADER
#define rspfDemPoint_HEADER

#include <vector>

class rspfDemPoint
{
public:

   rspfDemPoint(double x = 0.0, double y = 0.0);
     
   // Accessors
   double getX() const;
   double getY() const;
   void getXY(double& x, double& y) const;

   // Modifiers
   void setX(double x);
   void setY(double y);
   void setXY(double x, double y);
   bool operator<(rspfDemPoint const& /* rhs */) const {return false;}
   bool operator==(rspfDemPoint const& /* rhs */) const {return false;}

private:

   double _x;
   double _y;
};

// Inline Methods for class rspfDemPoint

inline
rspfDemPoint::rspfDemPoint(double x, double y)
{
   _x = x;
   _y = y;
}

inline
double
rspfDemPoint::getX() const
{
   return _x;
}

inline
double
rspfDemPoint::getY() const
{
   return _y;
}

inline
void
rspfDemPoint::getXY(double& x, double& y) const
{
   x = _x;
   y = _y;
}

inline
void
rspfDemPoint::setX(double x)
{
   _x = x;
}

inline
void
rspfDemPoint::setY(double y)
{
   _y = y;
}

inline
void
rspfDemPoint::setXY(double x, double y)
{
   _x = x;
   _y = y;
}

typedef std::vector<rspfDemPoint> rspfDemPointVector;

#endif  // #ifndef rspfDemPoint_HEADER
