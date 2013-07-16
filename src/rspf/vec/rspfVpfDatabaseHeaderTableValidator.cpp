#include <rspf/vec/rspfVpfDatabaseHeaderTableValidator.h>
#include <rspf/vec/rspfVpfTable.h>
rspfVpfDatabaseHeaderTableValidator::~rspfVpfDatabaseHeaderTableValidator()
{
}
bool rspfVpfDatabaseHeaderTableValidator::isValid(rspfVpfTable& aTable)const
{
   if(&aTable == NULL)
   {
      return false;
   }
   if(aTable.isClosed())
   {
      return false;
   }
   const vpf_table_type* data = aTable.getVpfTableData();
   if(!data)
   {
      return false;
   }
   
   if(!data->fp)
   {
      return false;
   }
   long column = table_pos("ID", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("VPF_VERSION", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("DATABASE_NAME", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("DATABASE_DESC", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("MEDIA_STANDARD", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("ORIGINATOR", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("ADDRESSEE", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("MEDIA_VOLUMES", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("SEQ_NUMBERS", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("NUM_DATA_SETS", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("SECURITY_CLASS", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("DOWNGRADING", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("DOWNGRADE_DATE", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("RELEASABILITY", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("TRANSMITTAL_ID", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("EDITION_NUMBER", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("EDITION_DATE", *data);
   if(column < 0)
   {
      return false;
   }
   
   return true;
}
