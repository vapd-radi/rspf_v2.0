//*****************************************************************************
// FILE: rspfGeoidManager.cpp
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfGeoidManager
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGeoidManager.cpp 20096 2011-09-14 16:44:20Z dburken $

#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfEnvironmentUtility.h>

RTTI_DEF1(rspfGeoidManager, "rspfGeoidManager", rspfGeoid);

#include <rspf/base/rspfGeoidNgs.h>
#include <rspf/base/rspfGeoidEgm96.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfNotifyContext.h>
//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfGeoidManager:exec");
static rspfTrace traceDebug ("rspfGeoidManager:debug");

rspfGeoidManager* rspfGeoidManager::theInstance = 0;

//*****************************************************************************
//  CONSTRUCTOR: rspfGeoidManager
//  
//*****************************************************************************
rspfGeoidManager::rspfGeoidManager()
{
   theInstance = this;
   theIdentityGeoid = new rspfIdentityGeoid();
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfGeoidManager
//  
//*****************************************************************************
rspfGeoidManager::~rspfGeoidManager()
{
//    vector<rspfGeoid*>::iterator g = theGeoidList.begin();
//    while (g != theGeoidList.end())
//    {
//       delete *g;
//       ++g;
//    }
   theGeoidList.clear();
}

//*****************************************************************************
//  METHOD: rspfGeoidManager::instance()
//  
//*****************************************************************************
rspfGeoidManager* rspfGeoidManager::instance()
{
   if (!theInstance)
   {
      theInstance = new rspfGeoidManager();
   }
   
   return theInstance;
}

//*****************************************************************************
//  METHOD: rspfElevManager::addGeoidSource
//  
//*****************************************************************************
void rspfGeoidManager::addGeoid(rspfRefPtr<rspfGeoid> geoid, bool toFrontFlag)
{
   if(!toFrontFlag)
   {
      theGeoidList.push_back(geoid);
   }
   else
   {
      theGeoidList.insert(theGeoidList.begin(), geoid);
   }
}

bool rspfGeoidManager::saveState(rspfKeywordlist& /* kwl */,
                                  const char* /* prefix */ ) const
{
   return true;
}

//*****************************************************************************
//  METHOD: rspfGeoidManager::loadState()
//  
//*****************************************************************************
bool rspfGeoidManager::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   static const char MODULE[] = "rspfGeoidManager::loadState()";

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: " << MODULE << ", entering...\n";

   
   // Look for the ngs geoid directories
   const char* lookup = kwl.find(prefix, "geoid_99_directory");
   rspfByteOrder geoidNgsByteOrder = RSPF_LITTLE_ENDIAN;
   const char* byteOrder = kwl.find(prefix, "geoid_99_directory.byte_order");
   if (!lookup)
   {
      lookup = kwl.find(prefix, "geoid_ngs_directory");
      byteOrder = kwl.find(prefix, "geoid_ngs_directory.byte_order");
   }
   if(byteOrder)
   {
      if(rspfString(byteOrder).contains("little"))
      {
         geoidNgsByteOrder = RSPF_LITTLE_ENDIAN;
      }
      else
      {
         geoidNgsByteOrder = RSPF_BIG_ENDIAN;
      }
   }
   if(lookup)
   {
      rspfFilename f = lookup;
      if (f.isDir())
      {
         rspfRefPtr<rspfGeoid> geoid = new rspfGeoidNgs(f, geoidNgsByteOrder);

         if (geoid->getErrorStatus() == rspfErrorCodes::RSPF_OK)
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG: " << MODULE
                  << "\nAdded geoid dir:  " << f.c_str() << "\n";
            }

            addGeoid(geoid.get());
         }
         else
         {
            geoid = 0;
         }
      }
   }

   rspfFilename geoidGrid1996 = rspfEnvironmentUtility::instance()->getUserOssimSupportDir();
   geoidGrid1996 = geoidGrid1996.dirCat("geoids");
   geoidGrid1996 = geoidGrid1996.dirCat("geoid1996");
   geoidGrid1996 = geoidGrid1996.dirCat("egm96.grd");

   if(!geoidGrid1996.exists())
   {
      geoidGrid1996 = rspfEnvironmentUtility::instance()->getInstalledOssimSupportDir();
      geoidGrid1996 = geoidGrid1996.dirCat("geoids");
      geoidGrid1996 = geoidGrid1996.dirCat("geoid1996");
      geoidGrid1996 = geoidGrid1996.dirCat("egm96.grd");
   }

   if( geoidGrid1996.exists() )
   {
      rspfRefPtr<rspfGeoid> geoid = new rspfGeoidEgm96(geoidGrid1996);
      if (geoid->getErrorStatus() == rspfErrorCodes::RSPF_OK)
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "DEBUG: " << MODULE
               << "\nAdded geoid egm 96:  " << geoidGrid1996.c_str()
               << "\n";
         }
         addGeoid(geoid.get());
      }
      else
      {
         geoid = 0;
      }
   }
   else
   {
      // Look for the geoid Earth Gravity Model (EGM) 96 directory.
      lookup = kwl.find(prefix, "geoid_egm_96_grid");
      if (lookup)
      {
         rspfFilename f = lookup;
         if (f.isDir() || f.isFile())
         {
            rspfRefPtr<rspfGeoid> geoid = new rspfGeoidEgm96(f);
            
            if (geoid->getErrorStatus() == rspfErrorCodes::RSPF_OK)
            {
               if (traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << "DEBUG: " << MODULE
                     << "\nAdded geoid egm 96:  " << f.c_str()
                     << "\n";
               }
               
               addGeoid(geoid.get());
            }
            else
            {
               geoid = 0;
            }
         }
      }
   }
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: " << MODULE << ", returning...\n";
   }
   
   return true;
}


//*****************************************************************************
//  METHOD: rspfGeoidManager::open()
//  
//*****************************************************************************
bool rspfGeoidManager::open(const rspfFilename& dir, rspfByteOrder byteOrder)
{
   std::vector<rspfRefPtr<rspfGeoid> >::iterator g = theGeoidList.begin();
   bool status = true;
   while (g != theGeoidList.end())
   {
      status &= (*g)->open(dir, byteOrder);
      ++g;
   }

   return status;
}

//*****************************************************************************
//  METHOD: rspfGeoidManager::open()
//  
//*****************************************************************************
double rspfGeoidManager::offsetFromEllipsoid(const rspfGpt& gpt) const
{
   double offset = rspf::nan();
   std::vector<rspfRefPtr<rspfGeoid> >::const_iterator geoid =
      theGeoidList.begin();
   
   while ( rspf::isnan(offset) && (geoid != theGeoidList.end()))
   {
      offset = ((*geoid))->offsetFromEllipsoid(gpt);
      ++geoid;
   }
   
   return offset;
}

rspfGeoid* rspfGeoidManager::findGeoidByShortName(const rspfString& shortName, bool caseSensitive)
{
   rspf_uint32 idx=0;
   rspfString testString = shortName;
   if(shortName == "identity")
   {
      return theIdentityGeoid.get();
   }
   if(!caseSensitive)
   {
      testString  = testString.downcase();
   }
   for(idx = 0; idx < theGeoidList.size(); ++idx)
   {
      if(!caseSensitive)
      {
         if(theGeoidList[idx]->getShortName().downcase() == testString)
         {
            return theGeoidList[idx].get();
         }
      }
      else
      {
         if(theGeoidList[idx]->getShortName() == testString)
         {
            return theGeoidList[idx].get();
         }
      }
   }
   return 0;
}
