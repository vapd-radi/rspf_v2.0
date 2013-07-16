//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNitf20Writer.cpp 2982 2011-10-10 21:28:55Z david.burken $


#include <rspf/imaging/rspfNitf20Writer.h>
#include <rspf/imaging/rspfNitfTileSource.h>
#include <rspf/base/rspfDate.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfRpcSolver.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/imaging/rspfRectangleCutFilter.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/support_data/rspfNitfCommon.h>
#include <rspf/support_data/rspfNitfGeoPositioningTag.h>
#include <rspf/support_data/rspfNitfLocalGeographicTag.h>
#include <rspf/support_data/rspfNitfLocalCartographicTag.h>
#include <rspf/support_data/rspfNitfProjectionParameterTag.h>
#include <rspf/support_data/rspfNitfNameConversionTables.h>
#include <rspf/support_data/rspfNitfBlockaTag.h>
#include <tiffio.h>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>

RTTI_DEF1(rspfNitf20Writer, "rspfNitf20Writer", rspfNitfWriterBase);

static rspfTrace traceDebug(rspfString("rspfNitfWriter:debug"));
     
static rspfString monthConversionTable[] = {"   ", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

rspfNitf20Writer::rspfNitf20Writer(const rspfFilename& filename,
                                     rspfImageSource* inputSource)
   : rspfNitfWriterBase(filename, inputSource),
     theOutputStream(0),
     theFileHeader(),
     theImageHeader(),
     theEnableRpcbTagFlag(false),
     theEnableBlockaTagFlag(true),
     theCopyFieldsFlag(false)
{
   //---
   // Since the internal nitf tags are not very accurate, write an external
   // geometry out as default behavior.  Users can disable this via the
   // property interface or keyword list.
   //---
   setWriteExternalGeometryFlag(true);
   
   theFileHeader      = new rspfNitfFileHeaderV2_0;
   theImageHeader     = new rspfNitfImageHeaderV2_0;
   theOutputImageType = "nitf20_block_band_separate";

   // These are always set:
   theFileHeader->setEncryption( rspfString("0") );
   theImageHeader->setJustification( rspfString("R") );
   
   // Set any site defaults.
   initializeDefaultsFromConfigFile(
      dynamic_cast<rspfNitfFileHeaderV2_X*>(theFileHeader.get()),
      dynamic_cast<rspfNitfImageHeaderV2_X*>(theImageHeader.get()) );
}

rspfNitf20Writer::~rspfNitf20Writer()
{
   //---
   // This looks like a leak but it's not as both of these are rspfRefPtr's.
   //---
   theFileHeader=0;
   theImageHeader=0;
   
   close();
}

bool rspfNitf20Writer::isOpen()const
{
   return (theOutputStream != 0);
}

bool rspfNitf20Writer::open()
{
   if(isOpen())
   {
      close();
   }
   theOutputStream = new std::ofstream;
   theOutputStream->open(theFilename.c_str(), ios::out|ios::binary);
   
   return theOutputStream->good();
}

void rspfNitf20Writer::close()
{
   if(theOutputStream)
   {
      theOutputStream->close();
      delete theOutputStream;
      theOutputStream = (std::ofstream*)0;
   }
}

bool rspfNitf20Writer::writeFile()
{
   if(!theInputConnection->isMaster())
   {
      theInputConnection->slaveProcessTiles();

      return true;
   }

   open();

   if (!isOpen())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitf20Writer::writeFile ERROR:"
            << " Could not open!  Returning..."
            << std::endl;
      }

      return false;
   }

   // Write out the geometry info.
   writeGeometry(theImageHeader.get(), theInputConnection.get());
   
   addTags();
   
   bool result = false;
   if((theOutputImageType == "nitf20_block_band_separate")||
      (theOutputImageType == "image/nitf20"))
   {
      result =  writeBlockBandSeparate();
   }
   else if(theOutputImageType == "nitf20_block_band_sequential")
   {
      result =  writeBlockBandSequential();
   }
   
   close();
   
   return result;
}

void rspfNitf20Writer::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   imageTypeList.push_back(rspfString("nitf20_block_band_separate"));
   imageTypeList.push_back(rspfString("nitf20_block_band_sequential"));
}

void rspfNitf20Writer::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;
   
   rspfString name = property->getName();
   
   if(name == "file_header")
   {
      rspfContainerProperty* containerProperty = PTR_CAST(rspfContainerProperty,
                                                           property.get());
      if(containerProperty)
      {
         std::vector<rspfRefPtr<rspfProperty> > propertyList;
         containerProperty->getPropertyList(propertyList);
         theFileHeader->setProperties(propertyList);
      }
   }
   else if(name == "image_header")
   {
      rspfContainerProperty* containerProperty = PTR_CAST(rspfContainerProperty,
                                                           property.get());
      if(containerProperty)
      {
         std::vector<rspfRefPtr<rspfProperty> > propertyList;
         containerProperty->getPropertyList(propertyList);
         theImageHeader->setProperties(propertyList);
      }
   }
   else if (name == "enable_rpcb_tag")
   {
      theEnableRpcbTagFlag = property->valueToString().toBool();
   }
   else if (name == "enable_blocka_tag")
   {
      theEnableBlockaTagFlag = property->valueToString().toBool();
   }
   else if(name == "copy_fields_flag")
   {
      theCopyFieldsFlag = property->valueToString().toBool();
   }
   else
   {
      // just in case it was an nitf specific tag we can pass it safely like this
      theFileHeader->setProperty(property);
      theImageHeader->setProperty(property);
      rspfImageFileWriter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfNitf20Writer::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;
   
   if(name == "file_header")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      std::vector<rspfRefPtr<rspfProperty> > propertyList;
      
      theFileHeader->getPropertyList(propertyList);
      container->addChildren(propertyList);
      
      result = container;
   }
   else if(name == "image_header")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      std::vector<rspfRefPtr<rspfProperty> > propertyList;
      
      theImageHeader->getPropertyList(propertyList);
      container->addChildren(propertyList);
      
      result = container;
   }
   else if(name == "enable_rpcb_tag")
   {
      rspfBooleanProperty* boolProperty =
         new rspfBooleanProperty(name, theEnableRpcbTagFlag);

      result = boolProperty;
   }   
   else if(name == "enable_blocka_tag")
   {
      rspfBooleanProperty* boolProperty =
         new rspfBooleanProperty(name, theEnableBlockaTagFlag);

      result = boolProperty;
   }   
   else if(name == "copy_fields_flag")
   {
      rspfBooleanProperty* boolProperty =
      new rspfBooleanProperty(name, theCopyFieldsFlag);
      
      result = boolProperty;
   }
   else
   {
      return rspfImageFileWriter::getProperty(name);
   }

   return result;
}

void rspfNitf20Writer::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageFileWriter::getPropertyNames(propertyNames);

   propertyNames.push_back("file_header");
   propertyNames.push_back("image_header");
   propertyNames.push_back("enable_rpcb_tag");
   propertyNames.push_back("enable_blocka_tag");
   propertyNames.push_back("copy_fields_flag");
}

bool rspfNitf20Writer::writeBlockBandSeparate()
{
   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   rspf_uint64 byteSize      = rspf::scalarSizeInBytes(scalarType);
   rspfIrect rect            = theInputConnection->getBoundingRect();
   rspf_uint64 bands         = theInputConnection->getNumberOfOutputBands();
   rspf_uint64 idx           = 0;
   rspf_uint64 headerStart   = (rspf_uint64)theOutputStream->tellp();

   rspfIpt blockSize(64, 64);
   rspf_uint64 blocksHorizontal = (rspf_uint32)ceil(((double)rect.width()/(double)blockSize.x));
   rspf_uint64 blocksVertical   = (rspf_uint32)ceil(((double)rect.height()/(double)blockSize.y));

   rspfNitfImageInfoRecordV2_0 imageInfoRecord;
   //imageInfoRecord.setSubheaderLength(439); // ok if no tags
   imageInfoRecord.setImageLength(bands*byteSize*blocksVertical*blockSize.y*blocksHorizontal*blockSize.x);

   theFileHeader->addImageInfoRecord(imageInfoRecord);

   //---
   // This makes space for the file header; it is written again at the end of
   // this method with updated values
   // need a better way to get the length.  This should be queried on the
   // header before writing
   //---
   theFileHeader->writeStream(*theOutputStream); 
   rspf_uint64 headerLength = ((rspf_uint64)theOutputStream->tellp() - headerStart) /* + 1 */;
   
   rspfString representation;
   theImageHeader->setActualBitsPerPixel(
      rspf::getActualBitsPerPixel(scalarType));
   theImageHeader->setBitsPerPixel(rspf::getBitsPerPixel(scalarType));
   theImageHeader->setPixelType(rspfNitfCommon::getNitfPixelType(scalarType));
   theImageHeader->setNumberOfBands(bands);
   theImageHeader->setImageMode('B');// blocked

   if((bands == 3)&&
      (scalarType == RSPF_UCHAR))
   {
      theImageHeader->setRepresentation("RGB");
      theImageHeader->setCategory("VIS");
   }
   else if(bands == 1)
   {
      theImageHeader->setRepresentation("MONO");
      theImageHeader->setCategory("MS");
   }
   else
   {
      theImageHeader->setRepresentation("MULTI");
      theImageHeader->setCategory("MS");
   }
   
   theImageHeader->setBlocksPerRow(blocksHorizontal);
   theImageHeader->setBlocksPerCol(blocksVertical);
   theImageHeader->setNumberOfPixelsPerBlockRow(blockSize.x);
   theImageHeader->setNumberOfPixelsPerBlockCol(blockSize.y);
   theImageHeader->setNumberOfRows(rect.height());
   theImageHeader->setNumberOfCols(rect.width());

   rspfNitfImageBandV2_0 bandInfo;
   for(idx = 0; idx < bands; ++idx)
   {
      std::ostringstream out;
      
      out << std::setfill('0')
          << std::setw(2)
          << idx;
      
      bandInfo.setBandRepresentation(out.str().c_str());
      theImageHeader->setBandInfo(idx,
                              bandInfo);
   }

   rspf_uint64 imageHeaderStart = theOutputStream->tellp();
   theImageHeader->writeStream(*theOutputStream);
   rspf_uint64 imageHeaderEnd = theOutputStream->tellp();
   rspf_uint64 imageHeaderSize = imageHeaderEnd - imageHeaderStart;

   theInputConnection->setTileSize(blockSize);
   rspf_uint64 numberOfTiles   = theInputConnection->getNumberOfTiles();
   theInputConnection->setToStartOfSequence();
   rspfRefPtr<rspfImageData> data = theInputConnection->getNextTile();
   rspf_uint64 tileNumber = 1;
   rspfEndian endian;

   
   while( data.valid() && !needsAborting())
   {
      if(endian.getSystemEndianType() == RSPF_LITTLE_ENDIAN)
      {
         switch(data->getScalarType())
         {
            case RSPF_USHORT16:
            case RSPF_USHORT11:
            {
               endian.swap((rspf_uint16*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            case RSPF_SSHORT16:
            {
               endian.swap((rspf_sint16*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            case RSPF_FLOAT:
            case RSPF_NORMALIZED_FLOAT:
            {
               endian.swap((rspf_float32*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            case RSPF_DOUBLE:
            case RSPF_NORMALIZED_DOUBLE:
            {
               endian.swap((rspf_float64*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            default:
               break;
         }
      }
      
      theOutputStream->write((char*)(data->getBuf()),
                             data->getSizeInBytes());
      
      setPercentComplete(((double)tileNumber / (double)numberOfTiles) * 100);
      
      if(!needsAborting())
      {
         data = theInputConnection->getNextTile();
      }
      ++tileNumber;
   }

   std::streamoff pos = theOutputStream->tellp();

   setComplexityLevel(pos, theFileHeader.get());

   /*
    * Need to change the way I compute file length and header length later
    * We need to figure out a better way to compute.
    */
   theFileHeader->setFileLength(static_cast<rspf_uint64>(pos));
   theFileHeader->setHeaderLength(headerLength);
   theOutputStream->seekp(0, ios::beg);
   imageInfoRecord.setSubheaderLength(imageHeaderSize);
   theFileHeader->replaceImageInfoRecord(0, imageInfoRecord);
   theFileHeader->writeStream(*theOutputStream);
   
   return true;
}

bool rspfNitf20Writer::writeBlockBandSequential()
{
   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   rspf_uint64    byteSize   = rspf::scalarSizeInBytes(scalarType);
   rspfIrect      rect       = theInputConnection->getBoundingRect();
   rspf_uint64    bands      = theInputConnection->getNumberOfOutputBands();
   rspf_uint64    idx        = 0;
   rspf_uint64    headerStart   = (rspf_uint64)theOutputStream->tellp();

   rspfIpt blockSize(64, 64);

   rspf_uint64 blocksHorizontal = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint64 blocksVertical   = theInputConnection->getNumberOfTilesVertical();
   theInputConnection->setTileSize(blockSize);
   rspf_uint64 numberOfTiles   = theInputConnection->getNumberOfTiles();
   theInputConnection->setToStartOfSequence();
   
   rspfNitfImageInfoRecordV2_0 imageInfoRecord;
 //  imageInfoRecord.setSubheaderLength(439);
   imageInfoRecord.setImageLength(bands*byteSize*blocksHorizontal*blocksVertical*blockSize.x*blockSize.y);

   rspfDate currentDate;
   
   theFileHeader->setDate(currentDate);
   theFileHeader->addImageInfoRecord(imageInfoRecord);

   //---
   // This makes space for the file header; it is written again at the end of
   // this method with updated values
   // need a better way to get the length.  This should be queried on the
   // header before writing
   //---  
   theFileHeader->writeStream(*theOutputStream);
   rspf_uint64 headerLength = ((rspf_uint64)theOutputStream->tellp() - headerStart) /* + 1 */;
   
   rspfString representation;
   theImageHeader->setActualBitsPerPixel(
      rspf::getActualBitsPerPixel(scalarType));
   theImageHeader->setBitsPerPixel(rspf::getBitsPerPixel(scalarType));
   theImageHeader->setPixelType(rspfNitfCommon::getNitfPixelType(scalarType));
   theImageHeader->setNumberOfBands(bands);
   theImageHeader->setImageMode('S');// blocked
   
   if((bands == 3)&&
      (scalarType == RSPF_UCHAR))
   {
      theImageHeader->setRepresentation("RGB");
      theImageHeader->setCategory("VIS");
   }
   else if(bands == 1)
   {
      theImageHeader->setRepresentation("MONO");
      theImageHeader->setCategory("MS");
   }
   else
   {
      theImageHeader->setRepresentation("MULTI");
      theImageHeader->setCategory("MS");
   }
   theImageHeader->setBlocksPerRow(blocksHorizontal);
   theImageHeader->setBlocksPerCol(blocksVertical);
   theImageHeader->setNumberOfPixelsPerBlockRow(blockSize.x);
   theImageHeader->setNumberOfPixelsPerBlockCol(blockSize.y);
   theImageHeader->setNumberOfRows(rect.height());
   theImageHeader->setNumberOfCols(rect.width());

   rspfNitfImageBandV2_0 bandInfo;
   for(idx = 0; idx < bands; ++idx)
   {
      std::ostringstream out;
      
      out << std::setfill('0')
          << std::setw(2)
          << idx;
      
      bandInfo.setBandRepresentation(out.str().c_str());
      theImageHeader->setBandInfo(idx,
                              bandInfo);
   }

   int imageHeaderStart = theOutputStream->tellp();
   theImageHeader->writeStream(*theOutputStream);
   int imageHeaderEnd = theOutputStream->tellp();
   int imageHeaderSize = imageHeaderEnd - imageHeaderStart;

   // rspfIpt ul = rect.ul();
   rspfRefPtr<rspfImageData> data = theInputConnection->getNextTile();
   rspf_uint64 tileNumber = 0;
   rspfEndian endian;

   // get the start to the first band of data block
   //
   rspf_uint64 streamOffset = theOutputStream->tellp();
   
   // holds the total pixels to the next band

   rspf_uint64 blockSizeInBytes = blockSize.x*blockSize.y*rspf::scalarSizeInBytes(data->getScalarType());
   rspf_uint64 bandOffsetInBytes = (blockSizeInBytes*blocksHorizontal*blocksVertical);

   bool needSwapping = endian.getSystemEndianType() == RSPF_LITTLE_ENDIAN;
   while(data.valid() && !needsAborting())
   {
      if(needSwapping)
      {
         switch(data->getScalarType())
         {
            case RSPF_USHORT16:
            case RSPF_USHORT11:
            {
               endian.swap((rspf_uint16*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            case RSPF_SSHORT16:
            {
               endian.swap((rspf_sint16*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            case RSPF_FLOAT:
            case RSPF_NORMALIZED_FLOAT:
            {
               endian.swap((rspf_float32*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            case RSPF_DOUBLE:
            case RSPF_NORMALIZED_DOUBLE:
            {
               endian.swap((rspf_float64*)data->getBuf(),
                           data->getWidth()*data->getHeight()*data->getNumberOfBands());
               break;
            }
            default:
               break;
         }
      }

      for(idx = 0; idx < bands; ++idx)
      {
         theOutputStream->seekp(streamOffset+ // start of image stream
                                tileNumber*blockSizeInBytes + // start of block for band separate output
                                bandOffsetInBytes*idx, // which band offset is it
                                ios::beg); 
         
         theOutputStream->write((char*)(data->getBuf(idx)),
                                blockSizeInBytes);
      }
      ++tileNumber;
      
      setPercentComplete(((double)tileNumber / (double)numberOfTiles) * 100);

      if(!needsAborting())
      {
         data = theInputConnection->getNextTile();
      }
   }
   
   std::streamoff pos = theOutputStream->tellp();

   setComplexityLevel(pos, theFileHeader.get());

   /*
    * Need to change the way I compute file length and header length later
    * We need to figure out a better way to compute.
    */
   theFileHeader->setFileLength(static_cast<rspf_uint64>(pos));
   theFileHeader->setHeaderLength(headerLength);
   theOutputStream->seekp(0, ios::beg);
   imageInfoRecord.setSubheaderLength(imageHeaderSize);
   theFileHeader->replaceImageInfoRecord(0, imageInfoRecord);
   theFileHeader->writeStream(*theOutputStream);
   
   return true;
}


void rspfNitf20Writer::addRegisteredTag(
   rspfRefPtr<rspfNitfRegisteredTag> registeredTag)
{
   rspfNitfTagInformation tagInfo;
   tagInfo.setTagData(registeredTag.get());
     theImageHeader->addTag(tagInfo);
}

void rspfNitf20Writer::addTags()
{
   rspfDate currentDate;
   theFileHeader->setDate(currentDate);
   
   if(theCopyFieldsFlag)
   {
      rspfTypeNameVisitor visitor(rspfString("rspfNitfTileSource"),
                                   true,
                                   (rspfVisitor::VISIT_CHILDREN|rspfVisitor::VISIT_INPUTS));
      accept(visitor);
      
      // If there are multiple image handlers, e.g. a mosaic do not uses.
      rspfRefPtr<rspfNitfTileSource> nitf = 0;
      if ( visitor.getObjects().size() == 1 )
      {
         nitf = visitor.getObjectAs<rspfNitfTileSource>( 0 );
      }

      if( nitf.valid() )
      {
         rspfString value;
         rspfPropertyInterface* fileHeaderProperties = dynamic_cast<rspfPropertyInterface*>(theFileHeader.get());
         rspfPropertyInterface* imageHeaderProperties = dynamic_cast<rspfPropertyInterface*>(theImageHeader.get());
         bool nitf21Flag = false;
         const rspfNitfFileHeader*   header      = nitf->getFileHeader();
         const rspfNitfImageHeader*  imageHeader = nitf->getCurrentImageHeader();
         if(header)
         {
            rspfString version = header->getPropertyValueAsString("fhdr");
            nitf21Flag = version.contains("2.1");
            value = header->getPropertyValueAsString("stype");
            if(value.size())
            {
               fileHeaderProperties->setProperty("stype", value);
            }
            value = header->getPropertyValueAsString("ostaid");
            if(value.size())
            {
               fileHeaderProperties->setProperty("ostaid", value);
            }
            value = header->getPropertyValueAsString("ftitle");
            if(value.size())
            {
               fileHeaderProperties->setProperty("ftitle", value);
            }
            value = header->getPropertyValueAsString("fsclas");
            if(value.size())
            {
               fileHeaderProperties->setProperty("fsclas", value);
            }
            value = header->getPropertyValueAsString("oname");
            if(value.size())
            {
               fileHeaderProperties->setProperty("oname", value);
            }
            value = header->getPropertyValueAsString("ophone");
            if(value.size())
            {
               fileHeaderProperties->setProperty("ophone", value);
            }
            int idx = 0;
            for(idx = 0; idx < header->getNumberOfTags(); ++idx)
            {
               rspfNitfTagInformation info;
               header->getTagInformation(info,
                                         idx);
               theFileHeader->addTag(info);
            }
            value = header->getPropertyValueAsString("");
            if(value.size())
            {
               fileHeaderProperties->setProperty("", value);
            }
            value = header->getPropertyValueAsString("fdt");
            if(value.size()==14)
            {
               if(nitf21Flag)
               {
                  rspfString year(value.begin()+2, value.begin()+4);
                  rspfString mon(value.begin()+4, value.begin()+6);
                  rspfString day(value.begin()+6, value.begin()+8);
                  rspfString hour(value.begin()+8, value.begin()+10);
                  rspfString min(value.begin()+10, value.begin()+12);
                  rspfString sec(value.begin()+12, value.begin()+14);
                  if(mon.toUInt32() < 13)
                  {
                     mon = monthConversionTable[mon.toUInt32()];
                  }
                  fileHeaderProperties->setProperty("fdt", day+hour+min+sec+"Z"+mon+year);
               }
               else
               {
                  fileHeaderProperties->setProperty("fdt", value);
               }
            }
         }
         if(imageHeader)
         {
            rspf_uint32 idx = 0;
            for(idx = 0; idx < imageHeader->getNumberOfTags(); ++idx)
            {
               rspfNitfTagInformation info;
               imageHeader->getTagInformation(info,
                                              idx);
               theImageHeader->addTag(info);
            }
            value = imageHeader->getPropertyValueAsString("tgtid");
            if(value.size())
            {
               imageHeaderProperties->setProperty("tgtid", value);
            }
            value = imageHeader->getPropertyValueAsString("isclas");
            if(value.size())
            {
               imageHeaderProperties->setProperty("isclas", value);
            }
            value = imageHeader->getPropertyValueAsString("igeolo");
            if(value.size())
            {
               imageHeaderProperties->setProperty("igeolo", value);
            }
            value = imageHeader->getPropertyValueAsString("ititle");
            if(nitf21Flag)
            {
               value = imageHeader->getPropertyValueAsString("iid2");
            }
            if(value.size())
            {
               imageHeaderProperties->setProperty("ititle", value);
            }
            if(!nitf21Flag)
            {
               value = imageHeader->getPropertyValueAsString("iscaut");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("iscaut", value);
               }
               value = imageHeader->getPropertyValueAsString("iscode");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("iscode", value);
               }
               value = imageHeader->getPropertyValueAsString("isctlh");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("isctlh", value);
               }
               value = imageHeader->getPropertyValueAsString("isrel");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("isrel", value);
               }
               value = imageHeader->getPropertyValueAsString("isctln");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("isctln", value);
               }
               value = imageHeader->getPropertyValueAsString("isdwng");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("isdevt", value);
               }
               value = imageHeader->getPropertyValueAsString("isorce");
               if(value.size())
               {
                  imageHeaderProperties->setProperty("isorce", value);
               }
            }
            value = imageHeader->getPropertyValueAsString("idatim");
            if(value.size()==14)
            {
               if(nitf21Flag)
               {
                  rspfString year(value.begin()+2, value.begin()+4);
                  rspfString mon(value.begin()+4, value.begin()+6);
                  rspfString day(value.begin()+6, value.begin()+8);
                  rspfString hour(value.begin()+8, value.begin()+10);
                  rspfString min(value.begin()+10, value.begin()+12);
                  rspfString sec(value.begin()+12, value.begin()+14);
                  if(mon.toUInt32() < 13)
                  {
                     mon = monthConversionTable[mon.toUInt32()];
                  }
                  imageHeaderProperties->setProperty("idatim", day+hour+min+sec+"Z"+mon+year);
               }
               else
               {
                  imageHeaderProperties->setProperty("idatim", value);
               }
            }
         }
         
         // we will port over only a selected few 
      }
   }
}

bool rspfNitf20Writer::saveState(rspfKeywordlist& kwl,
                                  const char* prefix) const
{
   return rspfNitfWriterBase::saveState(kwl, prefix);
}

bool rspfNitf20Writer::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   return rspfNitfWriterBase::loadState(kwl, prefix);
}
