#ifndef rspfVpfTable_HEADER
#define rspfVpfTable_HEADER
#include <iostream>
#include <vector>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/vec/vpf.h>
class RSPF_DLL rspfVpfTable
{
public:
	friend RSPF_DLL std::ostream& operator <<(std::ostream& out,
                               const rspfVpfTable& data);
   
   rspfVpfTable();
   virtual ~rspfVpfTable();
   /*!
    * Will open the table.  Will return RSPF_OK if it was successful
    * or RSPF_ERROR if unsuccessful.
    */
   virtual bool openTable(const rspfFilename& tableName);
   /*!
    * Closes the table.
    */
   virtual void closeTable();
   /*!
    * This code is going to be cut paste from the vpf_dump_table
    * found in vpf_util/vpf_dump_table.c.  This function only allows
    * you to output to a file.  We need to re-write it to go to a stream.
    * the best way to do this is to cut and paste the code.
    */
   virtual void print(std::ostream& out)const;
  int getNumberOfRows()const;
  int getNumberOfColumns()const;
  rspfString getColumnName(int idx)const;
   virtual bool isClosed()const;
   /*!
    * This method will set the row pointer back to
    * the start of the table.
    */
   virtual void reset()const;
   virtual bool goToRow(long row)const;
   vpf_table_type* getVpfTableData()
      {
         return theTableInformation;
      }
	  std::vector<rspfString> getColumnValues(const rspfString& columnName)const;
	  std::vector<rspfString> getColumnValues(long columnNumber)const;
   
   rspfString getColumnValueAsString(row_type& row,
                                      long columnNumber)const;
   rspfString getColumnValueAsString(rspf_int32 rowNumber,
                                      long columnNumber)const;
   rspfString getColumnValueAsString(const rspfString& columnName);
   rspf_int32 getColumnPosition(const rspfString& columnName)const;
protected:
   /*!
    * this structure is in vpf_util/vpftable.h file.
    * it holds all the access information to the table.
    */
   vpf_table_type* theTableInformation;
   /*!
    * Will hold the complete path and name to this table.
    */
   rspfFilename   theTableName;
};
#endif
