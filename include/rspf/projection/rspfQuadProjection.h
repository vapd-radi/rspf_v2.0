#ifndef rspfQuadProjection_HEADER
#define rspfQuadProjection_HEADER
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDblGrid.h>
class rspfQuadProjection : public rspfProjection
{
 public:
   rspfQuadProjection();
   rspfQuadProjection(const rspfQuadProjection& rhs);
   rspfQuadProjection(const rspfIrect& rect,
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
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return false.
    */
   virtual bool isAffectedByElevation() const { return false; }
 protected:
   virtual ~rspfQuadProjection();
   
   rspfIrect theInputRect;
   rspfGpt theUlg;
   rspfGpt theUrg;
   rspfGpt theLrg;
   rspfGpt theLlg;
   rspfDblGrid theLatGrid;   
   rspfDblGrid theLonGrid;
   void initializeGrids();
   rspfGpt extrapolate(const rspfDpt& imagePoint,
                        const double&   height) const;
TYPE_DATA   
};
#endif /* #ifndef rspfQuadProjection_HEADER */
