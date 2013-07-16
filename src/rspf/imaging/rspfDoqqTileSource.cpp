//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfDoqqTileSource.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfDoqqTileSource.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/imaging/rspfGeneralRasterInfo.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/support_data/rspfDoqq.h>

RTTI_DEF1(rspfDoqqTileSource,
          "rspfDoqqTileSource",
          rspfGeneralRasterTileSource);

rspfDoqqTileSource::rspfDoqqTileSource()
   :theHeaderInformation(0)
{
}

rspfDoqqTileSource::~rspfDoqqTileSource()
{
   theHeaderInformation = 0;
}

bool rspfDoqqTileSource::open(const rspfFilename& filename)
{
   bool result = false;
   
   theHeaderInformation = new rspfDoqq(filename);
   if(theHeaderInformation->errorStatus() == rspfDoqq::RSPF_ERROR)
   {
      theHeaderInformation = 0;
      
      return result;
   }
   else
   {
     // Set the Acquisition Date
     theAcqDate = theHeaderInformation->theAcqYear;
     theAcqDate += "-";
     theAcqDate += theHeaderInformation->theAcqMonth;
     theAcqDate += "-";
     theAcqDate += theHeaderInformation->theAcqDay;

     vector<rspfFilename> f;
     f.push_back(filename);
     rspfGeneralRasterInfo genRasterInfo(f,
					  RSPF_UCHAR,
					  RSPF_BIP,
					  theHeaderInformation->theRgb,
					  theHeaderInformation->theLine,
					  theHeaderInformation->theSample,
					  theHeaderInformation->theHeaderSize,
					  rspfGeneralRasterInfo::NONE,
					  0);
     
     result = rspfGeneralRasterTileSource::open(genRasterInfo);
   }

   return result;
} 

//**************************************************************************************************
//! Returns the image geometry object associated with this tile source or NULL if non defined.
//! The geometry contains full-to-local image transform as well as projection (image-to-world)
//**************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfDoqqTileSource::getImageGeometry()
{
   if (theGeometry.valid()) return theGeometry;
   
   if(theHeaderInformation.valid())
   {
      rspfKeywordlist kwl;
      const char* prefix = 0; // legacy

      rspfString proj  = theHeaderInformation->theProjection.trim().upcase();
      rspfString datum = theHeaderInformation->theDatum.trim().upcase();

      if(proj == rspfString("UTM") || proj == rspfString("1"))
      {
         kwl.add(prefix,
                 rspfKeywordNames::TYPE_KW,
                 "rspfUtmProjection",
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::ZONE_KW,
                 theHeaderInformation->theUtmZone,
                 true);
      }
      else
      {
         return rspfRefPtr<rspfImageGeometry>();
      }

      if(datum == rspfString("NAR") || datum == rspfString("4"))
      {
         kwl.add(prefix,
                 rspfKeywordNames::DATUM_KW,
                 "NAR-C", // North American Conus 1983
                 true);
      }
      else if(datum == rspfString("NAS") || datum == rspfString("1"))
      {
         kwl.add(prefix,
                 rspfKeywordNames::DATUM_KW,
                 "NAS-C", // North American Conus 1927
                 true);
      }
      else if(datum == rspfString("2"))
      {
         kwl.add(prefix,
                 rspfKeywordNames::DATUM_KW,
                 "WGD", // WGS 72
                 true);
      }
      else if(datum == rspfString("3"))
      {
         kwl.add(prefix,
                 rspfKeywordNames::DATUM_KW,
                 "WGE", // WGS 84
                 true);
      }
      else
      {
         return rspfRefPtr<rspfImageGeometry>();
      }

      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_X_KW,
              theHeaderInformation->theGsd.x,
              true);

      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_Y_KW,
              theHeaderInformation->theGsd.y,
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_EASTING_KW,
//            theHeaderInformation->theUE + (theHeaderInformation->theGsd/2.0),
	      theHeaderInformation->theEasting,
              true);

      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_NORTHING_KW,
//            theHeaderInformation->theUN - (theHeaderInformation->theGsd/2.0),
	      theHeaderInformation->theNorthing,
              true);

      // Capture this for next time.
      theGeometry = new rspfImageGeometry;
      theGeometry->loadState(kwl, prefix);

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
      
      return theGeometry;
   }
   return rspfRefPtr<rspfImageGeometry>();
}

rspfRefPtr<rspfProperty> rspfDoqqTileSource::getProperty(const rspfString& name)const
{
	if(name == "acquisition_date")
	{
		if(theHeaderInformation.valid())
		{
			std::stringstream out;
			out << std::setw(4) << std::setfill(' ') << theHeaderInformation->theAcqYear.c_str()
			<< std::setw(2) << setfill('0') << theHeaderInformation->theAcqMonth.c_str()
			<< std::setw(2) << setfill('0') << theHeaderInformation->theAcqDay.c_str();
			return new rspfStringProperty("acquisition_date", out.str());
		}
		return 0;
	}
	else if(name == "file_type")
	{
		return new rspfStringProperty("file_type", "DOQQ");
	}
	return rspfImageHandler::getProperty(name);
}

void rspfDoqqTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
	rspfImageHandler::getPropertyNames(propertyNames);
	propertyNames.push_back("acquisition_date");
	propertyNames.push_back("file_type");
	
}

bool rspfDoqqTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if(!rspfImageHandler::loadState(kwl, prefix))
   {
      return false;
   }

   if(lookup)
   {
      theHeaderInformation = new rspfDoqq(rspfFilename(lookup));
      if(theHeaderInformation->errorStatus() != rspfDoqq::RSPF_OK)
      {
         theHeaderInformation = 0;
         return false;
      }
      else
      {
         rspfFilename filename(lookup);
         vector<rspfFilename> f;
         f.push_back(filename);
         rspfGeneralRasterInfo genRasterInfo(f,
                                              RSPF_UCHAR,
                                              RSPF_BIP,
                                              theHeaderInformation->theRgb,
                                              theHeaderInformation->theLine,
                                              theHeaderInformation->theSample,
                                              theHeaderInformation->theHeaderSize,
                                              rspfGeneralRasterInfo::NONE,
                                              0);
         return open(genRasterInfo);
      }
   }
   else
   {
      return false;
   }
   
   return true;
}

rspfString rspfDoqqTileSource::getShortName()const
{
   return rspfString("doqq");
}

rspfString rspfDoqqTileSource::getLongName()const
{
   return rspfString("doqq handler");
}

bool rspfDoqqTileSource::open(const rspfGeneralRasterInfo& info)
{
   return rspfGeneralRasterTileSource::open(info);
}

rspfString rspfDoqqTileSource::acqdate() const
{
   return theAcqDate;
}
