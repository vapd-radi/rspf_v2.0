//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Frank Warmerdam (warmerda@home.com)
//
//*******************************************************************
//  $Id: rspfImageWriterFactory.h 18002 2010-08-30 18:01:10Z gpotts $

#ifndef rspfImageWriterFactory_HEADER
#define rspfImageWriterFactory_HEADER
#include <rspf/imaging/rspfImageWriterFactoryBase.h>

class rspfImageWriter;
class rspfKeywordlist;

class rspfImageWriterFactory: public rspfImageWriterFactoryBase
{   
public:
   virtual ~rspfImageWriterFactory();
   static rspfImageWriterFactory* instance();
   
   rspfImageFileWriter *createWriterFromExtension(const rspfString& fileExtension)const;
   virtual rspfImageFileWriter* createWriter(const rspfKeywordlist& kwl,
                                              const char *prefix=0)const;
   virtual rspfImageFileWriter* createWriter(const rspfString& typeName)const;
   
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char *prefix=0)const;
   virtual rspfObject* createObject(const rspfString& typeName)const;
   
   virtual void getExtensions(std::vector<rspfString>& result)const;
   
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
   /*!
    * getImageTypeList.  This is the actual image type name.  So for
    * example, rspfTiffWriter has several image types.  Some of these
    * include TIFF_TILED, TIFF_TILED_BAND_SEPARATE ... etc.  The
    * rspfGdalWriter
    * may include GDAL_IMAGINE_HFA, GDAL_RGB_NITF, GDAL_JPEG20000, ... etc
    * A writer should be able to be instantiated by this name as well as a
    * class name
    */
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;
   
   virtual void getImageFileWritersBySuffix(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                            const rspfString& ext)const;
   virtual void getImageFileWritersByMimeType(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                              const rspfString& mimeType)const;
protected:
   rspfImageWriterFactory() {}

   static rspfImageWriterFactory* theInstance;

	rspfImageFileWriter* createFromMimeType(const rspfString& mimeType)const;
   bool isImageTypeName(const rspfString& name)const;
};

#endif
