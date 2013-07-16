//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Factory for info objects.
// 
//----------------------------------------------------------------------------
// $Id: rspfInfoFactory.h 20125 2011-10-11 19:47:19Z dburken $
#ifndef rspfInfoFactory_HEADER
#define rspfInfoFactory_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoFactoryInterface.h>

class rspfFilename;
class rspfInfoBase;

/**
 * @brief Info factory.
 */
class RSPF_DLL rspfInfoFactory : public rspfInfoFactoryInterface
{
public:

   /** virtual destructor */
   virtual ~rspfInfoFactory();

   static rspfInfoFactory* instance();

   /**
    * @brief create method.
    *
    * @param file Some file you want info for.
    *
    * @return rspfInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
   virtual rspfInfoBase* create(const rspfFilename& file) const;
   
private:
   
   /** hidden from use default constructor */
   rspfInfoFactory();

   /** hidden from use copy constructor */
   rspfInfoFactory(const rspfInfoFactory& obj);

   /** hidden from use operator = */
   const rspfInfoFactory& operator=(const rspfInfoFactory& rhs);

   /** The single instance of this class. */
   static rspfInfoFactory* theInstance;
};

#endif /* End of "#ifndef rspfInfoFactory_HEADER" */
