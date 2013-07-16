//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Class for reading and writing an ENVI (The Environment for Visualizing
// Images) header file.
//
//----------------------------------------------------------------------------
// $Id: rspfEnviHeader.cpp 21527 2012-08-26 16:50:49Z dburken $

#include <rspf/support_data/rspfEnviHeader.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <algorithm>
#include <fstream>
#include <string>

typedef  std::unary_function<std::pair<rspfString, rspfString>, bool> KwlCompareFunctionType;
typedef  std::pair<const rspfString, rspfString> KwlComparePairType;

class KwlKeyCaseInsensitiveEquals : public KwlCompareFunctionType
{
public:
   KwlKeyCaseInsensitiveEquals(const rspfString& key):m_key(key){}
   virtual bool operator()(const KwlComparePairType& rhs)const
   {
      return (m_key == rhs.first.downcase());
   }
   rspfString m_key;
};
class KwlKeySubStringCaseInsensitive : public KwlCompareFunctionType
{
public:
   KwlKeySubStringCaseInsensitive(const rspfString& key):m_key(key){}
   virtual bool operator()(const KwlComparePairType& rhs)const
   {
      return (rhs.first.downcase().contains(m_key));
   }
   rspfString m_key;
};

// Define keywords not already found in rspfKeywordNames.
static const char HEADER_OFFSET_KW[]    = "header_offset";
static const char FILE_TYPE_KW[]        = "file_type";
static const char DATA_TYPE_KW[]        = "data_type";
static const char SENSOR_TYPE_KW[]      = "sensor_type";
static const char X_START_KW[]          = "x_start";
static const char Y_START_KW[]          = "y_start";
static const char MAP_INFO_KW[]         = "map_info";
static const char WAVELENGTH_UNITS_KW[] = "wavelength_units";
static const char BAND_NAME_KW[]        = "band_name";
static const char WAVELENGTH_KW[]       = "wavelength";

static rspfTrace traceDebug(rspfString("rspfEnviHeader:debug"));

rspfEnviHeader::rspfEnviHeader()
   :
   m_file(),
   m_keywords()
{
   // Start the byte order of with the machine byte order.
   m_keywords[FILE_TYPE_KW] = "ENVI Standard";
   setByteorder(rspf::byteOrder());   
}

rspfEnviHeader::~rspfEnviHeader()
{
}

void rspfEnviHeader::reset()
{
   m_file.string().clear();
   m_keywords.clear();
   m_keywords[FILE_TYPE_KW] = "ENVI Standard";
   setByteorder(rspf::byteOrder());
}

const rspfKeywordlist& rspfEnviHeader::getMap() const
{
   return m_keywords;
}

rspfKeywordlist& rspfEnviHeader::getMap()
{
   return m_keywords;
}

bool rspfEnviHeader::getValue( const rspfString& key, rspfString& value ) const
{
   bool result = false;
   value.string() = m_keywords.findKey( key.string());
   if( value.size() )
   {
      result = true;
   }
   else
   {
      result = m_keywords.hasKey( key.string() );
   }
   return result;
}

bool rspfEnviHeader::findCaseInsensitive(const rspfString& key, rspfString& value) const
{
   return m_keywords.findValue<KwlKeyCaseInsensitiveEquals>(
      value, KwlKeyCaseInsensitiveEquals(key));
}

bool rspfEnviHeader::findSubStringCaseInsensitive(const rspfString& key,
                                                   rspfString& value) const
{
   return m_keywords.findValue<KwlKeySubStringCaseInsensitive>(
      value, KwlKeySubStringCaseInsensitive(key));
}

bool rspfEnviHeader::open(const rspfFilename& file)
{
   bool result = false;

   reset(); // Clear the map, file name.

   std::ifstream in;
   in.open(file.c_str(), std::ios::in | std::ios::binary);
   if( in.is_open() )
   {
      result = readStream( in );
      if ( result )
      {
         m_file = file;
      }
   }

   return result;
}

bool rspfEnviHeader::readStream(std::istream& in)
{
   reset();
   bool result = isEnviHeader( in );
   if ( result )
   {
      m_keywords.clear();
      
      while(!in.eof()&&in.good())
      {
         // read name
         rspfString name = "";
         rspfString value = "";
         rspf::skipws(in);
         int c = static_cast<char>(in.get());
         while((c != '=')&&(in.good())&&!in.eof())
         {
            name +=static_cast<char>(c);
            c = in.get();
         }
         rspf::skipws(in);
         c = in.get();
         
         if(in.good()&&!in.eof())
         {
            if(c == '{') // continue til '}'
            {
               c = in.get();
               while((c != '}')&&(in.good())&&!in.eof())
               {
                  value +=static_cast<char>(c);
                  c = in.get();
               }
            }
            else
            {
               while(((c != '\n')&&(c!='\r'))&&(in.good())&&!in.eof())
               {
                  value +=static_cast<char>(c);
                  c = (in.get());
               }
               
            }
            m_keywords.add(name.trim(), value);
         }
      }

      // Test for minimum set of keywords needed.
      if (m_keywords["samples"].empty() || m_keywords["lines"].empty() || 
          m_keywords["bands"].empty())
      {
         result =  false;
      }
   }

   return result;
}

bool rspfEnviHeader::writeFile(const rspfFilename& file)
{
   if (m_keywords["description"].empty())
   {
      m_keywords["description"] = file.c_str();
   }
   
   std::ofstream out(file.c_str(), std::ios_base::out);

   if (!out)
   {
      return false;
   }
   
   print(out);
   out.close();
   return true;
}

std::ostream& rspfEnviHeader::print(std::ostream& out) const
{
   out << "ENVI" << "\n" << m_keywords << std::endl;
   return out;
}

std::ostream& operator<<(std::ostream& out, const rspfEnviHeader& obj)
{
   return obj.print( out );
}

rspfString rspfEnviHeader::getDescription() const
{
   return m_keywords["description"];
}

void rspfEnviHeader::setDescription(const rspfString& description)
{
   m_keywords["description"] = description.c_str();
}

rspf_uint32 rspfEnviHeader::getSamples() const
{
   rspf_uint32 result = 0;
   rspfString value = m_keywords[ std::string("samples") ];
   if( value.size() )
   {
      result = value.toUInt32();
   }
   return result;
}

void rspfEnviHeader::setSamples(rspf_uint32 samples)
{
   m_keywords[ std::string("samples") ] = rspfString::toString(samples).c_str();
}

rspf_uint32 rspfEnviHeader::getLines() const
{
   rspf_uint32 result = 0;
   rspfString value = m_keywords["lines"];
   if( value.size() )
   {
      result = value.toUInt32();
   }
   return result;
}

void rspfEnviHeader::setLines(rspf_uint32 lines)
{
   m_keywords["lines"] = rspfString::toString(lines).c_str();
}

rspf_uint32 rspfEnviHeader::getBands() const
{
   rspf_uint32 result = 0;
   rspfString value = m_keywords["bands"];
   if( value.size() )
   {
      result = value.toUInt32();
   }
   return result;
}

void rspfEnviHeader::setBands(rspf_uint32 bands)
{
   m_keywords["bands"] = rspfString::toString(bands).c_str();
}

rspf_uint32 rspfEnviHeader::getHeaderOffset() const
{
   rspf_uint32 result = 0;
   rspfString value = m_keywords["header offset"];
   if( value.size() )
   {
      result = value.toUInt32();
   }
   return result;
}

void rspfEnviHeader::setHeaderOffset(rspf_uint32 headerOffset)
{
   m_keywords["header offset"] = rspfString::toString(headerOffset).c_str();
}

rspfString rspfEnviHeader::getFileType() const
{
   return m_keywords[FILE_TYPE_KW];
}

void rspfEnviHeader::setFileType(const rspfString& fileType)
{
    m_keywords[FILE_TYPE_KW] = fileType.c_str();
}

rspf_uint32 rspfEnviHeader::getDataType() const
{
   return rspfString(m_keywords["data type"]).toUInt32();
}

rspfScalarType rspfEnviHeader::getOssimScalarType() const
{
   rspf_uint32 dataTypeInt = rspfString(m_keywords["data type"]).toUInt32();
   
   rspfScalarType result = RSPF_SCALAR_UNKNOWN;

   switch( dataTypeInt )
   {
      case 1:
         result = RSPF_UINT8;
         break;
      case 2:
         result = RSPF_SINT16;
         break;
      case 3:
         result = RSPF_SINT32;
         break;
      case 4:
         result = RSPF_FLOAT32;
         break;
      case 5:
         result = RSPF_FLOAT64;
         break;
      case 6:
         result = RSPF_CFLOAT32;
         break;
      case 9:
         result = RSPF_CFLOAT64;
         break;
      case 12:
         result = RSPF_UINT16;
         break;
      case 13:
         result = RSPF_UINT32;
         break;
      default:
         break;
   }

   return result;
}

void rspfEnviHeader::setDataType(rspfScalarType scalar)
{
   rspfString dataTypeString = "";
   switch( scalar )
   {
      case RSPF_UINT8:
         dataTypeString = "1";
         break;
      case RSPF_SINT16:
         dataTypeString = "2";
         break;
      case RSPF_SINT32:
         dataTypeString = "3";
         break;
      case RSPF_FLOAT32:
         dataTypeString = "4";
         break;
      case RSPF_FLOAT64:
         dataTypeString = "5";
         break;
      case RSPF_CFLOAT32:
         dataTypeString = "6";
         break;
      case RSPF_CFLOAT64:
         dataTypeString = "9";
         break;
      case RSPF_UINT16:
         dataTypeString = "12";
         break;
      case RSPF_UINT32:
         dataTypeString = "13";
         break;
      default:
         break;
   }
   
   if(!dataTypeString.empty())
   {
      m_keywords["data type"] = dataTypeString.c_str();
   }
}
rspfString rspfEnviHeader::getInterleaveType() const
{
   return m_keywords["interleave"];
}
rspfInterleaveType rspfEnviHeader::getOssimInterleaveType() const
{
   rspfString interleave = getInterleaveType();
   if (interleave == "bsq")
   {
      return RSPF_BSQ;
   }
   else if  (interleave == "bil")
   {
      return RSPF_BIL;
   }
   else if  (interleave == "bip")
   {
      return RSPF_BIP;
   }
   else
   {
      return RSPF_INTERLEAVE_UNKNOWN;
   }
}

void rspfEnviHeader::setInterleaveType(rspfInterleaveType interleave)
{
   std::string interleaveString = "";
   switch (interleave)
   {
      case RSPF_BIL:
         interleaveString = "bil";
         break;
      case RSPF_BSQ:
         interleaveString = "bsq";
         break;
      case RSPF_BIP:
         interleaveString = "bip";
         break;
      default:
         interleaveString = "Unknown";
         break;
   }
   m_keywords["interleave"] = interleaveString;
}

rspfString rspfEnviHeader::getSensorType() const
{
   return m_keywords["sensor type"];
}

void rspfEnviHeader::setSensorType(const rspfString& sensorType)
{
   m_keywords["sensor type"] = sensorType.c_str();
}

rspfByteOrder rspfEnviHeader::getByteOrder() const
{
   rspfByteOrder result = rspf::byteOrder(); // System byte order.
   std::string value = m_keywords["byte order"];
   if ( value.size() )
   {
      // 0 = LITTLE_ENDIAN
      // 1 = BIG_ENDIAN
      rspf_int32 order = rspfString(value).toInt32();
      result = order ? RSPF_BIG_ENDIAN : RSPF_LITTLE_ENDIAN;
   }
   return result;
}

void rspfEnviHeader::setByteorder(rspfByteOrder byteOrder)
{
   if(byteOrder==RSPF_LITTLE_ENDIAN)
   {
      m_keywords["byte order"] = "0";
   }
   else
   {
      m_keywords["byte order"] = "1";
   }
}

rspf_int32 rspfEnviHeader::getXStart() const
{
   rspf_int32 result = 0;
   std::string s = m_keywords.findKey( std::string("x start") );
   if ( !s.size() )
   {
      s = m_keywords.findKey( std::string("sample start") );
   }
   if ( s.size() )
   {
      result = rspfString( s ).toInt32();
   }
   return result;
}

void rspfEnviHeader::setXStart(rspf_int32 xstart)
{
   m_keywords["x start"] = rspfString::toString(xstart).c_str();
}

rspf_int32 rspfEnviHeader::getYStart() const
{
   rspf_int32 result = 0;
   std::string s = m_keywords.findKey( std::string("y start") );
   if ( !s.size() )
   {
      s = m_keywords.findKey( std::string("line start") );
   }
   if ( s.size() )
   {
      result = rspfString( s ).toInt32();
   }
   return result;
}

void rspfEnviHeader::setYStart(rspf_int32 ystart)
{
   m_keywords["y start"] = rspfString::toString(ystart).c_str();
}

rspfString rspfEnviHeader::getMapInfo() const
{
   return m_keywords["map info"];
}

void rspfEnviHeader::setMapInfo(const rspfString& mapInfo)
{
   m_keywords["map info"] = mapInfo.c_str();
}

void rspfEnviHeader::setMapInfo(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfEnviHeader::setMapInfo DEBUG:"
         << "\nkwl:\n"
         << kwl
         << std::endl;
   }
   rspfString mapInfoString;
   const char* lookup;
   
   // Get the projection type.
   rspfString projection;
   lookup = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if (lookup)
   {
      projection = lookup;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfEnviHeader::setMapInfo WARNING:"
         << "\nNo projection type found!\nReturning..."
         << std::endl;
         
      return; // Have to have the projection type!
   }

   // Get the datum.
   rspfString datum = "WGS-84";
   lookup = kwl.find(prefix, rspfKeywordNames::DATUM_KW);
   if (lookup)
   {
      rspfString os = lookup;
      if (os == "WGE")
      {
         datum = "WGS-84";
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfEnviHeader::setMapInfo WARNING:"
            << "\nUnhandled rspf -> envi datum:  " << datum
            << "\nAssuming WGS-84!"
            << std::endl;
      }
   }

   if ( (projection == "rspfEquDistCylProjection") ||
        (projection == "rspfLlxyProjection") )
   {
      const char* tieLat = NULL;
      const char* tieLon = NULL;
      const char* degLat = NULL;
      const char* degLon = NULL;
      tieLat = kwl.find(prefix, rspfKeywordNames::TIE_POINT_LAT_KW);
      tieLon = kwl.find(prefix, rspfKeywordNames::TIE_POINT_LON_KW);
      degLat = kwl.find(prefix,
                        rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT);
      degLon = kwl.find(prefix,
                       rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON);
      
      if (!tieLat || !tieLon || !degLat || !degLon)
      {
         return;
      }
                 
      mapInfoString = "{Geographic Lat/Lon, 1.0000, 1.0000, ";
      mapInfoString += tieLon;
      mapInfoString += ", ";
      mapInfoString += tieLat;
      mapInfoString += ", ";
      mapInfoString += degLon;
      mapInfoString += ", ";
      mapInfoString += degLat;
      mapInfoString += ", ";
      mapInfoString += datum;
      mapInfoString += ", units=degrees}";
   }

   m_keywords["map info"] = mapInfoString.c_str();
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfEnviHeader::setMapInfo DEBUG:"
         << "\ntheMapInfo:  " << mapInfoString
         << std::endl;
   }
}

rspfString rspfEnviHeader::getWavelengthUnits() const
{
   return m_keywords["wavelength units"];
}

void rspfEnviHeader::setWavelengthUnits(const rspfString& waveLengthUnits)
{
   
   m_keywords["wavelength units"] = waveLengthUnits.c_str();
}

void rspfEnviHeader::getBandNames(std::vector<rspfString>& bandNames) const
{
   bandNames.clear();
   rspfString bandNamesString = m_keywords["band names"];
   bandNamesString.split(bandNames, ",");
}

void rspfEnviHeader::setBandNames(const std::vector<rspfString>& bandNames)
{
   rspfString value;
   value.join(bandNames, ",");
   m_keywords["band names"] = value.c_str();
}

void rspfEnviHeader::getWavelengths(std::vector<rspfString>& waveLengths)
   const
{
   waveLengths.clear();
   rspfString bandNamesString = m_keywords[WAVELENGTH_KW];
   bandNamesString.split(waveLengths, ",");
}

void rspfEnviHeader::setWavelengths(
   const std::vector<rspfString>& wavelengths)
{
   rspfString value;
   value.join(wavelengths, ",");
   m_keywords[WAVELENGTH_KW] = value.c_str();
}

bool rspfEnviHeader::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   std::string lookup;
   std::string pfx = (prefix ? prefix: "" );
   rspfString s;

   reset();

   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::FILENAME_KW));
   if (lookup.size())
   {
      m_file.string() = lookup;
   }
  
   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::DESCRIPTION_KW));
   if (lookup.size())
   {
      setDescription(lookup);
   }

   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::NUMBER_SAMPLES_KW));
   if (lookup.size())
   {
      m_keywords["samples"] = lookup;
   }
   
   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::NUMBER_LINES_KW));
   if (lookup.size())
   {
      m_keywords["lines"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::NUMBER_BANDS_KW));
   if (lookup.size())
   {
      m_keywords["bands"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(HEADER_OFFSET_KW));
   if (lookup.size())
   {
      m_keywords["header offset"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(FILE_TYPE_KW));
   if (lookup.size())
   {
       m_keywords["file_type"] = lookup;
   }

   lookup = kwl.findKey(pfx, DATA_TYPE_KW);
   if (lookup.size())
   {
      m_keywords["data type"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::INTERLEAVE_TYPE_KW));
   if (lookup.size())
   {
      m_keywords["interleave"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(SENSOR_TYPE_KW));
   if (lookup.size())
   {
      m_keywords["sensor type"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(rspfKeywordNames::BYTE_ORDER_KW));
   if (lookup.size())
   {
      s = lookup;
      s.downcase();
      if (s == "little_endian")
      {
         m_keywords["byte order"] = "0";
      }
      else if (s == "big_endian")
      {
         m_keywords["byte order"] = "1";
      }
      else
      {
         m_keywords["byte order"] = lookup;
      }
   }
   
   lookup = kwl.findKey(pfx, std::string(X_START_KW));
   if (lookup.size())
   {
      m_keywords["x start"] = lookup;
   }
   lookup = kwl.findKey(pfx, std::string(Y_START_KW));
   if (lookup.size())
   {
      m_keywords["y start"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(MAP_INFO_KW));
   if (lookup.size())
   {
      m_keywords["map info"] = lookup;
   }

   lookup = kwl.findKey(pfx, std::string(WAVELENGTH_UNITS_KW));
   if (lookup.size())
   {
      m_keywords["wavelength units"] = lookup;
   }

   rspf_uint32 n;
   rspf_uint32 count;
   const rspf_uint32 MAX_TRIES = 1024; // Avoid infinate loop.
   
   // Get the band names.
   n = kwl.numberOf(prefix, BAND_NAME_KW);
   if (n)
   {
      rspfString value = "";
      count = 0;
      while ( (count < n) || (count > MAX_TRIES) )
      {
         s = BAND_NAME_KW;
         s += rspfString::toString(count);
         lookup = kwl.findKey(pfx, s.string());
         if (lookup.size())
         {
            if(!value.empty())
            {
               value += rspfString(lookup);
            }
            else
            {
               value += (", " + rspfString(lookup));

            }
            
         }
         ++count;
      }
      m_keywords["band names"] = value.c_str();
   }
            
   // Get the band names.
   n = kwl.numberOf(prefix, WAVELENGTH_KW);
   if (n)
   {
      rspfString value;
      count = 0;
      while ( (count < n) || (count > MAX_TRIES) )
      {
         s = WAVELENGTH_KW;
         s += rspfString::toString(count);
         lookup = kwl.findKey(pfx, s.string());
         if (lookup.size())
         {
            if(!value.empty())
            {
               value += rspfString(lookup);
            }
            else
            {
               value += (", " + rspfString(lookup));
            }
         }
         ++count;
      }
      m_keywords["wavelength"] = value.c_str();
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfEnviHeader::loadState DEUG\n";
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   
   return true;
}

bool rspfEnviHeader::isEnviHeader( const rspfFilename& file )
{
   bool result = false;
   std::ifstream in;
   in.open(file.c_str(), std::ios::in | std::ios::binary);
   if ( in.is_open() )
   {
      result = isEnviHeader( in );
      in.close();
   }
   return result;
}

bool rspfEnviHeader::isEnviHeader( std::istream& in )
{
   bool result = false;
   rspf::skipws(in);
   char eh[5];
   in.read(eh, 4);
   eh[4] = '\0';
   std::string s(eh);
   if ( s == "ENVI" )
   {
      result = true;
   }
   return result;
}

const rspfFilename& rspfEnviHeader::getFile() const
{
   return m_file;
}
