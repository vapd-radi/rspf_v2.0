//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfImageGeometryEventListener.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfImageGeometryEventListener_HEADER
#define rspfImageGeometryEventListener_HEADER
#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfImageGeometryEvent.h>

class rspfImageGeometryEventListener : public rspfListener
{
public:
   rspfImageGeometryEventListener():rspfListener(){}
   virtual ~rspfImageGeometryEventListener(){}
   virtual void processEvent(rspfEvent& event);

   virtual void imageGeometryEvent(rspfImageGeometryEvent& /* event */)
      {}
   
TYPE_DATA
};

#endif
