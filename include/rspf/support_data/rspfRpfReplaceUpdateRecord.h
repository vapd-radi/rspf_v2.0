//----------------------------------------------------------------------------
//
// File:     rspfRpfReplaceUpdateRecord.h
// 
// License:  See top level LICENSE.txt file.
// 
// Author:   David Burken
//
// Description: See class description.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfReplaceUpdateRecord.h 20324 2011-12-06 22:25:23Z dburken $

#ifndef rspfRpfReplaceUpdateRecord_HEADER
#define rspfRpfReplaceUpdateRecord_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <iosfwd>

/**
 * @class rspfRpfReplaceUpdateRecord
 * @brief RPF replace/update section subheader record.
 * See MIL-STD-2411 for detailed information.
 */
class rspfRpfReplaceUpdateRecord
{
public:

   /** @brief Convenience output operator. */
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfReplaceUpdateRecord& data);

   /** @brief default constructor */
   rspfRpfReplaceUpdateRecord();

   /** @brief copy constructor */
   rspfRpfReplaceUpdateRecord(const rspfRpfReplaceUpdateRecord& obj);

   /** @brief assignment operator */
   const rspfRpfReplaceUpdateRecord& operator=(const rspfRpfReplaceUpdateRecord& rhs);

   /**
    * @brief Method to parse the record.
    * Note no byte swapping required for this record.
    * @param in Stream sitting at record.
    */
   rspfErrorCode parseStream(std::istream& in);

   /**
    * @brief Write method.
    * Note no byte swapping required for this record.
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   /** @brief Clears fields. */
   void clearFields();

  /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix This will be prepended to key.
    * @param recordNumber This is added to key.  See below.
    * e.g. Where prefix = "nitf.rpf.",  recordNumber = 0 and key is "new_file"
    * key becomes: "nitf.rpf.replace_update_record0.new_file:"
    * @return output stream.
    */
 
   std::ostream& print( std::ostream& out,
                        const std::string& prefix=std::string(),
                        rspf_uint32 recordNumber=0) const;

   /**
    * @brief Gets new file field.
    * @param file Set by this.
    */
   void getNewFilename(std::string& file) const;

   /**
    * @brief Gets old file field.
    * @param file Set by this.
    */
   void getOldFilename(std::string& file) const;

   /** @return The update status field. */
   rspf_uint8 getUpdateStatus() const;

   /**
    * @brief Sets old file name 12 byte field.
    * @param file
    */
   void setNewFilename(const std::string& file);

   /**
    * @brief Sets new file name 12 byte field.
    * @param file
    */
   void setOldFilename(const std::string& file);

   /** @return Sets the update status field. */
   void setUpdateStatus(rspf_uint8 status);
   
private:
   
   /** This is a 12 byte asci field. */
   char m_newFile[13];

   /** This is a 12 byte asci field. */
   char m_oldFile[13];

   /** This is a 1 byte unsigned integer. */
   rspf_uint8 m_updateStatus;
};

#endif /* #ifndef rspfRpfReplaceUpdateRecord_HEADER */
