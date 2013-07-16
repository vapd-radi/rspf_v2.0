//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
// 
// Class declarations for:
//
// rspfIStream
// rspfOStream
// rspfIOStream
// rspfIOMemoryStream
// rspfIMemoryStream
// rspfOMemoryStream
// rspfIOFStream
// rspfIFStream
// rspfOFStream
//
//*******************************************************************
//  $Id: rspfIoStream.h 11176 2007-06-07 19:45:56Z dburken $
#ifndef rspfIoStream_HEADER
#define rspfIoStream_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfStreamBase.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <rspf/base/rspfString.h>


class RSPF_DLL rspfIStream : public rspfStreamBase, public std::istream   
{
public:
   rspfIStream();
   rspfIStream(std::streambuf* sb);
   virtual ~rspfIStream();
};


class RSPF_DLL rspfOStream : public rspfStreamBase, public std::ostream
{
public:
   rspfOStream();
   rspfOStream(std::streambuf* sb);   
   virtual ~rspfOStream();
};

class RSPF_DLL rspfIOStream : public rspfStreamBase, public std::iostream
{
public:
   rspfIOStream();
   virtual ~rspfIOStream();
};

class RSPF_DLL rspfIOMemoryStream : public rspfIOStream
{
public:
   rspfIOMemoryStream();

   virtual ~rspfIOMemoryStream();

   // ??? (drb)
   bool is_open()const;

   // ??? (drb)
   virtual void open(const char* /* protocolString */,
                     int /* openMode */);

   rspfString str();

   // ??? (drb)
   virtual void close();

   // ??? (drb) std::streamsize
   rspf_uint64 size()const;

protected:
   std::stringbuf theBuf;
};

class RSPF_DLL rspfIMemoryStream : public rspfIStream
{
public:
   
   rspfIMemoryStream(const rspfString& inputBuf);
   
   virtual ~rspfIMemoryStream();
   
   bool is_open()const;
   
   rspf_uint64 size()const;
   
   virtual void open(const char* /* protocolString */,
                     int /* openMode */ );

   virtual void close();
   
   rspfString str();

protected:
   std::stringbuf theBuf;
   
};

class RSPF_DLL rspfOMemoryStream : public rspfOStream
{
public:
   rspfOMemoryStream();
   virtual ~rspfOMemoryStream();   

   bool is_open()const;

   rspf_uint64 size()const;

   virtual void open(const char* /* protocolString */,
                     int /* openMode */ );

   virtual void close();

   rspfString str();

protected:
   std::stringbuf theBuf;
};

class RSPF_DLL rspfIOFStream : public rspfStreamBase, public std::fstream
{
public:
   rspfIOFStream();

   rspfIOFStream(const char* name,
                  std::ios_base::openmode mode =
                  std::ios_base::in | std::ios_base::out);

   virtual ~rspfIOFStream();
};

class RSPF_DLL rspfIFStream : public rspfStreamBase, public std::ifstream
{
public:
   rspfIFStream();
   
   rspfIFStream(const char* file,
                 std::ios_base::openmode mode = std::ios_base::in);

   virtual ~rspfIFStream();

};

class RSPF_DLL rspfOFStream : public rspfStreamBase, public std::ofstream
{
public:
   rspfOFStream();

   rspfOFStream(const char* name,
                 std::ios_base::openmode mode =
                 std::ios_base::out|std::ios_base::trunc);

   virtual ~rspfOFStream();

};

RSPF_DLL void operator >> (rspfIStream& in,rspfOStream& out);
RSPF_DLL rspfIOStream& operator >> (rspfIStream& in,rspfIOStream& out);
RSPF_DLL void operator >> (rspfIOStream& in,rspfOStream& out);
RSPF_DLL rspfIOStream& operator >> (rspfIOStream& in,rspfIOStream& out);
RSPF_DLL void operator << (rspfOStream& out, rspfIStream& in);
RSPF_DLL void operator << (rspfOStream& out, rspfIOStream& in);
RSPF_DLL rspfIOStream& operator << (rspfIOStream& out, rspfIStream& in);
RSPF_DLL rspfIOStream& operator << (rspfIOStream& out, rspfIOStream& in);


#endif
