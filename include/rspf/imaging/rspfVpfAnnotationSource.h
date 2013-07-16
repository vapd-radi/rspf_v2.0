#ifndef rspfVpfAnnotationSource_HEADER
#define rspfVpfAnnotationSource_HEADER
#include <rspf/vec/rspfVpfDatabase.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfGeoAnnotationSource.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/imaging/rspfVpfAnnotationLibraryInfo.h>
class rspfVpfAnnotationFeatureInfo;
class rspfVpfAnnotationSource : public rspfGeoAnnotationSource
{
public:
   rspfVpfAnnotationSource();
   virtual bool open();
   virtual bool open(const rspfFilename& file);
   virtual bool isOpen()const;

   virtual void close();
   virtual rspfFilename getFilename()const;
  virtual void computeBoundingRect();

   virtual void transformObjects(rspfImageGeometry* geom=0);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);

   virtual void drawAnnotations(rspfRefPtr<rspfImageData> tile);
   virtual void getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& features);
  virtual void setAllFeatures(const std::vector<rspfVpfAnnotationFeatureInfo*>& features);
 
protected:
   virtual ~rspfVpfAnnotationSource();
   
   rspfVpfDatabase theDatabase;
   rspfFilename    theFilename;
   std::vector<rspfVpfAnnotationLibraryInfo*> theLibraryInfo;
   std::vector<rspfVpfAnnotationFeatureInfo*> theFeatureList;

  void deleteAllLibraries();
  bool openDatabase(const rspfFilename& file);
};

#endif
