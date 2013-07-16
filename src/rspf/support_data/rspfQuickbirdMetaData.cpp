//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
// 
// Class definition for rspfQuickbirdMetaData.
// 
// This class parses a Space Imaging Quickbird meta data file.
//
//********************************************************************
// $Id: rspfQuickbirdMetaData.cpp 14431 2009-04-30 21:58:33Z dburken $

#include <rspf/support_data/rspfQuickbirdMetaData.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <cstdio>
#include <iostream>


RTTI_DEF1(rspfQuickbirdMetaData, "rspfQuickbirdMetaData", rspfObject);
 
// Define Trace flags for use within this file:
static rspfTrace traceExec  ("rspfQuickbirdMetaData:exec");
static rspfTrace traceDebug ("rspfQuickbirdMetaData:debug");

rspfQuickbirdMetaData::rspfQuickbirdMetaData()
   :
   theGenerationDate("Unknown"),
   theBandId("Unknown"),
   theBitsPerPixel(0),
   theSatID("Unknown"),
   theTLCDate("Unknown"),
   theSunAzimuth(0.0),
   theSunElevation(0.0),
   theSatAzimuth(0.0),
   theSatElevation(0.0),
   theTDILevel(0),
   theAbsCalFactors(),
   theBandNameList("Unknown"),
   theImageSize()
{
   theImageSize.makeNan();
   theAbsCalFactors.clear();
}

rspfQuickbirdMetaData::~rspfQuickbirdMetaData()
{
}

bool rspfQuickbirdMetaData::open(const rspfFilename& imageFile)
{
   static const char MODULE[] = "rspfQuickbirdMetaData::open";

   clearFields();

   //retrieve information from the metadata file
   //if the Quickbird tif is 02APR01105228-M1BS-000000128955_01_P001.TIF
   //the metadata file will be 02APR01105228-M1BS-000000128955_01_P001.IMD

   rspfFilename metadatafile = imageFile;
   metadatafile.setExtension(rspfString("IMD"));

   if( parseMetaData(metadatafile) == false )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
	    << MODULE << " errors parsing metadata" << std::endl;
      }
      return false;
   }

   return true;
}

void rspfQuickbirdMetaData::clearFields()
{
   theGenerationDate = "Unknown";
   theBitsPerPixel = 0;
   theBandId = "Unknown";
   theSatID = "Unknown";
   theTLCDate = "Unknown";
   theSunAzimuth = 0.0;
   theSunElevation = 0.0;
   theSatAzimuth = 0.0;
   theSatElevation = 0.0;
   theTDILevel = 0;
   theAbsCalFactors.clear();
   theBandNameList = "Unknown";
   theImageSize.makeNan();
}

std::ostream& rspfQuickbirdMetaData::print(std::ostream& out) const
{

   out << "\n----------------- Info on Quickbird Image -------------------"
       << "\n  "
       << "\n  Generation date:    " << theGenerationDate
       << "\n  Band Id:            " << theBandId
       << "\n  Bits per pixel:     " << theBitsPerPixel
       << "\n  Sat Id:             " << theSatID
       << "\n  TLC date:           " << theTLCDate
       << "\n  Sun Azimuth:        " << theSunAzimuth
       << "\n  Sun Elevation:      " << theSunElevation
       << "\n  Sat Azimuth:        " << theSatAzimuth
       << "\n  Sat Elevation:      " << theSatElevation
       << "\n  Band name list:     " << theBandNameList
       << "\n  TDI Level:          " << theTDILevel
       << "\n  abs Calibration Factors:   " 
       << std::endl;
   for(unsigned int i=0; i<theAbsCalFactors.size(); i++)
   {
      out<<theAbsCalFactors[i] << "   ";
   }
   out << "\n  Image Size:         " << theImageSize
       << "n"
       << "\n---------------------------------------------------------"
       << "\n  " << std::endl;
   return out;
}

bool rspfQuickbirdMetaData::saveState(rspfKeywordlist& kwl,
				       const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfQuickbirdMetaData",
           true);

   kwl.add(prefix,
           "generation_date",
           theGenerationDate,
           true);

   kwl.add(prefix,
           "bits_per_pixel",
           theBitsPerPixel,
           true);

   kwl.add(prefix,
           "band_id",
           theBandId,
           true);

   kwl.add(prefix,
           "sat_id",
           theSatID,
           true);

   kwl.add(prefix,
           "tlc_date",
           theTLCDate,
           true);

   kwl.add(prefix,
           rspfKeywordNames::AZIMUTH_ANGLE_KW,
           theSunAzimuth,
           true);

   kwl.add(prefix,
           rspfKeywordNames::ELEVATION_ANGLE_KW,
           theSunElevation,
           true);

   kwl.add(prefix,
           "sat_azimuth_angle",
           theSatAzimuth,
           true);

   kwl.add(prefix,
           "sat_elevation_angle",
           theSatElevation,
           true);
          
   kwl.add(prefix,
           "TDI_level",
           theTDILevel,
           true);
   kwl.add(prefix,
           "band_name_list",
           theBandNameList,
           true);

   if( theBandId=="Multi" )
   {
      std::vector<rspfString> bandNameList = theBandNameList.split(" ");
      for(unsigned int i = 0 ; i < bandNameList.size(); ++i)
      {
         kwl.add(prefix,
                 bandNameList[i] + "_band_absCalFactor",
                 theAbsCalFactors[i],
                 true);
      }
   }
   else if(!theAbsCalFactors.empty())
   {
      kwl.add(prefix,
              "absCalFactor",
              theAbsCalFactors[0],
              true);  	
   }

   return true;
}

bool rspfQuickbirdMetaData::loadState(const rspfKeywordlist& kwl,
				       const char* prefix)
{
   clearFields();

   const char* lookup = 0;
   rspfString s;
  
   lookup = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if (lookup)
   {
      s = lookup;
      if(s != "rspfQuickbirdMetaData")
      {
         return false;
      }
   }

   lookup = kwl.find(prefix, "generation_date");
   if (lookup)
   {
      theGenerationDate = lookup;
   }

   lookup = kwl.find(prefix, "band_id");
   if (lookup)
   {
      theBandId = lookup;
   }
  
   lookup = kwl.find(prefix, "bits_per_pixel");
   if (lookup)
   {
      s = lookup;
      theBitsPerPixel = s.toInt();;
   }
    
   lookup = kwl.find(prefix, "sat_id");
   if (lookup)
   {
      theSatID = lookup;
   }

   lookup = kwl.find(prefix, "tlc_date");
   if (lookup)
   {
      theTLCDate= lookup;
   }

   lookup = kwl.find(prefix, "TDI_level");
   if (lookup)
   {
      s = lookup;
      theTDILevel = s.toInt();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::AZIMUTH_ANGLE_KW);
   if (lookup)
   {
      s = lookup;
      theSunAzimuth = s.toFloat64();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::ELEVATION_ANGLE_KW);
   if (lookup)
   {
      s = lookup;
      theSunElevation = s.toFloat64();
   }

   lookup = kwl.find(prefix, "sat_azimuth_angle");
   if (lookup)
   {
      s = lookup;
      theSatAzimuth = s.toFloat64();
   }

   lookup = kwl.find(prefix, "sat_elevation_angle");
   if (lookup)
   {
      s = lookup;
      theSatElevation = s.toFloat64();
   }

   lookup = kwl.find(prefix, "band_name_list");
   if (lookup)
   {
      theBandNameList= lookup;
   }

   if(theBandId=="Multi")
   {
      std::vector<rspfString> bandNameList = theBandNameList.split(" ");
      theAbsCalFactors = std::vector<double>(bandNameList.size(), 1.);
      for(unsigned int i = 0 ; i < bandNameList.size() ; ++i)
      {
         lookup = kwl.find(prefix, bandNameList[i] + "_band_absCalFactor");
         if (lookup)
         {
            s = lookup;
            theAbsCalFactors[i] = s.toDouble();
         }
      }
   }
   else if (theBandId=="P")
   {
      theAbsCalFactors = std::vector<double>(1, 1.);
      lookup = kwl.find(prefix, "absCalFactor");
      if (lookup)
      {
         s = lookup;
         theAbsCalFactors[0] = s.toDouble();
      }	
   }
   return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfQuickbirdMetaData::parseMetaData()
//
//  Parses the Quickbird IMD file.
//
//*****************************************************************************
bool rspfQuickbirdMetaData::parseMetaData(const rspfFilename& data_file)
{
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfQuickbirdMetaData::parseMetaData(data_file): entering..." << std::endl;
   
   if( !data_file.exists() )
   {
      if (traceExec()) rspfNotify(rspfNotifyLevel_WARN) << "rspfQuickbirdMetaData::parseMetaData(data_file) WARN:" << "\nmetadate data file <" << data_file << ">. " << "doesn't exist..." << std::endl;
      return false;
   }
  

   FILE* fptr = fopen (data_file, "r");
   if (!fptr)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
	    << "rspfQuickbirdRpcModel::parseMetaData(data_file) DEBUG:"
	    << "\nCould not open Meta data file:  " << data_file
	    << "\nreturning with error..." << std::endl;
      }
      return false;
   }

   char* strptr(NULL);

   //---
   // Read the file into a buffer:
   //---
   rspf_int32 fileSize = static_cast<rspf_int32>(data_file.fileSize());
   char* filebuf = new char[fileSize];
   fread(filebuf, 1, fileSize, fptr);
   strptr = filebuf;
   fclose(fptr);
   rspfString temp;

   //---
   // Generation time:
   //---
  
   if(getEndOfLine( strptr, rspfString("\ngenerationTime ="), "%17c %s", temp))
      theGenerationDate = rspfString(temp).before(";");
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
	    << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
	    << "\n\tAborting construction. Error encountered parsing "
	    << "presumed meta-data file." << std::endl;

         delete [] filebuf;
         return false;
      }
   }

   // Number of rows and columns in full image:
   if(getEndOfLine( strptr, rspfString("\nnumRows ="), "%10c %s", temp))
      theImageSize.line = rspfString(temp).before("\";").toInt();

   if(getEndOfLine( strptr, rspfString("\nnumColumns ="), "%13c %s", temp))
      theImageSize.samp = rspfString(temp).before("\";").toInt();

   //---
   // BandId:
   //---
   if(getEndOfLine( strptr, rspfString("\nbandId ="), "%9c %s", temp))
      theBandId = rspfString(temp).after("\"").before("\";");
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
	    << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
	    << "\n\tAborting construction. Error encountered parsing "
	    << "presumed meta-data file." << std::endl;

         delete [] filebuf;
         return false;
      }
   }
    
  
   //---
   // BitsPerPixel:
   //---
   if(getEndOfLine( strptr, rspfString("\nbitsPerPixel = "), "%16c %s", temp))
      theBitsPerPixel = rspfString(temp).before(";").toInt();
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
	    << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
	    << "\n\tAborting construction. Error encountered parsing "
	    << "presumed meta-data file." << std::endl;

         delete [] filebuf;
         return false;
      }
   }
     
   //---
   // absCalFactors:
   //---

   char *iter = (char *)filebuf;
   unsigned int nbBand = 0;
   theBandNameList = "";
   for(iter = strstr(iter, "BEGIN_GROUP = BAND_"); iter ; iter = strstr(iter, "BEGIN_GROUP = BAND_"))
   {
      ++nbBand;
      char dummy[80], nameChar[80];
      sscanf(iter, "%19c %s", dummy, nameChar);
      rspfString bandCur = rspfString(nameChar).before("\n");
      theBandNameList = theBandNameList + bandCur + " ";
      ++iter;
   }
   theBandNameList.trim();
   
   //--- Multispectral
   if(theBandId=="Multi")
   {
      std::vector<rspfString> bandList;
      bandList = theBandNameList.split(" ");
      theAbsCalFactors = std::vector<double>(bandList.size(), 1.);
      for(unsigned int j=0; j<bandList.size(); j++)
      {
         rspfString begin_group = "BEGIN_GROUP = BAND_" + bandList[j];
         strptr = strstr(filebuf, begin_group.c_str());
         if(!strptr && traceDebug())
         {	  				
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;
            delete [] filebuf;
            return false;
         }
         else
         {
            char dummy[80], nameChar[80];
            sscanf(strptr, "%19c %s", dummy, nameChar);
            rspfString bandCur = rspfString(nameChar).before("\n");
            if(!strptr && traceDebug())
            {	  				
               rspfNotify(rspfNotifyLevel_FATAL)
                  << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
                  << "\n\tAborting construction. Error encountered parsing "
                  << "presumed meta-data file." << std::endl;
               delete [] filebuf;
               return false;
            }
            else
            {
               if(bandList[j] == bandCur)
               {
                  strptr = strstr(strptr, "\tabsCalFactor = ");
                  sscanf(strptr, "%16c %s", dummy, nameChar);
                  theAbsCalFactors[j] = rspfString(nameChar).before(";").toDouble();
               }
            }
         }
      }
   }
   //--- Panchromatic
   else
   {
      theAbsCalFactors = std::vector<double>(1, 1.);
      if(getEndOfLine( strptr, rspfString("\tabsCalFactor = "), "%16c %s", temp))
         theAbsCalFactors[0] = rspfString(temp).before(";").toDouble();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;

            delete [] filebuf;
            return false;
         }
      }
   }
    
   //---
   // SatID:
   //---
   if(getEndOfLine( strptr, rspfString("\n\tsatId ="), "%9c %s", temp))
      theSatID = rspfString(temp).after("\"").before("\";");
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
	    << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
	    << "\n\tAborting construction. Error encountered parsing "
	    << "presumed meta-data file." << std::endl;

         delete [] filebuf;
         return false;
      }
   }

   //---
   // TLCTime:
   //---
   if(getEndOfLine( strptr, rspfString("\n\tTLCTime ="), "%11c %s", temp))
      theTLCDate = rspfString(temp).before("\";");
   else
   {
      if(getEndOfLine( strptr, rspfString("\n\tfirstLineTime ="), "%17c %s", temp))
         theTLCDate = rspfString(temp).before("\";");
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;
            delete [] filebuf;
            return false;
         }
      }
   }
   
   //---
   // Sun Azimuth:
   //---
   if(getEndOfLine( strptr, rspfString("\n\tsunAz ="), "%9c %s", temp))
        theSunAzimuth = rspfString(temp).before(";").toFloat64();
   else
   {
      if(getEndOfLine( strptr, rspfString("\n\tmeanSunAz ="), "%13c %s", temp))
         theSunAzimuth = rspfString(temp).before(";").toFloat64();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;

            delete [] filebuf;
            return false;
         }
      }
   }

   //---
   // Sun Elevation:
   //---
   if(getEndOfLine( filebuf, rspfString("\n\tsunEl ="), "%9c %s", temp))
        theSunElevation = rspfString(temp).before(";").toFloat64();
   else
   {
      if(getEndOfLine( filebuf, rspfString("\n\tmeanSunEl ="), "%13c %s", temp))
         theSunElevation = rspfString(temp).before(";").toFloat64();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;

            delete [] filebuf;
            return false;
         }
      }
   }

   //---
   // Sun Azimuth:
   //---
   if(getEndOfLine( strptr, rspfString("\n\tsunAz ="), "%9c %s", temp))
      theSunAzimuth = rspfString(temp).before(";").toFloat64();
   else
   {
      if(getEndOfLine( strptr, rspfString("\n\tmeanSunAz ="), "%13c %s", temp))
         theSunAzimuth = rspfString(temp).before(";").toFloat64();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;
            
            delete [] filebuf;
            return false;
         }
      }
   }

   //---
   // Sun Elevation:
   //---
   if(getEndOfLine( filebuf, rspfString("\n\tsunEl ="), "%9c %s", temp))
      theSunElevation = rspfString(temp).before(";").toFloat64();
   else
   {
      if(getEndOfLine( filebuf, rspfString("\n\tmeanSunEl ="), "%13c %s", temp))
         theSunElevation = rspfString(temp).before(";").toFloat64();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;

            delete [] filebuf;
            return false;
         }
      }
   }


   //---
   // Sat Azimuth:
   //---
   if(getEndOfLine( strptr, rspfString("\n\tsatAz ="), "%9c %s", temp))
      theSatAzimuth = rspfString(temp).before(";").toFloat64();
   else
   {
      if(getEndOfLine( strptr, rspfString("\n\tmeanSatAz ="), "%13c %s", temp))
         theSatAzimuth = rspfString(temp).before(";").toFloat64();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;
            
            delete [] filebuf;
            return false;
         }
      }
   }
   
   //---
   // Sat Elevation:
   //---
   if(getEndOfLine( filebuf, rspfString("\n\tsatEl ="), "%9c %s", temp))
      theSatElevation = rspfString(temp).before(";").toFloat64();
   else
   {
      if(getEndOfLine( filebuf, rspfString("\n\tmeanSatEl ="), "%13c %s", temp))
         theSatElevation = rspfString(temp).before(";").toFloat64();
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
               << "\n\tAborting construction. Error encountered parsing "
               << "presumed meta-data file." << std::endl;
            
            delete [] filebuf;
            return false;
         }
      }
   }
   
   
   //---
   // TDILevel:
   //---
   if(getEndOfLine( strptr, rspfString("\n\tTDILevel = "), "%13c %s", temp))
      theTDILevel = rspfString(temp).before(";").toInt();
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
	    << "FATAL rspfQuickbirdRpcModel::parseMetaData(data_file): "
	    << "\n\tAborting construction. Error encountered parsing "
	    << "presumed meta-data file." << std::endl;

         delete [] filebuf;
         return false;
      }
   }


   delete [] filebuf;
   filebuf = 0;

   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfQuickbirdRpcModel::parseMetaData(data_file): returning..."
         << std::endl;
   }

   return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfQuickbirdMetaData::parseGEOData()
//
//  Parses the Quickbird GEO file.
//
//*****************************************************************************
bool rspfQuickbirdMetaData::parseGEOData(const rspfFilename& /* data_file */)
{
  return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfQuickbirdMetaData::parseRpcData()
//
//  Parses the Quickbird EPH file.
//
//*****************************************************************************
bool rspfQuickbirdMetaData::parseEPHData(const rspfFilename& /* data_file */)
{
  return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfQuickbirdMetaData::parseRpcData()
//
//  Parses the Quickbird ATT file.
//
//*****************************************************************************
bool rspfQuickbirdMetaData::parseATTData(const rspfFilename& /* data_file */)
{
  return true;
}

rspfString rspfQuickbirdMetaData::getSatID() const
{
   return theSatID;
}

//*****************************************************************************
// PROTECTED METHOD: rspfQuickbirdMetaData::getEndOfLine
//
// //  Parse a char * to find another char *. Change the pointer only if the second char * is found.
//
//*****************************************************************************
bool rspfQuickbirdMetaData::getEndOfLine( char * fileBuf,
                                           rspfString lineBeginning,
                                           const char * format,
                                           rspfString & name)
{
   //char * res = strstr(fileBuf, lineBeginning.c_str());
   //if (!res)
   //{
   //  return false;
   // }
   //// if the lineBeginning is found, update the start pointer adress
   //fileBuf = res;
   
   //char dummy[80], nameChar[80];
   //sscanf(res, format, dummy, nameChar);    
   //name = rspfString(nameChar);
    
   char * res = strstr(fileBuf, lineBeginning.c_str());
   if(!res)
   {
      return false;
   }
   fileBuf = strstr(fileBuf, lineBeginning.c_str());
   char dummy[80], nameChar[80];
   sscanf(fileBuf, format, dummy, nameChar);  
   name = rspfString(nameChar);
    
   return true;
}

const rspfIpt& rspfQuickbirdMetaData::getImageSize() const
{
   return theImageSize;
}
