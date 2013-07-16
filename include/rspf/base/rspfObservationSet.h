//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Storage class for observations.
//----------------------------------------------------------------------------
#ifndef rspfObservationSet_HEADER
#define rspfObservationSet_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfPointObservation.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <iostream>
#include <vector>

class RSPF_DLL rspfObservationSet : public rspfObject
{
public:
   rspfObservationSet();

   ~rspfObservationSet();
  
   inline rspf_uint32 numImages()const { return (rspf_uint32)theImageHandlers.size(); }
   inline rspf_uint32    numObs()const { return (rspf_uint32)theObs.size(); }
   inline rspf_uint32 numAdjPar()const { return (rspf_uint32)theNumAdjPar; }
   inline rspf_uint32   numMeas()const { return (rspf_uint32)theNumMeas; }

   // observation accessor
   inline rspfRefPtr<rspfPointObservation> observ(const int& i)const{ return theObs[i]; }

   // image accessor
   rspfImageGeometry* getImageGeom(const int index);
   void setImageGeom(const int index, rspfImageGeometry* geom);

   // image index accessor
   inline int imIndex(const int& i)const { return theImageIndex[i]; }

   // adjustable parameter count accessor
   inline int adjParCount(const int& i)const { return theNumAdjParams[i]; }

   // Image file access
   inline rspfFilename imageFile(const int index)const { return theImageFiles[index]; }

   /**
    * standard evaluation
    *   [1] measResiduals:  [x,y] residuals (numMeas X 2)
    *   [2] objPartials:    object point partial derivatives (numMeas*3 X 2)
    *   [3] parPartials:    image parameter partial derivatives (numParams X 2)
    */
   bool evaluate(NEWMAT::Matrix& measResiduals,
                 NEWMAT::Matrix& objPartials,
                 NEWMAT::Matrix& parPartials);

   /**
    * operations
    */
   bool addObservation(rspfRefPtr<rspfPointObservation> obs);

   /**
    * text output : header + tab separated tie points
    */
   std::ostream& print(std::ostream& os) const;

protected:
   int theNumAdjPar;
   int theNumMeas;
   int theNumPartials;

   // member observations
   std::vector< rspfRefPtr<rspfPointObservation> > theObs;

   // measurement -> image index
   std::vector<int> theImageIndex;

   // image files
   std::vector<rspfFilename> theImageFiles;

   // image adjustable parameter count
   std::vector<int> theNumAdjParams;

   std::vector< rspfRefPtr<rspfImageHandler> > theImageHandlers;

   // groups (TODO in future integration of correlated parameters)
   //   Note: Currently, each image is assumed to be independent, which can result
   //     in redundant parameters.  For example, images from a single flight line
   //     clearly share a common focal length and, most likely, a common position
   //     bias.  Implementation of this enhancement will require a mechanism for
   //     defining inter-image parametric relationships.
   //std::vector<????????*> theGroups;
};

#endif // #ifndef rspfObservationSet_HEADER
