//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Class declaration of Registry (singleton) for info factories.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfInfoFactoryRegistry_HEADER
#define rspfInfoFactoryRegistry_HEADER 1

#include <rspf/base/rspfConstants.h> /* for RSPF_DLL macro */
#include <OpenThreads/Mutex>
#include <vector>

// Forward class declarations.
class rspfInfoFactoryInterface;
class rspfInfoBase;
class rspfFilename;

class RSPF_DLL rspfInfoFactoryRegistry
{
public:
   
   /** destructor */
   ~rspfInfoFactoryRegistry();

   /**
    * instance method
    *
    * @return Pointer to the instance of the registry.
    */
   static rspfInfoFactoryRegistry* instance();

   /**
    * @brief Method to add factory to registry.
    * @param factory Factory to register.
    */
   void registerFactory(rspfInfoFactoryInterface* factory);

   /**
    * @brief Adds factory to the front of the registry.
    * @param factory Factory to register.
    */
   void registerFactoryToFront( rspfInfoFactoryInterface* factory );
                               
   /**
    * Method to remove a factory from the registry.  Used by plugins when they
    * are unloaded.
    * @param factory Factory to remove.
    */
   void unregisterFactory(rspfInfoFactoryInterface* factory);

   /**
    * @brief Create method.
    *
    * @param file Some file you want info for.
    *
    * @return rspfInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
   rspfInfoBase* create(const rspfFilename& file) const;
   
protected:

   /**
    * @brief Hidden from use default constructor.
    * @note Adds rspfInfoFactory to registry.
    */
   rspfInfoFactoryRegistry();

   /** @brief hidden from use copy constructor */
   rspfInfoFactoryRegistry(const rspfInfoFactoryRegistry& obj);

   /** @brief hidden from use assignment operator */
   const rspfInfoFactoryRegistry& operator=(
      const rspfInfoFactoryRegistry& rhs);
   
   std::vector<rspfInfoFactoryInterface*> m_factoryList;

   OpenThreads::Mutex m_mutex;

   static rspfInfoFactoryRegistry* m_instance;
};

#endif /* #ifndef rspfInfoFactoryRegistry_HEADER */
