#ifndef rspfGdalFactory_HEADER
#define rspfGdalFactory_HEADER
#include <rspf/imaging/rspfImageHandlerFactoryBase.h>
#include <rspf/base/rspfString.h>
class rspfGdal;
class rspfFilename;
class rspfKeywordlist;
class rspfGdalFactory : public rspfImageHandlerFactoryBase
{
public:
   virtual ~rspfGdalFactory();
   static rspfGdalFactory* instance();
   
   /**
    * @brief open that takes a filename.
    * @param fileName File to open.
    * @param trySuffixFirst If true calls code to try to open by suffix first,
    * then goes through the list of available handlers. default=true.
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    * @return Pointer to image handler or null if cannot open.
    */
   virtual rspfImageHandler* open(const rspfFilename& fileName,
                                   bool openOverview=true)const;
   virtual rspfImageHandler* open(const rspfKeywordlist& kwl,
                                   const char* prefix=0)const;
   
   virtual rspfObject* createObject(const rspfString& typeName)const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   virtual void getSupportedExtensions(rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const;
   virtual void getImageHandlersBySuffix(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& ext)const;
   virtual void getImageHandlersByMimeType(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& mimeType)const;
  
protected:
   rspfGdalFactory(){}
   rspfGdalFactory(const rspfGdalFactory&){}
   void operator = (const rspfGdalFactory&){}
   static rspfGdalFactory* theInstance;
TYPE_DATA
};
#endif
