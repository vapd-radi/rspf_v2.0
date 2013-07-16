#ifndef rspfBilinearProjection_HEADER
#define rspfBilinearProjection_HEADER
#include <vector>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/projection/rspfOptimizableProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfLeastSquaresBilin.h>
class RSPF_DLL rspfBilinearProjection : public rspfProjection,
                                          public rspfOptimizableProjection
     
{
public:
   rspfBilinearProjection();
   rspfBilinearProjection(const rspfBilinearProjection& rhs);
   rspfBilinearProjection(const rspfDpt& ul,
                           const rspfDpt& ur,
                           const rspfDpt& lr,
                           const rspfDpt& ll,
                           const rspfGpt& ulg,
                           const rspfGpt& urg,
                           const rspfGpt& lrg,
                           const rspfGpt& llg);
   virtual rspfObject *dup()const;
   virtual rspfGpt origin()const;
   
   virtual void worldToLineSample(const rspfGpt& worldPoint,
                                  rspfDpt&       lineSampPt) const;
   /*!
    * METHOD: lineSampleToWorld()
    * Performs the inverse projection from line, sample to ground (world):
    */
   virtual void lineSampleToWorld(const rspfDpt& lineSampPt,
                                  rspfGpt&       worldPt) const;
   
   /*!
    * METHOD: lineSampleHeightToWorld
    * This is the pure virtual that projects the image point to the given
    * elevation above ellipsoid, thereby bypassing reference to a DEM. Useful
    * for projections that are sensitive to elevation (such as sensor models).
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                        const double&   heightAboveEllipsoid,
                                        rspfGpt&       worldPt) const;
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual bool operator==(const rspfProjection& projection) const;
   virtual rspfDpt getMetersPerPixel() const;
   virtual std::ostream& print(std::ostream& out) const;
   
   /*!
    * METHOD: setTiePoints
    * add any number of tie points and calculate bilinear fit
    * returns the ground error variance (=RMS^2), in meters^2
    */
   virtual rspf_float64 setTiePoints(const std::vector<rspfDpt>& lsPt, 
                                      const std::vector<rspfGpt>& geoPt);
   
   /*
    * optimizable interface
    */
   virtual bool setupOptimizer(const rspfString& setup);
   
   inline virtual bool useForward()const {return false;}
   
   virtual rspf_uint32 degreesOfFreedom()const;
   virtual double optimizeFit(const rspfTieGptSet& tieSet, double* targetVariance=0);
   
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return false.
    */
   virtual bool isAffectedByElevation() const { return false; } 
   
   void getTiePoints(std::vector<rspfDpt>& lsPt, std::vector<rspfGpt>& geoPt) const;
protected:
   virtual ~rspfBilinearProjection();
   void initializeBilinear();
   /**
    * Checks theLineSamplePt for nans.
    * @return true if any point has a nan.
    * @note If theLineSamplePt size is 0 this returns false.
    */
   bool dPtsHaveNan() const;
   
   /**
    * Checks theGeographicPt for nans.
    * @return true if any point has a nan.
    * @note If theLineSamplePt size is 0 this returns false.
    */
   bool gPtsHaveNan() const;
   /**
    * @return The mid point of theLineSamplePts.
    */
   rspfDpt midLineSamplePt() const;
   
   std::vector<rspfDpt>  theLineSamplePt;
   std::vector<rspfGpt>  theGeographicPt;
   rspfLeastSquaresBilin theLatFit;
   rspfLeastSquaresBilin theLonFit;
   rspfLeastSquaresBilin theXFit;
   rspfLeastSquaresBilin theYFit;
   bool theInterpolationPointsHaveNanFlag;
   bool theInverseSupportedFlag;
TYPE_DATA   
};
#endif /* #ifndef rspfBilinearProjection_HEADER */
