#ifndef rspfVpfBoundingRecordTable_HEADER
#define rspfVpfBoundingRecordTable_HEADER
#include <rspf/vec/rspfVpfTable.h>
#include <rspf/vec/rspfVpfExtent.h>
class rspfVpfBoundingRecordTable : public rspfVpfTable
{
public:
   rspfVpfBoundingRecordTable();
   virtual bool openTable(const rspfFilename& tableName);
   void getExtent(rspfVpfExtent& extent)const;
protected:
   rspfVpfExtent theExtent;
};
#endif
