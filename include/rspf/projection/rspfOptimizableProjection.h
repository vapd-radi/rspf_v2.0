#ifndef rspfOptimizableProjection_HEADER
#define rspfOptimizableProjection_HEADER
#include <rspf/projection/rspfProjection.h>
class rspfTieGptSet;
/**
 * virtual class enabling projection optimization
 * (can be used for outlier rejection - RANSAC)
 */
class RSPFDLLEXPORT rspfOptimizableProjection
{
public:
   /*!
    * Construction and init
    */
   rspfOptimizableProjection();
   
   rspfOptimizableProjection(const rspfOptimizableProjection& source);
   
   virtual ~rspfOptimizableProjection();
   
   virtual rspfOptimizableProjection& operator=(
      const rspfOptimizableProjection& source);
   
   /*!
    * @brief setupFromString()  Derived classes should implement as needed.
    * Initialize parameters needed for optimizeFit and degreesOfFreedom
    * 
    * @return This implementation does nothing and returns false.
    */
   virtual bool setupOptimizer(const rspfString& setup);
   /*!
    * useForward()
    * return true when it's better (more accurate / fast) to use forward
    * (from ground to image) than inverse(from image to ground)
    */
   virtual bool useForward()const=0;
   /*!
    * degreesOfFreedom()
    * number of degrees of freedom (eg: 8 for bilinear, 6 for affine,
    * 2 for translation)
    * especially useful when not inheriting from AdjustableParameterInterface
    */
   virtual rspf_uint32 degreesOfFreedom()const=0;
   /*!
    * @brief needsInitialState()
    * 
    * @return This implementation returns false.  Sub classes should true if
    * the projection needs to be initialized to be optimized,
    * eg: sensor models.  
    */
   virtual bool needsInitialState()const;
   /*!
    * optimizeFit()
    * adjusts projection to GLOBALLY fit best to ALL tie points
    * (no outlier rejection)
    * can select an optional threshold for accuracy
    *
    * RETURNS : resulting image error variance of fitted model, or negative
    * value for error
    *
    * IMPORTANT : the unit for *targetVariance and the returned value is :
    *    meters^2 when useForward() is true
    *   OR
    *    pixel^2  if useForward() is false
    */
   virtual double optimizeFit(const rspfTieGptSet& tieSet,
                              double* targetVariance=0)=0;
TYPE_DATA
};
#endif
