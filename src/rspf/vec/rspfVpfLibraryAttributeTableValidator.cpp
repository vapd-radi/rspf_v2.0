#include <rspf/vec/rspfVpfLibraryAttributeTableValidator.h>
#include <rspf/vec/rspfVpfTable.h>
bool rspfVpfLibraryAttributeTableValidator::isValid(rspfVpfTable& aTable)const
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
   column = table_pos("LIBRARY_NAME", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("XMIN", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("YMIN", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("XMAX", *data);
   if(column < 0)
   {
      return false;
   }
   column = table_pos("YMAX", *data);
   if(column < 0)
   {
      return false;
   }
   return true;
}
