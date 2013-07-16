//*******************************************************************
//
//  License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfFile.cpp 21351 2012-07-20 13:27:15Z dburken $

#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/support_data/rspfNitfImageHeader.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_0.h>
#include <rspf/support_data/rspfNitfFileHeaderV2_1.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_0.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_1.h>
#include <rspf/support_data/rspfNitfTagFactoryRegistry.h>
#include <rspf/support_data/rspfNitfRegisteredTag.h>
#include <rspf/support_data/rspfRpfToc.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfNotify.h>
#include <fstream>
#include <iostream>
#include <iomanip>


// Static trace for debugging
static rspfTrace traceDebug("rspfNitfFile:debug");

std::ostream& operator <<(std::ostream& out, const rspfNitfFile& data)
{
   return data.print(out);
}

std::ostream& rspfNitfFile::print(std::ostream& out,
                                   const std::string& prefix,
                                   bool printOverviews) const
{
   if(theNitfFileHeader.valid())
   {
      std::string pfx = prefix;
      pfx += "nitf.";
      theNitfFileHeader->print(out, pfx);
      
      rspf_int32 n = theNitfFileHeader->getNumberOfImages();
      for(rspf_int32 idx = 0; idx < n; ++idx)
      {
         rspfNitfImageHeader* ih = getNewImageHeader(idx);
         if(ih)
         {
            bool printIt = true;
            
            if ( !printOverviews )
            {
               // Check the IMAG field.
               rspf_float64 imag;
               ih->getDecimationFactor(imag);
               if ( !rspf::isnan(imag) )
               {
                  if ( imag < 1.0)
                  {
                     printIt = false;
                  }
               }

               //---
               // Now see if it's a cloud mask image.  Do not print
               // cloud mask images if the printOverviews is false.
               //---
               if ( printIt )
               {
                  if ( (ih->getCategory().trim(rspfString(" ")) ==
                        "CLOUD") &&
                       (ih->getRepresentation().trim(rspfString(" ")) ==
                        "NODISPLY") )
                  {
                     printIt = false;
                  }
               }
            }

            if (printIt)
            {
               // Add our prefix onto prefix.
               std::string s = pfx;
               s += "image";
               s += rspfString::toString(idx).string();
               s += ".";
               
               ih->print(out, s);
            }
            
            delete ih;
            ih = 0;
         }
      }

      //---
      // Check for RPF stuff:
      //---
      rspfNitfTagInformation info; 
      theNitfFileHeader->getTag(info, "RPFHDR");
      if(info.getTagName() == "RPFHDR")
      {
         // Open of the a.toc.
         rspfRefPtr<rspfRpfToc> toc = new rspfRpfToc;
         if ( toc->parseFile(getFilename()) ==
              rspfErrorCodes::RSPF_OK )
         {
            pfx += "rpf.";
            toc->print(out, pfx, printOverviews);
         }
      }
      
   } // matches:  if(theNitfFileHeader.valid())

   return out;
   
}

bool rspfNitfFile::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   bool result = theNitfFileHeader.valid(); 
   
   if(theNitfFileHeader.valid())
   {
      theNitfFileHeader->saveState(kwl, prefix);
   }
   rspf_int32 n = theNitfFileHeader->getNumberOfImages();
   for(rspf_int32 idx = 0; idx < n; ++idx)
   {
      rspfRefPtr<rspfNitfImageHeader> ih = getNewImageHeader(idx);
      rspfString newPrefix = prefix + "image" + rspfString::toString(idx) + ".";
#if 1     
      ih->saveState(kwl, newPrefix);
#else
      if ( (ih->getCategory().trim(rspfString(" ")) !=
            "CLOUD") ||
          (ih->getRepresentation().trim(rspfString(" ")) !=
           "NODISPLY") )
      {
         ih->saveState(kwl, newPrefix);
      }
#endif
   }
   return result;
}

rspfNitfFile::rspfNitfFile()
   : theFilename(""),
     theNitfFileHeader(0)
{
}

rspfNitfFile::~rspfNitfFile()
{
   theNitfFileHeader = 0;
}

bool rspfNitfFile::parseFile(const rspfFilename& file)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfNitfFile::parseFile: "
         << "endtered......"
         << std::endl;
   }
   std::ifstream in(file.c_str(), std::ios::in|std::ios::binary);
   if (in.fail())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfNitfFile::parseFile: "
            << "Could not open file:  " << file.c_str()
            << "\nReturning..." << std::endl;
      }
      return false;
   }
      
   if(theNitfFileHeader.valid())
   {
      theNitfFileHeader = 0;
   }

   char temp[10];
   in.read(temp, 9);
   in.seekg(0, std::ios::beg);
   temp[9] ='\0';
   
   theFilename = file;

   rspfString s(temp);
   if(s == "NITF02.00")
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG: NITF Version 2.0"
            << std::endl;
      }
      theNitfFileHeader = new rspfNitfFileHeaderV2_0;
   }
   else if ( (s == "NITF02.10") || (s == "NSIF01.00") )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG: NITF Version 2.1"
            << std::endl;
      }
      theNitfFileHeader = new rspfNitfFileHeaderV2_1;
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfNitfFile::parseFile: "
            << "Not an NITF file!"
            << std::endl;
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfNitfFile::parseFile: returning...........false"
            << "endtered......"
            << std::endl;
      }
      return false;
   }

   if(theNitfFileHeader.valid())
   {
      try
      {
         theNitfFileHeader->parseStream(in);

         // Sanity check the size before going on:
         if ( file.fileSize() < theNitfFileHeader->getFileSize() )
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfNitfFile::parseFile ERROR:\n"
                  << "File size is less than file length in header!"
                  << "\nNITF FL field: " << theNitfFileHeader->getFileSize()
                  << "\nActual file length: " << file.fileSize()
                  << std::endl;
            }
            return false;
         }
      }
      catch( const rspfException& e )
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfNitfFile::parseFile caught exception:\n"
               << e.what()
               << std::endl;
         }
         return false;
      }
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfNitfFile::parseFile: returning...........true"
         << std::endl;
   }
   return true;
}

const rspfNitfFileHeader* rspfNitfFile::getHeader() const
{
   return theNitfFileHeader.get();
}

rspfNitfFileHeader* rspfNitfFile::getHeader() 
{
   return theNitfFileHeader.get();
}

rspfIrect rspfNitfFile::getImageRect()const
{
   if(theNitfFileHeader.valid())
   {
      return theNitfFileHeader->getImageRect();
   }

   return rspfIrect(rspfIpt(0,0), rspfIpt(0,0));
}

rspfNitfImageHeader* rspfNitfFile::getNewImageHeader(
   rspf_uint32 imageNumber)const
{
   rspfNitfImageHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      try // getNewImageHeader can throw exception on parse.
      {
         std::ifstream in(theFilename.c_str(), std::ios::in|std::ios::binary);
         result = theNitfFileHeader->getNewImageHeader(imageNumber, in);
         in.close();
      }
      catch( const rspfException& e )
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfNitfFile::getNewImageHeader caught exception:\n"
               << e.what()
               << std::endl;
         }
         result = 0;
      }
   }
   return result;
}

rspfNitfSymbolHeader* rspfNitfFile::getNewSymbolHeader(
   rspf_uint32 symbolNumber)const
{
   rspfNitfSymbolHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::ifstream in(theFilename.c_str(), std::ios::in|std::ios::binary);

      result = theNitfFileHeader->getNewSymbolHeader(symbolNumber, in);
      in.close();
   }
   
   return result;
}

rspfNitfLabelHeader* rspfNitfFile::getNewLabelHeader(
   rspf_uint32 labelNumber)const
{
   rspfNitfLabelHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::ifstream in(theFilename.c_str(), std::ios::in|std::ios::binary);

      result = theNitfFileHeader->getNewLabelHeader(labelNumber, in);
      in.close();
   }
   
   return result;
}

rspfNitfTextHeader* rspfNitfFile::getNewTextHeader(
   rspf_uint32 textNumber)const
{
   rspfNitfTextHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::ifstream in(theFilename.c_str(), std::ios::in|std::ios::binary);

      result = theNitfFileHeader->getNewTextHeader(textNumber, in);
      in.close();
   }
   
   return result;
}

rspfNitfDataExtensionSegment* rspfNitfFile::getNewDataExtensionSegment(
   rspf_uint32 dataExtNumber)const
{
   rspfNitfDataExtensionSegment* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::ifstream in(theFilename.c_str(), std::ios::in|std::ios::binary);

      result = theNitfFileHeader->getNewDataExtensionSegment(dataExtNumber, in);
      in.close();
   }
   
   return result;
}

rspfString rspfNitfFile::getVersion()const
{
   if(theNitfFileHeader.valid())
   {
      return rspfString(theNitfFileHeader->getVersion());
   }
   
   return rspfString("");
}

rspfFilename rspfNitfFile::getFilename() const
{
   return theFilename;
}
