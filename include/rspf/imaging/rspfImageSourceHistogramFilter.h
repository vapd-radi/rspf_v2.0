//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageSourceHistogramFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfImageSourceHistogramFilter_HEADER
#define rspfImageSourceHistogramFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>

class RSPFDLLEXPORT rspfImageSourceHistogramFilter : public rspfImageSourceFilter
{
public:
   rspfImageSourceHistogramFilter();
   rspfImageSourceHistogramFilter(rspfImageSource* inputSource,
                        rspfRefPtr<rspfMultiResLevelHistogram> histogram);
   virtual void setHistogram(rspfRefPtr<rspfMultiResLevelHistogram> histogram);
   virtual bool setHistogram(const rspfFilename& filename);
   virtual const rspfFilename& getHistogramFilename()const;
   virtual rspfRefPtr<rspfMultiResLevelHistogram> getHistogram();
   virtual const rspfRefPtr<rspfMultiResLevelHistogram> getHistogram()const;
   
   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object)const;

   virtual void connectInputEvent(rspfConnectionEvent& event);
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = NULL);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = NULL)const;

   virtual std::ostream& print(std::ostream& out) const;

protected:
   virtual ~rspfImageSourceHistogramFilter();
   rspf_int32                         theCurrentResLevel;

private:
   rspfRefPtr<rspfMultiResLevelHistogram> theHistogram;
   rspfFilename                theFilename;
   
TYPE_DATA
};

#endif /* #ifndef rspfImageSourceHistogramFilter_HEADER */
