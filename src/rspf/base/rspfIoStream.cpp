//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
// 
// Class definitiaons for:
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
//  $Id: rspfIoStream.cpp 11206 2007-06-13 13:11:35Z gpotts $
#include <rspf/base/rspfIoStream.h>

rspfIStream::rspfIStream()
   : rspfStreamBase(),
#ifdef _MSC_VER
   std::istream((std::_Uninitialized)0)
#else
   std::istream()
#endif
{}

rspfIStream::rspfIStream(std::streambuf* sb)
   : rspfStreamBase(),
     std::istream(sb)
{}

rspfIStream::~rspfIStream()
{}

rspfOStream::rspfOStream()
   : rspfStreamBase(),
#ifdef _MSC_VER
   std::ostream((std::_Uninitialized)0)
#else
   std::ostream()
#endif
{}

rspfOStream::rspfOStream(std::streambuf* sb)
   : rspfStreamBase(),
     std::ostream(sb)
{}

rspfOStream::~rspfOStream()
{}

rspfIOStream::rspfIOStream()
   : rspfStreamBase(),
#ifdef _MSC_VER
   std::iostream((std::_Uninitialized)0)
#else
   std::iostream()
#endif
{}

rspfIOStream::~rspfIOStream()
{}

rspfIOMemoryStream::rspfIOMemoryStream()
   : rspfIOStream(),
     theBuf(std::ios::in|std::ios::out)
{
   rspfIOStream::init(&theBuf);
}

rspfIOMemoryStream::~rspfIOMemoryStream()
{
}

bool rspfIOMemoryStream::is_open()const
{
   return true;
}

void rspfIOMemoryStream::open(const char* /* protocolString */,
                               int /* openMode */)
{
}

rspfString rspfIOMemoryStream::str()
{
   return theBuf.str();
}

void rspfIOMemoryStream::close()
{}

rspf_uint64 rspfIOMemoryStream::size()const
{
   rspfIOMemoryStream*  thisPtr = const_cast<rspfIOMemoryStream*>(this);
   std::streampos pos = thisPtr->tellg();
   thisPtr->seekg(0, std::ios::end);
   std::streampos endPos = thisPtr->tellg();
   thisPtr->seekg(pos, std::ios::beg);
   
   return (rspf_uint64)(endPos);
}

rspfIMemoryStream::rspfIMemoryStream(const rspfString& inputBuf)
   
   : rspfIStream(),
     theBuf(inputBuf.c_str(), std::ios::in)
{
   rspfIStream::init(&theBuf);
}

rspfIMemoryStream::~rspfIMemoryStream()
{
}

bool rspfIMemoryStream::is_open()const
{
   return true;
}

rspf_uint64 rspfIMemoryStream::size()const
{
   rspfIMemoryStream*  thisPtr = const_cast<rspfIMemoryStream*>(this);
   std::streampos pos = thisPtr->tellg();
   thisPtr->seekg(0, std::ios::end);
   std::streampos endPos = thisPtr->tellg();
   thisPtr->seekg(pos, std::ios::beg);
   return (rspf_uint64)(endPos);
}

void rspfIMemoryStream::open(const char* /* protocolString */,
                              int /* openMode */ )
{
}

void rspfIMemoryStream::close()
{}

rspfString rspfIMemoryStream::str()
{
   return theBuf.str();
}

rspfOMemoryStream::rspfOMemoryStream()
   : rspfOStream(),
     theBuf(std::ios::out)
{
   rspfOStream::init(&theBuf);
}

rspfOMemoryStream::~rspfOMemoryStream()
{
}

bool rspfOMemoryStream::is_open()const
{
   return true;
}

rspf_uint64 rspfOMemoryStream::size()const
{
   rspfOMemoryStream*  thisPtr = const_cast<rspfOMemoryStream*>(this);
   std::streampos pos = thisPtr->tellp();
   thisPtr->seekp(0, std::ios::end);
   std::streampos endPos = thisPtr->tellp();
   thisPtr->seekp(pos, std::ios::beg);
   return (rspf_uint64)(endPos);
}

void rspfOMemoryStream::open(const char* /* protocolString */,
                              int /* openMode */ )
{
}

void rspfOMemoryStream::close()
{}

rspfString rspfOMemoryStream::str()
{
   return theBuf.str();
}

rspfIOFStream::rspfIOFStream()
   : rspfStreamBase(),
     std::fstream()
{
}

rspfIOFStream::rspfIOFStream(const char* name,
                               std::ios_base::openmode mode)
   : rspfStreamBase(),
     std::fstream(name, mode)
{
}

rspfIOFStream::~rspfIOFStream()
{
}

rspfIFStream::rspfIFStream()
   : rspfStreamBase(),
     std::ifstream()
{
}
rspfIFStream::rspfIFStream(const char* file, std::ios_base::openmode mode)
   : rspfStreamBase(),
     std::ifstream(file, mode)
{
}

rspfIFStream::~rspfIFStream()
{
}

rspfOFStream::rspfOFStream()
   : rspfStreamBase(),
     std::ofstream()
{
}

rspfOFStream::rspfOFStream(const char* name, std::ios_base::openmode mode)
   : rspfStreamBase(),
     std::ofstream(name, mode)
{
}

rspfOFStream::~rspfOFStream()
{
}

void operator >> (rspfIStream& in,rspfOStream& out)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }
}

rspfIOStream& operator >> (rspfIStream& in,rspfIOStream& out)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }
   
   return out;
}

void operator >> (rspfIOStream& in,rspfOStream& out)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }
}

rspfIOStream& operator >> (rspfIOStream& in,rspfIOStream& out)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }

   return out;
}

void operator << (rspfOStream& out, rspfIStream& in)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }
}

void operator << (rspfOStream& out, rspfIOStream& in)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }
}

rspfIOStream& operator << (rspfIOStream& out, rspfIStream& in)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }

   return out;
}

rspfIOStream& operator << (rspfIOStream& out, rspfIOStream& in)
{
   char buf[1024];
   bool done = false;

   while(!done&&!in.fail())
   {
      in.read(buf, 1024);
      if(in.gcount() < 1024)
      {
         done = true;
      }
      if(in.gcount() > 0)
      {
         out.write(buf, in.gcount());
      }
   }

   return out;
}
