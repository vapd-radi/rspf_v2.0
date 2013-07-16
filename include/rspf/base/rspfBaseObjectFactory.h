//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfBaseObjectFactory.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfBaseObjectFactory_HEADER
#define rspfBaseObjectFactory_HEADER
#include <rspf/base/rspfObjectFactory.h>

class RSPFDLLEXPORT rspfBaseObjectFactory : public rspfObjectFactory
{
public:
   virtual ~rspfBaseObjectFactory(){theInstance = NULL;}
   
   static rspfBaseObjectFactory* instance();
   virtual rspfObject* createObject(const rspfString& typeName)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
protected:
   static rspfBaseObjectFactory* theInstance;
   
TYPE_DATA
};
#endif
