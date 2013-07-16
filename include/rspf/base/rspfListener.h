//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfListener.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfListener_HEADER
#define rspfListener_HEADER
#include <rspf/base/rspfObject.h>

class rspfEvent;

/*!
 * Base class for all listners.  Listners nned to derive from this
 * class and override the processEvent method.
 */
class RSPFDLLEXPORT rspfListener
{
public:

   rspfListener();

   virtual ~rspfListener();

   /**
    * ProcessEvent.  The defaul is to do nothing.  Derived
    * classes need to override this class.
    */
   virtual void processEvent(rspfEvent& event);

   void enableListener();

   void disableListener();

   void setListenerEnableFlag(bool flag);

   bool isListenerEnabled() const;

   bool getListenerEnableFlag() const;

protected:
   bool theListenerEnableFlag;
TYPE_DATA
};

#endif
