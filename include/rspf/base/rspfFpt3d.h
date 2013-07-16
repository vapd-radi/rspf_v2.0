//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:  
//
// Contains class declaration for fpt3d.
//
// Used to represent a three dimensional point containing an x and y data
// member.
//
//*******************************************************************
// $Id: rspfFpt3d.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfFpt3d_HEADER
#define rspfFpt3d_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFpt.h>


class RSPFDLLEXPORT rspfFpt3d
{
public:

   rspfFpt3d()
      : x(0.0), y(0.0), z(0.0)
      {}
   rspfFpt3d(const rspf_float32& aX,
              const rspf_float32& aY,
              const rspf_float32& aZ)
      : x(aX), y(aY), z(aZ)
      {}

   rspfFpt pt2d() const { return rspfFpt(x, y); }
   
   ostream& print(ostream& os) const;

   friend ostream& operator<<(ostream& os, const rspfFpt3d& pt);
   friend istream& operator>>(istream& is, rspfFpt3d& pt);

   rspf_float32 x;
   rspf_float32 y;
   rspf_float32 z;
};

#endif
