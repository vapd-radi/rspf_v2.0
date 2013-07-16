//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts 
//
//*************************************************************************
// $Id: rspfImageSourceFactoryBase.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfImageSourceFactoryBase_HEADER
#define rspfImageSourceFactoryBase_HEADER
#include <rspf/base/rspfObjectFactory.h>
class rspfImageSource;

class RSPFDLLEXPORT rspfImageSourceFactoryBase : public rspfObjectFactory
{
public:
   /*!
    * Convenient conversion method.  Gurantees an rspfImageSource is returned.  Returns
    * NULL otherwise
    */
   virtual rspfImageSource* createImageSource(const rspfString& name)const;
   virtual rspfImageSource* createImageSource(const rspfKeywordlist& kwl,
                                               const char* prefix=0)const;
   
protected:
   // Hide from use.
   rspfImageSourceFactoryBase();
   rspfImageSourceFactoryBase(const rspfImageSourceFactoryBase&);
   const rspfImageSourceFactoryBase& operator=(const rspfImageSourceFactoryBase&);
};

#endif
