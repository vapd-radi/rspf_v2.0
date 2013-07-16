//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Shuttle Radar Topography Mission (SRTM) factory to return an
// rspfSrtmElevSource given a ground point.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmFactory.cpp 16123 2009-12-17 22:07:31Z dburken $

#include <cstdlib> /* abs() */
#include <iostream>
#include <iomanip>
#include <sstream>
#include <rspf/elevation/rspfSrtmFactory.h>
#include <rspf/elevation/rspfSrtmHandler.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfStreamFactoryRegistry.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/support_data/rspfSrtmFilename.h>
#include <rspf/support_data/rspfSrtmSupportData.h>

static rspfTrace traceDebug ("rspfSrtmFactory:debug");

RTTI_DEF1(rspfSrtmFactory, "rspfSrtmFactory", rspfElevSourceFactory)

rspfSrtmFactory::rspfSrtmFactory()
   : rspfElevSourceFactory()
{}

rspfSrtmFactory::rspfSrtmFactory(const rspfFilename& dir)
   : rspfElevSourceFactory()
{
   theDirectory = dir;
}
      
rspfSrtmFactory::~rspfSrtmFactory()
{}

rspfElevSource* rspfSrtmFactory::getNewElevSource(const rspfGpt& gpt) const
{
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfSrtmFactory::getNewElevSource: Entered..."
         << std::endl;
   }

   rspfRefPtr<rspfElevCellHandler> srtmPtr;

   if (theDirectory == rspfFilename::NIL)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "rspfSrtmFactory::getNewElevSource: "
         << "SRTM directory has not been set!"
         << "\nReturning null elevation source..."
         << std::endl;
      
      return srtmPtr.release();
   }

   //---
   // Build up a srtm file name.
   //
   // Standard for name is upper case 'N' and 'W' lower case "hgt" like:
   // N27W081.hgt
   //---
   rspfFilename srtmFileBasename;

   int ilat =  static_cast<int>(floor(gpt.latd()));
   if (ilat < 0)
   {
      srtmFileBasename = "S";
   }
   else
   {
      srtmFileBasename = "N";
   }

   ilat = abs(ilat);
   std::ostringstream  os1;
   
   os1 << std::setfill('0') << std::setw(2) <<ilat;
   
   srtmFileBasename += os1.str().c_str();

   int ilon = static_cast<int>(floor(gpt.lond()));
   
   if (ilon < 0)
   {
      srtmFileBasename += "W";
   }
   else
   {
      srtmFileBasename += "E";
   }

   ilon = abs(ilon);
   std::ostringstream  os2;
   os2 << std::setfill('0') << std::setw(3) << ilon;
   
   srtmFileBasename += os2.str().c_str();
   srtmFileBasename.setExtension(".hgt");

   rspfFilename srtmFile = theDirectory.dirCat(srtmFileBasename);

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfSrtmFactory::getNewElevSource:"
         << "\nSearching for file:  " << srtmFile
         << std::endl;
   }

   rspfRefPtr<rspfIFStream> is = rspfStreamFactoryRegistry::instance()->
      createNewIFStream(srtmFile, std::ios::in | std::ios::binary);

   // Look for the file mix case, then all lower case, then all upper case.
   if (is.valid())
   {
      if(is->fail())
      {
         // Try down casing the whole thing.
         srtmFileBasename = srtmFileBasename.downcase();
         srtmFile = theDirectory.dirCat(srtmFileBasename);
         
         is =  rspfStreamFactoryRegistry::instance()->
            createNewIFStream(srtmFile, std::ios::in | std::ios::binary);      
         if (is.valid())
         {
            if(is->fail())
            {
               // OK, try upcasing the whole thing.
               srtmFileBasename = srtmFileBasename.upcase();
               srtmFile = theDirectory.dirCat(srtmFileBasename);
               is =  rspfStreamFactoryRegistry::instance()->
                  createNewIFStream(srtmFile, std::ios::in | std::ios::binary);
            }
         }
      }
   }

   if ( is.valid() && (!is->fail()) )
   {
      is->close();
      is = 0;
      srtmPtr = new rspfSrtmHandler();
      if(srtmPtr->open(srtmFile)&&srtmPtr->pointHasCoverage(gpt) )
      {
         return srtmPtr.release();
      }
      else
      {
         srtmPtr = 0;
      }
   }
   return srtmPtr.release();
}
