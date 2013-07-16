#ifndef rspfProjection_HEADER
#define rspfProjection_HEADER
#include <iostream>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfRefPtr.h>
class RSPFDLLEXPORT rspfProjection : public rspfObject, public rspfErrorStatusInterface
{
public:
   /*!
    * Constructors, Destructor:
    */
   rspfProjection();
   virtual ~rspfProjection() {}
   virtual rspfObject *dup()const=0;
   /*!
    * METHOD: origin()
    * Returns projection's ground point origin. That is the GP corresponding
    * to line=0, sample=0.
    */
   virtual rspfGpt origin()const=0;
   /*!
    * METHODS: forward(), reverse() 
    * OBSOLETE -- provided for existing GUI code only. Bogus return value.
    */
   virtual rspfDpt forward(const rspfGpt &wp) const;  //inline below
   virtual rspfGpt inverse(const rspfDpt &pp) const;  //inline below
    virtual rspfGpt inverse_do(const rspfDpt &pp,const rspfGpt &wp) const;  //inline below
   /*!
    * METHOD: worldToLineSample()
    * Performs the forward projection from ground point to line, sample.
    */
   virtual void worldToLineSample(const rspfGpt& worldPoint,
                                  rspfDpt&       lineSampPt) const = 0;
   /*!
    * METHOD: lineSampleToWorld()
    * Performs the inverse projection from line, sample to ground (world):
    */
   virtual void lineSampleToWorld(const rspfDpt& lineSampPt,
                                  rspfGpt&       worldPt) const = 0;
   
   /*!
    * METHOD: lineSampleHeightToWorld
    * This is the pure virtual that projects the image point to the given
    * elevation above ellipsoid, thereby bypassing reference to a DEM. Useful
    * for projections that are sensitive to elevation (such as sensor models).
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                        const double&   heightAboveEllipsoid,
                                        rspfGpt&       worldPt) const = 0;
   virtual void getRoundTripError(const rspfDpt& imagePoint,
                                  rspfDpt& errorResult)const;
   virtual void getRoundTripError(const rspfGpt& groundPoint,
                                  rspfDpt& errorResult)const;
   
   virtual std::ostream& print(std::ostream& out) const;
   virtual void getGroundClipPoints(rspfGeoPolygon& gpts)const;
   /*!
    * METHODS:  saveState, loadState
    * Fulfills rspfObject base-class pure virtuals.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType = RSPF_COMPARE_FULL)const
   {
      return rspfObject::isEqualTo(obj, compareType);
   }
   
   /*!
    * OPERATOR: ==
    * Compares this instance with arg projection.
    */
   virtual bool operator==(const rspfProjection& projection) const=0;
   virtual bool operator!=(const rspfProjection& projection) const {return !(*this == projection);}
   /*!
    * ACCESS METHODS: 
    */
   virtual rspfDpt getMetersPerPixel() const=0;
   /**
    * @brief Pure virtual method to query if projection is affected by
    * elevation.
    * @return true if affected, false if not.
    */
   virtual bool isAffectedByElevation() const=0;
   
protected:
  
   TYPE_DATA
};
inline rspfDpt rspfProjection::forward(const rspfGpt &wp) const
{
   rspfDpt p;
   worldToLineSample(wp, p);
   return p;
}
inline rspfGpt rspfProjection::inverse(const rspfDpt &pp) const
{
   rspfGpt g;
   lineSampleToWorld(pp, g);
   return g;
}

inline rspfGpt rspfProjection::inverse_do(const rspfDpt &pp,const rspfGpt &gpp) const
{
   rspfGpt g(gpp);
   lineSampleToWorld(pp, g);
   return g;
}
#endif