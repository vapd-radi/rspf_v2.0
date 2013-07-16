//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfTagFactoryRegistry.h 10291 2007-01-17 19:20:23Z dburken $
#ifndef rspfNitfTagFactoryRegistry_HEADER
#define rspfNitfTagFactoryRegistry_HEADER

#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfNitfRegisteredTag.h>

class rspfString;
class rspfNitfTagFactory;

class RSPF_DLL rspfNitfTagFactoryRegistry
{
public:
   virtual ~rspfNitfTagFactoryRegistry();
   void registerFactory(rspfNitfTagFactory* aFactory);
   void unregisterFactory(rspfNitfTagFactory* aFactory);
   
   static rspfNitfTagFactoryRegistry* instance();
   
   rspfRefPtr<rspfNitfRegisteredTag> create(const rspfString &tagName)const;
   bool exists(rspfNitfTagFactory* factory)const;
   
protected:
   rspfNitfTagFactoryRegistry();

private:
   /** hidden copy constructory */
   rspfNitfTagFactoryRegistry(const rspfNitfTagFactoryRegistry& factory);

   /** hidden operator= */
   const rspfNitfTagFactoryRegistry& operator=(
      const rspfNitfTagFactoryRegistry& factory);

   void initializeDefaults();

   static rspfNitfTagFactoryRegistry* theInstance;
   
   std::vector<rspfNitfTagFactory*> theFactoryList;
};

#endif
