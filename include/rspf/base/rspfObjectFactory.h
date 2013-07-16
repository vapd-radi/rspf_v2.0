//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfObjectFactory.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfObjectFactory_HEADER
#define rspfObjectFactory_HEADER

#include <vector>
#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfObjectFactory : public rspfObject
{
public:
   virtual ~rspfObjectFactory(){}

   /*!
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const=0;

   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const=0;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const=0;
   
protected:


TYPE_DATA
};

#endif
