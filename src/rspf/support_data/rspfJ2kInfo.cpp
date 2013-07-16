//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: J2K Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <rspf/support_data/rspfJ2kInfo.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/support_data/rspfJ2kCodRecord.h>
#include <rspf/support_data/rspfJ2kSizRecord.h>
#include <rspf/support_data/rspfJ2kSotRecord.h>

// Static trace for debugging
static rspfTrace traceDebug("rspfJ2kInfo:debug");
static rspfTrace traceDump("rspfJ2kInfo:dump"); // This will dump offsets.

static const rspf_uint16 SOC_MARKER = 0xff4f; // start of codestream marker
static const rspf_uint16 SIZ_MARKER = 0xff51; // size maker
static const rspf_uint16 COD_MARKER = 0xff52; // cod maker
static const rspf_uint16 SOT_MARKER = 0xff90; // start of tile marker
static const rspf_uint16 EOC_MARKER = 0xffd9; // End of codestream marker.

rspfJ2kInfo::rspfJ2kInfo()
   : rspfInfoBase(),
     theFile(),
     theEndian(0)
{
}

rspfJ2kInfo::~rspfJ2kInfo()
{
   if (theEndian)
   {
      delete theEndian;
      theEndian = 0;
   }
}

bool rspfJ2kInfo::open(const rspfFilename& file)
{
   bool result = false;

   //---
   // Open the file.
   //---
   std::ifstream str(file.c_str(), std::ios_base::binary|std::ios_base::in);
   if (str.good()) 
   {
      if (rspf::byteOrder() == RSPF_LITTLE_ENDIAN)
      {
         if (!theEndian)
         {
            theEndian = new rspfEndian();
         }
      }
      else if (theEndian)
      {
         delete theEndian;
         theEndian = 0;
      }

      //---
      // Check for the Start Of Codestream (SOC) and Size (SIZ) markers which
      // are required as first and second fields in the main header.
      //---
      rspf_uint16 soc;
      rspf_uint16 siz;
      readShort(soc, str);
      readShort(siz, str);

      if ( (soc == SOC_MARKER) && (siz == SIZ_MARKER) )
      {
         result = true; // Is a j2k...
      }
   }

   if (result)
   {
      theFile = file;
   }
   else
   {
      theFile.clear();
      if (theEndian)
      {
         delete theEndian;
         theEndian = 0;
      }
   }

   return result;
}

std::ostream& rspfJ2kInfo::print(std::ostream& out) const
{
   static const char MODULE[] = "rspfJ2kInfo::print";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG Entered...\n";
   }
      
   //---
   // Open the tif file.
   //---
   std::ifstream str(theFile.c_str(), std::ios_base::binary|std::ios_base::in);
   if (str.good())
   {
      rspf_uint16 marker;
      readShort(marker, str); // SOC
      readShort(marker, str); // SIZ

      std::string prefix = "j2k.";

      // SIZ marker required next.
      printSizMarker(out, prefix, str);

      readShort(marker, str);
      
      while ( str.good() && (marker != EOC_MARKER) ) // marker != SOT_MARKER )
      {
         switch(marker)
         {
            case COD_MARKER:
            {
               printCodMarker(out, prefix, str);
               break;
            }
            case SOT_MARKER:
            {
               printSotMarker(out, prefix, str);
               break;
            }
            default:
            {
               printUnknownMarker(out, prefix, str, marker);
            }
         }

         readShort(marker, str);
        
      }
   }
   else
   {
      if (traceDebug())
      {
         out << MODULE << " Cannot open file:  " << theFile << std::endl;
      }
   }

   return out;
}

void rspfJ2kInfo::readShort(rspf_uint16& s, std::ifstream& str) const
{
   str.read((char*)&s, sizeof(s));
   if (theEndian)
   {
      theEndian->swap(s);
   }
}

std::ostream& rspfJ2kInfo::printCodMarker(std::ostream& out,
                                           const std::string& prefix,
                                           std::ifstream& str) const
{
   rspfJ2kCodRecord siz;
   siz.parseStream(str);
   siz.print(out, prefix);
   return out;
}

std::ostream& rspfJ2kInfo::printSizMarker(std::ostream& out,
                                           const std::string& prefix,
                                           std::ifstream& str) const
{
   rspfJ2kSizRecord siz;
   siz.parseStream(str);
   siz.print(out, prefix);
   return out;
}

std::ostream& rspfJ2kInfo::printSotMarker(std::ostream& out,
                                           const std::string& prefix,
                                           std::ifstream& str) const
{
   // Get the stream posistion.
   std::streamoff pos = str.tellg();
   
   rspfJ2kSotRecord sot;
   sot.parseStream(str);
   pos += sot.thePsot - 2;

   // Seek past the tile to the next marker.
   str.seekg(pos, std::ios_base::beg);
   
   sot.print(out,prefix);
   return out;
}


std::ostream& rspfJ2kInfo::printUnknownMarker(std::ostream& out,
                                               const std::string& prefix,
                                               std::ifstream& str,
                                               rspf_uint16 marker) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();

   rspf_uint16 segmentLength;
   readShort(segmentLength, str);

   std::string pfx = prefix;
   pfx += "unkown.";

   out.setf(std::ios_base::hex, std::ios_base::basefield);
   out << pfx << "marker: 0x" << std::setfill('0') << std::setw(4)
       << marker << "\n";
   out.setf(std::ios_base::fmtflags(0), std::ios_base::basefield);
   
   out << pfx << "length: " << segmentLength
       << std::endl;

   // Reset flags.
   out.setf(f);

   // Seek to the next marker.
   str.seekg( (segmentLength-2), std::ios_base::cur);

   return out;
}
      

