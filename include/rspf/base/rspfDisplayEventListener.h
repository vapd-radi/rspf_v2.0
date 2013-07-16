//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id
#ifndef rspfDisplayEventListener_HEADER
#define rspfDisplayEventListener_HEADER

#include <rspf/base/rspfListener.h>

class rspfDisplayListEvent;
class rspfDisplayRefreshEvent;

class RSPFDLLEXPORT rspfDisplayEventListener : public rspfListener
{
public:
   rspfDisplayEventListener();
   virtual ~rspfDisplayEventListener();
   virtual void processEvent(rspfEvent& event);
   virtual void displayListEvent(rspfDisplayListEvent&);
   virtual void displayRefreshEvent(rspfDisplayRefreshEvent&);

TYPE_DATA
};

#endif /* End of "#ifndef rspfDisplayEventListener_HEADER". */
