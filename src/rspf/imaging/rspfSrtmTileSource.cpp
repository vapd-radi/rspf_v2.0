//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Image handler class for a Shuttle Radar Topography Mission (SRTM) file.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmTileSource.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfSrtmTileSource.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/support_data/rspfSrtmSupportData.h>

RTTI_DEF1(rspfSrtmTileSource,
          "rspfSrtmTileSource",
          rspfGeneralRasterTileSource)

static rspfTrace traceDebug("rspfSrtmTileSource:debug");

rspfSrtmTileSource::rspfSrtmTileSource()
   :
   rspfGeneralRasterTileSource(),
   m_SrtmSupportData()
{
}

rspfSrtmTileSource::~rspfSrtmTileSource()
{
}

bool rspfSrtmTileSource::open()
{
   static const char MODULE[] = "rspfSrtmTileSource::open";
   if (traceDebug())
   {
      CLOG << " Entered..." << std::endl
           << "srtm " << theImageFile << std::endl;
   }
   
   if(isOpen())
   {
      close();
   }

   //---
   // Set the scan for min max flag to true.  This will only scan if not found
   // in a omd file so this is a one time hit typically taken when building
   // reduced res sets.
   //---
   if (!m_SrtmSupportData.setFilename(theImageFile, true))
   {
      if (traceDebug()) CLOG << " Unable to set filename"<< std::endl;
      
      return false;
   }

   rspfKeywordlist kwl;
   m_SrtmSupportData.saveState(kwl);
   
   rspfGeneralRasterInfo generalRasterInfo;
   bool result = generalRasterInfo.loadState(kwl);
   
   // theMetaData.loadState(kwl);
   theMetaData = generalRasterInfo.getImageMetaData();

   if(result)
   {
       result =  rspfGeneralRasterTileSource::open(generalRasterInfo);
   }

   if (traceDebug())
   {
      CLOG << " Exited with " << (result?"true":"false") << " ..."
           << std::endl;
   }
   
   return result;
}
   
//************************************************************************************************
//! Returns the image geometry object associated with this tile source or NULL if non defined.
//! The geometry contains full-to-local image transform as well as projection (image-to-world)
//************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfSrtmTileSource::getImageGeometry()
{
   if(!theGeometry.valid())
   {
      // First time through set the projection.
      theGeometry = new rspfImageGeometry();

      theGeometry->setProjection( m_SrtmSupportData.getProjection().get() );

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   return theGeometry;
}

bool rspfSrtmTileSource::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{
   if (m_SrtmSupportData.saveState(kwl, prefix))
   {
      return rspfImageHandler::saveState(kwl,prefix);
   }
   return false;
}

bool rspfSrtmTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   if (m_SrtmSupportData.loadState(kwl, prefix))
   {
      setFilename(m_SrtmSupportData.getFilename());
      if (open())
      {
         // Must call to pick up id for connections.
         return rspfImageSource::loadState(kwl, prefix);
      }
   }

   return false;
}

rspfRefPtr<rspfProperty> rspfSrtmTileSource::getProperty(
   const rspfString& name)const
{
   if(name == "file_type")
   {
      return new rspfStringProperty(name, getShortName());
   }
   
   return rspfImageHandler::getProperty(name);
}

void rspfSrtmTileSource::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   rspfImageHandler::getPropertyNames(propertyNames);
   propertyNames.push_back("file_type");
}

rspfString rspfSrtmTileSource::getShortName() const
{
   return rspfString("SRTM");
}
   
rspfString rspfSrtmTileSource::getLongName() const
{
   return rspfString("SRTM reader");
}
 
