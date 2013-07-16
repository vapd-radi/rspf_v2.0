// ============================================================================
// gzstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : gzstream.h
// Revision      : $Revision: 13050 $
// Revision_date : $Date: 2008-06-20 02:07:35 +0800 (周五, 2008-06-20) $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

#ifndef rspfGzStream_HEADER
#define rspfGzStream_HEADER

// standard C++ with new header file names and std:: namespace
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/rspfConfig.h>
#include <iostream>
#include <fstream>
#include <rspf/base/rspfIoStream.h>

#if RSPF_HAS_LIBZ
// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See below for user classes.
// ----------------------------------------------------------------------------

class RSPF_DLL rspfGzStreamBuf : public std::streambuf
{

public:
   rspfGzStreamBuf();

   virtual ~rspfGzStreamBuf();
   
   bool is_open()const;
   rspfGzStreamBuf* open( const char* name, int open_mode);
   rspfGzStreamBuf* close();
      
   virtual int overflow( int c = EOF);
   
   // will not use  buffer for get.
   virtual std::streamsize xsgetn(char_type* __s, std::streamsize n);
   virtual int     underflow();
   virtual int     sync();
   virtual pos_type seekoff(off_type t, std::ios_base::seekdir dir,
                            std::ios_base::openmode omode = std::ios_base::in |
                            std::ios_base::out);
/*     virtual pos_type seekpos(pos_type posType,  */
/*                              std::ios_base::openmode __mode = std::ios_base::in | */
/*                              std::ios_base::out); */
   
private:
	struct PrivateData;
   int flush_buffer();

   static const int bufferSize = 303; // 47+256 size of data buff
   // totals 512 bytes under g++ for igzstream at the end.
   PrivateData*     prvtData;
   char             buffer[bufferSize]; // data buffer
   bool             opened;             // open/close state of stream
   int              mode;               // I/O mode

}; // End of class rspfGzStreamBuf

/* class RSPF_DLL rspfGzStreamBase : virtual public rspfProtocolStream */
/* { */
/* protected: */
/*     rspfGzStreamBuf buf; */
    
/* public: */
/*     rspfGzStreamBase():rspfProtocolStream(&buf) {  } */
/*     rspfGzStreamBase( const char* name, int open_mode); */
/*     virtual ~rspfGzStreamBase(); */
/*     virtual void open( const char* name, int open_mode); */
/*     virtual void close(); */
/*     rspfGzStreamBuf* rdbuf() { return &buf; } */
    
/* TYPE_DATA     */
/* }; */

//class RSPF_DLL rspfIgzStream : public rspfGzStreamBase, public std::istream
class RSPF_DLL rspfIgzStream : public rspfIFStream
{
public:
   rspfIgzStream();
   rspfIgzStream( const char* name,
                   std::ios_base::openmode mode = std::ios_base::in);
   virtual ~rspfIgzStream();
   rspfGzStreamBuf* rdbuf();
   
   virtual void open( const char* name,
                      std::ios_base::openmode mode = std::ios_base::in);
   
   virtual void close();
   virtual bool is_open()const;
   virtual bool isCompressed()const;

protected:
    rspfGzStreamBuf buf;
   
}; // End of class rspfIgzStream

class RSPF_DLL rspfOgzStream : public rspfOFStream
{
public:
   rspfOgzStream();
   rspfOgzStream( const char* name,
                   std::ios_base::openmode mode =
                   std::ios_base::out|std::ios_base::trunc );
   virtual ~rspfOgzStream();
   
   rspfGzStreamBuf* rdbuf();
   void open( const char* name,
              std::ios_base::openmode mode =
              std::ios_base::out|std::ios_base::trunc );
   virtual void close();
   virtual bool is_open()const;
   virtual bool isCompressed()const;

protected:
   rspfGzStreamBuf buf;

}; // End of class rspfOgzStream

#endif /* #if RSPF_HAS_LIBZ */
   
#endif /* #define rspfGzStream_HEADER */
