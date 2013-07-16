//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Support data class for a Shuttle Radar Topography Mission (SRTM) file.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmSupportData.cpp 21527 2012-08-26 16:50:49Z dburken $

#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <rspf/support_data/rspfSrtmSupportData.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfRegExp.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfStreamFactoryRegistry.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfMapProjection.h>

// Static trace for debugging
static rspfTrace traceDebug("rspfSrtmSupportData:debug");

//---
// Start with the min and max at some default setting.
// Mt. Everest highest elev point on Earth 8850 meters.
//---
static const rspf_float64 DEFAULT_MIN = -8850.0;
static const rspf_float64 DEFAULT_MAX =  8850.0;


rspfSrtmSupportData::rspfSrtmSupportData()
   :
   theFile(),
   theNumberOfLines(0),
   theNumberOfSamples(0),
   theSouthwestLatitude(rspf::nan()),
   theSouthwestLongitude(rspf::nan()),
   theLatSpacing(rspf::nan()),
   theLonSpacing(rspf::nan()),
   theMinPixelValue(DEFAULT_MIN),
   theMaxPixelValue(DEFAULT_MAX),
   theScalarType(RSPF_SCALAR_UNKNOWN)
{
}

rspfSrtmSupportData::~rspfSrtmSupportData()
{
}

bool rspfSrtmSupportData::setFilename(const rspfFilename& srtmFile,
                                       bool scanForMinMax)
{
   theFile = srtmFile;

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSrtmSupportData::setFilename: entered:"
         << "\nsrtmFile:  " << srtmFile
         << "\nscanForMinMax flag:  " << scanForMinMax
         << std::endl;
   }
   
   theFileStream =  rspfStreamFactoryRegistry::instance()->
      createNewIFStream(theFile,
                        std::ios_base::in | std::ios_base::binary);
   if (theFileStream.valid())
   {
      if(theFileStream->fail())
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << theFile << " does not exist: leaving ..." << std::endl;
         }
         clear();
         return false;
      }
   }
   else
   {
      return false;
   }

   // Start with default.
   theMinPixelValue = DEFAULT_MIN;
   theMaxPixelValue = DEFAULT_MAX;
   
   // See if we have an rspf metadata file to initialize from.
   bool outputOmd     = false;
   bool loadedFromOmd = false;
   
   rspfFilename omdFile = theFile;
   omdFile.setExtension(rspfString("omd"));
   if (omdFile.exists())
   {
      //---
      // The loadOmd is called instead of loadState so theFile is not
      // overwrote.
      //---
      rspfKeywordlist kwl(omdFile);
      loadedFromOmd = loadOmd(kwl);
   }

   if (!loadedFromOmd)
   {
      if (!setCornerPoints())
      {
         clear();
         return false;
      }
      if (!setSize())
      {
         clear();
         return false;
      }
      outputOmd = true;
   }

   if (scanForMinMax)
   {
      // These could have been picked up in the loadOmd.
      if ( (theMinPixelValue == DEFAULT_MIN) ||
           (theMaxPixelValue == DEFAULT_MAX) )
      {
         if ( computeMinMax() )
         {
            outputOmd = true;
         }
         else
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Unable to compute min max: leaving ..." << std::endl;
            }
            clear();
            return false;
         }
      }
   }

   //---
   // NOTE:  The outputOmd flag should probably be set if !loadedFromOmd.
   // Leaving as is for now (only set if scanForMinMax).
   //---
   if (outputOmd)
   {
      rspfKeywordlist kwl;
      saveState(kwl);
      kwl.write(omdFile);
   }

   if(theFileStream->is_open())
   {
      theFileStream->close();
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << *this << std::endl;
   }
   
   return true;
}

rspfFilename rspfSrtmSupportData::getFilename() const
{
   return theFile;
}

rspf_uint32 rspfSrtmSupportData::getNumberOfLines() const
{
   return theNumberOfLines;
}

rspf_uint32 rspfSrtmSupportData::getNumberOfSamples() const
{
   return theNumberOfSamples;
}

bool rspfSrtmSupportData::getImageGeometry(rspfKeywordlist& kwl,
                                            const char* prefix)
{
   if (theFile == rspfFilename::NIL)
   {
      return false;
   }
   
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfEquDistCylProjection",
           true);

   kwl.add(prefix,
           rspfKeywordNames::ORIGIN_LATITUDE_KW,
           0.0,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::CENTRAL_MERIDIAN_KW,
           theSouthwestLongitude,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LAT_KW,
           (theSouthwestLatitude+1.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LON_KW,
           theSouthwestLongitude,
           true);

   // Add the pixel scale.
   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
           theLatSpacing,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
           theLonSpacing,
           true);

   // Add the datum.  (always WGS-84 per spec)
   kwl.add(prefix,
           rspfKeywordNames::DATUM_KW,
           rspfDatumFactory::instance()->wgs84()->code(),
           true);

   // Add the number of lines and samples.
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_LINES_KW,
           theNumberOfLines,
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_SAMPLES_KW,
           theNumberOfSamples,
           true);
   
   return true;
}

rspfRefPtr<rspfProjection> rspfSrtmSupportData::getProjection() const
{
   //---
   // Make an Equidistant Cylindrical projection with a origin at the equator
   // since the DTED post spacing is considered to be square.
   //---
   const rspfDatum* datum = rspfDatumFactory::instance()->wgs84();
   rspfRefPtr<rspfEquDistCylProjection> eq = new rspfEquDistCylProjection(*(datum->ellipsoid()));
   
   //---
   // Set the tie point.
   // NOTE: Latitude southwest corner we need northwest; hence, the +1.
   //---
   eq->setUlTiePoints( rspfGpt(theSouthwestLatitude+1.0, theSouthwestLongitude, 0.0, datum) );
   
   // Set the scale:
   eq->setDecimalDegreesPerPixel( rspfDpt(theLonSpacing, theLatSpacing) );

   rspfRefPtr<rspfProjection> proj = eq.get();

   return proj;
}

bool rspfSrtmSupportData::saveState(rspfKeywordlist& kwl,
                                     const char* prefix) const
{
   if (theFile == rspfFilename::NIL)
   {
      return false;
   }
   
   rspfString bandPrefix;
   if (prefix)
   {
      bandPrefix = prefix;
   }
   bandPrefix += "band1.";
   
   kwl.add(prefix,
           rspfKeywordNames::FILENAME_KW,
           theFile.c_str(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_BANDS_KW,
           1,
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_LINES_KW,
           theNumberOfLines,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_SAMPLES_KW,
           theNumberOfSamples,
           true);

   //---
   // Special case, store the tie point as the upper left lat so add one.
   //---
   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LAT_KW,
           (theSouthwestLatitude + 1.0),
           true);

   kwl.add(prefix,
           rspfKeywordNames::TIE_POINT_LON_KW,
           theSouthwestLongitude,
           true);

   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
           theLatSpacing,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
           theLonSpacing,
           true);

   // User can opt out of scanning for this so don't save if it is still nan.
   if (theMinPixelValue != DEFAULT_MIN)
   {
      kwl.add(bandPrefix,
              rspfKeywordNames::MIN_VALUE_KW,
              theMinPixelValue,
              true);
   }

   // User can opt out of scanning for this so don't save if it is still nan.
   if (theMaxPixelValue != DEFAULT_MAX)
   {
      kwl.add(bandPrefix.c_str(),
              rspfKeywordNames::MAX_VALUE_KW,
              theMaxPixelValue,
              true);
   }

   // constant
   kwl.add(bandPrefix,
           rspfKeywordNames::NULL_VALUE_KW,
           "-32768",
           true);

   // constant
   kwl.add(prefix,
           rspfKeywordNames::BYTE_ORDER_KW,
           "big_endian",
           true);

   // constant
   kwl.add(prefix,
           rspfKeywordNames::SCALAR_TYPE_KW,
           rspfScalarTypeLut::instance()->getEntryString(theScalarType),
           true);

   return true;
}
   
bool rspfSrtmSupportData::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   rspfString bandPrefix;
   if (prefix)
   {
      bandPrefix = prefix;
   }
   bandPrefix += "band1.";
   
   rspfString s; // For numeric conversions.
   
   const char* lookup;

   // Look for "filename" then look for deprecated "image_file" next.
   lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if (lookup)
   {
      theFile = lookup;
   }
   else
   {
      // Deprecated...
      lookup = kwl.find(prefix, rspfKeywordNames::IMAGE_FILE_KW);
      if (lookup)
      {
         theFile = lookup;
      }
      else
      {
         return false;
      }
   }

   return loadOmd(kwl, prefix);
}

bool rspfSrtmSupportData::loadOmd(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   std::string pfx;
   std::string bandPrefix;
   
   if (prefix) // Cannot give null to std::string.
   {
      pfx        = prefix;
      bandPrefix = prefix;
   }
   bandPrefix += "band1.";
   
   rspfString value;

   //---
   // Look for the min and max first since they could have been populated by
   // "cmm" (compute min max).  Min and Max are not required by this method
   // as they are handled elsewhere if they are not found here.
   //---
   
   // Not an error if not present.
   value.string() = kwl.findKey(bandPrefix, std::string(rspfKeywordNames::MIN_VALUE_KW));
   if ( value.size() )
   {
      theMinPixelValue = value.toFloat64();
   }
   
   // Not an error if not present.
   value.string() = kwl.findKey(bandPrefix.c_str(), std::string(rspfKeywordNames::MAX_VALUE_KW));
   if ( value.size() )
   {
      theMaxPixelValue = value.toFloat64();
   }
   
   value.string() = kwl.findKey(pfx, std::string(rspfKeywordNames::NUMBER_LINES_KW));
   if ( value.size() )
   {
      theNumberOfLines = value.toUInt32();
   }
   else
   {
      return false;
   }
   
   value.string() = kwl.findKey(pfx, std::string(rspfKeywordNames::NUMBER_SAMPLES_KW));
   if ( value.size() )
   {
      theNumberOfSamples = value.toUInt32();
   }
   else
   {
      return false;
   }

   //---
   // Special case the tie point was stored as the upper left so we must
   // subtract one.
   //---
   value.string() = kwl.findKey(pfx, std::string(rspfKeywordNames::TIE_POINT_LAT_KW));
   if ( value.size() )
   {
      theSouthwestLatitude = value.toFloat64() - 1.0;
   }
   else
   {
      return false;
   }

   value.string() = kwl.findKey(pfx, std::string(rspfKeywordNames::TIE_POINT_LON_KW));
   if ( value.size() )
   {
      theSouthwestLongitude = value.toFloat64();
   }
   else
   {
      return false;
   }
   
   int scalar = rspfScalarTypeLut::instance()->getEntryNumber(kwl, prefix);
   
   if (scalar != rspfLookUpTable::NOT_FOUND)
   {
      theScalarType = (rspfScalarType)scalar;
      if((theScalarType != RSPF_FLOAT32) && (theScalarType != RSPF_SINT16))
      {
         return false;
      }
   }
   else
   {
      return false;
   }
   
   theLatSpacing = 1.0 / (theNumberOfLines   - 1);
   theLonSpacing = 1.0 / (theNumberOfSamples - 1);
   
   return true;
}

rspf_float64 rspfSrtmSupportData::getSouthwestLatitude() const
{
   return theSouthwestLatitude;
}

rspf_float64 rspfSrtmSupportData::getSouthwestLongitude() const
{
   return theSouthwestLongitude;
}
rspf_float64 rspfSrtmSupportData::getLatitudeSpacing() const
{
   return theLatSpacing;
}

rspf_float64 rspfSrtmSupportData::getLongitudeSpacing() const
{
   return theLonSpacing;
}

void rspfSrtmSupportData::clear()
{
   theFile               = rspfFilename::NIL;
   theNumberOfLines      = 0;
   theNumberOfSamples    = 0;
   theSouthwestLatitude  = rspf::nan();
   theSouthwestLongitude = rspf::nan();
   theLatSpacing         = rspf::nan();
   theLonSpacing         = rspf::nan();
   theMinPixelValue      = DEFAULT_MIN;
   theMaxPixelValue      = DEFAULT_MAX;
}

bool rspfSrtmSupportData::setCornerPoints()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSrtmSupportData::setCornerPoints(): entered..." << std::endl;
   }
   
   rspfFilename f = theFile.fileNoExtension();
   rspfString regularExp1 = "[N|S][0-9][0-9][E|W][0-9][0-9][0-9]";
   rspfString regularExp2 = "[E|W][0-9][0-9][0-9][N|S][0-9][0-9]";
   rspfRegExp regEx;
   bool latLonOrderFlag = true;
   bool foundFlag = false;
   f = f.upcase();

   regEx.compile(regularExp1.c_str());
   foundFlag = regEx.find(f.c_str());
   if(!foundFlag)
   {
      regEx.compile(regularExp2.c_str());
      foundFlag = regEx.find(f.c_str());
      if(foundFlag)
      {
         latLonOrderFlag = false;
         f = rspfFilename(rspfString(f.begin()+regEx.start(),
                                       f.begin()+regEx.end()));
      }
   }
   if(foundFlag)
   {
      f = rspfFilename(rspfString(f.begin()+regEx.start(),
                                    f.begin()+regEx.end()));
   }
   if (f.size() != 7)
   {
      return false;
   }
//    rspfString s;

   if(latLonOrderFlag)
   {
      
//       s.push_back(f[1]);
//       s.push_back(f[2]);
      theSouthwestLatitude = rspfString(f.begin()+1,
                                         f.begin()+3).toDouble();//s.toDouble();
      // Get the latitude.
      if (f[static_cast<std::string::size_type>(0)] == 'S')
      {
         theSouthwestLatitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(0)] != 'N')
      {
         return false; // Must be either 's' or 'n'.
      }
      // Get the longitude.
//       s.clear();
//       s.push_back(f[4]);
//       s.push_back(f[5]);
//       s.push_back(f[6]);
      theSouthwestLongitude = rspfString(f.begin()+4,
                                          f.begin()+7).toDouble();//s.toDouble();
      if (f[static_cast<std::string::size_type>(3)] == 'W')
      {
      theSouthwestLongitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(3)] != 'E')
      {
         return false; // Must be either 'e' or 'w'.
      }
   }
   else
   {
      // Get the longitude.
//       s.clear();
//       s.push_back(f[1]);
//       s.push_back(f[2]);
//       s.push_back(f[3]);
      theSouthwestLongitude =  rspfString(f.begin()+1,
                                           f.begin()+4).toDouble();//s.toDouble();
      if (f[static_cast<std::string::size_type>(0)] == 'W')
      {
      theSouthwestLongitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(0)] != 'E')
      {
         return false; // Must be either 'e' or 'w'.
      }
//       s.clear();
      
//       s.push_back(f[5]);
//       s.push_back(f[6]);
      theSouthwestLatitude = rspfString(f.begin()+5,
                                         f.begin()+7).toDouble();//s.toDouble();
      // Get the latitude.
      if (f[static_cast<std::string::size_type>(4)] == 'S')
      {
         theSouthwestLatitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(4)] != 'N')
      {
         return false; // Must be either 's' or 'n'.
      }
   }
   

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSrtmSupportData::setCornerPoints(): leaving with true..."
         << std::endl;
   }
   return true;
}

bool rspfSrtmSupportData::setSize()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSrtmSupportData::setSize(): entered..." << std::endl;
   }

   if(theFileStream->is_open() == false)
   {
      theFileStream =
         rspfStreamFactoryRegistry::instance()->createNewIFStream(
            theFile,
            std::ios_base::in | std::ios_base::binary);
   }
   
   if (!theFileStream.valid())
   {
      return false;
   }

   if(theFileStream->fail())
   {
      return false;
   }
   
   rspf_float64 size = 0.0;
   theFileStream->seekg(0, std::ios::beg);
   if(theFileStream->isCompressed())
   {
      rspfFilename tmp = theFile;
      tmp.setExtension("hgt");
      if(!tmp.exists())
      {
         rspfOFStream out(tmp.c_str(),
                           std::ios::out|std::ios::binary);

         if(!out.fail())
         {
            bool done = false;
            char buf[1024];
            while(!done&&!theFileStream->fail())
            {
               theFileStream->read(buf, 1024);
               if(theFileStream->gcount() < 1024)
               {
                  done = true;
               }
               if(theFileStream->gcount() > 0)
               {
                  out.write(buf, theFileStream->gcount());
               }
            }
            out.close();
            size = tmp.fileSize();
            tmp.remove();
         }
      }
   }
   else
   {
      size = theFile.fileSize();
   }
   if (!size)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfSrtmSupportData::setSize(): leaving with false at line "
            << __LINE__ << std::endl;
      }
      return false;
   }

   theScalarType = RSPF_SCALAR_UNKNOWN;
   
   //---
   // Assuming square for now.  Have to check the spec for this.
   //---
   if (size == 25934402) // 2 * 3601 * 3601 three arc second
   {
      theNumberOfLines     = 3601;
      theNumberOfSamples   = 3601;
      theScalarType = RSPF_SINT16;
   }
   else if(size == 51868804) // 4*3601*3601
   {
      theNumberOfLines   = 3601;
      theNumberOfSamples = 3601;
      theScalarType = RSPF_FLOAT32;
   }
   else if (size == 2884802) // 2 * 1201 * 1201 one arc second
   {
      theNumberOfLines   = 1201;
      theNumberOfSamples = 1201;
      theScalarType = RSPF_SINT16;
   }
   else if (size == 5769604)
   {
      theNumberOfLines   = 1201;
      theNumberOfSamples = 1201;
      theScalarType = RSPF_FLOAT32;
   }
   else // try to get a square width and height
   {
      rspf_uint64 lines   = (rspf_uint64)sqrt((rspf_float64)(size / 2));
      rspf_uint64 samples = (rspf_uint64)sqrt((rspf_float64)(size / 2));
      // check square
      if(lines*samples*2 == size)
      {
         theNumberOfLines   = lines;
         theNumberOfSamples = samples;
         theScalarType = RSPF_SINT16;
         
      }
      else
      {
         rspf_uint64 lines   = (rspf_uint64)sqrt((rspf_float64)(size / 4));
         rspf_uint64 samples = (rspf_uint64)sqrt((rspf_float64)(size / 4));
         // check square
         if(lines*samples*4 == size)
         {
            theNumberOfLines   = lines;
            theNumberOfSamples = samples;
            theScalarType = RSPF_FLOAT32;
         }
         else
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfSrtmSupportData::setSize(): leaving with false at line " << __LINE__ << std::endl;
            }
            return false;
         }
      }
   }
      
   theLatSpacing      = 1.0 / (theNumberOfLines   - 1);
   theLonSpacing      = 1.0 / (theNumberOfSamples - 1);
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSrtmSupportData::setSize(): leaving with true..."
         << std::endl;
   }

   theFileStream->close();
   
   return true;
}

bool rspfSrtmSupportData::computeMinMax()
{
   if(theScalarType == RSPF_FLOAT32)
   {
      return computeMinMaxTemplate((rspf_float32)0,
                                   -32768.0);
   }
   return computeMinMaxTemplate((rspf_sint16)0,
                                -32768.0);
}

template <class T>
bool rspfSrtmSupportData::computeMinMaxTemplate(T /* dummy */,
                                                 double defaultNull)
{
   if(theFileStream->is_open() == false)
   {
      theFileStream =
         rspfStreamFactoryRegistry::instance()->createNewIFStream(
            theFile,
            std::ios_base::in | std::ios_base::binary);
   }
   
   if (!theFileStream.valid())
   {
      return false;
   }

   if(theFileStream->fail())
   {
      return false;
   }
   
   const size_t BYTES_IN_LINE = theNumberOfSamples * 2;
   const T NULL_PIX = (T)defaultNull;

   double minValue = 1.0/FLT_EPSILON;
   double maxValue = -1.0/FLT_EPSILON;
   T* line_buf = new T[theNumberOfSamples];
   char* char_buf = (char*)line_buf;
   rspfEndian swapper;

   rspfByteOrder endianType = rspf::byteOrder();
   for (rspf_uint32 line = 0; line < theNumberOfLines; ++line)
   {
      theFileStream->read(char_buf, (std::streamsize)BYTES_IN_LINE);
      if(endianType == RSPF_LITTLE_ENDIAN)
      {
         swapper.swap(line_buf, theNumberOfSamples);
      }
      for (rspf_uint32 sample = 0; sample < theNumberOfSamples; ++sample)
      {
         if (line_buf[sample] == NULL_PIX) continue;
         if (line_buf[sample] > maxValue) maxValue = line_buf[sample];
         if (line_buf[sample] < minValue) minValue = line_buf[sample];
      }
   }
   delete [] line_buf;
   theMinPixelValue = minValue;
   theMaxPixelValue = maxValue;

   theFileStream->close();

   return true;
}

rspf_float64 rspfSrtmSupportData::getMinPixelValue() const
{
   return theMinPixelValue;
}

rspf_float64 rspfSrtmSupportData::getMaxPixelValue() const
{
   return theMaxPixelValue;
}

rspfScalarType rspfSrtmSupportData::getScalarType()const
{
   return theScalarType;
}

std::ostream& rspfSrtmSupportData::print(std::ostream& out) const
{
   out << std::setprecision(15) << "rspfSrtmSupportData data members:"
       << "\nFile:                  " << theFile
       << "\nLines:                 " << theNumberOfLines
       << "\nSamples:               " << theNumberOfSamples
       << "\nSouth West Latitude:   " << theSouthwestLatitude
       << "\nSouth West Longitude:  " << theSouthwestLongitude
       << "\nLatitude spacing:      " << theLatSpacing
       << "\nLongitude spacing:     " << theLonSpacing
       << "\nMin post value:        " << theMinPixelValue
       << "\nMax post value:        " << theMaxPixelValue
       << std::endl;
   return out;
}


