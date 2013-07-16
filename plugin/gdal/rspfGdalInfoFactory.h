#ifndef rspfGdalInfoFactory_HEADER
#define rspfGdalInfoFactory_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoFactoryInterface.h>
class rspfFilename;
class rspfInfoBase;
/**
 * @brief Info factory.
 */
class rspfGdalInfoFactory : public rspfInfoFactoryInterface
{
public:
   /** virtual destructor */
   virtual ~rspfGdalInfoFactory();
   static rspfGdalInfoFactory* instance();
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
   rspfGdalInfoFactory();
   /** hidden from use copy constructor */
   rspfGdalInfoFactory(const rspfGdalInfoFactory& obj);
   /** hidden from use operator = */
   const rspfGdalInfoFactory& operator=(const rspfGdalInfoFactory& rhs);
};
#endif /* End of "#ifndef rspfGdalInfoFactory_HEADER" */
