//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Factory for all rspf sensor models.
// 
//*******************************************************************
//  $Id: rspfSensorModelFactory.h 12081 2007-11-26 21:44:18Z dburken $
#ifndef rspfSensorModelFactory_HEADER
#define rspfSensorModelFactory_HEADER
#include <rspf/projection/rspfProjectionFactoryBase.h>

class rspfProjection;
class rspfString;
class rspfFilename;

class RSPF_DLL rspfSensorModelFactory : public rspfProjectionFactoryBase
{
public:
   /*!
    * METHOD: instance()
    * For accessing static instance of concrete factory.
    */
   static rspfSensorModelFactory* instance();
   
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx)const;
   /*!
    * METHOD: create()
    * Attempts to create an instance of the Product given a Specifier or
    * keywordlist. Returns successfully constructed product or NULL.
    */
   virtual rspfProjection* createProjection(const rspfString& spec)     const;
   virtual rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                             const char* prefix = 0)      const;

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
   
protected:
   rspfSensorModelFactory() {}
   
   static rspfSensorModelFactory*  theInstance;
   bool isNitf(const rspfFilename& filename)const;
   bool isLandsat(const rspfFilename& filename)const;
   void findCoarseGrid(rspfFilename& result,
                       const rspfFilename& geomFile)const;
};

#endif
