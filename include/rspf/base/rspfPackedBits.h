//*******************************************************************
// Copyright (C) 2004 Garrett Potts.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for rspfPackBits.
// 
//*******************************************************************
//  $Id: rspfPackedBits.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfPackedBits_HEADER
#define rspfPackedBits_HEADER
#include <rspf/base/rspfConstants.h>

/**
 * This class will allow access to individual packed pixels given an offset and
 * return as an rspf_uint32 type.
 */
class RSPF_DLL rspfPackedBits
{
public:
   /**
    * @brief Constructor that takes packed bits array and word size in bits.
    *
    * @param packedBits The unsigned eight bit array of packed bits.  This
    * pointer is not owned by this class, only used.
    *
    * @param bits The number of bits in the packed bits word.
    */
   rspfPackedBits(const rspf_uint8* packedBits, rspf_uint16 bits);
   
   /**
    * Extract out the value at offset indicated by idx and convert the
    * the vale to an rspf_uint32.  This class only supports packed bits
    * less than 32.
    *
    * @param idx The packed bit word index to grab and stuff into an
    * unsigned 32 bit integer.
    */
   rspf_uint32 getValueAsUint32(rspf_uint32 idx)const;
   
protected:
   const rspf_uint8* thePackedBits;
   rspf_uint16 theBits;
   rspf_uint16 theBytes;
   rspf_uint16 theBitsLeft;
};

#endif
