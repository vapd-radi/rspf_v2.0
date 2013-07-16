//*******************************************************************
// Copyright (C) 2004 Garrett Potts.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class definition for rspfPackBits.
//*******************************************************************
//  $Id: rspfPackedBits.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/base/rspfPackedBits.h>
#include <iostream>

rspfPackedBits::rspfPackedBits(const rspf_uint8* packedBits,
                                 rspf_uint16 bits)
      :thePackedBits(packedBits),
       theBits(bits)
{
   theBytes    = theBits>>3;
   theBitsLeft = theBits%8;
}

rspf_uint32 rspfPackedBits::getValueAsUint32(rspf_uint32 idx)const
{
   rspf_uint32 value        = 0;
   rspf_uint32 tempBits     = (theBitsLeft*idx);
   rspf_uint32 tempBitShift = tempBits%8;
   rspf_uint32 byteOffset   = ( (idx*theBytes) + (tempBits>>3) );
   rspf_uint32 bitCount     = 0;
   rspf_uint32 bitsSeen     = tempBitShift;
   rspf_uint8 bitmask       = 1 << (7-tempBitShift);
   
   while(bitCount < theBits)
   {
      value |= (rspf_uint8)((bool)(thePackedBits[byteOffset]&bitmask));
      ++bitCount;
      ++bitsSeen;
      if(bitsSeen >= 8)
      {
         bitsSeen = 0;
         bitmask = 0x80; // binary pattern 10000000
         ++byteOffset;
      }
      else
      {
         bitmask>>=1;
      }
      if(bitCount < theBits)
      {
         value <<=1;
      }
   }
   
   return value;
}
