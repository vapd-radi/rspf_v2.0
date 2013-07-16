//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfToc.cpp 21214 2012-07-03 16:20:11Z dburken $

#include <rspf/support_data/rspfRpfToc.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_X.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_0.h>
#include <rspf/support_data/rspfRpfFrame.h>
#include <rspf/support_data/rspfRpfFrameFileIndexSectionSubheader.h>
#include <rspf/support_data/rspfRpfFrameFileIndexSubsection.h>
#include <rspf/support_data/rspfRpfBoundaryRectTable.h>
#include <rspf/support_data/rspfRpfLocationSection.h>
#include <rspf/support_data/rspfRpfTocEntry.h>
#include <rspf/support_data/rspfRpfFrameFileIndexRecord.h>
#include <rspf/support_data/rspfRpfBoundaryRectSectionSubheader.h>
#include <rspf/support_data/rspfRpfPathnameRecord.h>
#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/base/rspfTrace.h>
#include <iostream>
#include <fstream>

static rspfTrace traceDebug("rspfRpfToc:debug");

std::ostream& operator <<(std::ostream& out, const rspfRpfToc& data)
{
   return data.print(out);
}

rspfRpfToc::rspfRpfToc()
   :m_tocEntryList(),
    m_filename(),
    m_nitfFileHeader(0),
    m_rpfHeader(0)
{
}

rspfRpfToc::~rspfRpfToc()
{
   deleteAll();
}

rspfErrorCode rspfRpfToc::parseFile(const rspfFilename &fileName, bool keepFileHeader)
{
   static const char MODULE[] = "rspfRpfToc::parseFile";
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered....." << std::endl;
   }

   rspfRefPtr<rspfNitfFile> nitfFile = new rspfNitfFile;

   clearAll();

   nitfFile->parseFile(fileName);

   m_nitfFileHeader = nitfFile->getHeader();

   m_rpfHeader = 0; // rspfRefPtr
   
   if( !m_nitfFileHeader.valid() )
   {
      nitfFile = 0;
      
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " leaving with no nitf header found....." << std::endl;
      }
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   rspfNitfTagInformation info; 
   m_nitfFileHeader->getTag(info, "RPFHDR");

   if(traceDebug() && (info.getTagName() == "RPFHDR") )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " nitf file header print\n";
      m_nitfFileHeader->print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
   
   if ( !keepFileHeader )
   {
      // we no longer need access to the nitf header.  We got what we needed.
      m_nitfFileHeader = 0;
   }
   nitfFile = 0;
   
   m_filename = fileName;

   if(info.getTagName() == "RPFHDR")
   {
      m_rpfHeader = new rspfRpfHeader;

      std::ifstream in(m_filename.c_str(), std::ios::in|std::ios::binary);
      
      // set the get pointer for the stream to the start
      // of the Rpf header data
      in.seekg(info.getTagDataOffset(), std::ios::beg);
      
      // now get the header data.  We do not need to pass in the byte order.
      // this is grabbed from the first byte of the stream.  To see this,
      // Look at the RpfHeader implementation.
      m_rpfHeader->parseStream(in);

      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG: Found RPF HEADER tag\n";
         m_rpfHeader->print( rspfNotify(rspfNotifyLevel_DEBUG) );
      }

   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfRpfToc::parseFile: Leaving No RPFHDR tag found" << "\n";
      }
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   if( m_rpfHeader.valid() )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG: Building toc list" << "\n";
      }
      buildTocEntryList( m_rpfHeader.get() );
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfRpfToc::parseFile: Leaving no RPFHDR " << __LINE__ << "\n";
      }
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfRpfToc::parseFile: Returning with OK." << std::endl;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfToc::createTocAndCopyFrames( const rspfFilename& dotRpfFile,
                                          const rspfFilename& outputDir )
{
   static const char MODULE[] = "rspfRpfToc::createTocAndCopyFrames";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\ndot rpf file:      " << dotRpfFile
         << "\noutput directory:  " << outputDir
         << "\n";
   }

   if ( outputDir.expand().exists() == false )
   {
      if ( !outputDir.createDirectory(true, 0775) )
      {
         std::string e = MODULE;
         e += " ERROR:\nCould not create directory: ";
         e+= outputDir.c_str();
         throw rspfException(e);
      }
   }

   // Open the dot rpf file.
   std::ifstream* dotRpfStr = new std::ifstream;
   dotRpfStr->open(dotRpfFile.c_str(), ios_base::in);
   if ( !dotRpfStr->good() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;

      std::string e = MODULE;
      e += " ERROR:\nCould not open: ";
      e += dotRpfFile.c_str();
      throw rspfException(e);
   }

   rspfFilename sourceADotTocFile = getSourceTocFile(*dotRpfStr);
   if ( sourceADotTocFile.empty() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;

      std::string e = MODULE;
      e += " ERROR:\nCould not deduce source a.toc file!";
      throw rspfException(e);
   }
   
   // Open the source a.toc file. Note the true flag is to keep the file header.
   rspfRefPtr<rspfRpfToc> sourceADotToc = new rspfRpfToc;
   if ( sourceADotToc->parseFile(sourceADotTocFile, true) != rspfErrorCodes::RSPF_OK )
   {
      delete dotRpfStr;
      dotRpfStr = 0;
 
      std::string e = MODULE;
      e += " ERROR:\nCould not open: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e);
   }

   rspfRefPtr<const rspfNitfFileHeader> sourceNitfFileHdr = sourceADotToc->getNitfFileHeader();
   if ( !sourceNitfFileHdr.valid() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;        

      std::string e = MODULE;
      e += " ERROR:\nCould not get nitf file header from: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e);
   }
   
   rspfRefPtr<const rspfRpfHeader> sourceRpfHdr = sourceADotToc->getRpfHeader();
   if ( !sourceRpfHdr.valid() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;
      
      std::string e = MODULE;
      e += " ERROR:\nCould not get rpf header from: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e);
   }

   // Get the boundary rect sub header from the source a.toc.
   rspfRefPtr<rspfRpfBoundaryRectSectionSubheader> boundaryRectSectionSubheader =
      sourceRpfHdr->getNewBoundaryRectSectSubheader(sourceADotTocFile);
   if ( !boundaryRectSectionSubheader.valid() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;

      std::string e = MODULE;
      e += " ERROR:\nCould not pull boundary rect sub header from source file: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e);
   }   

   // Get the boundary rect table from the source a.toc.
   rspfRefPtr<rspfRpfBoundaryRectTable> boundaryRectTable =
      sourceRpfHdr->getNewBoundaryRectTable(sourceADotTocFile);
   if ( !boundaryRectTable.valid() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;
      
      std::string e = MODULE;
      e += " ERROR:\nCould not pull boundary rect table from source file: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e);
   }
   
   // Get the frame file subheader from the source a.toc.
   rspfRefPtr<rspfRpfFrameFileIndexSectionSubheader> frameFileSubHeader =
      sourceRpfHdr->getNewFrameFileIndexSectionSubheader(sourceADotTocFile);
   if ( !frameFileSubHeader.valid() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;

      std::string e = MODULE;
      e += " ERROR:\nCould not pull frame file sub header from source file: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e);
   }

   // Get the frame file subsection from the source a.toc.
   rspfRefPtr<rspfRpfFrameFileIndexSubsection> frameFileSubSection =
      sourceRpfHdr->getNewFileIndexSubsection(sourceADotTocFile);
   if ( !frameFileSubSection.valid() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;

      std::string e = MODULE;
      e += " ERROR:\nCould not pull frame file sub section from source file: ";
      e += sourceADotTocFile.c_str();
      throw rspfException(e); 
   }
   
   // Open the output file to write to.
   const rspfFilename A_DOT_TOC_FILE = "a.toc";
   rspfFilename dotTocFile = outputDir.dirCat(A_DOT_TOC_FILE);
   std::ofstream* dotTocStr = new std::ofstream;
   dotTocStr->open( dotTocFile.c_str(), ios::out|ios::binary );
   if ( !dotTocStr->good() )
   {
      delete dotRpfStr;
      dotRpfStr = 0;
      delete dotTocStr;
      dotTocStr =0;

      std::string e = MODULE;
      e += " ERROR:\nCould not open: ";
      e += dotTocFile.c_str();
      throw rspfException(e);
   }
   
   // Variables used throughout:
   rspfRefPtr<rspfProperty> prop = new rspfStringProperty();
   rspfString field;
   rspfString s;
   // std::streampos fileHeaderLength = 0;
   // std::streampos fileLength = 0;
      
   rspfRefPtr<rspfNitfFileHeaderV2_0> fileHdr = new rspfNitfFileHeaderV2_0();
   
   // Set the CLEVEL:
   s = "01";
   fileHdr->setComplexityLevel(s);
   
   // Set the OSTAID:
   prop = sourceNitfFileHdr->getProperty(rspfNitfFileHeaderV2_X::OSTAID_KW);
   fileHdr->setProperty(prop);
   
   // Set the FDT (date):
   fileHdr->setDate();
   
   // Set the FTITLE:
   s = "a.toc";
   fileHdr->setTitle(s);
   
   // Set the FSCLAS:
   prop = sourceNitfFileHdr->getProperty(rspfNitfFileHeaderV2_X::FSCLAS_KW);
   fileHdr->setProperty(prop);
   
   // Set the FSCODE:
   prop = sourceNitfFileHdr->getProperty(rspfNitfFileHeaderV2_X::FSCODE_KW);
   fileHdr->setProperty(prop);
   
   // Set the FSCTLH:
   prop = sourceNitfFileHdr->getProperty(rspfNitfFileHeaderV2_X::FSCTLH_KW);
   fileHdr->setProperty(prop);
   
   // Set the ONAME:
   prop = sourceNitfFileHdr->getProperty(rspfNitfFileHeaderV2_X::ONAME_KW);
   fileHdr->setProperty(prop);
   
   // Set the OPHONE:
   prop = sourceNitfFileHdr->getProperty(rspfNitfFileHeaderV2_X::OPHONE_KW);
   fileHdr->setProperty(prop);
   
   // Add the rpf header.
   rspfRpfHeader* rpfHdr = new rspfRpfHeader( *(sourceRpfHdr.get()) );
   
   rspfRefPtr<rspfNitfRegisteredTag> rpfHdrRp = rpfHdr;
   rspfNitfTagInformation rpfHdrInfo(rpfHdrRp);
   fileHdr->addTag(rpfHdrInfo);
   
   //---
   // Write it out...
   // The first write will be with an rpfheader with no location sections just
   // to see where the end of the file header is.
   //---
   fileHdr->writeStream(*dotTocStr);
   
   //---
   // End of file header. Get the header length. This will also be the
   // start of the location section.
   //---
   std::streampos pos = dotTocStr->tellp();
   std::streamoff locationSectionOffset = pos;
   
   // Set the header length:
   fileHdr->setHeaderLength( static_cast<rspf_uint64>(locationSectionOffset) );
   
   // Set the location of the location section.
   rpfHdr->setLocationSectionPos(locationSectionOffset);

   // Set the file name.
   rpfHdr->setFilename(A_DOT_TOC_FILE);
   
   // Add the component location records to the header.
   rspfRpfLocationSection* locSec = rpfHdr->getLocationSection();
   
   // Clear the records copied from the source a.toc.
   locSec->clearFields();
   
   //---
   // Set the length of the locSec to 74.  The record itself is 14 bytes plus
   // an additional 60 bytes for six location records ten bytes each.
   //---
   const rspf_uint16 LOCATION_SECTION_SIZE = 74;
   locSec->setLocationSectionLength(LOCATION_SECTION_SIZE);
   
   // Set the offset which 14 bytes to get to the first record.
   locSec->setLocationTableOffset(14);
   
   // Six records:
   locSec->setNumberOfComponentLocationRecords(6);
   
   // Each record 10 bytes:
   locSec->setLocationRecordLength(10);
   
   // Don't know the aggregate length yet.
   
   rspfRpfComponentLocationRecord locRec;
   
   // Note: See rspfRpfConstants for enum rspfRpfComponentId
   
   const rspf_uint32 RPFHDR_SIZE = 48;
   const rspf_uint32 LOCATION_SECTION_OFFSET = static_cast<rspf_uint32>(locationSectionOffset);
   const rspf_uint32 BOUNDARY_SUBHEADER_SIZE = 8;
   const rspf_uint32 BOUNDARY_RECORD_SIZE = 132;
   const rspf_uint32 FILE_SUBHEADER_SIZE = 13;
   // const rspf_uint32 = ;
   
   // Record 1 RPFHDR location:
   rspf_uint32 rpfHdrOffset = 0;
   if ( fileHdr->getTag(rpfHdrInfo, "RPFHDR") )
   {
      rpfHdrOffset = rpfHdrInfo.getTagDataOffset();
   }

   locRec.m_componentId = RSPF_RPF_HEADER_COMPONENT; // 128
   locRec.m_componentLength = RPFHDR_SIZE;
   locRec.m_componentLocation = static_cast<rspf_uint32>(rpfHdrInfo.getTagDataOffset());
   locSec->addComponentRecord(locRec);

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<< "rpf hdr offset: " << rpfHdrOffset << "\n";
      locRec.print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
   
   // Record 2 location section:
   locRec.m_componentId = RSPF_RPF_LOCATION_COMPONENT; // 129
   locRec.m_componentLength = LOCATION_SECTION_SIZE;
   locRec.m_componentLocation = LOCATION_SECTION_OFFSET;
   locSec->addComponentRecord(locRec);

   if ( traceDebug() )
   {
      locRec.print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
   
   // Record 3 boundary rect sub header section:
   locRec.m_componentId = RSPF_RPF_BOUNDARY_RECT_SECTION_SUBHEADER; // 148
   locRec.m_componentLength = BOUNDARY_SUBHEADER_SIZE;
   locRec.m_componentLocation = locRec.m_componentLocation + LOCATION_SECTION_SIZE;
   locSec->addComponentRecord(locRec);

   if ( traceDebug() )
   {
      locRec.print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
   
   // Capture the location.
   std::streamoff boundaryRectPosition = locRec.m_componentLocation;
   
   // Record 4 boundary rect table:
   locRec.m_componentId = RSPF_RPF_BOUNDARY_RECT_TABLE; // 149
   locRec.m_componentLength = BOUNDARY_RECORD_SIZE;
   locRec.m_componentLocation = locRec.m_componentLocation + BOUNDARY_SUBHEADER_SIZE;
   locSec->addComponentRecord(locRec);

   if ( traceDebug() )
   {
      locRec.print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
   
   // Record 5 file index sub header:
   locRec.m_componentId = RSPF_RPF_FRAME_FILE_INDEX_SECTION_SUBHEADER; // 150
   locRec.m_componentLength = FILE_SUBHEADER_SIZE;
   locRec.m_componentLocation = locRec.m_componentLocation + BOUNDARY_RECORD_SIZE;
   locSec->addComponentRecord(locRec);

   if ( traceDebug() )
   {
      locRec.print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
    
   // Record 6 file index sub header:
   locRec.m_componentId = RSPF_RPF_FRAME_FILE_INDEX_SUBSECTION; // 151
   locRec.m_componentLength = 0;  // need to calculate.
   locRec.m_componentLocation = locRec.m_componentLocation + FILE_SUBHEADER_SIZE;
   locSec->addComponentRecord(locRec);

   if ( traceDebug() )
   {
      locRec.print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }
   
   // Seek back and re-write...
   dotTocStr->seekp(0, ios::beg);
   fileHdr->writeStream(*dotTocStr);
   
   dotTocStr->seekp(boundaryRectPosition, ios::beg);

   // Only writing one entry:
   boundaryRectSectionSubheader->setNumberOfEntries(1);

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "writing boundaryRectSectionSubheader:\n" << *(boundaryRectSectionSubheader.get())
         << "\n";
   }

   //---
   // Write the boundary rectangle section.  This includes the subheader and subsection.
   // These coorespond to location records 3 and 4 above.
   //---
   boundaryRectSectionSubheader->writeStream(*dotTocStr);

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) 
         << "Original boundaryRectTable:\n" << *(boundaryRectTable.get()) << "\n";
   }

   rspf_uint32 entry;
   if ( getCorespondingEntry( frameFileSubSection.get(), *dotRpfStr, entry ) )
   {
      rspfRpfBoundaryRectRecord boundaryRectRecord;
      if ( boundaryRectTable->getEntry( entry, boundaryRectRecord) )
      {
         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG) 
               << "writing boundaryRectTable:\n" << boundaryRectRecord << "\n";
         }
         
         boundaryRectRecord.writeStream(*dotTocStr);
      }
      else
      {
         std::string e = MODULE;
         e += " ERROR:\nCould not get bounding rect record for entry: ";
         e += rspfString::toString(entry).c_str();
         throw rspfException(e);
      }
   }
   else
   {
      std::string e = MODULE;
      e += " ERROR:\nCould not deduce entry from frame list!";
      throw rspfException(e);
   }

   frameFileSubHeader->setNumberOfIndexRecords( getNumberOfFrames(*dotRpfStr) );
   frameFileSubHeader->setNumberOfPathnameRecords(1);
   const rspf_uint16 FRAME_FILE_INDEX_RECORD_LENGTH = 33;
   frameFileSubHeader->setIndexRecordLength( FRAME_FILE_INDEX_RECORD_LENGTH );

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "writing frameFileSubHeader:\n" << *(frameFileSubHeader.get()) << "\n";
   }
   frameFileSubHeader->writeStream( *dotTocStr );

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "writing frameFileSubSection:\n";
   }

   std::streamoff frameFileIndexSectionStartPos = dotTocStr->tellp();
   
   writeFrameFileIndexSection(frameFileSubSection.get(), *dotRpfStr, *dotTocStr);
   
   std::streamoff endOfFilePos = dotTocStr->tellp();

   // Update the location section length for the frame file index section.
   locSec->getLocationRecordList()[5].m_componentLength =
      static_cast<rspf_uint32>(endOfFilePos - frameFileIndexSectionStartPos);

   // Update the length of all location sections.
   locSec->setComponentAggregateLength(
      static_cast<rspf_uint32>(endOfFilePos) - rpfHdr->getLocationSectionLocation() );
   

   fileHdr->setFileLength(static_cast<rspf_uint64>(endOfFilePos));
   dotTocStr->seekp(0, ios::beg);
   fileHdr->writeStream(*dotTocStr);

   rspfNotify(rspfNotifyLevel_DEBUG) << "Wrote file: " << dotTocFile << "\n";

   // Copy the frames to the output directory.
   copyFrames(*dotRpfStr, outputDir);

   // Cleanup:
   delete dotRpfStr;
   dotRpfStr = 0;
   delete dotTocStr;
   dotTocStr =0;
}

std::ostream& rspfRpfToc::print(std::ostream& out,
                                 const std::string& prefix,
                                 bool printOverviews) const
{
   if( m_rpfHeader.valid() )
   {
      m_rpfHeader->print(out, prefix);

      //---
      // Go through the entries...  We're going to skip overviews here.
      //---
      rspf_uint32 prefixIndex = 0;
      std::vector< rspfRpfTocEntry*>::const_iterator tocEntry =
         m_tocEntryList.begin();
      while(tocEntry != m_tocEntryList.end())
      {
         if (*tocEntry)
         {
            if ( traceDebug() )
            {
               (*tocEntry)->print(out, prefix);
            }
            
            const rspfRpfBoundaryRectRecord REC =
               (*tocEntry)->getBoundaryInformation();

            rspfString scale = REC.getScale();
            if ( (scale.contains("OVERVIEW")) == false ||
                 printOverviews )
            {
               rspfString entryPrefix = prefix;
               entryPrefix += "image";
               entryPrefix += rspfString::toString(prefixIndex);
               entryPrefix += ".";
               REC.print(out, entryPrefix);

               //---
               // Get the first frame that exists so we can get to
               // the attributes.
               //---
               rspfRpfFrameEntry frameEntry;
               getFirstEntry((*tocEntry), frameEntry);

               if (frameEntry.exists())
               {
                  rspfRpfFrame rpfFrame;
                  if ( rpfFrame.parseFile(frameEntry.getFullPath())
                       == rspfErrorCodes::RSPF_OK )
                  {
                     rpfFrame.print(out, entryPrefix);
                  }
               }
            }
         }
         ++prefixIndex;
         ++tocEntry;
      }
   }
   return out;
}

rspf_uint32 rspfRpfToc::getNumberOfEntries()const
{
   return static_cast<rspf_uint32>(m_tocEntryList.size());
}

const rspfRpfTocEntry* rspfRpfToc::getTocEntry(rspf_uint32 index)const
{
   if(index < m_tocEntryList.size())
   {
      return m_tocEntryList[index];
   }
   return 0;
}

rspf_int32 rspfRpfToc::getTocEntryIndex(const rspfRpfTocEntry* entry)

{
   for(rspf_int32 i = 0; i < (rspf_int32)m_tocEntryList.size(); ++i)
   {
      if(m_tocEntryList[i] == entry)
      {
         return i;
      }
   }

   return -1;
}

const rspfNitfFileHeader* rspfRpfToc::getNitfFileHeader()const
{
   return m_nitfFileHeader.get();
}

const rspfRpfHeader* rspfRpfToc::getRpfHeader()const
{
   return m_rpfHeader.get();
}

rspf_uint32 rspfRpfToc::getNumberOfFramesHorizontal(rspf_uint32 idx) const
{
   rspf_uint32 nFrames = 0;
   const rspfRpfTocEntry* pEntry = getTocEntry( idx );
   if ( pEntry != NULL )
   {
      nFrames = pEntry->getNumberOfFramesHorizontal();
   }
   return nFrames;
}

rspf_uint32 rspfRpfToc::getNumberOfFramesVertical(rspf_uint32 idx) const
{
   rspf_uint32 nFrames = 0;
   const rspfRpfTocEntry* pEntry = getTocEntry( idx );
   if ( pEntry != NULL )
   {
      nFrames = pEntry->getNumberOfFramesVertical();
   }
   return nFrames;
}

bool rspfRpfToc::getRpfFrameEntry(rspf_uint32 entryIdx, 
                                   rspf_uint32 row,
                                   rspf_uint32 col,
                                   rspfRpfFrameEntry& result)const
{
   const rspfRpfTocEntry* pEntry = getTocEntry( entryIdx );
   if ( pEntry != NULL )
   {
      return pEntry->getEntry( row, col, result );
   }
   return false;
}

const rspfString rspfRpfToc::getRelativeFramePath( rspf_uint32 entryIdx,
                                                     rspf_uint32 row,
                                                     rspf_uint32 col) const
{
   rspfRpfFrameEntry frameEntry;
   bool bResult = getRpfFrameEntry( entryIdx, row, col, frameEntry );
   if ( bResult == true )
   {
      return frameEntry.getPathToFrameFileFromRoot();
   }
   return rspfString("");
}

void rspfRpfToc::deleteAll()
{
   m_rpfHeader = 0;

   deleteTocEntryList();
}

void rspfRpfToc::clearAll()
{
   deleteTocEntryList();
   m_filename = "";
}

void rspfRpfToc::deleteTocEntryList()
{
   std::vector< rspfRpfTocEntry*>::iterator tocEntry =
      m_tocEntryList.begin();
   while(tocEntry != m_tocEntryList.end())
   {
      if(*tocEntry)
      {
         delete (*tocEntry);
         *tocEntry = NULL;
      }
      ++tocEntry;
   }
   m_tocEntryList.clear();
}

void rspfRpfToc::buildTocEntryList(rspfRpfHeader* rpfHeader)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfRpfToc::buildTocEntryList: entered.....\n";
   }
   std::ifstream in(m_filename.c_str(), std::ios::in | std::ios::binary);

   if(!in)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfRpfToc::buildTocEntryList: invalid input leaving..... " << std::endl;
      }
      return;
   }
   
   if(rpfHeader)
   {
      if(traceDebug())
      {
         rpfHeader->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
      
      deleteTocEntryList();
      rspfRpfBoundaryRectSectionSubheader *boundaryRect =
         rpfHeader->getNewBoundaryRectSectSubheader(in);

      if(boundaryRect)
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG: Got boundary rect\n";
         }
         std::streamoff current = in.tellg();

         // they give the offset from the
         // end of the boundary subheader to the start of the 
         // entry table.  We have to create an absolute
         // offset.
         current += boundaryRect->getTableOffset();

         // take to the start of the table entries
         in.seekg(current, ios::beg);
         allocateTocEntryList(boundaryRect->getNumberOfEntries());

         // now we can read the entries
         if(m_tocEntryList.size() > 0)
         {
            for(rspf_uint32 index = 0; index < m_tocEntryList.size(); index++)
            {
               m_tocEntryList[index]->parseStream(in, rpfHeader->getByteOrder());
            }
         }
         
         rspfRpfFrameFileIndexSectionSubheader* frameFileIndexHead = rpfHeader->getNewFrameFileIndexSectionSubheader(in);
         // get the offset to the table
         long offsetToIndexSubsection = in.tellg();
         if(frameFileIndexHead)
         {
            rspfRpfFrameFileIndexRecord tempIndexRec;
            rspfRpfPathnameRecord       tempPathNameRec;
            
            rspf_int32 count = frameFileIndexHead->getNumberOfIndexRecords();
            while(count > 0)
            {
               tempIndexRec.parseStream(in, rpfHeader->getByteOrder() );

               // get the path information.  we must seek to a different location
               // within the file.  So we must remember where we currently are at
               std::streamoff currentPosition = in.tellg();
               
               in.seekg(offsetToIndexSubsection + tempIndexRec.getPathnameRecordOffset(), ios::beg);
               tempPathNameRec.parseStream(in, rpfHeader->getByteOrder());

               // We have the root directory where all frame files are subfiles of
//               rspfString rootDirectory(rspfFilename(m_filename.path())+
               // rspfFilename(rspfFilename::thePathSeparator));
               rspfFilename rootDirectory;
               getRootDirectory(rootDirectory);

               // we have the actual path from the root directory to the
               // frame file.  We must separate the two.  There have been
               // occurrences where the path in the A.TOC file
               // is upper case but the path in the directory on the file
               // system is lower case.  This
               // will fool the system in thinking the file does not exist
               // when it actually does.
               rspfString pathToFrameFile( rspfFilename(tempPathNameRec.getPathname()) +
                                              tempIndexRec.getFilename());

               rspfRpfFrameEntry entry(rootDirectory,
                                        pathToFrameFile);
               m_tocEntryList[tempIndexRec.getBoundaryRecNumber()]->setEntry(entry,
                                                                              tempIndexRec.getLocationRowNumber(),
                                                                              tempIndexRec.getLocationColNumber());
               // now go back to where we were
               in.seekg(currentPosition, ios::beg);
               
               --count;
            }
            delete frameFileIndexHead;
            frameFileIndexHead = 0;
         }
      }
      delete boundaryRect;
      boundaryRect = NULL;
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfRpfToc::buildTocEntryList: leaving....." << std::endl;
   }
}

void rspfRpfToc::allocateTocEntryList(rspf_uint32 numberOfEntries)
{
   if(m_tocEntryList.size() > 0)
   {
      deleteTocEntryList();
   }
   for(rspf_uint32 index = 0; index < numberOfEntries; index++)
   {
      m_tocEntryList.push_back(new rspfRpfTocEntry);
   }   
}

void rspfRpfToc::getRootDirectory(rspfFilename& dir) const
{
   dir = m_filename.expand().path();
}

void rspfRpfToc::getFirstEntry(const rspfRpfTocEntry* rpfTocEntry,
                                rspfRpfFrameEntry& frameEntry) const
{
   if (rpfTocEntry)
   {
      rspf_int32 framesHorizontal =
         rpfTocEntry->getNumberOfFramesHorizontal();
      rspf_int32 framesVertical =
         rpfTocEntry->getNumberOfFramesVertical();
   
      bool foundEntry = false;
      for (rspf_int32 v = 0; v < framesVertical; ++v)
      {
         for (rspf_int32 h = 0; h < framesHorizontal; ++h)
         {
            rpfTocEntry->getEntry(v, h, frameEntry);
            if (frameEntry.exists())
            {
               foundEntry = true;
               break;
            }
         }
         if (foundEntry)
         {
            break;
         }
      }
   }
}

void rspfRpfToc::writeFrameFileIndexSection(rspfRpfFrameFileIndexSubsection* frameFileSubSection,
                                             std::ifstream& dotRpfStr,
                                             std::ofstream& dotTocStr)
{
   static const char MODULE[] = "rspfRpfToc::writeFrameFileIndexSection";

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   const rspf_uint16 FRAME_FILE_INDEX_RECORD_LENGTH = 33;
   rspf_uint32 frames = getNumberOfFrames(dotRpfStr);
   rspf_uint32 pathnameRecordOffset = FRAME_FILE_INDEX_RECORD_LENGTH * frames;

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "frames: " << frames << "\n";
   }
   
   if ( !dotRpfStr.good() )
   {
      // see if we can clear it.  Someone might have hit end of file(eof).
      dotRpfStr.clear();
   }

   dotRpfStr.seekg(0, ios_base::beg);

   std::string line;
   rspfFilename file;
   rspfRpfFrameFileIndexRecord record;
   rspf_uint32 framesWritten = 0;

   // Eat the first line which is the bounding rect line
   std::getline(dotRpfStr, line);

   while( dotRpfStr.good() )
   {
      std::getline(dotRpfStr, line);

      if ( dotRpfStr.good() )
      {
         if ( getFile(line, file) )
         {
            if ( frameFileSubSection->getFrameFileIndexRecordFromFile(file.file(), record) )
            {
               // Always single entry.
               record.setBoundaryRecNumber(0);
               
               record.setPathnameRecordOffset(pathnameRecordOffset);
               record.writeStream(dotTocStr);
               ++framesWritten;

               if ( traceDebug() )
               {
                  rspfNotify(rspfNotifyLevel_DEBUG) << "wrote record:\n" << record << "\n";
               }
            }
         }
      }
   }

   // Now set the path record.
   rspfFilename d = file.path();
   rspfString s = "./";
   s += d.file();
   s += "/";
   rspfRpfPathnameRecord pathRecord;
   pathRecord.setPathName(s);
   pathRecord.writeStream(dotTocStr);

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "frames written: " << framesWritten
         << "\nwrote record:\n" << pathRecord
         << "\n";
   }

   dotRpfStr.clear();
   dotRpfStr.seekg(0, ios_base::beg);

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited..." << std::endl;
   }
}

void rspfRpfToc::copyFrames(std::ifstream& dotRpfStr, const rspfFilename& outputDir)
{
   static const char MODULE[] = "rspfRpfToc::copyFrames";

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspf_uint32 frames = getNumberOfFrames(dotRpfStr);

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "frames to copy: " << frames << "\n";
   }
   
   if ( !dotRpfStr.good() )
   {
      // see if we can clear it.  Someone might have hit end of file(eof).
      dotRpfStr.clear();
   }

   dotRpfStr.seekg(0, ios_base::beg);

   std::string line;
   rspfFilename file;
   rspfFilename destinationFile;
   rspfFilename subDir;
   rspf_uint32 framesCopied = 0;
            
   // Eat the first line which is the bounding rect line
   std::getline(dotRpfStr, line);

   // Get the second line which is first file. 
   std::getline(dotRpfStr, line);

   // Get the file name and make the sub directory if needed.
   if ( getFile(line, file) )
   {
      destinationFile = outputDir;
      subDir = file.path();
      subDir = subDir.file();
      destinationFile = destinationFile.dirCat( subDir );
      
      // This is output_dir/subdir.  See if subdir exist:
      if ( !destinationFile.exists() )
      {
         destinationFile.createDirectory();
      }
   }

   // Start over:
   if ( !dotRpfStr.good() )
   {
      // see if we can clear it.  Someone might have hit end of file(eof).
      dotRpfStr.clear();
   }
   dotRpfStr.seekg(0, ios_base::beg);
   
   // Eat the first line which is the bounding rect line
   std::getline(dotRpfStr, line);
   
   while( dotRpfStr.good() )
   {
      std::getline(dotRpfStr, line);

      if ( dotRpfStr.good() )
      {
         if ( getFile(line, file) )
         {
            destinationFile = outputDir;
            subDir = file.path();
            subDir = subDir.file();
            destinationFile = destinationFile.dirCat( subDir );
            destinationFile = destinationFile.dirCat( file.file() );

            if ( file.copyFileTo(destinationFile) )
            {
               ++framesCopied;
            }
            if ( traceDebug() )
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "Copied frame: " << destinationFile << "\n";
            }
         }
      }
   }

   rspfNotify(rspfNotifyLevel_NOTICE) << "Frames copied: " << framesCopied << std::endl;

   dotRpfStr.clear();
   dotRpfStr.seekg(0, ios_base::beg);

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited..." << std::endl;
   }
}

rspf_uint32 rspfRpfToc::getNumberOfFrames(std::ifstream& dotRpfStr) const
{
   rspf_uint32 result = 0;

   if ( !dotRpfStr.good() )
   {
      // see if we can clear it.  Someone might have hit end of file(eof).
      dotRpfStr.clear();
   }

   dotRpfStr.seekg(0, ios_base::beg);

   // Eat the first line which is the bounding rect.
   std::string line;
   std::getline(dotRpfStr, line);
   
   while( dotRpfStr.good() )
   {
      std::getline(dotRpfStr, line);

      if ( dotRpfStr.good() )
      {
         ++result;
      }
   }

   dotRpfStr.clear();
   dotRpfStr.seekg(0, ios_base::beg);

   return result;
}

rspfFilename rspfRpfToc::getSourceTocFile(std::ifstream& dotRpfStr) const
{
   rspfFilename tocFile;
   
   if ( !dotRpfStr.good() )
   {
      dotRpfStr.clear();
   }

   dotRpfStr.seekg(0, ios_base::beg);

   // Eat the first line which is the bounding rect.
   std::string line;
   std::getline(dotRpfStr, line);

   // Get the second line which is first file. 
   std::getline(dotRpfStr, line);

   // Get the file name and make the sub directory if needed.
   rspfFilename file;
   if ( getFile(line, file) )
   {
      rspfFilename subDir = file.path();
      tocFile = subDir.dirCat("a.toc");
      if ( !tocFile.exists() )
      {
         tocFile = subDir.dirCat("A.TOC");
         if ( !tocFile.exists() )
         {
            subDir = subDir.path();
            tocFile = subDir.dirCat("a.toc");
            if ( !tocFile.exists() )
            {
               tocFile = subDir.dirCat("A.TOC");
               if ( !tocFile.exists() )
               {
                  tocFile.clear();
               }
            }
         }
      }
   }

   if ( !dotRpfStr.good() )
   {
      dotRpfStr.clear();
   }
   dotRpfStr.seekg(0, ios_base::beg);

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfRpfToc::getSourceTocFile result: " << tocFile << "\n";
   }

   return tocFile;
}

bool rspfRpfToc::getCorespondingEntry(
   rspfRpfFrameFileIndexSubsection* frameFileSubSection,
   std::ifstream& dotRpfStr, rspf_uint32& entry) const
{
   bool result = false;
   
   if ( !dotRpfStr.good() )
   {
      // see if we can clear it.  Someone might have hit end of file(eof).
      dotRpfStr.clear();
   }
   
   dotRpfStr.seekg(0, ios_base::beg);

   std::string line;
   rspfFilename file;
   rspfRpfFrameFileIndexRecord record;

   // Eat the first line which is the bounding rect line
   std::getline(dotRpfStr, line);

   std::getline(dotRpfStr, line);

   if ( getFile(line, file) )
   {
      if ( frameFileSubSection->getFrameFileIndexRecordFromFile(file.file(), record) )
      {
         entry = record.getBoundaryRecNumber();
         result = true;
      }
   }

   if ( !dotRpfStr.good() )
   {
      dotRpfStr.clear();
   }

   dotRpfStr.seekg(0, ios_base::beg);
   return result;
}

bool rspfRpfToc::getFile(const std::string& line, rspfFilename& file) const
{
   //---
   // line form:
   // /some_path/N03E030/006EZC1B.I21|30.4654240835311,3.59074642049858|30.5401678385356,3.65984532949882
   //           <file>                    <lr_lon>           <lr_lat>        <ur_lon>          <ur_lat>
   //---
   
   bool result = false;
   if ( line.size() )
   {
      rspfString os(line);
      std::vector<rspfString> lineArray;
      os.split(lineArray, "|");
      if ( lineArray.size() )
      {
         file = lineArray[0];
         result = true;
      }
   }
   return result;
}
