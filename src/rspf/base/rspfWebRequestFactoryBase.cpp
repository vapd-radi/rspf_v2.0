#include <rspf/base/rspfWebRequestFactoryBase.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfHttpRequest.h>

#if 0
rspfHttpRequest* rspfWebRequestFactoryBase::createHttpRequest(const rspfUrl& url,
                                            const rspfString& method)
{
   rspfKeywordlist kwl;
   kwl["type"] = url.getProtocol().c_str();
   kwl["method"] = method.c_str();
   rspfRefPtr<rspfObject> httpRequestResult = createObject(kwl);
   rspfRefPtr<rspfHttpRequest> result = dynamic_cast<rspfHttpRequest*>(httpRequestResult.get());

   return result.release();
}
#endif
