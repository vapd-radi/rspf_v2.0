//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Ken Melero (kmelero@imagelinks.com)
//         Orginally written by Oscar Kramer (SoBe Software)
// Description: Container class for LandSat5 Fast Format header files (RevB & C)
//
// ********************************************************************
// $Id: rspfFfL5.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfFfL5_HEADER
#define rspfFfL5_HEADER


#include <rspf/support_data/rspfFfL7.h>
#include <rspf/support_data/rspfFfRevb.h>

using namespace std;

// Forward class declarations.

// ***************************************************************************
// CLASS: rspfFfL5
// ***************************************************************************

class RSPF_DLL rspfFfL5 : public rspfFfL7
{
   friend ostream& operator<<(ostream& os, const rspfFfL5& head);

public:
   rspfFfL5();
   rspfFfL5(const char* headerFile);
   rspfRefPtr<rspfFfRevb> revb();
   const rspfRefPtr<rspfFfRevb> revb()const;
 
protected:
   virtual ~rspfFfL5();

private:
   void    readHeaderRevB(const rspfString& header_name);
   int     convertGeoPoint(const char* sptr, rspfGpt& geo_point);

   rspfRefPtr<rspfFfRevb> theRevb;
};

#endif
