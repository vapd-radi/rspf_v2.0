#ifndef rspfImageStatistics_HEADER
#define rspfImageStatistics_HEADER
#include <rspf/base/rspfSource.h>

class RSPFDLLEXPORT rspfImageStatisticsSource : public rspfSource
{
public:
   rspfImageStatisticsSource();
   
   
   virtual void computeStatistics();
   
   virtual bool canConnectMyInputTo(rspf_int32 inputIndex,
                                    const rspfConnectableObject* object)const;
   
   const std::vector<rspf_float64>& getMean()const;
   const std::vector<rspf_float64>& getMin()const;
   const std::vector<rspf_float64>& getMax()const;
   
protected:
   virtual ~rspfImageStatisticsSource();
   void clearStatistics();
   template <class T>
   void computeStatisticsTemplate(T dummyVariable);
   void setStatsSize(rspf_uint32 size);
   
   std::vector<rspf_float64> theMean;
   std::vector<rspf_float64> theMin;
   std::vector<rspf_float64> theMax;
};

#endif
