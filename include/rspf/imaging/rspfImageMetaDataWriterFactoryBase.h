//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//----------------------------------------------------------------------------
// $Id: rspfImageMetaDataWriterFactoryBase.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfImageMetaDataWriterFactoryBase_HEADER
#define rspfImageMetaDataWriterFactoryBase_HEADER
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfRefPtr.h>

class rspfMetadataFileWriter;

class rspfImageMetaDataWriterFactoryBase : public rspfObjectFactory
{
public:
   /**
    * Pure virtual.  All rspfImageMetaDataWriterFactories must implement.
    * 
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
      const rspfString& type)const=0;

   /**
    * Pure virtual.  All rspfImageMetaDataWriterFactories must implement.
    * 
    * Appends the meta data writers types to the "metadatatypeList".
    * 
    * This is the actual metadata type name like:
    *   rspf_geometry
    *   envi_header
    *   tiff_world_file
    *   jpeg_world_file
    *   
    * @param metadatatypeList stl::vector<rspfString> List to append to.
    *
    * @note All writers should append to the list, not, clear it and then add
    * their types.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList) const=0;

protected:
   rspfImageMetaDataWriterFactoryBase();
   
   rspfImageMetaDataWriterFactoryBase(
      const rspfImageMetaDataWriterFactoryBase&);
   
   const rspfImageMetaDataWriterFactoryBase& operator=(
      const rspfImageMetaDataWriterFactoryBase&);

TYPE_DATA
};
#endif
