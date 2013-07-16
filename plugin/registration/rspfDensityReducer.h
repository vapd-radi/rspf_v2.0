#ifndef rspfDensityReducer_HEADER
#define rspfDensityReducer_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include "rspfRegistrationExports.h"

/**
 * class rspfDensityReducer
 *
 * orders NON NULL elements in tile
 * and nullify weaker ones so that a maximum density of non NULL pixel is reached
 * MaxDensity is a real number, which should between 0 and 1 (unit: pixel^-2)
 * 
 * TBD:
 * -so far all bands are processed in parallel (slow) : define multi-band density reduction
 *   (I only use it for single band data so far)
 * -dealing with NULL values
 * -random pick for too small density values (to avoid no corners at all)
 */
class RSPF_REGISTRATION_DLL rspfDensityReducer : public rspfImageSourceFilter
{
public:
   /**
    * own methods
    */
   rspfDensityReducer(rspfObject* owner=NULL);
   rspfDensityReducer(rspfImageSource* inputSource);
   rspfDensityReducer(rspfObject* owner, rspfImageSource* inputSource);
  
   inline const rspf_float64& getMaxDensity()const { return theMaxDensity; }
   inline void                 setMaxDensity(const rspf_float64& aMaxDensity) { theMaxDensity = aMaxDensity; }

   /**
    * inherited methods
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
   rspf_float64                theMaxDensity; //! between 0 and 1 included
   
   template <class T>
   void runFilter(T dummyVariable, rspfRefPtr<rspfImageData> inputData);
   
TYPE_DATA
};


#endif
