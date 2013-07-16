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
// $Id: rspfDisplayInterface.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfDisplayInterface_HEADER
#define rspfDisplayInterface_HEADER
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfDisplayInterface
{
public:
   rspfDisplayInterface();

   virtual ~rspfDisplayInterface();

   /*!
    * Returns the display back to the caller
    */
   virtual void* getDisplayDevice()=0;

   virtual void setTitle(const rspfString& title)=0;
   virtual void getTitle(rspfString& title)const=0;
   virtual rspfString getTitle()const;
   virtual void close()=0;
   virtual bool isOpen() const=0;
   virtual bool open()=0;
   virtual bool isMinimized()const=0;

TYPE_DATA
};

#endif
