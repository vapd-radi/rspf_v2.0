//**************************************************************************
// FILE: rspfDtedFactory.cc
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Class definition for rspfDtedFactory.
//
// This is the base class interface for elevation source factories.  Contains
// pure virtual methods that all elevation source factories must implement.
//
//**************************************************************************
// $Id: rspfDtedFactory.cpp 16104 2009-12-17 18:09:59Z gpotts $

#include <cstdlib> /* abs() */
#include <sstream>
#include <iomanip>
#include <rspf/elevation/rspfDtedFactory.h>
#include <rspf/elevation/rspfDtedHandler.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfNotifyContext.h>

static rspfTrace traceDebug ("rspfDtedFactory:debug");

RTTI_DEF1(rspfDtedFactory, "rspfDtedFactory", rspfElevSourceFactory)

rspfDtedFactory::rspfDtedFactory()
   : rspfElevSourceFactory()
{}

rspfDtedFactory::rspfDtedFactory(const rspfFilename& dir)
   : rspfElevSourceFactory()
{
   theDirectory = dir;
}

rspfDtedFactory::~rspfDtedFactory()
{}

rspfElevSource* rspfDtedFactory::getNewElevSource(const rspfGpt& gpt) const
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfDtedFactory::getNewElevSource: Entered..."
         << std::endl;
   }
   
   rspfRefPtr<rspfElevSource> dted_ptr;
   
   if (theDirectory == rspfFilename::NIL)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfDtedFactory::getNewElevSource: "
         << "DTED directory has not been set!"
         << "\nReturning null elevation source..."
         << std::endl;
      return dted_ptr.release();
   }
   
   // Build up a dted file name.
   rspfFilename dted_file_base;
   
   int ilon = static_cast<int>(floor(gpt.lond()));
   
   if (ilon < 0)
   {
      dted_file_base = "w";
   }
   else
   {
      dted_file_base = "e";
   }
   
   ilon = abs(ilon);
   std::ostringstream  s1;
   s1 << std::setfill('0') << std::setw(3)<< ilon;
   
   dted_file_base += s1.str().c_str();//rspfString::toString(ilon);
   dted_file_base += "/";
   
   int ilat =  static_cast<int>(floor(gpt.latd()));
   if (ilat < 0)
   {
      dted_file_base += "s";
   }
   else
   {
      dted_file_base += "n";
   }

   ilat = abs(ilat);
   std::ostringstream  s2;

   s2<< std::setfill('0') << std::setw(2)<< ilat;
   
   dted_file_base += s2.str().c_str();

   // Look for a dted file with a level 2 extension first.
   rspfFilename dted_name = theDirectory.dirCat(dted_file_base);
   dted_name += ".dt2";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfDtedFactory::getNewElevSource:"
         << "\nSearching for file:  " << dted_name
         << std::endl;
   }
   
   if (dted_name.exists())
   {
      dted_ptr = new rspfDtedHandler(dted_name);
      if ( (!(dted_ptr->getErrorStatus())) &&
           dted_ptr->pointHasCoverage(gpt) )
      {
         return dted_ptr.release();
      }
      else
      {
         dted_ptr = 0;
      }
   }

   // Look for a dted file with a level 1 extension next.
   dted_name = theDirectory.dirCat(dted_file_base);
   dted_name += ".dt1";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfDtedFactory::getNewElevSource:"
         << "\nSearching for file:  " << dted_name
         << std::endl;
   }
   
   if (dted_name.exists() )
   {
      dted_ptr = new rspfDtedHandler(dted_name);
      if ( (!(dted_ptr->getErrorStatus())) &&
           dted_ptr->pointHasCoverage(gpt) )
      {
         return dted_ptr.release();
      }
      else
      {
         dted_ptr = 0;
      }
   }

   // Look for a dted file with a level 0 extension next.
   dted_name = theDirectory.dirCat(dted_file_base);
   dted_name += ".dt0";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfDtedFactory::getNewElevSource:"
         << "\nSearching for file:  " << dted_name
         << std::endl;
   }
   
   if (dted_name.exists() )
   {
      dted_ptr = new rspfDtedHandler(dted_name);
      if ( (!(dted_ptr->getErrorStatus())) &&
           dted_ptr->pointHasCoverage(gpt) )
      {
         return dted_ptr.release();
      }
      else
      {
         dted_ptr = 0;
      }
   }

   return dted_ptr.release();
}

void rspfDtedFactory::createIndex()
{
}
