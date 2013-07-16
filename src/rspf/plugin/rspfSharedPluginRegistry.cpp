#include <algorithm>
#include <iterator>
#include <rspf/plugin/rspfSharedPluginRegistry.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/plugin/rspfSharedObjectBridge.h>
static rspfTrace traceDebug("rspfSharedPluginRegistry:debug");
rspfSharedPluginRegistry::rspfSharedPluginRegistry()
{
}
rspfSharedPluginRegistry::~rspfSharedPluginRegistry()
{
   theLibraryList.clear();
}
rspfSharedPluginRegistry* rspfSharedPluginRegistry::instance()
{
   static rspfSharedPluginRegistry sharedInstance;
   return &sharedInstance;//theInstance;
}
bool rspfSharedPluginRegistry::registerPlugin(const rspfFilename& filename, const rspfString& options)//, bool insertFrontFlag)
{
   bool result = false;
   if(!getPlugin(filename))
   {
      rspfPluginLibrary *lib =new rspfPluginLibrary;
      if(lib->load(filename))
      {
         lib->setOptions(options);
         if(lib->getSymbol("rspfSharedLibraryInitialize"))
         {
            lib->initialize();
               theLibraryList.push_back(lib);
            result = true;
         }
         else
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfSharedPluginRegistry DEBUG:"
                  << "rspfSharedLibraryInitialize symbol not found\n"
                  << std::endl;
            }
            
         }
      }
      else
      {
         delete lib;
         lib = 0;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: Plugin with the name " << filename << std::endl
                                         << "Already registered with RSPF" << std::endl;
   }
   
   return result;
}
bool rspfSharedPluginRegistry::unregisterPlugin(int idx)
{
   rspfPluginLibrary* lib = getPlugin(idx);
   
   return unregisterPlugin(lib);
}
bool rspfSharedPluginRegistry::unregisterPlugin(rspfPluginLibrary* library)
{
   std::vector<rspfRefPtr<rspfPluginLibrary> >::iterator iter = theLibraryList.begin();
   while(iter!=theLibraryList.end())
   {
      if((*iter).get() == library)
      {
         theLibraryList.erase(iter);
         return true;
      }
      ++iter;
   }
   return false;
}
const rspfPluginLibrary* rspfSharedPluginRegistry::getPlugin(const rspfFilename& filename)const
{
   rspf_uint32 idx = 0;
   rspfFilename fileOnly = filename.file();
   for(idx = 0; idx < theLibraryList.size();++idx)
   {
      if(fileOnly == rspfFilename(theLibraryList[idx]->getName()).file())
      {
         return theLibraryList[idx].get();
      }
   }
   
   return 0;
}
rspfPluginLibrary* rspfSharedPluginRegistry::getPlugin(const rspfFilename& filename)
{
   rspf_uint32 idx = 0;
   rspfFilename fileOnly = filename.file();
   for(idx = 0; idx < theLibraryList.size();++idx)
   {
      if(fileOnly == rspfFilename(theLibraryList[idx]->getName()).file())
      {
         return theLibraryList[idx].get();
      }
   }
   
   return 0;
}
rspf_uint32 rspfSharedPluginRegistry::getIndex(const rspfPluginLibrary* lib)const
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theLibraryList.size(); ++idx)
   {
      if(theLibraryList[idx] == lib)
      {
         return idx;
      }
   }
   return idx;
}
rspfPluginLibrary* rspfSharedPluginRegistry::getPlugin(rspf_uint32 idx)
{
   rspfPluginLibrary* result = 0;
   
   if((idx>=0)&&(idx < theLibraryList.size()))
   {
      result = theLibraryList[idx].get();
   }
   return result;
}
const rspfPluginLibrary* rspfSharedPluginRegistry::getPlugin(rspf_uint32 idx)const
{
   const rspfPluginLibrary* result = 0;
   
   if((idx>=0)&&(idx < theLibraryList.size()))
   {
      result = theLibraryList[idx].get();
   }
   return result;
}
rspf_uint32 rspfSharedPluginRegistry::getNumberOfPlugins()const
{
   return (rspf_uint32)theLibraryList.size();
}
bool rspfSharedPluginRegistry::isLoaded(const rspfFilename& filename) const
   
{
   rspfFilename fileOnly = filename.file();
   bool result = false;
   rspf_uint32 count = getNumberOfPlugins();
   for (rspf_uint32 i = 0; i < count; ++i)
   {
      const rspfPluginLibrary* pi = getPlugin(i);
      if (pi)
      {
         if (fileOnly == rspfFilename(pi->getName()).file())
         {
            result = true;
            break;
         }
      }
   }
   return result;
}
void rspfSharedPluginRegistry::printAllPluginInformation(std::ostream& out)
{
   rspf_uint32 count = getNumberOfPlugins();
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < count; ++idx)
   {
      std::vector<rspfString> classNames;
      const rspfPluginLibrary* pi = getPlugin(idx);
      if(pi)
      {
         pi->getClassNames(classNames);
         out << "Plugin: " << pi->getName() << std::endl;
         out << "DESCRIPTION: \n";
         out << pi->getDescription() << "\n";
         out << "CLASSES SUPPORTED\n     ";
         std::copy(classNames.begin(),
                   classNames.end(),
                   std::ostream_iterator<rspfString>(out, "\n     "));
         out << "\n";
      }
   }
}
