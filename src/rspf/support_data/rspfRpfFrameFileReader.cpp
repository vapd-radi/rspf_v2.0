//*******************************************************************
//
// LICENSE:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfFrameFileReader.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <rspf/support_data/rspfRpfFrameFileReader.h>
#include <rspf/support_data/rspfRpfHeader.h>
#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/support_data/rspfNitfTagInformation.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <fstream>
#include <ostream>

std::ostream& operator<<(std::ostream& out, const rspfRpfFrameFileReader& data)
{
   return data.print(out);
}

rspfRpfFrameFileReader::rspfRpfFrameFileReader()
   :theRpfHeader(0),
    theFilename("")
{
}

rspfRpfFrameFileReader::~rspfRpfFrameFileReader()
{
   theRpfHeader = 0;
}

rspfErrorCode rspfRpfFrameFileReader::parseFile(const rspfFilename& fileName)
{
   std::ifstream in(fileName.c_str(), ios::in | ios::binary);
   rspfRefPtr<rspfNitfFile> nitfFile = new rspfNitfFile;

   if(!in)
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   clearAll();
   nitfFile->parseFile(fileName);
   const rspfRefPtr<rspfNitfFileHeader> nitfFileHeader =
      nitfFile->getHeader();
   if(!nitfFileHeader)
   {
      nitfFile = 0;
      return rspfErrorCodes::RSPF_ERROR;
   }

   rspfNitfTagInformation info; 
   nitfFileHeader->getTag(info, "RPFHDR");
   // we no longer need access to the nitf header.  We got what we needed
   nitfFile = 0;
   theFilename = fileName;
   if(info.getTagName() == "RPFHDR")
   {      
      theRpfHeader = new rspfRpfHeader;
      
      // set the get pointer for the stream to the start
      // of the Rpf header data
      in.seekg(info.getTagDataOffset());
      
      // now get the header data.  We do not need to pass in the byte order.
      // this is grabbed from the first byte of the stream.  To see this,
      // Look at the RpfHeader implementation.
      theRpfHeader->parseStream(in); // ==rspfErrorCodes::RSPF_OK)
      if ( in.fail() )
      {
         theRpfHeader = 0;
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   if( !theRpfHeader.valid() )
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

std::ostream& rspfRpfFrameFileReader::print(std::ostream& out)const
{
   if( theRpfHeader.valid() )
   {
      theRpfHeader->print(out, std::string(""));
   }
   return out;
}

const rspfRpfHeader* rspfRpfFrameFileReader::getRpfHeader()const
{
   return theRpfHeader.get();
}

void rspfRpfFrameFileReader::clearAll()
{
   theRpfHeader = 0;
   theFilename = "";
}
