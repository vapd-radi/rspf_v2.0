//*****************************************************************************
// FILE: rspfSensorModelFactory.cc
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR:  Oscar Kramer
//
// DESCRIPTION:
//   Contains implementation of class rspfSensorModelFactory
//
//*****************************************************************************
//  $Id: rspfSensorModelFactory.cpp 20238 2011-11-09 18:39:10Z gpotts $
#include <fstream>
#include <algorithm>
#include <rspf/projection/rspfSensorModelFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  = rspfTrace("rspfSensorModelFactory:exec");
static rspfTrace traceDebug = rspfTrace("rspfSensorModelFactory:debug");

//***
// Note to programmer: To add a new model, search this file for "ADD_MODEL"
// to locate places requiring editing. Functional example below...
//
// ADD_MODEL: Include all sensor model headers here:
//***
#include <rspf/projection/rspfCoarseGridModel.h>
#include <rspf/projection/rspfRpcModel.h>
#include <rspf/projection/rspfRpcProjection.h>
#include <rspf/projection/rspfIkonosRpcModel.h>
#include <rspf/projection/rspfNitfRpcModel.h>
#include <rspf/projection/rspfQuickbirdRpcModel.h>
#include <rspf/projection/rspfLandSatModel.h>
#include <rspf/projection/rspfSpot5Model.h>
#include <rspf/projection/rspfBuckeyeSensor.h>
#include <rspf/projection/rspfSarModel.h>
#include <rspf/projection/rspfRS1SarModel.h>
#include <rspf/support_data/rspfSpotDimapSupportData.h>
#include <rspf/projection/rspfNitfMapModel.h>
#include <rspf/projection/rspfFcsiModel.h>
#include <rspf/projection/rspfApplanixUtmModel.h>
#include <rspf/projection/rspfApplanixEcefModel.h>
#include <rspf/projection/rspfSkyBoxLearSensor.h>
#include <rspf/projection/rspfIpodSensor.h>
#include <rspf/support_data/rspfFfL7.h>
#include <rspf/support_data/rspfFfL5.h>

//***
// ADD_MODEL: List names of all sensor models produced by this factory:
//***
//const char* MY_NEW_MODEL = "myNewModel";
// const char* OCG_MODEL        = "rspfCoarseGridModel";
// const char* RPC_MODEL        = "rspfRpcModel";
// const char* IKONOS_RPC_MODEL = "rspfIkonosRpcModel";
// const char* NITF_RPC_MODEL   = "rspfNitfRpcModel";
// const char* LANDSAT_MODEL    = "rspfLandSatModel";
// const char* NITF_MAP_MODEL   = "rspfNitfMapModel";
// const char* FCSI_MODEL       = "rspfFcsiModel";
// const char* EMERGE_FCSI_MODEL= "rspfEmergeFcsiModel";

rspfSensorModelFactory* rspfSensorModelFactory::theInstance = 0;

//*****************************************************************************
//  STATIC METHOD: rspfSensorModelFactory::instance()
//  
//*****************************************************************************
rspfSensorModelFactory*  rspfSensorModelFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfSensorModelFactory;
   }

   return (rspfSensorModelFactory*) theInstance;
}

//*****************************************************************************
//  METHOD: rspfSensorModelFactory::create(kwl, prefix)
//*****************************************************************************
rspfProjection* rspfSensorModelFactory::createProjection(const rspfKeywordlist &keywordList,
                                                           const char *prefix) const
{
   rspfRefPtr<rspfProjection> result;

   //
   // Permit specification of geometry file name in lieu of type:
   //
   const char*  value = keywordList.find(prefix, rspfKeywordNames::GEOM_FILE_KW);
   if (value)
   {
      result = createProjection(rspfFilename(value), 0);
   }
   //
   // Search for occurence of "type" keyword:
   //
   else
   {
      value = keywordList.find(prefix, rspfKeywordNames::TYPE_KW);
      if(value)
      {
         result = createProjection(rspfString(value));
         if(result.valid())
         {
            if(!result->loadState(keywordList, prefix))
            {
               result = 0;
            }
        }
      }
   }
   
   return result.release();
}

//*****************************************************************************
//  METHOD: 
//  
//*****************************************************************************
rspfProjection*
rspfSensorModelFactory::createProjection(const rspfString &name) const
{
   //***
   // Name should represent the model type:
   //***
   if(name == STATIC_TYPE_NAME(rspfSkyBoxLearSensor))
   {
      return new rspfSkyBoxLearSensor;
   }
   if(name == STATIC_TYPE_NAME(rspfIpodSensor))
   {
      return new rspfIpodSensor;
   }
   if(name == STATIC_TYPE_NAME(rspfCoarseGridModel))
   {
      return new rspfCoarseGridModel;
   }
   if(name == STATIC_TYPE_NAME(rspfBuckeyeSensor))
   {
      return new rspfBuckeyeSensor;
   }
   
   if (name ==  STATIC_TYPE_NAME(rspfRpcModel))
   {
      return new rspfRpcModel;
   }

   if(name == STATIC_TYPE_NAME(rspfLandSatModel))
   {
      return new rspfLandSatModel;
   }

   if(name == STATIC_TYPE_NAME(rspfNitfMapModel))
   {
      return new rspfNitfMapModel;
   }

   if(name == STATIC_TYPE_NAME(rspfQuickbirdRpcModel))
   {
      return new rspfQuickbirdRpcModel;
   }
   
   if(name == STATIC_TYPE_NAME(rspfIkonosRpcModel))
   {
     return new rspfIkonosRpcModel;
   }

   if(name == STATIC_TYPE_NAME(rspfNitfRpcModel))
   {
      return new rspfNitfRpcModel;
   }
   if(name == STATIC_TYPE_NAME(rspfRpcProjection))
   {
      return new rspfRpcProjection;
   }
   if(name == STATIC_TYPE_NAME(rspfFcsiModel))
   {
      return new rspfFcsiModel;
   }
   if(name == STATIC_TYPE_NAME(rspfApplanixUtmModel))
   {
      return new rspfApplanixUtmModel;
   }
   if(name == STATIC_TYPE_NAME(rspfApplanixEcefModel))
   {
      return new rspfApplanixEcefModel;
   }
   if(name == STATIC_TYPE_NAME(rspfSpot5Model))
   {
      return new rspfSpot5Model;
   }
   if(name == STATIC_TYPE_NAME(rspfSarModel))
   {
      return new rspfSarModel;
   }
   if(name == STATIC_TYPE_NAME(rspfRS1SarModel))
   {
      return new rspfRS1SarModel;
   }

   //***
   // ADD_MODEL: (Please leave this comment for the next programmer)
   //***
//   if(name == MY_NEW_MODEL)
//      return new myNewModel;

   return NULL;
}

//*****************************************************************************
//  METHOD
//*****************************************************************************
rspfObject*
rspfSensorModelFactory::createObject(const rspfString& typeName)const
{
   return createProjection(typeName);
}

//*****************************************************************************
//  METHOD
//*****************************************************************************
rspfObject*
rspfSensorModelFactory::createObject(const rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   return createProjection(kwl, prefix);
}
   
//*****************************************************************************
//  METHOD
//*****************************************************************************
void
rspfSensorModelFactory::getTypeNameList(std::vector<rspfString>& typeList)
   const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfApplanixEcefModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfApplanixUtmModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfCoarseGridModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfRpcModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfIkonosRpcModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfQuickbirdRpcModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfNitfRpcModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfLandSatModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfNitfMapModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfFcsiModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfSpot5Model));
   typeList.push_back(STATIC_TYPE_NAME(rspfSarModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfRS1SarModel));
   typeList.push_back(STATIC_TYPE_NAME(rspfBuckeyeSensor));
   typeList.push_back(STATIC_TYPE_NAME(rspfSkyBoxLearSensor));
   typeList.push_back(STATIC_TYPE_NAME(rspfIpodSensor));

   //***
   // ADD_MODEL: Please leave this comment for the next programmer. Add above.
   //***
   //typeList.push_back(rspfString(MY_NEW_MODEL));
   
}

rspfProjection* rspfSensorModelFactory::createProjection(
   const rspfFilename& filename, rspf_uint32  entryIdx) const
{
   static const char MODULE[] = "rspfSensorModelFactory::createProjection";
   
   rspfKeywordlist kwl;
   rspfRefPtr<rspfProjection> model = 0;

   rspfFilename geomFile = filename;
   geomFile = geomFile.setExtension("geom");
   
   if(geomFile.exists()&&
      kwl.addFile(filename.c_str()))
   {
      rspfFilename coarseGrid;
      
      const char* type = kwl.find(rspfKeywordNames::TYPE_KW);
      if(type)
      {
         if(rspfString(type) ==
            rspfString(STATIC_TYPE_NAME(rspfCoarseGridModel)))
         {
            findCoarseGrid(coarseGrid, filename);
            
            if(coarseGrid.exists() &&(coarseGrid != ""))
            {
               kwl.add("grid_file_name",
                       coarseGrid.c_str(),
                       true);
               model = new rspfCoarseGridModel(kwl);
               if(!model->getErrorStatus())
               {
                  return model.release();
               }
               model = 0;
            }
         }
      }
      kwl.clear();
   }

   // See if there is an external geomtry.
   rspfRefPtr<rspfProjection> proj =
      createProjectionFromGeometryFile(filename, entryIdx);
   if (proj.valid())
   {
      return proj.release();
   }

   if(model.valid())
   {
      model = 0;
   }
   
   // first check for override
   //
   if(geomFile.exists()&&kwl.addFile(geomFile.c_str()))
   {
      model =  createProjection(kwl);
      if(model.valid())
      {
         return model.release();
      }
      model = 0;
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: Testing rspfCoarsGridModel" << std::endl;
   }
   
   ifstream input(geomFile.c_str());
   char ecgTest[4];
   input.read((char*)ecgTest, 3);
   ecgTest[3] = '\0';
   input.close();
   if(rspfString(ecgTest) == "eCG")
   {
      rspfKeywordlist kwlTemp;
      kwlTemp.add("type",
                  "rspfCoarseGridModel",
                  true);
      kwlTemp.add("geom_file",
                  geomFile.c_str(),
                  true);
      return createProjection(kwlTemp);
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: testing rspfRpcModel" << std::endl;
   }

   //---
   // Test for quick bird rpc.  Could be either a tiff or nitf so not wrapped
   // around "isNitf()" anymore.
   //---
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: testing rspfQuickbirdRpcModel"
         << std::endl;
   }
   
   rspfRefPtr<rspfQuickbirdRpcModel> qbModel = new rspfQuickbirdRpcModel;
   if(qbModel->parseFile(filename))
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG: returning rspfQuickbirdRpcModel"
            << std::endl;
      }
      model = qbModel.get();
      qbModel = 0;
      return model.release();
   }
   else
   {
      qbModel = 0;
   }
   
   //---
   // Test for ikonos rpc.  Could be tiff or nitf which is handled in
   // parseFile method.
   //---
   rspfRefPtr<rspfIkonosRpcModel> ikModel = new rspfIkonosRpcModel;
   if(ikModel->parseFile(filename))
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG returning rspfQuickbirdRpcModel"
            << std::endl;
      }
      model = ikModel.get();
      ikModel = 0;
      return model.release();
   }
   else
   {
      ikModel = 0;
   }

   if(isNitf(filename))
   {
     if(traceDebug())
     {
        rspfNotify(rspfNotifyLevel_DEBUG)
           << MODULE << " DEBUG: testing rspfNitfRpcModel" << std::endl;
     }

     rspfRefPtr<rspfNitfRpcModel> rpcModel = new rspfNitfRpcModel();
     if ( rpcModel->parseFile(filename, entryIdx) ) // filename = NITF_file
     {
        model = rpcModel.get();
        rpcModel = 0;
        return model.release();
     }
     else
     {
        rpcModel = 0;
     }
     
     if(traceDebug())
     {
        rspfNotify(rspfNotifyLevel_DEBUG)
           << MODULE << " DEBUG: testing rspfIkinosRpcModel" << std::endl;
     }
     
     model = new rspfNitfMapModel(filename); // filename = NITF_file
     if(!model->getErrorStatus())
     {
        return model.release();
     }
     model = 0;
   }
   else if(isLandsat(filename))
   {
      model = new rspfLandSatModel(filename);
      if(!model->getErrorStatus())
      {
         return model.release();
      }
      model = 0;
   }
   
   model = new rspfRS1SarModel(filename);
   if(model->getErrorStatus()!= rspfErrorCodes::RSPF_OK)
   {
      return model.release();
   }
   model = 0;

   rspfFilename spot5Test = geomFile;
   if(!spot5Test.exists())
   {
      spot5Test = geomFile.path();
      spot5Test = spot5Test.dirCat(rspfFilename("METADATA.DIM"));
      if (spot5Test.exists() == false)
      {
         spot5Test = geomFile.path();
         spot5Test = spot5Test.dirCat(rspfFilename("metadata.dim"));
      }
   }
   if(spot5Test.exists())
   {
      rspfRefPtr<rspfSpotDimapSupportData> meta =
         new rspfSpotDimapSupportData;
      if(meta->loadXmlFile(spot5Test))
      {
         model = new rspfSpot5Model(meta.get());
         if(!model->getErrorStatus())
         {
            return model.release();
         }
         model = 0;
      }
   }
   
   model = new rspfCoarseGridModel(geomFile);
   if(model.valid())
   {
      if(!model->getErrorStatus())
      {
         return model.release();
      }
      model = 0;
   }

   return model.release();
}
   
bool rspfSensorModelFactory::isNitf(const rspfFilename& filename)const
{
   std::ifstream in(filename.c_str(), ios::in|ios::binary);
   
   if(in)
   {
      char nitfFile[4];
      in.read((char*)nitfFile, 4);

      return (rspfString(nitfFile,
                          nitfFile+4) == "NITF");
   }

   return false;
}

bool rspfSensorModelFactory::isLandsat(const rspfFilename& filename)const
{
   rspfFilename temp(filename);
   temp.downcase();
   rspfRefPtr<rspfFfL7> ff_headerp;
   if (temp.contains("header.dat"))
   {
       ff_headerp = new rspfFfL5(filename);
   } 
   else 
   {
       ff_headerp = new rspfFfL7(filename);
   }
   bool r = !(ff_headerp->getErrorStatus());
   ff_headerp = 0;
   return r;
}

void rspfSensorModelFactory::findCoarseGrid(rspfFilename& result,
                                             const rspfFilename& geomFile)const
{
   result = geomFile;
   result.setFile(result.fileNoExtension()+"_ocg");
   result.setExtension("dat");
   
   if(!result.exists())
   {
      result = geomFile;
      result.setExtension("dat");
   }
   
   // let's find a .dat file in the current directory
   //
   if(!result.exists())
   {
      result = "";
      rspfDirectory directoryList(geomFile.path());
      rspfFilename file;
      if(directoryList.getFirst(file,
                                rspfDirectory::RSPF_DIR_FILES))
      {
         rspfString testString = "RSPF_DBL_GRID";
         char tempBuf[14];
         do
         {
            if(file.ext().downcase() == "dat")
            {
               std::ifstream in(file.c_str());
               if(in)
               {
                  in.read((char*)tempBuf, 14);
                  in.close();
                  if(rspfString(tempBuf, tempBuf+14) == testString)
                  {
                     result = file;
                  }
               }
               
            }
         }while((directoryList.getNext(file))&&(result == ""));
      }
      // try to find it
   }
}

