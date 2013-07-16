//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Class container for multi-ray point observation.
//----------------------------------------------------------------------------

#include <rspf/base/rspfPointObservation.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/base/rspfAdjustableParameterInterface.h>
#include <iostream>
#include <iomanip>


static rspfTrace traceExec  ("rspfPointObservation:exec");
static rspfTrace traceDebug ("rspfPointObservation:debug");


rspfPointObservation::rspfPointObservation() :
   theID(""),
   theScore(0.0)
{
   thePt.makeNan();
   for (int i=0; i<3; ++i) theObsSigmas[i] = 1000.0;
}

rspfPointObservation::rspfPointObservation(const rspfString& anID) : 
   theID(anID),
   theScore(0.0)
{
   thePt.makeNan();
   for (int i=0; i<3; ++i) theObsSigmas[i] = 1000.0;
}

rspfPointObservation::rspfPointObservation(const rspfGpt& aPt,
                                             const rspfString& anID,
                                             const rspfColumnVector3d& latLonHgtSigmas) : 
   thePt(aPt),
   theID(anID),
   theScore(0.0)
{
   for (int i=0; i<3; ++i) theObsSigmas[i] = latLonHgtSigmas[i];    
}

rspfPointObservation::rspfPointObservation(const rspfPointObservation& tpt) : 
   thePt(tpt.thePt),
   theID(tpt.theID),
   theScore(tpt.theScore)
{
   for (int i=0; i<3; ++i) theObsSigmas[i] = tpt.theObsSigmas[i]; 
   theImageHandlers = tpt.theImageHandlers;
   theMeas = tpt.theMeas;  
   theMeasSigmas = tpt.theMeasSigmas;
   theImageFiles = tpt.theImageFiles;
   theNumPars = tpt.theNumPars;
}


rspfPointObservation::~rspfPointObservation()
{
   reset();

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: ~rspfPointObservation(): returning..." << std::endl;
}


void rspfPointObservation::reset()
{
   thePt.makeNan();
   theID = "";
   for (int i=0; i<3; ++i)
      theObsSigmas[i] = 0.0;
   theScore = 0.0;

   for (rspf_uint32 i=0; i<theImageHandlers.size(); ++i)
      theImageHandlers[i] = 0;
   theMeas.clear();
   theMeasSigmas.clear();
   theImageFiles.clear();
   theNumPars.clear();
   theImageHandlers.clear();
}


void rspfPointObservation::addMeasurement(const double& x,
                                           const double& y,
                                           const std::string& imgFile,
                                           const rspfDpt& measSigma)
{
   rspfDpt meas(x, y);
   addMeasurement(meas, imgFile, measSigma);
}


void rspfPointObservation::addMeasurement(const rspfDpt& meas,
                                           const rspfFilename& imgFile,
                                           const rspfDpt& measSigma)
{
   theImageFiles.push_back(imgFile);

   // Image handler
   rspfRefPtr<rspfImageHandler> ih;
   ih = rspfImageHandlerRegistry::instance()->open(imgFile);
   theImageHandlers.push_back(ih);

   // Image measurements & sigmas
   theMeas.push_back(meas);
   theMeasSigmas.push_back(measSigma);

   // Number of parameters
   rspfAdjustableParameterInterface* adjParInterface =
      ih->getImageGeometry().get()->getAdjustableParameterInterface();
   theNumPars.push_back(adjParInterface->getNumberOfAdjustableParameters());
}


rspfImageGeometry* rspfPointObservation::getImageGeom(const int index)
{
   return theImageHandlers[index]->getImageGeometry().get();
}


void rspfPointObservation::setImageGeom(const int index, rspfImageGeometry* imgGeom)
{
   theImageHandlers[index]->setImageGeometry(imgGeom);
}


void rspfPointObservation::getMeasurement(const int index, NEWMAT::Matrix& meas)const
{
   meas = NEWMAT::Matrix(1, 2);
   meas[0][0] = theMeas[index].x;
   meas[0][1] = theMeas[index].y;
}


void rspfPointObservation::getMeasCov(const int index, NEWMAT::Matrix& cov)const
{
   cov = NEWMAT::Matrix(2,2);
   cov = 0.0;

   cov(1,1) = theMeasSigmas[index].x*theMeasSigmas[index].x;
   cov(2,2) = theMeasSigmas[index].y*theMeasSigmas[index].y;
}


void rspfPointObservation::getObsCov(NEWMAT::Matrix& cov)const
{
   cov = NEWMAT::Matrix(3,3);
   cov = 0.0;

   double stdrp = theObsSigmas[0]/thePt.metersPerDegree().y/DEG_PER_RAD; //lat
   double stdrl = theObsSigmas[1]/thePt.metersPerDegree().x/DEG_PER_RAD; //lon

   cov(1,1) = stdrp*stdrp;
   cov(2,2) = stdrl*stdrl;
   cov(3,3) = theObsSigmas[2]*theObsSigmas[2];
}


void rspfPointObservation::getResiduals(const int index, NEWMAT::Matrix& resid)
{
   rspfDpt computedImagePosition;
   getImageGeom(index)->worldToLocal(refGroundPoint(), computedImagePosition);
   rspfDpt residual  = theMeas[index] - computedImagePosition;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"\n========getResiduals======== "<<index;
      theMeas[index].print(rspfNotify(rspfNotifyLevel_DEBUG),1);
      computedImagePosition.print(rspfNotify(rspfNotifyLevel_DEBUG),1);
      refGroundPoint().print(rspfNotify(rspfNotifyLevel_DEBUG));
   }

   resid = NEWMAT::Matrix(1, 2);
   resid[0][0] = residual.x;
   resid[0][1] = residual.y;
}


void rspfPointObservation::getParameterPartials(const int index, NEWMAT::Matrix& parPartials)
{
   parPartials = NEWMAT::Matrix(theNumPars[index], 2);
   getImageGeom(index)->computeImageToGroundPartialsWRTAdjParams(parPartials, refGroundPoint());
}


void rspfPointObservation::getObjSpacePartials(const int index, NEWMAT::Matrix& objPartials)
{
   objPartials = NEWMAT::Matrix(3, 2);
   getImageGeom(index)->computeGroundToImagePartials(objPartials, refGroundPoint());
}


std::ostream& rspfPointObservation::print(std::ostream& os) const
{
   // Observation
   os << "\n "<<theID;
   if (rspf::isnan(theScore) == false)
   {
      os << std::setiosflags(std::ios::fixed) << std::setprecision(2);
      os << "\t" << theScore;
   }
   else
   {
      os << "\t" << "nan";
   }
   os << "\t" << dynamic_cast<const rspfGpt&>(*this);

   // Measurements
   for (rspf_uint32 i=0; i<numMeas(); ++i)
   {
      os << "\n\t\t\t";
      theMeas[i].print(os,1);
   }

   return os;
}


std::ostream& operator<<(std::ostream& os, const rspfPointObservation& pt)
{
   return pt.print(os);
}
