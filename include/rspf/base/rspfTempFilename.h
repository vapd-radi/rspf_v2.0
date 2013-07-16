//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Description: This is a tmeporary filename class it will create a temporary
//              file and will also delete it upon destruction
//
//*************************************************************************
// $Id: rspfTempFilename.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfTempFilename_HEADER
#define rspfTempFilename_HEADER
#include <rspf/base/rspfFilename.h>

class RSPF_DLL rspfTempFilename : public rspfFilename
{
public:
   rspfTempFilename(const rspfString& tempDir="",
                     const rspfString& prefix = "",
                     const rspfString& extension = "",
                     bool autodelete=true,
                     bool useWildcardDelete=true);

   void generateRandomFile();
   void generateRandomDir();
   
   ~rspfTempFilename();
   
protected:
   rspfString theTempDir;
   rspfString thePrefix;
   rspfString theExtension;
   bool theAutoDeleteFlag;
   bool theWildCardDeleteFlag;
   

   void generate(bool createAsDirectoryFlag);
};
#endif
