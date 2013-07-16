//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfDirectory.cpp 20229 2011-11-08 17:01:17Z oscarkramer $

#include <cstring> /* for strncasecmp */
#include <iostream>

#if defined (_WIN32)
#include <io.h>
#include <direct.h>
#else
#include <rspf/base/rspfDirectoryData.h>
#endif
#ifdef __BORLANDC__
# include <dir.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#define _chdir chdir
#endif

#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfRegExp.h>

//#include "wx/filefn.h"          // for wxMatchWild

// #include <sys/types.h>

// #include <dirent.h>

// ----------------------------------------------------------------------------
// rspfDirectory construction/destruction
// ----------------------------------------------------------------------------

#if defined (_WIN32)
rspfDirectory::rspfDirectory()
   :
      theData(0),
      theDirectoryName(),
      theFlags(0)
{}

rspfDirectory::rspfDirectory(const rspfFilename &dirname)
   :
      theData(0),
      theDirectoryName(dirname),
      theFlags(0)
{
   open(dirname);
}

rspfDirectory::~rspfDirectory()
{
   if (theData != 0) _findclose( theData );
}

bool rspfDirectory::open(const rspfFilename &dirname)
{
   // close out currently open directory
   if (theData != 0)
   {
      _findclose( theData );
      theData = 0;
   }
   
   // set new directory name
   theDirectoryName = dirname;

	return (dirname.isDir());
   // cd to the new directory
//   if (_chdir( theDirectoryName) == -1)
//   {
//      return false;
//   }
   
//   return true;
}

bool rspfDirectory::getFirst(rspfFilename &filename, int flags)
{
   struct _finddata_t c_file;
   rspfFilename temp;
   //long theData1 = _findfirst( "*.*", &c_file );
   rspfFilename dirName = theDirectoryName.dirCat("*");
   if( (theData = _findfirst( dirName.c_str(), &c_file )) != 0L )
   {
      setFlags(flags);
      
      temp = theDirectoryName.dirCat(c_file.name);
      
	  
      while (!fileMatched(temp))
      {
         // look for next file in the directory
         if (_findnext( theData, &c_file ) == 0 )
         {
            temp = theDirectoryName.dirCat(c_file.name);
         }
         else
         {
            // no more file in the directory
            filename.clear();
            return false;
         }
      }
   }

   // set the filenane that matches
   filename = temp.trim();

   return (filename!="");
}

bool rspfDirectory::getNext(rspfFilename &filename) const
{
   struct _finddata_t c_file;
   bool matches = false;
   rspfFilename temp;

   while (!matches )
   {
      // look for next file in the directory
      if (_findnext( theData, &c_file ) == 0 )
      {
         temp = theDirectoryName.dirCat(c_file.name);
         matches = fileMatched(temp);
      }
      else
      {
         // no more file in the directory
         filename.clear();
         return false;
      }
   }

   // set the filenane that matches
   if (matches)
   {
      filename = temp.trim();
   }

   return (matches&&(filename!=""));
}

bool rspfDirectory::fileMatched(rspfFilename &filename) const
{
   bool matches = false;

   // Don't return "." and ".." unless asked for.
   if ( (filename.file() == "..") || (filename.file() == ".")   )
   {
      if (theFlags & rspfDirectory::RSPF_DIR_DOTDOT)
      {
         matches = true;
      }
   }
   else if((filename.isDir()) && (theFlags & rspfDirectory::RSPF_DIR_DIRS))
   {
      matches = true;
   }
   else if((filename.isFile()) && (theFlags & rspfDirectory::RSPF_DIR_FILES))
   {
      matches = true;
   }

   return matches;
}

bool rspfDirectory::isOpened() const
{
   return theDirectoryName.isDir();
    //return theData != 0;
}

#else

rspfDirectory::rspfDirectory()
   :
      theData(NULL)
{}

rspfDirectory::rspfDirectory(const rspfFilename &dirname)
{
    theData = NULL;
    open(dirname);
}

bool rspfDirectory::open(const rspfFilename &dirname)
{
    delete theData;
    theData = new rspfDirectoryData(dirname);

    if ( theData &&
         (!theData->isOk()) )
    {
        delete theData;
        theData = NULL;

        return false;
    }

    return true;
}

rspfDirectory::~rspfDirectory()
{
    delete theData;
}

// ----------------------------------------------------------------------------
// rspfDirectory enumerating
// ----------------------------------------------------------------------------

bool rspfDirectory::getFirst(rspfFilename &filename,
                              int flags)
{
   if(theData && isOpened())
   {
      theData->rewind();

      theData->setFlags(flags);

      return getNext(filename);
   }

   return false;
}

bool rspfDirectory::getNext(rspfFilename &filename) const
{
   if(theData && isOpened())
   {
      return theData->read(filename);
   }

   return false;
}

bool rspfDirectory::isOpened() const
{
    return theData != NULL;
}

#endif

void rspfDirectory::findAllFilesThatMatch(std::vector<rspfFilename>& result,
					   const rspfString& regularExpressionPattern,
					   int flags)
{
   rspfFilename filename;
   rspfRegExp   regExpr;
   regExpr.compile(regularExpressionPattern.c_str());
   if(getFirst(filename, flags))
   {
      do
      {
         rspfString fileOnly = filename.file();
         if(regExpr.find(fileOnly.c_str()))
         {
            result.push_back(filename);
         }
      }while(getNext(filename));
   }
}

// ESH 07/2008, Trac #234: RSPF is case sensitive 
// when using worldfile templates during ingest
bool rspfDirectory::findCaseInsensitiveEquivalents(
   const rspfFilename &filename, 
   std::vector<rspfFilename>& result,
   bool bExcludeExactMatch )
{
   bool bSuccess = false;
   rspfFilename candidate;
   bool bFoundCandidate = getFirst( candidate );
   int compareSize = static_cast<int>( filename.length() );
   
   while( bFoundCandidate == true )
   {
      // Do a case insensitive string compare
#if defined (_WIN32)
      bool bFoundEquivalent = _strnicmp( filename.c_str(), candidate.c_str(), 
                                         compareSize ) == 0 ? true : false;
#else
      //bool bFoundEquivalent =  strnicmp( filename.c_str(), candidate.c_str(), // 
      //                                         compareSize ) == 0 ? true : false;
      bool bFoundEquivalent =  strncasecmp( filename.c_str(), candidate.c_str(), 
                                            compareSize ) == 0 ? true : false;
#endif
      
      if ( bFoundEquivalent == true )
      {
         bool bFoundExact = ( filename == candidate.c_str() ) ? true : false;
         bool bShouldExclude = ( bFoundExact == true && 
                                 bExcludeExactMatch == true ) ? true : false;
         
         if ( bShouldExclude == false )
         {
            bSuccess = true;
            result.push_back( candidate );
         }
      }
      
      bFoundCandidate = getNext( candidate );
   }
   
   return bSuccess;
}
