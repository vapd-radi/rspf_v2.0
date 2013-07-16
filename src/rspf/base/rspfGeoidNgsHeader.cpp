#include <rspf/base/rspfGeoidNgsHeader.h>
#include <fstream>
#include <rspf/base/rspfEndian.h>
#include <cmath>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfCommon.h>

using namespace std;

ostream& operator << (ostream &out, const rspfGeoidNgsHeader &data)
{
   out << "Filename:        " << data.theFilename             << endl
       << "Southern lat:    " << data.theSouthernMostLatitude << endl
       << "Western lon:     " << data.theWesternMostLongitude << endl
       << "Lat spacing:     " << data.theLatDelta             << endl
       << "Lon spacing:     " << data.theLonDelta             << endl
       << "rows:            " << data.theRows                 << endl
       << "cols:            " << data.theCols                 << endl
       << "Datatype:        " << data.theDataType             << endl;

   return out;
}


rspfGeoidNgsHeader::rspfGeoidNgsHeader()
   :theFilename(""),
    theByteOrder(RSPF_LITTLE_ENDIAN),
    theSouthernMostLatitude(0.0),
    theWesternMostLongitude(0.0),
    theLatDelta(0.0),
    theLonDelta(0.0),
    theRows(0),
    theCols(0),
    theDataType(0)
{   
}

rspfGeoidNgsHeader::rspfGeoidNgsHeader(const rspfFilename &fileName,
                                       rspfByteOrder byteOrder)
{
   initialize(fileName, byteOrder);
}

bool rspfGeoidNgsHeader::initialize(const rspfFilename &fileName,
                                    rspfByteOrder byteOrder)
{
   ifstream input(fileName.c_str(), ios::in|ios::binary);
   if(!input)
   {
//      cerr << MODULE << " Cannot open geoid file in " << fileName.c_str()
//           << endl;
       return false;
   }

   theByteOrder = byteOrder;
   double latOrigin  = 0.0;
   double lonOrigin  = 0.0;
   double latSpacing = 0.0;
   double lonSpacing = 0.0;
   int    rows       = 0;
   int    cols       = 0;
   int    type       = 0;
   
   input.read((char*)&latOrigin, 8);
   input.read((char*)&lonOrigin, 8);
   input.read((char*)&latSpacing, 8);
   input.read((char*)&lonSpacing, 8);
   input.read((char*)&rows, 4);
   input.read((char*)&cols, 4);
   input.read((char*)&type, 4);

   rspfEndian endian;

   if(endian.getSystemEndianType() != byteOrder)
   {
      endian.swap(latOrigin);
      endian.swap(lonOrigin);
      endian.swap(latSpacing);
      endian.swap(lonSpacing);
      endian.swap(rows);
      endian.swap(cols);
      endian.swap(type);
   }
   
   theFilename = fileName;
   theSouthernMostLatitude = latOrigin;
   theWesternMostLongitude = lonOrigin;
   theLatDelta             = latSpacing;
   theLonDelta             = lonSpacing;
   theRows                 = rows;
   theCols                 = cols;
   theDataType             = type;

   return ((theSouthernMostLatitude >= -FLT_EPSILON)&&
           (theSouthernMostLatitude <= 90.0)&&
           (theWesternMostLongitude >= -FLT_EPSILON)&&
           (theWesternMostLongitude <= 360.0));
}

bool rspfGeoidNgsHeader::pointWithin(double lat, double lon)const
{
   return( (lat >= theSouthernMostLatitude) &&
           (lat <= (theSouthernMostLatitude + theRows*theLatDelta)) &&
           (lon >= theWesternMostLongitude) &&
           (lon <= theWesternMostLongitude + theCols*theLonDelta));
}

double rspfGeoidNgsHeader::getHeightDelta(double lat,
                                           double lon)const
{
   float result=0.0;

   // note the headers go from 0 to 360 degrees starting at the prime meridian.
   // ours goes from -180 to 180 degrees.  We will need to shift
   //
   if(lon < 0.0) lon += 360;
   double latSpaces = (lat - theSouthernMostLatitude)/theLatDelta;
   double lonSpaces = (lon - theWesternMostLongitude)/theLonDelta;

   if(latSpaces < -FLT_EPSILON) return rspf::nan();
   if(lonSpaces < -FLT_EPSILON) return rspf::nan();

   long latSpace0 = (long)std::floor(latSpaces);
   long latSpace1 = latSpace0 + 1;
   long lonSpace0 = (long)std::floor(lonSpaces);
   long lonSpace1 = lonSpace0 + 1;

   ifstream input(filename().c_str(), ios::in|ios::binary);
   if(!input)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL: " << "rspfGeoidNgsHeader::getGeoidHeight, " << "unable to open file: " << theFilename << "\n";
      return 0;
   }

   long offset00 = (long)(headerSize() + (latSpace0*theCols + lonSpace0)*dataTypeSize());
   long offset01 = (long)(headerSize() + (latSpace0*theCols + lonSpace1)*dataTypeSize());
   long offset11 = (long)(headerSize() + (latSpace1*theCols + lonSpace1)*dataTypeSize());
   long offset10 = (long)(headerSize() + (latSpace1*theCols + lonSpace0)*dataTypeSize());

   double tLat = latSpaces - floor(latSpaces);
   double tLon = lonSpaces - floor(lonSpaces);


   float v00;
   float v01;
   float v11;
   float v10;
   
   input.seekg(offset00,ios::beg);
   input.read((char*)&v00, 4);
   
   input.seekg(offset01,ios::beg);
   input.read((char*)&v01, 4);
   
   input.seekg(offset11,ios::beg);
   input.read((char*)&v11, 4);

   input.seekg(offset10,ios::beg);
   input.read((char*)&v10, 4);
   rspfEndian endian;
   if(endian.getSystemEndianType() != theByteOrder)
   {
      endian.swap(v00);
      endian.swap(v01);
      endian.swap(v11);
      endian.swap(v10);
   }
   double bottom = (v00 + (v01 - v00)*tLat);
   double top    = (v10 + (v11 - v10)*tLat);
   result        = bottom + (top - bottom)*tLon;

   return result;
   
}