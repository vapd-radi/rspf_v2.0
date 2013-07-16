//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfFrameFileIndexRecord.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfFrameFileIndexRecord_HEADER
#define rspfRpfFrameFileIndexRecord_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfFilename.h>

class rspfRpfFrameFileIndexRecord
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfFrameFileIndexRecord& data);

   /** default constructor */
   rspfRpfFrameFileIndexRecord();

   /** copy constructor */
   rspfRpfFrameFileIndexRecord(const rspfRpfFrameFileIndexRecord& obj);

   /** assignment operator */
   const rspfRpfFrameFileIndexRecord& operator=(const rspfRpfFrameFileIndexRecord& rhs);
   
   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);
   
   void clearFields();
   
   std::ostream& print(std::ostream& out)const;
   
   rspf_uint16  getBoundaryRecNumber()const;
   rspf_uint16  getLocationRowNumber()const;
   rspf_uint16  getLocationColNumber()const;
   rspf_uint32  getPathnameRecordOffset()const;
   rspfFilename getFilename()const;

   /** @brief Sets the zero based entry number. */
   void setBoundaryRecNumber(rspf_uint16 entry);
   
   void setPathnameRecordOffset(rspf_uint32 offset);
   
private:
   rspf_uint16 m_boundaryRectRecordNumber;
   rspf_uint16 m_locationRowNumber;
   rspf_uint16 m_locationColumnNumber;
   rspf_uint32 m_pathnameRecordOffset;

   /*!
    * This is a 12 byte asci field.
    */
   char           m_filename[13];

   /*!
    * this is a 6 byte asci field.
    */
   char           m_geographicLocation[7];

   char           m_securityClassification;

   /*!
    * is a 2 byte field.
    */
   char           m_fileSecurityCountryCode[3];

   /*!
    * This is a 2 byte field.
    */
   char           m_fileSecurityReleaseMarking[3];
};

#endif
