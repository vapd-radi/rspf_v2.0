//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfFrameEntry.h 18052 2010-09-06 14:33:08Z dburken $
#ifndef rspfRpfFrameEntry_HEADER
#define rspfRpfFrameEntry_HEADER

#include <iosfwd>
#include <rspf/base/rspfFilename.h>

class rspfRpfFrameEntry
{
public:
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfRpfFrameEntry& data);
   rspfRpfFrameEntry(const rspfFilename& rootDirectory=rspfFilename(""),
                      const rspfFilename& pathToFrameFileFromRoot=rspfFilename(""));

   /** @brief copy constructor */
   rspfRpfFrameEntry(const rspfRpfFrameEntry& obj);

   /** @brief operator= */
   const rspfRpfFrameEntry& operator=(const rspfRpfFrameEntry& rhs);
   
   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;  

   bool exists() const;
   
   void setEntry(const rspfFilename& rootDirectory,
                 const rspfFilename& pathToFrameFileFromRoot);
   
   const rspfFilename& getFullPath() const;
   const rspfString&   getRootDirectory() const;
   const rspfString    getPathToFrameFileFromRoot() const;
   
private:
   bool          m_exists;
   rspfFilename m_rootDirectory;
   rspfFilename m_pathToFrameFileFromRoot;
   rspfFilename m_fullValidPath;
};

#endif
