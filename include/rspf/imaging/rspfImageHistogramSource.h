//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageHistogramSource.h 17194 2010-04-23 15:05:19Z dburken $
#ifndef rspfImageHistogramSource_HEADER
#define rspfImageHistogramSource_HEADER
#include <rspf/base/rspfHistogramSource.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfIrect.h>

/*!
 * This source expects as input an rspfImageSource.
 * it will slice up the requested region into tiles and compute
 * the histogram of the passed in rectangle.
 */
class RSPFDLLEXPORT rspfImageHistogramSource : public rspfHistogramSource,
                                                 public rspfConnectableObjectListener,
                                                 public rspfProcessInterface
{
public:

   rspfImageHistogramSource(rspfObject* owner = 0);
   
   virtual rspfObject* getObject();
   virtual const rspfObject* getObject()const;

   void setAreaOfInterest(const rspfIrect& rect);

   rspfIrect getAreaOfInterest()const;

   void getAreaOfInterest(rspfIrect& rect)const;

   virtual rspf_uint32 getMaxNumberOfRLevels()const;

   virtual void setMaxNumberOfRLevels(rspf_uint32 number);

   /*!
    * The first argument is the region of interest that you wish to
    * use for the histogram.  If the region is different than
    * what it has already computed before then the histogram is considered
    * dirty and will be re-computed.  The returned object is a
    * multi-reslevel histogram
    */
   virtual rspfRefPtr<rspfMultiResLevelHistogram> getHistogram(const rspfIrect& rect);

   virtual rspfRefPtr<rspfMultiResLevelHistogram> getHistogram();
   virtual bool execute();
   
   virtual bool canConnectMyInputTo(rspf_int32 myInputIndex,
                                    const rspfConnectableObject* object)const;

   void setNumberOfBinsOverride(rspf_int32 numberOfBinsOverride);

   void setMinValueOverride(rspf_float32 minValueOverride);

   void setMaxValueOverride(rspf_float32 maxValueOverride);

   rspfHistogramMode getComputationMode()const;
   void setComputationMode(rspfHistogramMode mode);
	
   virtual void propertyEvent(rspfPropertyEvent& event);
   
   virtual void connectInputEvent(rspfConnectionEvent& event);
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
	
protected:
   virtual ~rspfImageHistogramSource();

   void getBinInformation(rspf_uint32& numberOfBins,
                          rspf_float64& minValue,
                          rspf_float64& maxValue)const;
   virtual void computeNormalModeHistogram();
   virtual void computeFastModeHistogram();
   
   /*!
    * Initialized to rspfNAN'S
    */
   rspfIrect theAreaOfInterest;
   bool       theHistogramRecomputeFlag;
   
   /*!
    * Will default to 1.  This means it will use up to max
    * number of res levels for the histogram.
    */
   rspf_uint32       theMaxNumberOfResLevels; 
   rspf_float64      theMinValueOverride;
   rspf_float64      theMaxValueOverride;
   rspf_int32        theNumberOfBinsOverride;
   rspfHistogramMode theComputationMode;
   rspf_uint32       theNumberOfTilesToUseInFastMode;
TYPE_DATA
};

#endif
