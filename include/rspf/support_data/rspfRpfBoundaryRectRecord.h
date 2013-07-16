//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfBoundaryRectRecord.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfBoundaryRectRecord_HEADER
#define rspfRpfBoundaryRectRecord_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfString.h>
#include <rspf/support_data/rspfRpfCoverageSection.h>

class rspfRpfBoundaryRectRecord
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfRpfBoundaryRectRecord& data);

   /** default constructor */
   rspfRpfBoundaryRectRecord();

   /** copy constructor */
   rspfRpfBoundaryRectRecord(const rspfRpfBoundaryRectRecord& ojb);

   /** assignment operator */
   const rspfRpfBoundaryRectRecord& operator=(const rspfRpfBoundaryRectRecord& rhs);

   /** destructor */
   ~rspfRpfBoundaryRectRecord();
   
   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);   
   
   void clearFields();

   rspfString getProductType()const{return rspfString(m_productDataType);}
   rspfString getScale()const{return rspfString(m_scale);}
   rspfString getCompressionRatio()const{return rspfString(m_compressionRatio);}
   
   rspf_uint32 getNumberOfFramesVertical()const{ return m_numberOfFramesNorthSouth;}
   rspf_uint32 getNumberOfFramesHorizontal()const{ return m_numberOfFramesEastWest;}

   const rspfRpfCoverageSection& getCoverage()const{return m_coverage;}

   void setCoverage(const rspfRpfCoverageSection& coverage);
   
   char getZone()const{return m_zone;}

   /**
    * @brief prints to out.
    * @param out Stream to print to.
    * @param prefix If not empty will be prepended onto the keyword.
    */
   std::ostream& print(std::ostream& out, rspfString prefix) const;

private:
   /*!
    * a five byte asci field
    */
   char m_productDataType[6];

   /*!
    * five byte asci field
    */
   char m_compressionRatio[6];

   /*!
    * 12 byte asci field.
    */
   char m_scale[13];

   /*!
    *
    */
   char m_zone;

   /*!
    * is a 5 byte asci field.
    */
   char m_producer[6];

   rspfRpfCoverageSection m_coverage;

   /*!
    * Four byte fields.
    */
   rspf_uint32 m_numberOfFramesNorthSouth;

   /*!
    * Four byte fields.
    */
   rspf_uint32 m_numberOfFramesEastWest;   
};

#endif
