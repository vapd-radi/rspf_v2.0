#include <rspf/base/rspfHttpRequest.h>

RTTI_DEF1(rspfHttpRequest, "rspfHttpRequest", rspfWebRequest);

bool rspfHttpRequest::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool result = true;
   rspfObject::loadState(kwl, prefix);
   rspfString url = kwl.find(prefix, "url");
   rspfString method = kwl.find(prefix, "method");
   method = method.upcase();
   
   if(method == "GET")
   {
      m_methodType = HTTP_METHOD_GET;
   }
   else if(method == "POST")
   {
      m_methodType = HTTP_METHOD_POST;
   }
   else
   {
      m_methodType = HTTP_METHOD_UNKNOWN;
   }
   
   
   clearHeaderOptions();
   
   return result;
}
