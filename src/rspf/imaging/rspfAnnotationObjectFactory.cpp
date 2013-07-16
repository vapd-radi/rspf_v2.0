//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationObjectFactory.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/imaging/rspfAnnotationObjectFactory.h>

#include <rspf/imaging/rspfGeoAnnotationEllipseObject.h>
#include <rspf/imaging/rspfGeoAnnotationFontObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiPolyLineObject.h>
#include <rspf/imaging/rspfGeoAnnotationPolyObject.h>

rspfAnnotationObjectFactory*  rspfAnnotationObjectFactory::theInstance=NULL;

rspfAnnotationObjectFactory::rspfAnnotationObjectFactory()
   :rspfFactoryBase<rspfAnnotationObject>()
{
}

rspfAnnotationObject* rspfAnnotationObjectFactory::create(
   const rspfString& spec) const
{
   if(spec == STATIC_TYPE_NAME(rspfGeoAnnotationEllipseObject))
   {
      return new rspfGeoAnnotationEllipseObject;
   }
   if(spec == STATIC_TYPE_NAME(rspfGeoAnnotationFontObject))
   {
      return new rspfGeoAnnotationFontObject;
   }
   if(spec == STATIC_TYPE_NAME(rspfGeoAnnotationMultiPolyLineObject))
   {
      return new rspfGeoAnnotationMultiPolyLineObject;
   }
   if(spec == STATIC_TYPE_NAME(rspfGeoAnnotationPolyObject))
   {
      return new rspfGeoAnnotationPolyObject;
   }

   return NULL;
}

rspfAnnotationObject* rspfAnnotationObjectFactory::create(
   const rspfKeywordlist& kwl,
   const char* prefix)    const
{
   rspfAnnotationObject* result = NULL;
   
   const char* type = kwl.find(prefix, "type");
   if(type)
   {
      result = create(rspfString(type));
      if(result)
      {
         result->loadState(kwl, prefix);
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfAnnotationObjectFactory::create WARNING"
            << "Unable to create object of type:  "
            << type << std::endl;
      }
   }

   return result;
}

rspfAnnotationObjectFactory* rspfAnnotationObjectFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfAnnotationObjectFactory();
   }

   return theInstance;
}

//*****************************************************************************
//  METHOD: rspfProjectionFactory::getList()
//  
//*****************************************************************************
list<rspfString> rspfAnnotationObjectFactory::getList() const 
{
   list<rspfString> rtn_list;
   list<rspfString> sub_list;
   list<rspfFactoryBase<rspfAnnotationObject>*>::const_iterator factory;

   factory = theRegistry.begin();
   while(factory != theRegistry.end())
   {
      sub_list = (*factory)->getList();
      rtn_list.merge(sub_list);
      ++factory;
   }

   return rtn_list;
}
