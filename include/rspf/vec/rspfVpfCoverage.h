#ifndef rspfVpfCoverage_HEADER
#define rspfVpfCoverage_HEADER
#include <map>
#include <vector>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/vec/rspfVpfTable.h>
#include <rspf/vec/rspfVpfFeatureClass.h>
#include <rspf/vec/rspfVpfExtent.h>
class rspfVpfLibrary;
class rspfVpfFeatureClassSchema;
class RSPFDLLEXPORT rspfVpfCoverage
{
public:
   rspfVpfCoverage ();
   virtual ~rspfVpfCoverage();
   
   bool openCoverage(rspfVpfLibrary* library,
                     const rspfString& name,
                     const rspfFilename& fileName);
   const rspfFilename& getPath()const;
   const rspfString& getName()const;
   
   rspf_uint32 getNumberOfFeatureClasses()const;
   void print(ostream& out)const;
   bool getExtent(rspfVpfExtent& extent)const;
   rspfVpfLibrary* getLibrary();
   bool openFeatureClassSchema(rspfVpfFeatureClassSchema& schema);
   
private:
   rspfVpfLibrary* theLibrary;
   rspfString      theCoverageName;
   /*!
    * will hold the full path to this coverage.
    */
   rspfFilename theCoverageNameFullPath;
   
};
#endif
