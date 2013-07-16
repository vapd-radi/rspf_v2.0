//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageAoiListener.h 17195 2010-04-23 17:32:18Z dburken $
#ifndef rspfImageAoiListener_HEADER
#define rspfImageAoiListener_HEADER
#include <rspf/base/rspfListener.h>

class rspfImageRectangleEvent;
class rspfImagePolygonEvent;

class RSPFDLLEXPORT rspfImageAoiListener : public rspfListener
{
public:
   rspfImageAoiListener():rspfListener(){}
   virtual ~rspfImageAoiListener(){}
   
   virtual void processEvent(rspfEvent& event);

   virtual void imageRectangleEvent(rspfImageRectangleEvent& /* event */)
      {}
   virtual void imagePolygonEvent(rspfImagePolygonEvent& /* event */)
      {}
};
#endif
