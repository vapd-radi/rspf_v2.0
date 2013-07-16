#include <iostream>
#include <iomanip>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfProjection:exec");
static rspfTrace traceDebug ("rspfProjection:debug");
using namespace std;
RTTI_DEF1(rspfProjection, "rspfProjection", rspfObject);
rspfProjection::rspfProjection()
{
}
bool rspfProjection::saveState(rspfKeywordlist& kwl,
                           const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           TYPE_NAME(this),
           true);
   return true;
}
bool rspfProjection::loadState(const rspfKeywordlist& /* kwl */,
                                const char*             /* prefix */)
{
   return true;
}
void rspfProjection::worldToLineSample(const rspfGpt& worldPoint,
                                        rspfDpt&       ip) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfProjection::worldToLineSample: entering..." << std::endl;
   static const double PIXEL_THRESHOLD    = 0.1; // acceptable pixel error
   static const int    MAX_NUM_ITERATIONS = 20;
   int iters = 0;
   double height = worldPoint.hgt;
   if ( rspf::isnan(height) )
   {
      height = 0.0;
   }
   ip.u = 0;
   ip.v = 0;
   
   rspfDpt ip_du;
   rspfDpt ip_dv;
   rspfGpt gp, gp_du, gp_dv;
   double dlat_du, dlat_dv, dlon_du, dlon_dv;
   double delta_lat, delta_lon, delta_u, delta_v;
   double inverse_norm;
   
   do
   {
      ip_du.u = ip.u + 1.0;
      ip_du.v = ip.v;
      ip_dv.u = ip.u;
      ip_dv.v = ip.v + 1.0;
      
      lineSampleHeightToWorld(ip,    height, gp);
      lineSampleHeightToWorld(ip_du, height, gp_du);
      lineSampleHeightToWorld(ip_dv, height, gp_dv);
      dlat_du = gp_du.lat - gp.lat; //e
      dlon_du = gp_du.lon - gp.lon; //g
      dlat_dv = gp_dv.lat - gp.lat; //f
      dlon_dv = gp_dv.lon - gp.lon; //h
      delta_lat = worldPoint.lat - gp.lat;
      delta_lon = worldPoint.lon - gp.lon;
      
      inverse_norm = dlat_dv*dlon_du - dlat_du*dlon_dv; // fg-eh
      if (inverse_norm != 0)
      {
          delta_u = (-dlon_dv*delta_lat + dlat_dv*delta_lon)/inverse_norm;
          delta_v = ( dlon_du*delta_lat - dlat_du*delta_lon)/inverse_norm;
          ip.u += delta_u;
          ip.v += delta_v;
      }
      else
      {
         delta_u = 0;
         delta_v = 0;
      }
      iters++;
      
   } while (((fabs(delta_u) > PIXEL_THRESHOLD) ||
             (fabs(delta_v) > PIXEL_THRESHOLD)) &&
            (iters < MAX_NUM_ITERATIONS));
   if (iters == MAX_NUM_ITERATIONS)
   {
   }
   
}
void rspfProjection::getRoundTripError(const rspfDpt& imagePoint,
                                        rspfDpt& errorResult)const
{
   rspfGpt world;
   rspfDpt testPt;
   
   lineSampleToWorld(imagePoint, world);
   worldToLineSample(world, testPt);
   errorResult = imagePoint - testPt;
}
void rspfProjection::getRoundTripError(const rspfGpt& groundPoint,
                                        rspfDpt& errorResult)const
{
   rspfDpt tempPt;
   rspfGpt tempGround;
   worldToLineSample(groundPoint, tempPt);
   lineSampleToWorld(tempPt, tempGround);
   
   errorResult = rspfDpt(groundPoint) - rspfDpt(tempGround);
}
void rspfProjection::getGroundClipPoints(rspfGeoPolygon& /* gpts */)const
{
}
std::ostream& rspfProjection::print(std::ostream& out) const
{
   return rspfErrorStatusInterface::print(out);
}
