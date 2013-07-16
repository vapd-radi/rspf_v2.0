//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
//----------------------------------------------------------------------------
// $Id: rspfFilterTable.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfFilterTable_HEADER
#define rspfFilterTable_HEADER

#include <cmath>
#include <rspf/base/rspfConstants.h>

class rspfFilter;

class rspfFilterTable
{
public:
   /** default constructor */
   rspfFilterTable();

   /** destructor */
   ~rspfFilterTable();

   /** Builds a table with filter being used in both x and y direction. */
   void buildTable(rspf_uint32  filterSteps,
                   const rspfFilter& filter);

   /** Builds a table. */
   void buildTable(rspf_uint32  filterSteps,
                   const rspfFilter& xFilter,
                   const rspfFilter& yFilter);

   /** @return theWidth*theHeight */
   rspf_uint32 getWidthByHeight() const;

   /** @return theXSupport */
   rspf_uint32 getXSupport()      const;

   /** @return theYSupport */
   rspf_uint32 getYSupport()      const;

   /** @return theWidth */
   rspf_uint32 getWidth()         const;

   /** @return theHeight */
   rspf_uint32 getHeight()        const;

   /**
    * Inlined below.
    *
    * @return const double* to the closest weight of x and y.
    */
   const double* getClosestWeights(const double& x, const double& y)const;

protected:

   /**
    * Creates the weight array "theWeights".  Will delete if previously
    * allocated.
    */
   void allocateWeights();

   double*      theWeights;
   rspf_uint32 theWidth;
   rspf_uint32 theHeight;
   rspf_uint32 theWidthHeight;
   rspf_uint32 theXSupport;
   rspf_uint32 theYSupport;
   rspf_uint32 theFilterSteps;
};

/**
 * Method inlined for speed as it is used in the inner pixel loop of
 * rspfFilterResampler::resampleBilinearTile method.
 */
inline const double* rspfFilterTable::getClosestWeights(const double& x,
                                                         const double& y)const
{
   // Extract the decimal part of x and y.
   double intPartDummy;
   double decimalPrecisionX = fabs(modf(x, &intPartDummy));
   double decimalPrecisionY = fabs(modf(y, &intPartDummy));
   
   rspf_int32 kernelLine =
      (rspf_int32)(theFilterSteps*decimalPrecisionY);
   rspf_int32 kernelSamp =
      (rspf_int32)(theFilterSteps*decimalPrecisionX);
  
  return &theWeights[(kernelLine*theFilterSteps +
                      kernelSamp)*theWidthHeight];
}

#endif /* End of "#ifndef rspfFilterTable_HEADER" */
