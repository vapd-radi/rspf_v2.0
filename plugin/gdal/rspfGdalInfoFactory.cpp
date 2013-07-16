#include <rspfGdalInfoFactory.h>
#include <rspfOgrInfo.h>
#include <rspfHdfInfo.h>
#include <rspf/support_data/rspfInfoFactory.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
rspfGdalInfoFactory::~rspfGdalInfoFactory()
{}
rspfGdalInfoFactory* rspfGdalInfoFactory::instance()
{
   static rspfGdalInfoFactory sharedInstance;
   return &sharedInstance;
}
rspfInfoBase* rspfGdalInfoFactory::create(const rspfFilename& file) const
{
   rspfRefPtr<rspfInfoBase> result = 0;
   result = new rspfHdfInfo();
   if ( result->open(file) )
   {
      return result.release();
   }
   result = new rspfOgrInfo();
   if ( result->open(file) )
   {
      return result.release();
   }
   return 0;
}
rspfGdalInfoFactory::rspfGdalInfoFactory()
{}
rspfGdalInfoFactory::rspfGdalInfoFactory(const rspfGdalInfoFactory& /* obj */ )
{}
const rspfGdalInfoFactory& rspfGdalInfoFactory::operator=(
   const rspfGdalInfoFactory& /* rhs */)
{
   return *this;
}
