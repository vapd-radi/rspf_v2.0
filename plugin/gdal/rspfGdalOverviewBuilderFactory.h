#ifndef rspfGdalOverviewBuilderFactory_HEADER
#define rspfGdalOverviewBuilderFactory_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryBase.h>
class rspfOverviewBuilderBase;
class rspfString;
/**
 * @class rspfGdalOverviewBuilderFactory
 * @brief The rspf overview builder factory.
 */
class rspfGdalOverviewBuilderFactory:
   public rspfOverviewBuilderFactoryBase
{
public:
   /** @brief static instance method. */
   static rspfGdalOverviewBuilderFactory* instance();
   
   /** virtual destructor */
   virtual ~rspfGdalOverviewBuilderFactory();
   /**
    * @brief Creates a builder from a string.  This should match a string from
    * the getTypeNameList() method.  Pure virtual.
    * 
    * @return Pointer to rspfOverviewBuilderBase or NULL is not found
    * within registered factories.
    */
   virtual rspfOverviewBuilderBase* createBuilder(
      const rspfString& typeName) const;
   /**
    * @brief Method to populate a list of supported types for the factory.
    * registered to this registry.  Pure virtual.
    *
    * @param typeList List of rspfStrings to add to.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
private:
   /** default constructor hidden from use */
   rspfGdalOverviewBuilderFactory();
   /** copy constructor hidden from use */
   rspfGdalOverviewBuilderFactory(const rspfGdalOverviewBuilderFactory& obj);
   /** operator= hidden from use. */
   void operator=(const rspfGdalOverviewBuilderFactory& rhs);
   static rspfGdalOverviewBuilderFactory* theInstance;
};
#endif /* #ifndef rspfGdalOverviewBuilderFactory_HEADER */
