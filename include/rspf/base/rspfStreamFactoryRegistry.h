//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*******************************************************************
//  $Id: rspfStreamFactoryRegistry.h 11176 2007-06-07 19:45:56Z dburken $
//
#ifndef rspfStreamFactoryRegistry_HEADER
#define rspfStreamFactoryRegistry_HEADER
#include <vector>

#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfIoStream.h>
#include <rspf/base/rspfStreamFactoryBase.h>


class RSPF_DLL rspfStreamFactoryRegistry : public rspfStreamFactoryBase
{
public:
   static rspfStreamFactoryRegistry* instance();
   virtual ~rspfStreamFactoryRegistry();
   
   void registerFactory(rspfStreamFactoryBase* factory);
   
   virtual rspfRefPtr<rspfIFStream> createNewIFStream(
      const rspfFilename& file, std::ios_base::openmode openMode) const;
   
protected:
   rspfStreamFactoryRegistry();
private:
   /** @brief copy constructor hidden from use */
   rspfStreamFactoryRegistry(const rspfStreamFactoryRegistry&);
   
   std::vector<rspfStreamFactoryBase*> theFactoryList;
   static rspfStreamFactoryRegistry* theInstance;
};

#endif
