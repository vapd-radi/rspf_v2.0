//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts  (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id: rspfAigBounds.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAigBounds_HEADER
#define rspfAigBounds_HEADER

#include <iosfwd>

class rspfAigBounds
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfAigBounds& data);

   rspfAigBounds();

   void reset();

   bool writeStream(std::ostream& out);
   double ll_x;
   double ll_y;
   double ur_x;
   double ur_y;
};


#endif
