//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: rspf-foo.cpp
//
// Author:  Garrett Potts
//
// Description: This is a basic_streambuf that can be used for binary in memory streams
//
//
// $Id$
//----------------------------------------------------------------------------
#ifndef rspfByteStreamBuffer_HEADER
#define rspfByteStreamBuffer_HEADER
#include <rspf/base/rspfConstants.h>
#include <cstdio> /* for EOF */
#include <streambuf>
#include <iosfwd>
#include <ios>

//using namespace std;
class RSPF_DLL rspfByteStreamBuffer : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
   rspfByteStreamBuffer();
    
   rspfByteStreamBuffer(char_type* buf, rspf_int64 bufSize, bool shared=true);
    
   rspfByteStreamBuffer(const rspfByteStreamBuffer& src);
   virtual ~rspfByteStreamBuffer();
   virtual std::streambuf* setbuf ( char_type * s, std::streamsize n );
   void clear();
   // added so we can set a buffer and make it shared
   std::streambuf* setBuf(char* buf, std::streamsize bufSize, bool shared);
   virtual int overflow( int c = EOF);
   /**
    * Returns a pointer to the buffer
    */
   char_type* buffer();
   const char_type* buffer()const;
    
   /**
    * The buffer is no longer managed by this stream buffer and is removed.
    */
   char_type* takeBuffer();
   rspf_uint64 bufferSize()const;
    
protected:
   virtual int_type pbackfail(int_type __c  = traits_type::eof());
   virtual pos_type seekoff(off_type offset, std::ios_base::seekdir dir,
                            std::ios_base::openmode __mode = std::ios_base::in | std::ios_base::out);
   virtual pos_type seekpos(pos_type pos, std::ios_base::openmode __mode = std::ios_base::in | std::ios_base::out);
   virtual std::streamsize xsgetn(char_type* __s, std::streamsize __n);
   virtual std::streamsize xsputn(const char_type* __s, std::streamsize __n);
    
    
   void deleteBuffer();
   void extendBuffer(rspf_uint64 bytes);

   char_type* m_buffer;
   rspf_int64 m_bufferSize;
   bool m_sharedBuffer;
};

#endif