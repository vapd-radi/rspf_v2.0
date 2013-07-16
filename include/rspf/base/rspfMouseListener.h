//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfMouseListener.h 17195 2010-04-23 17:32:18Z dburken $
#ifndef rspfMouseListener_HEADER
#define rspfMouseListener_HEADER
#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfMouseEvent.h>

class RSPFDLLEXPORT rspfMouseListener : public rspfListener
{
public:
   rspfMouseListener():rspfListener(){}
   virtual ~rspfMouseListener(){}
   
   virtual void processEvent(rspfEvent& event);

   /*!
    * processes all mouse events
    */
   virtual void mouseEvent(rspfMouseEvent& /* event */)
      {}

TYPE_DATA
};
#endif
