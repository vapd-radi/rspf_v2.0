//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfMultiBandHistogramTileSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfMultiBandHistogramTileSource_HEADER
#define rspfMultiBandHistogramTileSource_HEADER
#include <rspf/imaging/rspfImageSourceHistogramFilter.h>
#include <vector>
class RSPFDLLEXPORT rspfMultiBandHistogramTileSource : public rspfImageSourceHistogramFilter
{
public:
   enum rspfMultiBandHistogramAlgorithm
   {
      RSPF_HISTOGRAM_NO_ALGORITHM             = 0,
      RSPF_HISTOGRAM_LINEAR_STRETCH_ALGORITHM = 1
   };
   rspfMultiBandHistogramTileSource();
   rspfMultiBandHistogramTileSource(double minValuePercent,
                                  double maxValuePercent,
                                  rspfImageSource* inputSource,
                                  rspfMultiResLevelHistogram* histogram);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   virtual void setMinValuePercent(const std::vector<double>& arrayPercents)
      {
         theMinValuePercentArray = arrayPercents;
      }
   virtual void setMaxValuePercent(const std::vector<double>& arrayPercents)
      {
         theMaxValuePercentArray = arrayPercents;
      }
   virtual void setMinValuePercent(rspf_uint32 arrayIndex,
                                   double minValue)
      {
         if(arrayIndex < theMinValuePercentArray.size())
         {
            theMinValuePercentArray[arrayIndex] = minValue;
         }
      }
   virtual void setMaxValuePercent(rspf_uint32 arrayIndex,
                                   double maxValue)
      {
         if(arrayIndex < theMaxValuePercentArray.size())
         {
            theMaxValuePercentArray[arrayIndex] = maxValue;
         }
      }
   virtual double getMinValuePercent(rspf_uint32 arrayIndex)const
      {
         if(arrayIndex < theMinValuePercentArray.size())
         {
            return theMinValuePercentArray[arrayIndex];
         }
         return rspf::nan();
      }
   virtual double getMaxValuePercent(rspf_uint32 arrayIndex)const
      {
         if(arrayIndex < theMaxValuePercentArray.size())
         {
            return theMaxValuePercentArray[arrayIndex];
         }
         return rspf::nan();
      }
   virtual rspf_uint32 getNumberOfMinValues()const
      {
         return (rspf_uint32)theMinValuePercentArray.size();
      }
   virtual rspf_uint32 getNumberOfMaxValues()const
      {
         return (rspf_uint32)theMinValuePercentArray.size();
      }
   virtual void initialize();
      
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
protected:
   virtual ~rspfMultiBandHistogramTileSource();

   void allocate();
   
   rspfRefPtr<rspfImageData>      theTile;
   rspfMultiBandHistogramAlgorithm theAlgorithm;
   std::vector<double>              theMinValuePercentArray;
   std::vector<double>              theMaxValuePercentArray;
   
   template <class T> rspfRefPtr<rspfImageData> runLinearStretchAlgorithm(
      T dummyVariable,
      rspfRefPtr<rspfImageData>& tile);

TYPE_DATA
};
#endif
