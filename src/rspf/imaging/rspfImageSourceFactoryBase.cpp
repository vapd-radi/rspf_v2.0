//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: David Burken <dburken@imagelinks.com>
//
//*************************************************************************
// $Id: rspfImageSourceFactoryBase.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfImageSourceFactoryBase.h>
#include <rspf/imaging/rspfImageSource.h>
rspfImageSourceFactoryBase::rspfImageSourceFactoryBase()
   : rspfObjectFactory()
{}

rspfImageSourceFactoryBase::rspfImageSourceFactoryBase(const rspfImageSourceFactoryBase&)
   : rspfObjectFactory()
{}

const rspfImageSourceFactoryBase& rspfImageSourceFactoryBase::operator=(const rspfImageSourceFactoryBase&)
{
   return *this;
}

rspfImageSource* rspfImageSourceFactoryBase::createImageSource(const rspfString& name)const
{
   rspfRefPtr<rspfObject> result =createObject(name);
   
   if(PTR_CAST(rspfImageSource, result.get()))
   {
      return (rspfImageSource*)result.release();
   }
   result = 0;

   return 0;
}

rspfImageSource* rspfImageSourceFactoryBase::createImageSource(const rspfKeywordlist& kwl,
                                                                 const char* prefix)const
{
   rspfRefPtr<rspfObject> result =createObject(kwl, prefix);
   
   if(PTR_CAST(rspfImageSource, result.get()))
   {
      return (rspfImageSource*)result.release();
   }
   result = 0;
   
   return (rspfImageSource*)0;
}
