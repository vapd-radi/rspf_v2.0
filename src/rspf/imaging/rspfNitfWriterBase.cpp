//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: RSPF Kakadu based nitf writer.
//
//----------------------------------------------------------------------------
// $Id: rspfNitfWriterBase.cpp 2981 2011-10-10 21:14:02Z david.burken $

#include <rspf/imaging/rspfNitfWriterBase.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSourceSequencer.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfRpcSolver.h>
#include <rspf/support_data/rspfNitfBlockaTag.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_X.h>
#include <rspf/support_data/rspfNitfImageHeader.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_X.h>
#include <rspf/support_data/rspfNitfRegisteredTag.h>
#include <rspf/support_data/rspfNitfTagInformation.h>

static const char ENABLE_BLOCKA_KW[] = "enable_blocka_tag";
static const char ENABLE_RPCB_KW[]   = "enable_rpcb_tag";

RTTI_DEF1(rspfNitfWriterBase, "rspfNitfWriterBase", rspfImageFileWriter)
   
static rspfTrace traceDebug(rspfString("rspfNitfWriterBase:debug"));

rspfNitfWriterBase::rspfNitfWriterBase()
   : rspfImageFileWriter(),
     theEnableRpcbTagFlag(false),
     theEnableBlockaTagFlag(true)
{
}

rspfNitfWriterBase::rspfNitfWriterBase(const rspfFilename& filename,
                                         rspfImageSource* inputSource)
   : rspfImageFileWriter(filename, inputSource, 0),
     theEnableRpcbTagFlag(false),
     theEnableBlockaTagFlag(true)
{
}

rspfNitfWriterBase::~rspfNitfWriterBase()
{
}

void rspfNitfWriterBase::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(property.valid())
   {
      rspfString name = property->getName();

      if (name == ENABLE_RPCB_KW)
      {
         theEnableRpcbTagFlag = property->valueToString().toBool();
      }
      else if (name == ENABLE_BLOCKA_KW)
      {
         theEnableBlockaTagFlag = property->valueToString().toBool();
      }
      else
      {
         rspfImageFileWriter::setProperty(property);
      }
   }
}

rspfRefPtr<rspfProperty> rspfNitfWriterBase::getProperty(
   const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;
   
   if(name == ENABLE_RPCB_KW)
   {
      result = new rspfBooleanProperty(name, theEnableRpcbTagFlag);
   }   
   else if(name == ENABLE_BLOCKA_KW)
   {
      result = new rspfBooleanProperty(name, theEnableBlockaTagFlag);
   }   
   else
   {
      result = rspfImageFileWriter::getProperty(name);
   }

   return result;
}

void rspfNitfWriterBase::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   rspfImageFileWriter::getPropertyNames(propertyNames);

   propertyNames.push_back(ENABLE_BLOCKA_KW);
   propertyNames.push_back(ENABLE_RPCB_KW);
}


bool rspfNitfWriterBase::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   kwl.add(prefix, ENABLE_RPCB_KW, theEnableRpcbTagFlag, true);
   kwl.add(prefix, ENABLE_BLOCKA_KW, theEnableBlockaTagFlag, true);

   return rspfImageFileWriter::saveState(kwl, prefix);
}

bool rspfNitfWriterBase::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   // Look for the rpcb enable flag keyword.
   const char* lookup = kwl.find(prefix, ENABLE_RPCB_KW);
   if(lookup)
   {
      rspfString os = lookup;
      theEnableRpcbTagFlag = os.toBool();
   }

   // Look for the blocka enable flag keyword.
   lookup = kwl.find(prefix, ENABLE_BLOCKA_KW);
   if(lookup)
   {
      rspfString os = lookup;
      theEnableBlockaTagFlag = os.toBool();
   }

   return rspfImageFileWriter::loadState(kwl, prefix);
}

void rspfNitfWriterBase::writeGeometry(rspfNitfImageHeaderV2_X* hdr,
                                        rspfImageSourceSequencer* seq)
{
   if (hdr && seq)
   {
      rspfRefPtr<rspfImageGeometry> geom = theInputConnection->getImageGeometry();
      rspfKeywordlist kwl;

      if (geom.valid()&&geom->hasProjection())
      {
         // Get the requested bounding rectangles.
         rspfIrect rect = seq->getBoundingRect();
         
         // See if it's a map projection; else, a sensor model.
         rspfMapProjection* mapProj =
            PTR_CAST(rspfMapProjection, geom->getProjection());
         if (mapProj)
         {
            // Use map info to get the corners.
            rspfMapProjectionInfo mapInfo(mapProj, rect);
            mapInfo.setPixelType(RSPF_PIXEL_IS_AREA);
            
            // See if it's utm.
            rspfUtmProjection* utmProj = PTR_CAST(rspfUtmProjection,
                                                   mapProj);
            if(utmProj)
            {
               rspfDpt ul = mapInfo.ulEastingNorthingPt();
               rspfDpt ur = mapInfo.urEastingNorthingPt();
               rspfDpt lr = mapInfo.lrEastingNorthingPt();
               rspfDpt ll = mapInfo.llEastingNorthingPt();
               
               if(utmProj->getHemisphere() == 'N')
               {
                  hdr->setUtmNorth(utmProj->getZone(), ul, ur, lr, ll);
               }
               else
               {
                  hdr->setUtmSouth(utmProj->getZone(), ul, ur, lr, ll);
               }
            }
            else
            {
               rspfGpt ul = mapInfo.ulGroundPt();
               rspfGpt ur = mapInfo.urGroundPt();
               rspfGpt lr = mapInfo.lrGroundPt();
               rspfGpt ll = mapInfo.llGroundPt();
               hdr->setGeographicLocationDms(ul, ur, lr, ll);
            }

            if (theEnableBlockaTagFlag)
            {
               addBlockaTag(mapInfo, hdr);
            }
         }
         
         if (theEnableRpcbTagFlag)
         {
            addRpcbTag(rect, geom->getProjection(), hdr);
         }
         
      } // matches:  if (proj.valid())
      
   } // matches: if (hdr && seq)
}

void rspfNitfWriterBase::addBlockaTag(rspfMapProjectionInfo& mapInfo,
                                       rspfNitfImageHeaderV2_X* hdr)
{
   if (hdr)
   {
      // Capture the current pixel type.
      rspfPixelType originalPixelType = mapInfo.getPixelType();
      
      // This tag wants corners as area:
      mapInfo.setPixelType(RSPF_PIXEL_IS_AREA);
      
      // Stuff the blocka tag which has six digit precision.
      rspfNitfBlockaTag* blockaTag = new rspfNitfBlockaTag();
      
      // Set the block number.
      blockaTag->setBlockInstance(1);
      
      // Set the number of lines.
      blockaTag->setLLines(mapInfo.linesPerImage());
      
      // Set first row, first column.
      blockaTag->setFrfcLoc(rspfDpt(mapInfo.ulGroundPt()));
      
      // Set first row, last column.
      blockaTag->setFrlcLoc(rspfDpt(mapInfo.urGroundPt()));
      
      // Set last row, last column.
      blockaTag->setLrlcLoc(rspfDpt(mapInfo.lrGroundPt()));
      
      // Set last row, first column.
      blockaTag->setLrfcLoc(rspfDpt(mapInfo.llGroundPt()));
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfWriterBase::addBlockaTag DEBUG:"
            << "\nBLOCKA Tag:" << *((rspfObject*)(blockaTag))
            << std::endl;
      }
      
      // Add the tag to the header.
      rspfRefPtr<rspfNitfRegisteredTag> blockaTagRp = blockaTag;
      rspfNitfTagInformation blockaTagInfo(blockaTagRp);
      hdr->addTag(blockaTagInfo);
      
      // Reset the pixel type to original value
      mapInfo.setPixelType(originalPixelType);
      
   } // matches: if (hdr)
}

void rspfNitfWriterBase::addRpcbTag(const rspfIrect& rect,
                                     rspfProjection* proj,
                                     rspfNitfImageHeaderV2_X* hdr)
{
   if (proj && hdr)
   {
      bool useElevation = false;
      
      if (PTR_CAST(rspfMapProjection, proj))
      {
         // If we're already map projected turn the elevation off.
         useElevation = false;
      }
      
      // Make an rpc solver.
      rspfRefPtr<rspfRpcSolver> rs = new rspfRpcSolver(useElevation);
      
      // Compute the coefficients.
      rs->solveCoefficients(rspfDrect(rect), proj, 64, 64);
      
      // Add the tag.
      rspfRefPtr<rspfNitfRegisteredTag> tag = rs->getNitfRpcBTag();
      rspfNitfTagInformation tagInfo(tag);
      hdr->addTag(tagInfo);
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfWriterBase::addRpcbTag DEBUG:"
            << "\nRPCB Tag:" << *((rspfObject*)(tag.get()))
            << "\nProjection:\n";
         
         proj->print(rspfNotify(rspfNotifyLevel_DEBUG));
         
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "\nRect: " << rect << std::endl;
      }
      
   } // matches: if (proj && hdr)
}

void rspfNitfWriterBase::setComplexityLevel(std::streamoff endPosition,
                                             rspfNitfFileHeaderV2_X* hdr)
{
   if (hdr)
   {
      //---
      // See MIL-STD-2500C, Table A-10:
      //
      // Lots of rules here, but for now we will key off of file size.
      //---
      const std::streamoff MB   = 1024 * 1024;
      const std::streamoff MB50 = 50   * MB;
      const std::streamoff GIG  = 1000 * MB;
      const std::streamoff GIG2 = 2    * GIG;
      
      rspfString complexity = "03"; // Less than 50 mb.
      
      if ( (endPosition >= MB50) && (endPosition < GIG) )
      {
         complexity = "05";
      }
      else if ( (endPosition >= GIG) && (endPosition < GIG2) )
      {
         complexity = "06";
      }
      else if (endPosition >= GIG2)
      {
         complexity = "07";
      }
      
      hdr->setComplexityLevel(complexity);
   }
}

rspfString rspfNitfWriterBase::getExtension() const
{
   return rspfString("ntf");
}

void rspfNitfWriterBase::initializeDefaultsFromConfigFile( rspfNitfFileHeaderV2_X* fileHdr,
                                                            rspfNitfImageHeaderV2_X* imgHdr )
{
   // Look in prefs for site configuration file:
   const char* lookup = rspfPreferences::instance()->
      findPreference("nitf_writer.site_configuration_file");
   if ( lookup && fileHdr && imgHdr )
   {
      rspfKeywordlist kwl;
      if ( kwl.addFile( lookup ) )
      {
         fileHdr->loadState( kwl, "nitf.file." );
         imgHdr->loadState( kwl, "nitf.image." );
      }
   }
}

