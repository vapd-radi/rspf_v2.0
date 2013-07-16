#include <rspf/plugin/rspfPluginLibrary.h>
#include <iostream>
RTTI_DEF1(rspfPluginLibrary, "rspfPluginLibrary", rspfDynamicLibrary);
rspfPluginLibrary::rspfPluginLibrary()
   :rspfDynamicLibrary(),
    m_info(0)
{
}
rspfPluginLibrary::rspfPluginLibrary(const rspfString& name, const rspfString& options)
   :rspfDynamicLibrary(name),
    m_options(options),
    m_info(0)
{
   initialize();
}
rspfPluginLibrary::~rspfPluginLibrary()
{
   finalize();
}
void rspfPluginLibrary::initialize()
{
   if(!isLoaded())
   {
      load();
   }
   
   rspfSharedLibraryInitializePtr init = (rspfSharedLibraryInitializePtr)getSymbol("rspfSharedLibraryInitialize");
   if(init)
   {
      init(&m_info, m_options.c_str());
   }
}
void rspfPluginLibrary::finalize()
{
   rspfSharedLibraryFinalizePtr finalizeLib = (rspfSharedLibraryFinalizePtr)getSymbol("rspfSharedLibraryFinalize");
   if(finalizeLib)
   {         
      finalizeLib();
   }
   
   unload();
}
rspfString rspfPluginLibrary::getDescription()const
{
   rspfString result;
   if(m_info&&isLoaded()&&m_info->getDescription)
   {
      result = m_info->getDescription();
   }
   return result;
}
void rspfPluginLibrary::getClassNames(std::vector<rspfString>& classNames)const
{
   if(m_info&&m_info->getNumberOfClassNames&&m_info->getClassName)
   {
      rspf_int32 idx        = 0;
      rspf_int32 numObjects = 0;
      numObjects = m_info->getNumberOfClassNames();
      for(idx = 0; idx < numObjects; ++idx)
      {
         classNames.push_back(m_info->getClassName(idx));
      }
   }
}
void rspfPluginLibrary::setOptions(const rspfString& options)
{
   m_options = options;
   
}
