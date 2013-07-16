#ifndef rspfSquareFunction_HEADER
#define rspfSquareFunction_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include "rspfRegistrationExports.h"

/**
 * rspfSquareFunction : filter to raise to power of 2
 */
class RSPF_REGISTRATION_DLL rspfSquareFunction : public rspfImageSourceFilter
{
public:
   rspfSquareFunction(rspfObject* owner=NULL);
   virtual ~rspfSquareFunction();

   virtual rspfString getShortName()const{return rspfString("SquareFunction");}
   virtual rspfString getLongName()const{return rspfString("raises to power of two");}

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   virtual void initialize();

   virtual double getNullPixelValue(rspf_uint32 band=0) const;
   virtual double getMinPixelValue(rspf_uint32 band=0)  const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)  const;

protected:

   template<class T>
   void applyFunction(T,
                      rspfRefPtr<rspfImageData> inputData,
                      rspfRefPtr<rspfImageData> outputData);

   void allocate();
   void clearNullMinMax();
   void computeNullMinMax();
   
   vector<double> theNullPixValue;
   vector<double> theMinPixValue;
   vector<double> theMaxPixValue;   
   
   rspfRefPtr<rspfImageData> theTile;                       

TYPE_DATA
};

#endif /* #ifndef rspfSquareFunction_HEADER */
