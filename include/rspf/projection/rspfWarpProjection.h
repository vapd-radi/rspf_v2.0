#ifndef rspfWarpProjection_HEADER
#define rspfWarpProjection_HEADER
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfIpt.h>
class rspf2dTo2dTransform;
/*!****************************************************************************
 *
 * CLASS:  rspfWarpProjection
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfWarpProjection : public rspfProjection
{
public:
   /*!
    * Default Contructor:
    */
   rspfWarpProjection();
   /*!
    * Primary constructor accepting pointer to the underlying client projection.
    */
   rspfWarpProjection(rspfProjection* client);
   /*!
    * Constructor accepts RSPF keywordlist geometry file.
    */
   rspfWarpProjection(const rspfKeywordlist& geom_kwl,
                       const char* prefix=0);
   
   ~rspfWarpProjection();
   
   /*!
    * METHOD: worldToLineSample()
    * Performs the forward projection from ground point to line, sample.
    */
   virtual void worldToLineSample(const rspfGpt& worldPoint,
                                  rspfDpt&       lineSampPt) const;
   /*!
    * METHOD: lineSampleToWorld()
    * Performs the inverse projection from line, sample to ground (world):
    */
   virtual void lineSampleToWorld(const rspfDpt& lineSampPt,
                                      rspfGpt&       worldPt) const;
   
   /*!
    * Performs the inverse projection from line, sample to ground, bypassing
    * reference to elevation surface:
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                        const double&   hgtEllipsoid,
                                        rspfGpt&       worldPt) const;
   
   /*!
    * Extends base-class implementation. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   friend std::ostream& operator<<(std::ostream& os,
                                   const rspfWarpProjection& m);
   
   /*!
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0) const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   /*!
    * Returns pointer to a new instance, copy of this.
    */
   virtual rspfObject* dup() const;
   
   /*!
    * Returns projection's ground point origin. That is the GP corresponding
    * to line=0, sample=0.
    */
   virtual rspfGpt origin() const;
   /*!
    * Compares this instance with arg projection. NOT IMPLEMENTED.
    */
   virtual bool operator==(const rspfProjection& /* projection */) const
      { return false; }
   virtual rspfDpt getMetersPerPixel() const;
   virtual rspf2dTo2dTransform* getWarpTransform()
      {
         return theWarpTransform.get();
      }
   virtual rspf2dTo2dTransform* getAffineTransform()
      {
         return theAffineTransform.get();
      }
   virtual rspfProjection* getClientProjection()
      {
         return theClientProjection.get();
      }
   virtual void setNewWarpTransform(rspf2dTo2dTransform* warp);
   virtual void setNewAffineTransform(rspf2dTo2dTransform* affine);
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return based on theClientProjection->isAffectedByElevation().
    * If theClientProjection is null this returns true just to be safe.
    */
   virtual bool isAffectedByElevation() const;
   
protected:
   
   /*!
    * Data Members:
    */
   rspfRefPtr<rspfProjection>      theClientProjection;
   rspfRefPtr<rspf2dTo2dTransform> theWarpTransform;
   rspfRefPtr<rspf2dTo2dTransform> theAffineTransform;
   
   TYPE_DATA
};
inline bool rspfWarpProjection::isAffectedByElevation() const
{
   return ( theClientProjection.valid() ?
            theClientProjection->isAffectedByElevation() :
            false );
}
#endif
