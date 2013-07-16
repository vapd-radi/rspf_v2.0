//----------------------------------------------------------------------------
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
// $Id: rspfProcessListener.h 14799 2009-06-30 08:54:44Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfProcessListener_HEADER
#define rspfProcessListener_HEADER
#include <rspf/base/rspfListener.h>

class rspfProcessProgressEvent;

class RSPFDLLEXPORT rspfProcessListener : public rspfListener
{
public:
   rspfProcessListener();
   virtual ~rspfProcessListener();
   virtual void processEvent(rspfEvent& event);
   virtual void processProgressEvent(rspfProcessProgressEvent& event);

TYPE_DATA
};

#endif
