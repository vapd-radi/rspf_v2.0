#ifndef rspfVpfLibrary_HEADER
#define rspfVpfLibrary_HEADER
#include <vector>
#include <map>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfErrorContext.h>
class rspfVpfCoverage;
class rspfVpfDatabase;
class rspfVpfExtent;
class RSPFDLLEXPORT rspfVpfLibrary
{
public:
   rspfVpfLibrary();
   virtual ~rspfVpfLibrary();
   bool openLibrary(rspfVpfDatabase* database,
                    const rspfString& name,
                    const rspfFilename& libraryPath);
   void              setDatabase(rspfVpfDatabase* database);
   rspfVpfDatabase* getDatabase()const;
   bool getCoverage(long coverageNumber, rspfVpfCoverage& coverage);
   bool getCoverage(const rspfString& name, rspfVpfCoverage& coverage);
   bool                getExtent(rspfVpfExtent& result)const;
   bool                hasGazette()const;
   rspfFilename       getPath()const;
   rspfString         getName()const;
   bool getCoverageNames(std::vector<rspfString>& coverageNames)const;
   int getNumberOfCoverages()const;
   bool hasCoverage(const rspfString& coverageName)const;
   rspfString getTileName(rspf_int32 id)const;
   void getTileNames(std::vector<rspfString>& tileNames)const;
private:
   rspfVpfDatabase*        theDatabase;
   rspfString              theLibraryName;
   rspfFilename            theLibraryNameFullPath;
   rspf_uint32             theNumberOfCoverages;
   std::vector<rspfString> theCoverageNames;
   mutable std::map<rspf_int32, rspfString> theTileNameMap;
   void setCoverageNames();
   void setTileNames()const;
};
#endif
