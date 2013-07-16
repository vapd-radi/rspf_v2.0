//*******************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Kenneth Melero (kmelero@sanz.com)
//
//*******************************************************************
//  $Id: rspfWorldFileWriter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfWorldFileWriter.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfStatePlaneProjectionInfo.h>
// #include <rspf/projection/rspfStatePlaneProjectionFactory.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>


RTTI_DEF1(rspfWorldFileWriter,
          "rspfWorldFileWriter",
          rspfMetadataFileWriter)

static rspfTrace traceDebug("rspfWorldFileWriter:debug");

rspfWorldFileWriter::rspfWorldFileWriter()
   :
   rspfMetadataFileWriter(),
   theUnits(RSPF_METERS)
{
}

rspfWorldFileWriter::~rspfWorldFileWriter()
{
}

bool rspfWorldFileWriter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   const char* lookup = kwl.find(prefix, "linear_units");
   if(lookup)
   {
      rspfUnitType units = (rspfUnitType)rspfUnitTypeLut::instance()->
         getEntryNumber(lookup, true);
      if ( (units == RSPF_METERS) ||
           (units == RSPF_FEET)   ||
           (units == RSPF_US_SURVEY_FEET) )
      {
         theUnits = units;
      }
   }

   return rspfMetadataFileWriter::loadState(kwl, prefix);
}

//**************************************************************************************************
// Outputs projection information to the output file. Returns TRUE if successful.
//**************************************************************************************************
bool rspfWorldFileWriter::writeFile()
{
   if(!theInputConnection)
      return false;

   std::ofstream out(theFilename.c_str(), ios_base::out);
   if (!out)
      return false;
   
   // Fetch the map projection of the input image if it exists:
   const rspfMapProjection* mapProj = 0;
   rspfRefPtr<rspfImageGeometry> imgGeom = theInputConnection->getImageGeometry();
   if( imgGeom.valid() )
   {
      const rspfProjection* proj = imgGeom->getProjection();
      mapProj = PTR_CAST(rspfMapProjection, proj);
   }
   if (!mapProj)
   {
      out.close();
      return false;
   }

   // Convert projection info to proper units:
   rspfDpt gsd = mapProj->getMetersPerPixel();
   rspfDpt ul  = mapProj->getUlEastingNorthing();

   // ESH 05/2008 -- If the pcs code has been given, we
   // make use of the implied units.
   theUnits = mapProj->getProjectionUnits();
   if (theUnits == RSPF_FEET)
   {
      gsd.x = rspfUnitConversionTool(gsd.x, RSPF_METERS).getFeet();
      gsd.y = rspfUnitConversionTool(gsd.y, RSPF_METERS).getFeet();
      ul.x  = rspfUnitConversionTool(ul.x,  RSPF_METERS).getFeet();
      ul.y  = rspfUnitConversionTool(ul.y,  RSPF_METERS).getFeet();
   }
   else if (theUnits == RSPF_US_SURVEY_FEET)
   {
      gsd.x = rspfUnitConversionTool(gsd.x, RSPF_METERS).getUsSurveyFeet();
      gsd.y = rspfUnitConversionTool(gsd.y, RSPF_METERS).getUsSurveyFeet();
      ul.x  = rspfUnitConversionTool(ul.x,  RSPF_METERS).getUsSurveyFeet();
      ul.y  = rspfUnitConversionTool(ul.y,  RSPF_METERS).getUsSurveyFeet();
   }

   // output projection info to file:
   out << setiosflags(ios::fixed) << setprecision(15)
      << gsd.x  << endl
      << 0.0    << endl // rotation value X
      << 0.0    << endl // rotation value y
      << -gsd.y << endl
      << ul.x   << endl
      << ul.y   << endl;

   out.close();
   return true;
}

//**************************************************************************************************
void rspfWorldFileWriter::getMetadatatypeList(
   std::vector<rspfString>& metadatatypeList) const
{
   metadatatypeList.push_back(rspfString("tiff_world_file"));
   metadatatypeList.push_back(rspfString("jpeg_world_file")); 
}

bool rspfWorldFileWriter::hasMetadataType(
   const rspfString& metadataType)const
{
   if ( (metadataType == "tiff_world_file") ||
        (metadataType == "jpeg_world_file") )
   {
      return true;
   }
   
   return false;
}

void rspfWorldFileWriter::setLinearUnits(rspfUnitType units)
{
   if ( (units == RSPF_UNIT_UNKNOWN) ||
        (units == RSPF_METERS)  ||
        (units == RSPF_FEET)    ||
        (units == RSPF_US_SURVEY_FEET) )
   {
      theUnits = units;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfWorldFileWriter::setLinearUnits WARNING!"
         << "\nUnsupported units passed to method:  "
         << rspfUnitTypeLut::instance()->getEntryString(units)
         << "\nUnits unchanged..."
         << std::endl;
   }
}
