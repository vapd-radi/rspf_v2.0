//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author: Garrett Potts
//
// Description: This is an initial cut at an http response object.  The HttpResponse is
//              returned from the HttpRequest base object.
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfHttpResponse_HEADER
#define rspfHttpResponse_HEADER

#include <iostream>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfByteStreamBuffer.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfWebResponse.h>

class RSPF_DLL rspfHttpResponse : public rspfWebResponse
{
public:
   rspfHttpResponse()
   :m_headerStream(&m_headerBuffer),
   m_bodyStream(&m_bodyBuffer)
   {
      clear();
   }
   
   rspfByteStreamBuffer& headerBuffer(){return m_headerBuffer;}
   const rspfByteStreamBuffer& headerBuffer()const{return m_headerBuffer;}
   rspfByteStreamBuffer& bodyBuffer(){return m_bodyBuffer;}
   const rspfByteStreamBuffer& bodyBuffer()const{return m_bodyBuffer;}
   
   std::iostream& headerStream(){return m_headerStream;}
   std::iostream& bodyStream(){return m_bodyStream;}
   
   virtual std::istream* getInputStream()
         {return static_cast<std::istream*>(&m_bodyStream);}
   
   /**
    * Clears out the Response and prepares for a new response.
    */
   virtual void clear()
   {
      m_headerBuffer.clear();
      m_bodyBuffer.clear();
      m_headerKwl.clear();
      m_statusLine = "";
      m_statusCode = 200;
   }
   
   /**
    * This will parse out the response code from the status line and initialize
    * the header variables into a keywordlist.
    */
   void convertHeaderStreamToKeywordlist();
   virtual void clearLastError(){m_statusCode = 200;m_statusLine="";}
   
   virtual rspfString getLastError()const{return ((m_statusCode == 200)?rspfString(""):m_statusLine);}
   
   rspfKeywordlist& headerKwl(){return m_headerKwl;}
   const rspfKeywordlist& headerKwl()const{return m_headerKwl;}
   
   const rspfString& statusLine()const{return m_statusLine;}
   rspf_uint32 getStatusCode()const{return m_statusCode;}
   
protected:
   rspfKeywordlist      m_headerKwl;
   rspfByteStreamBuffer m_headerBuffer;
   rspfByteStreamBuffer m_bodyBuffer;
   std::iostream         m_headerStream;
   std::iostream         m_bodyStream;
   rspfString           m_statusLine;
   rspf_uint32          m_statusCode;
   
TYPE_DATA;
};
#endif

