#ifndef rspfImageViewTransformFactory_HEADER
#define rspfImageViewTransformFactory_HEADER
#include <rspf/base/rspfObject.h>
class rspfImageViewTransform;
class rspfKeywordlist;
class rspfImageViewTransformFactory : public rspfObject
{
public:
   static rspfImageViewTransformFactory* instance();
   virtual rspfImageViewTransform* createTransform(const rspfKeywordlist& kwl,
                                                    const char* prefix = 0);
   
protected:
   rspfImageViewTransformFactory(){}//hide
   rspfImageViewTransformFactory(const rspfImageViewTransformFactory&){}
   void operator =(const rspfImageViewTransformFactory&){}
   static rspfImageViewTransformFactory* theInstance;
TYPE_DATA
};
#endif
