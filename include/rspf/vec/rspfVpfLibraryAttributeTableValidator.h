#ifndef rspfVpfLibraryAttributeTableValidator_HEADER
#define rspfVpfLibraryAttributeTableValidator_HEADER
#include <rspf/vec/rspfVpfTableValidator.h>
class rspfVpfTable;
class rspfVpfLibraryAttributeTableValidator :public rspfVpfTableValidator
{
public:
   virtual bool isValid(rspfVpfTable& aTable)const;
};
#endif
