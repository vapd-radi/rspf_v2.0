#ifndef rspfHttpRequest_HEADER
#define rspfHttpRequest_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfWebRequest.h>
#include <rspf/base/rspfHttpResponse.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfRefPtr.h>

class RSPF_DLL rspfHttpRequest :public rspfWebRequest
{
public:
   enum HttpMethodType
   {
      HTTP_METHOD_UNKNOWN = 0,
      HTTP_METHOD_GET     = 1,
      HTTP_METHOD_POST    = 2
      // later we will add support for post
   };
   rspfHttpRequest()
   :m_methodType(HTTP_METHOD_UNKNOWN)
   {
   }
   bool set(const rspfUrl& url, 
                         const rspfKeywordlist& headerOptions, 
                         HttpMethodType methodType=HTTP_METHOD_GET)
   {
      bool result = setUrl(url);
      m_headerOptions = headerOptions;
      m_methodType    = methodType;
      
      return result;
   }
   virtual ~rspfHttpRequest()
   {
   }
   void setHeaderOptions(const rspfKeywordlist& options)
   {
      m_headerOptions = options;
   }
   void addHeaderOption(const rspfString& name, const rspfString& value)
   {
      m_headerOptions.add(name, value);
   }
   void clearHeaderOptions(){m_headerOptions.clear();}
   rspfKeywordlist& getHeaderOptions(){return m_headerOptions;}
   const rspfKeywordlist& getHeaderOptions()const{return m_headerOptions;}
   
   
   virtual rspfString getLastError()const{return m_lastError;}
   
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   
protected:
   rspfKeywordlist m_headerOptions;
   HttpMethodType   m_methodType;
   TYPE_DATA;
};
#endif
