//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class.
// 
//********************************************************************
// $Id: rspfRpfFrameFileIndexSubsection.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfFrameFileIndexSubsection_HEADER
#define rspfRpfFrameFileIndexSubsection_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/support_data/rspfRpfFrameFileIndexRecord.h>
#include <rspf/support_data/rspfRpfPathnameRecord.h>

class rspfFilename;

class rspfRpfFrameFileIndexSubsection : public rspfReferenced
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfFrameFileIndexSubsection& data);

   /** default constructor */
   rspfRpfFrameFileIndexSubsection();

   /** copy constructor */
   rspfRpfFrameFileIndexSubsection(const rspfRpfFrameFileIndexSubsection& obj);

   /** assignment operator */
   const rspfRpfFrameFileIndexSubsection& operator=(const rspfRpfFrameFileIndexSubsection& rhs);

   virtual ~rspfRpfFrameFileIndexSubsection();

   rspfErrorCode parseStream(std::istream &in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);
   
   void clearFields();

   std::ostream& print(std::ostream& out) const;
   
   void setNumberOfFileIndexRecords(rspf_uint32 numberOfIndexRecords);
   void setNumberOfPathnames(rspf_uint32 numberOfPathnames);
   
   const std::vector<rspfRpfFrameFileIndexRecord>& getIndexTable() const;
   const std::vector<rspfRpfPathnameRecord>& getPathnameTable() const;

   /**
    * @brief Gets the record matching file.
    *
    * @param file In the form of: 003H1U1B.I21
    *
    * @param record The record to initialize.
    *
    * @return true if matching record found, false if not.
    */
   bool getFrameFileIndexRecordFromFile(const rspfFilename& file,
                                        rspfRpfFrameFileIndexRecord& record) const;
   
private:
   std::vector<rspfRpfFrameFileIndexRecord> m_indexTable;

   std::vector<rspfRpfPathnameRecord> m_pathnameTable;
};

#endif
