//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Chong-Ket Chuah
//
// Description:
//
// Implementation for the class "rspfERSTileSource".  rspfERSTileSource
// is used for reading ER Mapper raster file format.  The format consists
// of a raster file with no extension and a header file with the same name 
// as the raster file but with an .ers extension.
//
//*******************************************************************
//  $Id: rspfERSTileSource.cpp 21512 2012-08-22 11:53:57Z dburken $

#include <rspf/imaging/rspfERSTileSource.h>
#include <rspf/support_data/rspfERS.h>

#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>

RTTI_DEF1(rspfERSTileSource, "rspfERSTileSource", rspfGeneralRasterTileSource);

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfERSTileSource::rspfERSTileSource()
   :  rspfGeneralRasterTileSource(),
      theHdr(NULL)
{
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfERSTileSource::rspfERSTileSource(const rspfKeywordlist& kwl,
                                               const char* prefix)
   :  rspfGeneralRasterTileSource(),
      theHdr(NULL)
{
   if (loadState(kwl, prefix) == false)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }
}

//*******************************************************************
// Destructor:
//*******************************************************************
rspfERSTileSource::~rspfERSTileSource()
{
   if (theHdr)
   {
      delete theHdr;
      theHdr = NULL;
   }
}

bool rspfERSTileSource::open(const rspfFilename& fileName)
{
   // Open and parse the header file
   theHdr = new rspfERS(fileName);
   
//   theHdr->dump(cout);
   
   if(theHdr->errorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      delete theHdr;
      theHdr = NULL;
      return false;
   }
   else
   {
      vector<rspfFilename> f;
	  rspfFilename fne;
	  rspfFilename fpath = fileName.path();
	  if (fpath.empty())
		fne = fileName.fileNoExtension();
	  else
		fne = fpath+"/"+fileName.fileNoExtension();
      f.push_back(fne);
      rspfGeneralRasterInfo genRasterInfo(f,
                                           theHdr->theCelltype,
                                           RSPF_BIL,
                                           theHdr->theBands,
                                           theHdr->theLine,
                                           theHdr->theSample,
                                           0,
                                           rspfGeneralRasterInfo::NONE,
                                           0);
      if(theHdr->theHasNullCells)
      {
         rspf_uint32 i = 0;
         rspf_uint32 bands = static_cast<rspf_uint32>(theHdr->theBands);
         for(i = 0; i < bands; ++i)
         {
            genRasterInfo.getImageMetaData().setNullPix(i, theHdr->theNullCell);
         }
      }
      rspfFilename metadataFile = fne;
      metadataFile = metadataFile + ".omd";

      if(metadataFile.exists())
      {
         rspfKeywordlist kwl;
      
         kwl.addFile(metadataFile.c_str());
      
         theMetaData.loadState(kwl);
         rspf_uint32 i = 0;
         rspf_uint32 bands = static_cast<rspf_uint32>(theHdr->theBands);
         for(i = 0; i < bands; ++i)
         {
            if(theMetaData.getMinValuesValidFlag())
            {
               genRasterInfo.getImageMetaData().setMinPix(i, theMetaData.getMinPix(i));
            }
            if(theMetaData.getMaxValuesValidFlag())
            {
               genRasterInfo.getImageMetaData().setMaxPix(i, theMetaData.getMaxPix(i));
            }
            if(theMetaData.getNullValuesValidFlag())
            {
               genRasterInfo.getImageMetaData().setNullPix(i, theMetaData.getNullPix(i));
            }
         }
      }
      rspfNotify(rspfNotifyLevel_INFO)
         << "general raster info is\n";
      genRasterInfo.print(rspfNotify(rspfNotifyLevel_INFO));
      rspfGeneralRasterTileSource::open(genRasterInfo);
   }
	   
   return true;
}
   

rspfRefPtr<rspfImageGeometry> rspfERSTileSource::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      
      if ( !theGeometry )
      {
         
         theGeometry = new rspfImageGeometry;

         if(theHdr)
         {
            rspfKeywordlist kwl;
            if ( theHdr->toOssimProjectionGeom(kwl) )
            {
               theGeometry->loadState(kwl);  
            }
         }

         // At this point it is assured theGeometry is set.
         
         //---
         // WARNING:
         // Must create/set the geometry at this point or the next call to
         // rspfImageGeometryRegistry::extendGeometry will put us in an infinite loop
         // as it does a recursive call back to rspfImageHandler::getImageGeometry().
         //---         
         
         // Check for set projection.
         if ( !theGeometry->getProjection() )
         {
            // Try factories for projection.
            rspfImageGeometryRegistry::instance()->extendGeometry(this);
         }
      }

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   return theGeometry;
}

bool rspfERSTileSource::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);

   if (lookup)
   {
      rspfFilename fileName = lookup;

      bool result =  open(fileName);
      rspfImageSource::loadState(kwl, prefix);
      return result;
   }
      
   return rspfGeneralRasterTileSource::loadState(kwl, prefix);
}

rspfString rspfERSTileSource::getShortName() const
{
   return rspfString("ERS");
}

rspfString rspfERSTileSource::getLongName() const
{
   return rspfString("ER Mapper Raster");
}

rspfString  rspfERSTileSource::className() const
{
   return rspfString("rspfERSTileSource");
}

