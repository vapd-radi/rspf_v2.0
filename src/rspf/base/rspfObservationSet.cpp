//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Storage class for observations.
//----------------------------------------------------------------------------
#include <iostream>
#include <iomanip>

#include <rspf/base/rspfObservationSet.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>

static rspfTrace traceExec  ("rspfObservationSet:exec");
static rspfTrace traceDebug ("rspfObservationSet:debug");



rspfObservationSet::rspfObservationSet() :
theNumAdjPar(0),
theNumMeas(0),
theNumPartials(0)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: rspfObservationSet(): returning..." << std::endl;
}


rspfObservationSet::~rspfObservationSet()
{
   for (rspf_uint32 i=0; i<theImageHandlers.size(); ++i)
      theImageHandlers[i] = 0;
   // for (int i=0; i<theObs.size(); ++i)
   //    theObs[i] = 0;

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: ~rspfObservationSet(): returning..." << std::endl;
}


bool 
rspfObservationSet::addObservation(rspfRefPtr<rspfPointObservation> obs)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"\n rspfObservationSet::addObservation: "<<obs->ID()<<endl;
   }

   theObs.push_back(obs);
   theNumMeas += obs->numMeas();


   // Update image list
   for (rspf_uint32 i=0; i<obs->numImages(); ++i)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)<<"  i="<<i<<endl;
      }

      bool found = false;

      // Check for image already in list
      for (rspf_uint32 j=0; j<theImageFiles.size(); ++j)
      {
         if (theImageFiles[j] == obs->imageFile(i))
         {
            found = true;
            theImageIndex.push_back(j);
            int nAdjPar = obs->numPars(i);
            theNumPartials += nAdjPar;
         }
      }

      // If not found yet, add to image list
      if (!found)
      {
         theImageFiles.push_back(obs->imageFile(i));

         // Geometry
         rspfRefPtr<rspfImageHandler> ih;
         ih = rspfImageHandlerRegistry::instance()->open(obs->imageFile(i));        
         theImageHandlers.push_back(ih);

         theImageIndex.push_back(theImageHandlers.size()-1);

         // Handle rspfAdjustableParameterInterface
         rspfAdjustableParameterInterface* adjParIface = 
            obs->getImageGeom(i)->getAdjustableParameterInterface();
         if (!adjParIface)
         {
            return false;
         }
         else
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)<<"  Found new image... "<<i<<endl;
            }
            int nAdjPar = adjParIface->getNumberOfAdjustableParameters();
            theNumAdjPar   += nAdjPar;
            theNumPartials += nAdjPar;
            theNumAdjParams.push_back(nAdjPar);
         }
      }
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"    theNumAdjPar   = "<<theNumAdjPar<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"    theNumPartials = "<<theNumPartials<<endl;
   }

   return true;
}


rspfImageGeometry* rspfObservationSet::getImageGeom(const int index)
{
   return theImageHandlers[index]->getImageGeometry().get();
}


void rspfObservationSet::setImageGeom(const int index, rspfImageGeometry* geom)
{
   theImageHandlers[index]->setImageGeometry(geom);
}


bool rspfObservationSet::evaluate(NEWMAT::Matrix& measResiduals,
                                   NEWMAT::Matrix& objPartials,
                                   NEWMAT::Matrix& parPartials)
{

   // Dimension output matrices
   measResiduals = NEWMAT::Matrix(numMeas(), 2);
   objPartials   = NEWMAT::Matrix(numMeas()*3, 2);
   parPartials   = NEWMAT::Matrix(theNumPartials, 2);

   int img = 1;
   int cParIndex = 1;
   int cObjIndex = 1;
   for (rspf_uint32 cObs=0; cObs<numObs(); ++cObs)
   {
      int numMeasPerObs = theObs[cObs]->numMeas();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)<<"\n cObs= "<<cObs;
      }

      for (int cImg=0; cImg<numMeasPerObs; ++cImg)
      {
         NEWMAT::Matrix cResid(1, 2);
         theObs[cObs]->getResiduals(cImg, cResid);
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               <<"\n   cImg, img, cObjIndex, cParIndex, cResid: "
               <<cImg<<" "<<img<<" "<<cObjIndex<<" "<<cParIndex<<" "<<cResid;
         }
         measResiduals.Row(img) = cResid;
         img++;

         NEWMAT::Matrix cObjPar(3, 2);
         theObs[cObs]->getObjSpacePartials(cImg, cObjPar);
         objPartials.SubMatrix(cObjIndex,cObjIndex+2,1,2) << cObjPar;
         cObjIndex += 3;

         int numPar = theObs[cObs]->numPars(cImg);
         NEWMAT::Matrix cParamPar(numPar, 2);
         theObs[cObs]->getParameterPartials(cImg, cParamPar);
         parPartials.SubMatrix(cParIndex,cParIndex+numPar-1,1,2) << cParamPar;
         cParIndex += numPar;
      }
   }

   return true;
}


std::ostream& rspfObservationSet::print(std::ostream& os) const
{
   int idx = 0;

   os<<"\nImages:\n";
   rspf_uint32 spaces = 0;
   for (rspf_uint32 i=0; i<numImages(); ++i)
   {
      if (theImageFiles[i].size() > spaces)
         spaces = theImageFiles[i].size();
   }
   for (rspf_uint32 i=0; i<numImages(); ++i)
   {
      int nAdj = theNumAdjParams[i];
      os << " "<<i+1<<": " << setw(spaces) << theImageFiles[i] << "   nPar: "<< setw(2) << nAdj <<endl;
   }

   os<<"\nObservations:";
   for (rspf_uint32 cObs=0; cObs<numObs(); ++cObs)
   {
      os << "\n " << theObs[cObs]->ID();
      if (rspf::isnan(theObs[cObs]->getScore()) == false)
      {
         os << std::setiosflags(std::ios::fixed) << std::setprecision(2);
         os << "\t" << theObs[cObs]->getScore();
      }
      else
      {
         os << "\t" << "nan";
      }
      os << "\t";
      theObs[cObs]->getGroundPoint().print(os);

      int numMeasPerObs = theObs[cObs]->numMeas();
      for (int cImg=0; cImg<numMeasPerObs; ++cImg)
      {
         os << "\n\t\t" << theImageIndex[idx]+1;
         os << "\t";
         theObs[cObs]->getMeasurement(cImg).print(os,1);
         idx++;
      }
   }
   return os;
}
