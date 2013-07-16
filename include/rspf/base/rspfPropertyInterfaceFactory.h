//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfPropertyInterfaceFactory.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfPropertyInterfaceFactory_HEADER
#define rspfPropertyInterfaceFactory_HEADER


#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfObjectFactory.h>
class rspfObject;
class rspfPropertyInterface;

class RSPFDLLEXPORT rspfPropertyInterfaceFactory : public rspfObjectFactory
{
public:
   rspfPropertyInterfaceFactory()
      :rspfObjectFactory()
      {}
   virtual ~rspfPropertyInterfaceFactory()
      {}

   virtual rspfPropertyInterface* createInterface( const rspfObject* obj ) const = 0;

   
TYPE_DATA
};

#endif
