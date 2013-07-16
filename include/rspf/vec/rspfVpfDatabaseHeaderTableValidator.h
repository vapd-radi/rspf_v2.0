#ifndef rspfVpfDatabaseHeaderTableValidator_HEADER
#define rspfVpfDatabaseHeaderTableValidator_HEADER
#include <rspf/vec/rspfVpfTableValidator.h>
class rspfVpfTable;
class rspfVpfDatabaseHeaderTableValidator : public rspfVpfTableValidator
{
public:
   virtual ~rspfVpfDatabaseHeaderTableValidator();
   virtual bool isValid(rspfVpfTable& aTable)const;
};
#endif
