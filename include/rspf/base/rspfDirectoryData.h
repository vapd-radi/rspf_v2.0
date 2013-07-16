//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks)
// 
// Description:
// Class designed to be used by rspfDirectory class.  Should not be used
// directly.
//
//*************************************************************************
// $Id: rspfDirectoryData.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfDirectoryData_HEADER
#define rspfDirectoryData_HEADER

#ifndef _MSC_VER

#include <dirent.h>
#include <rspf/base/rspfFilename.h>

class RSPFDLLEXPORT rspfDirectoryData
{
 public:
   rspfDirectoryData(const rspfFilename& dirname);
   ~rspfDirectoryData();
   
   bool isOk() const { return theDir != NULL; }
   
   void setFlags(int flags);
   int  getFlags() const;
   
   void rewind();
   bool read(rspfFilename &filename);
   
 private:
   DIR*          theDir;
   rspfFilename theDirectoryName;
   int           theFlags;
};

#endif /* #ifndef _MSC_VER */

#endif /* #ifndef rspfDirectoryData_HEADER */
