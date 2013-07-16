//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Ken Melero (kmelero@imagelinks.com)
//         Orginally written by Oscar Kramer (SoBe Software)
// Description: Container class for LandSat7 Fast Format header files.
//
// ********************************************************************
// $Id: rspfFfL5.cpp 17501 2010-06-02 11:14:55Z dburken $
//

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
using namespace std;

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDate.h>
#include <rspf/support_data/rspfFfL5.h>
#include <rspf/base/rspfNotifyContext.h>

rspfFfL5::rspfFfL5()
   : rspfFfL7()
{
}

rspfFfL5::rspfFfL5(const char* headerFile)
   : rspfFfL7()    
{   
   // open the header file:
   readHeaderRevB(headerFile);  
}

rspfFfL5::~rspfFfL5()
{
}


void rspfFfL5::readHeaderRevB(const rspfString& header_name)
{
   theErrorStatus = rspfErrorCodes::RSPF_ERROR; // assume error, prove not at end;
   
   theRevb = new rspfFfRevb(header_name);
   
   if (theRevb->errorStatus() != rspfFfRevb::RSPF_OK) return;
   
   strcpy(theRequestNumber,   theRevb->theProductOrderNumber);
   strcpy(theLocationCode,    theRevb->thePathRowNumber);
   strcpy(theAcquisitionDate, theRevb->theAcquisitionDate);
   strcpy(theSatName,         theRevb->theSatNumber);
   strcpy(theSensorName,      theRevb->theInstrumentType);
   strcpy(theSensorMode,      "");
   theOffNadirAngle =         theRevb->theOrientationAngle;
   strcpy(theProductType,     theRevb->theProductType);
   strcpy(theProductSize,     theRevb->theProductSize);
   strcpy(theProcessingType,  theRevb->theProcessingType);
   strcpy(theResampAlgorithm, theRevb->theResampAlgorithm);
   thePixelsPerLine         = theRevb->thePixelsPerLine;
   theLinesPerBand          = theRevb->theLinesPerImage;
   theRecordSize            = theRevb->theRecordSize;
   theGsd                   = theRevb->theGsd;
   theOutputBitsPerPixel    = 8;
   theAcquiredBitsPerPixel  = 8;
   strcpy(theBandsPresentString, theRevb->theBandsPresentString);
   
   //generate band names   
   int i=0;
   while((theBandsPresentString[i]>='0') && (theBandsPresentString[i]<='9'))
   {
      sprintf(theBandFileNames[i],"band%c.dat",theBandsPresentString[i]);
      ++i;
   }
   int nbb=i;

   std::vector<rspfString> arrayPathRow = rspfString(theLocationCode).split("/");
   if(arrayPathRow.size() == 2)
   {
      thePathNumber = arrayPathRow[0].toInt();
      theRowNumber = arrayPathRow[1].beforePos(3).toInt();
   }
   
   //extract gains/biases from max/min radiance
   double min,max;
   theBias.clear();
   theGain.clear();
   for(i=0;i<nbb;++i)
   {
      if (sscanf( theRevb->theBandRadiance[i]," %lf/%lf",&max,&min) != 2) return;
      theBias.push_back(min);
      theGain.push_back(max/254.0-min/255.0);
   }
   
   strcpy(theMapProjectionName      ,theRevb->theMapProjName);
   strcpy(theEllipsoid              ,theRevb->theEllipsoid); 
   strcpy(theDatum, "WGS-84");

   // added to read in all 15 parameters instead of the first two.
   
   for(long parameter=0;parameter < 15; ++parameter)
   {
      rspfString temp(theRevb->theUsgsProjParam[parameter]);
      theProjectionParams[parameter] = temp.gsub('D','E').toDouble();
   }
   
   theUsgsMapZone            = theRevb->theUsgsMapZone;
   
   char temps[256];
   sprintf(temps,"%s %s",theRevb->theUlLon, theRevb->theUlLat);
   if (convertGeoPoint(temps, theUL_Corner) != rspfErrorCodes::RSPF_OK) return;
   
   sprintf(temps,"%s %s",theRevb->theUrLon, theRevb->theUrLat);
   if (convertGeoPoint(temps, theUR_Corner) != rspfErrorCodes::RSPF_OK) return;
   
   sprintf(temps,"%s %s",theRevb->theLrLon, theRevb->theLrLat);
   if (convertGeoPoint(temps, theLR_Corner) != rspfErrorCodes::RSPF_OK) return;
   
   sprintf(temps,"%s %s",theRevb->theLlLon, theRevb->theLlLat);
   if (convertGeoPoint(temps, theLL_Corner) != rspfErrorCodes::RSPF_OK) return;        
   
   sprintf(temps,"%s %s",theRevb->theCenterLon, theRevb->theCenterLat);
   if (convertGeoPoint(temps, theCenterGP) != rspfErrorCodes::RSPF_OK) return;        
      
   theCenterImagePoint.x     = theRevb->theCenterSample;
   theCenterImagePoint.y     = theRevb->theCenterLine;
      
   theHorizontalOffset       = theRevb->theOffset;
   theOrientationAngle       = theRevb->theOrientationAngle;
   theSunElevation           = theRevb->theSunElevation;
   theSunAzimuth             = theRevb->theSunAzimuth;

   // The header was successfully extracted:
   theErrorStatus = rspfErrorCodes::RSPF_OK;   
   return;
}
 
int rspfFfL5::convertGeoPoint(const char* sptr, rspfGpt& geo_point)
{
   int    londeg;
   int    lonmin;
   double lonsec;
   char   lonhem;
   int    latdeg;
   int    latmin;
   double latsec;
   char   lathem;
   
   int converted = sscanf(sptr, "%03d%02d%7lf%c %02d%02d%7lf%c",
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

rspfRefPtr<rspfFfRevb> rspfFfL5::revb()
{
   return theRevb;
}

const rspfRefPtr<rspfFfRevb> rspfFfL5::revb()const
{
   return theRevb;
}
