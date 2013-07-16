#ifndef rspfRegistrationImageSourceFactory_HEADER
#define rspfRegistrationImageSourceFactory_HEADER
#include <rspf/imaging/rspfImageSourceFactoryBase.h>

class rspfRegistrationImageSourceFactory : public rspfImageSourceFactoryBase
{
public:
   virtual ~rspfRegistrationImageSourceFactory();
   static rspfRegistrationImageSourceFactory* instance();
   virtual rspfObject* createObject(const rspfString& name)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
protected:
   // Hide from use.
   rspfRegistrationImageSourceFactory();
   rspfRegistrationImageSourceFactory(const rspfRegistrationImageSourceFactory&);
   const rspfRegistrationImageSourceFactory& operator=(rspfRegistrationImageSourceFactory&);

   static rspfRegistrationImageSourceFactory* theInstance;
TYPE_DATA
};

#endif
