
#ifndef rspfExtremaFilter_HEADER
#define rspfExtremaFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include "rspfRegistrationExports.h"

/**
 * class rspfExtremaFilter
 *
 * returns local extrema unchanged (can be maxima or minima, strict or not)
 * nullify other pixels
 * uses a 8 pixel neighborhood
 *
 * ToBeDone:
 *  -add vector tiles support
 *  -add different kind of neighborhoods  - mostly larger
 */
class RSPF_REGISTRATION_DLL rspfExtremaFilter : public rspfImageSourceFilter
{
public:
  /**
   * own methods
   */
   rspfExtremaFilter(rspfObject* owner=NULL);
   rspfExtremaFilter(rspfImageSource* inputSource);
   rspfExtremaFilter(rspfObject* owner, rspfImageSource* inputSource);

   inline bool isMaximum()const { return theIsMaximum; }
   inline void setIsMaximum(bool aMax) { theIsMaximum = aMax; }
   
   inline bool isStrict()const { return theIsStrict; }
   inline void setIsStrict(bool aStrict) { theIsStrict = aStrict; }

  /**
   *inherited methods
   */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool saveState(rspfKeywordlist& kwl,const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,const char* prefix=0);
   
protected:
   rspfRefPtr<rspfImageData> theTile;
   bool                        theIsMaximum;
   bool                        theIsStrict;
   
   void adjustRequestRect(rspfIrect& requestRect)const;
   
   template <class T>
   void runFilter(T dummyVariable,
                  rspfRefPtr<rspfImageData> inputData);
   
TYPE_DATA
};

#endif
