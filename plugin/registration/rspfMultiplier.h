// class rspfMultiplier : Image Combiner

#ifndef rspfMultiplier_HEADER
#define rspfMultiplier_HEADER

#include <rspf/imaging/rspfImageCombiner.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfTDpt.h>
#include <vector>
#include "rspfRegistrationExports.h"
/**
 * class rspfMultiplier : Image Combiner
 * product of input images, of same scalar type
 *
 * restrict to minimum number of bands of inputs
 * no NODATA values used
 * TODO : any number of inputs
 *        support any operation : + / -
 */
class RSPF_REGISTRATION_DLL rspfMultiplier : public rspfImageCombiner
{
public:
   //own public methods
   rspfMultiplier();
   rspfMultiplier(rspfObject *owner,         
                   rspfImageSource *left,
                   rspfImageSource *right);
   virtual ~rspfMultiplier();   
   
   //inherited public methods
   virtual void                        initialize();
   virtual bool                        canConnectMyInputTo(rspf_int32 index,const rspfConnectableObject* object)const;
   virtual double                      getNullPixelValue(rspf_uint32 band)const;
   virtual double                      getMinPixelValue(rspf_uint32 band=0)const;
   virtual double                      getMaxPixelValue(rspf_uint32 band=0)const;

   virtual rspf_uint32                getNumberOfOutputBands()const;
   virtual rspfIrect                  getBoundingRect(rspf_uint32 resLevel=0)const;

   virtual rspfRefPtr<rspfImageData> getTile (const rspfIrect &rect, rspf_uint32 resLevel=0);   

protected:
   std::vector<double> theMin;
   std::vector<double> theMax;

   void computeMinMax();
   void switchTypeRun(const rspfIrect &rect, rspf_uint32 resLevel);
   template<class T> void operate( T, const rspfIrect& tileRect, rspf_uint32 resLevel);

TYPE_DATA
};
#endif
