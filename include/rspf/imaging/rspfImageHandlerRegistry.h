//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
// Description:  Contains class definition for the class
//               ImageHandlerRegistry.
//
//*******************************************************************
//  $Id: rspfImageHandlerRegistry.h 22228 2013-04-12 14:11:45Z dburken $

#ifndef rspfImageHandlerRegistry_HEADER
#define rspfImageHandlerRegistry_HEADER 1

#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/imaging/rspfImageHandlerFactoryBase.h>
#include <rspf/base/rspfFactoryListInterface.h>
#include <vector>

class rspfImageHandler;
class rspfFilename;
class rspfKeywordlist;

class RSPFDLLEXPORT rspfImageHandlerRegistry : public rspfObjectFactory,
                                                public rspfFactoryListInterface<rspfImageHandlerFactoryBase, rspfImageHandler>
{
public:
   virtual ~rspfImageHandlerRegistry();
   
   static rspfImageHandlerRegistry* instance();
   

   /**
    * @brief open that takes a filename.
    * @param fileName File to open.
    * @param trySuffixFirst If true calls code to try to open by suffix first,
    * then goes through the list of available handlers. default=true.
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    * @return Pointer to image handler or null if cannot open.
    */
   virtual rspfImageHandler* open(const rspfFilename& fileName,
                                   bool trySuffixFirst=true,
                                   bool openOverview=true)const;
   
   /**
    *  Given a keyword list return a pointer to an ImageHandler.  Returns
    *  null if a valid handler cannot be found.
    */
   virtual rspfImageHandler* open(const rspfKeywordlist& kwl,
                                   const char* prefix=0)const;
   
   /**
    * @brief Open overview that takes a file name.
    *
    * This will only check readers that can be overview handlers.
    * 
    * @param file File to open.
    * 
    * @return rspfRefPtr to image handler on success or null on failure.
    */
   virtual rspfRefPtr<rspfImageHandler> openOverview(
      const rspfFilename& file ) const;

   /*!
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName) const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;

   /**
    * openBySuffix will call the mthod getImageHandlersBySuffix and go through
    * each handler to try and open the file.  This should be a faster open
    * for we do not have to do a magic number compare on all prior files and
    * keep opening and closing files.
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    */
   virtual rspfRefPtr<rspfImageHandler> openBySuffix(const rspfFilename& file,
                                                       bool openOverview=true)const; 
   
   /**
    *
    * Will add to the result list any handler that supports the passed in extensions
    *
    */
   virtual void getImageHandlersBySuffix(rspfImageHandlerFactoryBase::ImageHandlerList& result,
                                         const rspfString& ext)const;
   /**
    *
    * Will add to the result list and handler that supports the passed in mime type
    *
    */
   virtual void getImageHandlersByMimeType(rspfImageHandlerFactoryBase::ImageHandlerList& result,
                                           const rspfString& mimeType)const;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList( std::vector<rspfString>& typeList ) const;

   virtual void getSupportedExtensions(
      rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const;

   /**
    * @brief Prints list of readers and properties.
    * @param  out Stream to print to.
    * @return std::ostream&
    */
   std::ostream& printReaderProps(std::ostream& out) const;
   
protected:
   rspfImageHandlerRegistry();
   rspfImageHandlerRegistry(const rspfImageHandlerRegistry& rhs);
   const rspfImageHandlerRegistry&
      operator=(const rspfImageHandlerRegistry& rhs);
   
   //static rspfImageHandlerRegistry*            theInstance;
   
TYPE_DATA
};

extern "C"
{
   RSPF_DLL  void* rspfImageHandlerRegistryGetInstance();
}

#endif /* #ifndef rspfImageHandlerRegistry_HEADER */
