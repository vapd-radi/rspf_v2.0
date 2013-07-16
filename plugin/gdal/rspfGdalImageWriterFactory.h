#ifndef rspfGdalImageWriterFactory_HEADER
#define rspfGdalImageWriterFactory_HEADER
#include <rspf/imaging/rspfImageWriterFactoryBase.h>
#include <gdal.h>
class rspfImageFileWriter;
class rspfKeywordlist;
class rspfImageWriterFactory;
class rspfGdalImageWriterFactory: public rspfImageWriterFactoryBase
{   
public:
   virtual ~rspfGdalImageWriterFactory();
   static rspfGdalImageWriterFactory* instance();
   virtual rspfImageFileWriter* createWriter(const rspfKeywordlist& kwl,
                                              const char *prefix=0)const;
   virtual rspfImageFileWriter* createWriter(const rspfString& typeName)const;
   
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char *prefix=0)const;
   virtual rspfObject* createObject(const rspfString& typeName)const;
   
   virtual void getExtensions(std::vector<rspfString>& result)const;
   
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
   /**
    * void getImageTypeList(std::vector<rspfString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * This writer can have the following types dependent upon how the gdal
    * library was compiled:
    * gdal_imagine_hfa
    * gdal_nitf_rgb_band_separate
    * gdal_jpeg2000
    * gdal_arc_info_aig
    * gdal_arc_info_gio
    * gdal_arc_info_ascii_grid
    *
    * @param imageTypeList stl::vector<rspfString> list to append to.
    */
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;
   
   virtual void getImageFileWritersBySuffix(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                            const rspfString& ext)const;
   virtual void getImageFileWritersByMimeType(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                              const rspfString& mimeType)const;
protected:
   rspfGdalImageWriterFactory() {theInstance = this;}
   static rspfGdalImageWriterFactory* theInstance;
   
   rspfString convertToDriverName(const rspfString& imageTypeName)const;
   bool canWrite(GDALDatasetH handle)const;
};
#endif
