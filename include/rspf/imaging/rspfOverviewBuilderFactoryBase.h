//----------------------------------------------------------------------------
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: The base class for overview builders.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderFactoryBase.h 19187 2011-03-23 12:21:58Z gpotts $

#ifndef rspfOverviewBuilderFactoryBase_HEADER
#define rspfOverviewBuilderFactoryBase_HEADER

#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObjectFactory.h>

class rspfOverviewBuilderBase;
class rspfString;

/**
 * @class rspfOverviewBuilderFactoryBase
 * @brief The base class for factory overview builder factories.
 */
class RSPF_DLL rspfOverviewBuilderFactoryBase : public rspfObjectFactory
{
public:
   /** default constructor hidden from use */
   rspfOverviewBuilderFactoryBase();

   /** virtual destructor */
   virtual ~rspfOverviewBuilderFactoryBase();

   /**
    * @brief Creates a builder from a string.  This should match a string from
    * the getTypeNameList() method.  Pure virtual.
    * 
    * @return Pointer to rspfOverviewBuilderBase or NULL is not found
    * within registered factories.
    */
   virtual rspfOverviewBuilderBase* createBuilder(const rspfString& typeName) const = 0;

   /**
    * @brief Method to populate a list of supported types for the factory.
    * registered to this registry.  Pure virtual.
    *
    * @param typeList List of rspfStrings to add to.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const = 0;

   /*!
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   
};

#endif /* #ifndef rspfOverviewBuilderFactoryBase_HEADER */
