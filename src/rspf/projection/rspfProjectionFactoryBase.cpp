#include <rspf/projection/rspfProjectionFactoryBase.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfTrace.h>
static rspfTrace traceDebug = rspfTrace("rspfProjectionFactoryBase:debug");
RTTI_DEF1(rspfProjectionFactoryBase, "rspfProjectionFactoryBase", rspfObjectFactory);
rspfProjection* rspfProjectionFactoryBase::createProjectionFromGeometryFile(const rspfFilename& imageFile, rspf_uint32 entryIdx)const
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfProjectionFactoryBase::createProjectionFromGeometryFile "
         << "DEBUG:"
         << "\nimageFile:  " << imageFile
         << "\nentryIdx:   " << entryIdx
         << std::endl;
   }
   
   rspfFilename geomFile = imageFile;
   if (entryIdx == 0)
   {
      geomFile.setExtension("geom");
      if(geomFile.exists())
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "Found geometry file:  " << geomFile
               << std::endl;
         }
         rspfKeywordlist kwl;
         if(kwl.addFile(geomFile))
         {
            return rspfProjectionFactoryRegistry::instance()->
               createProjection(kwl);
         }
      }
   }
   geomFile = imageFile.fileNoExtension();
   geomFile += "_e";
   geomFile += rspfString::toString(entryIdx);
   geomFile.setExtension("geom");
   if(geomFile.exists())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "Found geometry file:  " << geomFile
            << std::endl;
      }
      
      rspfKeywordlist kwl;
      if(kwl.addFile(geomFile))
      {
         return rspfProjectionFactoryRegistry::instance()->
            createProjection(kwl);
      }
   }   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG:  No geometry file found.  Returning NULL..."
         << std::endl;
   }
   return NULL;
}
rspfProjection* rspfProjectionFactoryBase::createProjection(rspfImageHandler* handler)const
{
   if(!handler) return 0;
   return createProjection(handler->getFilename(), handler->getCurrentEntry());
}
