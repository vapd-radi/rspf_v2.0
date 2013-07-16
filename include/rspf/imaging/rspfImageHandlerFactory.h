//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfImageHandlerFactoryMaker.
//*******************************************************************
// $Id: rspfImageHandlerFactory.h 22228 2013-04-12 14:11:45Z dburken $

#ifndef rspfImageHandlerFactory_HEADER
#define rspfImageHandlerFactory_HEADER 1

#include <rspf/imaging/rspfImageHandlerFactoryBase.h>
#include <rspf/base/rspfString.h>

class rspfImageHandler;
class rspfFilename;
class rspfKeywordlist;

//*******************************************************************
// CLASS:  rspfImageHandlerFactory
//*******************************************************************
class RSPFDLLEXPORT rspfImageHandlerFactory : public rspfImageHandlerFactoryBase
{
public:
   virtual ~rspfImageHandlerFactory();
   static rspfImageHandlerFactory* instance();

   /**
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    */
   virtual rspfImageHandler* open(const rspfFilename& fileName,
                                   bool openOverview=true)const;
   virtual rspfImageHandler* open(const rspfKeywordlist& kwl,
                                   const char* prefix=0)const;

   /**
    * @brief Open overview that takes a file name.
    * 
    * @param file File to open.
    * 
    * @return rspfRefPtr to image handler on success or null on failure.
    */
   virtual rspfRefPtr<rspfImageHandler> openOverview(
      const rspfFilename& file ) const;
   
   virtual rspfObject* createObject(const rspfString& typeName)const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   virtual void getSupportedExtensions(rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const;
   virtual void getImageHandlersBySuffix(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& ext)const;
   virtual void getImageHandlersByMimeType(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& mimeType)const;
   
protected:

   /**
    * @brief Open method that looks at extension e.g. "tif", "jpg" to select
    * class to open.
    *
    * This is to be called by standard open before just going down the list
    * of classes the brute force hard way.
    * 
    * @param fileName The filename to open.
    * 
    * @return Pointer to image handler or null if nothing opened.
    */
//   virtual rspfImageHandler* openFromExtension(
//      const rspfFilename& fileName) const;
  
   rspfImageHandlerFactory(){}
   rspfImageHandlerFactory(const rspfImageHandlerFactory&){}
   void operator = (const rspfImageHandlerFactory&){}

   static rspfImageHandlerFactory* theInstance;

TYPE_DATA
};

#endif
