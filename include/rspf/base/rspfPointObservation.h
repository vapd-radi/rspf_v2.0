//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Class container for multi-ray point observation.
//----------------------------------------------------------------------------
#ifndef rspfPointObservation_HEADER
#define rspfPointObservation_HEADER

#include <iostream>
#include <vector>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageGeometry.h>


class RSPF_DLL rspfPointObservation : public rspfObject
{
public:

   rspfPointObservation();

   rspfPointObservation(const rspfString& anID);

   rspfPointObservation(const rspfGpt& aPt,
                         const rspfString& anID,
                         const rspfColumnVector3d& latLonHgtSigmas);
         
   rspfPointObservation(const rspfPointObservation& tpt);

   ~rspfPointObservation();

   void reset();

   inline rspfGpt& Gpt() { return thePt; }
   inline rspfString ID()const { return theID; }
   inline void setID(const std::string& anID) { theID=anID; }
   inline unsigned int numImages()const { return (unsigned int)theImageFiles.size(); }
   inline unsigned int   numMeas()const { return (unsigned int)theMeas.size(); }

   // Image geometry access
   rspfImageGeometry* getImageGeom(const int index);
   void setImageGeom(const int index, rspfImageGeometry* imgGeom);

   // Image file access
   inline rspfFilename imageFile(const int index)const { return theImageFiles[index]; }

   // Measurement addition
   void addMeasurement(const rspfDpt& meas,
                       const rspfFilename& imgFile,
                       const rspfDpt& measSigma = rspfDpt(1.0,1.0));
   void addMeasurement(const double& x,
                       const double& y,
                       const std::string& imgFile,
                       const rspfDpt& measSigma = rspfDpt(1.0,1.0));

   // Measurement access
   void getResiduals(const int index, NEWMAT::Matrix& resid);

   void getMeasCov(const int index, NEWMAT::Matrix& cov)const;
   void getObsCov(NEWMAT::Matrix& cov)const;

   void getMeasurement(const int index, NEWMAT::Matrix& meas)const;
   inline rspfDpt getMeasurement(const int index)const { return theMeas[index]; }

   inline int numPars(const int index)const { return theNumPars[index]; }


   // Partial derivative access
   void getParameterPartials(const int index, NEWMAT::Matrix& parPartials);
   void  getObjSpacePartials(const int index, NEWMAT::Matrix& objPartials);


   const rspfPointObservation& operator=(const rspfPointObservation&);

   inline void setGroundPoint(const double& lat,
                              const double& lon,
                              const double& hgt)
   {
      thePt.latd(lat);
      thePt.lond(lon);
      thePt.height(hgt);
   }

   inline void setGroundSigmas(const double& latSig,
                               const double& lonSig,
                               const double& hgtSig)
   {
      theObsSigmas[0] = latSig;
      theObsSigmas[1] = lonSig;
      theObsSigmas[2] = hgtSig;
   }

   inline void            setGroundPoint(const rspfGpt& mPt) { thePt = mPt; }
   inline const rspfGpt& getGroundPoint()const               { return thePt; }
   inline       rspfGpt& refGroundPoint()                    { return thePt; }

   // Note: Member variable "theScore" is currently unused.  Eventually, it is
   //       intended as a figure-of-merit representation.
   inline void          setScore(const double& s) { theScore = s; }
   inline const double& getScore()const           { return theScore; }
   inline       double& refScore()                { return theScore; }

   void makeNan() 
   {
      thePt.makeNan();
      theScore=rspf::nan();
   }
   
   bool hasNans()const
   {
      return (thePt.hasNans() || (rspf::isnan(theScore)));
   }
   
   bool isNan()const
   {
      return (thePt.isNan() && (rspf::isnan(theScore)));
   }
   std::ostream& print(std::ostream& os) const;

   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfPointObservation& pt);
   /**
    * Method to input the formatted string of the "operator<<".
    *
    * Expected format:  ( ( rspfGpt ), ( rspfDpt ), 0.50000000000000 )
    *                     --*this---- , ----tie-----, ---score--------
    */
   friend RSPFDLLEXPORT std::istream& operator>>(std::istream& is,
                                                  rspfPointObservation& pt);

protected:
   rspfGpt thePt;
   rspfString theID;
   double theObsSigmas[3];
   double theScore;

   std::vector<rspfDpt> theMeas;
   std::vector<rspfDpt> theMeasSigmas;
   std::vector<rspfFilename> theImageFiles;
   std::vector<int> theNumPars;
   std::vector< rspfRefPtr<rspfImageHandler> > theImageHandlers;
};

#endif // #ifndef rspfPointObservation_HEADER
