//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Utility class to convert to/from EBCDIC/ASCII
//
// ASCII = American National Standard Code for Information Interchange
//
// EBCDIC = Extended Binary Coded Decimal Interchange Code  
//
// $Id: rspfEbcdicToAscii.h 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfEbcdicToAscii_HEADER
#define rspfEbcdicToAscii_HEADER

#include <rspf/base/rspfConstants.h>

class RSPF_DLL rspfEbcdicToAscii
{
public:

   /** default constructor */
   rspfEbcdicToAscii();

   /** destructor */
   ~rspfEbcdicToAscii();
   
   /**
    * @brief Converts ascii character c to ebcdic character.
    *
    * @param c ascii character to convert.
    *
    * @return ebcdic character.
    */
   rspf_uint8 asciiToEbcdic(rspf_uint8 c) const;

   /**
    * @brief Converts ebcdic character c to ascii character.
    *
    * @param c ebcdic character to convert.
    *
    * @return ascii character.
    */
   rspf_uint8 ebcdicToAscii(rspf_uint8 c) const;
   
   /**
    * @brief Converts ebcdic character string "str" to ascii characters.
    *
    * @param str ebcdic character string to convert.
    *
    * @param size Number of characters to convert.  Note that "str" should be
    * at least >= size.
    */
   void ebcdicToAscii(char* str, rspf_uint32 size) const;
};

#endif /* rspfEbcdicToAcsii_HEADER */
