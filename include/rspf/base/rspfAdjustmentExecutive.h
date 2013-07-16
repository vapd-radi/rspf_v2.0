//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
// test
//
// Description: Executive class for adjustment operations.
//----------------------------------------------------------------------------
#ifndef rspfAdjustmentExecutive_HEADER
#define rspfAdjustmentExecutive_HEADER

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfObservationSet.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfString.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/matrix/newmatio.h>

#include <ctime>
#include <vector>
#include <iostream>

class rspfWLSBundleSolution;
class rspfAdjSolutionAttributes;


class RSPF_DLL rspfAdjustmentExecutive : public rspfObject
{
public:

   /**
    * @brief constructor
    */
   rspfAdjustmentExecutive(std::ostream& report);
   
   /** @brief rspfObservationSet constructor
    *
    * @param obsSet     tiepoint observation set.
    */
   rspfAdjustmentExecutive(rspfObservationSet& obsSet, std::ostream& report);
   

   /** @brief destructor */
   ~rspfAdjustmentExecutive();
   
   /**
    * @brief initialize adjustment solution
    *
    * @param obsSet     tiepoint observation set.
    *
    * @return true on success, false on error.
    */
   bool initializeSolution(rspfObservationSet& obsSet);

   /**
    * @brief run adjustment solution
    *
    * @return true on success, false on error.
    */
   bool runSolution();

   /**
    * @brief summarize solution
    */
   void summarizeSolution() const;


   /**
    * @brief compute mean and RMS error of residuals
    *
    * @param res   image residual matrix.
    *
    * @return true on success, false on error.
    */
   bool computeResidualStatistics(NEWMAT::Matrix& res);


   /**
    * @brief compute SEUW
    *
    * @return   standard error of unit weight.
    */
   double computeSEUW();
   
   /**
    * @brief Print parameter correction method.
    */
   std::ostream& printParameterCorrectionSummary(std::ostream& out) const;
   
   /**
    * @brief Print observation correction method.
    */
   std::ostream& printObservationCorrectionSummary(std::ostream& out) const;
   
   /**
    * @brief Print residuals method.
    */
   std::ostream& printResidualSummary(std::ostream& out) const;


   inline bool isValid() const { return theExecValid; }


   // Generate time stamp
   inline rspfString timeStamp()const
   {
      char timeString[22];
      time_t now = time(NULL);
      strftime(timeString, 22, "%a %m.%d.%y %H:%M:%S", localtime(&now));
      string timeStamp(timeString);
      return timeStamp;
   }

protected:
   bool theExecValid;

   // Observation set
   rspfObservationSet* theObsSet;

   // Optimizer
   rspfWLSBundleSolution* theSol;

   // Attribute interface
   rspfAdjSolutionAttributes* theSolAttributes;

   // Status parameters
   double theConvCriteria;
   int    theMaxIter;
   bool   theMaxIterExceeded;
   bool   theSolDiverged;
   bool   theSolConverged;

   // Traits
   int theNumObsInSet;
   int theNumImages;
   int theNumParams;
   int theNumMeasurements;
   int theRankN;

   // Solution arrays
   NEWMAT::Matrix theMeasResiduals; // theNumMeasurements X 2
   NEWMAT::Matrix theObjPartials;   // theNumObjObs*3 X 2
   NEWMAT::Matrix theParPartials;   // theNumImages*(npar/image) X 2

   // Statistics
   double theXrms;
   double theYrms;
   double theXmean;
   double theYmean;
   std::vector<double> theSEUW;

   // Adjustable parameter info
   std::vector<double> theParInitialValues;
   std::vector<double> theParInitialStdDev;
   std::vector<rspfString> theParDesc;
   std::vector<int> theImgs;

   // Observation info
   std::vector<double> theObsInitialValues;
   std::vector<double> theObsInitialStdDev;
   
   std::ostream& theRep;
                                  
   
   /**
    * @brief Update adjustable parameters.
    */
   bool updateParameters();
   
   /**
    * @brief Update observatin.
    */
   bool updateObservations();

};

#endif // #ifndef rspfAdjustmentExecutive_HEADER
