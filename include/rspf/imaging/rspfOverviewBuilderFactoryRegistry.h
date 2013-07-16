//----------------------------------------------------------------------------
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: The factory registry for overview builders.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderFactoryRegistry.h 19907 2011-08-05 19:55:46Z dburken $

#ifndef rspfOverviewBuilderFactoryRegistry_HEADER
#define rspfOverviewBuilderFactoryRegistry_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryBase.h>
#include <rspf/imaging/rspfOverviewBuilderBase.h>
#include <rspf/base/rspfFactoryListInterface.h>
#include <vector>

/**
 * @class rspfOverviewBuilderFactoryRegistry
 * @brief The factory registry for overview builders.
 *
 * Typical usage:
 * std::vector<rspfString> typeList;
 * rspfOverviewBuilderFactoryRegistry::instance()->getTypeNameList(typeList);
 * rspfOverviewBuilderBase* builder =
 * rspfOverviewBuilderFactoryRegistry::instance()->createBuilder(
 * someStringFromTypeList);
 */
class RSPF_DLL rspfOverviewBuilderFactoryRegistry :
   public rspfObjectFactory, 
   public rspfFactoryListInterface<rspfOverviewBuilderFactoryBase, rspfOverviewBuilderBase>
{
public:

   /**
    * @brief instance method for access to theInstance pointer.
    * @return Pointer to this class.
    */
   static rspfOverviewBuilderFactoryRegistry* instance();

   /*!
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName) const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0) const;
   /**
    * @brief Creates a builder from a string.  This should match a string from
    * the getTypeNameList() method.
    * @return Pointer to rspfOverviewBuilderBase or NULL is not found
    * within registered factories.
    */
   rspfOverviewBuilderBase* createBuilder(const rspfString& typeName) const;

   /**
    * @brief Method to populate a list of supported types of all factories
    * registered to this registry.
    * 
    * Example:
    * 
    * rspfTiffOverviewNearest
    * rspfTiffOverviewBox
    *
    * @param typeList List of rspfStrings to add to.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList) const;
   
private:
   /** default constructor hidden from use */
   rspfOverviewBuilderFactoryRegistry();

   /** copy constructor hidden from use */
   rspfOverviewBuilderFactoryRegistry(const rspfOverviewBuilderFactoryRegistry& obj);

   /** operator= hidden from use. */
   void operator=(const rspfOverviewBuilderFactoryRegistry& rhs);

   /** The static instance of this class. */
   static rspfOverviewBuilderFactoryRegistry* m_instance;
};

#endif /* #ifndef rspfOverviewBuilderFactoryRegistry_HEADER */
