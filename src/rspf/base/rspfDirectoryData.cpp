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
// $Id: rspfDirectoryData.cpp 9966 2006-11-29 02:01:07Z gpotts $

#ifndef _MSC_VER

#include <sys/types.h>
#include <iostream>
using namespace std;

#include <rspf/base/rspfDirectoryData.h>
#include <rspf/base/rspfDirectory.h>

// ----------------------------------------------------------------------------
// rspfDirectoryData
// ----------------------------------------------------------------------------

#if !defined( __VMS__ ) || ( __VMS_VER >= 70000000 )

rspfDirectoryData::rspfDirectoryData(const rspfFilename& dirname)
         : theDirectoryName(dirname)
{
    theDir = NULL;

    // throw away the trailing slashes
    size_t n = theDirectoryName.length();
    if(n > 0)
    {

       while ( n > 0 && theDirectoryName[--n] == '/' )
        ;

//       theDirectoryName.Truncate(n + 1);

    // do open the dir
       theDir = opendir(theDirectoryName.c_str());
    }
}

rspfDirectoryData::~rspfDirectoryData()
{
   if ( theDir )
   {
      if ( closedir(theDir) != 0 )
      {
         // ERROR closing directory
      }
   }
}

bool rspfDirectoryData::read(rspfFilename &filename)
{
   filename = rspfFilename::NIL;
   dirent *de = (dirent *)NULL;    // just to silent compiler warnings
   bool matches = false;
   rspfFilename temp;

   while ( !matches )
   {
      de = readdir(theDir);
      if ( !de )
      {
         return false;
      }
      
      temp = theDirectoryName.dirCat(de->d_name);

      //***
      // NOTE:
      // The check for "." and ".." did not work so added pattern match check.
      //***
      
      // Check for "." and ".." match at the end of the name.
      rspfString s = temp.match("\\.$|\\.\\.$"); 
      
      // don't return "." and ".." unless asked for
      if ( (temp == "..") ||
           (temp == ".")  ||
           (s    == ".")  ||
           (s    == "..") )
      {
         if (theFlags & rspfDirectory::RSPF_DIR_DOTDOT)
         {
            matches = true;
         }
      }
      else if((temp.isDir()) &&
              (theFlags & rspfDirectory::RSPF_DIR_DIRS))
      {
         matches = true;
      }
      else if((temp.isFile()) &&
              (theFlags & rspfDirectory::RSPF_DIR_FILES))
      {
         matches = true;
      }
      else
      {
         matches = false;
      }
   }
   
   if (matches)
   {
      filename = temp;//de->d_name;
   }
   
   return matches;
}

#else // old VMS (TODO)

rspfDirectoryData::rspfDirectoryData(const rspfFilename& WXUNUSED(dirname))
{
}

rspfDirectoryData::~rspfDirectoryData()
{
}

bool rspfDirectoryData::read(rspfFilename &filename)
{
    return false;
}

#endif // not or new VMS/old VMS

void rspfDirectoryData::setFlags(int flags)
{
   theFlags = flags;
}

int rspfDirectoryData::getFlags() const
{
   return theFlags;
}

void rspfDirectoryData::rewind()
{
   rewinddir(theDir);
}

#endif /* #ifndef _MSC_VER */

 
