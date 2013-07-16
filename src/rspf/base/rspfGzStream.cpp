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
// File          : gzstream.C
// Revision      : $Revision: 17195 $
// Revision_date : $Date: 2010-04-24 01:32:18 +0800 (周六, 2010-04-24) $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

#include <rspf/base/rspfGzStream.h>

#if RSPF_HAS_LIBZ
#include <zlib.h>

#include <iostream>
#include <fstream>
#include <cstring>  // for memcpy


// --------------------------------------
// class rspfGzStreamBuf:
// --------------------------------------


struct rspfGzStreamBuf::PrivateData
{
	gzFile           file;               // file handle for compressed file
};
// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See header file for user classes.
// ----------------------------------------------------------------------------
rspfGzStreamBuf::rspfGzStreamBuf()
   : prvtData(new PrivateData()),
     opened(false),
     mode(0) // ??? (drb)
{

   setp( buffer, buffer + (bufferSize-1));
   setg( buffer + 4,     // beginning of putback area
         buffer + 4,     // read position
         buffer + 4);    // end position      
   // ASSERT: both input & output capabilities will not be used together
}

rspfGzStreamBuf::~rspfGzStreamBuf()
{
   close();
   if(prvtData)
   {
	   delete prvtData;
	   prvtData = 0;
   }
}

bool rspfGzStreamBuf::is_open() const
{
   return opened;
}

rspfGzStreamBuf* rspfGzStreamBuf::open( const char* name, int open_mode)
{
    if ( is_open())
    {
        return (rspfGzStreamBuf*)0;
    }
    mode = open_mode;
    // no append nor read/write mode
    if ((mode & std::ios::ate) || (mode & std::ios::app)
        || ((mode & std::ios::in) && (mode & std::ios::out)))
    {
        return (rspfGzStreamBuf*)0;
    }
    
    char  fmode[10];
    char* fmodeptr = fmode;
    if ( mode & std::ios::in)
    {
       *fmodeptr++ = 'r';
    }
    else if ( mode & std::ios::out)
    {
        *fmodeptr++ = 'w';
    }
    *fmodeptr++ = 'b';
    *fmodeptr = '\0';
    prvtData->file = gzopen( name, fmode);
    if (prvtData->file == 0)
    {
       return (rspfGzStreamBuf*)0;
    }
    opened = true;
    return this;
}

rspfGzStreamBuf * rspfGzStreamBuf::close()
{
   if ( is_open())
   {
      sync();
      opened = false;
      if ( gzclose( prvtData->file) == Z_OK)
      {
         return this;
      }
    }
    return (rspfGzStreamBuf*)0;
}

std::streamsize rspfGzStreamBuf::xsgetn(char_type* __s,
                                         std::streamsize n)
{
   int num = gzread( prvtData->file, __s, n);

   if (num <= 0) // ERROR or EOF
      return EOF;
   
   return num;
}

int rspfGzStreamBuf::underflow()
{
   // used for input buffer only
   if ( gptr() && ( gptr() < egptr()))
      return * reinterpret_cast<unsigned char *>( gptr());

   if ( ! (mode & std::ios::in) || ! opened)
        return EOF;
    // Josuttis' implementation of inbuf
    int n_putback = gptr() - eback();
    if ( n_putback > 4)
        n_putback = 4;
    memcpy( buffer + (4 - n_putback), gptr() - n_putback, n_putback);

    int num = gzread( prvtData->file, buffer+4, bufferSize-4);
    if (num <= 0) // ERROR or EOF
        return EOF;

    // reset buffer pointers
    setg( buffer + (4 - n_putback),   // beginning of putback area
          buffer + 4,                 // read position
          buffer + 4 + num);          // end of buffer

    // return next character
    return * reinterpret_cast<unsigned char *>( gptr());
}

int rspfGzStreamBuf::flush_buffer()
{
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    int w = pptr() - pbase();
    if ( gzwrite( prvtData->file, pbase(), w) != w)
        return EOF;
    pbump( -w);
    return w;
}

int rspfGzStreamBuf::overflow( int c) { // used for output buffer only
    if ( ! ( mode & std::ios::out) || ! opened)
        return EOF;
    if (c != EOF) {
        *pptr() = c;
        pbump(1);
    }
    if ( flush_buffer() == EOF)
        return EOF;
    return c;
}

int rspfGzStreamBuf::sync() {
    // Changed to use flush_buffer() instead of overflow( EOF)
    // which caused improper behavior with std::endl and flush(),
    // bug reported by Vincent Ricard.
    if ( pptr() && pptr() > pbase()) {
        if ( flush_buffer() == EOF)
            return -1;
    }
    return 0;
}

rspfGzStreamBuf::pos_type rspfGzStreamBuf::seekoff(off_type t,
                                                     std::ios_base::seekdir dir,
                                                     std::ios_base::openmode /* omode */)
{
   int whence = 0;
   switch(dir)
   {
      case std::ios::beg:
      {
         whence = SEEK_SET;
         break;
      }
      case std::ios::end:
      {
         whence = SEEK_END;
         break;
      }
      case std::ios::cur:
      {
         whence = SEEK_CUR;
         break;
      }
      default:
      {
         whence = SEEK_CUR;
         break;
      }
   }

   return gzseek(prvtData->file, t, whence);
}

// rspfGzStreamBuf::pos_type rspfGzStreamBuf::seekpos(pos_type posType, 
//                                                      std::ios_base::openmode)
// {
//    int whence = 0;
//    switch(posType)
//    {
//       case std::ios::beg:
//       {
//          whence = SEEK_SET;
//          break;
//       }
//       case std::ios::end:
//       {
//          whence = SEEK_END;
//          break;
//       }
//       case std::ios::cur:
//       {
//          whence = SEEK_CUR;
//          break;
//       }
//       default:
//       {
//          whence = SEEK_CUR;
//          break;
//       }
//    }
   
//    return gzseek(file, t, whence);
// }

// --------------------------------------
// class rspfGzStreamBase:
// --------------------------------------


rspfIgzStream::rspfIgzStream()
   : rspfIFStream()
{
   init(&buf);
}

rspfIgzStream::rspfIgzStream( const char* name,
                                std::ios_base::openmode mode )
   : rspfIFStream()
{
   init(&buf);
   open(name, mode);
}

rspfIgzStream::~rspfIgzStream()
{
   buf.close();
}

rspfGzStreamBuf* rspfIgzStream::rdbuf()
{
   return &buf;
}

void rspfIgzStream::open( const char* name,
                           std::ios_base::openmode mode )
{
   if ( ! buf.open( name, mode))
   {
      clear( rdstate() | std::ios::badbit);
   }
}

void rspfIgzStream::close()
{
   if ( buf.is_open())
   {
      if ( !buf.close())
      {
         clear( rdstate() | std::ios::badbit);
      }
   }
}

bool rspfIgzStream::is_open()const
{
   return buf.is_open();
}

bool rspfIgzStream::isCompressed()const
{
   return true;
}

rspfOgzStream::rspfOgzStream()
   : rspfOFStream()
{
   init(&buf);
}

rspfOgzStream::rspfOgzStream( const char* /* name */,
                                std::ios_base::openmode /* mode */)
   : rspfOFStream()
{
   init(&buf);
}

rspfOgzStream::~rspfOgzStream()
{
   buf.close();
}

rspfGzStreamBuf* rspfOgzStream::rdbuf()
{
   return &buf;
}

void rspfOgzStream::open( const char* name,
                           std::ios_base::openmode mode )
{
   if ( ! buf.open( name, mode))
   {
      clear( rdstate() | std::ios::badbit);
   }
}

void rspfOgzStream::close()
{
   if ( buf.is_open())
   {
      if ( !buf.close())
      {
         clear( rdstate() | std::ios::badbit);
      }
   }
}

bool rspfOgzStream::is_open()const
{
   return buf.is_open();
}

bool rspfOgzStream::isCompressed()const
{
   return true;
}

// ============================================================================
// EOF //
#endif // RSPF_HAS_LIBZ
