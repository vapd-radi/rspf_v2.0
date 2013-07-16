//*******************************************************************
// Copyright (C) 2004 Garrett Potts
//
// LICENSE: LGPL see top level LICENSE.txt for more details
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfVqCompressionHeader.h 18413 2010-11-11 19:56:22Z gpotts $
#ifndef rspfNitfVqCompressionHeader_HEADER
#define rspfNitfVqCompressionHeader_HEADER

#include <vector>
#include <rspf/support_data/rspfNitfCompressionHeader.h>

class RSPF_DLL rspfNitfVqCompressionOffsetTableData
{
public:

   rspfNitfVqCompressionOffsetTableData();
   rspfNitfVqCompressionOffsetTableData(const rspfNitfVqCompressionOffsetTableData& rhs);
   ~rspfNitfVqCompressionOffsetTableData();
   const rspfNitfVqCompressionOffsetTableData& operator =(const rspfNitfVqCompressionOffsetTableData& rhs);
   rspf_uint32 getDataLengthInBytes()const;
   void clearFields();
   void parseStream(std::istream& in);
   
   rspf_uint16 theTableId;
   rspf_uint32 theNumberOfCompressionLookupRecords;
   rspf_uint16 theNumberOfValuesPerCompressionLookup;
   rspf_uint16 theCompressionLookupValueBitLength;
   rspf_uint32 theCompressionLookupTableOffset;
   rspf_uint8* theData;
};

class RSPF_DLL rspfNitfVqCompressionHeader : public rspfNitfCompressionHeader
{
public:
   rspfNitfVqCompressionHeader();
   virtual void parseStream(std::istream &in);

   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;  

   virtual bool saveState(rspfKeywordlist& kwl, const rspfString& prefix="")const;
   virtual rspf_uint32 getBlockSizeInBytes()const;
   virtual rspf_uint32 getNumberOfImageRows()const;
   virtual rspf_uint32 getNumberOfImageCodesPerRow()const;
   virtual rspf_uint32 getCompressionAlgorithmId()const;
   virtual rspf_uint32 getImageCodeBitLength()const;
   virtual rspf_uint32 getNumberOfTables()const;
   const std::vector<rspfNitfVqCompressionOffsetTableData>& getTable()const;
   
protected:
   rspf_uint32 theNumberOfImageRows;
   rspf_uint32 theNumberOfImageCodesPerRow;
   rspf_uint8  theImageCodeBitLength;

   rspf_uint16 theCompressionAlgorithmId;
   rspf_uint16 theNumberOfCompressionLookupOffsetRecords;
   rspf_uint16 theNumberOfCompressionParameterOffsetRecords;

   rspf_uint32 theCompressionLookupOffsetTableOffset;
   rspf_uint16 theCompressionLookupTableOffsetRecordLength;

   std::vector<rspfNitfVqCompressionOffsetTableData> theTable;
   void clearFields();

TYPE_DATA   
};

#endif
