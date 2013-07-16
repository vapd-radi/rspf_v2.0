//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfDirectoryTree.cpp 9966 2006-11-29 02:01:07Z gpotts $

#include <rspf/base/rspfDirectoryTree.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfRegExp.h>
rspfDirectoryTree::rspfDirectoryTree()
   :
      theCurrentDirectoryData(NULL),
      theFlags(0)
{
}

rspfDirectoryTree::~rspfDirectoryTree()
{
   deleteAll();
}

void rspfDirectoryTree::findAllFilesThatMatch(std::vector<rspfFilename>& result,
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
        if(regExpr.find(filename.c_str()))
        {
           result.push_back(filename);
        }
     }while(getNext(filename));
  }
}

bool rspfDirectoryTree::open(const rspfFilename& dir)
{
   if(theCurrentDirectoryData)
   {
      deleteAll();
   }

   theCurrentDirectoryData = new rspfDirData(new rspfDirectory, dir);
   theCurrentDirectoryData->theDirectory->open(dir);
   if(theCurrentDirectoryData->theDirectory->isOpened())
   {
      return true;
   }
   else
   {
      delete theCurrentDirectoryData;
      theCurrentDirectoryData = NULL;
   }

   return isOpened();
}

bool rspfDirectoryTree::isOpened() const
{
   if(theCurrentDirectoryData)
   {
      return theCurrentDirectoryData->theDirectory->isOpened();
   }
   
   return false;
}

bool rspfDirectoryTree::getFirst(rspfFilename &filename, int flags)
{
   bool result = false;
   theFlags    = flags | rspfDirectory::RSPF_DIR_DIRS;
   if(theCurrentDirectoryData && isOpened())
   {
      result = theCurrentDirectoryData->theDirectory->getFirst(filename,
                                                               flags);
      while(result&&filename.isDir())
      {
         checkToPushDirectory(filename);
         result = theCurrentDirectoryData->theDirectory->getNext(filename);
      }

      if(!result)
      {
         if(!theDirectoryQueue.empty())
         {
            rspfFilename newDir = theDirectoryQueue.front();
            theDirectoryQueue.pop();
            theCurrentDirectoryData->theDirectory->open(newDir);
            
            return getFirst(filename, flags);
         }
      }
   }
   
   return result;
}

// get next file in the enumeration started with either GetFirst() or
// GetFirstNormal()
bool rspfDirectoryTree::getNext(rspfFilename &filename)
{
   bool result = false;
   
   if(theCurrentDirectoryData)
   {
      result = theCurrentDirectoryData->theDirectory->getNext(filename);
      if(result)
      {
         checkToPushDirectory(filename);
      }
      else
      {
         if(!theDirectoryQueue.empty())
         {
            rspfFilename newDir = theDirectoryQueue.front();
            theDirectoryQueue.pop();
            theCurrentDirectoryData->theDirectory->open(newDir);
            return getFirst(filename);
         }
      }
   }
   
   return result;
}

void rspfDirectoryTree::deleteAll()
{
   if(theCurrentDirectoryData)
   {
      delete theCurrentDirectoryData;
      theCurrentDirectoryData = NULL;
   }
}

void rspfDirectoryTree::checkToPushDirectory(const rspfFilename &filename)
{
   if((filename.file().trim() == ".") ||
      (filename.file().trim() == ".."))
   {
      return;
   }
   else if(filename.isDir())
   {
      theDirectoryQueue.push(filename);
   }
}
