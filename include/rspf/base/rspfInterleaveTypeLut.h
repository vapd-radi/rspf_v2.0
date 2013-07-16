//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Contains class declaration for InterleaveTypeLUT.
//*******************************************************************
//  $Id: rspfInterleaveTypeLut.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfInterleaveTypeLUT_HEADER
#define rspfInterleaveTypeLUT_HEADER

#include <rspf/base/rspfLookUpTable.h>
#include <rspf/base/rspfKeywordNames.h>

class RSPFDLLEXPORT rspfInterleaveTypeLut : public rspfLookUpTable
{
public:

   rspfInterleaveTypeLut();

   virtual ~rspfInterleaveTypeLut(){}

   virtual rspfKeyword getKeyword() const
      { return rspfKeyword(rspfKeywordNames::INTERLEAVE_TYPE_KW, ""); }
};

#endif
