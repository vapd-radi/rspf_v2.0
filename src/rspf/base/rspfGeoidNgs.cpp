//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*******************************************************************
//  $Id: rspfGeoidNgs.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/base/rspfGeoidNgs.h>

#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfDatumFactory.h>


#include <stdlib.h>  /* standard C general utility library */
#include <iostream>
#include <fstream>
using namespace std;

RTTI_DEF1(rspfGeoidNgs, "rspfGeoidNgs", rspfGeoid)

static rspfTrace traceDebug ("rspfGeoidNgs:debug");

rspfGeoidNgs::rspfGeoidNgs()
   :theShortName("")
{
}

rspfGeoidNgs::rspfGeoidNgs(const rspfFilename& dir,
                             rspfByteOrder /* byteOrder */)
{
   open(dir);
}


bool rspfGeoidNgs::open(const rspfFilename& dir,
                         rspfByteOrder byteOrder)
{
   if (!dir.isDir())
   {
      return false;
   }

   if(dir.isDir())
   {
      rspfGeoidNgsHeader h;

      if(dir.dirCat("g2003u01.bin").exists())
      {
         theShortName = "geoid2003";
         addFile(dir.dirCat("g2003u01.bin"), byteOrder);
      }
      if(dir.dirCat("g2003u02.bin").exists())
      {
         theShortName = "geoid2003";
         addFile(dir.dirCat("g2003u02.bin"), byteOrder);
      }
      if(dir.dirCat("g2003u03.bin").exists())
      {
         theShortName = "geoid2003";
         addFile(dir.dirCat("g2003u03.bin"), byteOrder);
      }
      if(dir.dirCat("g2003u04.bin").exists())
      {
         theShortName = "geoid2003";
         addFile(dir.dirCat("g2003u04.bin"), byteOrder);
      }
      if(dir.dirCat("g2003u05.bin").exists())
      {
         theShortName = "geoid2003";
         addFile(dir.dirCat("g2003u05.bin"), byteOrder);
      }
      if(dir.dirCat("g2003u06.bin").exists())
      {
         theShortName = "geoid2003";
         addFile(dir.dirCat("g2003u06.bin"), byteOrder);
      }

      if(dir.dirCat("g1999u01.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u01.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u02.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u02.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u03.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u03.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u04.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u04.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u05.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u05.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u06.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u06.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u07.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u07.bin"), byteOrder);
      }
      if(dir.dirCat("g1999u08.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999u08.bin"), byteOrder);
      }
      if(dir.dirCat("g1999a01.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999a01.bin"), byteOrder);
      }
      if(dir.dirCat("g1999a02.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999a02.bin"), byteOrder);
      }
      if(dir.dirCat("g1999a03.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999a03.bin"), byteOrder);
      }
      if(dir.dirCat("g1999a04.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999a04.bin"), byteOrder);
      }
      if(dir.dirCat("g1999h01.bin"))
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999h01.bin"), byteOrder);
      }
      if(dir.dirCat("g1999p01.bin").exists())
      {
         theShortName = "geoid1999";
         addFile(dir.dirCat("g1999p01.bin"), byteOrder);
      }
   }
   else
   {
      addFile(dir, byteOrder);
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "Opened geoid grids from:  " << dir.c_str() << endl;
   }
   
   return true;
}

rspfGeoidNgs::~rspfGeoidNgs()
{
}


bool rspfGeoidNgs::addFile(const rspfFilename& file, rspfByteOrder byteOrder)
{
   bool result = false;
   if(file.exists())
   {
      rspf_uint32 idx = 0;
      for(idx = 0; idx < theGridVector.size();++idx)
      {
         if(file == theGridVector[idx].filename())
         {
            result = true;
            return result;
         }
      }
      rspfGeoidNgsHeader h;
      result = h.initialize(file, byteOrder);
      if(result)
      {
         theGridVector.push_back(h);
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGeoidNgs::addFile: Adding file = " << file << std::endl;
   }
   return result;
}

double rspfGeoidNgs::offsetFromEllipsoid(const rspfGpt& gpt) const
{
   rspfGpt savedGpt = gpt;
   if(rspfDatumFactory::instance()->wgs84())
   {
      savedGpt.changeDatum(rspfDatumFactory::instance()->wgs84());
   }
   double lat = savedGpt.latd();
   double lon = savedGpt.lond();
   fixLatLon(lat, lon);
   
   return deltaHeight(lat, lon); 
}

double rspfGeoidNgs::geoidToEllipsoidHeight(double lat,
                                             double lon,
                                             double geoidHeight) const
{

   fixLatLon(lat, lon);
   double delta = deltaHeight(lat, lon);
   if (!rspf::isnan(delta))
   {
      return (geoidHeight + delta);
   }
   return delta; // nan
}

double rspfGeoidNgs::ellipsoidToGeoidHeight(double lat,
                                      double lon,
                                      double ellipsoidHeight) const
{

   fixLatLon(lat, lon);
   double delta = deltaHeight(lat, lon);
   if (!rspf::isnan(delta))
   {
      return (ellipsoidHeight - delta);
   }
   return delta; // nan
}

void rspfGeoidNgs::fixLatLon(double &lat, double &lon) const
{
   if(lat < 0)
   {
      lat += 180;
   }
   if(lon < 0)
   {
      lon += 360;
   }
}

double rspfGeoidNgs::deltaHeight(double lat, double lon)const
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theGridVector.size();++idx)
   {
      if( theGridVector[idx].pointWithin(lat, lon) )
      {
         return theGridVector[idx].getHeightDelta(lat,
                                                  lon);
      }
   }
   return rspf::nan();
}

rspfString rspfGeoidNgs::getShortName()const
{
   return theShortName;
}
