#ifndef rspfWebRequest_HEADER
#define rspfWebRequest_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfUrl.h>
#include <rspf/base/rspfWebResponse.h>

class RSPF_DLL rspfWebRequest : public rspfObject
{
public:
   rspfWebRequest(){}
   rspfWebRequest(const rspfUrl& url)
   :m_url(url)
   {}
   const rspfUrl& getUrl()const{return m_url;}
   virtual bool setUrl(const rspfUrl& url){m_url = url; return true;}
   void clearLastError(){m_lastError = "";}
   const rspfString getLastError(){return m_lastError;}
   virtual rspfWebResponse* getResponse(){return 0;}
   
protected:
   rspfUrl m_url;
   mutable rspfString m_lastError;
   
   TYPE_DATA;
};
#endif
