//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
// Description: 
//
//*************************************************************************
// $Id: rspfDirectory.h 14776 2009-06-25 14:34:06Z dburken $
#ifndef rspfDirectory_HEADER
#define rspfDirectory_HEADER

#include <rspf/base/rspfFilename.h>

#ifndef _WIN32
class rspfDirectoryData;
#endif

class  RSPFDLLEXPORT rspfDirectory
{
public:
   
   /*!
    * These flags define what kind of filenames is included in the list
    * of files enumerated by GetFirst/GetNext.
    */
   enum
   {
      RSPF_DIR_FILES     = 0x0001,       // include files
      RSPF_DIR_DIRS      = 0x0002,       // include directories
      RSPF_DIR_HIDDEN    = 0x0004,       // include hidden files
      RSPF_DIR_DOTDOT    = 0x0008,       // include '.' and '..'
      
      // by default, enumerate everything except '.' and '..'
      RSPF_DIR_DEFAULT   = RSPF_DIR_FILES | RSPF_DIR_DIRS | RSPF_DIR_HIDDEN
   };


   rspfDirectory();
   
   rspfDirectory(const rspfFilename& dir);
   
   ~rspfDirectory();
   
   bool open(const rspfFilename& dir);
   
   bool isOpened() const;
   
   bool getFirst(rspfFilename &filename,
                 int flags = RSPF_DIR_DEFAULT);
   
   /*!
    * Get next file in the enumeration started with either GetFirst() or
    * GetFirstNormal().
    */
   bool getNext(rspfFilename &filename) const;
   
   void findAllFilesThatMatch(std::vector<rspfFilename>& result,
                              const rspfString& regularExpressionPattern,
                              int flags = RSPF_DIR_DEFAULT);
   
#if defined (_WIN32)
   void setFlags(int flags) { theFlags = flags; };
   bool fileMatched(rspfFilename &filename) const;
#endif    

   // ESH 07/2008, Trac #234: RSPF is case sensitive 
   // when using worldfile templates during ingest
   /*!
    * Case insensitive search for files with the same name but 
    * with letters that have different case than the input name. 
    *
    * On Windows there can only be one match, but the case can be 
    * different than the input name. On UNIX there can be multiple 
    * matches.
    *
    * If the bExcludeExactMatch input parameter is set to false, 
    * the original input name will be included in the 'result'
    * vector if it is found in the directory. Otherwise (the default),
    * the input name is excluded from the 'result' vector even if it
    * is found in the directory.
    *
    * Returns true if a name has been added to the result vector.
    */
   bool findCaseInsensitiveEquivalents( const rspfFilename &filename, 
                                        std::vector<rspfFilename>& result,
                                        bool bExcludeExactMatch = true );
   
   // TODO using scandir() when available later, emulating it otherwise
   
private:
#if defined (_WIN32)
   long          theData;
   rspfFilename theDirectoryName;
   int           theFlags;
#else
   rspfDirectoryData *theData;
#endif
};

#endif /* #ifndef rspfDirectory_HEADER */
