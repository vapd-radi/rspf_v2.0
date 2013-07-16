//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class.
// 
//********************************************************************
// $Id: rspfRpfPathnameRecord.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfPathnameRecord_HEADER
#define rspfRpfPathnameRecord_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfString.h>

class rspfRpfPathnameRecord
{
public:

   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfPathnameRecord& data);
   /** default constructor */
   rspfRpfPathnameRecord();

   /** copy constructor */
   rspfRpfPathnameRecord(const rspfRpfPathnameRecord& obj);

   /** assignment operator */
   const rspfRpfPathnameRecord& operator=(const rspfRpfPathnameRecord& rhs);

   void clearFields();

   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   std::ostream& print(std::ostream& out) const;

   rspfString getPathname()const;

   /**
    * @brief Set the pathname and length from string size.
    * @param path Path like: ./N03E030
    */
   void setPathName(const rspfString& path);
   
private:   
   rspf_uint16 m_length;
   rspfString  m_pathname;
};

#endif
