#ifndef rspfRpfColorConverterOffsetRecord_HEADER
#define rspfRpfColorConverterOffsetRecord_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>


class rspfRpfColorConverterOffsetRecord
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfColorConverterOffsetRecord& data);
   rspfRpfColorConverterOffsetRecord();
   
   rspfErrorCode parseStream(istream& in,
                              rspfByteOrder byteOrder);

   void clearFields();
   /*!
    * Is a two byte field as is described in Mil-STD-2411-1.
    * The id can be one of:
    *
    * 1       CCT/1--->1
    * 2       CCT/1--->2
    * 3       CCT/1--->3
    * 4       CCT/1--->4
    * 5       CCT/2--->2
    * 6       CCT/2--->3
    * 7       CCT/2--->4
    * 8       CCT/3--->3
    *
    * where CCT stands for Color Converter Table
    */
   rspf_uint16 theColorConverterTableId;

   /*!
    * Is a 4 byte field.
    */
   rspf_uint32  theNumberOfColorConverterRecords;

   /*!
    * Is a 4 byte field indicating the offset from the start of
    * the color converter subsection to the color converter table.
    */
   rspf_uint32  theColorConverterTableOffset;

   /*!
    * Is a 4 byte field indicating the offset from the colormap
    * subsection to the source color/grayscale offset table.
    */
   rspf_uint32  theSourceColorGrayscaleOffsetTableOffset;

   /*!
    * Is a 4 byte field indicating the offset from the colormap
    * subsection to the color/graysscale offset table.
    */
   rspf_uint32  theTargetColorGrayscaleOffsetTableOffset;
};

#endif
