//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Container class for J2K Coding style default (COD) record.
//
// See document BPJ2K01.00 Table 7-7 Image and tile size (15444-1 Annex A5.1)
// 
//----------------------------------------------------------------------------
// $Id: rspfJ2kCodRecord.h,v 1.5 2005/10/13 21:24:47 dburken Exp $
#ifndef rspfJ2kCodRecord_HEADER
#define rspfJ2kCodRecord_HEADER

#include <iosfwd>
#include <string>

#include <rspf/base/rspfConstants.h>

class RSPF_DLL rspfJ2kCodRecord
{
public:
   
   /** default constructor */
   rspfJ2kCodRecord();

   /** destructor */
   ~rspfJ2kCodRecord();

   /**
    * Parse method.  Performs byte swapping as needed.
    *
    * @param in Stream to parse.
    *
    * @note COD Marker (0xff52) is not read.
    */
   void parseStream(std::istream& in);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "j2k." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;

   /** operator<< */
   friend RSPF_DLL std::ostream& operator<<(
      std::ostream& out, const rspfJ2kCodRecord& obj);

   /** segmet marker 0xff52 (big endian) */
   rspf_uint16 theMarker;

   /** length of segment minus marker */
   rspf_uint16 theLcod;

   /** Coding style */
   rspf_uint8 theScod;

   /** Progression order, Number of layers, Multiple component transform. */
   rspf_uint32 theSGcod;

   /** Code-block style */
   rspf_uint8 theSPcod;

};

#endif /* End of "#ifndef rspfJ2kCodRecord_HEADER" */
