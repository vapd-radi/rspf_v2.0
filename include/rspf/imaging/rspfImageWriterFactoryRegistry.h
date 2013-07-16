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
//  $Id: rspfImageWriterFactoryRegistry.h 19907 2011-08-05 19:55:46Z dburken $

#ifndef rspfImageWriterFactoryRegistry_HEADER
#define rspfImageWriterFactoryRegistry_HEADER
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/imaging/rspfImageWriterFactoryBase.h>
#include <rspf/base/rspfFactoryListInterface.h>
#include <vector>
#include <iosfwd>

class rspfImageFileWriter;
class rspfKeywordlist;

class RSPFDLLEXPORT rspfImageWriterFactoryRegistry :
   public rspfObjectFactory,
   public rspfFactoryListInterface<rspfImageWriterFactoryBase, rspfImageFileWriter>
{   
public:
   static rspfImageWriterFactoryRegistry* instance();

   rspfImageFileWriter *createWriter(const rspfFilename& filename)const;
   rspfImageFileWriter *createWriterFromExtension(const rspfString& fileExtension)const;
   rspfImageFileWriter *createWriter(const rspfKeywordlist &kwl,
                                      const char *prefix=0)const;
   rspfImageFileWriter* createWriter(const rspfString& typeName)const;
   
   rspfObject* createObject(const rspfKeywordlist &kwl,
                             const char *prefix=0)const;

   rspfObject* createObject(const rspfString& typeName)const;
   
   /**
    * getTypeNameList.  This should return the class type of the object being
    * used to perform the writting.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
   /**
    * getImageTypeList.  This is the actual image type name.  So for
    * example, rspfTiffWriter has several image types.  Some of these
    * include TIFF_TILED, TIFF_TILED_BAND_SEPARATE ... etc.
    * The rspfGdalWriter
    * may include GDAL_IMAGINE_HFA, GDAL_RGB_NITF, GDAL_JPEG20000, ... etc
    * A writer should be able to be instantiated by this name as well as a
    * class name
    */
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;

   virtual void getImageFileWritersBySuffix(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                            const rspfString& ext)const;
   virtual void getImageFileWritersByMimeType(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                              const rspfString& mimeType)const;
   /**
    * @brief Prints list of writers from getImageTypeList.
    * @param  out Stream to print to.
    * @return std::ostream&
    */
   std::ostream& printImageTypeList(std::ostream& out)const;

   /**
    * @brief Prints list of writers from getImageTypeList.
    * @param  out Stream to print to.
    * @return std::ostream&
    */
   std::ostream& printWriterProps(std::ostream& out)const;
   
protected:
   rspfImageWriterFactoryRegistry();
   rspfImageWriterFactoryRegistry(const rspfImageWriterFactoryRegistry&);
   void operator=(const rspfImageWriterFactoryRegistry&);
   
   static rspfImageWriterFactoryRegistry*    theInstance;
};

extern "C"
{
  RSPFDLLEXPORT void* rspfImageWriterFactoryRegistryGetInstance();
}
#endif
