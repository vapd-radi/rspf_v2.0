//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNitfWriter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfNitfWriter.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfRpcSolver.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfRectangleCutFilter.h>
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

RTTI_DEF1(rspfNitfWriter, "rspfNitfWriter", rspfNitfWriterBase);

static rspfTrace traceDebug(rspfString("rspfNitfWriter:debug"));
                             
rspfNitfWriter::rspfNitfWriter(const rspfFilename& filename,
                                 rspfImageSource* inputSource)
   : rspfNitfWriterBase(filename, inputSource),
     m_outputStream(0),
     m_fileHeader(0),
     m_imageHeader(0),
     m_textHeader(0),
     m_textEntry(),
     m_blockSize(RSPF_DEFAULT_TILE_WIDTH, RSPF_DEFAULT_TILE_HEIGHT)
{
   //---
   // Since the internal nitf tags are not very accurate, write an external
   // geometry out as default behavior.  Users can disable this via the
   // property interface or keyword list.
   //---
   setWriteExternalGeometryFlag(true);
   
   m_fileHeader       = new rspfNitfFileHeaderV2_1;
   m_imageHeader      = new rspfNitfImageHeaderV2_1;

   // m_textHeader is conditional so we will not new here.
   
   theOutputImageType = "nitf_block_band_separate";

   //---
   // The tile size can be set in the preferences via "tile_size" keyword.
   // This will get it if set; else, set to default.
   //---
   rspf::defaultTileSize(m_blockSize);

   // These are always set:
   m_fileHeader->setEncryption( rspfString("0") );
   m_imageHeader->setJustification( rspfString("R") );

   // Set any site defaults.
   initializeDefaultsFromConfigFile(
      dynamic_cast<rspfNitfFileHeaderV2_X*>(m_fileHeader.get()),
      dynamic_cast<rspfNitfImageHeaderV2_X*>(m_imageHeader.get()) );
}

rspfNitfWriter::~rspfNitfWriter()
{
   //---
   // This looks like a leak but it's not as both of these are rspfRefPtr's.
   //---
   m_fileHeader=0;
   m_imageHeader=0;
   m_textHeader=0;

   close();
}

bool rspfNitfWriter::isOpen()const
{
   return (m_outputStream != 0);
}

bool rspfNitfWriter::open()
{
   if(isOpen())
   {
      close();
   }
   m_outputStream = new std::ofstream;
   m_outputStream->open(theFilename.c_str(), ios::out|ios::binary);
   
   return m_outputStream->good();
}

void rspfNitfWriter::close()
{
   if(m_outputStream)
   {
      m_outputStream->close();
      delete m_outputStream;
      m_outputStream = (std::ofstream*)0;
   }
}

bool rspfNitfWriter::writeFile()
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
            << "rspfNitfWriter::writeFile ERROR:"
            << " Could not open!  Returning..."
            << std::endl;
      }

      return false;
   }
   
   // Write out the geometry info.
   writeGeometry(m_imageHeader.get(), theInputConnection.get());

   // addStandardTags();
   
   bool result = false;
   if((theOutputImageType == "nitf_block_band_separate")||
      (theOutputImageType == "image/nitf"))
   {
      result =  writeBlockBandSeparate();
   }
   else if(theOutputImageType == "nitf_block_band_sequential")
   {
      result =  writeBlockBandSequential();
   }
   
   close();
   
   return result;
}

void rspfNitfWriter::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   imageTypeList.push_back(rspfString("nitf_block_band_separate"));
   imageTypeList.push_back(rspfString("nitf_block_band_sequential"));
}

void rspfNitfWriter::setProperty(rspfRefPtr<rspfProperty> property)
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
         m_fileHeader->setProperties(propertyList);
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
         m_imageHeader->setProperties(propertyList);
      }
   }
   else if(name == "block_size")
   {
      rspfIpt blockSize;
      blockSize.x = property->valueToString().toInt32();
      blockSize.y = blockSize.x;
      setTileSize(blockSize);
   }
   else
   {
      rspfNitfWriterBase::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfNitfWriter::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;
   
   if(name == "file_header")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      std::vector<rspfRefPtr<rspfProperty> > propertyList;
      
      m_fileHeader->getPropertyList(propertyList);
      container->addChildren(propertyList);
      
      result = container;
   }
   else if(name == "image_header")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      std::vector<rspfRefPtr<rspfProperty> > propertyList;
      
      m_imageHeader->getPropertyList(propertyList);
      container->addChildren(propertyList);
      
      result = container;
   }
   else if(name == "block_size")
   {
      rspfStringProperty* stringProp =
         new rspfStringProperty(name,
                                 rspfString::toString(m_blockSize.x),
                                 false); // editable flag
      stringProp->addConstraint(rspfString("64"));
      stringProp->addConstraint(rspfString("128"));
      stringProp->addConstraint(rspfString("256"));      
      stringProp->addConstraint(rspfString("512"));      
      stringProp->addConstraint(rspfString("1024"));      
      stringProp->addConstraint(rspfString("2048"));      
      return stringProp;
   }
   else
   {
      return rspfNitfWriterBase::getProperty(name);
   }

   return result;
}

void rspfNitfWriter::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   rspfNitfWriterBase::getPropertyNames(propertyNames);

   propertyNames.push_back("file_header");
   propertyNames.push_back("image_header");
   propertyNames.push_back("block_size");
}

bool rspfNitfWriter::writeBlockBandSeparate()
{
   rspfScalarType scalarType  = theInputConnection->getOutputScalarType();
   rspf_uint64    byteSize    = rspf::scalarSizeInBytes(scalarType);
   rspfIrect      rect        = theInputConnection->getBoundingRect();
   rspf_uint64    bands       = theInputConnection->getNumberOfOutputBands();
   rspf_uint64    idx         = 0;
   rspf_uint64    headerStart = (rspf_uint64)m_outputStream->tellp();

   // Set the sequencer block size to be the same as output.
   theInputConnection->setTileSize(m_blockSize);
   
   rspf_uint64 blocksHorizontal = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint64 blocksVertical   = theInputConnection->getNumberOfTilesVertical();
   rspf_uint64 numberOfTiles    = theInputConnection->getNumberOfTiles();

   rspfNitfImageInfoRecordV2_1 imageInfoRecord;
   imageInfoRecord.setSubheaderLength(439); // ok if no tags
   imageInfoRecord.setImageLength(bands*byteSize*blocksVertical*m_blockSize.y*blocksHorizontal*m_blockSize.x);

   m_fileHeader->setDate();
   m_fileHeader->addImageInfoRecord(imageInfoRecord);

   if ( m_textHeader.valid() )
   {
      // Add any text headers
      rspfNitfTextFileInfoRecordV2_1 textInfoRecord;
      textInfoRecord.setSubheaderLength(285); //default
      // Set length of text to be that of input text
      textInfoRecord.setTextLength(m_textEntry.length()); 
      m_fileHeader->addTextInfoRecord(textInfoRecord);
   }  

   //---
   // This makes space for the file header; it is written again at the end of
   // this method with updated values
   // need a better way to get the length.  This should be queried on the
   // header before writing
   //---
   m_fileHeader->writeStream(*m_outputStream); 
   rspf_uint64 headerLength = ((rspf_uint64)m_outputStream->tellp() - headerStart) /* + 1 */;
   
   rspfString representation;
   m_imageHeader->setActualBitsPerPixel(rspf::getActualBitsPerPixel(scalarType));
   m_imageHeader->setBitsPerPixel(rspf::getBitsPerPixel(scalarType));
   m_imageHeader->setPixelType(rspfNitfCommon::getNitfPixelType(scalarType));
   m_imageHeader->setNumberOfBands(bands);
   m_imageHeader->setImageMode('B');// blocked

   if((bands == 3)&&
      (scalarType == RSPF_UCHAR))
   {
      m_imageHeader->setRepresentation("RGB");
      m_imageHeader->setCategory("VIS");
   }
   else if(bands == 1)
   {
      m_imageHeader->setRepresentation("MONO");
      m_imageHeader->setCategory("MS");
   }
   else
   {
      m_imageHeader->setRepresentation("MULTI");
      m_imageHeader->setCategory("MS");
   }
   
   m_imageHeader->setBlocksPerRow(blocksHorizontal);
   m_imageHeader->setBlocksPerCol(blocksVertical);
   m_imageHeader->setNumberOfPixelsPerBlockRow(m_blockSize.x);
   m_imageHeader->setNumberOfPixelsPerBlockCol(m_blockSize.y);
   m_imageHeader->setNumberOfRows(rect.height());
   m_imageHeader->setNumberOfCols(rect.width());

   rspfNitfImageBandV2_1 bandInfo;
   for(idx = 0; idx < bands; ++idx)
   {
      std::ostringstream out;
      
      out << std::setfill('0')
          << std::setw(2)
          << idx;
      
      bandInfo.setBandRepresentation(out.str().c_str());
      m_imageHeader->setBandInfo(idx, bandInfo);
   }

   rspf_uint64 imageHeaderStart = m_outputStream->tellp();
   m_imageHeader->writeStream(*m_outputStream);
   rspf_uint64 imageHeaderEnd = m_outputStream->tellp();
   rspf_uint64 imageHeaderSize = imageHeaderEnd - imageHeaderStart;

   // Start the sequence through tiles:
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
      
      m_outputStream->write((char*)(data->getBuf()), data->getSizeInBytes());
      
      setPercentComplete(((double)tileNumber / (double)numberOfTiles) * 100);
      
      if(!needsAborting())
      {
         data = theInputConnection->getNextTile();
      }
      ++tileNumber;
   }

   // Let's write our text header
   if ( m_textHeader.valid() )
   {
      m_textHeader->writeStream(*m_outputStream); 
      //Now write the text
      m_outputStream->write((char*)(m_textEntry.c_str()), m_textEntry.length());
   }   

   std::streamoff pos = m_outputStream->tellp();

   setComplexityLevel(pos, m_fileHeader.get());

   /*
    * Need to change the way I compute file length and header length later
    * We need to figure out a better way to compute.
    */
   m_fileHeader->setFileLength(static_cast<rspf_uint64>(pos));
   m_fileHeader->setHeaderLength(headerLength);
   m_outputStream->seekp(0, ios::beg);
   imageInfoRecord.setSubheaderLength(imageHeaderSize);
   m_fileHeader->replaceImageInfoRecord(0, imageInfoRecord);
   m_fileHeader->writeStream(*m_outputStream);
   
   return true;
}

bool rspfNitfWriter::writeBlockBandSequential()
{
   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   rspf_uint64    byteSize   = rspf::scalarSizeInBytes(scalarType);
   rspfIrect      rect       = theInputConnection->getBoundingRect();
   rspf_uint64    bands      = theInputConnection->getNumberOfOutputBands();
   rspf_uint64    idx        = 0;
   rspf_uint64    headerStart   = (rspf_uint64)m_outputStream->tellp();

   // Set the sequencer block size to be the same as output.
   theInputConnection->setTileSize(m_blockSize);
   
   rspf_uint64 blocksHorizontal = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint64 blocksVertical   = theInputConnection->getNumberOfTilesVertical();
   rspf_uint64 numberOfTiles    = theInputConnection->getNumberOfTiles();

   rspfNitfImageInfoRecordV2_1 imageInfoRecord;
   imageInfoRecord.setSubheaderLength(439);
   imageInfoRecord.setImageLength(bands*byteSize*blocksHorizontal*blocksVertical*m_blockSize.x*m_blockSize.y);

   m_fileHeader->setDate();
   m_fileHeader->addImageInfoRecord(imageInfoRecord);

   if ( m_textHeader.valid() )
   {
      // Add any text headers
      rspfNitfTextFileInfoRecordV2_1 textInfoRecord;
      textInfoRecord.setSubheaderLength(285); //default
      // Set length of text to be that of input text
      textInfoRecord.setTextLength(m_textEntry.length()); 
      m_fileHeader->addTextInfoRecord(textInfoRecord);
   }

   //---
   // This makes space for the file header; it is written again at the end of
   // this method with updated values
   // need a better way to get the length.  This should be queried on the
   // header before writing
   //---  
   m_fileHeader->writeStream(*m_outputStream);
   rspf_uint64 headerLength = ((rspf_uint64)m_outputStream->tellp() - headerStart) /* + 1 */;
   
   rspfString representation;
   m_imageHeader->setActualBitsPerPixel(rspf::getActualBitsPerPixel(scalarType));
   m_imageHeader->setBitsPerPixel(rspf::getBitsPerPixel(scalarType));
   m_imageHeader->setPixelType(rspfNitfCommon::getNitfPixelType(scalarType));
   m_imageHeader->setNumberOfBands(bands);
   m_imageHeader->setImageMode('S');// blocked
   
   if((bands == 3)&&
      (scalarType == RSPF_UCHAR))
   {
      m_imageHeader->setRepresentation("RGB");
      m_imageHeader->setCategory("VIS");
   }
   else if(bands == 1)
   {
      m_imageHeader->setRepresentation("MONO");
      m_imageHeader->setCategory("MS");
   }
   else
   {
      m_imageHeader->setRepresentation("MULTI");
      m_imageHeader->setCategory("MS");
   }
   m_imageHeader->setBlocksPerRow(blocksHorizontal);
   m_imageHeader->setBlocksPerCol(blocksVertical);
   m_imageHeader->setNumberOfPixelsPerBlockRow(m_blockSize.x);
   m_imageHeader->setNumberOfPixelsPerBlockCol(m_blockSize.y);
   m_imageHeader->setNumberOfRows(rect.height());
   m_imageHeader->setNumberOfCols(rect.width());

   rspfNitfImageBandV2_1 bandInfo;
   for(idx = 0; idx < bands; ++idx)
   {
      std::ostringstream out;
      
      out << std::setfill('0')
          << std::setw(2)
          << idx;
      
      bandInfo.setBandRepresentation(out.str().c_str());
      m_imageHeader->setBandInfo(idx, bandInfo);
   }

   int imageHeaderStart = m_outputStream->tellp();
   m_imageHeader->writeStream(*m_outputStream);
   int imageHeaderEnd = m_outputStream->tellp();
   int imageHeaderSize = imageHeaderEnd - imageHeaderStart;

   // rspfIpt ul = rect.ul();

   // Start the sequence through tiles:
   theInputConnection->setToStartOfSequence();

   rspfRefPtr<rspfImageData> data = theInputConnection->getNextTile();
   rspf_uint64 tileNumber = 0;
   rspfEndian endian;

   // get the start to the first band of data block
   //
   rspf_uint64 streamOffset = m_outputStream->tellp();
   
   // holds the total pixels to the next band

   rspf_uint64 blockSizeInBytes = m_blockSize.x*m_blockSize.y*rspf::scalarSizeInBytes(data->getScalarType());
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
         m_outputStream->seekp(streamOffset+ // start of image stream
                               tileNumber*blockSizeInBytes + // start of block for band separate output
                               bandOffsetInBytes*idx, // which band offset is it
                               ios::beg); 
         
         m_outputStream->write((char*)(data->getBuf(idx)),
                               blockSizeInBytes);
      }
      ++tileNumber;
      
      setPercentComplete(((double)tileNumber / (double)numberOfTiles) * 100);
      
      if(!needsAborting())
      {
         data = theInputConnection->getNextTile();
      }
   }

   // Let's write our text header
   if ( m_textHeader.valid() )
   {
      m_textHeader->writeStream(*m_outputStream); 
      //Now write the text
      m_outputStream->write((char*)(m_textEntry.c_str()), m_textEntry.length());
   }   

   std::streamoff pos = m_outputStream->tellp();

   setComplexityLevel(pos, m_fileHeader.get());

   /*
    * Need to change the way I compute file length and header length later
    * We need to figure out a better way to compute.
    */
   m_fileHeader->setFileLength(static_cast<rspf_uint64>(pos));
   m_fileHeader->setHeaderLength(headerLength);
   m_outputStream->seekp(0, ios::beg);
   imageInfoRecord.setSubheaderLength(imageHeaderSize);
   m_fileHeader->replaceImageInfoRecord(0, imageInfoRecord);
   m_fileHeader->writeStream(*m_outputStream);
   
   return true;
}


void rspfNitfWriter::addRegisteredTag(
   rspfRefPtr<rspfNitfRegisteredTag> registeredTag)
{
   rspfNitfTagInformation tagInfo;
   tagInfo.setTagData(registeredTag.get());
   m_imageHeader->addTag(tagInfo);
}

bool rspfNitfWriter::addTextToNitf(std::string &inputText)
{
   // Initialize the m_textHeader
   m_textEntry = inputText;
   if ( m_textHeader.valid() == false )
   {
      // Only created if we need it.
      m_textHeader = new rspfNitfTextHeaderV2_1;
   }
   return true;
}
void rspfNitfWriter::getTileSize(rspfIpt& size) const
{
   size = m_blockSize;
}

void rspfNitfWriter::setTileSize(const rspfIpt& tileSize)
{
   const rspf_int32 MIN_BLOCK_SIZE = 64;
   if ( (tileSize.x % MIN_BLOCK_SIZE) || (tileSize.y % MIN_BLOCK_SIZE) )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfWriter::setTileSize ERROR:"
            << "\nBlock size must be a multiple of " << MIN_BLOCK_SIZE
            << "\nSize remains:  " << m_blockSize
            << std::endl;
      }
   }
   else
   {
      m_blockSize = tileSize;
   }
}

#if 0
void rspfNitfWriter::addStandardTags()
{

   if(!theInputConnection)
   {
      return;
   }
   
   // commenting this out for now.  For some reason the pixels
   // are off when I add this tag.  I checked the parsing and it
   // appears to be the correct length???  So, I am not sure
   // why we are off when reading the output back in.
   //
   // first lets do the projection tag.  I REALLY need to add
   // parameter support soon or this tag is useless.
   //
   rspfKeywordlist kwl;
   theInputConnection->getImageGeometry(kwl);
   rspfRefPtr<rspfProjection> proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
   rspfNitfNameConversionTables table;
   rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, proj.get());
   rspfNitfTagInformation tagInfo;
   
   if(mapProj)
   {

      if(!PTR_CAST(rspfUtmProjection, mapProj))
      {
         rspfRefPtr<rspfNitfProjectionParameterTag> parameterTag = new rspfNitfProjectionParameterTag;
         
         rspfString nitfCode = table.convertMapProjectionNameToNitfCode(proj->getClassName());
         rspfString nitfName = table.convertNitfCodeToNitfProjectionName(nitfCode);
         
         parameterTag->setName(nitfName);
         parameterTag->setCode(nitfCode);
         parameterTag->setFalseX(mapProj->getFalseEasting());
         parameterTag->setFalseY(mapProj->getFalseNorthing());
         
         tagInfo.setTagData(parameterTag.get());
         m_imageHeader->addTag(tagInfo);
      }
   }
}
#endif

bool rspfNitfWriter::saveState(rspfKeywordlist& kwl,
                                const char* prefix) const
{
   return rspfNitfWriterBase::saveState(kwl, prefix);
}

bool rspfNitfWriter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   return rspfNitfWriterBase::loadState(kwl, prefix);
}
