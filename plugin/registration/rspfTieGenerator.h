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

#ifndef rspfTieGenerator_HEADER
#define rspfTieGenerator_HEADER

#include <fstream>

#include <rspf/base/rspfOutputSource.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfTDpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageSource.h>
#include "rspfChipMatch.h"
#include "rspfRegistrationExports.h"
class rspfImageSource;

//! Class rspfTieGenerator

/*!
 *  get all tie points from rspfChipMatch using a tiled vector method : getFeatures()
 */
class RSPF_REGISTRATION_DLL rspfTieGenerator : 
    public rspfOutputSource,
    public rspfProcessInterface
{
public:
   rspfTieGenerator(rspfImageSource* inputSource=NULL);
   virtual ~rspfTieGenerator();

   inline bool getStoreFlag()const   { return theStoreFlag; }
   inline void setStoreFlag(bool sf) { theStoreFlag = sf; }

   virtual       rspfObject* getObject()      { return this; }
   virtual const rspfObject* getObject()const { return this; }
   virtual       rspfObject* getObjectInterface() { return this; }

   void setAreaOfInterest(const rspfIrect& rect);
   virtual void setOutputName(const rspfString& filename);
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
         return (object&& PTR_CAST(rspfChipMatch, object));
      }

   const vector<rspfTDpt>& getTiePoints() { return theTiePoints; }

protected:
   bool getAllFeatures();
   void writeTiePoints(const vector<rspfTDpt>& tp);

private:
   rspfIrect        theAreaOfInterest;
   rspfFilename     theFilename;
   std::ofstream     theFileStream;
   vector<rspfTDpt> theTiePoints;
   bool              theStoreFlag;

   //! Disallow copy constructor and operator=
   rspfTieGenerator(const rspfTieGenerator&) {}
   const rspfTieGenerator& operator=(const rspfTieGenerator& rhs)
      {return rhs;}

TYPE_DATA
};

#endif
