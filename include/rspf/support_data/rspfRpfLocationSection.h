//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfLocationSection.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfLocationSection_HEADER
#define rspfRpfLocationSection_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfRpfConstants.h>
#include <rspf/base/rspfErrorCodes.h>

//  10 byte record
class rspfRpfComponentLocationRecord
{
public:
   friend std::ostream& operator<<(
      std::ostream& out, const rspfRpfComponentLocationRecord& data);

   /** default constructor */
   rspfRpfComponentLocationRecord();

   /** copy constructor */
   rspfRpfComponentLocationRecord(const rspfRpfComponentLocationRecord& record);

   /** assignment operator */
   const rspfRpfComponentLocationRecord& operator=(const rspfRpfComponentLocationRecord& rhs);
   
   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;

   rspfErrorCode parseStream(std::istream& in, rspfByteOrder endianOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   /*!
    * The component is a 2-Byte unsigned value which
    * identitfies a level-2 component.  The table can be
    * found in MIL-STD-2411-1 in section 5.1.1 and in rspfRpfConstants.h.
    */
   rspf_uint16 m_componentId;

   /*!
    * Is a 4 byte unsigned integer indicating the length in
    * bytes of the component.
    */
   rspf_uint32  m_componentLength;

   /*!
    * Is a 4 byte value defining the absolute address (byte number)
    * of the first byte of the component measured
    * from the beginning of the RPF Frame file.
    */ 
   rspf_uint32  m_componentLocation;
};

class rspfRpfLocationSection
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfLocationSection &data);
   rspfRpfLocationSection();
   virtual ~rspfRpfLocationSection(){}

   virtual rspfErrorCode parseStream(std::istream& in,
                                      rspfByteOrder endianOrder);

   /**
    * @brief Write method.
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
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;

   /*!
    * Will let you know if a component exists within the location section.
    * these component Ids' are defined in the MIL-STD-2411-1 standards doc.
    * the rspfRpfComponentId is in rspfRpfConstants.h
    */
   bool hasComponent(rspfRpfComponentId componentId)const;

   bool getComponent(rspfRpfComponentId componentId,
                     rspfRpfComponentLocationRecord& result)const;

   /**
    * @brief Method to add a component location record.
    *
    * This will push the record onto the back of theComponentLocationList array.
    *
    * @param record Record to add to array.
    */
   void addComponentRecord(const rspfRpfComponentLocationRecord& record);

   void setLocationSectionLength(rspf_uint16 length);
   void setLocationTableOffset(rspf_uint32 offset);
   void setNumberOfComponentLocationRecords(rspf_uint16 count);
   void setLocationRecordLength(rspf_uint16 length);
   void setComponentAggregateLength(rspf_uint32 length);

   /** @brief Clears records and fields. Public interface to clearFields. */
   void clearFields();

   /** Brief Direct access to the list of records. */
   std::vector<rspfRpfComponentLocationRecord>& getLocationRecordList();
   
private:

   /*!
    * This is a 2 byte unsigned short >= 34 indicatin
    * the length in bytes of the entire location section
    */
   rspf_uint16 m_locationSectionLength;

   rspf_uint32 m_locationTableOffset;

   rspf_uint16 m_numberOfComponentLocationRecords;

   rspf_uint16 m_locationRecordLength;

   rspf_uint32 m_componentAggregateLength;

   /*!
    * Since there are not very many components within an Rpf file
    * it would be overkill to sort these components into an stl
    * map.  We will just use an array (stl vector) and do linear searches
    * for components when we need to.
    */
   std::vector<rspfRpfComponentLocationRecord> m_componentLocationList;
};

#endif
