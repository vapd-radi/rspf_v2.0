#ifndef rspfGdalObjectFactory_HEADER
#define rspfGdalObjectFactory_HEADER 1
#include <rspfPluginConstants.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfObjectFactory.h>
class RSPF_PLUGINS_DLL rspfGdalObjectFactory : public rspfObjectFactory
{
public:
   static rspfGdalObjectFactory* instance();
   /** virtual destructor */
   virtual ~rspfGdalObjectFactory();
   /**
    * @brief Object from string.
    * @return Pointer to object or 0 if not in this factory.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const;
   /**
    * @brief Object from keyword list.
    * @return Pointer to object or 0 if not in this factory.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   /**
    * @brief Adds list of objects this factory supports.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
private:
   /** hidden from use default construtor. */
   rspfGdalObjectFactory();
   /** hidden from use copy construtor. */
   rspfGdalObjectFactory(const rspfGdalObjectFactory& rhs);
   /** hidden from use operator= . */
   const rspfGdalObjectFactory& operator=(const rspfGdalObjectFactory& rhs);
   /** The single instance of this class. */
   static rspfGdalObjectFactory* theInstance;
TYPE_DATA
};
#endif /* #ifndef rspfGdalObjectFactory_HEADER */
