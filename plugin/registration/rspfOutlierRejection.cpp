
#include "rspfOutlierRejection.h"
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/projection/rspfOptimizableProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <vector>
#include <algorithm>
#include <cmath>

RTTI_DEF1(rspfOutlierRejection, "rspfOutlierRejection", rspfModelOptimizer);

rspfOutlierRejection::rspfOutlierRejection() : 
       rspfModelOptimizer(),
       theInlierRatio(0.6),
       theInlierImageAccuracy(1.0),
       theInlierOutputFilename("inlier.xml")
{
   //change default name to avoid confusion
   setGeomOutputFilename("rejout.xml");
}

bool
rspfOutlierRejection::saveGMLTieSet(const rspfString& filepath)
{
   //create a XML document
   rspfXmlDocument gmlDoc;
   
   gmlDoc.initRoot(theTieSet.exportAsGmlNode());

   bool res = gmlDoc.write(filepath);
   if (!res) {
      rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: rspfOutlierRejection::loadGMLTieSet cannot save tie points as file "<<filepath<<"\n";
      return false;
   }

   return true;
}

/*!
 * removeOutliers(): use RANdom SAmple Consensus method (RANSAC)
 * but stops at the first good fit, without looking for better fits
 */
bool
rspfOutlierRejection::removeOutliers(rspf_float64* result_var_pix2,
                                      rspf_float64* target_var_pix2)
{
   //init optional variance to error
   if (result_var_pix2!=NULL) *result_var_pix2 = -1.0;

   if(!theModel.valid()) return false;
   
   static const double SUCCESS = 0.97; //required chance of success for RANSAC
   static const double NSTD    = 2.2;  //deviations allowed around mean
   //check args
   if ((theInlierRatio<=0.0) || (theInlierRatio>1.0))
   {
      rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: rspfOutlierRejection::removeOutliers() bad theInlierRatio\n";
      return false;
   }
   rspfOptimizableProjection* optProj = PTR_CAST(rspfOptimizableProjection, theModel.get());
   if(!optProj) return false;
   
   //direction of transform (cached)
   bool useForward = optProj->useForward();

   //keep a copy of projection if it needs initialization
   rspfKeywordlist initialState;
   bool nis=optProj->needsInitialState(); //cached
   if (nis)
   {
      //save state //TBD: make the copy/restore faster
      if (!theModel->saveState(initialState))
      {
         rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: rspfOutlierRejection::removeOutliers() can't save projection initial state\n";
      }
   }

   // convert error to ground if necessary
   double imageToGroundRatio;
   if (useForward)
   {
      imageToGroundRatio = 1.0;
   } else {
      //convert image error to ground error (meters)
      imageToGroundRatio = theModel->getMetersPerPixel().length()/std::sqrt(2.0);
   }
   double maxFitError = imageToGroundRatio * theInlierImageAccuracy; //either pixels or meters

   unsigned long dof = optProj->degreesOfFreedom();
   unsigned long dofPerPoint = 2L; //use 2 because only 2 uncorelated equations (height is correlated)
   unsigned long miniPts = (dof+ dofPerPoint-1) / dofPerPoint; //TBD: shouldn't be lower than nbsamples
   unsigned long miniInliers = (unsigned long)ceil(theTieSet.size()*theInlierRatio);

   //calculate maximum number of iterations (average and std)
   double probaAllGood = std::pow(theInlierRatio, (int)miniPts);
   double avg_iter_max = std::log(SUCCESS) / std::log(1.0-probaAllGood);
   double std_iter_max = std::sqrt(1.0-probaAllGood) / probaAllGood;
   long iter_max = (long)ceil(avg_iter_max + NSTD * std_iter_max);
   rspfNotify(rspfNotifyLevel_WARN) << 
         "INFO: max number of RANSAC iterations = "<<iter_max<<"\n"; //TBR

   unsigned long nbsamples = theTieSet.size();

   rspfNotify(rspfNotifyLevel_WARN) << 
        "INFO: samples before RANSAC = "<<nbsamples<<"\n"; //TBR

   //array for random draws : init on identity permutation
   //high memory cost?
   vector<unsigned long> vshuf(nbsamples);

   //init random / inliers tie point set from current tie set, but with no ties
   rspfTieGptSet randSelection;
   randSelection.setMasterPath(theTieSet.getMasterPath());
   randSelection.setSlavePath(theTieSet.getSlavePath());
   randSelection.setImageCov(theTieSet.getImageCov());
   randSelection.setGroundCov(theTieSet.getGroundCov());
   double bestfit;

   long iterations = 0;
   unsigned long npos=0;
   while(iterations < iter_max)
   {

      //TBD: find out why we need to reset that index table at every iteration
      for(unsigned long c=0; c<nbsamples; ++c) vshuf[c]=c;

      //randomly select miniPts indices
      for(unsigned long s=0;s<miniPts;++s)
      {
         unsigned long rpick = s+(unsigned long)(((double)std::rand())/(1.0+RAND_MAX)*(nbsamples-s)); //between 0 and nbsamples-1 inc.
         vshuf[s]     = rpick;
         vshuf[rpick] = s;
      }

      //use miniPts random tie points for selection
      randSelection.clearTiePoints();
      for(unsigned long c=0;c<miniPts;++c) randSelection.addTiePoint( theTieSet.getTiePoints()[vshuf[c]] );
      
      //optimize model with random selection
      double fitvar = optProj->optimizeFit(randSelection, NULL); //best possible fit, do not use target variance here

      //if optimization goes fine, then test it further
      if ((fitvar>=0) && (fitvar <= maxFitError*maxFitError)) //RMS must be below max distance
      {
         //find other compatible points (and enqueue them)
         npos = miniPts;
         double perr;
         for(unsigned long c=miniPts; c<nbsamples; ++c) 
         {
            rspfRefPtr< rspfTieGpt > rt = theTieSet.getTiePoints()[vshuf[c]];
            if (useForward)
            {
               perr = (theModel->forward(*rt) - rt->tie).length(); //image error, unit pixels
            } else {
               perr = (rspfEcefPoint(theModel->inverse(rt->tie)) - rspfEcefPoint(*rt)).magnitude(); //ground error, unit meters
            }
            if (perr <= maxFitError)
            {
               //keep the indices after the miniPts first elts
               vshuf[npos] = c;
               vshuf[c]    = npos;
               ++npos;
            }
         }

         //DEBUG TBR
         rspfNotify(rspfNotifyLevel_WARN) << 
            "INFO: model finds "<<100.0*npos/((double)nbsamples)<<"% inliers\n"; //TBR

         if (npos >= miniInliers )
         {
            //re-optimize model with the new possible inliers :
            //-add new inliers to sleection
            for(unsigned long c=miniPts;c<npos;++c) randSelection.addTiePoint( theTieSet.getTiePoints()[vshuf[c]] );

            //use target variance
            if (target_var_pix2!=NULL)
            {
               rspf_float64 target_var = (*target_var_pix2) * imageToGroundRatio * imageToGroundRatio;
               bestfit = optProj->optimizeFit(randSelection, &target_var); //best possible fit with target variance
            } else {
               bestfit = optProj->optimizeFit(randSelection); //best possible fit
            }
            //assume fit to be OK (which might NOT be the case...)
            //TBD iterate over minimal number of times
            break;
         }
      }

      if (nis)
      {
         //restore initial state //TBD: make the copy/restore faster
         if (!theModel->loadState(initialState))
         {
            rspfNotify(rspfNotifyLevel_WARN) << 
            "WARNING: rspfOutlierRejection::removeOutliers() can't reload projection initial state at iteration "<<iterations<<"\n";
         }
      }

      iterations++;
   }
   if (iterations >= iter_max) 
   {
      rspfNotify(rspfNotifyLevel_WARN) << 
         "WARNING: RANSAC didn't find a solution after "<<iterations<<" iterations"<<endl;
      return false;
   }

   //keep inliers
   theTieSet = randSelection;
   //display results, with RMS always in pixels
   rspfNotify(rspfNotifyLevel_WARN) << 
         "INFO: RANSAC solution found after "<<iterations<<" iterations, nbinliers="<<npos<<endl;

   //convert to pixel2 variance
   rspf_float64 variance_pix2 = bestfit / (imageToGroundRatio * imageToGroundRatio);
   //normal exit : return optional variance
   if (result_var_pix2!=NULL) *result_var_pix2 = variance_pix2;

   return true;
}

void
rspfOutlierRejection::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   
   rspfString name = property->getName();

   if(name == "inlier_output_filename")
   {
      theInlierOutputFilename = rspfFilename(property->valueToString());
   }
   else if(name =="inlier_ratio")
   {
      setInlierRatio(property->valueToString().toDouble());
   }
   else if(name == "inlier_image_accuracy")
   {
      setInlierImageAccuracy(property->valueToString().toDouble());
   }
   else {
      rspfModelOptimizer::setProperty(property);
   }
}

rspfRefPtr<rspfProperty>
rspfOutlierRejection::getProperty(const rspfString& name)const
{
   if(name == "inlier_ratio")
   {
      return new rspfNumericProperty(name, rspfString::toString(theInlierRatio), 0.0, 1.0);
   }
   else if(name == "inlier_image_accuracy")
   {
      return new rspfNumericProperty(name, rspfString::toString(theInlierImageAccuracy), 0.0, 1e50);
   }
   else if(name == "inlier_output_filename")
   {
      return new rspfFilenameProperty(name,theInlierOutputFilename); 
   }

   return rspfModelOptimizer::getProperty(name);
}

void
rspfOutlierRejection::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfModelOptimizer::getPropertyNames(propertyNames);

   propertyNames.push_back("inlier_ratio");
   propertyNames.push_back("inlier_image_accuracy");
   propertyNames.push_back("inlier_output_filename");
}

bool
rspfOutlierRejection::execute()
{
   if(!setupModel(theModelDefinitionString))
   {
      return false;
   }
   rspf_float64 variance_pix2;
   if(removeOutliers(&variance_pix2, NULL)) //TBD : use target variance
   {
      //display RMS with unit
      cout<<"RMS="<<std::sqrt(variance_pix2)<<" pixels"<<endl;

      //export results
      if(theGeomOutputFilename!="")
      {
         if(!exportModel(theGeomOutputFilename)) return false;
      }
      if(theInlierOutputFilename!="")
      {
         if(!saveGMLTieSet(theInlierOutputFilename)) return false;
      }
   }
   else
   {
      return false;
   }

   return true;
}
