//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Class definition for rspfEnviHeaderFileWriter.  Meta data class for
// writing an ENVI (The Environment for Visualizing Images) header file.
//
//----------------------------------------------------------------------------
// $Id: rspfEnviHeaderFileWriter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfEnviHeaderFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>


RTTI_DEF1(rspfEnviHeaderFileWriter,
          "rspfEnviHeaderFileWriter",
          rspfMetadataFileWriter)

rspfEnviHeaderFileWriter::rspfEnviHeaderFileWriter()
   :
   rspfMetadataFileWriter(),
   theHdr()
{
}

rspfEnviHeaderFileWriter::~rspfEnviHeaderFileWriter()
{
}

bool rspfEnviHeaderFileWriter::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   bool result = false;
   
   if (rspfMetadataFileWriter::loadState(kwl, prefix))
   {
      result = theHdr.loadState(kwl, prefix);
   }
   
   return result;
}

bool writeFile()
{
   bool result = true;

   return result;
}

bool rspfEnviHeaderFileWriter::writeFile()
{
   theHdr.setLines(theAreaOfInterest.height());
   theHdr.setSamples(theAreaOfInterest.width());
   theHdr.setBands(theInputConnection->getNumberOfOutputBands());
   
   // Get the geometry from the input.
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

      // Pass it on to envi header to set the map info string from geometry.
      theHdr.setMapInfo(kwl);
   }

   return theHdr.writeFile(theFilename);
}

void rspfEnviHeaderFileWriter::getMetadatatypeList(
   std::vector<rspfString>& metadatatypeList) const
{
   metadatatypeList.push_back(rspfString("envi_header")); 
}

bool rspfEnviHeaderFileWriter::hasMetadataType(
   const rspfString& metadataType)const
{
   return (metadataType == "envi_header");
}
