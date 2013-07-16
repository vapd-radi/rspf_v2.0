//*******************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: This class extends the stl's string class.
//
//********************************************************************
// $Id: rspfRpfImageDisplayParameterSubheader.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/support_data/rspfRpfImageDisplayParameterSubheader.h>

#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator<<(ostream& out,
                    const rspfRpfImageDisplayParameterSubheader& data)
{
   data.print(out);

   return out;
}

rspfRpfImageDisplayParameterSubheader::rspfRpfImageDisplayParameterSubheader()
{
   clearFields();
}

rspfErrorCode rspfRpfImageDisplayParameterSubheader::parseStream(istream& in,
                                                         rspfByteOrder byteOrder)
{
   clearFields();
   if(in)
   {
      theStartOffset = in.tellg();
      in.read((char*)&theNumberOfImageRows, 4);
      in.read((char*)&theNumberOfImageCodesPerRow, 4);
      in.read((char*)&theImageCodeBitLength, 1);
      theEndOffset = in.tellg();

      rspfEndian anEndian;
      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theNumberOfImageRows);
         anEndian.swap(theNumberOfImageCodesPerRow);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}
   
void rspfRpfImageDisplayParameterSubheader::print(ostream& out)const
{
   out << "theNumberOfImageRows:          " << theNumberOfImageRows << endl
       << "theNumberOfImageCodesPerRow:   " << theNumberOfImageCodesPerRow << endl
       << "theImageCodeBitLength:         " << (unsigned long)theImageCodeBitLength;
}

void rspfRpfImageDisplayParameterSubheader::clearFields()
{
   theNumberOfImageRows        = 0;
   theNumberOfImageCodesPerRow = 0;
   theImageCodeBitLength       = 0;
}
