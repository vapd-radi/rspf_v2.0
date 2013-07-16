//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Chong-Ket Chuah
// Contributor: Anrew Huang
// 
// Description:
// Implementation of rspfERS class for parsing a ER Mapper raster
// file format header.
//
//********************************************************************
// $Id: rspfERS.cpp 17501 2010-06-02 11:14:55Z dburken $

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/support_data/rspfERS.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>

static rspfTrace traceDebug("rspfERS:degug");


rspfERS::rspfERS()
   :
      rspfErrorStatusInterface()
{
   clearFields();
}

rspfERS::rspfERS(const char* headerFile)
   :
      rspfErrorStatusInterface()
{
   clearFields();

   // open the header file:
   std::ifstream in;
   in.open(headerFile, std::ios::in | std::ios::binary);

   if (!in)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }

   // Begin parsing records:
   if (!theErrorStatus)
   {
      parseHeader(in);
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfERS DEBUG:"
         << *this
         << std::endl;
   }
}

void rspfERS::parseError(const char* /* msg*/ )
{
   theErrorStatus = rspfErrorCodes::RSPF_ERROR;
}

void rspfERS::parseRasterInfo(std::istream& in)
{
   std::vector<rspfString> tokens;

   bool done = false;
   while (!done)
   {
      tokens = parseLine(in);
      if (tokens.empty())
      {
         parseError("premature end of header file.");
         return;
      }
      if (tokens[0] == "CellType")
      {
         if (tokens[1] == "Unsigned8BitInteger")
         {
            theCelltype = RSPF_UCHAR;
         }
         else if (tokens[1] == "Unsigned16BitInteger")
         {
            theCelltype = RSPF_USHORT16;
         }
         else if (tokens[1] == "Signed16BitInteger")
         {
            theCelltype = RSPF_SSHORT16;
         }
         else if (tokens[1] == "IEEE4ByteReal")
         {
            theCelltype = RSPF_FLOAT;
         }
         else if (tokens[1] == "IEEE8ByteReal")
         {
            theCelltype = RSPF_DOUBLE;
         }
         else
         {
            parseError("Unsupported CellType.");
         }
      }
      else if (tokens[0] == "NullCellValue")
      {
         theHasNullCells = true;
         theNullCell = tokens[1].toInt();	 // float nullcell value?
      }
      else if (tokens[0] == "Xdimension")
      {	// ignore "CellInfo"
         theCellSizeX = tokens[1].toDouble();
      }
      else if (tokens[0] == "Ydimension")
      {	// ignore "CellInfo"
         theCellSizeY = tokens[1].toDouble();
      }
      else if (tokens[0] == "NrOfLines")
      {
         theLine = tokens[1].toInt();
      }
      else if (tokens[0] == "NrOfCellsPerLine")
      {
         theSample = tokens[1].toInt();
      }
      else if (tokens[0] == "Eastings")
      {	// ignore "RegistrationCoord"
         theTieUnitType = RSPF_METERS;
         theOriginX = tokens[1].toDouble();
      }
      else if (tokens[0] == "Northings")
      {	// ignore "RegistrationCoord"
         theTieUnitType = RSPF_METERS;
         theOriginY = tokens[1].toDouble();
      }
      else if (tokens[0] == "Longitude")
      {	// ignore "RegistrationCoord"
                                                // need to check coordinatestype ("Cannot use Latitude with UTM projection")
         double deg, min, sec;
         sscanf (tokens[1].chars(), "%lg:%lg:%lg", &deg, &min, &sec);
         theOriginX = std::abs(deg) + min/60.0 + sec/3600.0;
         if (deg < 0.0)
         {
            theOriginX = -theOriginX;
         }
         theTieUnitType = RSPF_DEGREES;
      }
      else if (tokens[0] == "Latitude")
      {	// ignore "RegistrationCoord"
         double deg, min, sec;
         sscanf (tokens[1].chars(), "%lg:%lg:%lg", &deg, &min, &sec);
         theOriginY = std::abs(deg) + min/60.0 + sec/3600.0;
         if (deg < 0.0)
         {
            theOriginY = -theOriginY;
         }
         theTieUnitType = RSPF_DEGREES;
      }
      else if (tokens[0] == "NrOfBands")
      {
         theBands = tokens[1].toInt();
      }
      else if (tokens[0] == "Value")
      {	// ignore "BandID"
        // keep adding to the bandID vector
        // delete Value key then join the rest of tokens
         tokens.erase(tokens.begin());
         rspfString id;
         id.join(tokens, " ");
         theBandID.push_back(id);
      }
      else if (tokens[0] == "RasterInfo")
      {
         done = true;
      }
   }
}


void rspfERS::parseCoordinateSpace(std::istream& in)
{
   std::vector<rspfString> tokens;

   bool done = false;
   while (!done)
   {
      tokens = parseLine(in);
      if (tokens.empty())
      {
         parseError("premature end of header file.");
         return;
      }
      if (tokens[0] == "Datum")
      {
         theDatum = tokens[1];
         if (theDatum != "WGS84" && theDatum != "RAW")
         {
            parseError("Datum must be WGS84 or RAW");
         }
      }
      else if (tokens[0] == "Projection")
      {
         theProjection = tokens[1];
         // need to parse projection string
         /*(EqualStrings (tok(1,3), "UTM")) {
           coordinateSystem = 'U';
           if (tok(0) == 'N')
           northernHemisphere = 1;
           else
           northernHemisphere = 0;
           spString zoneStr = tok(4,2);
           sscanf (zoneStr.chars(), "%d", &zone);
         */
      }
      else if (tokens[0] == "CoordinateType")
      {
         // ignore it
      }
      else if (tokens[0] == "Units")
      {
         // ignore it for now
      }
      else if (tokens[0] == "Rotation")
      {
         double deg, min, sec;
         sscanf (tokens[1].chars(), "%lg:%lg:%lg", &deg, &min, &sec);
         theRotation = deg + min/60.0 + sec/3600.0;
      }
      else if (tokens[0] == "CoordinateSpace")
      {
         done = true;
      }
   }

   // requires more error checking on unit and rotation here
   return;
}

void rspfERS::parseHeader(std::istream& in)
{
   theErrorStatus = rspfErrorCodes::RSPF_OK;
   char magicNumberTest[14];
   in.read(magicNumberTest, 13);
   if(rspfString(magicNumberTest,
                  magicNumberTest+13) != "DatasetHeader")
   {
      parseError("First line must be DatasetHeader");
      return;
   }
   in.seekg(0);
   std::vector<rspfString> tokens = parseLine(in);
   if(!tokens.size())
   {
      parseError("First line must be DatasetHeader");
      return;
   }
   if (tokens[0] != "DatasetHeader")
   {
      parseError("First line must be DatasetHeader");
      return;
   }

   bool done = false;
   while (!done)
   {
      tokens = parseLine(in);
      if (tokens.empty())
      {
         parseError("premature end of header file.");
         return;
      }
      if (tokens[0] == "Version")
      {
         theVersion = tokens[1].toDouble();
         if (theVersion < 4.0)
         {
            parseError("Version must be 4.0 or greater");
         }
      }
      else if (tokens[0] == "DataSetType")
      {
         theDatasetType = tokens[1];
         if (theDatasetType != "ERStorage")
         {
            parseError("DataSetType must be ERStorage");
         }
      }
      else if (tokens[0] == "DataType")
      {
         theDatatype = tokens[1];
         if (theDatatype != "Raster")
         {
            parseError("DataType must be Raster");
         }
      }
      else if (tokens[0] == "ByteOrder")
      {
         theByteorder = tokens[1];
         if (theByteorder != "MSBFirst" && theByteorder != "LSBFirst")
         {
            parseError("ByteOrder must either be MSBFirst or LSBFirst");
         }
      }
      else if (tokens[0] == "CoordinateSpace")
      {
         if (tokens[1] == "Begin")
         {
            parseCoordinateSpace(in);
         }
      }
      else if (tokens[0] == "RasterInfo")
      {
         if (tokens[1] == "Begin")
         {
            parseRasterInfo(in);
         }
      }
      else if (tokens[0] == "SenseDate")
      {
         // ignore for now
      }
      else if (tokens[0] == "Comments")
      {
         // ignore for now
      }
      else if (tokens[0] == "DatasetHeader")
      {
         done = true;
      }
   }

   if(theProjection.contains("GEODETIC"))
   {
      theTieUnitType = RSPF_DEGREES;
   }

   return;
}


// read a line from the file, split it using "= \t\n" delimiters.
// if the first token is empty, if the line begins with a tab, delete
// the token.  The second token, the value for the key, may be quoted.
// trim the quotes.
std::vector<rspfString> rspfERS::parseLine(std::istream& in)
{
/*   const int bufSize = 500; */
   rspfString line;
   std::vector<rspfString> tokens;
   bool invalidCharHit = false;
   const int MAX_LENGTH = 10000;
   int tempCount = 0;
   // read a line, skipping empty line
   while (tokens.empty()&&(in)&&(tempCount < MAX_LENGTH))
   {
     tempCount = 1;
      char c = in.get();
      while( (c != '\n')&&
             (!in.eof())&&
             (!invalidCharHit))
      {
         if(c > 0x7e)
         {
            invalidCharHit = true;
         }
         else
         {
            line += (char)c;
            c = in.get();
	    ++tempCount;
         }
      }

      if(!invalidCharHit)
      {
         line.trim('\t');
         line.trim('\n');
         line.trim('\r');
         line.trim(' ');

         if(line != "")
         {
            tokens = line.split("= \t");
            if (tokens.size() > 1)
            {
               tokens[1].trim('\"');
               tokens.back().trim('\"');
            }
         }
      }
      else
      {
         tokens.clear();
         return tokens;
      }
   }
   if(in.bad()||(tempCount>=MAX_LENGTH))
     {
       tokens.clear();
     }

   return tokens;
}

bool rspfERS::writeFile(const rspfFilename &file)
{
   if (theDescription.empty())
   {
      theDescription = file;
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

std::ostream& rspfERS::print(std::ostream& out) const
{
   out<< std::setiosflags(std::ios::fixed) << std::setprecision(12);
   out<<"DatasetHeader Begin" <<std::endl;
   out<<"	Version		= \"5.5\"" <<std::endl;
   out<<"	DataSetType	= " <<theDatasetType <<std::endl;
   out<<"	DataType	= " <<theDatatype <<std::endl;
   out<<"	ByteOrder	= " <<theByteorder <<std::endl;
   
   out<<"	CoordinateSpace Begin" <<std::endl;
   out<<"		Datum		= \"" <<theDatum <<"\"" <<std::endl;
   out<<"		Projection	= \"" <<theProjection <<"\"" <<std::endl;
   out<<"		CoordinateType	= " <<theCoordSysType <<std::endl;
   out<<"	CoordinateSpace End" <<std::endl;
   
   rspfString celltype;
   if (theCelltype == RSPF_UINT8)
      celltype = "Unsigned8BitInteger";
   else if (theCelltype == RSPF_SINT8)
      celltype = "Signed8BitInteger";
   else if (theCelltype == RSPF_UINT16)
      celltype = "Unsigned16BitInteger";
   else if (theCelltype == RSPF_SINT16)
      celltype = "Signed16BitInteger";
   else if (theCelltype == RSPF_UINT32)
      celltype = "Unsigned32BitInteger";
   else if (theCelltype == RSPF_SINT32)
      celltype = "Signed32BitInteger";
   else if (theCelltype == RSPF_USHORT11)
      celltype = "Unsigned16BitInteger";
   else if (theCelltype == RSPF_USHORT16)
      celltype = "Unsigned16BitInteger";
   
   out<<"	RasterInfo Begin" <<std::endl;
   out<<"		CellType	= " <<celltype <<std::endl;
   
   out<<"		CellInfo Begin" <<std::endl;
   out<<"			Xdimension	= " <<theCellSizeX <<std::endl;
   out<<"			Ydimension	= " <<theCellSizeY <<std::endl;
   out<<"		CellInfo End" <<std::endl;
   
   out<<"		NrOfLines	= " <<theLine <<std::endl;
   out<<"		NrOfCellsPerLine	= " <<theSample <<std::endl;
   
   out<<"		RegistrationCoord Begin" <<std::endl;
   if (theTieUnitType == RSPF_METERS)
   {
      out<<"			Eastings	= " <<theOriginX <<std::endl;
      out<<"			Northings	= " <<theOriginY <<std::endl;
   }
   else if (theTieUnitType == RSPF_DEGREES)
   {
      bool minus = false;
      int min;
      double degrees, seconds;
      degrees = theOriginY;
      if (theOriginY < 0)
      {
         minus = true;
         degrees *=-1;
      }
      min = (int)((degrees-(int)degrees)*60);
      seconds =  ((degrees-(int)degrees)*60 - min)*60;
      if (minus)
         degrees *= -1;
      out<<"			Latitude	= "
         <<(int)degrees<<":"<<min<<":"<<seconds<<std::endl;
      
      minus = false;
      degrees = theOriginX;
      if (theOriginX < 0)
      {
         minus = true;
         degrees *=-1;
      }
      min = (int)((degrees-(int)degrees)*60);
      seconds =  ((degrees-(int)degrees)*60 - min)*60;
      if (minus)
         degrees *= -1;
      out<<"			Longitude	= "
         <<(int)degrees<<":"<<min<<":"<<seconds<<std::endl;
   }
   out<<"		RegistrationCoord End" <<std::endl;
   
   out<<"		NrOfBands	= " <<theBands <<std::endl;
   out<<"	RasterInfo End" <<std::endl;
   
   out<<"DatasetHeader End" <<std::endl;
   
   return out;
}

bool rspfERS::toOssimProjectionGeom(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   rspfString proj  = toOssimProjection();
   rspfString datum = toOssimDatum();

   if(proj == "")
   {
      return false;
   }

   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           proj.c_str(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::DATUM_KW,
           datum.c_str(),
           true);

   // if it's UTM we have to extract out zone and hemisphere
   // from the projection name stored by ERMapper.
   //
   if(proj == "rspfUtmProjection")
   {
      rspfString zone(theProjection.begin()+4,
                       theProjection.end());
      rspfString hemisphere(theProjection.begin(),
                             theProjection.begin()+1);
      kwl.add(prefix,
              rspfKeywordNames::ZONE_KW,
              zone.c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::HEMISPHERE_KW,
              hemisphere.c_str(),
              true);
   }

   if(theTieUnitType == RSPF_METERS)
   {
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_EASTING_KW,
              theOriginX + theCellSizeX/2,
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_NORTHING_KW,
              theOriginY - theCellSizeY/2,
              true);
      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_X_KW,
              theCellSizeX,
              true);
      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_Y_KW,
              theCellSizeY,
              true);

   }
   else if(theTieUnitType == RSPF_DEGREES)
   {
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_LAT_KW,
              theOriginY - theCellSizeY/2,
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_LON_KW,
              theOriginX + theCellSizeX/2,
              true);
      kwl.add(prefix,
              rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
              theCellSizeX,
              true);
      kwl.add(prefix,
              rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
              theCellSizeY,
              true);
   }

   return true;
}

rspfString rspfERS::toOssimProjection()const
{
   rspfString result = "";

   if(theProjection.contains("UTM"))
   {
      result = "rspfUtmProjection";
   }
   else if(theProjection.contains("GEODETIC"))
   {
      result = "rspfEquDistCylProjection";
   }

   return result;
}

rspfString rspfERS::toOssimDatum()const
{
   rspfString result = "WGE"; // wgs 84 default datum code

   if(theDatum != "WGS84")
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING Datum rspfERS::toOssimDatum: " << theDatum << " is not handled in rspfERS::toOssimDatum()\n"
                                            << "Please notify us with the datum name so we can add it\n";
   }

   return result;
}

//***************************************************************************
// PRIVATE METHOD: rspfERS::initialize()
// Initializes all fields to blanks (or 0's) and null terminates strings.
//***************************************************************************
void rspfERS::clearFields()
{
//   static const char source[] = "";

   theErrorStatus = rspfErrorCodes::RSPF_OK;
   theVersion       = 0;
   theFilename      = "";
   theDescription   = "";
   theSensorname    = "";
   theHeaderOffset = 0;
   theDatasetType   = "";
   theDatatype      = "";
   theByteorder     = "";
   theComments      = "";
   theDatum         = "";
   theProjection    = "";
   theCoordSysType  = "";
   theUnits         = "";
   theRotation      = 0.0;
   theCelltype      = RSPF_SCALAR_UNKNOWN;
   theCellsizeof    = 0;
   theHasNullCells  = false;
   theNullCell      = 0;
   theCellSizeX     = 0;
   theCellSizeY     = 0;
   theLine       = 0;
   theSample     = 0;
   theOriginX       = 0;
   theOriginY       = 0;
   theBands      = 0;
   theTieUnitType= RSPF_UNIT_UNKNOWN;
   theBandID.clear();

   return;
}