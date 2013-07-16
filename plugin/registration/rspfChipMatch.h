// class rspfChipMatch : Image Combiner
//
// INPUTS:
// - input image (slave) : one band
// - reference image (master) : one band
// - feature centers map
//    = image of centers of chips (center= non NULL, > 2.0, pixels), e.g. harris corner map
//   that image should be co-registered with the master image
// OPERATION:
//  matches master chips to slave chips and stores score + displacement.
//  uses normalized cross correlation
// PARAMETERS:
//  accuracy of slave positions (meters)
//  chip window size(pixels)
// OUTPUT:
//  normalized floating point image (0.0<->1.0) with chip matching intensity
//
// FUTURE:
//  handle NULL pixels
//  add matching on boundaries 
//  use vector features for matching positions
//  use vector tiles in Ossim for parallelism
//
// created by Frederic Claudel, EORU - CSIR - Aug 2005

#ifndef rspfChipMatch_HEADER
#define rspfChipMatch_HEADER

#include <rspf/imaging/rspfImageCombiner.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfTDpt.h>
#include <vector>
#include "rspfRegistrationExports.h"

#define RSPF_CHIPMATCH_SLAVEACCURACY_PROPNAME "SlaveAccuracy"
#define RSPF_CHIPMATCH_PIXELRADIUS_PROPNAME "PixelRadius"
#define RSPF_CHIPMATCH_MINNCC_PROPNAME "MinimumNCC"

class rspfNCC_FFTW;

class RSPF_REGISTRATION_DLL rspfChipMatch : public rspfImageCombiner
{
public:
   //own public methods
   rspfChipMatch();
   rspfChipMatch(rspfObject *owner,         
                               rspfImageSource *centers,
                               rspfImageSource *master, 
                               rspfImageSource *slave   );
   virtual ~rspfChipMatch();

   inline void setSlaveAccuracy(rspf_float64 acc) { theSlaveAccuracy=acc; } //unit: meters
   inline rspf_float64 getSlaveAccuracy()const { return theSlaveAccuracy; }

   inline void setMasterRadius(rspf_uint32 r) { theMRadius=r; } //unit : pixels
   inline rspf_uint32 getMasterRadius()const { return theMRadius; }

   inline void setBias(const rspfDpt& aBias) { theBias=aBias; } //using current projection unit & axes
   inline const rspfDpt& getBias()const { return theBias; }

   inline void setMinNCC(rspf_float64 m) { theMinNCC=m; } //unitless (between -1.0 and 1.0)
   inline rspf_float64 getMinNCC()const { return theMinNCC; }
   
   virtual const std::vector<rspfTDpt>& getFeatures(const rspfIrect &rect, rspf_uint32 resLevel=0); //vector method for getTile
   
   //inherited public methods
   virtual void                        initialize();
   virtual bool                        canConnectMyInputTo(rspf_int32 index,const rspfConnectableObject* object)const;
   virtual double                      getNullPixelValue(rspf_uint32 band)const;
   virtual double                      getMinPixelValue(rspf_uint32 band=0)const;
   virtual double                      getMaxPixelValue(rspf_uint32 band=0)const;
   virtual rspfScalarType 	         getOutputScalarType()const;
   virtual rspf_uint32                getNumberOfOutputBands()const;
   virtual rspfIrect                  getBoundingRect(rspf_uint32 resLevel=0)const;

   virtual rspf_uint32 getTileWidth() const;
   virtual rspf_uint32 getTileHeight() const;

   virtual void getDecimationFactor(rspf_uint32 resLevel, rspfDpt& result)const;
   virtual void getDecimationFactors(vector<rspfDpt>& decimations)const;
   virtual rspf_uint32 getNumberOfDecimationLevels()const;

   virtual rspfRefPtr<rspfImageData> getTile (const rspfIrect &rect, rspf_uint32 resLevel=0);

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

protected:
   bool runMatch(const rspfIrect &rect, rspf_uint32 resLevel=0);
   void getMaxCorrelation(rspfRefPtr<rspfImageData> Mchip, rspfRefPtr<rspfImageData> Schip, 
                                  double* pdispx, double* pdispy, double* pcor);

   std::vector<rspfTDpt>      theTies;
   rspf_float64               theSlaveAccuracy;
   rspf_uint32                theMRadius;
   rspfDpt                    theBias;
   rspf_float64               theMinNCC;
   rspfNCC_FFTW*              theNCCengine;
   rspfRefPtr<rspfImageData> theTile;
   //static data
   static double theLMS[6*9];

TYPE_DATA
};
#endif
