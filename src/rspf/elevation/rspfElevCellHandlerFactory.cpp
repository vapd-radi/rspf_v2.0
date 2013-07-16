//*****************************************************************************
// FILE: rspfElevCellHandlerFactory.cc
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// DESCRIPTION:
//   Contains implementation of class rspfElevCellHandlerFactory. This is
//   a "super-factory" owning a list of subfactories for each particular DEM
//   format class.
//
//   NOT CURRENTLY UTILIZED -- USE DTED MANAGER
//
// LIMITATIONS:
//   The intention of this factory is to produce individual instances of
//   elevation cell handlers. Presently this is not supported since DTED is
//   the only elevation source being handled. DTED is loaded via the
//   rspfDtedManager class as a DB interface, and not by accessing individual
//   DTED handlers.
//
// SOFTWARE HISTORY:
//>
//   01Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfElevCellHandlerFactory.cpp 14800 2009-06-30 08:58:55Z dburken $

#include <rspf/elevation/rspfElevCellHandlerFactory.h>
#include <rspf/base/rspfString.h>

rspfElevCellHandlerFactory* rspfElevCellHandlerFactory::theInstance = 0;

//*****************************************************************************
//  STATIC METHOD: instance()
//  
//*****************************************************************************
rspfElevCellHandlerFactory* rspfElevCellHandlerFactory::instance()
{
   if(!theInstance)
      theInstance = new rspfElevCellHandlerFactory;
 
   return (rspfElevCellHandlerFactory*) theInstance; 
} 

//*****************************************************************************
//  PROTECTED DEFAULT CONSTRUCTOR: rspfElevCellHandlerFactory
//  
//*****************************************************************************
rspfElevCellHandlerFactory::rspfElevCellHandlerFactory()
{
   //***
   // Add default sub factories to this factory's registry list:
   // NOTE: DTED handlers are managed by their own rspfDtedManager so should
   // never be created individually via a factory. This will be the typical
   // pattern for all but custom DEM files.
   //***
//   registerFactory(rspfUsgsDemCellFactory::instance());
}


//*****************************************************************************
//  METHOD: rspfElevCellHandlerFactory::create(kwl, prefix)
//  
//*****************************************************************************
rspfElevCellHandler*
rspfElevCellHandlerFactory::create(const rspfKeywordlist &keywordList,
                                    const char *prefix) const
{
   std::list<rspfFactoryBase<rspfElevCellHandler>*>::const_iterator
      elevCellFactory;

   rspfElevCellHandler* product = 0;
   
   elevCellFactory = theRegistry.begin();
   while((elevCellFactory != theRegistry.end()) && (!product))
   {
      product = (*elevCellFactory)->create(keywordList, prefix);
      elevCellFactory++;
   }

   return product;
}

//*****************************************************************************
//  METHOD: rspfElevCellHandlerFactory::create(proj_name)
//  
//*****************************************************************************
rspfElevCellHandler*
rspfElevCellHandlerFactory::create(const rspfString &name) const 
{
   std::list<rspfFactoryBase<rspfElevCellHandler>*>::const_iterator
      elevCellFactory;

   rspfElevCellHandler* product = 0;
   
   elevCellFactory = theRegistry.begin();
   while((elevCellFactory != theRegistry.end()) && (!product))
   {
      product = (*elevCellFactory)->create(name);
      elevCellFactory++;
   }

   return product;
}

//*****************************************************************************
//  METHOD: rspfElevCellHandlerFactory::getList()
//  
//*****************************************************************************
std::list<rspfString> rspfElevCellHandlerFactory::getList() const 
{
   std::list<rspfString> rtn_list;
   std::list<rspfString> sub_list;
   std::list<rspfFactoryBase<rspfElevCellHandler>*>::const_iterator factory_iter;

   factory_iter = theRegistry.begin();
   while(factory_iter != theRegistry.end())
   {
      sub_list = (*factory_iter)->getList();
      rtn_list.merge(sub_list);
      factory_iter++;
   }

   return rtn_list;
}

