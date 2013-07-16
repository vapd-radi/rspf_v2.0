//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Executive class for adjustment operations..
//
//----------------------------------------------------------------------------
// $Id$

#include <rspf/base/rspfAdjustmentExecutive.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfWLSBundleSolution.h>
#include <rspf/base/rspfAdjSolutionAttributes.h>
#include <iostream>

static rspfTrace traceExec  ("rspfAdjustmentExecutive:exec");
static rspfTrace traceDebug ("rspfAdjustmentExecutive:debug");



//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::rspfAdjustmentExecutive()
//  
//  Constructor.
//  
//*****************************************************************************
rspfAdjustmentExecutive::rspfAdjustmentExecutive(std::ostream& report)
   :
      theExecValid(false),
      theSol(0),
      theSolAttributes(0),
      theConvCriteria(5.0),
      theMaxIter(7),      
      theMaxIterExceeded(false),
      theSolDiverged(false),
      theSolConverged(false),
      theNumObsInSet(0),
      theNumImages(0),
      theNumParams(0),
      theNumMeasurements(0),
      theRankN(0),
      theMeasResiduals(),
      theObjPartials(),
      theParPartials(),
      theXrms(0.0),
      theYrms(0.0),
      theXmean(0.0),
      theYmean(0.0),
      theSEUW(0),
      theParInitialValues(0),
      theParInitialStdDev(0),
      theParDesc(0),
      theObsInitialValues(0),
      theObsInitialStdDev(0),
      theRep(report)      
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfAdjustmentExecutive::rspfAdjustmentExecutive 1 DEBUG:" << std::endl;
   }
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::rspfAdjustmentExecutive()
//  
//  rspfObservationSet-based constructor.
//  
//*****************************************************************************
rspfAdjustmentExecutive::
rspfAdjustmentExecutive(rspfObservationSet& obsSet, std::ostream& report)
   :
      theExecValid(false),
      theSol(0),
      theSolAttributes(0),
      theConvCriteria(5.0),
      theMaxIter(7),      
      theMaxIterExceeded(false),
      theSolDiverged(false),
      theSolConverged(false),
      theNumObsInSet(0),
      theNumImages(0),
      theNumParams(0),
      theNumMeasurements(0),
      theRankN(0),
      theMeasResiduals(),
      theObjPartials(),
      theParPartials(),
      theXrms(0.0),
      theYrms(0.0),
      theXmean(0.0),
      theYmean(0.0),
      theSEUW(0),
      theParInitialValues(0),
      theParInitialStdDev(0),
      theParDesc(0),
      theObsInitialValues(0),
      theObsInitialStdDev(0),
      theRep(report)      
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfAdjustmentExecutive::rspfAdjustmentExecutive 2 DEBUG:" << std::endl;
   }

   theExecValid = initializeSolution(obsSet);
}


//*****************************************************************************
//  DESTRUCTOR: ~rspfAdjustmentExecutive()
//  
//*****************************************************************************
rspfAdjustmentExecutive::~rspfAdjustmentExecutive()
{
   delete theSolAttributes;
   delete theSol;

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: ~rspfAdjustmentExecutive(): returning..." << std::endl;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::initializeSolution()
//  
//  Execute solution.
//  
//*****************************************************************************
bool rspfAdjustmentExecutive::initializeSolution(rspfObservationSet& obsSet)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfAdjustmentExecutive::initializeSolution DEBUG:" << std::endl; 
   }
   rspfString ts = timeStamp();

   theObsSet = &obsSet;

   // Initial report output
   theRep << "\nrspfAdjustmentExecutive Report     ";
   theRep << ts;
   theRep << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
   theRep << endl;
   theObsSet->print(theRep);
   theRep << endl;

   theExecValid = false;

   // Adjustment traits
   theNumObsInSet     = theObsSet->numObs();
   if (theNumObsInSet == 0)
      return theExecValid;

   theNumImages       = theObsSet->numImages();
   theNumParams       = theObsSet->numAdjPar();
   theNumMeasurements = theObsSet->numMeas();

   theRankN = theNumParams + theNumObsInSet*3;


   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"\n theNumObsInSet     = "<<theNumObsInSet
         <<"\n theNumImages       = "<<theNumImages
         <<"\n theNumParams       = "<<theNumParams
         <<"\n theNumMeasurements = "<<theNumMeasurements
         <<"\n theRankN           = "<<theRankN
         <<std::endl;
   }


   // Instantiate solution attributes
   theSolAttributes = new rspfAdjSolutionAttributes
      (theNumObsInSet, theNumImages, theNumMeasurements, theRankN);


   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"\n\nInitial Parameter Setup....";

      for (int i=0; i<theNumImages; i++)
      {
         rspfAdjustableParameterInterface* iface =
            theObsSet->getImageGeom(i)->getAdjustableParameterInterface();
         int np = iface->getNumberOfAdjustableParameters();
         for (int cp=0; cp<np; ++cp)
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               <<"\n "<<cp<<"  "<<iface->getParameterDescription(cp)
               // <<"=  "<<iface->getAdjustableParameter(cp)
               <<"=  "<<iface->getParameterCenter(cp)
               <<", units= "<<iface->getParameterUnit(cp)
               <<", sigma= "<<iface->getParameterSigma(cp);
         }
      }
      rspfNotify(rspfNotifyLevel_DEBUG)<<std::endl;
   }

   // Save parameter initial values and variances
   int start = 1;
   theSolAttributes->theAdjParCov.ReSize(theNumParams,theNumParams);
   for (int i=0; i<theNumImages; i++)
   {
      rspfAdjustableParameterInterface* iface =
         theObsSet->getImageGeom(i)->getAdjustableParameterInterface();
      int np = iface->getNumberOfAdjustableParameters();
      NEWMAT::Matrix parCov(np,np);
      parCov = 0.0;
      for (int cp=0; cp<np; ++cp)
      {
         // Get the a priori value
         // theParInitialValues.push_back(iface->getAdjustableParameter(cp));
         theParInitialValues.push_back(iface->getParameterCenter(cp));
         theImgs.push_back(i);

         theParDesc.push_back(iface->getParameterDescription(cp));
         double sig = iface->getParameterSigma(cp);
         
         // If parameter is locked, tighten down the sigma
         // TODO: Eventually need better handling of this
         if (iface->isParameterLocked(cp))
            sig /= 1000;

         theParInitialStdDev.push_back(sig);
         parCov(cp+1,cp+1) = sig*sig;
      }
      theSolAttributes->
         theAdjParCov.SubMatrix(start,start+np-1,start,start+np-1) = parCov;
      start += np;
   }

   // Ensure initial estimates for observations
   //   TODO: Currently uses mean of single-ray point drops; should
   //         eventually use multi-ray intersection, which is only
   //         available via the rspfSensorModelTuple class.
   for (int obs=0; obs<theNumObsInSet; obs++)
   {
      // If ground position not set, initialize it
      if (theObsSet->observ(obs)->hasNans())
      {
         double latMean = 0.0;
         double lonMean = 0.0;
         double hgtMean = 0.0;
         rspfGpt gpt;
         rspfDpt ipt;
         for (rspf_uint32 meas=0; meas<theObsSet->observ(obs)->numMeas(); ++meas)
         {
            ipt = theObsSet->observ(obs)->getMeasurement(meas);
            theObsSet->observ(obs)->getImageGeom(meas)->localToWorld(ipt, gpt);
            latMean += gpt.latr();
            lonMean += gpt.lonr();
            hgtMean += gpt.height();
         }
         latMean /= theObsSet->observ(obs)->numMeas();
         lonMean /= theObsSet->observ(obs)->numMeas();
         hgtMean /= theObsSet->observ(obs)->numMeas();
         theObsSet->observ(obs)->Gpt().latr(latMean);
         theObsSet->observ(obs)->Gpt().lonr(lonMean);
         theObsSet->observ(obs)->Gpt().height(hgtMean);
      }
   }


   // Save observation intial values
   for (int obs=0; obs<theNumObsInSet; obs++)
   {
      theObsInitialValues.push_back(theObsSet->observ(obs)->Gpt().latr());
      theObsInitialValues.push_back(theObsSet->observ(obs)->Gpt().lonr());
      theObsInitialValues.push_back(theObsSet->observ(obs)->Gpt().height());
   }


   // Load obj/image covariance data
   int cMeas = 1;
   theSolAttributes->theImagePtCov.ReSize(theNumMeasurements*2,2);
   theSolAttributes->theObjectPtCov.ReSize(theNumObsInSet*3,3);

   for (int obs=0; obs<theNumObsInSet; ++obs)
   {
      NEWMAT::Matrix ocov;
      theObsSet->observ(obs)->getObsCov(ocov);
      int idx = obs*3+1;
      theSolAttributes->theObjectPtCov.Rows(idx,idx+2) = ocov;
      for (int i=1; i<=3; ++i)
         theObsInitialStdDev.push_back(sqrt(ocov(i,i)));
      for (rspf_uint32 meas=0; meas<theObsSet->observ(obs)->numMeas(); ++meas)
      {
         NEWMAT::Matrix mcov;
         theObsSet->observ(obs)->getMeasCov((int)meas, mcov);
         int start = (cMeas-1)*2 + 1;
         theSolAttributes->theImagePtCov.Rows(start,start+1) = mcov;
         cMeas++;   
      }
   }


   // Load obj/image xref map
   for (int obs=0; obs<theNumObsInSet; ++obs)
   {
      for (rspf_uint32 meas=0; meas<theObsSet->observ(obs)->numMeas(); ++meas)
      {
         int img = theObsSet->imIndex(meas);
         theSolAttributes->theObjImgXref.insert(pair<int, int>(obs, img));
      }
   }

   // Load image/numpar xref map
   for (int img=0; img<theNumImages; ++img)
   {
      int numpar = theObsSet->adjParCount(img);
      theSolAttributes->theImgNumparXref.insert(pair<int, int>(img, numpar));
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"\ntheObjImgXref multimap  Obs/Img ....\n";
      for (ObjImgMapIter_t it =theSolAttributes->theObjImgXref.begin();
                           it!=theSolAttributes->theObjImgXref.end(); ++it)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            <<"  "<<it->first<<"    "<<it->second<<endl;
      }
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"\ntheImgNumparXref map  Img/Numpar ....\n";
      for (ImgNumparMapIter_t it =theSolAttributes->theImgNumparXref.begin();
                              it!=theSolAttributes->theImgNumparXref.end(); ++it)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            <<"  "<<it->first<<"    "<<it->second<<endl;
      }
   }

 
   theRep << "\n Iteration 0...";

   updateParameters();

   // Perform initial (0th iteration) observation evaluation
   theObsSet->evaluate(theMeasResiduals, theObjPartials, theParPartials);

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"\n theObjPartials\n"
         <<theObjPartials;
   }

   // Report residual summary
   printResidualSummary(theRep);

   // Residual statistics
   computeResidualStatistics(theMeasResiduals);

   // Load partials
   theSolAttributes->theObjPartials = theObjPartials;
   theSolAttributes->theParPartials = theParPartials;

   // Load residuals
   theSolAttributes->theMeasResiduals = theMeasResiduals;

   // Initial standard error of unit weight
   theSEUW.push_back(computeSEUW());

   // Instantiate solution object
   theSol = new rspfWLSBundleSolution();

   theExecValid = true;

   return theExecValid;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::runSolution()
//  
//  Execute solution.
//  
//*****************************************************************************
bool rspfAdjustmentExecutive::runSolution()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfAdjustmentExecutive::runSolution DEBUG:" << std::endl;
   }


   // Iterative loop
   int iter = 0;

   while (iter < theMaxIter &&
          !theSolConverged &&
          !theSolDiverged &&
          theExecValid)
   {
      iter++;

      theRep << "\n Iteration "<<iter<<"...";

      // Execute solution
      theExecValid = theSol->run(theSolAttributes);


      if (theExecValid)
      {
         // Report corrections
         printParameterCorrectionSummary(theRep);
         printObservationCorrectionSummary(theRep);

         // Update adjustable parameters
         updateParameters();

         // Update ground points
         updateObservations();

         // Perform observation evaluation
         theObsSet->evaluate(theMeasResiduals, theObjPartials, theParPartials);

         // Load partials
         theSolAttributes->theObjPartials = theObjPartials;
         theSolAttributes->theParPartials = theParPartials;

         // Load residuals
         theSolAttributes->theMeasResiduals = theMeasResiduals;

         // Report residual summary
         printResidualSummary(theRep);

         // Residual statistics
         computeResidualStatistics(theMeasResiduals);

         // Compute SEUW for current iteration
         theSEUW.push_back(computeSEUW());

         // Check convergence
         double percChange = 
         fabs((theSEUW[iter]-theSEUW[iter-1])/theSEUW[iter-1])*100.0;

         if ((percChange <= theConvCriteria) && iter>1)
            theSolConverged = true;
         else if (iter == theMaxIter)
            theMaxIterExceeded = true;
         else if (iter >= 3)
         {
            if (theSEUW[iter]   > theSEUW[iter-1] &&
                theSEUW[iter-1] > theSEUW[iter-2] &&
                theSEUW[iter-2] > theSEUW[iter-3])
               theSolDiverged = true;
         }
         else
            theSolConverged = false;
      }
   }

   return theExecValid;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::updateParameters()
//  
//  Update adjustable parameters with current iteration corrections.
//  
//*****************************************************************************
bool rspfAdjustmentExecutive::updateParameters()
{
   // Update local geometries
   int currPar = 1;
   for (int img=0; img<theNumImages; ++img)
   {
      rspfAdjustableParameterInterface* iface =
         theObsSet->getImageGeom(img)->getAdjustableParameterInterface();

      for (int par=0; par<theSolAttributes->theImgNumparXref[img]; ++par)
      {
         // double middle = iface->getAdjustableParameter(par);
         // middle += theSolAttributes->theLastCorrections(currPar);
         // iface->setAdjustableParameter(par, middle, true);

         double middle = iface->getParameterCenter(par);
         middle += theSolAttributes->theLastCorrections(currPar);
         iface->setParameterCenter(par, middle, true);

         currPar++;    
      }
   }

   // Copy updated local geometries to observation geometries
   for (int img=0; img<theNumImages; ++img)
   {
      for (int obs=0; obs<theNumObsInSet; ++obs)
      {
         for (rspf_uint32 imgInObs=0; imgInObs<theObsSet->observ(obs)->numImages(); ++imgInObs)
         {
            if (theObsSet->observ(obs)->imageFile(imgInObs) ==
                theObsSet->imageFile(img))
            {
               theObsSet->observ(obs)->setImageGeom
                  (imgInObs, theObsSet->getImageGeom(img));
            }
         }
      }
   }

   return true;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::updateObservations()
//  
//  Update adjustable ground points with current iteration corrections.
//  
//*****************************************************************************
bool rspfAdjustmentExecutive::updateObservations()
{
   int currPar = theNumParams + 1;
   for (int obs=0; obs<theNumObsInSet; ++obs)
   {
      double lat = theObsSet->observ(obs)->Gpt().latr();
      lat += theSolAttributes->theLastCorrections(currPar);
      currPar++;    
      double lon = theObsSet->observ(obs)->Gpt().lonr();
      lon += theSolAttributes->theLastCorrections(currPar);
      currPar++;    
      double hgt = theObsSet->observ(obs)->Gpt().height();
      hgt += theSolAttributes->theLastCorrections(currPar);
      currPar++;    
      theObsSet->observ(obs)->Gpt().latr(lat);
      theObsSet->observ(obs)->Gpt().lonr(lon);
      theObsSet->observ(obs)->Gpt().height(hgt);
   }

   return true;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::summarizeSolution()
//  
//  Execute solution.
//  
//*****************************************************************************
void rspfAdjustmentExecutive::summarizeSolution() const
{
   theRep << "\nrspfAdjustmentExecutive Summary..."<<std::endl;
   theRep << " Valid Exec:         "<<(theExecValid?"true":"false")<<std::endl;
   theRep << " Nbr Ground Pts:     "<<theNumObsInSet<<std::endl;
   theRep << " Nbr Image Points:   "<<theNumMeasurements<<std::endl;
   theRep << " Nbr Images:         "<<theNumImages<<std::endl;
   theRep << " Nbr Parameters:     "<<theNumParams<<std::endl;
   theRep << " -------------------------"<<std::endl;
   theRep << " Solution Converged: "<<(theSolConverged?"true":"false")<<std::endl;
   theRep << " Solution Diverged:  "<<(theSolDiverged?"true":"false")<<std::endl;
   theRep << " Max Iter Exceeded:  "<<(theMaxIterExceeded?"true":"false")<<std::endl;
   theRep << " Max Iterations:     "<<theMaxIter<<std::endl;
   theRep << " Convergence Crit:   "<<theConvCriteria<<"%"<<std::endl;

   // SEUW history
   theRep << "\n SEUW Trace...";
   theRep << "\n   Iter        SEUW";
   for (rspf_uint32 iter=0; iter<theSEUW.size(); ++iter)
      theRep << "\n"<<setw(7)<<iter<<setw(12)<<setprecision(3)<<theSEUW[iter];

   rspfString ts = timeStamp();
   theRep << "\n";
   theRep << "\n" << ts;
   theRep << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
   theRep << endl;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::computeResidualStatistics()
//  
//  Statistical evaluation.
//  
//*****************************************************************************
bool rspfAdjustmentExecutive::computeResidualStatistics(NEWMAT::Matrix& res)
{
   theXmean = 0.0;
   theYmean = 0.0;
   theXrms  = 0.0;
   theYrms  = 0.0;
   for (int n=1; n<=res.Nrows();++n)
   {
      theXmean += res(n,1);
      theYmean += res(n,2);
      theXrms  += res(n,1)*res(n,1);
      theYrms  += res(n,2)*res(n,2);
   }

   theXmean /= theNumMeasurements;
   theYmean /= theNumMeasurements;
   theXrms   = sqrt(theXrms/theNumMeasurements);
   theYrms   = sqrt(theYrms/theNumMeasurements);

   theRep<<"\n";
   theRep<<" ______________Mean:";
   theRep<<setprecision(1)<<setw(8)<<theXmean;
   theRep<<setprecision(1)<<setw(8)<<theYmean;
   theRep<<"    RMS:";
   theRep<<setprecision(1)<<setw(8)<<theXrms;
   theRep<<setprecision(1)<<setw(8)<<theYrms;
   theRep<<endl;

   return true;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::computeSEUW()
//  
//  Standard error of unit weight evaluation.
//  
//*****************************************************************************
double rspfAdjustmentExecutive::computeSEUW()
{
   double vuw = 0.0;

   // Observation contributions
   int cMeas = 1;
   for (int obs=0; obs<theNumObsInSet; ++obs)
   {
      NEWMAT::ColumnVector tc(3);
      NEWMAT::Matrix cov(3,3);
      int idx = theNumParams + 3*obs + 1;
      tc  = theSolAttributes->theTotalCorrections.Rows(idx, idx+2);
      idx -= theNumParams;
      cov = theSolAttributes->theObjectPtCov.SubMatrix(idx,idx+2,1,3);
      for (int i=1; i<=3; ++i)
      {
         vuw += tc(i)*tc(i)/cov(i,i);
      }
      for (rspf_uint32 meas=0; meas<theObsSet->observ(obs)->numMeas(); ++meas)
      {
         NEWMAT::RowVector res(2);
         NEWMAT::Matrix cov(2,2);
         res = theSolAttributes->theMeasResiduals.Row(cMeas);
         int start = (cMeas-1)*2 + 1;
         cov = theSolAttributes->theImagePtCov.SubMatrix(start,start+1,1,2);
         for (int i=1; i<=2; ++i)
         {
            vuw += res(i)*res(i)/cov(i,i);
         }
         cMeas++;
      }
   }

   // Parameter contributions
   NEWMAT::ColumnVector tc(theNumParams);
   tc = theSolAttributes->theTotalCorrections.Rows(1, theNumParams);
   for (int par=0; par<theNumParams; ++par)
   {
      vuw += tc(par+1)*tc(par+1)/(theParInitialStdDev[par]*theParInitialStdDev[par]);
   }

   // DF
   double df = theNumMeasurements*2 - theRankN;
   if (df<=0.0)
      df = 1.0;

   // SEUW
   double seuw = sqrt(vuw/df);

   return seuw;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::printParameterCorrectionSummary()
//  
//  Print info.
//  
//*****************************************************************************
std::ostream& rspfAdjustmentExecutive::
printParameterCorrectionSummary(std::ostream& out) const
{
   out<<"\nParameter Corrections...";
   out<<"\n  n im        parameter    a_priori  total_corr   last_corr initial_std    prop_std";
   for (int pc=1; pc<=theNumParams; ++pc)
   {
      out<<"\n "<<setprecision(5)<<setw(2)<<pc;
      out<<setw(3)<<theImgs[pc-1]+1;
      out<<setw(17)<<theParDesc[pc-1];
      out<<setw(12)<<theParInitialValues[pc-1];
      out<<setw(12)<<theSolAttributes->theTotalCorrections(pc);
      out<<setw(12)<<theSolAttributes->theLastCorrections(pc);
      out<<setw(12)<<theParInitialStdDev[pc-1];
      out<<setw(12)<<sqrt(theSolAttributes->theFullCovMatrix(pc,pc));
   }
   out<<endl;

   return out;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::printObservationCorrectionSummary()
//  
//*****************************************************************************
std::ostream& rspfAdjustmentExecutive::
printObservationCorrectionSummary(std::ostream& out) const
{
   out<<"\nObservation Corrections...";
   out<<"\n  n         observation    a_priori  total_corr   last_corr initial_std    prop_std";
   for (int obs=0; obs<theNumObsInSet; ++obs)
   {
      out<<"\n "<<setprecision(5)<<setw(2)<<obs+1;
      out<<" "<<setw(19)<<theObsSet->observ(obs)->ID();
      double mPerRadp = theObsSet->observ(obs)->Gpt().metersPerDegree().y*DEG_PER_RAD;
      double mPerRadl = theObsSet->observ(obs)->Gpt().metersPerDegree().x*DEG_PER_RAD;
      // rspf_uint32 iobs = obs*3+1;

      for (int k=0; k<3; ++k)
      {
         int idx = theNumParams + obs*3 + k + 1;
         if (k<2)
            out<<setw(12)<<theObsInitialValues[obs*3+k]*DEG_PER_RAD;
         else
            out<<setw(12)<<theObsInitialValues[obs*3+k];
         double factor;
         if (k==0)
            factor = mPerRadp;
         else if (k==1)
            factor = mPerRadl;
         else
            factor = 1.0;
         out<<setw(12)<<theSolAttributes->theTotalCorrections(idx)*factor;
         out<<setw(12)<<theSolAttributes->theLastCorrections(idx)*factor;
         out<<setw(12)<<theObsInitialStdDev[obs*3+k]*factor;
         out<<setw(12)<<sqrt(theSolAttributes->theFullCovMatrix(idx,idx))*factor;
         out<<endl<<"                       ";
      }
   }
   out<<endl;

   return out;
}


//*****************************************************************************
//  METHOD: rspfAdjustmentExecutive::printResidualSummary()
//  
//*****************************************************************************
std::ostream& rspfAdjustmentExecutive::
printResidualSummary(std::ostream& out) const
{
   out<<"\nMeasurement Residuals...";
   out<<"\n observation   image    samp    line    initial_meas";
   int j = 0;
   for (int obs=0; obs<theNumObsInSet; obs++)
   {
      int numMeasPerObs = theObsSet->observ(obs)->numMeas();
      for (int meas=0; meas<numMeasPerObs; ++meas)
      {
         int imIdx = theObsSet->imIndex(j);
         ++j;
         out<<"\n";
         out<<setw(12)<<theObsSet->observ(obs)->ID();
         out<<setw(8)<<imIdx+1;
         out<<setprecision(1)<<setw(8)<<theMeasResiduals(j,1);
         out<<setprecision(1)<<setw(8)<<theMeasResiduals(j,2);
         out<<"    ";
         theObsSet->observ(obs)->getMeasurement(meas).print(out,1);
      }
      out<<endl;
   }

   return out;
}
