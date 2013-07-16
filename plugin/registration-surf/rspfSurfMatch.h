// class TieGenerator
//
// features:
// execution gets all tie points using a rspfChipMatch object, given
// -a specific resoultionLevel
// -a specific rectangular ROI (default : everything in bounds)
// => it is a global operator
//
// can export tie points to a file (optional)
// tie points are provided by a rspfChipMatch object, using tiled method getFeatures()
//
// created by Frederic Claudel, CSIR - Aug 2005 - using rspfVertexExtractor as a model
//
// TBD: parallelize : use a sequencer (for vector data as well)
//

#ifndef rspfSurfMatch_HEADER
#define rspfSurfMatch_HEADER

#include <fstream>

#include <rspf/base/rspfOutputSource.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfTDpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageSource.h>
#include "rspfMatchExports.h"
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageSourceSequencer.h>


#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/legacy/compat.hpp"
#include "surf_match.h"

#include "RANSAC/ransac/ransac.h"
#include "RANSAC/estimators/Solver.h"
#include "RANSAC/estimators/affineSolver.h"
#include "RANSAC/estimators/affineError.h"
#include <armadillo>
using namespace groupsac;

class rspfImageSource;

//! Class rspfTieGenerator

/*!
 *  get all tie points from rspfChipMatch using a tiled vector method : getFeatures()
 */
class RSPF_REGISTRATION_DLL rspfSurfMatch : 
    public rspfOutputSource,
    public rspfProcessInterface
{
public:
   rspfSurfMatch(rspfImageSource* inputSource=NULL);
   virtual ~rspfSurfMatch();

   inline bool getStoreFlag()const   { return theStoreFlag; }
   inline void setStoreFlag(bool sf) { theStoreFlag = sf; }

   virtual       rspfObject* getObject()      { return this; }
   virtual const rspfObject* getObject()const { return this; }
   virtual       rspfObject* getObjectInterface() { return this; }


   void setAreaOfInterest(const rspfIrect& rect);
   virtual bool isOpen() const;
   virtual bool open();
   virtual void close();

   virtual bool execute();

   virtual rspfListenerManager* getListenerManagerInterface() { return this; }
   
/*   virtual void setPercentComplete(double percentComplete)
      {
         rspfProcessInterface::setPercentComplete(percentComplete);
         rspfProcessProgressEvent event(this,
                                         percentComplete);
         fireEvent(event);      
      }
*/
   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object)const
      {         
         return object;
      }

   const vector<rspfTDpt>& getTiePoints() { return theTiePoints; }

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

protected:
   double theHessianThreshold;
   rspfString theMasterFilename;
   rspfString theSlaverFilename;
   rspfString theOutputFilename;
   int theMasterBand;
   int theSlaverBand;

   rspfRefPtr<rspfImageSource> theMasterSource;
   rspfRefPtr<rspfImageSource> theSlaverSource;

   bool ExtractTile(rspfImageSource* imageSource,
	   const rspfIrect& tileRect,
	   int iband = 0,
	   rspf_uint32 resLevel = 0,
	   CvSeq** Keypoints = NULL,
	   CvSeq** Descriptors = NULL);
   bool getMasterFeatures(CvSeq** Keypoints, CvSeq** Descriptors);
   bool getSlaverFeatures(CvSeq** Keypoints, CvSeq** Descriptors);

   void addElementToList(CvSeq** root, CvSeq *elem);

private:
   rspfIrect        theAreaOfInterest;
   vector<rspfTDpt> theTiePoints;
   std::ofstream     theFileStream;
   bool              theStoreFlag;

   //! Disallow copy constructor and operator=
   rspfSurfMatch(const rspfSurfMatch&) {}
   const rspfSurfMatch& operator=(const rspfSurfMatch& rhs)
      {return rhs;}

TYPE_DATA
};

#endif
