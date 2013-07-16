//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
// Function object (functor) to for stl's sort method for sorting
// a vector of rspfElevSource* by best post spacing.
//*****************************************************************************
//  $Id: rspfElevLess.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfElevLess_HEADER
#define rspfElevLess_HEADER

#include <functional>

#include <rspf/elevation/rspfElevSource.h>

class rspfElevLess :
           public std::binary_function<const rspfRefPtr<rspfElevSource>,
                  const rspfRefPtr<rspfElevSource>, bool>
{
public:
   bool operator() (const rspfRefPtr<rspfElevSource> a, const rspfRefPtr< rspfElevSource> b) const
   {
      return (a->getMeanSpacingMeters() < b->getMeanSpacingMeters());
   }
};

#endif
