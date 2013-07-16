#ifndef rspfVpfAnnotationLibraryInfo_HEADER
#define rspfVpfAnnotationLibraryInfo_HEADER
#include <map>
#include <rspf/base/rspfString.h>
#include <rspf/imaging/rspfVpfAnnotationCoverageInfo.h>

class rspfVpfDatabase;
class rspfRgbImage;
class rspfVpfAnnotationFeatureInfo;

class RSPFDLLEXPORT rspfVpfAnnotationLibraryInfo
{
public:
  rspfVpfAnnotationLibraryInfo(const rspfString& libraryName = "",
				bool enabledFlag=true);

  virtual ~rspfVpfAnnotationLibraryInfo();

  bool getEnabledFlag()const;

  void setEnabledFlag(bool flag);

  const rspfString& getName()const;

  void setName(const rspfString& libraryName);

  void setDatabase(rspfVpfDatabase* database);

  rspfVpfDatabase* getDatabase();

  rspfIrect getBoundingProjectedRect()const;
  
  void transform(rspfImageGeometry* geom);
  void buildLibrary();
  void buildLibrary(const rspfString& coverageName, const rspfString& feature);
  void deleteAllCoverage();
  void drawAnnotations(rspfRgbImage* tile);
  void getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& features);

  virtual bool saveState(rspfKeywordlist& kwl,
			 const char* prefix=0)const;
  virtual bool loadState(const rspfKeywordlist& kwl,
			 const char* prefix=0);
  
protected:
   rspfString theName;
   bool theEnabledFlag;
   rspfVpfDatabase* theDatabase;
   std::vector<rspfVpfAnnotationCoverageInfo*> theCoverageLayerList;
};

#endif
