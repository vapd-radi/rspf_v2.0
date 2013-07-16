//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class definition for rspfScalarTypeLUT.
// 
//*******************************************************************
//  $Id: rspfScalarTypeLut.cpp 22072 2013-01-04 13:46:52Z dburken $

#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfString.h>

rspfScalarTypeLut* rspfScalarTypeLut::theInstance = NULL;

rspfScalarTypeLut* rspfScalarTypeLut::instance()
{
   if (!theInstance)
   {
      theInstance = new rspfScalarTypeLut;
   }
   return theInstance;
}

rspfScalarTypeLut::rspfScalarTypeLut()
{
   //---
   // Complete initialization of data member "theTable".
   // Note:  Scalar types defined in constants.h file.
   //---
   rspfKeyValueMap entry;
   entry.init(RSPF_SCALAR_UNKNOWN, "unknown");
   theTable.push_back(entry);
   
   entry.init(RSPF_UINT8, "rspf_uint8");
   theTable.push_back(entry);

   entry.init(RSPF_SINT8, "rspf_sint8");
   theTable.push_back(entry);

   entry.init(RSPF_UINT16, "rspf_uint16");
   theTable.push_back(entry);

   entry.init(RSPF_SINT16, "rspf_sint16");
   theTable.push_back(entry);

   entry.init(RSPF_UINT32, "rspf_uint32");
   theTable.push_back(entry);

   entry.init(RSPF_SINT32, "rspf_sint32");
   theTable.push_back(entry);

   entry.init(RSPF_UINT64, "rspf_uint64");
   theTable.push_back(entry);

   entry.init(RSPF_SINT64, "rspf_sint64");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT32, "rspf_float32");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT64, "rspf_float64");
   theTable.push_back(entry);

   entry.init(RSPF_CINT16, "rspf_cint16");
   theTable.push_back(entry);

   entry.init(RSPF_CINT32, "rspf_cint32");
   theTable.push_back(entry);

   entry.init(RSPF_CFLOAT32, "rspf_cfloat32");
   theTable.push_back(entry);

   entry.init(RSPF_CFLOAT64, "rspf_cfloat64");
   theTable.push_back(entry);
   
   entry.init(RSPF_UCHAR, "uchar");
   theTable.push_back(entry);

   entry.init(RSPF_USHORT16, "ushort16");
   theTable.push_back(entry);

   entry.init(RSPF_SSHORT16, "sshort16");
   theTable.push_back(entry);

   entry.init(RSPF_USHORT11, "ushort11");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT, "float");
   theTable.push_back(entry);

   entry.init(RSPF_NORMALIZED_FLOAT, "normalized_float");
   theTable.push_back(entry);

   entry.init(RSPF_NORMALIZED_DOUBLE, "normalized_double");
   theTable.push_back(entry);

   entry.init(RSPF_DOUBLE, "double");
   theTable.push_back(entry);

   // Short forms:

   entry.init(RSPF_UINT8, "U8");
   theTable.push_back(entry);

   entry.init(RSPF_SINT8, "S8");
   theTable.push_back(entry);

   entry.init(RSPF_USHORT11, "U11");
   theTable.push_back(entry);

   entry.init(RSPF_UINT16, "U16");
   theTable.push_back(entry);

   entry.init(RSPF_SINT16, "S16");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT32, "F32");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT64, "F64");
   theTable.push_back(entry);
   
   entry.init(RSPF_NORMALIZED_FLOAT, "N32");
   theTable.push_back(entry);

   entry.init(RSPF_NORMALIZED_DOUBLE, "N64");
   theTable.push_back(entry);

   // Forms from old code "radiometry" key in rspfImageMetaData:
   
   entry.init(RSPF_UINT8, "8-bit");
   theTable.push_back(entry);

   entry.init(RSPF_USHORT11, "11-bit");
   theTable.push_back(entry);

   entry.init(RSPF_UINT16, "16-bit unsigned");
   theTable.push_back(entry);

   entry.init(RSPF_SINT16, "16-bit signed");
   theTable.push_back(entry);
   
   entry.init(RSPF_UINT32, "32-bit unsigned");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT32, "float");
   theTable.push_back(entry);
   
   entry.init(RSPF_NORMALIZED_FLOAT, "normalized float");
   theTable.push_back(entry);

   entry.init(RSPF_FLOAT64, "double");
   theTable.push_back(entry);
   
   entry.init(RSPF_NORMALIZED_DOUBLE, "normalized double");
   theTable.push_back(entry);
}

rspfScalarTypeLut::~rspfScalarTypeLut()
{
   theInstance = NULL;
}

rspfScalarType
rspfScalarTypeLut::getScalarTypeFromString(const rspfString& s) const
{
   int scalar_type = getEntryNumber(s.c_str());
   
   if (scalar_type == -1)
   {
      return RSPF_SCALAR_UNKNOWN;
   }
   
   return static_cast<rspfScalarType>(scalar_type);
}   

rspfKeyword rspfScalarTypeLut::getKeyword() const
{
   return rspfKeyword((rspfKeywordNames::SCALAR_TYPE_KW), "");
}
