#include <sstream>
using namespace std;
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfTieGptSet.h>
#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfBilinearProjection.cpp 19682 2011-05-31 14:21:20Z dburken $";
#endif
static rspfTrace traceDebug("rspfBilinearProjection:debug");
RTTI_DEF2(rspfBilinearProjection, "rspfBilinearProjection", rspfProjection, rspfOptimizableProjection);
rspfBilinearProjection::rspfBilinearProjection()
   :
      rspfOptimizableProjection(),
      theLineSamplePt(0),
      theGeographicPt(0),
      theLatFit(),
      theLonFit()
{
#ifdef RSPF_ID_ENABLED
   if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG) << RSPF_ID << endl;
#endif    
}
rspfBilinearProjection::rspfBilinearProjection(const rspfBilinearProjection& rhs)
   :
      rspfOptimizableProjection(rhs),
      theLineSamplePt(rhs.theLineSamplePt),
      theGeographicPt(rhs.theGeographicPt),
      theLonFit(rhs.theLonFit)
{
}
rspfBilinearProjection::rspfBilinearProjection(const rspfDpt& ul,
                                         const rspfDpt& ur,
                                         const rspfDpt& lr,
                                         const rspfDpt& ll,
                                         const rspfGpt& ulg,
                                         const rspfGpt& urg,
                                         const rspfGpt& lrg,
                                         const rspfGpt& llg)
   :
      rspfOptimizableProjection(),
      theLineSamplePt(4),
      theGeographicPt(4),
      theLatFit(),
      theLonFit()
{
   theLineSamplePt[0] = ul;
   theLineSamplePt[1] = ur;
   theLineSamplePt[2] = lr;
   theLineSamplePt[3] = ll;
   theGeographicPt[0] = ulg;
   theGeographicPt[1] = urg;
   theGeographicPt[2] = lrg;
   theGeographicPt[3] = llg;
   
   initializeBilinear();
}
                       
rspfBilinearProjection::~rspfBilinearProjection()
{
}
rspfObject *rspfBilinearProjection::dup()const
{
   return new rspfBilinearProjection(*this);
}
rspfGpt rspfBilinearProjection::origin()const
{
   rspfGpt result;
   result.makeNan();
   if ( (theGeographicPt.size() == 0) || gPtsHaveNan() )
   {
      return result;
   }
   double lat        = 0.0;
   double lon        = 0.0;
   const double SIZE = theGeographicPt.size();
   
   vector<rspfGpt>::const_iterator i = theGeographicPt.begin();
   while (i != theGeographicPt.end())
   {
      lat += (*i).latd();
      lon += (*i).lond();
      ++i;
   }
   result.latd(lat/SIZE);
   result.lond(lon/SIZE);
   result.height(0.0);
   result.datum(theGeographicPt[0].datum());
   
   return result;
}
void rspfBilinearProjection::worldToLineSample(const rspfGpt& worldPoint,
                                            rspfDpt&       lineSampPt) const
{
   if(!theInverseSupportedFlag)
   {
      rspfProjection::worldToLineSample(worldPoint, lineSampPt);
   }
   else 
   {
      lineSampPt.makeNan();
      
      if (!theInterpolationPointsHaveNanFlag)
      {
         lineSampPt.x = theXFit.lsFitValue(worldPoint.lond(),
                                           worldPoint.latd());
         lineSampPt.y = theYFit.lsFitValue(worldPoint.lond(),
                                           worldPoint.latd());
      }
   }
}
void rspfBilinearProjection::lineSampleToWorld(const rspfDpt& lineSampPt,
                                                rspfGpt&       worldPt) const
{
   lineSampleHeightToWorld(lineSampPt,
                           rspf::nan(),
                           worldPt);
   
}
void rspfBilinearProjection::lineSampleHeightToWorld(
   const rspfDpt& lineSampPt,
   const double&   heightAboveEllipsoid,
   rspfGpt&       worldPt) const
{
   worldPt.makeNan();
   if (theInterpolationPointsHaveNanFlag)
   {
      return;
   }
   
   worldPt.lat = theLatFit.lsFitValue(lineSampPt.x, lineSampPt.y);
   worldPt.lon = theLonFit.lsFitValue(lineSampPt.x, lineSampPt.y);
   if (rspf::isnan(heightAboveEllipsoid) == false)
   {
      worldPt.hgt = heightAboveEllipsoid;
   }
   if (theGeographicPt.size())
   {
      worldPt.datum(theGeographicPt[0].datum());
   }
}   
bool rspfBilinearProjection::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   if (theLineSamplePt.size() != theGeographicPt.size())
   {
      return false;
   }
   rspfProjection::saveState(kwl, prefix);
   rspfString imagePoints;
   rspfString groundPoints;
   rspf::toStringList(imagePoints, theLineSamplePt);
   rspf::toStringList(groundPoints, theGeographicPt);
   kwl.add(prefix, 
           "image_points",
           imagePoints,
           true);
   kwl.add(prefix, 
           "ground_points",
           groundPoints,
           true);
#if 0
   const rspf_uint32 SIZE = (rspf_uint32)theLineSamplePt.size();
   for (rspf_uint32 i = 0; i < SIZE; ++i)
   {
      rspfString index_string = rspfString::toString(i);
      rspfString kw = "gpt";
      kw += index_string;
      ostringstream os1;
      os1 << theGeographicPt[i];
      kwl.add(prefix, kw, os1.str().c_str());
      kw = "dpt";
      kw += index_string;
      ostringstream os2;
      os2 << theLineSamplePt[i];
      kwl.add(prefix, kw, os2.str().c_str());
   }
#endif
   return true;
}
bool rspfBilinearProjection::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   rspfProjection::loadState(kwl, prefix);
   theLineSamplePt.clear();
   theGeographicPt.clear();
   rspfString imagePoints  = kwl.find(prefix, "image_points");
   rspfString groundPoints = kwl.find(prefix, "ground_points");
   
   if(!imagePoints.empty()&&!groundPoints.empty())
   {
      rspf::toVector(theLineSamplePt, imagePoints);
      rspf::toVector(theGeographicPt, groundPoints);
   }
   else 
   {
      const rspf_uint32 SIZE = kwl.numberOf(prefix, "gpt");
      if ( (SIZE == 0) || (SIZE != kwl.numberOf(prefix, "dpt")) )
      {
         return false;
      }
      
      for (rspf_uint32 i = 0; i < SIZE; ++i)
      {
         const char* lookup;
         rspfString index_string = rspfString::toString(i);
         
         rspfString kw = "gpt";
         kw += index_string;
         lookup = kwl.find(prefix, kw);
         if (lookup)
         {
            rspfGpt gp;
            gp.toPoint(std::string(lookup));
            
            if (gp.isHgtNan())
            {
               gp.height(0.0);
            }
            theGeographicPt.push_back(gp);
         }
         
         kw = "dpt";
         kw += index_string;
         lookup = kwl.find(prefix, kw);
         if (lookup)
         {
            rspfDpt dp;
            dp.toPoint(std::string(lookup));
            theLineSamplePt.push_back(dp);
         }
      }
   }
   if (traceDebug())
   {
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   
   initializeBilinear();
   return true;
}
bool rspfBilinearProjection::operator==(const rspfProjection& /* projection */) const
{
   return false;
}
rspfDpt rspfBilinearProjection::getMetersPerPixel() const
{
   rspfGpt centerG;
   rspfGpt rightG;
   rspfGpt topG;
   rspfDpt midPoint = midLineSamplePt();
                     
   lineSampleToWorld(midPoint, centerG);
   lineSampleToWorld(midPoint+rspfDpt(1,0), rightG);
   lineSampleToWorld(midPoint+rspfDpt(0,-1), topG);
   
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
void rspfBilinearProjection::initializeBilinear()
{
   theInterpolationPointsHaveNanFlag = dPtsHaveNan()||gPtsHaveNan();
   theInverseSupportedFlag = true;
   if(!theInterpolationPointsHaveNanFlag)
   {
      theLatFit.clear();
      theLonFit.clear();
      theXFit.clear();
      theYFit.clear();
      
      const rspf_uint32 SIZE = (rspf_uint32)theLineSamplePt.size();
      if (SIZE != theGeographicPt.size())
      {
         return;
      }
      
      for (rspf_uint32 i = 0; i < SIZE; ++i)
      {
         theLatFit.addSample(theLineSamplePt[i].x,
                             theLineSamplePt[i].y,
                             theGeographicPt[i].latd());
         
         theLonFit.addSample(theLineSamplePt[i].x,
                             theLineSamplePt[i].y,
                             theGeographicPt[i].lond());
         
         theXFit.addSample(theGeographicPt[i].lond(),
                           theGeographicPt[i].latd(),
                           theLineSamplePt[i].x);
         theYFit.addSample(theGeographicPt[i].lond(),
                           theGeographicPt[i].latd(),
                           theLineSamplePt[i].y);
         
      }
      
      theLatFit.solveLS();
      theLonFit.solveLS();
      theXFit.solveLS();
      theYFit.solveLS();
      rspfDpt errorResult;
      getRoundTripError(theLineSamplePt[0],
                        errorResult);
      if(errorResult.length() > 1)
      {
         theInverseSupportedFlag = false;
      }
   }
}
bool rspfBilinearProjection::dPtsHaveNan() const
{
   if (theLineSamplePt.size() == 0)
   {
      return false;
   }
   vector<rspfDpt>::const_iterator i = theLineSamplePt.begin();
   while (i != theLineSamplePt.end())
   {
      if ( (*i).hasNans() )
      {
         return true;
      }
      ++i;
   }
   return false;
}
bool rspfBilinearProjection::gPtsHaveNan() const
{
   
   if (theGeographicPt.size() == 0)
   {
      return false;
   }
   vector<rspfGpt>::const_iterator i = theGeographicPt.begin();
   while (i != theGeographicPt.end())
   {
      if ( (*i).isLatNan() || (*i).isLonNan() )
      {
         return true;
      }
      ++i;
   }
   return false;
}
rspfDpt rspfBilinearProjection::midLineSamplePt() const
{
   rspfDpt result;
   
   if ( (theLineSamplePt.size() == 0) || dPtsHaveNan())
   {
      result.makeNan();
      return result;
   }
   double x = 0.0;
   double y = 0.0;
   vector<rspfDpt>::const_iterator i = theLineSamplePt.begin();
   while (i != theLineSamplePt.end())
   {
      x += (*i).x;
      y += (*i).y;
      ++i;
   }
   const double SIZE = theLineSamplePt.size();
   result.x = x / SIZE;
   result.y = y / SIZE;
   return result;
}
std::ostream& rspfBilinearProjection::print(std::ostream& out) const
{
   rspfNotify(rspfNotifyLevel_INFO)
      << "rspfBilinearProjection::print\n";
   rspf_uint32 index = 0;
   vector<rspfDpt>::const_iterator di = theLineSamplePt.begin();
   while (di != theLineSamplePt.end())
   {
      rspfNotify(rspfNotifyLevel_INFO)
         << "theLineSamplePt[" << index << "]:  "
         << (*di) << endl;
      ++di;
      ++index;
   }
   index = 0;
   vector<rspfGpt>::const_iterator gi = theGeographicPt.begin();
   while (gi != theGeographicPt.end())
   {
      rspfNotify(rspfNotifyLevel_INFO)
         << "theGeographicPt[" << index << "]:  "
         << (*gi) << endl;
      ++gi;
      ++index;
   }
   return rspfProjection::print(out);
}
rspf_float64 rspfBilinearProjection::setTiePoints(const std::vector<rspfDpt>& lsPt, 
                                                    const std::vector<rspfGpt>& geoPt)
{
   if (lsPt.size() != geoPt.size())
   {
      rspfNotify(rspfNotifyLevel_INFO)
         << "mismatch in image and ground point number" << endl;
      return -1.0;
   }
   if (lsPt.size() < 4)
   {
      rspfNotify(rspfNotifyLevel_INFO)
         << "not enough tie points - need at least 4" << endl;
      return -1.0;
   }
   theLineSamplePt = lsPt;
   theGeographicPt = geoPt;
   initializeBilinear();
   rspf_float64 sumerr2=0.0;
   vector<rspfDpt>::const_iterator i;
   vector<rspfGpt>::const_iterator j;
   rspfGpt gres;
   for(i=theLineSamplePt.begin() , j=theGeographicPt.begin() ; i != theLineSamplePt.end() ; ++i, ++j )
   {
      gres.lat  = theLatFit.lsFitValue(i->x, i->y);
      gres.lon  = theLonFit.lsFitValue(i->x, i->y);
      gres.hgt  = j->hgt; //same height as ground point
      sumerr2 += ( rspfEcefPoint(gres) - rspfEcefPoint(*j) ).norm2(); //add squared error in meters
   }
   return sumerr2 / theLineSamplePt.size(); //variance in meter^2
}
bool rspfBilinearProjection::setupOptimizer(const rspfString& /* setup */)
{
   return false;
}
rspf_uint32
rspfBilinearProjection::degreesOfFreedom()const
{
   return 2*4; //height not used
}
double
rspfBilinearProjection::optimizeFit(const rspfTieGptSet& tieSet, double* /* targetVariance */)
{
   std::vector<rspfDpt> imagePoints;
   std::vector<rspfGpt> groundPoints;
   tieSet.getSlaveMasterPoints(imagePoints, groundPoints);
   return setTiePoints(imagePoints, groundPoints); //variance in meters
}
void rspfBilinearProjection::getTiePoints(std::vector<rspfDpt>& lsPt, 
                                           std::vector<rspfGpt>& geoPt) const
{
   lsPt  = theLineSamplePt;
   geoPt = theGeographicPt;
}
