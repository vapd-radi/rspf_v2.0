//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfBoundaryRectTable.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfBoundaryRectTable_HEADER
#define rspfRpfBoundaryRectTable_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/support_data/rspfRpfBoundaryRectRecord.h>

class rspfRpfBoundaryRectTable : public rspfReferenced
{
public:
   
   friend std::ostream& operator <<(std::ostream& out, const rspfRpfBoundaryRectTable& data);

   /** default constructor */
   rspfRpfBoundaryRectTable(rspf_uint32 numberOfEntries=0);

   /** copy constructor */
   rspfRpfBoundaryRectTable(const rspfRpfBoundaryRectTable& obj);

   /** assignment operator */
   const rspfRpfBoundaryRectTable& operator=(const rspfRpfBoundaryRectTable& rhs);
   
   virtual ~rspfRpfBoundaryRectTable();
   
   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);   

   void setNumberOfEntries(rspf_uint32 numberOfEntries);

   rspf_uint32 getNumberOfEntries() const;

   /**
    * @brief Gets record for entry.
    * @param entry Zero base entry to get.
    * @param record Record to initialize.
    * @return true on success, false if entry doesn't exist.
    */
   bool getEntry(rspf_uint32 entry, rspfRpfBoundaryRectRecord& record) const;

   std::ostream& print(std::ostream& out)const;
   
private:
   std::vector<rspfRpfBoundaryRectRecord> m_table;
};
#endif
