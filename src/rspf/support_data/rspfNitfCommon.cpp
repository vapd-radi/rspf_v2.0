//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Utility class for global nitf methods.
//
//----------------------------------------------------------------------------
// $Id: rspfNitfCommon.cpp 17978 2010-08-24 16:17:00Z dburken $

#include <cstring> /* for memcpy */
#include <sstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <rspf/support_data/rspfNitfCommon.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>

static const rspfTrace traceDebug(rspfString("rspfNitfCommon:debug"));

                                   
rspfNitfCommon::rspfNitfCommon(){}

rspfNitfCommon::~rspfNitfCommon(){}

rspfString rspfNitfCommon::convertToScientificString(
   const rspf_float64& aValue,
   rspf_uint32 size)
{
   // Precision cannot hit 0 for this to work...
   if ( ((aValue < 0.0) && (size < 8)) ||
        ((aValue >= 0.0) && (size < 7)) )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfCommon::convertToScientificString DEBUG:"
            << "\nsize range error!"
            << std::endl;
      }
      return rspfString();
   }

   //--
   // Set the precision to account for size with 1.xxxxxE+01
   //---
   rspf_uint32 precision = size - 6;
   
   if (aValue < 0.0)
   {
      --precision;
   }
   
   std::ostringstream s1;
   s1 << std::setiosflags(std::ios_base::scientific|std::ios_base::internal)
      << std::setfill('0')
      // << setw(size)
      << std::setprecision(precision)
      << aValue;
   
   rspfString result = s1.str();

   // Upcase the "e".
   result.upcase();
   
   if (traceDebug())
   {
      if (result.size() != size)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfCommon::convertToScientificString DEBUG:"
            << "\nIncorrect output size!"
            << "\nValue:  " << aValue
            << "\nString: " << result
            << std::endl;
      }
   }
   return result;
}

rspfString rspfNitfCommon::convertToDoubleString(
   const rspf_float64& aValue,
   rspf_uint32 precision,
   rspf_uint32 size)
{
   rspf_uint32 width = size;
   
   std::ostringstream s1;
   s1 << std::setiosflags(std::ios_base::fixed|
                     std::ios_base::internal)
      << std::setprecision(precision)
      << std::setfill('0')
      << std::setw(width)
      << aValue;
   
   rspfString result = s1.str();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfCommon::convertToDoubleString DEBUG:"
         << "\nresult: " << result << std::endl;
      
      if (result.size() != size)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfCommon::convertToDoubleString DEBUG:"
            << "\nIncorrect output size!"
            << std::endl;
      }
   }
   return result;
}

rspfString rspfNitfCommon::convertToUIntString(rspf_uint32 aValue,
                                                rspf_uint32 size)
{
   rspf_uint32 width = size;
   
   std::ostringstream s1;
   s1 << std::setiosflags(std::ios_base::fixed|
                     std::ios_base::internal)
      << std::setfill('0')
      << std::setw(width)
      << aValue;
   
   rspfString result = s1.str();

   if (traceDebug())
   {
      if (result.size() != size)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfCommon::convertToUIntString DEBUG:"
            << "\nIncorrect output size!"
            << "\nValue:  " << aValue
            << "\nString: " << result
            << std::endl;
      }
   }
   return result;   
}

rspfString rspfNitfCommon::convertToIntString(rspf_int32 aValue,
                                                rspf_uint32 size)
{
   rspf_uint32 width = size;
   
   std::ostringstream s1;
   s1 << std::setiosflags(std::ios_base::fixed|
                     std::ios_base::internal)
      << std::setfill('0')
      << std::setw(width)
      << aValue;
   
   rspfString result = s1.str();

   if (traceDebug())
   {
      if (result.size() != size)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfCommon::convertToIntString DEBUG:"
            << "\nIncorrect output size!"
            << "\nValue:  " << aValue
            << "\nString: " << result
            << std::endl;
      }
   }
   return result;   
}

rspfString rspfNitfCommon::convertToDdLatLonLocString(const rspfDpt& pt,
                                                        rspf_uint32 precision)
{
   rspfString lat;
   rspfString lon;
   rspf_uint32 latSize = precision+4; // size = precision + "-33."
   rspf_uint32 lonSize = precision+5; // size = precision + "-122."
   
   if (pt.y >= 0.0)
   {
      lat = "+";
      --latSize;
   }
   if (pt.x >= 0.0)
   {
      lon = "+";
      --lonSize;
   }
   lat += convertToDoubleString(pt.y,
                                precision,
                                latSize);
   lon += convertToDoubleString(pt.x,
                                precision,
                                lonSize);
   rspfString result = lat+lon;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfCommon::convertToDdLatLonLocString DEBUG:"
         << "\nresult: " << result
         << std::endl;
   }

   return result;
}

void rspfNitfCommon::setField(void* fieldDestination,
                               const rspfString& src,
                               std::streamsize width,
                               std::ios_base::fmtflags ioflags,
                               char fill)
{
   std::ostringstream out;
   out << std::setw(width)
       << std::setfill(fill)
       << std::setiosflags(ioflags)
       << src.trim().c_str();
   
   memcpy(fieldDestination, out.str().c_str(), width);
}

rspfString rspfNitfCommon::encodeUtm(
                                       rspf_uint32 zone,
                                       const rspfDpt& ul,
                                       const rspfDpt& ur,
                                       const rspfDpt& lr,
                                       const rspfDpt& ll)
{
   std::ostringstream out;
   
   if(zone > 60)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nUTM zone greate than 60!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   rspf_float64 east  = ul.x;
   rspf_float64 north = ul.y;
   
   if((rspf_uint32)(east+.5) > 999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nUpper left easting too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   if((rspf_uint32)(north+.5) > 9999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nUpper left northing too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   out << std::setw(2)
   << std::setfill('0')
   << zone
   << std::setw(6)
   << std::setfill('0')
   <<(rspf_uint32)(east+.5)
   << std::setw(7)
   << std::setfill('0')
   <<(rspf_uint32)(north+.5);
   
   
   east  = ur.x;
   north = ur.y;
   
   if((rspf_uint32)(east+.5) > 999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nUpper right easting too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   if((rspf_uint32)(north+.5) > 9999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nUpper right northing too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   out << std::setw(2)
   << std::setfill('0')
   << zone
   << std::setw(6)
   << std::setfill('0')
   <<(rspf_uint32)(east+.5)
   << std::setw(7)
   << std::setfill('0')
   <<(rspf_uint32)(north+.5);
   east  = lr.x;
   north = lr.y;
   
   if((rspf_uint32)(east+.5) > 999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nLower right easting too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   if((rspf_uint32)(north+.5) > 9999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nLower right northing too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }   
   
   out << std::setw(2)
   << std::setfill('0')
   << zone
   << std::setw(6)
   << std::setfill('0')
   <<(rspf_uint32)(east+.5)
   << std::setw(7)
   << std::setfill('0')
   <<(rspf_uint32)(north+.5);
   
   east  = ll.x;
   north = ll.y;
   
   if((rspf_uint32)(east+.5) > 999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nLower left easting too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   if((rspf_uint32)(north+.5) > 9999999)
   {
      std::string s = "rspfNitfImageHeaderV2_1::encodeUtm: ERROR\nLower left northing too large for NITF field!";
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << s << std::endl;
      }
      throw std::out_of_range(s);
   }
   
   out << std::setw(2)
   << std::setfill('0')
   << zone
   << std::setw(6)
   << std::setfill('0')
   <<(rspf_uint32)(east+.5)
   << std::setw(7)
   << std::setfill('0')
   <<(rspf_uint32)(north+.5);
   
   return out.str().c_str();
}

rspfString rspfNitfCommon::encodeGeographicDms(const rspfDpt& ul,
                                                 const rspfDpt& ur,
                                                 const rspfDpt& lr,
                                                 const rspfDpt& ll)
{
   std::ostringstream out;
   
   out << rspfDms(ul.y, true).toString("ddmmssC").c_str();
   out << rspfDms(ul.x, false).toString("dddmmssC").c_str();
   out << rspfDms(ur.y, true).toString("ddmmssC").c_str();
   out << rspfDms(ur.x, false).toString("dddmmssC").c_str();
   out << rspfDms(lr.y, true).toString("ddmmssC").c_str();
   out << rspfDms(lr.x, false).toString("dddmmssC").c_str();
   out << rspfDms(ll.y, true).toString("ddmmssC").c_str();
   out << rspfDms(ll.x, false).toString("dddmmssC").c_str();

   return rspfString(out.str());
}

rspfString rspfNitfCommon::encodeGeographicDecimalDegrees(const rspfDpt& ul,
                                                            const rspfDpt& ur,
                                                            const rspfDpt& lr,
                                                            const rspfDpt& ll)
{
   std::ostringstream out;
   
   out << (ul.lat >= 0.0?"+":"")
   << std::setw(6)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << ul.lat
   << (ul.lon >= 0.0?"+":"")
   << std::setw(7)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << ul.lon;
   out << (ur.lat >= 0.0?"+":"")
   << std::setw(6)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << ur.lat
   << (ur.lon >= 0.0?"+":"")
   << std::setw(7)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << ur.lon;
   out << (lr.lat >= 0.0?"+":"")
   << std::setw(6)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << lr.lat
   << (lr.lon >= 0.0?"+":"")
   << std::setw(7)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << lr.lon;
   out << (ll.lat >= 0.0?"+":"")
   << std::setw(6)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << ll.lat
   << (ll.lon >= 0.0?"+":"")
   << std::setw(7)
   << std::setfill('0')
   << std::setprecision(3)
   << std::setiosflags(std::ios::fixed)
   << ll.lon;
   
   return rspfString(out.str());
}

rspfString rspfNitfCommon::getNitfPixelType(rspfScalarType scalarType)
{
   rspfString pixelType;
   switch(scalarType)
   {
      case RSPF_UINT8:
      case RSPF_USHORT11:
      case RSPF_UINT16:
      case RSPF_UINT32:
      {
         pixelType = "INT";
         break;
      }
      case RSPF_SINT16:
      case RSPF_SINT32:
      {
         pixelType    = "SI";
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         pixelType    = "R";
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << __FILE__ << ":" << __LINE__
            << "\nUnhandled scalar type:  " << scalarType << std::endl;
         break;
      }
   }
   return pixelType;
}

rspfString rspfNitfCommon::getCompressionRate(const rspfIrect& rect,
                                                rspf_uint32 bands,
                                                rspfScalarType scalar,
                                                rspf_uint64 lengthInBytes)
{
   rspfString result("");
   
   rspf_float64 uncompressedSize =
      rspf::scalarSizeInBytes(scalar) * rect.width() * rect.height() * bands;
   rspf_float64 bitsPerPix = rspf::getBitsPerPixel(scalar);
   rspf_float64 rate = ( bitsPerPix *
                          (static_cast<rspf_float64>(lengthInBytes) /
                           uncompressedSize) );

   // Multiply by ten as there is an implied decimal point.
   rate *= 10.0;

   // Convert to string with zero precision.
   rspfString s = rspfString::toString(rate, 0);

   if (s.size())
   {
      if (s.size() <= 3)
      {
         result = "N";
         if (s.size() == 2)
         {
            result += "0";
         }
         else if (s.size() == 1)
         {
            result += "00";
         }
         result += s;
      }
   }
   return result;
}

