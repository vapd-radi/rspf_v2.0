//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts  (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id: rspfAigStatistics.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAigStatistics_HEADER
#define rspfAigStatistics_HEADER

#include <iosfwd>

class rspfAigStatistics
{
public:
   rspfAigStatistics();

   void reset();

   bool writeStream(std::ostream& out);

   double theMin;
   double theMax;
   double theMean;
   double theStandardDev;
};

#endif
