#include <rspf/projection/rspfQuadProjection.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/elevation/rspfElevManager.h>
RTTI_DEF1(rspfQuadProjection, "rspfQuadProjection", rspfProjection);
rspfQuadProjection::rspfQuadProjection()
      :rspfProjection()
{
   theInputRect.makeNan();
   theUlg.makeNan();
   theUrg.makeNan();
   theLrg.makeNan();
   theLlg.makeNan();
}
rspfQuadProjection::rspfQuadProjection(const rspfQuadProjection& rhs)
   :rspfProjection(rhs),
    theInputRect(rhs.theInputRect),
    theUlg(rhs.theUlg),
    theUrg(rhs.theUrg),
    theLrg(rhs.theLrg),
    theLlg(rhs.theLlg),
    theLatGrid(rhs.theLatGrid),
    theLonGrid(rhs.theLonGrid)
{
}
rspfQuadProjection::rspfQuadProjection(const rspfIrect& rect,
                                         const rspfGpt& ulg,
                                         const rspfGpt& urg,
                                         const rspfGpt& lrg,
                                         const rspfGpt& llg)
      :theInputRect(rect),
       theUlg(ulg),
       theUrg(urg),
       theLrg(lrg),
       theLlg(llg)
{
   initializeGrids();
}
                       
rspfQuadProjection::~rspfQuadProjection()
{
}
rspfObject *rspfQuadProjection::dup()const
{
   return new rspfQuadProjection(*this);
}
rspfGpt rspfQuadProjection::origin()const
{
   rspfGpt result;
   result.makeNan();
   if(theUlg.isLatNan()||
      theUlg.isLonNan()||
      theUrg.isLatNan()||
      theUrg.isLonNan()||
      theLrg.isLatNan()||
      theLrg.isLonNan()||
      theLlg.isLatNan()||
      theLlg.isLonNan())
   {
      return result;
   }
   result.latd( (theUlg.latd() + theUrg.latd() + theLrg.latd() + theLlg.latd())*.25);
   result.lond( (theUlg.lond() + theUrg.lond() + theLrg.lond() + theLlg.lond())*.25);
   result.datum(theUlg.datum());
   
   return result;
}
void rspfQuadProjection::worldToLineSample(const rspfGpt& worldPoint,
                                            rspfDpt&       lineSampPt) const
{
   rspfProjection::worldToLineSample(worldPoint, lineSampPt);
}
void rspfQuadProjection::lineSampleToWorld(const rspfDpt& lineSampPt,
                                            rspfGpt&       worldPt) const
{
   lineSampleHeightToWorld(lineSampPt,
                           rspf::nan(),
                           worldPt);
   
}
void rspfQuadProjection::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                                  const double&   heightAboveEllipsoid,
                                                  rspfGpt&       worldPt) const
{
   worldPt.makeNan();
   worldPt.datum(theUlg.datum());
   
   if(theUlg.isLatNan()||
      theUlg.isLonNan()||
      theUrg.isLatNan()||
      theUrg.isLonNan()||
      theLrg.isLatNan()||
      theLrg.isLonNan()||
      theLlg.isLatNan()||
      theLlg.isLonNan()||
      theInputRect.hasNans())
   {
      return;
   }
   if(!theInputRect.pointWithin(lineSampPt))
   {
      worldPt = extrapolate(lineSampPt,
                            heightAboveEllipsoid);
      worldPt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(worldPt);
      return;
   }
   
   worldPt.lat = theLatGrid(lineSampPt);
   worldPt.lon = theLonGrid(lineSampPt);
   worldPt.hgt = heightAboveEllipsoid;
   worldPt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(worldPt);
}   
bool rspfQuadProjection::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{
   rspfProjection::saveState(kwl, prefix);
   kwl.add(prefix,
           rspfKeywordNames::DATUM_KW,
           theUlg.datum()->code(),
           true);
   
   if(theInputRect.hasNans())
   {
      kwl.add(prefix,
              rspfKeywordNames::UL_X_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::UL_Y_KW,
              "nan",
              true);
      kwl.add(prefix,
              "width",
              0,
              true);
      kwl.add(prefix,
              "height",
              0,
              true);
   }
   else
   {
      kwl.add(prefix,
              rspfKeywordNames::UL_X_KW,
              theInputRect.ul().x,
              true);
      kwl.add(prefix,
              rspfKeywordNames::UL_Y_KW,
              theInputRect.ul().y,
              true);
      kwl.add(prefix,
              "width",
              theInputRect.width(),
              true);
      kwl.add(prefix,
              "height",
              theInputRect.height(),
              true);
   }
   
   if(theUlg.isLatNan()||
      theUlg.isLonNan()||
      theUrg.isLatNan()||
      theUrg.isLonNan()||
      theLrg.isLatNan()||
      theLrg.isLonNan()||
      theLlg.isLatNan()||
      theLlg.isLonNan())
   {
      kwl.add(prefix,
              rspfKeywordNames::UL_LAT_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::UL_LON_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::UR_LAT_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::UR_LON_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::LR_LAT_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::LR_LON_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::LL_LAT_KW,
              "nan",
              true);
      kwl.add(prefix,
              rspfKeywordNames::LL_LON_KW,
              "nan",
              true);
   }
   else
   {
      kwl.add(prefix,
              rspfKeywordNames::UL_LAT_KW,
              theUlg.latd(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::UL_LON_KW,
              theUlg.lond(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::UR_LAT_KW,
              theUrg.latd(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::UR_LON_KW,
              theUrg.lond(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::LR_LAT_KW,
              theLrg.latd(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::LR_LON_KW,
              theLrg.lond(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::LL_LAT_KW,
              theLlg.latd(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::LL_LON_KW,
              theLlg.lond(),
              true);
   }
   return true;
}
bool rspfQuadProjection::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   rspfProjection::loadState(kwl, prefix);
   theUlg.makeNan();
   theUrg.makeNan();
   theLrg.makeNan();
   theLlg.makeNan();
   theInputRect.makeNan();
   rspfString ulLat = kwl.find(prefix, rspfKeywordNames::UL_LAT_KW);
   rspfString ulLon = kwl.find(prefix, rspfKeywordNames::UL_LON_KW);
   rspfString urLat = kwl.find(prefix, rspfKeywordNames::UR_LAT_KW);
   rspfString urLon = kwl.find(prefix, rspfKeywordNames::UR_LON_KW);
   rspfString lrLat = kwl.find(prefix, rspfKeywordNames::LR_LAT_KW);
   rspfString lrLon = kwl.find(prefix, rspfKeywordNames::LR_LON_KW);
   rspfString llLat = kwl.find(prefix, rspfKeywordNames::LL_LAT_KW);
   rspfString llLon = kwl.find(prefix, rspfKeywordNames::LL_LON_KW);
   rspfString datum = kwl.find(prefix, rspfKeywordNames::DATUM_KW);
   rspfString ulX    = kwl.find(prefix, rspfKeywordNames::UL_X_KW);
   rspfString ulY    = kwl.find(prefix, rspfKeywordNames::UL_Y_KW);
   rspfString width  = kwl.find(prefix, "width");
   rspfString height = kwl.find(prefix, "height");
   if(ulLat == "nan")
   {
      theUlg.latd(rspf::nan());
   }
   else
   {
      theUlg.latd(ulLat.toDouble());
   }
   if(ulLon == "nan")
   {
      theUlg.lond(rspf::nan());
   }
   else
   {
      theUlg.lond(ulLon.toDouble());
   }
   
   if(urLat == "nan")
   {
      theUrg.latd(rspf::nan());
   }
   else
   {
      theUrg.latd(urLat.toDouble());
   }
   if(urLon == "nan")
   {
      theUrg.lond(rspf::nan());
   }
   else
   {
      theUrg.lond(urLon.toDouble());
   }
   
   if(lrLat == "nan")
   {
      theLrg.latd(rspf::nan());
   }
   else
   {
      theLrg.latd(lrLat.toDouble());
   }
   if(lrLon == "nan")
   {
      theLrg.lond(rspf::nan());
   }
   else
   {
      theLrg.lond(lrLon.toDouble());
   }
   
   if(llLat == "nan")
   {
      theLlg.latd(rspf::nan());
   }
   else
   {
      theLlg.latd(llLat.toDouble());
   }
   if(llLon == "nan")
   {
      theLlg.lond(rspf::nan());
   }
   else
   {
      theLlg.lond(llLon.toDouble());
   }
   rspfIpt ul;
   
   ul.makeNan();
   
   if((ulX != "nan") &&
      (ulY != "nan"))
   {
      ul.x = ulX.toInt32();
      ul.y = ulY.toInt32();
   }
   rspf_uint32 w = width.toUInt32();
   rspf_uint32 h = height.toUInt32();
   if(datum == "")
   {
      datum = "WGE";
   }
   const rspfDatum* datumPtr = rspfDatumFactory::instance()->create(datum);
   theUlg.datum(datumPtr);
   theUrg.datum(datumPtr);
   theLrg.datum(datumPtr);
   theLlg.datum(datumPtr);
   
   if(w&&h)
   {
      theInputRect = rspfIrect(ul.x,
                                ul.y,
                                ul.x + w - 1,
                                ul.y + h - 1);
   }
   else
   {
      theInputRect.makeNan();
   }
   initializeGrids();
   return true;
}
bool rspfQuadProjection::operator==(const rspfProjection& /* projection */) const
{
   return false;
}
rspfDpt rspfQuadProjection::getMetersPerPixel() const
{
   rspfGpt centerG;
   rspfGpt rightG;
   rspfGpt topG;
   lineSampleToWorld(theInputRect.midPoint(), centerG);
   lineSampleToWorld(theInputRect.midPoint()+rspfDpt(1,0), rightG);
   lineSampleToWorld(theInputRect.midPoint()+rspfDpt(0,-1), topG);
   
   rspfEcefPoint centerP = centerG;
   rspfEcefPoint rightP  = rightG;
   rspfEcefPoint topP    = topG;
   rspfEcefVector horizontal = rightP-centerP;
   rspfEcefVector vertical   = topP-centerP;
   rspfDpt result(horizontal.magnitude(),
                   vertical.magnitude());
   result.x = (result.x + result.y)/2.0;
   result.y = result.x;
   
   return result;
}
void rspfQuadProjection::initializeGrids()
{
   rspfIpt gridSize(2,2);
   
   rspfDpt spacing = rspfDpt((double)(theInputRect.width()-1)/(gridSize.x-1),
                               (double)(theInputRect.height()-1)/(gridSize.y-1));
   
   theLatGrid.setNullValue(rspf::nan());
   theLonGrid.setNullValue(rspf::nan());
   theLatGrid.initialize(gridSize, theInputRect.ul(), spacing);
   theLonGrid.initialize(gridSize, theInputRect.ul(), spacing);
   theLatGrid.setNode(0,0, theUlg.latd());
   theLatGrid.setNode(1,0, theUrg.latd());
   theLatGrid.setNode(1,1, theLrg.latd());
   theLatGrid.setNode(0,1, theLlg.latd());
   theLonGrid.setNode(0,0, theUlg.lond());
   theLonGrid.setNode(1,0, theUrg.lond());
   theLonGrid.setNode(1,1, theLrg.lond());
   theLonGrid.setNode(0,1, theLlg.lond());
}
rspfGpt rspfQuadProjection::extrapolate(const rspfDpt& imagePoint,
                                          const double&   height) const
{
   if (imagePoint.hasNans())
   {
      return rspfGpt(rspf::nan(), rspf::nan(), rspf::nan());
   }
   rspfGpt gpt;
   rspfDpt edgePt (imagePoint);
   rspfDpt image_center (theInputRect.midPoint());
   rspfDrect clipRect = theInputRect;
   clipRect.clip(image_center, edgePt);
   rspfDpt deltaPt (edgePt - image_center);
   rspfDpt epsilon (deltaPt/deltaPt.length());
   edgePt -= epsilon;  // insure that we are inside the image
   rspfDpt edgePt_prime (edgePt - epsilon); // epsilon=1pixel
       
   rspfGpt edgeGP;
   rspfGpt edgeGP_prime;
   if (rspf::isnan(height))
   {
      lineSampleToWorld(edgePt, edgeGP);
      lineSampleToWorld(edgePt_prime, edgeGP_prime);
   }
   else
   {
      lineSampleHeightToWorld(edgePt, height, edgeGP);
      lineSampleHeightToWorld(edgePt_prime, height, edgeGP_prime);
   }
   
   double dpixel    = (edgePt-edgePt_prime).length();
   double dlat_drad = (edgeGP.lat - edgeGP_prime.lat)/dpixel;
   double dlon_drad = (edgeGP.lon - edgeGP_prime.lon)/dpixel;
   double delta_pixel = (imagePoint - edgePt).length();
   gpt.lat = edgeGP.lat + dlat_drad*delta_pixel;
   gpt.lon = edgeGP.lon + dlon_drad*delta_pixel;
   gpt.hgt = height;
   
   return gpt;
   
}
