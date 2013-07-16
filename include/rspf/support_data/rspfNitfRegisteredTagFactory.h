//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level LICENSE.txt
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfRegisteredTagFactory.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNitfRegisteredTagFactory_HEADER
#define rspfNitfRegisteredTagFactory_HEADER
#include <rspf/support_data/rspfNitfTagFactory.h>

class rspfNitfRegisteredTagFactory : public rspfNitfTagFactory
{
public:
   rspfNitfRegisteredTagFactory();
   virtual ~rspfNitfRegisteredTagFactory();
   static rspfNitfRegisteredTagFactory* instance();
   
   virtual rspfRefPtr<rspfNitfRegisteredTag> create(const rspfString &tagName)const;

protected:
   static rspfNitfRegisteredTagFactory* theInstance;
TYPE_DATA   
};

#endif
