#ifndef rspfDynamicLibrary_HEADER
#define rspfDynamicLibrary_HEADER
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/rspfConfig.h>
#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else 
#include <dlfcn.h>
#endif
class rspfDynamicLibrary : public rspfReferenced
{
public:
   rspfDynamicLibrary();
   rspfDynamicLibrary(const rspfString& name);
   virtual ~rspfDynamicLibrary();
   bool isLoaded() const { return theLibrary != 0; }
   bool load(const rspfString& name);
   bool load();
   void unload();
   void *getSymbol(const rspfString& name) const;
   const rspfString& getName()const
      {
         return theLibraryName;
      }
   
protected:
#if defined(_WIN32)
	HINSTANCE theLibrary;
#else
	void* theLibrary;
#endif
   rspfString theLibraryName;
TYPE_DATA
};
#endif
