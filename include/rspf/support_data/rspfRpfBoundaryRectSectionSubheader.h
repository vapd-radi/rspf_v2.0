//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfBoundaryRectSectionSubheader.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfBoundaryRectSectionSubheader_HEADER
#define rspfRpfBoundaryRectSectionSubheader_HEADER

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfReferenced.h>

class rspfRpfBoundaryRectSectionSubheader : public rspfReferenced
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfBoundaryRectSectionSubheader &data);

   /** default constructor */
   rspfRpfBoundaryRectSectionSubheader();

   /** copy constructor */
   rspfRpfBoundaryRectSectionSubheader(const rspfRpfBoundaryRectSectionSubheader& obj);

   /** assignment operator */
   const rspfRpfBoundaryRectSectionSubheader& operator=(
      const rspfRpfBoundaryRectSectionSubheader& rhs);

   virtual ~rspfRpfBoundaryRectSectionSubheader();

   rspfErrorCode parseStream(std::istream& in, rspfByteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   /**
    * @brief print method
    * @param out String to output to.
    * @return output stream.
    */
   std::ostream& print(std::ostream& out) const;

   rspf_uint16 getNumberOfEntries() const;
   rspf_uint32 getTableOffset() const;
   rspf_uint16 getLengthOfEachEntry() const;

   void setNumberOfEntries(rspf_uint16 entries);
   void setTableOffset(rspf_uint32 offset);
   void setLengthOfEachEntry(rspf_uint16 length);
   
   void clearFields();
   
private:
   
   rspf_uint32 m_rectangleTableOffset;
   rspf_uint16 m_numberOfEntries;
   rspf_uint16 m_lengthOfEachEntry;
};

#endif
