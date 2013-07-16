#ifndef rspfEnvironmentUtility_HEADER
#define rspfEnvironmentUtility_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>

class RSPF_DLL rspfEnvironmentUtility
{
public:
   typedef std::vector<rspfFilename> FilenameListType;
   
   
   static rspfEnvironmentUtility* instance();

   rspfString getEnvironmentVariable(const rspfString& variable)const;

   rspfFilename getUserDir()const;
   rspfString   getUserName()const;
   rspfFilename getUserOssimSupportDir()const;
   rspfFilename getUserOssimPreferences()const;
   rspfFilename getUserOssimPluginDir()const;
   
   rspfFilename getInstalledOssimSupportDir()const;
   rspfFilename getInstalledOssimPluginDir()const;
   rspfFilename getInstalledOssimPreferences()const;

   /**
    * @return The current working dir which is $(PWD) for unix $(CD) for
    * windows.
    */
   rspfFilename getCurrentWorkingDir()const;

   rspfFilename searchAllPaths(const rspfFilename& file)const;
   
   rspfFilename findPlugin(const rspfFilename& plugin)const;
   rspfFilename findData(const rspfFilename& data)const;

   void addDataSearchPath(const rspfFilename& path);
   void addDataSearchPathToFront(const rspfFilename& path);
   void addPluginSearchPath(const rspfFilename& path);
   void addPluginSearchPathToFront(const rspfFilename& path);
 
   rspfEnvironmentUtility::FilenameListType& getPluginSearchPath();
   const rspfEnvironmentUtility::FilenameListType& getPluginSearchPath()const;
   rspfEnvironmentUtility::FilenameListType& getDataSearchPath();
   const rspfEnvironmentUtility::FilenameListType& getDataSearchPath()const;
   
private:
   static rspfEnvironmentUtility* theInstance;

   rspfEnvironmentUtility::FilenameListType thePluginSearchPath;
   rspfEnvironmentUtility::FilenameListType theDataSearchPath;

   rspfEnvironmentUtility();
   rspfEnvironmentUtility(const rspfEnvironmentUtility& obj);
   const rspfEnvironmentUtility& operator=
      (const rspfEnvironmentUtility& rhs);
};


#endif
