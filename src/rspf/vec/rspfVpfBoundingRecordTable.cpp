#include <rspf/vec/rspfVpfBoundingRecordTable.h>
rspfVpfBoundingRecordTable::rspfVpfBoundingRecordTable()
   :rspfVpfTable()
{
}
bool rspfVpfBoundingRecordTable::openTable(const rspfFilename& tableName)
{
   bool result = false;
   theExtent = rspfVpfExtent(0,0,0,0);
   bool firstOneSetFlag = false;
   
   if(rspfVpfTable::openTable(tableName))
   {
      rspf_int32 xminIdx = getColumnPosition("XMIN");
      rspf_int32 yminIdx = getColumnPosition("YMIN");
      rspf_int32 xmaxIdx = getColumnPosition("XMAX");
      rspf_int32 ymaxIdx = getColumnPosition("YMAX");
      
      if((xminIdx < 0)||
         (yminIdx < 0)||
         (xmaxIdx < 0)||
         (ymaxIdx < 0))
      {
         closeTable();
      }
      else
      {
         if(getNumberOfRows() > 0)
         {
            result = true;
            reset();
            rspf_int32 n = 1;
            
            rspf_float32 xmin;
            rspf_float32 ymin;
            rspf_float32 xmax;
            rspf_float32 ymax;
            
            row_type row;
            for(int rowIdx = 1; rowIdx < getNumberOfRows(); ++rowIdx)
            {
               if(rowIdx == 1)
               {
                  row = read_row(rowIdx,
                                 *theTableInformation);
               }
               else
               {
                  row = read_next_row(*theTableInformation);
               }
               get_table_element(xminIdx,
                                 row,
                                 *theTableInformation,
                                 &xmin,
                                 &n);
               get_table_element(yminIdx,
                                 row,
                                 *theTableInformation,
                                 &ymin,
                                 &n);
               get_table_element(xmaxIdx,
                                 row,
                                 *theTableInformation,
                                 &xmax,
                                 &n);
               get_table_element(ymaxIdx,
                                 row,
                                 *theTableInformation,
                                 &ymax,
                                 &n);
               if(!is_vpf_null_float(xmin)&&
                  !is_vpf_null_float(ymin)&&
                  !is_vpf_null_float(xmax)&&
                  !is_vpf_null_float(ymax))
               {
                  if(!firstOneSetFlag)
                  {
                     theExtent = rspfVpfExtent(xmin,
                                                ymin,
                                                xmax,
                                                ymax);
                     firstOneSetFlag = true;
                  }
                  else
                  {
                     theExtent = theExtent + rspfVpfExtent(xmin,
                                                            ymin,
                                                            xmax,
                                                            ymax);
                  }                  
               }
               free_row(row, *theTableInformation);
            }
         }
      }
   }
   return result;
}
void rspfVpfBoundingRecordTable::getExtent(rspfVpfExtent& extent)const
{
   extent = theExtent;
}
