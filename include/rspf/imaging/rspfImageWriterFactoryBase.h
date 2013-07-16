//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfImageWriterFactoryBase.h 18011 2010-08-31 12:48:56Z dburken $
#ifndef rspfImageWriterFactoryBase_HEADER
#define rspfImageWriterFactoryBase_HEADER
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfString.h>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <vector>
class rspfKeywordlist;

class RSPF_DLL rspfImageWriterFactoryBase : public rspfObjectFactory
{   
public:
   typedef std::vector<rspfRefPtr<rspfImageFileWriter> > ImageFileWriterList;
   
   virtual ~rspfImageWriterFactoryBase() {}
   virtual rspfImageFileWriter* createWriter(const rspfKeywordlist& kwl,
                                          const char *prefix=0)const=0;
   virtual rspfImageFileWriter* createWriter(const rspfString& typeName)const=0;

   virtual void getExtensions(std::vector<rspfString>& result)const=0;
   
   /*!
    * getImageTypeList.  This is the actual image type name.  So for
    * example, rspfTiffWriter has several image types.  Some of these
    * include tiff_tiled, tiff_tiled_band_separate ... etc.  The rspfGdalWriter
    * may include gdal_imagine_hfa,gdal_nitf_rgb_band_separate, ... etc
    * A writer should be able to be instantiated by this name as well as a class name
    */
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const=0;
   
   virtual void getImageFileWritersBySuffix(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                            const rspfString& ext)const;

   virtual void getImageFileWritersByMimeType(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                              const rspfString& mimeType)const;
   
TYPE_DATA
};

#endif
