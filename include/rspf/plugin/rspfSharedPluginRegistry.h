#ifndef rspfSharedPluginRegistry_HEADER
#define rspfSharedPluginRegistry_HEADER
#include <iostream>
#include <vector>
#include <rspf/plugin/rspfSharedObjectBridge.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/plugin/rspfPluginLibrary.h>
class RSPFDLLEXPORT rspfSharedPluginRegistry
{
public:
   virtual ~rspfSharedPluginRegistry();
   
   static rspfSharedPluginRegistry* instance();
   bool registerPlugin(const rspfFilename& filename, const rspfString& options="");//, bool insertFrontFlag=false);
   bool unregisterPlugin(int idx);
   bool unregisterPlugin(rspfPluginLibrary* plugin);
   const rspfPluginLibrary* getPlugin(const rspfFilename& filename)const;
   rspfPluginLibrary* getPlugin(const rspfFilename& filename);
   rspf_uint32 getIndex(const rspfPluginLibrary* lib)const;
   rspfPluginLibrary* getPlugin(rspf_uint32 idx);
   const rspfPluginLibrary* getPlugin(rspf_uint32 idx)const;
   rspf_uint32 getNumberOfPlugins()const;
   /**
    * Checks if filename is already loaded to avoid duplication.
    * 
    * @param filename The file to check.
    *
    * @return true if any of the plugins match file name, false if not.
    */
   bool isLoaded(const rspfFilename& filename) const;
   
   void printAllPluginInformation(std::ostream& out);
   
protected:
   rspfSharedPluginRegistry();
   rspfSharedPluginRegistry(const rspfSharedPluginRegistry&){}
   void operator = (const rspfSharedPluginRegistry&){}
   std::vector<rspfRefPtr<rspfPluginLibrary> > theLibraryList;
};
#endif
