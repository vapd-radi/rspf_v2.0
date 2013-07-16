//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
//
// LATITUDE AND LONGITUDE VALUES ARE IN DEGREES.
//
//*******************************************************************
//  $Id: rspfGpt.cpp 20615 2012-02-27 12:43:12Z gpotts $

#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfGeocent.h>
#include <iostream>
#include <sstream>

std::ostream& rspfGpt::print(std::ostream& os, rspf_uint32 precision) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = os.flags();
   
   // Set the new precision capturing old.
   std::streamsize oldPrecision = os.precision(precision);
   
   os << setiosflags(ios::fixed)
      << "( "; 

   if(isLatNan())
   {
      os << "nan" << ", ";
   }
   else
   {
      os << latd() << ", ";
   }
   if(isLonNan())
   {
      os << "nan" << ", ";
   }
   else
   {
      os << lond() << ", ";
   }
   if(isHgtNan())
   {
      os << "nan" << ", ";
   }
   else
   {
      // millimeter precision for height
      os << setprecision(3) << height() << ", ";
   }
   
   os << (theDatum?theDatum->code().c_str():"") << " )";

   // Reset flags and precision.
   os.setf(f);
   os.precision(oldPrecision);

   return os;
}

std::ostream& operator<<(std::ostream& os, const rspfGpt& pt)
{
   return pt.print(os);
}

rspfString rspfGpt::toString(rspf_uint32 precision) const
{
   std::ostringstream os;
   
   os << setprecision(precision);
   os << "(";
   
   if(isLatNan())
   {
      os << "nan" << ",";
   }
   else
   {
      os << latd() << ",";
   }
   if(isLonNan())
   {
      os << "nan" << ",";
   }
   else
   {
      os << lond() << ",";
   }
   if(isHgtNan())
   {
      os << "nan" << ",";
   }
   else
   {
      // millimeter precision for height
      os << height() << ",";
   }
   
   os << (theDatum?theDatum->code().c_str():"") << ")";
   return rspfString(os.str());
}


void rspfGpt::toPoint(const std::string& s)
{
   std::istringstream is(s);
   is >> *this;
}

std::istream& operator>>(std::istream& is, rspfGpt &pt)
{
   //---
   // Expected input format:
   // ( 30.00000000000000, -90.00000000000000, 0.00000000000000, WGE )
   //   -----latitude----  -----longitude----  ------height----  datum
   //---

   // Start with a nan point.
   pt.makeNan();

   // Check the stream.
   if (!is) return is;
   
   const int SZ = 64; // Handle real big number...
   rspfString os;
   char buf[SZ];
   char c = 0;

   //---
   // LATITUDE SECTION:
   //---
   
   // Grab data up to the first comma.
   is.get(buf, SZ, ',');

   if (!is) return is;

   // Copy to rspf string.
   os = buf;
   
   // Get rid of the '(' if there is any.
   std::string::size_type pos = os.find('(');
   if (pos != std::string::npos)
   {
      os.erase(pos, 1);
   }   

   if (os.contains("nan") == false)
   {
      pt.latd(os.toFloat64());
   }
   else
   {
      pt.latd(rspf::nan());
   }

   // Eat the comma that we stopped at.
   while (c != ',')
   {
      is.get(c);
      if (!is) break;
   }
   
   //---
   // LONGITUDE SECTION:
   //---
   
   // Grab the data up to the next ','
   is.get(buf, SZ, ',');

   if (!is) return is;

   // Copy to rspf string.
   os = buf;

   if (os.contains("nan") == false)
   {
      pt.lond(os.toFloat64());
   }
   else
   {
      pt.lond(rspf::nan());
   }

   // Eat the comma that we stopped at.
   c = 0;
   while (c != ',')
   {
      is.get(c);
      if (!is) break;
   }

   //---
   // HEIGHT SECTION:
   //---
   
   // Grab the data up to the ','
   is.get(buf, SZ, ',');

   if (!is) return is;

   // Copy to rspf string.
   os = buf;
   
   if (os.contains("nan") == false)
   {
      pt.height(os.toFloat64());
   }
   else
   {
      pt.height(rspf::nan());
   }

   // Eat the comma that we stopped at.
   c = 0;
   while (c != ',')
   {
      is.get(c);
      if (!is) break;
   }

   //---
   // DATUM SECTION:
   //---
   
   // Grab the data up to the ')'
   is.get(buf, SZ, ')');

   if (!is) return is;

   // Copy to rspf string.
   os = buf;
   os.trim(); // Just in case datum factory doesn't handle spaces.
   
   const rspfDatum* datum = rspfDatumFactoryRegistry::instance()->create(os);
   if (datum)
   {
      pt.datum(datum);
   }
   else
   {
      pt.datum(rspfDatumFactory::instance()->wgs84());
   }

   // Gobble the trailing ")".
   c = 0;
   while (c != ')')
   {
      is.get(c);
      if (!is) break;
   }

   // Finished
   return is;
}

//*****************************************************************************
//  COPY CONSTRUCTOR: rspfGpt(rspfGpt) 
//*****************************************************************************
rspfGpt::rspfGpt(const rspfGpt& src)
   : lat(src.lat),
     lon(src.lon),
     hgt(src.hgt)
{
   theDatum = src.datum();
   if(!theDatum)
   {
      theDatum = rspfDatumFactory::instance()->wgs84();
   }
//   limitLonTo180();
}

//*****************************************************************************
//  CONSTRUCTOR:  rspfGpt(rspfEcefPoint, datum)
//*****************************************************************************
rspfGpt::rspfGpt(const rspfEcefPoint& ecef_point,
                   const rspfDatum*     datum)
   :
      theDatum (datum)
{
   if(!theDatum)
   {
      theDatum = rspfDatumFactory::instance()->wgs84();
   }
   if(ecef_point.isNan())
   {
      makeNan();
   }
   else
   {
      theDatum->ellipsoid()->XYZToLatLonHeight(ecef_point.x(),
                                               ecef_point.y(),
                                               ecef_point.z(),
                                               lat,
                                               lon,
                                               hgt);
      
   }
}

//*****************************************************************************
//  METHOD: rspfGpt::changeDatum
//*****************************************************************************
void rspfGpt::changeDatum(const rspfDatum *datum)
{
   if (*datum == *theDatum) 
      return;

   // only shift if all values lat and lon is good
   if(!isLatNan() && !isLonNan()) 
   {
      if(datum)
      {
         double h = hgt;
         *this = datum->shift(*this);
         if(rspf::isnan(h))
         {
            hgt = h;
         }
         theDatum = datum;
      }
   }
}

//*****************************************************************************
//  METHOD: rspfGpt::toDmsString()
//*****************************************************************************
rspfString rspfGpt::toDmsString()const
{
   rspfString result;

   result += "lat: ";
   if(isLatNan())
   {
      result += "nan";
   }
   else
   {
      result += rspfDms(latd()).toString("dd@mm'ss.ssss\"C");
   }
   result += " lon: ";
   if(isLonNan())
   {
      result += "nan";
   }
   else
   {
      result += rspfDms(lond(),false).toString("dd@mm'ss.ssss\"C");
   }
   
   return result;
}

bool rspfGpt::isEqualTo(const rspfGpt& rhs, rspfCompareType compareType)const
{
   bool result = false;
   if(!rspf::isnan(lat)&&!rspf::isnan(rhs.lat))
   {
      result = rspf::almostEqual(lat, rhs.lat);
   }
   else 
   {
      result = rspf::isnan(lat)&&rspf::isnan(rhs.lat);
   }

   if(result)
   {
      if(!rspf::isnan(lon)&&!rspf::isnan(rhs.lon))
      {
         result = rspf::almostEqual(lon, rhs.lon);
      }
      else 
      {
         result = rspf::isnan(lon)&&rspf::isnan(rhs.lon);
      }
   }
   if(result)
   {
      if(!rspf::isnan(hgt)&&!rspf::isnan(rhs.hgt))
      {
         result = rspf::almostEqual(hgt, rhs.hgt);
      }
      else 
      {
         result = rspf::isnan(hgt)&&rspf::isnan(rhs.hgt);
      }
   }
   
   if(result)
   {
      if(theDatum&&rhs.theDatum)
      {
         if(compareType == RSPF_COMPARE_FULL)
         {
            result = theDatum->isEqualTo(*rhs.theDatum);
         }
         else 
         {
            result = theDatum == rhs.theDatum;
         }

      }
      else if(reinterpret_cast<rspf_uint64>(theDatum)|reinterpret_cast<rspf_uint64>(rhs.theDatum))
      {
         result = false;
      }
   }
   
   return result;
}


//*****************************************************************************
//  METHOD: rspfGpt::distanceTo(rspfGpt)
//
//  Computes straight-line distance to arg point, in meters.
//
//*****************************************************************************
double rspfGpt::distanceTo(const rspfGpt& arg_pt) const
{
   rspfEcefPoint p1 (*this);
   rspfEcefPoint p2 (arg_pt);

   return (p1 - p2).magnitude();
}

//*****************************************************************************
// METHOD: azimuthTo(rspfGpt)
// Computes the great-circle starting azimuth (i.e., at this gpt) to the argument gpt in degrees.
// In other words, what direction we would need to start walking in to travel the shortest 
// distance to arg_gpt (assumes spherical earth). 
// Taken from American Practical Navigator, a.k.a. Bowditch (NIMA 2002)
//*****************************************************************************
double rspfGpt::azimuthTo(const rspfGpt& gpt) const
{
   using namespace rspf; // for trig functions in degrees

   //### NOT WORKING ###
   //double dlo = fabs(lon - gpt.lon);
   //if (lat * gpt.lat < 0)
   //   dlo *= -1.0;

   //double c = atand(sind(dlo)/((cosd(lat)*tand(gpt.lat)) - (sind(lat)*cosd(dlo))));
   //return c;

   // Use alternate local method (not great circle):
   double mean_lat = 0.5*(lat + gpt.lat);
   double dlon = rspf::cosd(mean_lat)*(gpt.lon - lon);
   double dlat = gpt.lat - lat;
   double theta = rspf::atan2d(dlon,dlat);
   if (theta < 0)
      theta += 360.0;
   return theta;
}

//*****************************************************************************
//  METHOD: rspfGpt::heightMSL() const 
//
//  Returns the height data member adjusted for geoid.
//
//*****************************************************************************
double rspfGpt::heightMSL() const
{
   double offset = rspfGeoidManager::instance()->offsetFromEllipsoid(*this);
   return (hgt - offset);
}

//*****************************************************************************
//  METHOD: rspfGpt::heightMSL(double)
//
//  Sets the height data member adjusted for geoid.
//
//*****************************************************************************
void rspfGpt::heightMSL(double heightMSL)
{
   double offset = rspfGeoidManager::instance()->offsetFromEllipsoid(*this);
   hgt = heightMSL + offset;
}

//*****************************************************************************
//  METHOD: rspfGpt::metersPerDegree(double)
//
//  Computes geodetic arc lengths at this ground point.
//
//*****************************************************************************
rspfDpt rspfGpt::metersPerDegree() const
{
//#define USE_ELLIPTICAL_RADII
#ifdef USE_ELLIPTICAL_RADII
   rspfDpt radii;
   theDatum->ellipsoid()->geodeticRadii(lat, radii);
   return rspfDpt (RAD_PER_DEG * radii.x * rspf::cosd(lat), 
                    RAD_PER_DEG * radii.y);
#else 
   rspfDpt result;

   double radius = theDatum->ellipsoid()->geodeticRadius(lat);
   result.y =  RAD_PER_DEG * radius;
   result.x =  result.y * rspf::cosd(lat);

   return result;
#endif
}

bool rspfGpt::operator==(const rspfGpt& gpt) const
{
   return ( rspf::almostEqual(lat, gpt.lat) &&
            rspf::almostEqual(lon, gpt.lon) &&
            rspf::almostEqual(hgt, gpt.hgt) &&
            (*theDatum == *(gpt.theDatum)));
}

