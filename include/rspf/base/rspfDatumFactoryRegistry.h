//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Class declaration of Registry (singleton) for datum factories.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfDatumFactoryRegistry_HEADER
#define rspfDatumFactoryRegistry_HEADER

#include <vector>
#include <rspf/base/rspfConstants.h> /* for RSPF_DLL macro */
#include <rspf/base/rspfDatumFactoryInterface.h> 
#include <OpenThreads/ReadWriteMutex>

// Forward class declarations.
class rspfDatumFactoryInterface;
class rspfString;
class rspfDatum;

class RSPF_DLL rspfDatumFactoryRegistry : public rspfDatumFactoryInterface
{
public:
   
   /** destructor */
   ~rspfDatumFactoryRegistry();

   /**
    * instance method
    *
    * @return Point to the instance of the registry.
    */
   static rspfDatumFactoryRegistry* instance();

   /**
    * Method to add factory to registry.
    * @param factory Factory to register.
    */
   void registerFactory(rspfDatumFactoryInterface* factory);
   
   /**
    * create method
    *
    * Implements pure virtual rspfDatumFactoryInterface::create.
    *
    * @return const pointer to a datum.
    */
   virtual const rspfDatum* create(const rspfString& code)const;
   virtual const rspfDatum* create(const rspfKeywordlist& kwl, const char *prefix=0) const;

   /**
    * getList method to return a combined list of all datums from registered
    * datum factories.
    *
    * @param list The list to add to.
    */
   virtual void getList(std::vector<rspfString>& list) const;
   
protected:

   /** hidden from use default constructor */
   rspfDatumFactoryRegistry();

   /** Single static instance of this class. */
   static rspfDatumFactoryRegistry* theInstance;

	mutable OpenThreads::ReadWriteMutex theFactoryListMutex;
   std::vector<rspfDatumFactoryInterface*> theFactoryList;
};

#endif /* #ifndef rspfDatumFactoryRegistry_HEADER */
