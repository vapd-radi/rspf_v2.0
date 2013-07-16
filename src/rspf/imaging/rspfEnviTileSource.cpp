//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Image handler class for a raster files with an ENVI header file.
//
//----------------------------------------------------------------------------
// $Id$

#include <rspf/imaging/rspfEnviTileSource.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/projection/rspfAlphaSensorHRI.h>
#include <rspf/projection/rspfAlphaSensorHSI.h>
#include <rspf/support_data/rspfAlphaSensorSupportData.h>
#include <rspf/base/rspfAffineTransform.h>

RTTI_DEF1(rspfEnviTileSource,
          "rspfEnviTileSource",
          rspfGeneralRasterTileSource)

static rspfTrace traceDebug("rspfEnviTileSource:debug");

rspfEnviTileSource::rspfEnviTileSource()
   :
   rspfGeneralRasterTileSource(),
   m_enviHdr()
{
}

rspfEnviTileSource::~rspfEnviTileSource()
{
}

bool rspfEnviTileSource::open()
{
   static const char MODULE[] = "rspfEnviTileSource::open";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\nimage file: " << theImageFile << std::endl;
   }

   bool result = false;
   
   if(isOpen())
   {
      close();
   }

   // Look for a header file:
   rspfFilename hdr = theImageFile;
   hdr.setExtension("hdr"); // image.hdr
   if ( !hdr.exists() )
   {
      hdr = theImageFile;
      hdr.string() += ".hdr"; // image.ras.hdr
   }

   if ( hdr.exists() )
   {
      if ( traceDebug() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "header file: " << hdr << std::endl;
      }

      if ( m_enviHdr.open( hdr ) )
      {
         if ( m_rasterInfo.initializeFromEnviHdr( m_enviHdr ) )
         {
            // Set image file for initializeHandler method.
            m_rasterInfo.setImageFile( theImageFile );
            
            // Look for an omd file:
            rspfFilename omd = theImageFile;
            omd.setExtension("omd"); // image.omd
            if ( !omd.exists() )
            {
               omd.setExtension("kwl"); // image.kwl
            }
            
            if ( omd.exists() )
            {
               if ( traceDebug() )
               {
                  rspfNotify(rspfNotifyLevel_DEBUG) << "omd file: " << omd << std::endl;
               }

               // Pick up adjusted min / max values if present.
               rspfKeywordlist kwl( omd );
               m_rasterInfo.getImageMetaData().updateMetaData( kwl, std::string("") );
            }
           
            theMetaData = m_rasterInfo.getImageMetaData();
            
            result = initializeHandler();
            if ( result )
            {
               completeOpen();
               
               //---
               // This will set default output band list if we are a band selector and 
               // "default bands" key is found in the envi header.  If "default bands"
               // is not found it will set to identity(input = output).
               //---
               setDefaultBandList();
            }
         }
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " Exit status: " << (result?"true":"false") << std::endl;
   }
   
   return result;
}
   
//************************************************************************************************
//! Returns the image geometry object associated with this tile source or NULL if non defined.
//! The geometry contains full-to-local image transform as well as projection (image-to-world)
//************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfEnviTileSource::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      
      if ( !theGeometry )
      {
         // Check the internal geometry first to avoid a factory call.
         theGeometry = getInternalImageGeometry();

         // At this point it is assured theGeometry is set.

         //---
         // WARNING:
         // Must create/set theGeometry at this point or the next call to
         // rspfImageGeometryRegistry::extendGeometry will put us in an infinite loop
         // as it does a recursive call back to rspfImageHandler::getImageGeometry().
         //---         

         // Check for set projection.
         if ( theGeometry.valid()&&!theGeometry->getProjection() )
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

rspfRefPtr<rspfImageGeometry> rspfEnviTileSource::getInternalImageGeometry() const
{
   static const char MODULE[] = "rspfEnviTileSource::getInternalImageGeometry";
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   rspfRefPtr<rspfImageGeometry> geom;

// debugging to turn off model load
//   return geom;

   geom  = new rspfImageGeometry();

    rspfRefPtr<rspfAlphaSensorSupportData> supData = new rspfAlphaSensorSupportData();
    if ( supData->readSupportFiles( m_enviHdr ) )
    {
       if (supData->isHSI())
       {
          rspfRefPtr<rspfAlphaSensorHSI> sensor = new rspfAlphaSensorHSI();
          if ( sensor->initialize( *(supData.get()) ) )
          {
             geom->setProjection( sensor.get() );
          }
       }
       else
       {
          rspfRefPtr<rspfAlphaSensorHRI> sensor = new rspfAlphaSensorHRI();
          if ( sensor->initialize( *(supData.get()) ) )
          {
             geom->setProjection( sensor.get() );
          }
       }
       
    } // Matches: if ( supData->readSupportFiles( m_enviHdr ) )
    else
    {
      geom = 0;
    }

    return geom;
}

bool rspfEnviTileSource::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{
   return rspfGeneralRasterTileSource::saveState(kwl,prefix);
}

bool rspfEnviTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   return rspfGeneralRasterTileSource::loadState(kwl, prefix);
}

rspfRefPtr<rspfProperty> rspfEnviTileSource::getProperty(
   const rspfString& name)const
{
   if(name == "file_type")
   {
      return new rspfStringProperty(name, getShortName());
   }
   
   return rspfImageHandler::getProperty(name);
}

void rspfEnviTileSource::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   rspfImageHandler::getPropertyNames(propertyNames);
   propertyNames.push_back("file_type");
}

rspfString rspfEnviTileSource::getShortName() const
{
   return rspfString("ENVI");
}
   
rspfString rspfEnviTileSource::getLongName() const
{
   return rspfString("ENVI reader");
}
 

void rspfEnviTileSource::setDefaultBandList()
{
   if ( isBandSelector() )
   {
      rspfString value;
      value.string() = m_enviHdr.getMap().findKey( std::string("default bands") );
      if ( value.size() )
      {
         std::vector<rspfString> strLst;
         value.split( strLst, rspfString(","));
         if ( strLst.size() )
         {
            const rspf_uint32 INPUT_BANDS = getNumberOfInputBands();
            std::vector<rspfString>::const_iterator i = strLst.begin();
            std::vector<rspf_uint32> bands;
            rspf_uint32 band = 0;
            while ( i != strLst.end() )
            {
               band = (*i).toUInt32();
               if ( band )
               {
                  // Assuming "default bands" are one based.  Totally a hunch... (drb)
                  --band; 
               }
               else
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                     << "rspfEnviTileSource::setDefaultBandList WARN!"
                     << "\nDetected zero based bands in \"default bands\" from header!"
                     << std::endl;
               }
               
               if ( band < INPUT_BANDS )
               {
                  bands.push_back( band );
               }
               else
               {
                  bands.clear(); // Out of range.
                  break;
               }
               ++i;
            }
            if ( bands.size() )
            {
               rspfImageHandler::setOutputBandList(bands, m_outputBandList);
            }
         }
      }
   }
   if ( m_outputBandList.empty() )
   {
      // Initialized to identity (input = output):
      rspfImageSource::getOutputBandList( m_outputBandList );
   }
}
