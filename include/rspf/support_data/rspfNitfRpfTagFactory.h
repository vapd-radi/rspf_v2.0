//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL  see top level LICENSE.txt
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfRpfTagFactory.h 17207 2010-04-25 23:21:14Z dburken $
#ifndef rspfNitfRpfTagFactory_HEADER
#define rspfNitfRpfTagFactory_HEADER
#include <rspf/support_data/rspfNitfTagFactory.h>
class rspfNitfRegisteredTag;

class rspfNitfRpfTagFactory : public rspfNitfTagFactory
{
public:
   virtual ~rspfNitfRpfTagFactory();
   static rspfNitfRpfTagFactory* instance();
   virtual rspfRefPtr<rspfNitfRegisteredTag> create(const rspfString &tagName)const;
   
protected:
   rspfNitfRpfTagFactory();

private:
   static rspfNitfRpfTagFactory* theInstance;
   /*!
    * Hide this.
    */
   rspfNitfRpfTagFactory(const rspfNitfRpfTagFactory & /* rhs */){}

   /*!
    * Hide this.
    */ 
   rspfNitfRpfTagFactory& operator =(const rspfNitfRpfTagFactory & /* rhs */){return *this;}
TYPE_DATA
};
#endif
