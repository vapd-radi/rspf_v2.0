//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Frederic Claudel (fclaudel@laposte.net)
//         ICT4EO,Meraka Institute,CSIR
//
// Description: generic model optimization, high-level interface
//
//*************************************************************************
#include "rspfModelOptimizer.h"
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/projection/rspfOptimizableProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>

#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime> //for randomizing with time

RTTI_DEF2(rspfModelOptimizer, "rspfModelOptimizer", rspfConnectableObject, rspfProcessInterface);

rspfModelOptimizer::rspfModelOptimizer() :
   rspfConnectableObject(0, 0, 0),
   theTiesetFilename(""),
   theModelDefinitionString("rspfPolynomProjection{1 x y x2 xy y2 x3 y3 xy2 x2y z xz yz}"),
   theGeomOutputFilename("modopt.geom")
{
   //randomize
   std::srand(std::time(0));
}

bool
rspfModelOptimizer::loadGMLTieSet(const rspfString& filepath)
{
   // import GML SimpleTiePoints

   //open file and load full XML
   rspfXmlDocument gmlDoc;
   bool res = gmlDoc.openFile(filepath);
   if (!res) {
      rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: rspfModelOptimizer::loadGMLTieSet cannot open file "<<filepath<<"\n";
      return false;
   }
   
   //parse XML structure for ONE set of tie point
   std::vector< rspfRefPtr< rspfXmlNode > > tieSetList;
   gmlDoc.findNodes(rspfString("/") + rspfTieGptSet::TIEPTSET_TAG, tieSetList);

   if (tieSetList.size() != 1)
   {
      rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: rspfModelOptimizer::loadGMLTieSet need exactly one element of type "<<
         rspfTieGptSet::TIEPTSET_TAG<<", found "<<tieSetList.size()<<"\n";
      return false;
   }

   //get the TieSet object
   res = theTieSet.importFromGmlNode(tieSetList[0]);
   if (!res) {
      rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: rspfModelOptimizer::loadGMLTieSet error when decoding XML tie point info\n";
      return false;
   }

   theTiesetFilename = filepath;
   return true;
}

bool
rspfModelOptimizer::execute()
{
   if(!setupModel(theModelDefinitionString))
   {
      return false;
   }
   rspf_float64 variance_pix2;
   if(optimize(&variance_pix2, NULL)) //TBD: add target variance?
   {
      //display results with unit
      cout<<"RMS="<<std::sqrt(variance_pix2)<<" pixels"<<endl;

      if(theGeomOutputFilename!="")
      {
         if(!exportModel(theGeomOutputFilename)) return false;
      }
   } else {
      return false;
   }

   return true;
}

bool
rspfModelOptimizer::optimize(rspf_float64* result_var_pix2,
                              rspf_float64* target_var_pix2)
{
   //init optional variance to error
   if (result_var_pix2!=NULL) *result_var_pix2 = -1.0;

   if(!theModel.valid()) return false;

   //build optimization proj
   rspfOptimizableProjection* opp = PTR_CAST(rspfOptimizableProjection, theModel.get());

   if (!opp) return false;

   // get meter_per_pixel ratio
   double varRatio;
   if (opp->useForward())
   {
      varRatio = 1.0;
   } else {
      //converts pixel variance (pix2) to image variance (m2)
      rspfDpt mpp(theModel->getMetersPerPixel());
      varRatio = (mpp.x*mpp.x + mpp.y*mpp.y) / 2.0;
   }

   //check that enough info
   unsigned int minTPC = (opp->degreesOfFreedom()+1)/2;
   if (minTPC > theTieSet.size())
   {
      rspfNotify(rspfNotifyLevel_WARN) << 
      "WARNING: rspfModelOptimizer::optimize() should use more than "<<minTPC<<" tie points for model: "<<theModelDefinitionString<<"\n";
   }

   //optimize (using optional target variance)
   rspf_float64  target_var;
   rspf_float64* ptarget_var = NULL;
   if (target_var_pix2)
   {
      target_var = (*target_var_pix2) * varRatio;
      ptarget_var = &target_var;
   }
   
   rspf_float64 variance = opp->optimizeFit(theTieSet, ptarget_var);
   if (variance<0)
   {
      rspfNotify(rspfNotifyLevel_WARN) << 
      "WARNING: rspfModelOptimizer::optimize() failed optimization for "<<theModelDefinitionString<<"\n";
      return false;
   }
   //convert to pixel2 variance
   rspf_float64 variance_pix2 = variance / varRatio;

   //normal exit : return optional variance
   if (result_var_pix2!=NULL) *result_var_pix2 = variance_pix2;
   return true;
}

bool 
rspfModelOptimizer::setupModel(const rspfString& description)
{
   theModel = 0;
   rspfString argProj = description;
   argProj=argProj.after("{").trim();
   if (argProj != "")
   {
      argProj = argProj.before("}");
      argProj.trim();
   }
   rspfString projName=description;
   projName = projName.before("{");

   theModelDefinitionString = description;
   //create projection for rejection
   rspfProjection* inlierProj = rspfProjectionFactoryRegistry::instance()->createProjection(projName);
   if (!inlierProj)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << 
         "FATAL: rspfModelOptimizer::setupModel() can't create projection "<<projName<<"\n";
         return false;
   }
   //check if it is an optimizable projection //TBD: create a factory instead, so to get lists?
   rspfOptimizableProjection* opp = PTR_CAST(rspfOptimizableProjection, inlierProj);
   if (!opp)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << 
         "FATAL: rspfModelOptimizer::setupModel() rejection projection is not optimizable "<<projName<<"\n";
         return false;
   }

   //tune projection parameters
   if (argProj != "")
   {
      bool res = opp->setupOptimizer(argProj);
      if (!res)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << 
         "FATAL: rspfModelOptimizer::setupModel() projection cannot initialize with given args : "<<argProj<<"\n";
         return false;
      }
   }
   theModel = inlierProj;

   return true;
}

bool
rspfModelOptimizer::exportModel(const rspfFilename& geomFile)const
{
   rspfKeywordlist rpc_kwl;
   if(!theModel.valid())
   {
      return false;
   }
   theModel->saveState(rpc_kwl);
   rpc_kwl.write(geomFile);

   //normal exit
   return true;
}

void
rspfModelOptimizer::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   
   rspfString name = property->getName();

   if(name=="model_definition")
   {
      setupModel(property->valueToString());
   }
   else if(name=="gml_tieset_filename")
   {
      loadGMLTieSet(property->valueToString());
   }
   else if(name=="geom_output_filename")
   {
      theGeomOutputFilename = rspfFilename(property->valueToString());
   }
}

rspfRefPtr<rspfProperty>
rspfModelOptimizer::getProperty(const rspfString& name)const
{
   if(name == "model_definition")
   {
      return new rspfStringProperty(name, theModelDefinitionString);
   }
   else if(name == "geom_output_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, theGeomOutputFilename);
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_OUTPUT);
      
      return filenameProp;
   }
   else if(name == "gml_tieset_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, theTiesetFilename);
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_OUTPUT);
      
      return filenameProp;
   }

   return 0;
}

void
rspfModelOptimizer::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back("model_definition");
   propertyNames.push_back("gml_tieset_filename");
   propertyNames.push_back("geom_output_filename");
}
