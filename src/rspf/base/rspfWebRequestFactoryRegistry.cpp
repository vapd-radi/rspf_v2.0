#include <rspf/base/rspfWebRequestFactoryRegistry.h>
#include <rspf/base/rspfHttpRequest.h>

rspfWebRequestFactoryRegistry* rspfWebRequestFactoryRegistry::m_instance = 0;

rspfWebRequestFactoryRegistry::rspfWebRequestFactoryRegistry()
{
   m_instance = this;
}

rspfWebRequestFactoryRegistry* rspfWebRequestFactoryRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfWebRequestFactoryRegistry();
   }
   
   return m_instance;
}


rspfObject* rspfWebRequestFactoryRegistry::createObject(const rspfKeywordlist& kwl,
                                                          const char* prefix)const
{
  return createObjectFromRegistry(kwl, prefix);
}

rspfWebRequest* rspfWebRequestFactoryRegistry::create(const rspfUrl& url)
{
   rspf_uint32 idx = 0;
   rspfWebRequest* result = 0;
   for(idx = 0; ((idx < m_factoryList.size())&&!result); ++idx)
   {
      result = m_factoryList[idx]->create(url);
   }
   
   return result;
}

rspfHttpRequest* rspfWebRequestFactoryRegistry::createHttp(const rspfUrl& url)
{
   rspfRefPtr<rspfWebRequest> request = create(url);
   rspfRefPtr<rspfHttpRequest> result;
   if(request.valid())
   {
      result = dynamic_cast<rspfHttpRequest*>(request.get());
      request = 0;
   }
   
   return result.release();
}