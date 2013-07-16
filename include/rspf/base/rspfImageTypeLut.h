//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Contains class declaration for ImageTypeLut.  Currently has mapping of
// ouput image writes from enumeration to string and string to
// enumeration.
//
//*******************************************************************
//  $Id: rspfImageTypeLut.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef ImageTypeLUT_HEADER
#define ImageTypeLUT_HEADER

#include <rspf/base/rspfLookUpTable.h>

class RSPFDLLEXPORT rspfImageTypeLut : public rspfLookUpTable
{
public:

   rspfImageTypeLut();

   virtual ~rspfImageTypeLut(){}

   virtual rspfKeyword getKeyword() const { return IMAGE_TYPE_KW; }

   static const rspfKeyword IMAGE_TYPE_KW;
};
#endif
