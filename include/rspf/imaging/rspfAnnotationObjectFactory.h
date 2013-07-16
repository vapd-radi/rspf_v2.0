//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfAnnotationObjectFactory.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAnnotationObjectFactory_HEADER
#define rspfAnnotationObjectFactory_HEADER
#include <rspf/base/rspfFactoryBaseTemplate.h>
#include <rspf/imaging/rspfAnnotationObject.h>
class rspfAnnotationObjectFactory : public rspfFactoryBase<rspfAnnotationObject>
{
public:
   /*!
    * METHOD: instance()
    * For accessing static instance of concrete factory.
    */
   static rspfAnnotationObjectFactory* instance();
   
   /*!
    * METHOD: create()
    * Attempts to create an instance of the Product given a Specifier or
    * keywordlist. Returns successfully constructed product or NULL.
    */
   virtual rspfAnnotationObject* create(const rspfString& spec) const;
   virtual rspfAnnotationObject* create(const rspfKeywordlist& kwl,
                                         const char* prefix=0)    const;

   /*!
    * METHOD: getList()
    * Returns name list of all products represented by this factory:
    */
   virtual list<rspfString> getList() const;

protected:
   rspfAnnotationObjectFactory();
   static rspfAnnotationObjectFactory*  theInstance;
};

#endif
