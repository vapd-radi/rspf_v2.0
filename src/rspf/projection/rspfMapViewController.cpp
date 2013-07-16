#include <rspf/projection/rspfMapViewController.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <iostream>
#include <fstream>
using namespace std;
RTTI_DEF1(rspfMapViewController, "rspfMapViewController", rspfViewController);
rspfMapViewController::rspfMapViewController()
   :rspfViewController()
{
}
rspfMapViewController::rspfMapViewController(rspfObject* owner,
                                               rspf_uint32 inputListSize,
                                               rspf_uint32 outputListSize,
                                               bool inputListFixedFlag,
                                               bool outputListFixedFlag)
   :rspfViewController(owner,
                        inputListSize,
                        outputListSize,
                        inputListFixedFlag,
                        outputListFixedFlag)
{
   if(theOwner)
   {
      const rspfObject* obj = findFirstViewOfType(STATIC_TYPE_INFO(rspfMapProjection));
      rspfMapProjection* proj = PTR_CAST(rspfMapProjection, obj);
      
      if(proj)
      {
         theView = proj->dup();
      }
      else
      {
         theView = new rspfEquDistCylProjection;
      }
   }
   else
   {
      theView = new rspfEquDistCylProjection;
   }
}
rspfMapViewController::~rspfMapViewController()
{
}
bool rspfMapViewController::setView(rspfObject* object)
{
   if(PTR_CAST(rspfMapProjection, object))
   {
      return rspfViewController::setView(object);
   }
   
   return false;
}
void rspfMapViewController::changeOwner(rspfObject* owner)
{
   rspfConnectableObject::changeOwner(owner);
   
   if(!theView)
   {
      const rspfObject* obj   = findFirstViewOfType(STATIC_TYPE_INFO(rspfMapProjection));
      rspfMapProjection* proj = PTR_CAST(rspfMapProjection, obj);
      
      if(proj)
      {
         setView(proj->dup());
         propagateView();
      }
      else
      {
         setView(new rspfEquDistCylProjection);
         propagateView();
      }
   }
   else
   {
      propagateView();
   }   
}
void rspfMapViewController::setFilename(const rspfFilename& file)
{
   if(file.exists())
   {
      rspfKeywordlist kwl;
      kwl.addFile(file.c_str());
      
      rspfRefPtr<rspfProjection> proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
      if(dynamic_cast<rspfMapProjection*>(proj.get()))
      {
         setView(proj.get());
      }
   }
   theGeometryFile = file;
}
bool rspfMapViewController::saveState(rspfKeywordlist& kwl,
                                       const char* prefix)const
{
   rspfSource::saveState(kwl, prefix);
   rspfMapProjection* proj = PTR_CAST(rspfMapProjection, getView());
   rspfString newPrefix = prefix;
   newPrefix += "projection.";
   
   if(proj)
   {
      if(theGeometryFile == "")
      {
         proj->saveState(kwl, newPrefix.c_str());
      }
      else
      {
         ofstream output(theGeometryFile.c_str());
         if(output)
         {
            kwl.add(prefix,
                    rspfKeywordNames::FILENAME_KW,
                    theGeometryFile);
            rspfKeywordlist kwl2;
            proj->saveState(kwl2);
            kwl2.writeToStream(output);
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfMapViewController::saveState can't open filename " << theGeometryFile
                                               << "\nsaving inline to passed in keywordlist" << std::endl;
            proj->saveState(kwl, newPrefix);
         }
      }
   }
   
   return true;
}
bool rspfMapViewController::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   rspfSource::loadState(kwl, prefix);
   theGeometryFile = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   rspfRefPtr<rspfProjection> proj;
   
   if(theGeometryFile == "")
   {
      rspfString newPrefix = prefix;
      newPrefix += "projection.";
      proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl, newPrefix.c_str());
   }
   else
   {
      rspfKeywordlist kwl2;
      kwl2.addFile(theGeometryFile);
      proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl2);
   }
   
   if(dynamic_cast<rspfMapProjection*>(proj.get()))
   {
      setView(proj.get());
   }
   return true;
}
