#include <rspf/plugin/rspfDynamicLibrary.h>
#include <rspf/plugin/rspfSharedObjectBridge.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
RTTI_DEF(rspfDynamicLibrary, "rspfDynamicLibrary");
static rspfTrace traceDebug(rspfString("rspfDynamicLibrary:debug"));
rspfDynamicLibrary::rspfDynamicLibrary()
   :theLibrary(0)
{
}
rspfDynamicLibrary::rspfDynamicLibrary(const rspfString& name)
{
   load(name);
}
rspfDynamicLibrary::~rspfDynamicLibrary()
{
   unload();
}
bool rspfDynamicLibrary::load()
{
   return load(theLibraryName);
}
bool rspfDynamicLibrary::load(const rspfString& name)
{
   rspfFilename libraryName = name.trim();
   if(libraryName.empty()||!libraryName.isFile()) return false;
   
#  if defined(__WIN32__) || defined(_WIN32)
   theLibrary = LoadLibrary(libraryName.c_str());
#  else 
   theLibrary = dlopen(libraryName.c_str(), RTLD_LAZY);
#endif
   if (isLoaded())
   {
      theLibraryName = libraryName;
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfDynamicLibrary::load DEBUG:" << std::endl;
      
      if (isLoaded())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "Loaded library:  " << name << std::endl;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfDynamicLibrary::load DEBUG:"
            << "\nFailed to load library:  " << name
            << std::endl;
#  if !defined(__WIN32__) && !defined(_WIN32)
         rspfNotify(rspfNotifyLevel_DEBUG) << dlerror() << std::endl;
#endif
      }
      
   }
   
   return isLoaded();
}
void rspfDynamicLibrary::unload()
{
   if(isLoaded())
   {
#if defined(__WIN32__) || defined(_WIN32)
      FreeLibrary(theLibrary);
#else
      dlclose(theLibrary);
#endif
      theLibrary = 0;
   }
}
void *rspfDynamicLibrary::getSymbol(const rspfString& name) const
{
   if(isLoaded())
   {
#if defined(__WIN32__) || defined(_WIN32)
      return (void*)GetProcAddress( (HINSTANCE)theLibrary, name.c_str());
#else
      return dlsym(theLibrary, name.c_str());
#endif
   }
   return (void*)0;
}
