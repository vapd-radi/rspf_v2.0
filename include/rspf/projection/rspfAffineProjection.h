#ifndef rspfAffineProjection_HEADER
#define rspfAffineProjection_HEADER
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfIpt.h>
#include <iosfwd>
#include <rspf/base/rspf2dTo2dTransform.h>
#include <rspf/base/rspfRefPtr.h>
/*!****************************************************************************
 *
 * CLASS:  rspfAffineProjection
 *
 *****************************************************************************/
class rspfAffineProjection : public rspfProjection
{
public:
   /*!
    * Default Contructor:
    */
   rspfAffineProjection();
   /*!
    * Primary constructor accepting pointer to the underlying client projection.
    */
   rspfAffineProjection(rspfProjection* client);
   /*!
    * Constructor accepts RSPF keywordlist geometry file.
    */
   rspfAffineProjection(const rspfKeywordlist& geom_kwl,
                       const char* prefix=NULL);
   
   ~rspfAffineProjection();
   
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
                                   const rspfAffineProjection& m);
   
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
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return false.
    */
   virtual bool isAffectedByElevation() const { return false; }
      
protected:
   
   /*!
    * Data Members:
    */
   rspfRefPtr<rspfProjection>      theClientProjection;
   rspfRefPtr<rspf2dTo2dTransform> theAffineTransform;
   
   TYPE_DATA
};
#endif
