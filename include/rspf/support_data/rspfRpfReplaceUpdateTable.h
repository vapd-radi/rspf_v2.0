//----------------------------------------------------------------------------
//
// File:     rspfRpfReplaceUpdateTable.h
// 
// License:  See top level LICENSE.txt file.
// 
// Author:   David Burken
//
// Description: See class description.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfReplaceUpdateTable.h 20328 2011-12-07 17:43:24Z dburken $

#ifndef rspfRpfReplaceUpdateTable_HEADER
#define rspfRpfReplaceUpdateTable_HEADER 1

#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfRpfReplaceUpdateRecord.h>
#include <iosfwd>
#include <string>
#include <vector>

/**
 * @class rspfRpfReplaceUpdateTable
 * @brief Holds a table of RPF replace/update section subheader records.
 * See MIL-STD-2411 for detailed information.
 */
class RSPF_DLL rspfRpfReplaceUpdateTable : public rspfReferenced
{
public:

   /** @brief default constructor */
   rspfRpfReplaceUpdateTable();

   /** @brief copy constructor */
   rspfRpfReplaceUpdateTable(const rspfRpfReplaceUpdateTable& obj);

   /** @brief assignment operator */
   const rspfRpfReplaceUpdateTable& operator=(const rspfRpfReplaceUpdateTable& rhs);

   /**
    * @brief Method to add a record.
    * @param record to add.
    */
   void addRecord(const rspfRpfReplaceUpdateRecord& record);

   /** @brief Clears the table. */
   void clear();

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf.rpf." and key is "new_file" key becomes:
    * "nitf.rpf.replace_update_record0.new_file:
    * @return output stream.
    */
   std::ostream& print( std::ostream& out,
                        const std::string& prefix=std::string() ) const;
   
private:

   std::vector<rspfRpfReplaceUpdateRecord> m_table;
};

#endif /* #ifndef rspfRpfReplaceUpdateTable_HEADER */
