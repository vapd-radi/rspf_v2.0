#ifndef rspfProtocolStream_HEADER
#define rspfProtocolStream_HEADER
#include <rspf/base/rspfStreamBase.h>

class rspfProtocolStream : public rspfStreamBase
{
public:
   rspfProtocolStream(){}
   rspfProtocolStream(std::streambuf* buf):rspfStreamBase(buf)
      {
      }

      /**
       * This is a generic interface to any stream for opening and closing
       * If supported this could be an interface to not only file streams
       * but also http, https, ftp, ... etc
       */
   virtual void open(const char* protocolString,
                     int openMode=0)=0;

   /**
    * Closes the stream
    */
   virtual void close()=0;

   
   virtual bool is_open()const=0;

   virtual bool isCompressed()const=0;
};

#endif
