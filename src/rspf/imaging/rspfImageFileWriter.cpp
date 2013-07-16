//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Frank Warmerdam (warmerdam@pobox.com)
//
// Description:
//
// Contains class declaration for rspfImageFileWriter.
//*******************************************************************
//  $Id: rspfImageFileWriter.cpp 21963 2012-12-04 16:28:12Z dburken $


#include <tiff.h> /* for tiff compression defines */

#include <rspf/rspfConfig.h>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfImageTypeLut.h>
#include <rspf/base/rspfIoStream.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/imaging/rspfTiffOverviewBuilder.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/imaging/rspfMetadataFileWriter.h>
#include <rspf/imaging/rspfGeomFileWriter.h>
#include <rspf/imaging/rspfEnviHeaderFileWriter.h>
#include <rspf/imaging/rspfERSFileWriter.h>
#include <rspf/imaging/rspfFgdcFileWriter.h>
#include <rspf/imaging/rspfReadmeFileWriter.h>
#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/imaging/rspfWorldFileWriter.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>

static rspfTrace traceDebug("rspfImageFileWriter:debug");
static const rspfString AUTO_CREATE_DIRECTORY_KW("auto_create_directory");

#if RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfImageFileWriter.cpp 21963 2012-12-04 16:28:12Z dburken $";
#endif

RTTI_DEF3(rspfImageFileWriter,
          "rspfImageFileWriter",
          rspfImageWriter,
          rspfProcessInterface,
          rspfConnectableObjectListener)

static const char SCALE_TO_EIGHT_BIT_KW[] = "scale_to_eight_bit";

rspfImageFileWriter::rspfImageFileWriter(const rspfFilename& file,
                                           rspfImageSource* inputSource,
                                           rspfObject* owner)
   : rspfImageWriter(owner,
                      1,
                      0,
                      true,
                      false),
     rspfProcessInterface(),
     rspfConnectableObjectListener(),
     theViewController(NULL),
     theProgressListener(NULL),
     theFilename(file),
     theOutputImageType(rspfImageTypeLut().getEntryString(RSPF_IMAGE_TYPE_UNKNOWN)),
     theWriteImageFlag(true),
     theWriteOverviewFlag(false),
     theWriteHistogramFlag(false),
     theScaleToEightBitFlag(false),
     theOverviewCompressType(COMPRESSION_NONE),
     theOverviewJpegCompressQuality(75),
     theWriteEnviHeaderFlag(false),
     theWriteErsHeaderFlag(false),     
     theWriteExternalGeometryFlag(false),
     theWriteFgdcFlag(false),
     theWriteJpegWorldFileFlag(false),
     theWriteReadmeFlag(false),
     theWriteTiffWorldFileFlag(false),
     theWriteWorldFileFlag(false),
     theAutoCreateDirectoryFlag(true),
     theLinearUnits(RSPF_UNIT_UNKNOWN),
     thePixelType(RSPF_PIXEL_IS_POINT)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageFileWriter::rspfImageFileWriter entered..."
         << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif
   }

   theInputConnection = new rspfImageSourceSequencer(inputSource);

   // now map the sequencer to the same input
   connectMyInputTo(0, inputSource);
   initialize();

   // make sure we listen to our input changes so we can change
   // it for the sequencer as well.
   rspfImageWriter::addListener((rspfConnectableObjectListener*)this);

   theInputConnection->connectMyInputTo(0, inputSource, false);
   theAreaOfInterest.makeNan();
}

rspfImageFileWriter::~rspfImageFileWriter()
{
   theInputConnection = 0;
   theProgressListener = NULL;
   removeListener((rspfConnectableObjectListener*)this);
}

void rspfImageFileWriter::initialize()
{
   if(theInputConnection.valid())
   {
      theInputConnection->initialize();

      //---
      // Area Of Interest(AOI):
      // There are two AOIs to keep in sync.
      // One owned by this base: rspfImageWriter::theAreaOfInterest
      // One owned by the theInputConntion: rspfImageSourceSequencer::theAreaOfInterest
      //---
      if ( theAreaOfInterest.hasNans() == false )
      {
         //---
         // rspfImageSourceSequencer::setAreaOfInterest does more than just set the
         // variable, so only call if out of sync to avoid wasted cycles.
         //--- 
         if ( theAreaOfInterest != theInputConnection->getBoundingRect() )
         {
            theInputConnection->setAreaOfInterest( theAreaOfInterest );
         }
      }
      else
      {
         // Set this AOI to bounding rect of input.
         theAreaOfInterest = theInputConnection->getBoundingRect();
      }
   }
}

void rspfImageFileWriter::changeSequencer(rspfImageSourceSequencer* sequencer)
{
   if( sequencer )
   {
      theInputConnection = sequencer;
      theInputConnection->connectMyInputTo(0, getInput(0));
   }
}

bool rspfImageFileWriter::saveState(rspfKeywordlist& kwl,
                                 const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           TYPE_NAME(this),
           true);

   kwl.add(prefix,
           rspfKeywordNames::FILENAME_KW,
           theFilename.c_str(),
           true);

   kwl.add(prefix,
           "create_envi_hdr",
           (rspf_uint32)theWriteEnviHeaderFlag,
           true);

   kwl.add(prefix,
           "create_ers_hdr",
           (rspf_uint32)theWriteErsHeaderFlag,
           true);

   kwl.add(prefix,
           rspfKeywordNames::CREATE_EXTERNAL_GEOMETRY_KW,
           (rspf_uint32)theWriteExternalGeometryFlag,
           true);

   kwl.add(prefix,
           "create_fgdc",
           (rspf_uint32)theWriteFgdcFlag,
           true);
   
   kwl.add(prefix,
           "create_jpeg_world_file",
           (rspf_uint32)theWriteJpegWorldFileFlag,
           true);

   kwl.add(prefix,
           "create_readme",
           (rspf_uint32)theWriteReadmeFlag,
           true);
   
   kwl.add(prefix,
           "create_tiff_world_file",
           (rspf_uint32)theWriteTiffWorldFileFlag,
           true);

   kwl.add(prefix,
           "create_world_file",
           (rspf_uint32)theWriteWorldFileFlag,
           true);

   kwl.add(prefix,
           rspfKeywordNames::CREATE_IMAGE_KW,
           (rspf_uint32)theWriteImageFlag,
           true);

   kwl.add(prefix,
           rspfKeywordNames::CREATE_OVERVIEW_KW,
           (rspf_uint32)theWriteOverviewFlag,
           true);

   kwl.add(prefix,
           rspfKeywordNames::CREATE_HISTOGRAM_KW,
           (rspf_uint32)theWriteHistogramFlag,
           true);

   kwl.add(prefix,
           SCALE_TO_EIGHT_BIT_KW,
           (rspf_uint32)theScaleToEightBitFlag,
           true);
   kwl.add(prefix,
           AUTO_CREATE_DIRECTORY_KW,
           theAutoCreateDirectoryFlag,
           true);
   kwl.add(prefix,
           rspfKeywordNames::OVERVIEW_COMPRESSION_TYPE_KW,
           theOverviewCompressType,
           true);

   kwl.add(prefix,
           rspfKeywordNames::OVERVIEW_COMPRESSION_QUALITY_KW,
           theOverviewJpegCompressQuality,
           true);

   rspfImageTypeLut lut;
   kwl.add(prefix,
           rspfKeywordNames::IMAGE_TYPE_KW,
           theOutputImageType,
           true);

   //---
   // When "theLinearUnits" == "unknown" that is a key to the writer to just
   // use whatever units makes sense for the projection.  Since "unknown"
   // doesn't look good to the average user in a spec file, change to
   // "default" if "unknown".  When read back in by the lookup table since
   // "default" is not a valid "string" theLinearUnits will remain "unknown".
   // Kind of a hack but it works:)  (drb)
   //---
   rspfString os = rspfUnitTypeLut::instance()->
      getEntryString(theLinearUnits);
   if (os == "unknown")
   {
      os = "default";  // Avoid saving as unknown for user readability.
   }
   kwl.add(prefix, "linear_units", os, true);

   // Set the pixel type.
   rspfString pixelType;
   getPixelTypeString(pixelType);
   kwl.add(prefix,
           rspfKeywordNames::PIXEL_TYPE_KW,
           pixelType.c_str(),
           true);

   return rspfSource::saveState(kwl, prefix);
}

bool rspfImageFileWriter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   // Load the state of the base class.
   bool result = rspfSource::loadState(kwl, prefix);

   // Get the filename...
   const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if(lookup)
   {
      rspfString filename = lookup;

      lookup =  kwl.find(prefix, rspfKeywordNames::FILEPATH_KW);
      if(lookup)
      {
         filename = rspfString(lookup) + filename;
      }

      setFilename(filename);
   }

   lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_TYPE_KW);
   if(lookup)
   {
      if(rspfString(lookup).downcase().contains("area"))
      {
         thePixelType = RSPF_PIXEL_IS_AREA;
      }
      else
      {
         thePixelType = RSPF_PIXEL_IS_POINT;
      }
   }

   lookup = kwl.find(prefix, "linear_units");
   if(lookup)
   {
      rspfUnitType units = (rspfUnitType)rspfUnitTypeLut::instance()->
         getEntryNumber(lookup, true);
      if ( (units == RSPF_METERS) ||
           (units == RSPF_FEET)   ||
           (units == RSPF_US_SURVEY_FEET) )
      {
         theLinearUnits = units;
      }
   }

   lookup = kwl.find(prefix, rspfKeywordNames::CREATE_EXTERNAL_GEOMETRY_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theWriteExternalGeometryFlag = s.toBool();
   }
   
   lookup = kwl.find(prefix, "create_envi_hdr");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteEnviHeaderFlag = s.toBool();
   }
   
   lookup = kwl.find(prefix, "create_ers_hdr");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteErsHeaderFlag = s.toBool();
   }

   lookup = kwl.find(prefix, "create_fgdc");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteFgdcFlag = s.toBool();
   }
   
   lookup = kwl.find(prefix, "create_jpeg_world_file");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteJpegWorldFileFlag = s.toBool();
   }
   
   lookup = kwl.find(prefix, "create_readme");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteReadmeFlag = s.toBool();
   }
   
   lookup = kwl.find(prefix, "create_tiff_world_file");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteTiffWorldFileFlag = s.toBool();
   }

   lookup = kwl.find(prefix, "create_world_file");
   if(lookup)
   {
      rspfString s = lookup;
      theWriteWorldFileFlag = s.toBool();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::CREATE_IMAGE_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theWriteImageFlag = s.toBool();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::CREATE_OVERVIEW_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theWriteOverviewFlag = s.toBool();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::CREATE_HISTOGRAM_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theWriteHistogramFlag = s.toBool();
   }

   lookup = kwl.find(prefix, SCALE_TO_EIGHT_BIT_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theScaleToEightBitFlag = s.toBool();
   }

   lookup = kwl.find(prefix, AUTO_CREATE_DIRECTORY_KW);
   if(lookup)
   {
      theAutoCreateDirectoryFlag = rspfString(lookup).toBool();
   }
   lookup = kwl.find(prefix, rspfKeywordNames::OVERVIEW_COMPRESSION_TYPE_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theOverviewCompressType = static_cast<rspf_uint16>(s.toUInt32());
   }

   lookup = kwl.find(prefix, rspfKeywordNames::OVERVIEW_COMPRESSION_QUALITY_KW);
   if(lookup)
   {
      rspfString s = lookup;
      theOverviewJpegCompressQuality = s.toInt32();
   }

   const char* outputImageType = kwl.find(prefix, rspfKeywordNames::IMAGE_TYPE_KW);
   if(outputImageType)
   {
      theOutputImageType = outputImageType;
   }

   if(theInputObjectList.size() != 2)
   {
      theInputObjectList.resize(2);
      theInputListIsFixedFlag = true;
   }

   theOutputListIsFixedFlag = false;

   return result;
}

bool rspfImageFileWriter::writeOverviewFile(rspf_uint16 tiff_compress_type,
                                             rspf_int32 jpeg_compress_quality, 
                                             bool includeR0) 
{
   if(theFilename == "") return false;

   // Get an image handler for the source file.
   rspfRefPtr<rspfImageHandler> ih
      = rspfImageHandlerRegistry::instance()->open(theFilename);

   if (ih.valid() == false)
   {
      // Set the error...
      return false;
   }

   rspfRefPtr<rspfTiffOverviewBuilder> ob = new rspfTiffOverviewBuilder();
   
   if ( ob->setInputSource(ih.get()) )
   {
      // Give the listener to the overview builder if set.
      if (theProgressListener)
      {
         ob->addListener(theProgressListener);
      }
      
      // Build a new file name for the overview.
      rspfFilename overview_file = theFilename;
      overview_file.setExtension(rspfString("ovr"));
      ob->setCompressionType(tiff_compress_type);
      ob->setJpegCompressionQuality(jpeg_compress_quality);
      ob->setOutputFile(overview_file);
      ob->setCopyAllFlag(includeR0);
      ob->execute();

      // Remove the listener from the overview builder.
      if (theProgressListener)
      {
         ob->removeListener(theProgressListener);
      }
   }

   return true;
}

bool rspfImageFileWriter::writeEnviHeaderFile() 
{
   if( (theFilename == "") || !theInputConnection ||
       theAreaOfInterest.hasNans())
   {
      return false;
   }

   // Make the file name.
   rspfFilename outputFile = theFilename;
   outputFile.setExtension(rspfString("hdr"));

   // Make the writer.
   rspfRefPtr<rspfMetadataFileWriter> writer =
      new rspfEnviHeaderFileWriter();
   
   // Set things up.
   writer->connectMyInputTo(0, theInputConnection.get());
   writer->initialize();
   writer->setFilename(outputFile);
   writer->setPixelType(thePixelType);
   writer->setAreaOfInterest(theAreaOfInterest);
   
   // Write it to disk.
   return writer->execute();
}

bool rspfImageFileWriter::writeErsHeaderFile() 
{
   if( (theFilename == "") || !theInputConnection ||
       theAreaOfInterest.hasNans())
   {
      return false;
   }
   
   // Make the file name.
   rspfFilename outputFile = theFilename;
   outputFile.setExtension(rspfString("ers"));

   // Make the writer.
   rspfRefPtr<rspfMetadataFileWriter> writer =
	   new rspfERSFileWriter();
   
   // Set things up.
   writer->connectMyInputTo(0, theInputConnection.get());
   writer->initialize();
   writer->setFilename(outputFile);
   writer->setPixelType(thePixelType);
   writer->setAreaOfInterest(theAreaOfInterest);
   
   // Write it to disk.
   return writer->execute();
}

bool rspfImageFileWriter::writeExternalGeometryFile() 
{
   if( (theFilename == "") || !theInputConnection ||
       theAreaOfInterest.hasNans())
   {
      return false;
   }

   // Make the file name.
   rspfFilename geomFile = theFilename;
   geomFile.setExtension(rspfString("geom"));

   // Make the writer.
   rspfRefPtr<rspfMetadataFileWriter> writer = new rspfGeomFileWriter();

   // Set things up.
//   writer->connectMyInputTo(0, theInputConnection.get());
   writer->connectMyInputTo(0, theInputConnection.get());
   writer->setFilename(geomFile);
   writer->initialize();
   writer->setPixelType(thePixelType);
   writer->setAreaOfInterest(theAreaOfInterest);

   // Write it to disk.
   return writer->execute();
}

bool rspfImageFileWriter::writeFgdcFile() 
{
   if( (theFilename == "") || !theInputConnection ||
       theAreaOfInterest.hasNans())
   {
      return false;
   }

   // Make the file name.
   rspfFilename outputFile = theFilename;
   outputFile.setExtension(rspfString("xml"));

   // Make the writer.
   rspfRefPtr<rspfMetadataFileWriter> writer = new rspfFgdcFileWriter();
   
   // Set things up.
   writer->connectMyInputTo(0, theInputConnection.get());
   writer->initialize();
   writer->setFilename(outputFile);
   writer->setPixelType(thePixelType);
   writer->setAreaOfInterest(theAreaOfInterest);
   
   // Write it to disk.
   return writer->execute();
}

bool rspfImageFileWriter::writeJpegWorldFile() 
{
   if(theFilename == "")
   {
      return false;
   }
   
   // Make the file name.
   rspfFilename file = theFilename;
   file.setExtension(rspfString("jpw")); // jpw???

   return writeWorldFile(file);
}

bool rspfImageFileWriter::writeReadmeFile() 
{
   if( (theFilename == "") || !theInputConnection ||
       theAreaOfInterest.hasNans())
   {
      return false;
   }

   // Make the file name.
   rspfFilename outputFile = theFilename.noExtension();
   
   outputFile += "_readme.txt";

   // Make the writer.
   rspfRefPtr<rspfMetadataFileWriter> writer =
      new rspfReadmeFileWriter();
   
   // Set things up.
   writer->connectMyInputTo(0, theInputConnection.get());
   writer->initialize();
   writer->setFilename(outputFile);
   writer->setPixelType(thePixelType);
   writer->setAreaOfInterest(theAreaOfInterest);
   
   // Write it to disk.
   return writer->execute();
}

bool rspfImageFileWriter::writeTiffWorldFile() 
{
   if(theFilename == "")
   {
      return false;
   }
   
   // Make the file name.
   rspfFilename file = theFilename;
   file.setExtension(rspfString("tfw"));

   return writeWorldFile(file);
}

bool rspfImageFileWriter::writeMetaDataFiles() 
{
   bool status = true;
   
   if(theWriteEnviHeaderFlag&&!needsAborting())
   {
      if(writeEnviHeaderFile() == false)
      {
         status = false;

         rspfNotify(rspfNotifyLevel_WARN)
            << "Write of envi header file failed!" << endl;
      }
   }

   if(theWriteErsHeaderFlag&&!needsAborting())
   {
      if(writeErsHeaderFile() == false)
      {
         status = false;

         rspfNotify(rspfNotifyLevel_WARN)
            << "Write of ers header file failed!" << endl;
      }
   }

   if (theWriteExternalGeometryFlag&&!needsAborting())
   {
      if( writeExternalGeometryFile() == false)
      {
         status = false;
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of geometry file failed!" << endl;
         }
      }
   }
   if(theWriteFgdcFlag&&!needsAborting())
   {
      if(writeFgdcFile() == false)
      {
         status = false;
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of fgdc file failed!" << endl;
         }
      }
   }
   if(theWriteJpegWorldFileFlag&&!needsAborting())
   {
      if(writeJpegWorldFile() == false)
      {
         status = false;
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of jpeg world file failed!" << endl;
         }
      }
   }
   if(theWriteReadmeFlag&&!needsAborting())
   {
      if(writeReadmeFile() == false)
      {
         status = false;
         
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of readme file failed!" << endl;
         }
      }
   }
   if(theWriteTiffWorldFileFlag&&!needsAborting())
   {
      if(writeTiffWorldFile() == false)
      {
         status = false;
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of tiff world file failed!" << endl;
         }
      }
   }

   if(theWriteWorldFileFlag&&!needsAborting())
   {
      if(writeWorldFile() == false)
      {
         status = false;
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of world file failed!" << endl;
         }
      }
   }
   
   if (theWriteHistogramFlag&&!needsAborting())
   {
      if (!writeHistogramFile())
      {
         status = false;
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of histogram file failed!" << endl;
         }
      }
   }

   return status;
}

bool rspfImageFileWriter::writeHistogramFile() 
{
   if(needsAborting()) return false;
   if(theFilename == "") return false;

   rspfFilename histogram_file = theFilename;
   histogram_file.setExtension(rspfString("his"));

   rspfRefPtr<rspfImageHandler> handler = rspfImageHandlerRegistry::instance()->
      open(theFilename);

   if (!handler)
   {
      return false;
   }

   rspfRefPtr<rspfImageHistogramSource> histoSource = new rspfImageHistogramSource;

   rspfRefPtr<rspfHistogramWriter> writer = new rspfHistogramWriter;
   histoSource->setMaxNumberOfRLevels(1); // Only compute for r0.
   histoSource->connectMyInputTo(0, handler.get());
   histoSource->enableSource();
   writer->connectMyInputTo(0, histoSource.get());
   writer->setFilename(histogram_file);
   writer->addListener(&theStdOutProgress);
   writer->execute();
   writer->disconnect();
   histoSource->disconnect();
   handler->disconnect();
   writer = 0; histoSource = 0; handler = 0;
   return true;
}

void rspfImageFileWriter::disconnectInputEvent(rspfConnectionEvent& /* event */)
{
   if(!getInput(0))
   {
      theInputConnection->disconnectMyInput((rspf_int32)0, false);
   }
   theViewController = PTR_CAST(rspfViewController, getInput(1));

   initialize();
}

void rspfImageFileWriter::connectInputEvent(rspfConnectionEvent& /* event */)
{
   theInputConnection->connectMyInputTo((rspf_int32)0,
                                        getInput(0),
                                        false);
   theViewController = PTR_CAST(rspfViewController, getInput(1));
   initialize();
}

void rspfImageFileWriter::propertyEvent(rspfPropertyEvent& /* event */)
{
   initialize();
}

void rspfImageFileWriter::setOutputImageType(const rspfString& type)
{
   theOutputImageType = type;
}

void rspfImageFileWriter::setOutputImageType(rspf_int32 type)
{
   rspfImageTypeLut lut;
   theOutputImageType = lut.getEntryString(type);
}

rspfString rspfImageFileWriter::getOutputImageTypeString() const
{
   return theOutputImageType;

}

rspf_int32 rspfImageFileWriter::getOutputImageType() const
{
   rspfImageTypeLut lut;
   return lut.getEntryNumber(theOutputImageType);
}


bool rspfImageFileWriter::getWriteImageFlag() const
{
   return theWriteImageFlag;
}

bool rspfImageFileWriter::getWriteHistogramFlag() const
{
   return theWriteHistogramFlag;
}

bool rspfImageFileWriter::getWriteOverviewFlag() const
{
   return theWriteOverviewFlag;
}

bool rspfImageFileWriter::getScaleToEightBitFlag() const
{
   return theScaleToEightBitFlag;
}

bool rspfImageFileWriter::getWriteEnviHeaderFlag() const
{
   return theWriteEnviHeaderFlag;
}

bool rspfImageFileWriter::getWriteErsHeaderFlag() const
{
   return theWriteErsHeaderFlag;
}

bool rspfImageFileWriter::getWriteExternalGeometryFlag() const
{
   return theWriteExternalGeometryFlag;
}

bool rspfImageFileWriter::getWriteFgdcFlag() const
{
   return theWriteFgdcFlag;
}

bool rspfImageFileWriter::getWriteJpegWorldFileFlag() const
{
   return theWriteJpegWorldFileFlag;
}

bool rspfImageFileWriter::getWriteReadmeFlag() const
{
   return theWriteReadmeFlag;
}

bool rspfImageFileWriter::getWriteTiffWorldFileFlag() const
{
   return theWriteTiffWorldFileFlag;
}

bool rspfImageFileWriter::getWriteWorldFileFlag() const
{
   return theWriteWorldFileFlag;
}

void rspfImageFileWriter::setWriteOverviewFlag(bool flag)
{
   theWriteOverviewFlag = flag;
}

void rspfImageFileWriter::setWriteImageFlag(bool flag)
{
   theWriteImageFlag = flag;
}

void rspfImageFileWriter::setWriteHistogramFlag(bool flag)
{
   theWriteHistogramFlag = flag;
}

void rspfImageFileWriter::setScaleToEightBitFlag(bool flag)
{
   theScaleToEightBitFlag = flag;
}

void rspfImageFileWriter::setWriteEnviHeaderFlag(bool flag)
{
   theWriteEnviHeaderFlag = flag;
}

void rspfImageFileWriter::setWriteErsHeaderFlag(bool flag)
{
   theWriteErsHeaderFlag = flag;
}

void rspfImageFileWriter::setWriteExternalGeometryFlag(bool flag)
{
   theWriteExternalGeometryFlag = flag;
}

void rspfImageFileWriter::setWriteFgdcFlag(bool flag)
{
   theWriteFgdcFlag = flag;
}

void rspfImageFileWriter::setWriteJpegWorldFile(bool flag)
{
   theWriteJpegWorldFileFlag  = flag;
}

void rspfImageFileWriter::setWriteReadme(bool flag)
{
   theWriteReadmeFlag = flag;
}

void rspfImageFileWriter::setWriteTiffWorldFile(bool flag)
{
   theWriteTiffWorldFileFlag  = flag;
}

void rspfImageFileWriter::setWriteWorldFile(bool flag)
{
   theWriteWorldFileFlag  = flag;
}

rspfObject* rspfImageFileWriter::getObject()
{
   return this;
}

const rspfObject* rspfImageFileWriter::getObject() const
{
   return this;
}

void rspfImageFileWriter::setAreaOfInterest(const rspfIrect& inputRect)
{
   // This sets "theAreaOfInterest".
   rspfImageWriter::setAreaOfInterest(inputRect);
   
   if( theInputConnection.valid() )
   {
      //---
      // rspfImageSourceSequencer::setAreaOfInterest does more than just set the
      // variable, so only call if out of sync to avoid wasted cycles.
      //--- 
      if ( theAreaOfInterest != theInputConnection->getBoundingRect() )
      {
         // Set the sequencer area of interest.
         theInputConnection->setAreaOfInterest(inputRect);
      }
   }
}

rspfImageSourceSequencer* rspfImageFileWriter::getSequencer()
{
   return theInputConnection.get();
}

bool rspfImageFileWriter::execute()
{
   static const char MODULE[] = "rspfImageFileWriter::execute";
   
   if (!theInputConnection) // Must have a sequencer...
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:\ntheInputConnection is NULL!" << endl;
      }
      return false;
   }
   
   if (theFilename == "") // Must have a filename...
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:\ntheFilename is empty!" << endl;
      }
      return false;
   }
   
   
   // make sure we have a region of interest
   if(theAreaOfInterest.hasNans())
   {
      theInputConnection->initialize();
      theAreaOfInterest = theInputConnection->getAreaOfInterest();
   }
   else
   {
      theInputConnection->setAreaOfInterest(theAreaOfInterest);
   }

   if(theAreaOfInterest.hasNans()) // Must have an area of interest...
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " ERROR:  Area of interest has nans!"
            << "Area of interest:  "
            << theAreaOfInterest
            << "\nReturning..." << endl;
      }

      return false;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:\n"
         << "Area of interest:  " << theAreaOfInterest << endl;
   }
   
   rspfRefPtr<rspfImageSource> savedInput;
   if (theScaleToEightBitFlag)
   {
      if(theInputConnection->getOutputScalarType() != RSPF_UINT8)
      {
         savedInput = new rspfScalarRemapper;
         
         savedInput->connectMyInputTo(0, theInputConnection->getInput(0));
         theInputConnection->connectMyInputTo(0, savedInput.get());
         theInputConnection->initialize();
      }
   }

   rspfFilename path = theFilename.path();
   if(!path.exists())
   {
      path.createDirectory(true);
   }
   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_EXECUTING);
   //setPercentComplete(0.0); let writeFile set the precent complete
   bool wroteFile = true;
   bool result    = true;
   if (theWriteImageFlag)
   {
      wroteFile = writeFile();
   }
  
   /*
   if(needsAborting())
   {
      setProcessStatus(rspfProcessInterface::PROCESS_STATUS_ABORTED);
   }
   else
   {
      setProcessStatus(rspfProcessInterface::PROCESS_STATUS_NOT_EXECUTING);
   }
   */
   if (theWriteOverviewFlag&&!needsAborting())
   {
      // Writing overviews has been mpi'd so do on all processes.
      if (!writeOverviewFile(theOverviewCompressType,
                             theOverviewJpegCompressQuality))
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "Write of overview file failed!" << endl;
         }
      }
   }

   if(!needsAborting())
   {
      // Do these only on the master process. Note left to right precedence!
      if (getSequencer() && getSequencer()->isMaster())
      {
         if (wroteFile)
         {
            if (writeMetaDataFiles() == false)
            {
               if(traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                  << "Write of metadata file failed!" << endl;
               }
               result = false;
            }
         }
         
      } // End of "if (getSequencer() && getSequencer()->isMaster()))
   }

   if(savedInput.valid())
   {
      rspfConnectableObject* obj = theInputConnection->getInput(0);
      if(obj)
      {
         theInputConnection->connectMyInputTo(0, obj->getInput(0));
      }
   }
   if(!needsAborting())
   {
      setProcessStatus(rspfProcessInterface::PROCESS_STATUS_NOT_EXECUTING);
   }
   else 
   {
      setProcessStatus(rspfProcessInterface::PROCESS_STATUS_ABORTED);   
   }

   savedInput = 0;
   return result;
}

bool rspfImageFileWriter::writeStream()
{
   //---
   // Not implemented in this class. Derived classed should implement if
   // they can write to a stream.
   //--- 
   return false;
}

rspfRefPtr<rspfOStream> rspfImageFileWriter::getOutputStream() const
{
   //---
   // Not implemented in this class. Derived classed should implement if
   // they can write to a stream.
   //--- 
   return rspfRefPtr<rspfOStream>();  
}

bool rspfImageFileWriter::setOutputStream(
   rspfRefPtr<rspfOStream> /* stream */ )
{
   //---
   // Not implemented in this class. Derived classed should implement if
   // they can write to a stream.
   //---
   return false;
}

bool rspfImageFileWriter::setOutputStream(std::ostream& /* str */)
{
   //---
   // Not implemented in this class. Derived classed should implement if
   // they can write to a stream.
   //---
   return false;
}


void rspfImageFileWriter::setPercentComplete(double percentComplete)
{
   rspfProcessInterface::setPercentComplete(percentComplete);
   rspfProcessProgressEvent event(this, percentComplete);
   fireEvent(event);
}

void rspfImageFileWriter::setOutputName(const rspfString& outputName)
{
   rspfImageWriter::setOutputName(outputName);
   setFilename(rspfFilename(outputName));
}

void rspfImageFileWriter::setFilename(const rspfFilename& file)
{
   theFilename = file;
}

const rspfFilename& rspfImageFileWriter::getFilename()const
{
   return theFilename;
}

bool rspfImageFileWriter::canConnectMyInputTo(rspf_int32 inputIndex,
                                               const rspfConnectableObject* object)const
{
   return (object&&
           ( (PTR_CAST(rspfImageSource, object)&&inputIndex == 0)||
             (PTR_CAST(rspfViewController, object)&&inputIndex == 1)) );
}

rspf_uint16 rspfImageFileWriter::getOverviewCompressType() const
{
   return theOverviewCompressType;
}

rspf_int32  rspfImageFileWriter::getOverviewJpegCompressQuality() const
{
   return theOverviewJpegCompressQuality;
}

void rspfImageFileWriter::setOverviewCompressType(rspf_uint16 type)
{
   theOverviewCompressType = type;
}

void rspfImageFileWriter::setOverviewJpegCompressQuality(rspf_int32 quality)
{
   theOverviewJpegCompressQuality = quality;
}

void rspfImageFileWriter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;
   
   if(property->getName() == rspfKeywordNames::FILENAME_KW)
   {
      theFilename = property->valueToString();
   }
   else if (property->getName() == "create_envi_hdr")
   {
      theWriteEnviHeaderFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "create_ers_hdr")
   {
      theWriteErsHeaderFlag = property->valueToString().toBool();
   }
   else if (property->getName() ==
            rspfKeywordNames::CREATE_EXTERNAL_GEOMETRY_KW)
   {
      theWriteExternalGeometryFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "create_fgdc")
   {
      theWriteFgdcFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "create_jpeg_world_file")
   {
      theWriteJpegWorldFileFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "create_readme")
   {
      theWriteReadmeFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "create_tiff_world_file")
   {
      theWriteTiffWorldFileFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "create_world_file")
   {
      theWriteWorldFileFlag = property->valueToString().toBool();
   }
   else if (property->getName() == rspfKeywordNames::CREATE_IMAGE_KW)
   {
      theWriteImageFlag = property->valueToString().toBool();
   }
   else if (property->getName() == rspfKeywordNames::CREATE_OVERVIEW_KW)
   {
      theWriteOverviewFlag = property->valueToString().toBool();
   }
   else if (property->getName() == rspfKeywordNames::CREATE_HISTOGRAM_KW)
   {
      theWriteHistogramFlag = property->valueToString().toBool();
   }
   else if (property->getName() == SCALE_TO_EIGHT_BIT_KW)
   {
      theScaleToEightBitFlag = property->valueToString().toBool();
   }
   else if(property->getName() == AUTO_CREATE_DIRECTORY_KW)
   {
      theAutoCreateDirectoryFlag = property->valueToString().toBool();
   }
   else if (property->getName() == "linear_units")
   {
      theLinearUnits = (rspfUnitType)
         (rspfUnitTypeLut::instance()->
          getEntryNumber(property->valueToString()));
   }
   else if(property->getName() == rspfKeywordNames::IMAGE_TYPE_KW)
   {
      setOutputImageType(property->valueToString());
   }
   else if(property->getName() == rspfKeywordNames::PIXEL_TYPE_KW)
   {
      rspfStringProperty* stringProperty = PTR_CAST(rspfStringProperty,
                                                     property.get());
      if (stringProperty)
      {
         rspfString s;
         stringProperty->valueToString(s);
         setPixelType(s);
      }
   }
   else
   {
      rspfImageWriter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfImageFileWriter::getProperty(const rspfString& name)const
{
   if(name == rspfKeywordNames::FILENAME_KW)
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, theFilename);

      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_OUTPUT);

      return filenameProp;
   }
   else if(name == rspfKeywordNames::IMAGE_TYPE_KW)
   {
      rspfStringProperty* prop =  new rspfStringProperty(name, theOutputImageType, false);
      std::vector<rspfString> typeList;
      getImageTypeList(typeList);
      if(typeList.size() > 1)
      {
         prop->setReadOnlyFlag(false);
         prop->setConstraints(typeList);
         prop->setChangeType(rspfProperty::rspfPropertyChangeType_AFFECTS_OTHERS);
      }
      else
      {
         prop->setReadOnlyFlag(true);
      }
      return prop;
   }
   else if(name == AUTO_CREATE_DIRECTORY_KW)
   {
      return new rspfBooleanProperty(name, theAutoCreateDirectoryFlag);
   }
   else if (name == "create_envi_hdr")
   {
      return new rspfBooleanProperty(name, theWriteEnviHeaderFlag);
   }
   else if (name == "create_ers_hdr")
   {
      return new rspfBooleanProperty(name, theWriteErsHeaderFlag);
   }
   else if (name == rspfKeywordNames::CREATE_EXTERNAL_GEOMETRY_KW)
   {
      return new rspfBooleanProperty(name, theWriteExternalGeometryFlag);
   }
   else if (name == "create_fgdc")
   {
      return new rspfBooleanProperty(name, theWriteFgdcFlag);
   }
   else if (name == "create_jpeg_world_file")
   {
      return new rspfBooleanProperty(name, theWriteJpegWorldFileFlag);
   }
   else if (name == "create_readme")
   {
      return new rspfBooleanProperty(name, theWriteReadmeFlag);
   }
   else if (name == "create_tiff_world_file")
   {
      return new rspfBooleanProperty(name, theWriteTiffWorldFileFlag);
   }
   else if (name == "create_world_file")
   {
      return new rspfBooleanProperty(name, theWriteWorldFileFlag);
   }
   else if (name == rspfKeywordNames::CREATE_IMAGE_KW)
   {
      return new rspfBooleanProperty(name, theWriteImageFlag);
   }
   else if (name == rspfKeywordNames::CREATE_OVERVIEW_KW)
   {
      return new rspfBooleanProperty(name, theWriteOverviewFlag);
   }
   else if (name == rspfKeywordNames::CREATE_HISTOGRAM_KW)
   {
      return new rspfBooleanProperty(name, theWriteHistogramFlag);
   }
   else if (name == SCALE_TO_EIGHT_BIT_KW)
   {
      return new rspfBooleanProperty(name, theScaleToEightBitFlag);
   }
   else if (name == "linear_units")
   {
      // property value
      rspfString value = rspfUnitTypeLut::instance()->
         getEntryString(theLinearUnits);
      
      //---
      // theLinearUnits is set to unknown as a default.  This keys the
      // writer to use whatever makes sense for the given output.
      // Since "unknown" is a bad name to present to users, we will
      // use "default" if theLinearUnits is == "unknown".
      //---
      if (value == "unknown")
      {
         value = "default";
      }
      
      // constraint list
      vector<rspfString> constraintList;
      constraintList.push_back(rspfString("default"));
      constraintList.push_back(rspfString("meters"));
      constraintList.push_back(rspfString("feet"));
      constraintList.push_back(rspfString("us_survey_feet"));
      
      return new rspfStringProperty(name,
                                     value,
                                     false, // not editable
                                     constraintList);
   }
   else if (name == rspfKeywordNames::PIXEL_TYPE_KW)
   {
      rspfString value;
      getPixelTypeString(value);
      rspfStringProperty* stringProp =
         new rspfStringProperty(name,
                                 value,
                                 false); // editable flag
      stringProp->addConstraint(rspfString("point"));
      stringProp->addConstraint(rspfString("area"));
      return stringProp;
   }

   return rspfImageWriter::getProperty(name);
}

void rspfImageFileWriter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageWriter::getPropertyNames(propertyNames);

   propertyNames.push_back(rspfString(rspfKeywordNames::FILENAME_KW));
   propertyNames.push_back(rspfString("create_envi_hdr"));  
   propertyNames.push_back(rspfString(rspfKeywordNames::CREATE_EXTERNAL_GEOMETRY_KW));
   propertyNames.push_back(rspfString("create_fgdc"));
   propertyNames.push_back(rspfString("create_jpeg_world_file"));
   propertyNames.push_back(rspfString("create_readme"));
   propertyNames.push_back(rspfString("create_tiff_world_file"));
   propertyNames.push_back(rspfString("create_world_file"));   
   propertyNames.push_back(rspfString(rspfKeywordNames::CREATE_IMAGE_KW));
   propertyNames.push_back(rspfString(rspfKeywordNames::CREATE_OVERVIEW_KW));
   propertyNames.push_back(rspfString(rspfKeywordNames::CREATE_HISTOGRAM_KW));
   propertyNames.push_back(rspfString(AUTO_CREATE_DIRECTORY_KW));
   propertyNames.push_back(rspfString(SCALE_TO_EIGHT_BIT_KW));
   propertyNames.push_back(rspfString("linear_units"));
   propertyNames.push_back(rspfString(rspfKeywordNames::PIXEL_TYPE_KW));
   propertyNames.push_back(rspfString(rspfKeywordNames::IMAGE_TYPE_KW));

}

rspfString rspfImageFileWriter::getExtension() const
{
   // default unknown file extension
   return rspfString("ext");
}

bool rspfImageFileWriter::getOutputHasInternalOverviews( void ) const
{
   // default value: don't assume internal overviews will
   // be generated by image writers.
   return false;
}

bool rspfImageFileWriter::hasImageType(const rspfString& imageType) const
{
   std::vector<rspfString> imageTypeList;
   getImageTypeList(imageTypeList);

   std::vector<rspfString>::const_iterator i = imageTypeList.begin();

   while (i != imageTypeList.end())
   {
      if ( (*i) == imageType )
      {
         return true;
      }
      ++i;
   }

   return false; // not found...
}

bool rspfImageFileWriter::addListener(rspfListener* listener)
{
   theProgressListener = listener;
   return rspfImageWriter::addListener(listener);
}

bool rspfImageFileWriter::removeListener(rspfListener* listener)
{
   if (listener == theProgressListener)
   {
      theProgressListener = NULL;
   }
   return rspfImageWriter::removeListener(listener);
}

void rspfImageFileWriter::setPixelType(rspfPixelType type)
{
   thePixelType = type;
}

void rspfImageFileWriter::setPixelType(const rspfString& type)
{
   rspfString s = type;
   s.downcase();
   if (s == "point")
   {
      setPixelType(RSPF_PIXEL_IS_POINT);
   }
   else if (s == "area")
   {
      setPixelType(RSPF_PIXEL_IS_AREA);
   }
}

rspfPixelType rspfImageFileWriter::getPixelType() const
{
   return thePixelType;
}

void rspfImageFileWriter::getPixelTypeString(rspfString& type) const
{
   if (thePixelType == RSPF_PIXEL_IS_POINT)
   {
      type = "point";
   }
   else
   {
      type = "area";
   }
}

void rspfImageFileWriter::setTileSize(const rspfIpt& tileSize)
{
   if (theInputConnection.valid())
   {
      theInputConnection->setTileSize(tileSize);
   }
}

bool rspfImageFileWriter::writeWorldFile() 
{
   bool result = false;
   
   if( theFilename.size() )
   {
      // Make the file name.
      rspfFilename file = theFilename;
      rspfString ext = file.ext().downcase();
      
      rspfString wfExt = ""; // World file extension.
      
      if ( ( ext == "tif" ) || ( ext == "tiff" ) )
      {
         wfExt = "tfw";
      }
      else if ( ( ext == "jpg" ) || ( ext == "jpeg" ) )
      {
         wfExt = "jpw";
      }
      else if ( ext == "png" )
      {
         wfExt = "pgw";
      }
      else if ( ext == "sid" )
      {
         wfExt = "sdw";
      }
      
      if ( ext.size() )
      {
         file.setExtension(wfExt);
         
         result = writeWorldFile(file);
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Could not derive world file extension for: " << theFilename << "\n"; 
      }
   }
   
   if ( !result )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Write of world file failed!\n";
   }   
   
   return result;
}

bool rspfImageFileWriter::writeWorldFile(const rspfFilename& file) 
{
   if( !theInputConnection || theAreaOfInterest.hasNans())
   {
      return false;
   }

   // Make the writer.
   rspfRefPtr<rspfWorldFileWriter> writer = new rspfWorldFileWriter();
   
   // Set things up.
   writer->connectMyInputTo(0, theInputConnection.get());
   writer->initialize();
   writer->setFilename(file);
   writer->setAreaOfInterest(theAreaOfInterest);
   writer->setLinearUnits(theLinearUnits);
   writer->setPixelType(thePixelType);
   
   // Write it to disk.
   return writer->execute(); 
}
