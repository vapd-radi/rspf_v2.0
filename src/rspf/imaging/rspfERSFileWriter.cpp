//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Andrew Huang
//
// Description:
// 
// Class declaration for rspfERSFileWriter.  Meta data class for
// writing an ERS header file.
//
//----------------------------------------------------------------------------
// $Id: rspfERSFileWriter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfERSFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>


RTTI_DEF1(rspfERSFileWriter,
          "rspfERSFileWriter",
          rspfMetadataFileWriter)

rspfERSFileWriter::rspfERSFileWriter()
   :
   rspfMetadataFileWriter(),
   theHdr()
{
}

rspfERSFileWriter::~rspfERSFileWriter()
{
}

bool rspfERSFileWriter::loadState(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   // To pick up output filename.
   return rspfMetadataFileWriter::loadState(kwl, prefix);
}

bool rspfERSFileWriter::writeFile()
{
   rspfString tmp;
   
   theHdr.theDatasetType	= "ERStorage";
   theHdr.theDatatype		= "Raster";
   theHdr.theBands		= theInputConnection->getNumberOfOutputBands();
   theHdr.theSample		= theAreaOfInterest.width();
   theHdr.theLine		= theAreaOfInterest.height();
   
   rspfByteOrder byteorder = rspf::byteOrder();
   if (byteorder == RSPF_BIG_ENDIAN)
   {
      theHdr.theByteorder = "MSBFirst";
   }
   else
   {
      theHdr.theByteorder = "LSBFirst";
   }
   
   theHdr.theCelltype = theInputConnection->getOutputScalarType();
   
   // Get the geometry from the input.
   rspfMapProjection* mapProj = 0;
   rspfRefPtr<rspfImageGeometry> inputGeom = theInputConnection->getImageGeometry();
   if ( inputGeom.valid() ) mapProj = PTR_CAST(rspfMapProjection, inputGeom->getProjection());
   if (mapProj)
   {
      // Create the projection info.
      rspfRefPtr<rspfMapProjectionInfo> projectionInfo
         = new rspfMapProjectionInfo(mapProj, theAreaOfInterest);

      // Set the tie points in the keyword list.
      rspfKeywordlist kwl;
      projectionInfo->getGeom(kwl);

      // Get the projection type.
      const char* lookup;
      rspfString projection;
      lookup = kwl.find(rspfKeywordNames::TYPE_KW);
      if (lookup)
      {
         projection = lookup;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfERSFileWriter::writeFile WARNING:"
            << "\nNo projection type found!\nReturning..."
            << std::endl;
         return false; // Have to have the projection type!
      }

      if (projection == "rspfUtmProjection")
      {
         rspfString utm;
         lookup = kwl.find(rspfKeywordNames::HEMISPHERE_KW);
         if (lookup)
         {
            utm = lookup;
            utm += "UTM";
         }
         else
            return false;

         lookup = kwl.find(rspfKeywordNames::ZONE_KW);
         if (lookup)
            utm += lookup;
         else
            return false;

         theHdr.theProjection = utm;
         theHdr.theCoordSysType = "EN";


         //we get tie points
         lookup = kwl.find(rspfKeywordNames::TIE_POINT_XY_KW);
         tmp = lookup;
         if (lookup)
         {
            tmp = tmp.trim("(");
            tmp = tmp.trim(")");
            rspfString a = tmp.before(",", 0);
            rspfString b = tmp.after(",", 0);
            theHdr.theOriginX = a.toDouble();
            theHdr.theOriginY = b.toDouble();
         }
         //set the tie point unit type to meters
         theHdr.theTieUnitType = RSPF_METERS;

      }
      else if (projection == "rspfEquDistCylProjection")
      {
         theHdr.theProjection = "GEODETIC";
         theHdr.theCoordSysType = "LL";

         //we get tie points
         lookup = kwl.find(rspfKeywordNames::TIE_POINT_XY_KW);
         tmp = lookup;
         if (lookup)
         {
            tmp = tmp.trim("(");
            tmp = tmp.trim(")");
            rspfString a = tmp.before(",", 0);
            rspfString b = tmp.after(",", 0);
            theHdr.theOriginX = a.toDouble();
            theHdr.theOriginY = b.toDouble();
         }
         //set the tie point unit type to degrees
         theHdr.theTieUnitType = RSPF_DEGREES;

      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfERSFileWriter::writeFile WARNING:"
            << "\nOnly LatLon and UTM supported!\nReturning..."
            << std::endl;
         return false;		
      }

      // Get the datum.
      rspfString datum = "WGS-84";
      lookup = kwl.find(rspfKeywordNames::DATUM_KW);
      if (lookup)
      {
         rspfString os = lookup;
         if (os == "WGE")
         {
            theHdr.theDatum = "WGS84";
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfERSFileWriter::writeFile WARNING:"
               << "\nOnly WGS84 supported!\nReturning..."
               << std::endl;
            return false; // Datum has to be WGS84 for now...
         }
      }

      //get cell size
      lookup = kwl.find(rspfKeywordNames::PIXEL_SCALE_XY_KW);
      tmp = lookup;
      if (lookup)
      {
         tmp = tmp.trim("(");
         tmp = tmp.trim(")");
         rspfString a = tmp.before(",", 0);
         rspfString b = tmp.after(",", 0);

         theHdr.theCellSizeX = a.toDouble();
         theHdr.theCellSizeY = b.toDouble();
      }

      //get pixel is area or point
      if(thePixelType == RSPF_PIXEL_IS_POINT)
      {
         theHdr.theOriginX -= theHdr.theCellSizeX/2.0;
         theHdr.theOriginY += theHdr.theCellSizeY/2.0;
      }
   }

   return theHdr.writeFile(theFilename);
}

void rspfERSFileWriter::getMetadatatypeList(
   std::vector<rspfString>& metadatatypeList) const
{
   metadatatypeList.push_back(rspfString("ers_header")); 
}

bool rspfERSFileWriter::hasMetadataType(
   const rspfString& metadataType)const
{
   return (metadataType == "ers_header");
}
