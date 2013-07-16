//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts  (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id: rspfAigIndexFileHeader.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAigIndexFileHeader_HEADER
#define rspfAigIndexFileHeader_HEADER
#include <iostream>
#include <rspf/base/rspfConstants.h>
#include <string.h>

class rspfAigIndexFileHeader
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfAigIndexFileHeader& data)
      {
         out << "theFileSize: " << data.theFileSize;
         
         return out;
      }
   rspfAigIndexFileHeader()
      {
         reset();
      }
   void reset()
      {
         theMagicNumber[0] = 0x00;
         theMagicNumber[1] = 0x00;
         theMagicNumber[2] = 0x27;
         theMagicNumber[3] = 0x0A;
         theMagicNumber[4] = 0xFF;
         theMagicNumber[5] = 0xFF;
         theMagicNumber[6] = 0xFC;
         theMagicNumber[7] = 0x14;
         memset((char*)theZeroFill1,
                0x00,
                16);
         memset((char*)theZeroFill2,
                0x00,
                72);
         theFileSize = 50;
      }
   bool writeStream(std::ostream& out);

   rspf_uint8 theMagicNumber[8];

   rspf_uint8 theZeroFill1[16];
   /*!
    *  The size of this file in number of shorts.
    */
   rspf_int32 theFileSize;

   rspf_uint8 theZeroFill2[72];


   /*!
    *  The next two fields that follow are replicated the number
    * of tiles in the file.  These two fields holds the offset of the tile
    * in the data file and the size of the tile in the data file( both are in
    * shorts and not bytes);
    */
// 100 + t*8 
//           4 
//                     MSB Int32 
//                              Offset to tile t in w001001.adf measured in two byte
//                              shorts. 
// 104 + t*8 
//           4 
//                     MSB Int32 
//                              Size of tile t in 2 byte shorts.   
};

#endif
