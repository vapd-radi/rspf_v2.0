//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Helper interface class for rspfAdjustmentExecutive
//              and rspfWLSBundleSolution.
//----------------------------------------------------------------------------
#ifndef rspfAdjSolutionAttributes_HEADER
#define rspfAdjSolutionAttributes_HEADER

#include <rspf/base/rspfString.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/matrix/newmatio.h>
#include <iostream>
#include <map>
#include <cmath>


typedef std::multimap<int, int> ObjImgMap_t;
typedef ObjImgMap_t::iterator ObjImgMapIter_t;
typedef std::map<int, int> ImgNumparMap_t;
typedef ImgNumparMap_t::iterator ImgNumparMapIter_t;
typedef std::pair<ObjImgMapIter_t, ObjImgMapIter_t> ObjImgMapIterPair_t;


class rspfAdjSolutionAttributes
{
public:
   rspfAdjSolutionAttributes
         (const int& numObjObs, const int& numImages, const int& numMeas, const int& rank);

   ~rspfAdjSolutionAttributes();

   // Access traits
   inline int numObjObs() const { return theNumObjObs; }
   inline int numImages() const { return theNumImages; }
   inline int fullRank()  const { return theFullRank; }


   friend class rspfWLSBundleSolution;
   friend class rspfAdjustmentExecutive;


protected:
   // Traits
   int theNumObjObs;
   int theNumImages;
   int theFullRank;
   int theNumMeasurements;

   // Stacked observation evaluation matrices
   NEWMAT::Matrix theMeasResiduals;          // theNumMeasurements X 2
   NEWMAT::Matrix theObjPartials;            // theNumObjObs*3 X 2
   NEWMAT::Matrix theParPartials;            // theNumImages*(npar/image) X 2

   // Stacked a priori covariance matrices
   NEWMAT::Matrix theImagePtCov;             // theNumMeasurements*2 X 2
   NEWMAT::Matrix theObjectPtCov;            // theNumObjObs*3 X 3

   // Full parameter covariance matrix
   //  TODO....  This is not stacked because npar/image may vary.  However, it's
   //            not treated as a full matrix in the solution due to
   //            current use of simple partitioning, assuming no correlation.
   NEWMAT::Matrix theAdjParCov;              // theNumImages*(npar/image) X theNumImages*(npar/image)

   // Correction vectors
   NEWMAT::ColumnVector theLastCorrections;  // theFullRank X 1
   NEWMAT::ColumnVector theTotalCorrections; // theFullRank X 1

   // A posteriori full covariance matrix
   NEWMAT::UpperTriangularMatrix theFullCovMatrix;

   // Map obj vs. images (measurements)
   ObjImgMap_t theObjImgXref;

   // Map images vs. number of adj parameters
   ImgNumparMap_t theImgNumparXref;

   // Output operator
   friend std::ostream& operator << (std::ostream&, rspfAdjSolutionAttributes&);

};
#endif // rspfAdjSolutionAttributes_HEADER

