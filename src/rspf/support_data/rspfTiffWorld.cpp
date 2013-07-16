//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
// 
// Description: Container class for a tiff world file data.
//
//********************************************************************
// $Id: rspfTiffWorld.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/support_data/rspfTiffWorld.h>
#include <rspf/base/rspfNotify.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace std;

//**************************************************************************
// rspfTiffWorld::rspfTiffWorld()
//***************************************************************************
rspfTiffWorld::rspfTiffWorld()
   :
      theXform1(1.0),
      theXform2(0.0),
      theXform3(0.0),
      theXform4(-1.0),
      theTranslation(0.0,0.0),
      thePixelType(RSPF_PIXEL_IS_AREA),
      theUnit(RSPF_METERS),
      theComputedScale(0.0, 0.0),
      theComputedRotation(0.0)
{}

//**************************************************************************
// rspfTiffWorld::rspfTiffWorld(const char* file, PixelType ptype)
//***************************************************************************
rspfTiffWorld::rspfTiffWorld(const char* file, 
			       rspfPixelType ptype,
			       rspfUnitType  scaleUnits)
   :
      theXform1(1.0),
      theXform2(0.0),
      theXform3(0.0),
      theXform4(-1.0),
      theTranslation(0.0,0.0),
      thePixelType(ptype),
      theUnit(scaleUnits),
      theComputedScale(0.0, 0.0),
      theComputedRotation(0.0)
{
   open(rspfFilename(file), ptype, scaleUnits);
}

bool rspfTiffWorld::open(const rspfFilename& file, rspfPixelType ptype, rspfUnitType  unit)
{
   bool result = false;

   ifstream is;
   is.open(file.c_str());

   if( !is.is_open() )
   {
      // ESH 07/2008, Trac #234: RSPF is case sensitive 
      // when using worldfile templates during ingest
      // -- If first you don't succeed with the user-specified
      // filename, try again with the results of a case insensitive search.
      rspfFilename fullName(file);
      rspfDirectory directory(fullName.path());
      rspfFilename filename(fullName.file());
      
      std::vector<rspfFilename> result;
      bool bSuccess = directory.findCaseInsensitiveEquivalents(
         filename, result );
      if ( bSuccess == true )
      {
         int numResults = (int)result.size();
         int i;
         for ( i=0; i<numResults && !is.is_open(); ++i )
         {
            is.open( result[i].c_str() );
         }
      }
   }
      
   if ( is.is_open() )
   {
      double x,y;
      is >> theXform1 >> theXform2 >> theXform3 >> theXform4 >> x >> y;
      theTranslation = rspfDpt(x,y);
      
      // Compute the affine parameters from the transform:
      theComputedRotation = atan2(theXform2,theXform1);
      
/*
 * Commented out warning.
 * Used all the time for tie and scale, NOT for affine. We could wrap around trace
 * if we added trace to class. (drb - 20110115)
 */
#if 0
      double angle2 = atan2(theXform4,theXform3);
      if (fabs(theComputedRotation - angle2) > 0.00001)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfTiffWorld -- Non-affine transform encountered."
            << " Use of an affine transform to represent this world file geometry will result in errors."
            << endl;
      }
#endif
      
      double cos_rot = cos(theComputedRotation);
      if (cos_rot != 0.0)
      {
         theComputedScale.x = theXform1/cos_rot;
         theComputedScale.y = theXform4/cos_rot;
      }
      else
      {
         theComputedScale.x = theXform4;
         theComputedScale.y = theXform1;
      }
      thePixelType = ptype;
      theUnit = unit;
      is.close();
      result = true;
   }
   return result;
}

rspfTiffWorld::~rspfTiffWorld()
{
}

void rspfTiffWorld::forward(const rspfDpt& ip,
                             rspfDpt& transformedPoint)
{
   transformedPoint.x = ip.x*theXform1 + ip.y*theXform2 + theTranslation.x;
   transformedPoint.y = ip.x*theXform3 + ip.y*theXform4 + theTranslation.y;
}

bool rspfTiffWorld::saveToOssimGeom(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfDpt scale(fabs(theXform1), fabs(theXform4));
   rspfDpt tie(theTranslation.x, theTranslation.y);

   if ( (theUnit == RSPF_FEET) || (theUnit == RSPF_US_SURVEY_FEET) )
   {
      // Convert to meters.
      scale.x = rspfUnitConversionTool(scale.x, theUnit).getMeters();
      scale.y = rspfUnitConversionTool(scale.y, theUnit).getMeters();
      tie.x   = rspfUnitConversionTool(tie.x, theUnit).getMeters();
      tie.y   = rspfUnitConversionTool(tie.y, theUnit).getMeters();
   }
   
   // Shift the tie point to be relative to the center of the pixel.
   if(thePixelType == RSPF_PIXEL_IS_AREA)
   {
      tie.x += (scale.x/2.0);
      tie.y -= (scale.y/2.0);

      // Adjust the keyword list to reflect pixel is point.
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_TYPE_KW,
              "pixel_is_point",
              true);
   }

   // Units in feet converted to meters up above.
   rspfString units = "meters";
   if(theUnit == RSPF_DEGREES)
   {
      units = "degrees";
   }
   
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_XY_KW,
           tie.toString().c_str(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_UNITS_KW,
           units.c_str(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::PIXEL_SCALE_XY_KW,
           scale.toString().c_str(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::PIXEL_SCALE_UNITS_KW,
           units.c_str(),
           true);

   return true;
}

bool rspfTiffWorld::loadFromOssimGeom(const rspfKeywordlist& kwl, const char* prefix)
{
   theXform2 = 0.0;
   theXform3 = 0.0;

   const char* lookup;

   // Get the scale...
   lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_SCALE_XY_KW);
   if (lookup)
   {
      rspfDpt scale;
      scale.toPoint(std::string(lookup));
      theXform1      = scale.x;
      theXform4      = -(scale.y);
   }
   else // BACKWARDS COMPATIBILITY LOOKUPS...
   {
      rspfString xscale = kwl.find(prefix, rspfKeywordNames::METERS_PER_PIXEL_X_KW);
      rspfString yscale = kwl.find(prefix, rspfKeywordNames::METERS_PER_PIXEL_Y_KW);
      theXform1 = xscale.toDouble();
      theXform4 = -(yscale.toDouble());
   }

   // Get the tie...
   lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_XY_KW);
   if (lookup)
   {
      rspfDpt tie;
      tie.toPoint(std::string(lookup));
      theTranslation.x  = tie.x;
      theTranslation.y  = tie.y;
   }
   else // BACKWARDS COMPATIBILITY LOOKUPS...
   {
      rspfString easting  =
         kwl.find(prefix, rspfKeywordNames::TIE_POINT_EASTING_KW);
      rspfString northing =
         kwl.find(prefix, rspfKeywordNames::TIE_POINT_NORTHING_KW);
      theTranslation.x  = easting.toDouble();
      theTranslation.y  = northing.toDouble(); 
   }
   
   return true;
}

std::ostream& rspfTiffWorld::print(std::ostream& out) const
{
   out << setiosflags(ios::fixed) << setprecision(15)
       << theXform1     << "\n"
       << theXform2     << "\n"
       << theXform3     << "\n"
       << theXform4     << "\n"
       << theTranslation
       << endl;
   return out;
}

std::ostream& operator<<(std::ostream& out, const rspfTiffWorld& obj)
{
   return obj.print(out);
}

