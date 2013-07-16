//*******************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kenneth Melero (kmelero@sanz.com)
//
//*******************************************************************
//  $Id: rspfImageMetaDataWriterFactory.h 9094 2006-06-13 19:12:40Z dburken $

#ifndef rspfImageMetaDataWriterFactory_HEADER
#define rspfImageMetaDataWriterFactory_HEADER

#include <rspf/imaging/rspfImageMetaDataWriterFactoryBase.h>
#include <rspf/base/rspfRefPtr.h>

class rspfMetadataFileWriter;

class RSPFDLLEXPORT rspfImageMetaDataWriterFactory:
   public rspfImageMetaDataWriterFactoryBase
{
public:
   ~rspfImageMetaDataWriterFactory();

   static rspfImageMetaDataWriterFactory* instance();
   virtual rspfObject* createObject(const rspfString& typeName)const;
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
    * Satisfies pure virtual from rspfImageMetaDataWriterFactoryBase.
    * 
    * Appends the meta data writers types from each of the supported meta
    * data writers in this factor to the "metadatatypeList".
    * 
    * This is the actual metadata type name like:
    *   rspf_geometry
    *   envi_header
    *   tiff_world_file
    *   jpeg_world_file
    *   
    * @param metadatatypeList stl::vector<rspfString> List to append to.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList) const;
 
protected:
   static rspfImageMetaDataWriterFactory* theInstance;

   rspfImageMetaDataWriterFactory();

   rspfImageMetaDataWriterFactory(const rspfImageMetaDataWriterFactory& rhs);

   const rspfImageMetaDataWriterFactory& operator =(
      const rspfImageMetaDataWriterFactory& rhs);
   
TYPE_DATA 
};

#endif
