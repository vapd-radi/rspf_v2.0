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
// $Id: rspfStdOutProgress.h 10422 2007-02-06 04:03:13Z gpotts $
#ifndef rspfStdOutProgress_HEADER
#define rspfStdOutProgress_HEADER

#include <rspf/base/rspfProcessListener.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfNotifyContext.h>

class RSPF_DLL rspfStdOutProgress : public rspfProcessListener
{
public:
   rspfStdOutProgress(rspf_uint32 precision = 0, bool flushStream=false);

   virtual void processProgressEvent(rspfProcessProgressEvent& event);
      
   virtual void setFlushStreamFlag(bool flag);

protected:
   rspf_uint32 thePrecision;
   bool         theFlushStreamFlag;
TYPE_DATA
};

RSPF_DLL extern rspfStdOutProgress theStdOutProgress;
#endif
