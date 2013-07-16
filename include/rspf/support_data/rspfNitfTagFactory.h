//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfTagFactory.h 17207 2010-04-25 23:21:14Z dburken $
#ifndef rspfNitfTagFactory_HEADER
#define rspfNitfTagFactory_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>

class rspfNitfRegisteredTag;

class RSPF_DLL rspfNitfTagFactory : public rspfObject
{
public:
   rspfNitfTagFactory();
   virtual ~rspfNitfTagFactory();
   virtual rspfRefPtr<rspfNitfRegisteredTag> create(const rspfString &tagName)const=0;
private:
   /*!
    * Hide this.
    */
   rspfNitfTagFactory(const rspfNitfTagFactory & /* rhs */){}

   /*!
    * Hide this.
    */ 
   rspfNitfTagFactory& operator =(const rspfNitfTagFactory & /* rhs */){return *this;}
TYPE_DATA
};
#endif
