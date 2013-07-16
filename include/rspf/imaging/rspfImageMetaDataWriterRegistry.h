//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//----------------------------------------------------------------------------
// $Id: rspfImageMetaDataWriterRegistry.h 13508 2008-08-27 15:51:38Z gpotts $

#ifndef rspfImageMetaDataWriterRegistry_HEADER
#define rspfImageMetaDataWriterRegistry_HEADER
#include <rspf/imaging/rspfImageMetaDataWriterFactoryBase.h>
#include <rspf/base/rspfRefPtr.h>

class rspfMetadataFileWriter;

class RSPFDLLEXPORT rspfImageMetaDataWriterRegistry :
   public rspfImageMetaDataWriterFactoryBase
{
public:
   ~rspfImageMetaDataWriterRegistry();
   
   static rspfImageMetaDataWriterRegistry* instance();
   
   void registerFactory(rspfImageMetaDataWriterFactoryBase* factory);
   void unregisterFactory(rspfImageMetaDataWriterFactoryBase* factory);
   bool findFactory(rspfImageMetaDataWriterFactoryBase* factory)const;
   /**
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const;

   /**
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;

   /**
    * Creates a writer from either a class name or a string representing the
    * meta data type like "tiff_world_file".
    *
    * @return rspfRefPtr<rspfMetadataFileWriter>
    *
    * @note Return can have a null pointer if type is not found in a factory so
    * the caller should always check the pointer like:
    * rspfRefPtr<rspfMetadataFileWriter> mw =
    *    rspfImageMetaDataWriterRegistry::instance("rspf_readme");
    * if (!mw.valid())
    * {
    *    // not in factory
    * }
    */
   virtual rspfRefPtr<rspfMetadataFileWriter> createWriter(
      const rspfString& type)const;
   
   /**
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;

   /**
    * Initializes list with meta data types from all registered
    * metadata factories.
    * 
    * This is the actual image type name like:
    *   rspf_geometry
    *   envi_header
    *   tiff_world_file
    *   jpeg_world_file
    *   
    * @param metadatatypeList stl::vector<rspfString> list to append to.
    *
    * @note Since this is the master registry for all meta data factories
    * this method starts by clearing the metadatatypeList.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList)const;
   
protected:
   rspfImageMetaDataWriterRegistry();

   rspfImageMetaDataWriterRegistry(
      const rspfImageMetaDataWriterRegistry& rhs);
   
   const rspfImageMetaDataWriterRegistry& operator=(
      const rspfImageMetaDataWriterRegistry &rhs);

  // static rspfImageMetaDataWriterRegistry* theInstance;
   std::vector<rspfImageMetaDataWriterFactoryBase*> theFactoryList;
};


extern "C"
{
   RSPFDLLEXPORT void* rspfImageMetaDataWriterRegistryGetInstance();
}
#endif
