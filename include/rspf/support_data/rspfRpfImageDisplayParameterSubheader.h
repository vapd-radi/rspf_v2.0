//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfImageDisplayParameterSubheader.h 9967 2006-11-29 02:01:23Z gpotts $
#ifndef rspfRpfImageDisplayParameterSubheader_HEADER
#define rspfRpfImageDisplayParameterSubheader_HEADER
#include <iostream>
using namespace std;

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfImageDisplayParameterSubheader
{
public:
   friend ostream& operator<<(ostream& out,
                              const rspfRpfImageDisplayParameterSubheader& data);
   rspfRpfImageDisplayParameterSubheader();
   rspfErrorCode parseStream(istream& in, rspfByteOrder byteOrder);
   
   void print(ostream& out)const;

   rspf_uint32 getStartOffset()const
      {
         return theStartOffset;
      }
   rspf_uint32 getEndOffset()const
      {
         return theEndOffset;
      }
   rspf_uint32 getNumberOfImageRows()const
      {
         return theNumberOfImageRows;
      }
   rspf_uint32 getNumberOfImageCodesPerRow()const
      {
         return theNumberOfImageCodesPerRow;
      }
   rspf_uint32 getImageCodeLength()const
      {
         return theImageCodeBitLength;
      }
   
private:
   void clearFields();

   rspf_uint32 theStartOffset;
   rspf_uint32 theEndOffset;
   
   rspf_uint32 theNumberOfImageRows;
   rspf_uint32 theNumberOfImageCodesPerRow;
   rspf_uint8  theImageCodeBitLength;
};

#endif
