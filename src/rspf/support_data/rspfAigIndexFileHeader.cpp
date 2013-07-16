//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts  (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id: rspfAigIndexFileHeader.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/support_data/rspfAigIndexFileHeader.h>
#include <rspf/base/rspfEndian.h>

bool rspfAigIndexFileHeader::writeStream(std::ostream& out)
{
   rspfEndian endian;
   rspf_uint32 tempInt;
   
   if(endian.getSystemEndianType() == RSPF_LITTLE_ENDIAN)
   {
      out.write((char*)theMagicNumber, 8);
      out.write((char*)theZeroFill1, 16);
      tempInt = theFileSize;
      endian.swap(tempInt);
      out.write((char*)(&tempInt), 4);
      out.write((char*)theZeroFill2, 72);
   }
   else
   {
      out.write((char*)(&theFileSize), 8);
   }
   
   return out.good();
}
