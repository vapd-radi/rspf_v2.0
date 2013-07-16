//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
// 
// Class definition for rspfIkonosMetaData.
// 
// This class parses a Space Imaging Ikonos meta data file.
//
//********************************************************************
// $Id: rspfIkonosMetaData.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <cstdio>
#include <iostream>
#include <rspf/support_data/rspfIkonosMetaData.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>

RTTI_DEF1(rspfIkonosMetaData, "rspfIkonosMetaData", rspfObject);

// Define Trace flags for use within this file:
static rspfTrace traceExec  ("rspfIkonosMetaData:exec");
static rspfTrace traceDebug ("rspfIkonosMetaData:debug");

rspfIkonosMetaData::rspfIkonosMetaData()
  :
  theNominalCollectionAzimuth(0.0),
  theNominalCollectionElevation(0.0),
  theSunAzimuth(0.0),
  theSunElevation(0.0),
  theNumBands(0),
  theBandName("Unknown"),
  theProductionDate("Unknown"),
  theAcquisitionDate("Unknown"),
  theAcquisitionTime("Unknown"),
  theSensorID("Unknown")
{
}

rspfIkonosMetaData::~rspfIkonosMetaData()
{
}

bool rspfIkonosMetaData::open(const rspfFilename& imageFile)
{
   static const char MODULE[] = "rspfIkonosMetaData::open";
   
   clearFields();
   
   //retrieve information from the metadata file
   //if the ikonos tif is po_2619900_pan_0000000.tif
   //the metadata file will be po_2619900_metadata.txt

   rspfString separator("_");
   rspfString filenamebase = imageFile.fileNoExtension();
   std::vector< rspfString > filenameparts = filenamebase.split(separator);
   
   if(filenameparts.size() < 2)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " Ikonos filename non standard" << std::endl;
      }
      return false;
   }
   rspfFilename metadatafile = filenameparts[0];
   metadatafile += "_";
   metadatafile += filenameparts[1];
   metadatafile += "_metadata.txt";
   
   metadatafile.setPath(imageFile.path());
   
   if( parseMetaData(metadatafile) == false )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " errors parsing metadata" << std::endl;
      }
      return false;
   }
   
   rspfFilename hdrfile = imageFile;
   hdrfile.setExtension(rspfString("hdr"));
   if( parseHdrData(hdrfile) == false )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " errors parsing hdr" << std::endl;
      }
      return false;
   }
   
   rspfFilename rpcfile = imageFile.noExtension();
   rpcfile += "_rpc.txt";
   if (parseRpcData (rpcfile) == false)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " errors parsing rpc" << std::endl;
      }
      return false;
   }

   return true;
}

void rspfIkonosMetaData::clearFields()
{
  theNominalCollectionAzimuth = 0.0;
  theNominalCollectionElevation = 0.0;
  theSunAzimuth = 0.0;
  theSunElevation = 0.0;
  theNumBands = 0;
  theBandName = "Unknown";
  theProductionDate = "Unknown";
  theAcquisitionDate = "Unknown";
  theAcquisitionTime = "Unknown";
  theSensorID = "Unknown";
}

std::ostream& rspfIkonosMetaData::print(std::ostream& out) const
{

  out << "\n----------------- Info on Ikonos Image -------------------"
      << "\n  "
      << "\n  Nominal Azimuth:    " << theNominalCollectionAzimuth
      << "\n  Nominal Elevation:   " << theNominalCollectionElevation
      << "\n  Sun Azimuth:    " << theSunAzimuth
      << "\n  Sun Elevation:   " << theSunElevation
      << "\n  Number of bands:   " << theNumBands
      << "\n  Band name:   " << theBandName
      << "\n  Production date:   " << theProductionDate
      << "\n  Acquisition date:   " << theAcquisitionDate
      << "\n  Acquisition time:   " << theAcquisitionTime
      << "\n  Sensor Type:   " << theSensorID
      << "\n"
      << "\n---------------------------------------------------------"
      << "\n  " << std::endl;
  return out;
}

rspfString rspfIkonosMetaData::getSensorID() const
{
  return theSensorID;
}

bool rspfIkonosMetaData::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{

  kwl.add(prefix,
          rspfKeywordNames::TYPE_KW,
          "rspfIkonosMetaData",
          true);

  kwl.add(prefix,
          "nominal_collection_azimuth_angle",
          theNominalCollectionAzimuth,
          true);

  kwl.add(prefix,
          "nominal_collection_elevation_angle",
          theNominalCollectionElevation,
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
          rspfKeywordNames::NUMBER_BANDS_KW,
          theNumBands,
          true);

  kwl.add(prefix,
          "band_name",
          theBandName,
          true);

  kwl.add(prefix,
          "production_date",
          theProductionDate,
          true);

  kwl.add(prefix,
          "acquisition_date",
          theAcquisitionDate,
          true);

  kwl.add(prefix,
          "acquisition_time",
          theAcquisitionTime,
          true);

  kwl.add(prefix,
          "sensor",
          theSensorID,
          true);

  return true;
}

bool rspfIkonosMetaData::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   clearFields();

   const char* lookup = 0;
   rspfString s;
  
   lookup = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if (lookup)
   {
      s = lookup;
      if(s != "rspfIkonosMetaData")
      {
         return false;
      }
   }

  lookup = kwl.find(prefix, "nominal_collection_azimuth_angle");
  if (lookup)
  {
     s = lookup;
     theNominalCollectionAzimuth = s.toFloat64();
  }

  lookup = kwl.find(prefix, "nominal_collection_elevation_angle");
  if (lookup)
  {
     s = lookup;
     theNominalCollectionElevation = s.toFloat64();
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
  
   lookup = kwl.find(prefix, rspfKeywordNames::NUMBER_BANDS_KW);
   if (lookup)
   {
      s = lookup;
      theNumBands = s.toUInt32();
   }

   lookup = kwl.find(prefix, "band_name");
   if (lookup)
   {
      theBandName = lookup;
   }

   lookup = kwl.find(prefix, "production_date");
   if (lookup)
   {
      theProductionDate = lookup;
   }

  lookup = kwl.find(prefix, "acquisition_date");
  if (lookup)
  {
     theAcquisitionDate = lookup;
  }

  lookup = kwl.find(prefix, "acquisition_time");
  if (lookup)
  {
     theAcquisitionTime = lookup;
  }

  lookup = kwl.find(prefix, "sensor");
  if (lookup)
  {
     theSensorID = lookup;
  }
  
   return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfIkonosMetaData::parseMetaData()
//
//  Parses the Ikonos metadata file.
//
//*****************************************************************************
bool rspfIkonosMetaData::parseMetaData(const rspfFilename& data_file)
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseMetaData(data_file): entering..."
         << std::endl;
   }

   FILE* fptr = fopen (data_file, "r");
   if (!fptr)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfIkonosRpcModel::parseMetaData(data_file) DEBUG:"
            << "\nCould not open Meta data file:  " << data_file
            << "\nreturning with error..." << std::endl;
      }
      return false;
   }

   char* strptr;
   char dummy[80], name[80];
   double value;

   //---
   // Read the file into a buffer:
   //---
   rspf_int32 fileSize = static_cast<rspf_int32>(data_file.fileSize());
   char* filebuf = new char[fileSize];
   fread(filebuf, 1, fileSize, fptr);
   fclose(fptr);

   //---
   // Production date:
   //---
   strptr = strstr(filebuf, "\nCreation Date:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }

   sscanf(strptr, "%15c %s", dummy, name);
   theProductionDate = name;

   //***
   // Sensor Type:
   //***
   strptr = strstr(strptr, "\nSensor:");
   if (!strptr)
   {
      if(traceDebug())
      { 
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }

   sscanf(strptr, "%8c %s", dummy, name);
   theSensorID = name;


   //***
   // Nominal Azimuth:
   //***
   strptr = strstr(strptr, "\nNominal Collection Azimuth:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }

   sscanf(strptr, "%28c %lf %s", dummy, &value, dummy);
   theNominalCollectionAzimuth = value;

   //***
   // Nominal Elevation:
   //***
   strptr = strstr(strptr, "\nNominal Collection Elevation:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }
  
   sscanf(strptr, "%31c %lf %s", dummy, &value, dummy);
   theNominalCollectionElevation = value;

   //***
   // Sun Azimuth:
   //***
   strptr = strstr(strptr, "\nSun Angle Azimuth:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }

   sscanf(strptr, "%19c %lf %s", dummy, &value, dummy);
   theSunAzimuth = value;

   //***
   // Sun Elevation:
   //***
   strptr = strstr(strptr, "\nSun Angle Elevation:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }

   sscanf(strptr, "%21c %lf %s", dummy, &value, name);
   theSunElevation = value;

   //---
   // Acquisition date and time:
   //---
   strptr = strstr(filebuf, "\nAcquisition Date/Time:");
   if (!strptr)
   {
      if(traceDebug())
      {
      rspfNotify(rspfNotifyLevel_FATAL)
          << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
          << "\n\tAborting construction. Error encountered parsing "
          << "presumed meta-data file." << std::endl;
      }
      delete [] filebuf;
      return false;
   }

   char name2[80];
   sscanf(strptr, "%23c %s %s", dummy, name, name2);
   theAcquisitionDate = name;
   theAcquisitionTime = name2;

   delete [] filebuf;
   filebuf = 0;

   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseMetaData(data_file): returning..."
         << std::endl;
   }

   return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfIkonosMetaData::parseHdrData()
//
//  Parses the Ikonos hdr file.
//
//*****************************************************************************
bool rspfIkonosMetaData::parseHdrData(const rspfFilename& data_file)
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseHdrData(data_file): entering..."
         << std::endl;
   }

   FILE* fptr = fopen (data_file, "r");
   if (!fptr)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file) WARN:"
            << "\nCould not open hdr data file <" << data_file << ">. "
            << "returning with error..." << std::endl;
      }
      return false;
   }

   char* strptr;
   // char linebuf[80];
   char dummy[80];
   char name[80];
   int value=0;

   //***
   // Read the file into a buffer:
   //***
   char filebuf[5000];
   fread(filebuf, 1, 5000, fptr);
   fclose(fptr);

   //***
   // Band name:
   //***
   strptr = strstr(filebuf, "\nBand:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file):"
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed hdr file." << std::endl;
      }

      return false;
   }

   sscanf(strptr, "%6c %s", dummy, name);
   theBandName = name;

   //***
   // Number of Bands:
   //***
   strptr = strstr(filebuf, "\nNumber of Bands:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file):"
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed hdr file." << std::endl;
      }

      return false;
   }

   sscanf(strptr, "%17c %d", dummy, &value);
   theNumBands = value;

   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseHdrData(data_file): returning..."
         << std::endl;
   }
   return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfIkonosMetaData::parseRpcData()
//
//  Parses the Ikonos rpc file.
//
//*****************************************************************************
bool rspfIkonosMetaData::parseRpcData(const rspfFilename& /* data_file */)
{
   return true;
}
