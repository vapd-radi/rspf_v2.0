#include <rspf/vec/rspfVpfDatabaseHeader.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/vpfutil/vpftable.h>
rspfVpfDatabaseHeader::rspfVpfDatabaseHeader()
{
}
rspfVpfDatabaseHeader::~rspfVpfDatabaseHeader()
{
}
rspfErrorCode rspfVpfDatabaseHeader::open(const rspfFilename& databaseHeaderTable)
{
   vpf_table_type tableTypeData;
   if( is_vpf_table( databaseHeaderTable.c_str() ) )
   {
      tableTypeData = vpf_open_table(databaseHeaderTable.c_str(),
                                     (storage_type)DISK,
                                     "rb",
                                     NULL);
      if(isDatabaseHeaderTable(tableTypeData))
      {
         
      }
      else
      {
         return rspfErrorCodes::RSPF_ERROR;
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   return rspfErrorCodes::RSPF_OK;
}
bool rspfVpfDatabaseHeader::isDatabaseHeaderTable(vpf_table_type& /* tableType */)
{
   return true;
}
