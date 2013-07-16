//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
//
// Description: This class provides manipulation of filenames.
//
//*************************************************************************
// $Id: rspfFilename.h 20192 2011-10-25 17:27:25Z dburken $

#ifndef rspfFilename_HEADER
#define rspfFilename_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>

class rspfLocalTm;

class RSPF_DLL rspfFilename : public rspfString
{
public:
   enum AccessModes
   {
      RSPF_READ_WRITE = 6,
      RSPF_READ       = 4,
      RSPF_WRITE      = 2,
      RSPF_EXE	       = 1, // The PC docs say that this is ignored
      RSPF_EXIST      = 0
   };
   rspfFilename();
   rspfFilename(const rspfFilename& src);
   rspfFilename(const rspfString& src);
   rspfFilename(const std::string& src);
   rspfFilename(const char* src);

   template <class Iter> rspfFilename(Iter s, Iter e);

   static const rspfFilename NIL;

   bool operator == (const rspfFilename& rhs)const;
   
   bool operator == (const rspfString& rhs)const;
   bool operator == (const char* rhs)const;

   /** @brief Writes f to the output stream os. */
//    friend std::ostream& operator<<(std::ostream& os,
   //                                const rspfFilename& s);
   
   void convertBackToForwardSlashes();
   void convertForwardToBackSlashes();

   bool setTimes(rspfLocalTm* accessTime,
                 rspfLocalTm* modTime,
                 rspfLocalTm* createTime)const;
   bool getTimes(rspfLocalTm* accessTime,
                 rspfLocalTm* modTime,
                 rspfLocalTm* createTime)const;
   
   bool touch()const;
   
   /**
    * Method to do file name expansion.
    *
    * Like: ~/docs will be expanded to /home/user/docs
    *
    * @return The expanded file name.
    *
    * @note METHOD IS NOT COMPLETE YET.
    */
   rspfFilename expand() const;
   
   // Methods to test rspfFilename for various states.
   bool        exists()       const;
   bool        isFile()       const;
   bool        isDir()        const;
   bool        isReadable()   const;
   bool        isWriteable()  const;
   bool        isExecutable() const;
   rspf_int64 fileSize()     const;
   
   // Methods to access parts of the rspfFilename.

   /**
    * @return "tif" if file is "/data/images/t1.tif".
    *
    * @note The '.' (dot) is not returned.
    */
   rspfString   ext() const;

   /**
    * @return @return "/data/images" if file is "/data/images/t1.tif".
    */
   rspfFilename path() const;

   /**
    * @return @return "c:" if file is "c:\data\images\t1.tif".
    */
   rspfFilename drive() const;
   
   /**
    * @return @return "t1.tif" if file is "/data/images/t1.tif".
    */
   rspfFilename file() const;

   /**
    * @return @return "t1" if file is "/data/images/t1.tif".
    */
   rspfFilename fileNoExtension() const;

   /**
    * @return @return "/data/images/t1" if file is "/data/images/t1.tif".
    *
    * @note  The '.' (dot) is not returned.
    */
   rspfFilename noExtension() const;

   /**
    * Sets the extension of a file name.
    *
    * Given:
    * rspfFilename f = "foo";
    * f.setExtension("tif");
    * f now equals "foo.tif"
    *
    * Given: 
    * rspfFilename f = "foo.jpg";
    * f.setExtension("tif");
    * f now equals "foo.tif"
    * 
    * Given: 
    * rspfFilename f = "foo.jpg";
    * f.setExtension(".tif");
    * f now equals "foo.tif"
    *
    * Given: 
    * rspfFilename f = "foo.";
    * rspfFilename f2;
    * f2 = f.setExtension("tif");
    * f now equals "foo.tif"
    * f2 now equals "foo.tif"
    *
    * @param e Extension to add or replace.
    *
    * @returns a reference to this.
    */
   rspfFilename& setExtension(const rspfString& e);

   /**
	* Sets the file path and drive.
	* Input: the drive to be set (this should come from the user's preferences)
	* Example: "x:"
	* Given: "/filepath/file.ext"
    * @return @return "c:\filepath\file.ext".
    */
   rspfFilename& setDrive(const rspfString& d);
   rspfFilename& setPath(const rspfString& p);
   rspfFilename& setFile(const rspfString& f);
   
   void split(rspfString& drivePart,
              rspfString& pathPart,
              rspfString& filePart,
              rspfString& extPart)const;
   
   void merge(const rspfString& drivePart,
              const rspfString& pathPart,
              const rspfString& filePart,
              const rspfString& extPart);
   /*!
    * Returns file appended onto this string. Path seperator is always placed
    * between this and file.  Returns file if this string is empty.
    * Given: this = /foo  and file  = bar output  = /foo/bar
    * Given: this = /foo/ and file  = bar output  = /foo/bar
    */
   rspfFilename dirCat(const rspfFilename& file) const;

   /*!
    */
   bool createDirectory(bool recurseFlag=true,
                        int perm=0775)const;

   /*!
    * @brief Removes pathname from filesystem if supported by platform.
    *
    * If pathname is a directory it will only be removed if empty. If supported
    * will set errno and output equivalent string if one occurs.
    *
    * @return true on success, false on error.
    */
   static bool remove(const rspfFilename& pathname);

   /**
    * Will use a wildcard remove.  NOTE:  This is in
    * a regular expression format so if you want all
    * files with prefix myfile to be erased then you would give
    * it <path>/myfile.*
    *
    * The . here is a regular expression '.' character which says any character
    * and the * "is any number of"
    * 
    */ 
   static bool wildcardRemove(const rspfFilename& pathname);

   bool rename(const rspfFilename& destFile, bool overwriteDestinationFlag=true)const;
   
   bool remove()const;
   bool wildcardRemove()const;

   /**
    * @brief Copies this file to output file.
    *
    * Copies this to output file. If output file is a directory, this->file() is
    * appended to output file.
    * 
    * @param outputFile File name to copy to.
    * 
    * @return True on success, false on error.
    */
   bool copyFileTo(const rspfFilename& ouputFile) const;

   /**
    * @brief Checks whether file name is relative or absolute.
    * 
    * @return true if file name has a relative path, false if absolute.
    *
    * @note This will return true if file name is empty.
    */
   bool isRelative() const;
   
   /**
    * @brief Method to check if expansion is needed.
    *
    * This checks if file isRelative() first if isRelative is false (has
    * absolute path) it then walks the file and looks for '$'.
    * 
    * @return true if file name is relative or has environment vars.
    *
    * @note This will return false if file name is empty.
    */
   bool needsExpansion() const;

   /** @return The path separator. */
   char getPathSeparator() const;
   
protected:

   void convertToNative();

   /*!
    * since windows uses \ for path separation
    * and unix / we need to be consistent.
    */
   static const char thePathSeparator;

};

// inline std::ostream& operator<<(std::ostream& os, const rspfFilename& f)
// {
//   return os << f.string().c_str();
// }

#endif
