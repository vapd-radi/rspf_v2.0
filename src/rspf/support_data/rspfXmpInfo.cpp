//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Extensible Metadata Platform (XMP) Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/support_data/rspfXmpInfo.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// Static trace for debugging.
static const rspfTrace traceDebug( rspfString("rspfXmpInfo:debug") );

rspfXmpInfo::rspfXmpInfo()
   : rspfInfoBase(),
     m_file(),
     m_xmpApp1XmlBlock()
{
}

rspfXmpInfo::~rspfXmpInfo()
{
}

bool rspfXmpInfo::open(const rspfFilename& file)
{
   //---
   // NOTE:
   // This parser was written for a specific aerial camera.  If something more generic is
   // needed please provide sample to the group. D. Burken, 17 January 2013.
   //---
   
   bool result = false;

   //---
   // Open the file.
   //---
   std::ifstream str(file.c_str(), std::ios_base::binary|std::ios_base::in);
   if ( str.good() ) 
   {
      // Parse for XMP APP1 XML block. Currently only case.
      std::string xmlString;
      result = getXmpApp1XmlBlock( str, xmlString );
      if ( result )
      {
         m_file = file;
         m_xmpApp1XmlBlock = xmlString;
      }
      else
      {
         m_file.clear();
         m_xmpApp1XmlBlock.clear();
      }
   }

   return result;
}

bool rspfXmpInfo::getXmpApp1XmlBlock( std::ifstream& str, std::string& xmpApp1XmlBlock ) const
{
   bool result = false;

   if ( str.good() )
   {
      rspf_uint8 c;
      
      // Look for Start Of Image (SOI) marker 0xFFD8:
      str.read( (char*)&c, 1);
      if ( c == 0xff )
      {
         str.read( (char*)&c, 1);
         if ( c == 0xd8 )
         {
            // Found SOI marker.
            
            // Now find APP1 marker 0xffe1.  Only do 24 iterations max:
            for ( rspf_uint32 i = 0; i < 24; ++i )
            {
               str.read( (char*)&c, 1);
               if ( c == 0xff )
               {
                  str.read( (char*)&c, 1);
                  if ( c == 0xe1 )
                  {
                     // Found APP1 marker.
                     
                     //---
                     // Next two bytes are the length, MSB first (big endian) per jpeg spec.
                     // length = 2 + 29 + length_of_xmp_packet
                     //---
                     rspf_uint16 xpacketLength = 0;
                     str.read( (char*)&xpacketLength, 2 );
                     
                     // See if system is not big endian and swap if needed.
                     if ( rspf::byteOrder() != RSPF_BIG_ENDIAN )
                     {
                        rspfEndian swapper;
                        swapper.swap( xpacketLength );
                     }
                     
                     if ( xpacketLength > 31 )
                     {
                        //---
                        // XMP namepsace URI, used as unique ID:
                        // Null terminated ascii string: "http://ns.adobe.com/xap/1.0/"
                        // (29 bytes)
                        // Not using getline until ID verified.
                        //---
                        const rspf_uint32 XMP_ID_SIZE = 29;
                        std::vector<char> v( XMP_ID_SIZE );
                        
                        // Read the next 29 bytes.  (string + null)
                        str.read( &v.front(), XMP_ID_SIZE );
                        
                        // Copy.
                        v.push_back( '\0' ); // Just in case string is bad.
                        std::string s = &v.front();
                        
                        if ( s == "http://ns.adobe.com/xap/1.0/" )
                        {
                           //---
                           // Now read header to determine encoding:
                           // Note: Currently hard coded for UTF-8.
                           //---
                           
                           //---
                           // Look for "<?xpacket begin="
                           // Line has binary data in it or getline could be used.
                           //---
                           const rspf_uint32 HEADER_SIZE = 16;
                           v.resize( HEADER_SIZE );
                           str.read( &v.front(), HEADER_SIZE );
                           
                           v.push_back( '\0' ); // Null terminate.
                           s = &v.front();
                           
                           if ( s == "<?xpacket begin=" )
                           {
                              // Skip the quote which can be single or double.
                              str.seekg( 1, std::ios_base::cur );
                              
                              // Now look for 0xEF, 0xBB, 0xBF which is UTF-8
                              rspf_uint8 encode_bytes[3];
                              str.read( (char*)encode_bytes, 3 );
                              
                              if ( ( encode_bytes[0] == 0xef ) &&
                                   ( encode_bytes[1] == 0xbb ) &&
                                   ( encode_bytes[2] == 0xbf ) )
                              {
                                 // Skip the next six bytes: "' id='"
                                 str.seekg( 6, std::ios_base::cur );
                                 
                                 // Read the ID: W5M0MpCehiHzreSzNTczkc9d
                                 const rspf_uint32 XPACKET_ID_SIZE = 24;
                                 v.resize( XPACKET_ID_SIZE );
                                 str.read ( &v.front(), XPACKET_ID_SIZE );
                                 
                                 v.push_back( '\0' ); // null terminate
                                 s = &v.front();
                                 
                                 if ( s == "W5M0MpCehiHzreSzNTczkc9d" )
                                 {
                                    //---
                                    // Gobble the rest of the line. No more binary characters
                                    // so just use getline.
                                    //---
                                    std::getline( str, s );
                                    
                                    // Read in the xml string (single line):
                                    std::getline( str, xmpApp1XmlBlock );
                                    
                                    if ( xmpApp1XmlBlock.size() )
                                    {
                                       result = true;
                                    }
                                 }
                              }
                              
                           } // Matches: if ( s == "<?xpacket begin=" )
                           
                        } // Matches: if ( s == "http://ns.adobe.com/xap/1.0/" )
                        
                     } // Matches: if ( xpacketLength > 31 )
                  
                  } // Matches: if ( c == 0xd8 )
                  
               } // Matches: if ( c == 0xff )
               
            } // Matches: for ( rspf_uint32 i; i < 24; ++i )
            
         } // Matches: if ( c == 0xd8 )
         
      } // Matches: if ( c == 0xff )
      
   } // Matches: if ( str.good() ) 

   return result;
   
} // End: rspfXmpInfo::getXmpApp1XmlBlock

std::ostream& rspfXmpInfo::print(std::ostream& out) const
{
   if ( getXmpApp1XmlBlock().size() )
   {
      std::string prefix = "";
      std::string ext = m_file.ext().downcase().string();
      if ( ( ext == ".jpg" ) || ( ext == ".jpeg" ) )
      {
         prefix = "jpeg.";
      }
      out << prefix << "xmp.app1.xml: " << getXmpApp1XmlBlock() << std::endl;
   }
   return out;
}

const std::string& rspfXmpInfo::getXmpApp1XmlBlock() const
{
   return m_xmpApp1XmlBlock;
}

rspfRefPtr<rspfProjection> rspfXmpInfo::getProjection( const rspfDrect& imageRect ) const
{
   rspfRefPtr<rspfProjection> result = 0;

   if ( getXmpApp1XmlBlock().size() && ( imageRect.hasNans() == false ) )
   {
      // Open a stream to the xml block.
      std::istringstream is;
      is.str( m_xmpApp1XmlBlock );

      rspfXmlDocument xmlDoc;

      // Read the xml document:
      if ( xmlDoc.read( is ) )
      {
         // Find the four corner ground points.
         rspfString s;
         rspfString path;
         rspfGpt ulg;
         rspfGpt urg;
         rspfGpt lrg;
         rspfGpt llg;
         bool foundAll = false;

         while ( 1 )
         {
            // Upper left:
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:UL_Latitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               ulg.lat = s.toFloat64();
            }
            else
            {
               break;
            }
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:UL_Longitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               ulg.lon = s.toFloat64();
            }
            else
            {
               break;
            }

            // Upper right:
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:UR_Latitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               urg.lat = s.toFloat64();
            }
            else
            {
               break;
            }
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:UR_Longitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               urg.lon = s.toFloat64();
            }
            else
            {
               break;
            }
            
            // Lower right:
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:LR_Latitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               lrg.lat = s.toFloat64();
            }
            else
            {
               break;
            }
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:LR_Longitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               lrg.lon = s.toFloat64();
            }
            else
            {
               break;
            }
            
            // Lower left:
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:LL_Latitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               llg.lat = s.toFloat64();
            }
            else
            {
               break;
            }
            path = "/x:xmpmeta/rdf:RDF/rdf:Description/dc:LL_Longitude";
            if ( getPath( path, xmlDoc, s ) )
            {
               llg.lon = s.toFloat64();

               // If we get here mark the status good:
               foundAll = true;
            }
            else
            {
               break;
            }
            

            //---
            // Final trailing break out of infinite loop.
            // No code past this point if while loop.
            //---
            break;
         }

         if ( foundAll )
         {
            result = new rspfBilinearProjection( imageRect.ul(),
                                                  imageRect.ur(),
                                                  imageRect.lr(),
                                                  imageRect.ll(),
                                                  ulg,
                                                  urg,
                                                  lrg,
                                                  llg );
         }
         
      } // Matches: if ( xmlDoc.read( is ) )
      
   } // Matches: if ( getXmpApp1XmlBlock().size() )
   
   return result;
   
} // End: rspfXmpInfo::getProjection


bool rspfXmpInfo::getPath(
   const rspfString& path, const rspfXmlDocument& xdoc, rspfString& s) const
{
   bool result = false;

   std::vector<rspfRefPtr<rspfXmlNode> > xnodes;
   xdoc.findNodes(path, xnodes);
   if ( xnodes.size() == 1 ) // Error if more than one.
   {
      if ( xnodes[0].valid() )
      {
         s = xnodes[0]->getText();
         result = true;
      }
      else if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfXmpInfo::getPath ERROR:\n"
            << "Node not found: " << path
            << std::endl;
      }
   }
   else if ( xnodes.size() == 0 )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfXmpInfo::getPath ERROR:\n"
            << "Node not found: " << path
            << std::endl;
      }
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfXmpInfo::getPath ERROR:\n"
            << "Multiple nodes found: " << path
            << std::endl;
      }
   }

   if (!result)
   {
      s.clear();
   }
   return result;
   
} // bool rspfXmpInfo::getPath

