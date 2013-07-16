//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: JP2 Info object.
// 
//----------------------------------------------------------------------------
// $Id: rspfJp2Info.cpp 20504 2012-01-27 15:34:21Z dburken $

#include <rspf/support_data/rspfJp2Info.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/support_data/rspfTiffInfo.h>
#include <fstream>
#include <iostream>

rspfJp2Info::rspfJp2Info()
   : rspfInfoBase(),
     m_file()
{
}

rspfJp2Info::~rspfJp2Info()
{
}

bool rspfJp2Info::open(const rspfFilename& file)
{
   bool result = false;

   std::ifstream str;
   str.open( file.c_str(), std::ios_base::in | std::ios_base::binary);

   if ( str.is_open() )
   {
      const rspf_uint8 J2K_SIGNATURE_BOX[SIGNATURE_BOX_SIZE] = 
         {0x00,0x00,0x00,0x0c,0x6a,0x50,0x20,0x20,0x0d,0x0a,0x87,0x0a};
      
      rspf_uint8 box[SIGNATURE_BOX_SIZE];
      
      // Read in the box.
      str.read((char*)box, SIGNATURE_BOX_SIZE);
      
      result = true;
      for (rspf_uint32 i = 0; i < SIGNATURE_BOX_SIZE; ++i)
      {
         if (box[i] != J2K_SIGNATURE_BOX[i])
         {
            result = false;
            break;
         }
      }
      str.close();
   }

   if ( result ) m_file = file; // Capture file name.

   return result;
}

std::ostream& rspfJp2Info::print(std::ostream& out) const
{
   if ( m_file.size() )
   {
      // Look for the GeoTIFF Box
      std::ifstream str;
      str.open( m_file.c_str(), std::ios_base::in | std::ios_base::binary);
      
      if ( str.is_open() )
      {
         // Position to start of code file.
         str.seekg(SIGNATURE_BOX_SIZE, std::ios_base::beg);
         
         // See if we need to byte swap.  JP2 boxes are big endian.
         rspfEndian* swapper = 0;
         if ( rspf::byteOrder() == RSPF_LITTLE_ENDIAN )
         {
            swapper = new rspfEndian();
         }
         
         rspf_uint32 lbox  = 0;
         rspf_uint32 tbox  = 0;
         rspf_uint64 xlbox = 0;
         
         const rspf_uint32 UUID_TYPE = 0x75756964;
         
         const rspf_uint8 GEOTIFF_UUID[GEOTIFF_UUID_SIZE] = 
            {
               0xb1, 0x4b, 0xf8, 0xbd,
               0x08, 0x3d, 0x4b, 0x43,
               0xa5, 0xae, 0x8c, 0xd7,
               0xd5, 0xa6, 0xce, 0x03
            };
         
         while ( str.good() )
         {
            str.read((char*)&lbox, 4);
            str.read((char*)&tbox, 4);
            rspf_uint32 bytesRead = 8;
            if (swapper)
            {
               swapper->swap(lbox);
               swapper->swap(tbox);
            }
            
            if (tbox == UUID_TYPE)
            {
               // Check for GeoTIFF box.
               
               rspf_uint8 geotiffUuid[GEOTIFF_UUID_SIZE];
               
               // Read in the id.
               str.read((char*)geotiffUuid, GEOTIFF_UUID_SIZE);
               bytesRead += GEOTIFF_UUID_SIZE;
               
               bool foundIt = true;
               for (rspf_uint32 i = 0; i < GEOTIFF_UUID_SIZE; ++i)
               {
                  if (geotiffUuid[i] != GEOTIFF_UUID[i])
                  {
                     foundIt = false;
                     break;
                  }
               }
               
               if ( foundIt )
               {
                  //---
                  // Feed the stream to the tiff info object to get a keyword
                  // list with geometry/projection info.
                  //---
                  rspfTiffInfo info;
                  
                  //---
                  // Have geotiff boxes with badly terminated geotiffs. So to keep
                  // the tag parser from walking past the first image file directory
                  // (IFD) into garbage we will set the process overview flag to false.
                  //
                  // Note if we ever get multiple entries we will need to take this out.
                  //---
                  info.setProcessOverviewFlag(false);

                  // Do a print to a memory stream.
                  std::ostringstream os;
                  info.print(str, os);

                  // Open an input stream to pass to the keyword list.
                  std::istringstream in( os.str() );
                  
                  rspfKeywordlist kwl;
                  if ( kwl.parseStream(in) )      
                  {
                     kwl.addPrefixToAll( rspfString("jp2.") );

                     // Print it:
                     out << kwl << std::endl;
                  }
                  
                  break; // Get out of while loop.
               }
            }
            else
            {
               str.read((char*)&xlbox, 8);
               bytesRead += 8;
               if (swapper)
               {
                  swapper->swap(xlbox);
               }
            }
            
            if (lbox == 0) // last box?
            {
               break;
            }
            
            // Seek to the next box.  If xlbox is 1
            std::streamoff seekPos = 0;
            if (lbox == 1)
            {
               seekPos = static_cast<std::streamoff>(xlbox) - static_cast<std::streamoff>(bytesRead);
            }
            else
            {
               seekPos = static_cast<std::streamoff>(lbox) - static_cast<std::streamoff>(bytesRead);
            }
            str.seekg(seekPos,  std::ios_base::cur);
            
         } // matches: while ( str.good() )
         
         // cleanup
         if (swapper)
         {
            delete swapper; 
            swapper = 0;
         }
         
         // Close the stream.
         str.close();
         
      } // matches: if ( str.is_open() )
      
   } // matches: if ( m_file.size() )
   
   return out; 
}
