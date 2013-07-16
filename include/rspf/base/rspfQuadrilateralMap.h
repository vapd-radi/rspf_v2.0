#ifndef rspfQuadrilateralMap_HEADER
#define rspfQuadrilateralMap_HEADER
#include <rspf/base/rspfDpt.h>

class RSPFDLLEXPORT rspfQuadrilateralMap
{
public:
   rspfQuadrilateralMap(const rspfDpt& sourceP1,
                         const rspfDpt& sourceP2,
                         const rspfDpt& sourceP3,
                         const rspfDpt& sourceP4,
                         const rspfDpt& destP1,
                         const rspfDpt& destP2,
                         const rspfDpt& destP3,
                         const rspfDpt& destP4);
   rspfQuadrilateralMap();

  rspfQuadrilateralMap(const rspfQuadrilateralMap& rhs);
   /*!
    * Uses a standard form of quadrilateral to quadrilateral
    * mapping:
    *
    * X = (a*U + b*V + c) / (g*U + h*V + 1)
    * Y = (d*U + e*V+f)/(g*U + h*V + 1)
    *
    * where:
    * X and Y are the mapped destination point.
    * U and V are the source point to warp
    * a, b, c, d, e, f, g, h are coefficients sovled by
    * the points passed to the constructor.
    */
   void map(const rspfDpt& source, rspfDpt& dest)const;

   void solveCoefficients(const rspfDpt& sourceP1,
                          const rspfDpt& sourceP2,
                          const rspfDpt& sourceP3,
                          const rspfDpt& sourceP4,
                          const rspfDpt& destP1,
                          const rspfDpt& destP2,
                          const rspfDpt& destP3,
                          const rspfDpt& destP4);

protected:
   double a;
   double b;
   double c;
   double d;
   double e;
   double f;
   double g;
   double h;
   
};

#endif
