//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class definition for rspfLlxy.  This is a simple "latitude /
// longitude to x / y" projection.
// 
//*******************************************************************
//  $Id: rspfLlxyProjection.cpp 20060 2011-09-07 12:33:46Z gpotts $

#include <rspf/projection/rspfLlxyProjection.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDatum.h>

// RTTI information for the rspfMapProjection
RTTI_DEF1(rspfLlxyProjection, "rspfLlxyProjection" , rspfMapProjection);

// About 1 meter.
static const rspf_float64 DEFAULT_DEGREES_PER_PIXEL = 8.9831528412e-006;

rspfLlxyProjection::rspfLlxyProjection()
{
  // set to about 1 meter per pixel
   theDegreesPerPixel.y = DEFAULT_DEGREES_PER_PIXEL;
   theDegreesPerPixel.x = DEFAULT_DEGREES_PER_PIXEL;
   theUlEastingNorthing.y = rspf::nan();
   theUlEastingNorthing.x = rspf::nan();
   computeMetersPerPixel(theOrigin, 
			 theDegreesPerPixel.y,
			 theDegreesPerPixel.x,
			 theMetersPerPixel);
}

rspfLlxyProjection::rspfLlxyProjection(const rspfLlxyProjection& rhs)
   :
      rspfMapProjection(rhs)
{
   theOrigin              = rhs.theOrigin;
   theUlGpt               = rhs.theUlGpt;
   theUlEastingNorthing.y = rspf::nan();
   theUlEastingNorthing.x = rspf::nan();
   theDatum               = theOrigin.datum();
   theEllipsoid           = *(theDatum->ellipsoid());
   theDegreesPerPixel.y    = rhs.theDegreesPerPixel.y;
   theDegreesPerPixel.x    = rhs.theDegreesPerPixel.x;
   computeMetersPerPixel(theOrigin, 
			 theDegreesPerPixel.y,
			 theDegreesPerPixel.x,
			 theMetersPerPixel);
}

rspfLlxyProjection::rspfLlxyProjection(const rspfGpt& origin,
                                         double latSpacing,
                                         double lonSpacing)
   :
      rspfMapProjection()
{
   theOrigin              = origin;
   theUlGpt               = origin;
   theUlEastingNorthing.y = 0.0;
   theUlEastingNorthing.x = 0.0;
   theDatum               = theOrigin.datum();
   theEllipsoid           = *(theDatum->ellipsoid());
   theDegreesPerPixel.y    = latSpacing;
   theDegreesPerPixel.x    = lonSpacing;
   computeMetersPerPixel(theOrigin, 
			 theDegreesPerPixel.y,
			 theDegreesPerPixel.x,
			 theMetersPerPixel);
}

rspfLlxyProjection::rspfLlxyProjection(const rspfEllipsoid& ellipsoid,
					 const rspfGpt& origin)
  :rspfMapProjection(ellipsoid, origin)
{
   theDegreesPerPixel.y = 1.0;
   theDegreesPerPixel.x = 1.0;
   theUlEastingNorthing.y = rspf::nan();
   theUlEastingNorthing.x = rspf::nan();
   computeMetersPerPixel(theOrigin, 
			 theDegreesPerPixel.y,
			 theDegreesPerPixel.x,
			 theMetersPerPixel);
}

rspfLlxyProjection::~rspfLlxyProjection()
{
}

rspfObject* rspfLlxyProjection::dup()const
{
   return new rspfLlxyProjection(*this);
}

//*****************************************************************************
//  METHOD: rspfMapProjection::computeDegreesPerPixel
//  
//*****************************************************************************
void rspfLlxyProjection::computeDegreesPerPixel(const rspfGpt& ground,
                                                const rspfDpt& metersPerPixel,
                                                double &deltaLat,
                                                double &deltaLon)
{
   rspfDpt mpd = ground.metersPerDegree();
   rspfDpt dpm(1.0/mpd.x,
                1.0/mpd.y);
   deltaLat = metersPerPixel.y*dpm.y;
   deltaLon = metersPerPixel.x*dpm.x;
}

//*****************************************************************************
//  METHOD: rspfMapProjection::computeMetersPerPixel
//  
//*****************************************************************************
void rspfLlxyProjection::computeMetersPerPixel(const rspfGpt& center,
						  double deltaDegreesPerPixelLat,
						  double deltaDegreesPerPixelLon,
						  rspfDpt &metersPerPixel)
{
  metersPerPixel = center.metersPerDegree();
  metersPerPixel.x *= deltaDegreesPerPixelLon;
  metersPerPixel.y *= deltaDegreesPerPixelLat;
}

void rspfLlxyProjection::worldToLineSample(const rspfGpt& worldPoint,
                                            rspfDpt&       lineSampPt) const
{
   rspfGpt gpt = worldPoint;
   
   if (*theOrigin.datum() != *gpt.datum())
   {
      // Apply datum shift if it's not the same.
      gpt.changeDatum(theOrigin.datum());
   }

   lineSampPt.line = (theUlGpt.latd() - gpt.latd()) / theDegreesPerPixel.y;
   lineSampPt.samp = (gpt.lond() - theUlGpt.lond()) / theDegreesPerPixel.x;
}

void rspfLlxyProjection::lineSampleToWorld(const rspfDpt& lineSampPt,
                                            rspfGpt&       worldPt) const
{
   worldPt.makeNan();
   // Start with the origin.  This will keep the origin's datum.
   worldPt.datum(theOrigin.datum());
   
   double lat = theUlGpt.latd() - (lineSampPt.line * theDegreesPerPixel.y);
   double lon = theUlGpt.lond() + (lineSampPt.samp * theDegreesPerPixel.x);

   //---
   // Assuming the origin had a lon between -180 and 180 and lat between -90
   // and 90.
   //---
//    if (lon > 180.0)
//    {
//       lon -= 360.0;
//    }
//    else if (lon < -180.0)
//    {
//       lon += 360.0;
//    }
//    if (lat > 90.0)
//    {
//       lat -= 90.0;
//    }
//    else if (lat < -90.0)
//    {
//       lat = -180.0 - lat;
//    }

   worldPt.latd(lat);
   worldPt.lond(lon);
   if(theElevationLookupFlag)
   {
      worldPt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(worldPt);
   }
}

std::ostream& rspfLlxyProjection::print(std::ostream& out) const
{
   out << setiosflags(ios::fixed) << setprecision(15)
       << "rspfLlxyProjection dump:"
       << "\norigin:  " << theOrigin
       << "\nlatitude spacing in decimal degrees:   " << theDegreesPerPixel.y
       << "\nlongitude spacing in decimal degrees:  " << theDegreesPerPixel.x
       << "\n\nrspfMapProjection dump:\n" << endl;

   return rspfMapProjection::print(out);
}

void rspfLlxyProjection::setMetersPerPixel(const rspfDpt& pt)
{
  rspfMapProjection::setMetersPerPixel(pt);
  computeDegreesPerPixel(theOrigin,
			 theMetersPerPixel,
			 theDegreesPerPixel.y,
			 theDegreesPerPixel.x);
}

bool rspfLlxyProjection::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   // Base class...
   rspfMapProjection::saveState(kwl, prefix);

   return true;
}

bool rspfLlxyProjection::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   // Base class...
   rspfMapProjection::loadState(kwl, prefix);

   if (theOrigin.hasNans() == false)
   {
      if ( (theDegreesPerPixel.hasNans() == false) &&
           theMetersPerPixel.hasNans() )
      {
         // Compute meters per pixel from origin and decimal degrees.
         computeMetersPerPixel(theOrigin, 
                               theDegreesPerPixel.y,
                               theDegreesPerPixel.x,
                               theMetersPerPixel);
      }
      else if( (theMetersPerPixel.hasNans() == false) &&
               theDegreesPerPixel.hasNans() )
      {
         // Compute decimal degrees per pixel from origin and meters.
         computeDegreesPerPixel(theOrigin, 
                                theMetersPerPixel,
                                theDegreesPerPixel.y,
                                theDegreesPerPixel.x);
      }
      else
      {
         // Assign some value.
         theDegreesPerPixel.y = DEFAULT_DEGREES_PER_PIXEL;
         theDegreesPerPixel.x = DEFAULT_DEGREES_PER_PIXEL;
         computeMetersPerPixel(theOrigin,
                               theDegreesPerPixel.y,
                               theDegreesPerPixel.x,
                               theMetersPerPixel);
      }
   }
    
   return true;
}

bool rspfLlxyProjection::operator==(const rspfProjection& projection) const
{
   const rspfLlxyProjection* proj
      = PTR_CAST(rspfLlxyProjection, &projection);

   if(!proj)
   {
      return false;
   }

   return ( // (theOrigin     == proj->theOrigin)     &&  // tmp... fix gpt!
            (theDegreesPerPixel.y == proj->theDegreesPerPixel.y) &&
            (theDegreesPerPixel.x == proj->theDegreesPerPixel.x) );
}


rspfDpt rspfLlxyProjection::forward(const rspfGpt &worldPoint) const
{
   rspfDpt result;

   worldToLineSample(worldPoint, result);

   return result;
}

rspfGpt rspfLlxyProjection::inverse(const rspfDpt &projectedPoint) const
{
   rspfGpt result;
   
   lineSampleToWorld(projectedPoint, result);
   
   return result;
}

void rspfLlxyProjection::setLatSpacing(double spacing)
{
   theDegreesPerPixel.y = spacing;

   // Update the meters per pixel.
   rspfDpt pt = rspfGpt(0.0, 0.0).metersPerDegree();
   theMetersPerPixel.y = pt.y * theDegreesPerPixel.y;
}

void rspfLlxyProjection::setLonSpacing(double spacing)
{
   theDegreesPerPixel.x = spacing;

   // Update the meters per pixel.
   rspfDpt pt = rspfGpt(0.0, 0.0).metersPerDegree();
   theMetersPerPixel.x = pt.x * theDegreesPerPixel.x;
}

bool rspfLlxyProjection::isGeographic() const
{
   return true;
}

double rspfLlxyProjection::getLatSpacing() const
{
   return theDegreesPerPixel.y;
}

double rspfLlxyProjection::getLonSpacing() const
{
   return theDegreesPerPixel.x;
}
