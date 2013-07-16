//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfViewListener.h 17195 2010-04-23 17:32:18Z dburken $
#ifndef rspfViewListener_HEADER
#define rspfViewListener_HEADER
#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfViewEvent.h>

class RSPFDLLEXPORT rspfViewListener : public rspfListener
{
public:
   rspfViewListener():rspfListener(){}
   virtual ~rspfViewListener(){}
   
   virtual void processEvent(rspfEvent& event);
   virtual void viewScaleChangeEvent(rspfViewEvent& /* event */)
      {}
   virtual void viewTypeChangeEvent(rspfViewEvent& /* event */)
      {}
   virtual void viewEvent(rspfViewEvent& /* event */ )
      {}
   virtual void viewTransformChangeEvent(rspfViewEvent& /* event */)
      {}

   virtual void allViewEvents(rspfViewEvent& /* event */)
      {}
TYPE_DATA
};

#endif
