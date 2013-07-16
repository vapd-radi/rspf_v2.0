#ifndef rspfSharedObjectBridge_HEADER
#define rspfSharedObjectBridge_HEADER
#include <vector>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
extern "C"
{
   typedef const char* (*rspfSharedLibraryGetDescriptionPtr)();
   typedef int (*rspfSharedLibraryGetNumberOfClassNamesPtr)();
   typedef const char* (*rspfSharedLibraryGetClassNamesPtr)(int idx);
   typedef const char* (*rspfSharedLibraryGetIdPtr())();
   typedef const char* (*rspfSharedLibraryGetOssimVersionStringPtr())();
   typedef void  (*rspfSharedLibraryRegisterPluginPtr())();
   
   struct RSPFDLLEXPORT rspfSharedObjectInfo
   {
      rspfSharedLibraryGetDescriptionPtr        getDescription;
      rspfSharedLibraryGetNumberOfClassNamesPtr getNumberOfClassNames;
      rspfSharedLibraryGetClassNamesPtr         getClassName;
/*       rspfSharedLibraryGetId                    getId; */
/*       rspfSharedLibraryGetOssimVersionString    getOssimVersion; */
/*       rspfSharedLibraryRegisterPluginPtr        registerPlugin; */
   };
   
   
   typedef void (*rspfSharedLibraryInitializePtr)(rspfSharedObjectInfo** info, const char* options);
   typedef void (*rspfSharedLibraryFinalizePtr)();
   /*
    * these need to define these in your plugin.  They are only here for clairity
    * 
    * extern void rspfSharedLibraryInitialize(rspfSharedObjectInfo** info);
    * extern void rspfSharedLibraryFinalize();
    */
}
#endif
