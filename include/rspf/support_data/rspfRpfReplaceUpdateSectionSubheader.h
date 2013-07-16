//----------------------------------------------------------------------------
//
// File:     rspfRpfReplaceUpdateSectionSubheader.h
// 
// License:  See top level LICENSE.txt file.
// 
// Author:   David Burken
//
// Description: See class description.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfReplaceUpdateSectionSubheader.h 20324 2011-12-06 22:25:23Z dburken $

#ifndef rspfRpfReplaceUpdateSubheader_Header
#define rspfRpfReplaceUpdateSubheader_Header 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfReferenced.h>
#include <iosfwd>

/**
 * @class rspfRpfReplaceUpdateSectionSubheader
 * @brief RPF replace/update section subheader record.
 * See MIL-STD-2411 for detailed information.
 */
class rspfRpfReplaceUpdateSectionSubheader : public rspfReferenced
{
public:

   /** @brief Convenience output operator. */
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfRpfReplaceUpdateSectionSubheader& data);

   /** @brief default constructor */
   rspfRpfReplaceUpdateSectionSubheader();

   /** @brief copy constructor */
   rspfRpfReplaceUpdateSectionSubheader(const rspfRpfReplaceUpdateSectionSubheader& obj);

   /** @brief assignment operator */
   const rspfRpfReplaceUpdateSectionSubheader& operator=(
      const rspfRpfReplaceUpdateSectionSubheader& rhs);

   /** @brief virtual destructor */
   virtual ~rspfRpfReplaceUpdateSectionSubheader();

   /**
    * @brief Method to parse the record.
    * @param in Stream sitting at record.
    * @param byteOrder Byte order of system.
    */   
   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * Note always writes out in big endian at this point.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out) const;

   /** @return Offset to first record. */
   rspf_uint32 getOffset() const;

   /** @return Number of records. */
   rspf_uint16 getNumberOfRecords() const;

   /** @return Length of one record. */
   rspf_uint16 getRecordLength() const;

   /**
    * @brief Sets number of records.
    * @param count
    */
   void setNumberOfRecords(rspf_uint16 count);

   void setRecordLength(rspf_uint16 length);
   void clearFields();
   
private:
   rspf_uint32  m_tableOffset;
   rspf_uint16  m_numberOfRecords;
   rspf_uint16  m_recordLength;
};



#endif /* #ifndef rspfRpfReplaceUpdateSubheader_Header */
