//----------------------------------------------------------------------------
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: The base for overview builder factories.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderFactoryBase.cpp 19187 2011-03-23 12:21:58Z gpotts $

#include <rspf/imaging/rspfOverviewBuilderFactoryBase.h>
#include <rspf/imaging/rspfOverviewBuilderBase.h>


rspfObject* rspfOverviewBuilderFactoryBase::createObject(const rspfString& typeName) const 
{ 
   return createBuilder(typeName); 
}

rspfObject* rspfOverviewBuilderFactoryBase::createObject(const rspfKeywordlist& kwl, const char* prefix) const
{
   rspfObject* result = 0;
   rspfString typeValue = kwl.find(prefix, "type");
   if(!typeValue.empty())
   {
      result = createObject(typeValue);
      if(result)
      {
         if(!result->loadState(kwl, prefix))
         {
            delete result;
            result = 0;
         }
      }
   }
   
   return result;
}


rspfOverviewBuilderFactoryBase::rspfOverviewBuilderFactoryBase()
{
}

rspfOverviewBuilderFactoryBase::~rspfOverviewBuilderFactoryBase()
{
}
