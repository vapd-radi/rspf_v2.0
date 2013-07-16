#ifndef rspfWebRequestFactoryBase_HEADER
#define rspfWebRequestFactoryBase_HEADER
#include <rspf/base/rspfBaseObjectFactory.h>
#include <rspf/base/rspfHttpRequest.h>
#include <rspf/base/rspfKeywordlist.h>
/**
 * We will for now hard code to be rspfHttpRequest but later
 * we will refactor and add a base rspfWebRequest so we can handle
 * other schema definitions such as ftp, scp, smtp, ... etc.
 */
class RSPF_DLL rspfWebRequestFactoryBase : public rspfBaseObjectFactory
{
public:
   virtual rspfWebRequest* create(const rspfUrl& /*url*/)
   {
      return 0;
   }
};
#endif
