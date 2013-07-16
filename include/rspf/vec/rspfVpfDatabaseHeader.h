#ifndef rspfVpfDatabaseHeader_HEADER
#define rspfVpfDatabaseHeader_HEADER
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/vpfutil/vpftable.h>
class rspfVpfDatabaseHeader
{
public:
   rspfVpfDatabaseHeader();
   virtual ~rspfVpfDatabaseHeader();
   virtual rspfErrorCode open(const rspfFilename& databaseHeaderTable);
private:
   bool isDatabaseHeaderTable(vpf_table_type& tableType);
};
#endif
