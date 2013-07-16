//*****************************************************************************
// FILE: rspfElevSource.cc
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains implementation of class rspfElevSource. This is the base class
//   for all sources of elevation data. it maintains a single common static
//   instance of the geoid for use by all instances of objects derived from
//   this one. 
//
// SOFTWARE HISTORY:
//>
//   18Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
// $Id: rspfElevSource.cpp 21380 2012-07-25 13:18:31Z gpotts $

#include <rspf/elevation/rspfElevSource.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF1(rspfElevSource, "rspfElevSource" , rspfSource)

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfElevSource:exec");
static rspfTrace traceDebug ("rspfElevSource:debug");


static const char ENABLE_STATS_KW[] = "elevation.compute_statistics.enabled";

rspfElevSource::rspfElevSource()
   :
      theMinHeightAboveMSL (0.0),
      theMaxHeightAboveMSL (0.0),
      theNullHeightValue   (rspf::nan()),
      theSeaLevelValue     (RSPF_DEFAULT_MEAN_SEA_LEVEL),
      theGroundRect(),
      theComputeStatsFlag(false)
   
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfElevSource::rspfElevSource: entering..."
         << std::endl;
   }
   
   // User can turn on off with a keyword.
   const char* lookup =
      rspfPreferences::instance()->findPreference(ENABLE_STATS_KW);
   if (lookup)
   {
     theComputeStatsFlag = rspfString(lookup).toBool();
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG:"
         << "\ntheComputeStatsFlag:  "
         << (theComputeStatsFlag?"true":"false")
         << endl;
   }


   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfElevSource::rspfElevSource: returning..."
         << std::endl;
   }
}

rspfElevSource::rspfElevSource(const rspfElevSource& src)
   :rspfSource(src),
    theMinHeightAboveMSL(src.theMinHeightAboveMSL),
    theMaxHeightAboveMSL(src.theMaxHeightAboveMSL),
    theNullHeightValue(src.theNullHeightValue),
    theSeaLevelValue(src.theSeaLevelValue),
    theGroundRect(src.theGroundRect),
    theComputeStatsFlag(src.theComputeStatsFlag)
{
}

rspfElevSource::~rspfElevSource()
{}

const rspfElevSource& rspfElevSource::operator = (const rspfElevSource& src)
{
   if (this != &src)
   {
      theMinHeightAboveMSL = src.theMinHeightAboveMSL;
      theMaxHeightAboveMSL = src.theMaxHeightAboveMSL;
      theNullHeightValue   = src.theNullHeightValue;
      theSeaLevelValue     = src.theSeaLevelValue;
      theGroundRect        = src.theGroundRect;
      theComputeStatsFlag  = src.theComputeStatsFlag;
      
      rspfSource::operator=(src);
   }
   return *this;
}

double rspfElevSource::getHeightAboveEllipsoid(const rspfGpt& /* argGpt */)
{
   rspfNotify(rspfNotifyLevel_FATAL)
      << "FATAL rspfElevSource::getHeightAboveEllipsoid Not implemented..."
      << std::endl;
   return theNullHeightValue;
}

//*****************************************************************************
//  METHOD: intersectRay()
//  
//  Service method for intersecting a ray with the elevation surface to
//  arrive at a ground point. The ray is expected to originate ABOVE the
//  surface and pointing down.
//
//  NOTE: the gpt argument is expected to be initialized with the desired
//  datum, including ellipsoid, for the proper intersection point to be
//  computed.
//
//  LIMITATION: This release supports only single valued solutions, i.e., it
//  is possible a ray passing through one side of a mountain and out the other
//  will return an intersection with the far side. Eventually, a more robust
//  algorithm will be employed.
//
//*****************************************************************************
bool rspfElevSource::intersectRay(const rspfEcefRay& ray, rspfGpt& gpt, double defaultElevValue)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfElevSource::intersectRay: entering..." << std::endl;

   static const double CONVERGENCE_THRESHOLD = 0.001; // meters
   static const int    MAX_NUM_ITERATIONS    = 50;
   
   double          h_ellips; // height above ellipsoid
   bool            intersected;
   rspfEcefPoint  prev_intersect_pt (ray.origin());
   rspfEcefPoint  new_intersect_pt;
   double          distance;
   bool            done = false;
   int             iteration_count = 0;

   if(ray.hasNans()) 
   {
      gpt.makeNan();
      return false;
   }
   //***
   // Set the initial guess for horizontal intersect position as the ray's
   // origin, and establish the datum and ellipsoid:
   //***
   const rspfDatum*     datum     = gpt.datum();
   const rspfEllipsoid* ellipsoid = datum->ellipsoid();
//    double lat, lon, h;

//    ellipsoid->XYZToLatLonHeight(ray.origin().x(),
//                                 ray.origin().y(),
//                                 ray.origin().z(),
//                                 lat, lon, h);
//    rspfGpt nadirGpt(lat, lon, h);

//    std::cout << "nadir pt = " << nadirGpt << std::endl;
   
   gpt = rspfGpt(prev_intersect_pt, datum);

   //
   // Loop to iterate on ray intersection with variable elevation surface:
   //
   do
   {
      //
      // Intersect ray with ellipsoid inflated by h_ellips:
      //
      h_ellips = getHeightAboveEllipsoid(gpt);
      if ( rspf::isnan(h_ellips) ) h_ellips = defaultElevValue;
      
      intersected = ellipsoid->nearestIntersection(ray,
                                                   h_ellips,
                                                   new_intersect_pt);
      if (!intersected)
      {
         //
         // No intersection (looking over the horizon), so set ground point
         // to NaNs:
         //
         gpt.makeNan();
         done = true;
      }
      else
      {
         //
         // Assign the ground point to the latest iteration's intersection
         // point:
         //
         gpt = rspfGpt(new_intersect_pt, datum);
         
         //
         // Determine if convergence achieved:
         //
         distance = (new_intersect_pt - prev_intersect_pt).magnitude();
         if (distance < CONVERGENCE_THRESHOLD)
            done = true;
         else
         {
            prev_intersect_pt = new_intersect_pt;
         }
      }

      iteration_count++;

   } while ((!done) && (iteration_count < MAX_NUM_ITERATIONS));

   if (iteration_count == MAX_NUM_ITERATIONS)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfElevSource::intersectRay: Max number of iterations reached solving for ground "
                                            << "point. Result is probably inaccurate." << std::endl;
      }
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfElevSource::intersectRay: returning..." << std::endl;
   return intersected;
}

double rspfElevSource::getMinHeightAboveMSL() const
{
   return theMinHeightAboveMSL;
}

double rspfElevSource::getMaxHeightAboveMSL() const
{
   return theMaxHeightAboveMSL;
}

double rspfElevSource::getNullHeightValue() const
{
   return theNullHeightValue;
}

double rspfElevSource::getSeaLevelValue() const
{
   return theSeaLevelValue;
}

const rspfGrect& rspfElevSource::getBoundingGndRect() const
{
   return theGroundRect;
}

bool rspfElevSource::canConnectMyInputTo(rspf_int32 /* inputIndex */,
                                          const rspfConnectableObject* /* object */)const
{
   return false;
}

void rspfElevSource::initialize()
{}


bool rspfElevSource::getComputeStatisticsFlag() const
{
   return theComputeStatsFlag;
}

void rspfElevSource::setComputeStatisticsFlag(bool flag)
{
   theComputeStatsFlag = flag;
}
