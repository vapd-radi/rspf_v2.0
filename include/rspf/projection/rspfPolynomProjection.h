#ifndef rspfPolynomProjection_HEADER
#define rspfPolynomProjection_HEADER
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfOptimizableProjection.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfPolynom.h>
/**
 * class for ground to image polynomial projection: (x,y,z)|->(x,y)
 * polynom order is configurable
 */
class RSPF_DLL rspfPolynomProjection : public rspfProjection,
                                         public rspfOptimizableProjection
{
public:
   
   /**
    * Constructors
    */
   rspfPolynomProjection();
   rspfPolynomProjection(const rspfPolynomProjection& copyme);
   ~rspfPolynomProjection();
   /**
    * Basic operations
    */
   virtual bool operator==(const rspfProjection& projection) const;
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual std::ostream& print(std::ostream& out) const;
   virtual rspfObject* dup() const { return new rspfPolynomProjection(*this); }
   /**
    * Projection / localization methods
    */
   virtual void  worldToLineSample(const rspfGpt& world_point,
                                   rspfDpt&       image_point) const;
   virtual void  lineSampleToWorld(const rspfDpt& image_point,
                                   rspfGpt&       world_point) const;
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   virtual rspfGpt origin()const;
   virtual rspfDpt getMetersPerPixel() const;
   
   /**
    * optimizableProjection implementation
    */
   virtual bool setupOptimizer(const rspfString& setup);
   inline virtual bool useForward()const {return true;}
   virtual rspf_uint32 degreesOfFreedom()const;
   virtual double optimizeFit(const rspfTieGptSet& tieSet, double* targetVariance=NULL);
   
   /**
    * accessors (not all of them inline)
    */
   const rspfPolynom< rspf_float64 , 3 >::EXPT_SET& getExpSet()const { return theExpSet; }
   void setExpSet(const rspfPolynom< rspf_float64 , 3 >::EXPT_SET& nset) { theExpSet = nset; }
   void setupDesiredExponents(int horizonal_ord, int vertical_ord); //!added for easy setup
   bool setupDesiredExponents(const rspfString& monoms);//!added for easy setup
   const rspfPolynom< rspf_float64 , 3 >& getPolyLine()const {return thePolyLine;}
   void setPolyLine(const rspfPolynom< rspf_float64 , 3 >& poly);
   const rspfPolynom< rspf_float64 , 3 >& getPolySamp()const {return thePolySamp;}
   void setPolySamp(const rspfPolynom< rspf_float64 , 3 >& poly);
   const rspf_float64& getLonOffset()const { return theLonOffset;}
   void setLonOffset(const rspf_float64& v) { theLonOffset = v; }
   const rspf_float64& getLonScale()const { return theLonScale;}
   void setLonScale(const rspf_float64& v) { theLonScale = v; }
   const rspf_float64& getLatOffset()const { return theLatOffset;}
   void setLatOffset(const rspf_float64& v) { theLatOffset = v; }
   const rspf_float64& getLatScale()const { return theLatScale;}
   void setLatScale(const rspf_float64& v) { theLatScale = v; }
   const rspf_float64& getHgtOffset()const { return theHgtOffset;}
   void setHgtOffset(const rspf_float64& v) { theHgtOffset = v; }
   const rspf_float64& getHgtScale()const { return theHgtScale;}
   void setHgtScale(const rspf_float64& v) { theHgtScale = v; }
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return true.
    */
   virtual bool isAffectedByElevation() const { return true; }
protected:
   /**
    * desired exponents tuples set (for both line and sample polynoms)
    * used for optimization
    */
   rspfPolynom< rspf_float64 , 3 >::EXPT_SET theExpSet;
   /** 
    * ground (3d) to image(2D) polynoms
    */
   rspfPolynom< rspf_float64 , 3 > thePolyLine;//!xyz polynom
   rspfPolynom< rspf_float64 , 3 > thePolySamp;
   /**
    * polynom partial derivatives
    */
   rspfPolynom< rspf_float64 , 3 > thePolyLine_DLon;
   rspfPolynom< rspf_float64 , 3 > thePolyLine_DLat;
   rspfPolynom< rspf_float64 , 3 > thePolySamp_DLon;
   rspfPolynom< rspf_float64 , 3 > thePolySamp_DLat;
   /**
    * normalization parameters
    */
   rspf_float64 theLonOffset;
   rspf_float64 theLonScale;
   rspf_float64 theLatOffset;
   rspf_float64 theLatScale;
   rspf_float64 theHgtOffset;
   rspf_float64 theHgtScale;
   /**
    * protected methods
    */
   void buildDerivatives();
   void buildLineDerivatives();
   void buildSampDerivatives();
   bool stringToExp(const rspfString& s, rspfPolynom< rspf_float64 , 3 >::EXP_TUPLE& et)const; //!takes string of xyz monoms, x2z3
   int getSymbol(rspfString& ts,const rspfString& symbols)const;
   int getExponent(rspfString& ts)const;
TYPE_DATA
};
#endif
