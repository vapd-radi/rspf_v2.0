// rspfImageCorrelator
// class for getting tie-points from a master/slave image pair
//
// TODO : generate one file only : XML or Tabulated Text
// TODO : change TieGPtSet to a generic TiePtSet
// TODO : increase speed

#ifndef rspfImageCorrelator_HEADER
#define rspfImageCorrelator_HEADER

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfOutputSource.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfBandSelector.h>
#include <rspf/imaging/rspfImageRenderer.h>
#include <rspf/imaging/rspfCastTileSourceFilter.h>
#include <rspf/base/rspfTDpt.h>
#include <rspf/base/rspfTieGptSet.h>
#include "rspfRegistrationExports.h"
#include "rspfHarrisCorners.h"
#include "rspfTieGenerator.h"
#include "rspfChipMatch.h"
#include <vector>

class rspfImageGeometry;
class rspfMapProjection;
class rspfListenerManager;

class RSPF_REGISTRATION_DLL rspfImageCorrelator :
    public rspfOutputSource,
    public rspfProcessInterface
{
public:
   rspfImageCorrelator();
   virtual ~rspfImageCorrelator();

   //accessors to parms
   inline void               setMaster(const rspfFilename& m) { theMaster=m; }
   inline const rspfFilename& getMaster()const { return theMaster; }
   inline void               setSlave(const rspfFilename& s) { theSlave=s; }
   inline const rspfFilename& getSlave()const { return theSlave; }
   inline void               setMasterBand(rspf_uint32 b) { theMasterBand=b; }
   inline rspf_uint32       getMasterBand()const { return theMasterBand; }
   inline void               setSlaveBand(rspf_uint32 b) { theSlaveBand=b; }
   inline rspf_uint32       getSlaveBand()const { return theSlaveBand; }
   inline void               setScaleRatio(const rspf_float64& r) { theScaleRatio=r; }
   inline rspf_float64      getScaleRatio()const { return theScaleRatio; }
   inline void               setSlaveAccuracy(const rspf_float64& a) { theSlaveAccuracy=a; }
   inline rspf_float64      getSlaveAccuracy()const { return theSlaveAccuracy; }
   inline void               setProjectionType(const rspfString& p) { theProjectionType=p; }
   inline const rspfString& getProjectionType()const { return theProjectionType; }
   inline void               setCornerDensity(const rspf_float64& a) { theCornerDensity=a; }
   inline rspf_float64      getCornerDensity()const { return theCornerDensity; }
   inline void               setMasterPointProj(const rspfString& p) { theMasterPointProj=p; }
   inline const rspfString& getMasterPointProj()const { return theMasterPointProj; }
   inline void               setSlavePointProj(const rspfString& p) { theSlavePointProj=p; }
   inline const rspfString& getSlavePointProj()const { return theSlavePointProj; }
   inline void               setTemplateRadius(const rspf_uint32& r) { theTemplateRadius = r; }
   inline rspf_uint32       getTemplateRadius()const { return theTemplateRadius; }
   inline void               setMinCorrel(const rspf_float64& c) { theMinCorrel=c; }
   inline rspf_float64      getMinCorrel()const { return theMinCorrel; }   
   
   inline bool hasRun()const { return theHasRun; }

   // inherited methods
   virtual bool isOpen() const;
   virtual bool open();
   virtual void close();

   virtual bool  execute(); //also creates tie point file
   virtual       rspfObject* getObject()      { return this; }
   virtual const rspfObject* getObject()const { return this; }
   virtual       rspfObject* getObjectInterface() { return this; }

   virtual rspfListenerManager* getListenerManagerInterface() { return this; }
   virtual bool addListener(rspfListener *listener);
   virtual bool removeListener(rspfListener *listener);

   virtual bool canConnectMyInputTo(rspf_int32 inputIndex,const rspfConnectableObject* object)const { return false; } //TBC : so far no input

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

protected:  

   rspfString         getRole() const;
   rspfImageHandler*  getProjectionHandler();
   
   rspfRefPtr<rspfImageGeometry> getOutputImageGeometry();

   rspfMapProjection* getOutputProjection();
   
   bool buildRenderer(
      rspfImageChain* chain,
//         rspfImageSource* source, 
         rspfMapProjection* outProjection, 
         rspfImageRenderer* renderer,
         const rspfFilterResampler::rspfFilterResamplerType& stype =  rspfFilterResampler::rspfFilterResampler_CUBIC
         )const;

   void computeDispStats(const vector<rspfTDpt>& tp)const;
   void rejectOutliersMedian(const vector<rspfTDpt>& tp, vector<rspfTDpt>& ftp, const rspf_float64& relError);
   void getMedianInlier1D(const vector<double>& sd, const rspf_float64& relError, rspf_float64& median, rspf_uint32& inlierCount);

   rspfFilename   theMaster;
   rspfFilename   theSlave;
   rspf_uint32  theMasterBand;
   rspf_uint32  theSlaveBand;
   rspf_float64 theScaleRatio;
   rspf_float64 theSlaveAccuracy;
   rspf_float64 theCornerDensity;
   rspfString   theProjectionType;
   rspfString   theMasterPointProj;
   rspfString   theSlavePointProj;
   rspf_uint32  theTemplateRadius; //in pixels
   rspf_float64 theMinCorrel;

   bool theHasRun; //to know whether execute has been run

   rspfRefPtr<rspfImageChain> theMChain;
   rspfRefPtr<rspfImageChain> theSChain;
   
   rspfRefPtr<rspfImageHandler>   handlerM;
   rspfRefPtr<rspfImageHandler>   handlerS;
   rspfRefPtr<rspfBandSelector>   theMasterBandSelector;
   rspfRefPtr<rspfBandSelector>   theSlaveBandSelector;
   rspfRefPtr<rspfImageRenderer>  rendererM;
   rspfRefPtr<rspfImageRenderer>  rendererS;
   rspfRefPtr<rspfHarrisCorners>  cornerDetector;
   std::vector<rspfRefPtr<rspfCastTileSourceFilter> > caster;
   rspfRefPtr<rspfChipMatch>      matcher;
   rspfRefPtr<rspfTieGenerator>   generator;
   rspfTieGptSet       theTset;

   //! Disallow operator=
   const rspfImageCorrelator& operator=(const rspfImageCorrelator& rhs) {return rhs;}

TYPE_DATA
};

#endif //rspfImageCorrelator_HEADER
