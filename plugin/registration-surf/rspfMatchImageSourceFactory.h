#ifndef rspfMatchImageSourceFactory_HEADER
#define rspfMatchImageSourceFactory_HEADER
#include <rspf/imaging/rspfImageSourceFactoryBase.h>

class rspfMatchImageSourceFactory : public rspfImageSourceFactoryBase
{
public:
   virtual ~rspfMatchImageSourceFactory();
   static rspfMatchImageSourceFactory* instance();
   virtual rspfObject* createObject(const rspfString& name)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
protected:
   // Hide from use.
   rspfMatchImageSourceFactory();
   rspfMatchImageSourceFactory(const rspfMatchImageSourceFactory&);
   const rspfMatchImageSourceFactory& operator=(rspfMatchImageSourceFactory&);

   static rspfMatchImageSourceFactory* theInstance;
TYPE_DATA
};

#endif
