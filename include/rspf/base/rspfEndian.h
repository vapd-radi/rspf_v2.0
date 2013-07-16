//******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: This file contains the interface to the
//              endian byte swap
//
//***********************************
// $ID$
#ifndef rspfEndian_HEADER
#define rspfEndian_HEADER

// for RSPF_LITTLE_ENDIAN AND BIG_ENDIAN
#include <rspf/base/rspfConstants.h>

class RSPFDLLEXPORT rspfEndian
{
public:
   rspfEndian();

   /*!
    *  Swap bytes methods that work on a single pixel.
    */
   inline void swap(rspf_sint8    &/*data*/){}
   inline void swap(rspf_uint8    &/*data*/){}
   inline void swap(rspf_int16   &data) const;
   inline void swap(rspf_uint16  &data) const;
   inline void swap(rspf_int32   &data) const;
   inline void swap(rspf_uint32  &data) const;
   inline void swap(rspf_uint64  &data) const;
   inline void swap(rspf_sint64  &data) const;
   inline void swap(rspf_float32 &data) const;
   inline void swap(rspf_float64 &data) const;

   /*!
    *  Swap bytes methods that work on arrays of pixels.
    *
    *  @note size is number of pixels, not number of bytes.
    */
   inline void swap(rspfScalarType scalar, void* data, rspf_uint32 size) const;
   // only here to allow template based swaps to compile correctly
   //
   inline void swap(rspf_sint8* data, rspf_uint32 size)const;
   inline void swap(rspf_uint8* data, rspf_uint32 size)const;
   
   inline void swap(rspf_int16* data, rspf_uint32 size) const;
   inline void swap(rspf_uint16* data, rspf_uint32 size) const;

   inline void swap(rspf_int32* data, rspf_uint32 size) const;
   inline void swap(rspf_uint32* data, rspf_uint32 size) const;

//#ifdef HAS_LONG_LONG
   inline void swap(rspf_int64* data, rspf_uint32 size) const;
   inline void swap(rspf_uint64* data, rspf_uint32 size) const;
//#endif

   inline void swap(rspf_float32* data, rspf_uint32 size) const;
   inline void swap(rspf_float64* data, rspf_uint32 size) const;

   inline void swapTwoBytes(void* data, rspf_uint32 size) const;
   inline void swapFourBytes(void* data, rspf_uint32 size) const;
   inline void swapEightBytes(void* data, rspf_uint32 size) const;
   inline rspfByteOrder getSystemEndianType() const;

private:

   // Holds the Endian of the architecture that you are running on.
   rspfByteOrder  theSystemEndianType;  
   
   void swapTwoBytes(void *data) const;
   void swapFourBytes(void *data) const;
   void swapEightBytes(void *data) const;

   void swapPrivate(rspf_uint8 *c1,
                    rspf_uint8 *c2) const;
};

inline rspfByteOrder rspfEndian::getSystemEndianType() const
{
   return theSystemEndianType;
}

inline void rspfEndian::swap(rspf_sint8* /* data */,
                              rspf_uint32 /* size */ )const
{
   //intentionally left blank
}

inline void rspfEndian::swap(rspf_uint8* /* data */,
                              rspf_uint32 /* size */ )const
{
   //intentionally left blank
}

inline void rspfEndian::swap(rspf_int16 &data) const
{
   swapTwoBytes(reinterpret_cast<void*>(&data));   
} 

inline void rspfEndian::swap(rspf_uint16 &data) const
{
   swapTwoBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swap(rspf_int32 &data) const
{
   swapFourBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swap(rspf_uint32 &data) const
{
   swapFourBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swap(rspf_uint64 &data) const
{
   swapEightBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swap(rspf_sint64 &data) const
{
   swapEightBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swap(rspf_float32 &data) const
{
   swapFourBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swap(rspf_float64 &data) const
{
   swapEightBytes(reinterpret_cast<void*>(&data));
}

inline void rspfEndian::swapTwoBytes(void *data) const
{
   unsigned char *c = reinterpret_cast<unsigned char*>(data);

   swapPrivate(&c[0], &c[1]);
}

inline void rspfEndian::swapFourBytes(void* data) const
{
   unsigned char *c = reinterpret_cast<unsigned char*>(data);

   swapPrivate(&c[0], &c[3]);
   swapPrivate(&c[1], &c[2]);
}

inline void rspfEndian::swapEightBytes(void* data) const
{
   unsigned char *c = reinterpret_cast<unsigned char*>(data);

   swapPrivate(&c[0], &c[7]);
   swapPrivate(&c[1], &c[6]);
   swapPrivate(&c[2], &c[5]);
   swapPrivate(&c[3], &c[4]);
}

inline void rspfEndian::swapPrivate(rspf_uint8 *c1,                       
                                     rspf_uint8 *c2) const
{
   rspf_uint8 temp_c = *c1;
   *c1 = *c2;
   *c2 = temp_c;
}

inline void rspfEndian::swap(rspfScalarType scalar,
                              void* data, rspf_uint32 size) const
{
   switch (scalar)
   {
      case RSPF_USHORT16:
      case RSPF_SSHORT16:
      case  RSPF_USHORT11:
         swapTwoBytes(data, size);
         return;
         
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
         swapFourBytes(data, size);
         return;
         
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
         swapEightBytes(data, size);
         break;
         
      default:
         return;
   }
}

inline void rspfEndian::swap(rspf_int16* data, rspf_uint32 size) const
{
   swapTwoBytes(data, size);
}

inline void rspfEndian::swap(rspf_uint16* data, rspf_uint32 size) const
{
   swapTwoBytes(data, size);
}

inline void rspfEndian::swap(rspf_int32* data, rspf_uint32 size) const
{
   swapFourBytes(data, size);
}

inline void rspfEndian::swap(rspf_uint32* data, rspf_uint32 size) const
{
   swapFourBytes(data, size);
}

#ifdef HAS_LONG_LONG

inline void rspfEndian::swap(rspf_int64* data, rspf_uint32 size) const
{
   swapEightBytes(data, size);
}

inline void rspfEndian::swap(rspf_uint64* data, rspf_uint32 size) const
{
   swapEightBytes(data, size);
}

#endif /* End of #ifdef HAS_LONG_LONG */

inline void rspfEndian::swap(rspf_float32* data, rspf_uint32 size) const
{
   swapFourBytes(data, size);
}

inline void rspfEndian::swap(rspf_float64* data, rspf_uint32 size) const
{
   swapEightBytes(data, size);
}

inline void rspfEndian::swapTwoBytes(void* data, rspf_uint32 size) const
{
   rspf_uint16* buf = reinterpret_cast<rspf_uint16*>(data);
   for (rspf_uint32 i=0; i<size; ++i)
   {
      buf[i] = ((buf[i] & 0x00ff) << 8) | ((buf[i] & 0xff00) >> 8);
   }
}

inline void rspfEndian::swapFourBytes(void* data, rspf_uint32 size) const
{
   rspf_uint32* buf = reinterpret_cast<rspf_uint32*>(data);
   for (rspf_uint32 i=0; i<size; ++i)
   {
      buf[i]
         = (  ((buf[i] & 0xff000000) >> 24)
            | ((buf[i] & 0x00ff0000) >> 8)
            | ((buf[i] & 0x0000ff00) << 8)
            | ((buf[i] & 0x000000ff) << 24));
   }
}

inline void rspfEndian::swapEightBytes(void* data, rspf_uint32 size) const
{
#ifdef HAS_LONG_LONG
   uint64* buf = reinterpret_cast<rspf_uint64*>(data);
   for (uint32 i=0; i<size; ++i)
   {
      buf[i]
         = (  ((buf[i] & 0xff00000000000000ull) >> 56)
            | ((buf[i] & 0x00ff000000000000ull) >> 40)
            | ((buf[i] & 0x0000ff0000000000ull) >> 24)
            | ((buf[i] & 0x000000ff00000000ull) >> 8)
            | ((buf[i] & 0x00000000ff000000ull) << 8)
            | ((buf[i] & 0x0000000000ff0000ull) << 24)
            | ((buf[i] & 0x000000000000ff00ull) << 40)
            | ((buf[i] & 0x00000000000000ffull) << 56));
   }
#else
   rspf_float64* buf = reinterpret_cast<rspf_float64*>(data);
   for (rspf_uint32 i=0; i<size; ++i)
   {
      swapEightBytes(buf+i);
   }
   
#endif
}

#endif /* End of #ifndef rspfEndian_HEADER */
