//*******************************************************************
// Copyright (C) 2004 Garrett Potts.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNitfNameConversionTables.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNitfNameConversionTables_HEADER
#define rspfNitfNameConversionTables_HEADER
#include <rspf/base/rspfConstants.h>

class rspfString;

class RSPF_DLL rspfNitfNameConversionTables
{
public:
   rspfNitfNameConversionTables();

   rspfString convertMapProjectionNameToNitfCode(const rspfString& mapProjectionName)const;
   rspfString convertNitfCodeToOssimProjectionName(const rspfString& nitfProjectionCode)const;
   rspfString convertNitfCodeToNitfProjectionName(const rspfString& nitfProjectionCode)const;
   rspfString convertNitfProjectionNameToNitfCode(const rspfString& nitfProjectionName)const;
};

#endif
