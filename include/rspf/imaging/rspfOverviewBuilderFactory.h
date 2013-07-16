//----------------------------------------------------------------------------
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: The rspf overview builder factory.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderFactory.h 15833 2009-10-29 01:41:53Z eshirschorn $

#ifndef rspfOverviewBuilderFactory_HEADER
#define rspfOverviewBuilderFactory_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryBase.h>


class rspfOverviewBuilderBase;
class rspfString;

/**
 * @class rspfOverviewBuilderFactory
 * @brief The rspf overview builder factory.
 */
class RSPF_DLL rspfOverviewBuilderFactory : public rspfOverviewBuilderFactoryBase
{
public:

   /** @brief static instance method. */
   static rspfOverviewBuilderFactory* instance();
   
   /** virtual destructor */
   virtual ~rspfOverviewBuilderFactory();

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
   rspfOverviewBuilderFactory();

   /** copy constructor hidden from use */
   rspfOverviewBuilderFactory(const rspfOverviewBuilderFactory& obj);

   /** operator= hidden from use. */
   void operator=(const rspfOverviewBuilderFactory& rhs);

   static rspfOverviewBuilderFactory* theInstance;
};

#endif /* #ifndef rspfOverviewBuilderFactory_HEADER */
