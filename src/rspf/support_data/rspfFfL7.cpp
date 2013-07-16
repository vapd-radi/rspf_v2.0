//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
//         Orginally written by Oscar Kramer (SoBe Software)
//         
// Description: Container class for LandSat7 Fast Format header files.
//
//********************************************************************
// $Id: rspfFfL7.cpp 17501 2010-06-02 11:14:55Z dburken $

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>  
using namespace std;

#include <rspf/support_data/rspfFfL7.h>

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDate.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfDateProperty.h>
#include <rspf/base/rspfStringProperty.h>

static const rspfString ACQUISITION_DATE_KW = "acquisition_date";
static const rspfString PATH_KW             = "path";
static const rspfString PATH_ROW_KW         = "path_row";
static const rspfString ROW_KW              = "row";
static const rspfString SATELLITE_NAME_KW   = "satellite_name";
static const rspfString SENSOR_NAME_KW      = "sensor_name";


rspfFfL7::rspfFfL7()
   :
      rspfErrorStatusInterface()
{
   initialize();
}

rspfFfL7::rspfFfL7(const char* headerFile)
   :
      rspfErrorStatusInterface()
{
   initialize();
   
   // open the header file:
   FILE* fptr = fopen(headerFile, "r");
   if (!fptr)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }

   // Begin reading records:
   if (!theErrorStatus)
      readAdminRecord(fptr);
   if (!theErrorStatus)
      readRadiomRecord(fptr);
   if (!theErrorStatus)
      readGeomRecord(fptr);
   if(fptr)
   {
      fclose(fptr);
   }
}
rspfFfL7::~rspfFfL7()
{}
 
rspfString rspfFfL7::pathRow() const
{
   rspfString tmp = "p";
   tmp += theLocationCode;
   tmp.gsub("/", "r");
   tmp.gsub(" ", "");

   return tmp;
}

void rspfFfL7::dump(std::ostream& os) const
{
   os << setiosflags(ios::left | ios::fixed)
      << "\nAdministrative Record:"
      << setw(30) << "\ntheRequestNumber: " 
      << theRequestNumber;

   os << setw(30) << "\nLocationCode:     " << theLocationCode
      << setw(30) << "\nAcquisitionDate:  " << theAcquisitionDate
      << setw(30) << "\nSatelliteName:    " << theSatName
      << setw(30) << "\nSensorName:       " << theSensorName
      << setw(30) << "\nSensorMode:       " << theSensorMode
      << setw(30) << "\nOffNadirAngle:    "
      << setprecision(2) << theOffNadirAngle;

   os << setw(30) << "\nProductType: " 
      << theProductType
      << setw(30) << "\nProductSize: " 
      << theProductSize
      << setw(30) << "\nProcessingType: " 
      << theProcessingType 
      << setw(30) << "\nResampAlgorithm: " 
      << theResampAlgorithm
      << setw(30) << "\nPixelsPerLine: " 
      << thePixelsPerLine
      << setw(30) << "\nLinesPerImage: " 
      << theLinesPerBand
      << setw(30) << "\nRecordSize: " 
      << theRecordSize
      << setw(30) << "\nGsd: " 
      << setprecision(2) << theGsd
      << setw(30) << "\nOutputBitsPerPixel: " 
      << theOutputBitsPerPixel
      << setw(30) << "\nAcquiredBitsPerPixel: " 
      << theAcquiredBitsPerPixel
      << setw(30) << "\nBandsPresentString: " 
      << theBandsPresentString;
   
   for (int i=0; i<6; i++)
      os << "\nBandFileNames["<<i<<"]:             " <<theBandFileNames[i];
   os << endl;

   os << setprecision(15) << setw(30) << "\nRadiometric record:" << endl;
   rspf_uint32 band = 0;
   vector<rspf_float64>::const_iterator bi = theBias.begin();
   while (bi != theBias.end())
   {
      os << "band[" << band << "] bias:  " << (*bi) << endl;
      ++bi;
      ++band;
   }

   band = 0;
   bi = theGain.begin();
   while (bi != theGain.end())
   {
      os << "band[" << band << "] gain:  " << (*bi) << endl;
      ++bi;
      ++band;
   }
   
   os << endl
      << "Geometric Record:"
      << setw(30) << "\nMapProjectionName:" 
      << theMapProjectionName
      << setw(30) << "\nEllipsoid:" 
      << theEllipsoid
      << setw(30) << "\nDatum:" 
      << theDatum;

   for (int i = 0; i < 15; i++)
   {
      os << "\nProjectionParams[" << setw(2) << i << setw(10) << "]:  "
         << theProjectionParams[i];
   }

   os << setprecision(3) 
      << setw(30) << "\nUL Corner:" << theUL_Corner
      << setw(30) << "\nUR Corner:" << theUR_Corner
      << setw(30) << "\nLR Corner:" << theLR_Corner
      << setw(30) << "\nLL Corner:" << theLL_Corner
      << setw(30) << "\nCenter Point (Ground):" << theCenterGP
      << setw(30) << "\nCenter Point (Image):" << theCenterImagePoint
      << setw(30) << "\nHorizontalOffset:" 
      << theHorizontalOffset
      << setprecision(2) << setw(30) << "\nOrientationAngle:" 
      << theOrientationAngle
      << setw(30) << "\nSunElevationAngle:" 
      << setprecision(1) << theSunElevation
      << setw(30) << "\nSunAzimuth:" <<  theSunAzimuth
      << resetiosflags(ios::left)
      << endl << endl;
}

void rspfFfL7::writeHeader(std::ostream& os) const
{
   rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfFfL7::writeHeader: This method is not yet implemented. Ignoring request..."
                                      << std::endl;
   os << "WARNING rspfFfL7::writeHeader: This method is not yet implemented. Ignoring request..."
      << std::endl;
}

void rspfFfL7::readAdminRecord(FILE* fptr)
{
   static const int OFFSET = -1;
   int converted;
   theErrorStatus = rspfErrorCodes::RSPF_ERROR; // assume error, prove not at end;

   fseek(fptr, OFFSET + 9, SEEK_SET);
   converted = fscanf(fptr, "%20c", theRequestNumber);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 35, SEEK_SET);
   converted = fscanf(fptr, "%17c", theLocationCode);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 71, SEEK_SET);
   converted = fscanf(fptr, "%8c", theAcquisitionDate);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 92, SEEK_SET);
   converted = fscanf(fptr, "%10c", theSatName);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 111, SEEK_SET);
   converted = fscanf(fptr, "%10c", theSensorName);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 135, SEEK_SET);
   converted = fscanf(fptr, "%6c", theSensorMode);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 154, SEEK_SET);
   converted = fscanf(fptr, "%6lf", &theOffNadirAngle);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 655, SEEK_SET);
   converted = fscanf(fptr, "%18c", theProductType);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 688, SEEK_SET);
   converted = fscanf(fptr, "%10c", theProductSize);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 741, SEEK_SET);
   converted = fscanf(fptr, "%11c", theProcessingType);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 765, SEEK_SET);
   converted = fscanf(fptr, "%2c", theResampAlgorithm);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 843, SEEK_SET);
   converted = fscanf(fptr, "%5d", &thePixelsPerLine);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 865, SEEK_SET);
   converted = fscanf(fptr, "%5d", &theLinesPerBand);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 932, SEEK_SET);
   converted = fscanf(fptr, "%9d", &theRecordSize);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 954, SEEK_SET);
   converted = fscanf(fptr, "%6lf", &theGsd);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 984, SEEK_SET);
   converted = fscanf(fptr, "%2d", &theOutputBitsPerPixel);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1012, SEEK_SET);
   converted = fscanf(fptr, "%2d", &theAcquiredBitsPerPixel);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1056, SEEK_SET);
   converted = fscanf(fptr, "%32c", theBandsPresentString);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1131, SEEK_SET);
   converted = fscanf(fptr, "%29c", theBandFileNames[0]);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1170, SEEK_SET);
   converted = fscanf(fptr, "%29c", theBandFileNames[1]);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1211, SEEK_SET);
   converted = fscanf(fptr, "%29c", theBandFileNames[2]);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1250, SEEK_SET);
   converted = fscanf(fptr, "%29c", theBandFileNames[3]);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1291, SEEK_SET);
   converted = fscanf(fptr, "%29c", theBandFileNames[4]);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 1330, SEEK_SET);
   converted = fscanf(fptr, "%29c", theBandFileNames[5]);
   if (converted != 1) return;

   std::vector<rspfString> arrayPathRow = rspfString(theLocationCode).split("/");
   if(arrayPathRow.size() == 2)
   {
      thePathNumber = arrayPathRow[0].toInt();
      theRowNumber = arrayPathRow[1].beforePos(3).toInt();
   }
   
   // The Admin record was successfully extracted:
   theErrorStatus = rspfErrorCodes::RSPF_OK;   
   return;
}

int rspfFfL7::path() const
{
   return thePathNumber;
}

int rspfFfL7::row() const
{
   return theRowNumber;
}

//***************************************************************************
// PRIVATE METHOD: rspfFfL7::readRadiomRecord()
//  
//***************************************************************************
void rspfFfL7::readRadiomRecord(FILE* fptr)
{
   int converted;
   char radiom_record[1537];
   
   if (!theErrorStatus)
   {
      fseek(fptr, 1536, SEEK_SET);
      converted = fscanf(fptr, "%1536c", radiom_record);
      if (converted != 1) theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }

   // Get the gains and biases.
   theBias.clear();
   theGain.clear();

   const rspf_uint32 MAX = 128;
   char temp_chars[MAX];
   std::istringstream is(radiom_record);

   // Eat the first line. "GAINS AND BIASES IN ASCENDING BAND NUMBER ORDER"
   is.getline(temp_chars, MAX);
   
   const char SPACE     = 0x20;
   const char LINE_FEED = 0x0a;
   
   // LS7 header has room for eight bands per spec.   
   for (rspf_uint32 band = 0; band < 8; ++band)
   {
      is.getline(temp_chars, MAX);
      bool blank_line = true;
      for (rspf_uint32 i = 0; i < MAX; ++i)
      {
         if (temp_chars[i] == LINE_FEED) break;  // end of line...

         if (temp_chars[i] != SPACE)
         {
            blank_line = false; // has bias and gain...
            break;
         }
      }

      if (blank_line == true) continue; // go to next line...
      
      std::vector<rspfString> splitString;
      rspfString tempString(temp_chars);
      tempString.split(splitString, " ");
      
      if(splitString.size() > 1)
      {
         theBias.push_back(splitString[0].toDouble());
         theGain.push_back(splitString[1].toDouble());
     }
   }
}

//***************************************************************************
// PRIVATE METHOD: rspfFfL7::readGeomRecord()
//  
//***************************************************************************
void rspfFfL7::readGeomRecord(FILE* fptr)
{
   static const int OFFSET = 3071;
   int converted;
   theErrorStatus = rspfErrorCodes::RSPF_ERROR; // assume error, prove not at end;

   fseek(fptr, OFFSET + 32, SEEK_SET);
   converted = fscanf(fptr, "%4c", theMapProjectionName);
   if (converted != 1) return;

   fseek(fptr, OFFSET + 48, SEEK_SET);
   converted = fscanf(fptr, "%18c", theEllipsoid);
   if (converted != 1) return;
      
   strcpy(theDatum, "WGS-84");

   // added to read in all 15 parameters instead of the first two.
   long currentOffset = OFFSET + 110;
   for(long parameter=0;parameter < 15; ++parameter)
   {
      fseek(fptr, currentOffset, SEEK_SET);
      converted = fscanf(fptr, "%24lf", &theProjectionParams[parameter]);
      if (converted != 1) return;

      currentOffset +=25;
   }
      
   
   fseek(fptr, OFFSET + 521, SEEK_SET);
   converted = fscanf(fptr, "%6d", &theUsgsMapZone);
   if (converted != 1) return;
      
   fseek(fptr, OFFSET + 566, SEEK_SET);
   if(convertGeoPoint(fptr, theUL_Corner)) return;
      
   fseek(fptr, OFFSET + 646, SEEK_SET);
   if(convertGeoPoint(fptr, theUR_Corner)) return;
      
   fseek(fptr, OFFSET + 726, SEEK_SET);
   if(convertGeoPoint(fptr, theLR_Corner)) return;
      
   fseek(fptr, OFFSET + 806, SEEK_SET);
   if(convertGeoPoint(fptr, theLL_Corner)) return;
      
   fseek(fptr, OFFSET + 890, SEEK_SET);
   if(convertGeoPoint(fptr, theCenterGP)) return;
      
   fseek(fptr, OFFSET + 945, SEEK_SET);

   int x;
   int y;
   
   converted = fscanf(fptr, "%5d %5d", &x, &y);
   if (converted != 2) return;
   
   theCenterImagePoint.x = x;
   theCenterImagePoint.y = y;
      
   fseek(fptr, OFFSET + 969, SEEK_SET);
   converted = fscanf(fptr, "%6d", &theHorizontalOffset);
   if (converted != 1) return;
      
   fseek(fptr, OFFSET + 995, SEEK_SET);
   converted = fscanf(fptr, "%6lf", &theOrientationAngle);
   if (converted != 1) return;
      
   fseek(fptr, OFFSET + 1062, SEEK_SET);
   converted = fscanf(fptr, "%4lf", &theSunElevation);
   if (converted != 1) return;
      
   fseek(fptr, OFFSET + 1086, SEEK_SET);
   converted = fscanf(fptr, "%5lf", &theSunAzimuth);
   if (converted != 1) return;

   // successfully reached the end of Geom record:
   theErrorStatus = rspfErrorCodes::RSPF_OK;

   return;
}


//***************************************************************************
// PRIVATE METHOD: rspfFfL7::convertGeoPoint(char* buffer);
//  
//***************************************************************************
int rspfFfL7::convertGeoPoint(FILE* fptr, rspfGpt& geo_point)
{
   int    londeg;
   int    lonmin;
   double lonsec;
   char   lonhem;
   int    latdeg;
   int    latmin;
   double latsec;
   char   lathem;
   
   int converted = fscanf(fptr, "%03d%02d%7lf%c %02d%02d%7lf%c",
                          &londeg, &lonmin, &lonsec, &lonhem,
                          &latdeg, &latmin, &latsec, &lathem);

   if (converted != 8)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   geo_point.lat = (double)latdeg + (double)latmin/60.0 + latsec/3600.0;
   if (lathem == 'S')
      geo_point.lat *= -1.0;
   
   geo_point.lon= (double)londeg + (double)lonmin/60.0 + lonsec/3600.0;
   if (lonhem == 'W')
      geo_point.lon *= -1.0;

   return rspfErrorCodes::RSPF_OK;
}
   
//***************************************************************************
// PRIVATE METHOD: rspfFfL7::initialize()
// Initializes all fields to blanks (or 0's) and null terminates strings.
//***************************************************************************
void rspfFfL7::initialize()
{
   static const char source[] = "";

   // The Admin Record:
   strncpy(theRequestNumber, source, 21);
   strncpy(theLocationCode, source, 18);
   thePathNumber = 0;
   theRowNumber = 0;
   strncpy(theAcquisitionDate, source, 20);
   strncpy(theSatName, source, 11);
   strncpy(theSensorName, source, 11);
   strncpy(theSensorMode, source, 7);
   theOffNadirAngle = 0.0;      
   strncpy(theProductType, source, 19);
   strncpy(theProductSize, source, 11);
   strncpy(theProcessingType, source, 12);
   strncpy(theResampAlgorithm, source, 3);
   thePixelsPerLine = 0;
   theLinesPerBand = 0;
   theRecordSize = 0;
   theGsd = 0.0;
   theOutputBitsPerPixel = 0;
   theAcquiredBitsPerPixel = 0;
   strncpy(theBandsPresentString, source, 33);
   strncpy(theBandFileNames[0], source, 30);
   strncpy(theBandFileNames[1], source, 30);
   strncpy(theBandFileNames[2], source, 30);
   strncpy(theBandFileNames[3], source, 30);
   strncpy(theBandFileNames[4], source, 30);
   strncpy(theBandFileNames[5], source, 30);

   // The Radiometry Record:
   theBias.clear();
   theGain.clear();

   // The Geometry Record:
   strncpy(theMapProjectionName, source, 5);
   strncpy(theEllipsoid, source, 19);
   strncpy(theDatum, source, 7);
   for (int i=0; i<15; i++)
      theProjectionParams[i] = 0.0;
   theUsgsMapZone = 0;

   theHorizontalOffset  = 0;   
   theOrientationAngle  = 0.0; 
   theSunElevation      = 0.0; 
   theSunAzimuth        = 0.0; 

   return;
}

void rspfFfL7::getBias(vector<double>& bias) const
{
   bias = theBias;
}

double rspfFfL7::getBias(long bandIdx)const
{
   return theBias[bandIdx];
}

void rspfFfL7::getGain(vector<double>& gain) const
{
   gain = theGain;
}

double rspfFfL7::getGain(long bandIdx)const
{
   return theGain[bandIdx];
}

double rspfFfL7::getParam(rspf_uint32 i)const
{
   if(i < 16)
   {
      return theProjectionParams[i];
   }
   return 0.0;
}

long rspfFfL7::getJulianDay()const
{
   rspfDate date;
   getAcquisitionDate(date);
   return static_cast<long>(date.getJulian());
}

rspfString rspfFfL7::getAcquisitionDate() const
{
   return rspfString(theAcquisitionDate);
}

void rspfFfL7::getAcquisitionDate(rspfDate& date)const
{
   rspfString y = rspfString(theAcquisitionDate,
                               theAcquisitionDate+4);
   rspfString m = rspfString(theAcquisitionDate+4,
                               theAcquisitionDate+6);
   rspfString d = rspfString(theAcquisitionDate+6,
                               theAcquisitionDate+8);

   date = rspfDate(m.toInt(),
                    d.toInt(),
                    y.toInt());
}

rspfString rspfFfL7::getSatelliteName() const
{
   return rspfString(theSatName);
}

rspfFilename rspfFfL7::getBandFilename(rspf_uint32 idx)const
{
   rspfFilename result;
   if (idx < getBandCount())
   {
      result = theBandFileNames[idx];
   }
   return result;
}

int rspfFfL7::getLinesPerBand() const
{
   return theLinesPerBand;
}

int rspfFfL7::getPixelsPerLine() const
{
   return thePixelsPerLine;
}

void rspfFfL7::getSunElevation(double& elevation) const
{
   elevation = theSunElevation;
}

void rspfFfL7::getSunAzimuth(double& azimuth) const
{
   azimuth = theSunAzimuth;
}

rspfRefPtr<rspfProperty> rspfFfL7::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;

   if(name == ACQUISITION_DATE_KW)
   {
      rspfDate date;
      getAcquisitionDate(date);
      result = new rspfDateProperty(ACQUISITION_DATE_KW, date);
   }
   else if (name == PATH_KW)
   {
      result = new rspfStringProperty(PATH_KW,
                                       rspfString::toString(path()),
                                       false);
   }
   else if (name == PATH_ROW_KW)
   {
      result = new rspfStringProperty(PATH_ROW_KW,
                                       pathRow(),
                                       false);
   }                                       
   else if (name == ROW_KW)
   {
      result = new rspfStringProperty(ROW_KW,
                                       rspfString::toString(row()),
                                       false);
   }
   else if (name == SATELLITE_NAME_KW)
   {
      result = new rspfStringProperty(SATELLITE_NAME_KW,
                                       rspfString(theSatName),
                                       false);
   }
   else if (name == SENSOR_NAME_KW)
   {
      result = new rspfStringProperty(SENSOR_NAME_KW,
                                       rspfString(theSensorName),
                                       false);
   }

   return result;
}

void rspfFfL7::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(ACQUISITION_DATE_KW);   
   propertyNames.push_back(PATH_KW);
   propertyNames.push_back(PATH_ROW_KW);
   propertyNames.push_back(ROW_KW);      
   propertyNames.push_back(SATELLITE_NAME_KW);
   propertyNames.push_back(SENSOR_NAME_KW);
}

unsigned int rspfFfL7::getBandCount()const
{
   rspfString tmp(theBandsPresentString);
   tmp.trim(); //remove spaces
   return (unsigned int)tmp.length();
//   return strlen(tmp.chars()); //beurk! should implement length in rspfString
}
 
//***************************************************************************
// Function:
// ostream& operator<<(ostream& os, const rspfFfL7& head)
//***************************************************************************
std::ostream& operator<<(std::ostream& os, const rspfFfL7& head)
{
   head.dump(os);
   return os;
}

