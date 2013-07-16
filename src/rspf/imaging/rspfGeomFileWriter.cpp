//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Kenneth Melero
//
//*******************************************************************
//  $Id: rspfGeomFileWriter.cpp 20506 2012-01-27 17:02:30Z dburken $

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/imaging/rspfGeomFileWriter.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjection.h>

RTTI_DEF1(rspfGeomFileWriter,
          "rspfGeomFileWriter",
          rspfMetadataFileWriter)

static const char DEFAULT_FILE_NAME[] = "output.geom";

static rspfTrace traceDebug("rspfGeomFileWriter:debug");

rspfGeomFileWriter::rspfGeomFileWriter()
   :
      rspfMetadataFileWriter()
{}

rspfGeomFileWriter::~rspfGeomFileWriter()
{}

bool rspfGeomFileWriter::writeFile()
{
   bool status = false;
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageGeometry> geom = theInputConnection->getImageGeometry();
      if(geom.valid())
      {
         //---
         // First check the rspfImageGeometry image size and adjust to area of interest if
         // necessary.  The rspfImageGeometry::applyScale method sometimes has rounding
         // issues so fix it here...
         //---
         if ( geom->getImageSize().hasNans() ||
              ( geom->getImageSize() != theAreaOfInterest.size() ) )
         {
            if ( traceDebug() )
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfGeomFileWriter::writeFile DEBUG:"
                  << "\nAdjusting rspfImageGeometry size to reflect the area of interest."
                  << "\narea of interest size: " << theAreaOfInterest.size()
                  << "\nrspfImageGeometry size: " << geom->getImageSize()
                  << std::endl;
            }
            geom->setImageSize( theAreaOfInterest.size() );
         }
               
         // Save the state to keyword list.
         rspfKeywordlist kwl;
         geom->saveState(kwl);

         const rspfMapProjection* mapProj =
            PTR_CAST(rspfMapProjection, geom->getProjection());

         if (mapProj)
         {
            const char* prefix = "projection.";
            
            rspfDpt tiePoint;
            if (mapProj->isGeographic())
            {
               // Get the ground tie point.
               rspfGpt gpt;
               mapProj->lineSampleToWorld(theAreaOfInterest.ul(), gpt);
               tiePoint = gpt;
               
               // Set the units to degrees.
               kwl.add(prefix,
                       rspfKeywordNames::TIE_POINT_UNITS_KW,
                       rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES),
                       true);
            }
            else
            {
               // Get the easting northing tie point.
               mapProj->lineSampleToEastingNorthing(theAreaOfInterest.ul(), tiePoint);
               
               
               // Set the units to meters.
               kwl.add(prefix,
                       rspfKeywordNames::TIE_POINT_UNITS_KW,
                       rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS),
                       true);
            }
            
            // Write the tie to keyword list.
            kwl.add(prefix,
                    rspfKeywordNames::TIE_POINT_XY_KW,
                    rspfDpt(tiePoint).toString().c_str(),
                    true);
            
         } // matches: if (mapProj)
         
         status = kwl.write(theFilename.c_str());
         
      } // matches: if(geom.valid())

   } // matches: if(theInputConnection)
   
   return status;
}

void rspfGeomFileWriter::getMetadatatypeList(std::vector<rspfString>& metadatatypeList) const
{
   metadatatypeList.push_back(rspfString("rspf_geometry")); 
}

bool rspfGeomFileWriter::hasMetadataType(
   const rspfString& metadataType)const
{
   return (metadataType == "rspf_geometry");
}
