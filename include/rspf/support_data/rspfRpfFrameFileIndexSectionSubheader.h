//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfFrameFileIndexSectionSubheader.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfFrameFileIndexSectionSubheader_HEADER
#define rspfRpfFrameFileIndexSectionSubheader_HEADER

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfReferenced.h>

class rspfRpfFrameFileIndexSectionSubheader : public rspfReferenced
{
public:
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfRpfFrameFileIndexSectionSubheader& data);

   /** default constructor */
   rspfRpfFrameFileIndexSectionSubheader();

   /** copy constructor */
   rspfRpfFrameFileIndexSectionSubheader(const rspfRpfFrameFileIndexSectionSubheader& obj);

   /** assignment operator */
   const rspfRpfFrameFileIndexSectionSubheader& operator=(
      const rspfRpfFrameFileIndexSectionSubheader& rhs);

   virtual ~rspfRpfFrameFileIndexSectionSubheader();

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

   rspf_uint32 getOffset() const;
   rspf_uint32 getNumberOfIndexRecords() const;
   rspf_uint16 getNumberOfPathnameRecords() const;
   rspf_uint16 getIndexRecordLength() const;

   void setNumberOfIndexRecords(rspf_uint32 count);
   void setNumberOfPathnameRecords(rspf_uint16 count);
   void setIndexRecordLength(rspf_uint16 length);

   void clearFields();
private:
   char          m_highestSecurityClassification;
   rspf_uint32  m_indexTableOffset;
   rspf_uint32  m_numberOfIndexRecords;
   rspf_uint16  m_numberOfPathnameRecords;
   rspf_uint16  m_indexRecordLength;
};

#endif
