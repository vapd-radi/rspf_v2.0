//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Container class for J2K "Start Of Tile" (SOT) record.
//
// See document BPJ2K01.00 Table 7-3 Image and tile size (15444-1 Annex A.4.2)
// 
//----------------------------------------------------------------------------
// $Id: rspfJ2kSotRecord.h,v 1.5 2005/10/13 21:24:47 dburken Exp $
#ifndef rspfJ2kSotRecord_HEADER
#define rspfJ2kSotRecord_HEADER

#include <iosfwd>
#include <string>

#include <rspf/base/rspfConstants.h>

class RSPF_DLL rspfJ2kSotRecord
{
public:
   
   /** default constructor */
   rspfJ2kSotRecord();

   /** destructor */
   ~rspfJ2kSotRecord();

   /**
    * Parse method.  Performs byte swapping as needed.
    *
    * @param in Stream to parse.
    *
    * @note Marker is not read.
    */
   void parseStream(std::istream& in);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;
   
   /**
    * operator<<.
    */
   friend RSPF_DLL std::ostream& operator<<(
      std::ostream& out, const rspfJ2kSotRecord& obj);

   /** Start of tile-part marker code. 0xff90 */
   rspf_uint16 theMarker;
   
   /** Length in bytes of the marker segment. */
   rspf_uint16 theLsot;

   /** Tile index. Tiles are in raster order starting at 0. */
   rspf_uint16 theIsot;

   /** The length in bytes of this record including the SOT marker. */
   rspf_uint32 thePsot;

   /** Tile-Part index. */
   rspf_uint8  theTpsot;

   /**
    * 0 = Number of tile-parts of this tile in the codestream is not defined
    * in this header.
    *
    * 1-255 number of tile-parts of this tile in the codestream.
    */
   rspf_uint8  theTnsot;
};

#endif /* End of "#ifndef rspfJ2kSotRecord_HEADER" */
