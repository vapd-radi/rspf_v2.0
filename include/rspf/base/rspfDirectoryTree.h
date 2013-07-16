//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks)
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfDirectoryTree.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfDirectoryTree_HEADER
#define rspfDirectoryTree_HEADER
#include <queue>
#include <vector>
using namespace std;

#include <rspf/base/rspfDirectory.h>

class RSPFDLLEXPORT rspfDirectoryTree
{
public:
   rspfDirectoryTree();
   ~rspfDirectoryTree();

   bool open(const rspfFilename& dir);

   bool isOpened() const;
   
   bool getFirst(rspfFilename &filename,
                 int flags = rspfDirectory::RSPF_DIR_DEFAULT);
   
   // get next file in the enumeration started with either GetFirst() or
   // GetFirstNormal()
   bool getNext(rspfFilename &filename) ;

  void findAllFilesThatMatch(std::vector<rspfFilename>& result,
			     const rspfString& regularExpressionPattern,
			     int flags = rspfDirectory::RSPF_DIR_DEFAULT);
private:
   class RSPFDLLEXPORT rspfDirData
   {
   public:
      ~rspfDirData()
         {
            if(theDirectory)
            {
               delete theDirectory;
               theFilename = "";
            }
         }
      rspfDirData(rspfDirectory *aDir,
                         const rspfFilename &filename)
                         :theDirectory(aDir),
                         theFilename(filename)
         {
            
         }
      rspfDirectory* theDirectory;
      rspfFilename   theFilename;
   };
   
   queue<rspfFilename> theDirectoryQueue;
   rspfDirData*        theCurrentDirectoryData;
   int                  theFlags;

   void deleteAll();

   void checkToPushDirectory(const rspfFilename &filename);
   // for now we will hide copy
   rspfDirectoryTree(const rspfDirectoryTree &rhs);
   const rspfDirectoryTree& operator =(const rspfDirectoryTree& rhs);
};

#endif
