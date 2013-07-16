#include <rspf/projection/rspfCadrgProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfDatum.h>
RTTI_DEF1(rspfCadrgProjection, "rspfCadrgProjection", rspfMapProjection)
double rspfCadrgProjection::theOldZoneExtents[] = {0.0, 32.0, 48.0, 56.0, 64.0,
                                                    68.0, 72.0, 76.0, 80.0, 90.0};
double rspfCadrgProjection::theCadrgArcA[] = { 369664, 302592, 245760, 199168,
					       163328, 137216, 110080, 82432 };
double rspfCadrgProjection::theNorthLimit = 90.0*M_PI/180.0;
double rspfCadrgProjection::theSouthLimit = -90.0*M_PI/180.0;
rspfCadrgProjection::rspfCadrgProjection()
   :rspfMapProjection(rspfEllipsoid(), rspfGpt()),
    theCadrgZone(1),
    theMapScale(5000000),
    theWidth(0.0),
    theHeight(0.0)
{
   computeParameters();
}
rspfCadrgProjection::~rspfCadrgProjection()
{
   
}
rspfObject *rspfCadrgProjection::dup()const
{
   return new rspfCadrgProjection(*this);
}
rspfDpt rspfCadrgProjection::worldToLineSample(const rspfGpt &worldPoint)    const
{
   rspfDpt lineSample;
   worldToLineSample(worldPoint, lineSample);
   return lineSample;
}
rspfGpt rspfCadrgProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat=0.0;
   double lon=0.0;
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}
rspfGpt rspfCadrgProjection::inverse(const rspfDpt& /* eastingNorthing */)const
{
   double lat=0.0;
   double lon=0.0;
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}
rspfDpt rspfCadrgProjection::forward(const rspfGpt &latLon)const
{
   double easting  = 0.0;
   double northing = 0.0;
   rspfGpt gpt = latLon;
   
   if (theDatum)
   {
      if (theDatum->code() != latLon.datum()->code())
      {
         gpt.changeDatum(theDatum); // Shift to our datum.
      }
   }
   
   return rspfDpt(easting, northing);
}
rspfGpt rspfCadrgProjection::lineSampleToWorld(const rspfDpt &projectedPoint)const
{
   rspfGpt worldPoint;
   lineSampleToWorld(projectedPoint, worldPoint);
   return worldPoint;
}
void rspfCadrgProjection::worldToLineSample(const rspfGpt &worldPoint,
                                            rspfDpt&       lineSample)const
{
   double lat = worldPoint.latd();
   double lon = worldPoint.lond();
   double centerLat = theOrigin.latd();
   double centerLon = theOrigin.lond()*DEG_PER_RAD;
   
   lineSample.y = (centerLat - lat)/90.0*thePixelConstant.y;
   lineSample.x = (lon - centerLon)/360.0*thePixelConstant.x;
   lineSample = lineSample - theUlLineSample;
}
void rspfCadrgProjection::lineSampleToWorld(const rspfDpt &projectedPoint,
                                            rspfGpt& gpt)const
{
   gpt = theOrigin;
   rspfDpt adjustedPixel(projectedPoint.x + theUlLineSample.x,
                          projectedPoint.y + theUlLineSample.y);
   double lat = gpt.latd() - (90/thePixelConstant.y)*adjustedPixel.y;
   double lon = gpt.lond() + (360/thePixelConstant.x)*adjustedPixel.x;
   
   gpt.latd(lat);
   gpt.lond(lon);
   gpt.clampLat(-90, 90);
   gpt.clampLon(-180, 180);
}
double rspfCadrgProjection::computeXPixConstant(double scale,
                                                 long zone)const
{
   double adrgscale = 1000000/scale;
   
   double x_pix = (double) adrgscale*theCadrgArcA[zone-1] / 512.0;
   
   x_pix = ceil(x_pix);
   x_pix = x_pix * 1.33333;//(512*100)/(150*256);
   
   x_pix = rspf::round<int>(x_pix);
   
   return x_pix*256.0;
   
}
double rspfCadrgProjection::computeYPixConstant(double scale)const
{
   double adrgscale = 1000000/scale;
   const long CADRG_ARC_B = 400384;
   
   double y_pix = (double) adrgscale * CADRG_ARC_B / 512.0;
   
   y_pix = ceil(y_pix);
   y_pix = y_pix * 0.33333;//(512*100)/(4*150*256);
   
   y_pix = rspf::round<int>(y_pix);
   
   return y_pix*256.0;
}
void rspfCadrgProjection::computeParameters()
{
   theUlLineSample = rspfDpt(0,0);
   thePixelConstant.y = computeYPixConstant(theMapScale);
   thePixelConstant.x = computeXPixConstant(theMapScale,
                                            theCadrgZone);
   double height = theHeight;
   double width  = theWidth;
   
   if(width > thePixelConstant.x)
   {
      width  = thePixelConstant.x;
   }
   
   if(height > thePixelConstant.y)
   {
      height = thePixelConstant.y;
   }
   theUlLineSample.x = -width/2.0;
   theUlLineSample.y = -height/2.0;
}
bool rspfCadrgProjection::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{
   bool result = rspfProjection::saveState(kwl, prefix);
   kwl.add(prefix,
           rspfKeywordNames::ZONE_KW,
           theCadrgZone,
           true);
   
   kwl.add(prefix,
           "map_scale",
           theMapScale,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_LINES_KW,
           theHeight,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_SAMPLES_KW,
           theWidth,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::UL_LAT_KW,
           theUlGpt.latd(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::UL_LON_KW,
           theUlGpt.lond(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::LL_LAT_KW,
           theLlGpt.latd(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::LL_LON_KW,
           theLlGpt.lond(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::LR_LAT_KW,
           theLrGpt.latd(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::LR_LON_KW,
           theLrGpt.lond(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::UR_LAT_KW,
           theUrGpt.latd(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::UR_LON_KW,
           theUrGpt.lond(),
           true);
  
   
   if(theDatum)
   {
      kwl.add(prefix,
              rspfKeywordNames::DATUM_KW,
              theDatum->code(),
              true);
   }
   return result;
}
bool rspfCadrgProjection::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   rspfProjection::loadState(kwl, prefix);
   
   const char* lookup = kwl.find(prefix, rspfKeywordNames::UL_LAT_KW);
   if(lookup)
   {
      theUlGpt.latd(rspfString(lookup).toDouble());
   }
   else
    {
       theUlGpt.latd(90.0);
    }
    lookup = kwl.find(prefix, rspfKeywordNames::UL_LON_KW);
    if(lookup)
    {
       theUlGpt.lond(rspfString(lookup).toDouble());
    }
    else
    {
       theUlGpt.lond(-180.0);
    }
    lookup = kwl.find(prefix, rspfKeywordNames::LL_LAT_KW);
    if(lookup)
    {
       theLlGpt.latd(rspfString(lookup).toDouble());
    }
    else
    {
       theLlGpt.latd(0.0);
    }
    lookup = kwl.find(prefix, rspfKeywordNames::LL_LON_KW);
    if(lookup)
    {
       theLlGpt.lond(rspfString(lookup).toDouble());
    }
   else
   {
      theLlGpt.lond(-180.0);
   }
   lookup = kwl.find(prefix, rspfKeywordNames::LR_LAT_KW);
   if(lookup)
   {
      theLrGpt.latd(rspfString(lookup).toDouble());
   }
   else
   {
      theLrGpt.latd(0.0);
   }
   lookup = kwl.find(prefix, rspfKeywordNames::LR_LON_KW);
   if(lookup)
   {
      theLrGpt.lond(rspfString(lookup).toDouble());
   }
   else
   {
      theLrGpt.lond(180.0);
   }
   lookup = kwl.find(prefix, rspfKeywordNames::LR_LAT_KW);
   if(lookup)
   {
      theLrGpt.latd(rspfString(lookup).toDouble());
   }
   else
   {
      theLrGpt.latd(0.0);
   }
   lookup = kwl.find(prefix, rspfKeywordNames::LR_LON_KW);
   if(lookup)
   {
      theLrGpt.lond(rspfString(lookup).toDouble());
   }
   else
   {
      theLrGpt.lond(180.0);
   }
   lookup = kwl.find(prefix, rspfKeywordNames::UR_LAT_KW);
   if(lookup)
   {
      theUrGpt.latd(rspfString(lookup).toDouble());
   }
   else
   {
      theUrGpt.latd(90.0);
   }
   lookup = kwl.find(prefix, rspfKeywordNames::UR_LON_KW);
   if(lookup)
   {
      theUrGpt.lond(rspfString(lookup).toDouble());
   }
   else
   {
      theUrGpt.lond(180.0);
   }
   const char* zone = kwl.find(prefix,
                               rspfKeywordNames::ZONE_KW);
   if(zone)
   {
      theCadrgZone = rspfString(zone).toLong();
   }
   const char* mapScale = kwl.find(prefix,
                                   "map_scale");
   if(mapScale)
   {
      theMapScale = rspfString(mapScale).toDouble();
   }
   const char *height = kwl.find(prefix,
                                 rspfKeywordNames::NUMBER_LINES_KW);
   const char *width  = kwl.find(prefix,
                                 rspfKeywordNames::NUMBER_SAMPLES_KW);
   if(height)
   {
      theHeight = rspfString(height).toDouble();
   }
   
   if(width)
   {
      theWidth  = rspfString(width).toDouble();
   }
   computeParameters();
   return true;
}
bool rspfCadrgProjection::operator==(const rspfProjection& proj) const
{
   if (!rspfMapProjection::operator==(proj))
      return false;
   rspfCadrgProjection* p = PTR_CAST(rspfCadrgProjection, &proj);
   if (!p) return false;
   if (theUlGpt != p->theUlGpt) return false;
   if (theLlGpt != p->theLlGpt) return false;
   if (theLrGpt != p->theLrGpt) return false;
   if (theUrGpt != p->theUrGpt) return false;
   if (!rspf::almostEqual(theWidth,p->theWidth)) return false;
   if (!rspf::almostEqual(theHeight,p->theHeight)) return false;
   if (theCadrgZone != p->theCadrgZone) return false;
   return true;
}
