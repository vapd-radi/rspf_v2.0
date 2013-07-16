//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfIkonosRpcModel. This 
//    derived class implements the capability of reading Ikonos RPC support
//    data.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfIkonosRpcModel.cpp 20606 2012-02-24 12:29:52Z gpotts $

#include <cstdlib>
#include <rspf/projection/rspfIkonosRpcModel.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/base/rspfTrace.h>


RTTI_DEF1(rspfIkonosRpcModel, "rspfIkonosRpcModel", rspfRpcModel);

//***
// Define Trace flags for use within this file:
//***
static rspfTrace traceExec  ("rspfIkonosRpcModel:exec");
static rspfTrace traceDebug ("rspfIkonosRpcModel:debug");

const rspfFilename INIT_RPC_GEOM_FILENAME ("rpc_init.geom");

static const char* MODEL_TYPE        = "rspfIkonosRpcModel";
static const char* META_DATA_FILE    = "meta_data_file";
static const char* RPC_DATA_FILE     = "rpc_data_file";
static const char* LINE_OFF_KW       = "LINE_OFF";
static const char* SAMP_OFF_KW       = "SAMP_OFF";
static const char* LAT_OFF_KW        = "LAT_OFF";
static const char* LONG_OFF_KW       = "LONG_OFF";
static const char* HEIGHT_OFF_KW     = "HEIGHT_OFF";
static const char* LINE_SCALE_KW     = "LINE_SCALE";
static const char* SAMP_SCALE_KW     = "SAMP_SCALE";
static const char* LAT_SCALE_KW      = "LAT_SCALE";
static const char* LONG_SCALE_KW     = "LONG_SCALE";
static const char* HEIGHT_SCALE_KW   = "HEIGHT_SCALE";
static const char* LINE_NUM_COEFF_KW = "LINE_NUM_COEFF_";
static const char* LINE_DEN_COEFF_KW = "LINE_DEN_COEFF_";
static const char* SAMP_NUM_COEFF_KW = "SAMP_NUM_COEFF_";
static const char* SAMP_DEN_COEFF_KW = "SAMP_DEN_COEFF_";


rspfIkonosRpcModel::rspfIkonosRpcModel()
   :rspfRpcModel(),
    theSupportData(new rspfIkonosMetaData())
{
}

//*****************************************************************************
//  CONSTRUCTOR: rspfIkonosRpcModel
//  
//  Constructs given a geometry file that specifies the filenames for the
//  metadata and RPC data files.
//  
//*****************************************************************************
rspfIkonosRpcModel::rspfIkonosRpcModel(const rspfFilename& geom_file)
   :  rspfRpcModel(),
      theSupportData(new rspfIkonosMetaData())
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel Constructor #1: entering..."
         << std::endl;
   }

   rspfKeywordlist kwl(geom_file);
   const char* value;
   
   //***
   // Assure this keywordlist contains correct type info:
   //***
   value = kwl.find(rspfKeywordNames::TYPE_KW);
   if (!value || (strcmp(value, "rspfIkonosRpcModel")))
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG  rspfIkonosRpcModel Constructor #1:"
            << "\nFailed attempt to construct. sensor type \""<<value
            << "\" does not match \"rspfIkonosRpcModel\"." << std::endl;
      }

      theErrorStatus++;
      if (traceExec())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG  rspfIkonosRpcModel Constructor #1: returning..."
            << std::endl;
      }
      return;
   }

   //***
   // Read meta data filename from geom file:
   //***
   value = kwl.find(META_DATA_FILE);
   if (!value)
   {
      theErrorStatus++;
      if (traceExec())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfIkonosRpcModel Constructor #1: returning..."
            << std::endl;
      }
      return;
   }

   rspfFilename metadata (value);

   //***
   // Read RPC data filename from geom file:
   //***
   value = kwl.find(RPC_DATA_FILE);
   if (!value)
   {
      theErrorStatus++;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG  rspfIkonosRpcModel Constructor #1: returning..." << std::endl;
      return;
   }
   rspfFilename rpcdata (value);

   parseMetaData(metadata);
   parseRpcData (rpcdata);
   finishConstruction();

   rspfString drivePart;
   rspfString pathPart;
   rspfString filePart;
   rspfString extPart;
   geom_file.split(drivePart,
                   pathPart,
                   filePart,
                   extPart);

   

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG returning..." << std::endl;
   
   return;
}

//*****************************************************************************
//  CONSTRUCTOR: rspfIkonosRpcModel
//  
//  Constructs given filenames for metadata and RPC data.
//  
//*****************************************************************************
rspfIkonosRpcModel::rspfIkonosRpcModel(const rspfFilename& metadata,
                                         const rspfFilename& rpcdata)
   :
   rspfRpcModel(),
   theSupportData(new rspfIkonosMetaData())
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfIkonosRpcModel Constructor #2: entering..." << std::endl;

   parseMetaData(metadata);
   parseRpcData (rpcdata);
   finishConstruction();

   //***
   // Save current state in RPC model format:
   //***
   rspfString drivePart;
   rspfString pathPart;
   rspfString filePart;
   rspfString extPart;
   metadata.split(drivePart,
                  pathPart,
                  filePart,
                  extPart);
   
   rspfFilename init_rpc_geom;
   init_rpc_geom.merge(drivePart,
                       pathPart,
                       INIT_RPC_GEOM_FILENAME,
                       "");
//      (metadata.path().dirCat(rspfRpcModel::INIT_RPC_GEOM_FILENAME));
   rspfKeywordlist kwl (init_rpc_geom);
   saveState(kwl);
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfIkonosRpcModel Constructor #2: returning..." << std::endl;
}

rspfIkonosRpcModel::~rspfIkonosRpcModel()
{
   theSupportData = 0;
}

//*****************************************************************************
//  METHOD: rspfIkonosRpcModel::finishConstruction()
//  
//*****************************************************************************
void rspfIkonosRpcModel::finishConstruction()
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel finishConstruction(): entering..."
         << std::endl;
   }

   //***
   // Assign other data members:
   //***
   thePolyType      = B; // This may not be true for early RPC imagery
   theRefImgPt.line = theLineOffset;
   theRefImgPt.samp = theSampOffset;
   theRefGndPt.lat  = theLatOffset;
   theRefGndPt.lon  = theLonOffset;
   theRefGndPt.hgt  = theHgtOffset;

   //***
   // Assign the bounding image space rectangle:
   //***
   theImageClipRect = rspfDrect(0.0, 0.0,
                                 theImageSize.samp-1, theImageSize.line-1);

   //---
   // NOTE:  We must call "updateModel()" to set parameter used by base
   // rspfRpcModel prior to calling lineSampleHeightToWorld or all
   // the world points will be same.
   //---
   updateModel();   

   //***
   // Assign the bounding ground polygon:
   //***
   rspfGpt v0, v1, v2, v3;
   rspfDpt ip0 (0.0, 0.0);
   lineSampleHeightToWorld(ip0, 0.0, v0);
   rspfDpt ip1 (theImageSize.samp-1.0, 0.0);
   lineSampleHeightToWorld(ip1, 0.0, v1);
   rspfDpt ip2 (theImageSize.samp-1.0, theImageSize.line-1.0);
   lineSampleHeightToWorld(ip2, 0.0, v2);
   rspfDpt ip3 (0.0, theImageSize.line-1.0);
   lineSampleHeightToWorld(ip3, 0.0, v3);
   theBoundGndPolygon
      = rspfPolygon (rspfDpt(v0), rspfDpt(v1), rspfDpt(v2), rspfDpt(v3));

   //---
   // Call compute gsd:
   // 
   // This will set theGSD and theMeanGSD using lineSampleHeightToWorld on
   // three image points.  Previously this was pulled from metadata.  Some of
   // which was in US Survey feet and not converted to meters.  This method
   // is more accurate as it uses the sensor model to compute.
   //---
   try
   {
      // Method throws rspfException.
      computeGsd();
   }
   catch (const rspfException& e)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfIkonosRpcModel finishConstruction Caught Exception:\n"
         << e.what() << std::endl;
   }
   
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel finishConstruction(): returning..."
         << std::endl;
   }
}

//*****************************************************************************
// PROTECTED METHOD: rspfIkonosRpcModel::parseMetaData()
//  
//  Parses the Ikonos metadata file.
//  
//*****************************************************************************
void rspfIkonosRpcModel::parseMetaData(const rspfFilename& data_file)
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
      ++theErrorStatus;
      if (traceExec())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfIkonosRpcModel::parseMetaData(data_file) DEBUG:"
            << "\nCould not open Meta data file:  " << data_file
            << "\nreturning with error..." << std::endl;
      }
      return;
   }

   char* strptr;
   // char linebuf[80];
   char dummy[80], name[80];

   //***
   // Read the file into a buffer:
   //***
   char filebuf[5000];
   fread(filebuf, 1, 5000, fptr);
   
   //***
   // Image ID:
   //***
   strptr = strstr(filebuf, "\nSource Image ID:");
   if (!strptr)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << endl;
      }
      return;
   }
      
   sscanf(strptr, "%17c %s", dummy, name);
   theImageID = name;

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
            << "presumed meta-data file." << endl;
         
         return;
      }
   }
   sscanf(strptr, "%8c %s", dummy, name);
   theSensorID = name;

   //***
   // GSD:  NOTE - this will be recomputed by computeGsd method later.
   //***
   strptr = strstr(strptr, "\nPixel Size X:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << endl;
         
         return;
      }
   }
   
   sscanf(strptr, "%14c %lf", dummy, &theGSD.samp);
   strptr = strstr(strptr, "\nPixel Size Y:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << endl;
         
         return;
      }
   }
   sscanf(strptr, "%14c %lf", dummy, &theGSD.line);

   //***
   // Image size:
   //***
   strptr = strstr(strptr, "\nColumns:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << endl;
         
         return;
      }
   }
   sscanf(strptr, "%s %d", dummy, &theImageSize.samp);
   strptr = strstr(strptr, "\nRows:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfIkonosRpcModel::parseMetaData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed meta-data file." << endl;
         
         return;
      }
   }
   sscanf(strptr, "%s %d", dummy, &theImageSize.line);
           
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseMetaData(data_file): returning..."
         << std::endl;
   }
}

//*****************************************************************************
// PROTECTED METHOD: rspfIkonosRpcModel::parseHdrData()
//  
//  Parses the Ikonos hdr file.
//  
//*****************************************************************************
bool rspfIkonosRpcModel::parseHdrData(const rspfFilename& data_file)
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseHdrData(data_file): entering..."
         << std::endl;
   }
   
   if( !data_file.exists() )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file) WARN:"
            << "\nrpc data file <" << data_file << ">. "<< "doesn't exist..."
            << std::endl;
      }
      return false;
   }

   FILE* fptr = fopen (data_file, "r");
   if (!fptr)
   {
      ++theErrorStatus;
      
      if (traceDebug())
      { 
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file) WARN:"
            << "\nCould not open hdr data file <" << data_file << ">. "
            << "returning with error..." << std::endl;
      }
      return false;
   }

   char* strptr = 0;
   // char linebuf[80];
   char dummy[80];
   // , name[80];

   //***
   // Read the file into a buffer:
   //***
   char filebuf[5000];
   fread(filebuf, 1, 5000, fptr);

   //***
   // GSD:  NOTE - this will be recomputed by computeGsd method later.
   //***
   strptr = strstr(filebuf, "\nPixel Size X:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file):"
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed hdr file." << endl;
      }
      
      return false;
   }
      
   sscanf(strptr, "%14c %lf", dummy, &theGSD.samp);
   strptr = strstr(strptr, "\nPixel Size Y:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed hdr file." << endl;
      }
         
      return false;
   }

   sscanf(strptr, "%14c %lf", dummy, &theGSD.line);

   //***
   // Image size:
   //***
   strptr = strstr(strptr, "\nColumns:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed hdr file." << endl;
      }
         
      return false;
   }
   sscanf(strptr, "%s %d", dummy, &theImageSize.samp);
   strptr = strstr(strptr, "\nRows:");
   if (!strptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseHdrData(data_file): "
            << "\n\tAborting construction. Error encountered parsing "
            << "presumed hdr file." << endl;
      }
         
      return false;
   }
   sscanf(strptr, "%s %d", dummy, &theImageSize.line);
           
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseHdrData(data_file): returning..."
         << std::endl;
   }
   return true;
}

//*****************************************************************************
// PROTECTED METHOD: rspfIkonosRpcModel::parseRpcData()
//  
//  Parses the Ikonos RPC data file.
//  
//*****************************************************************************
void rspfIkonosRpcModel::parseRpcData(const rspfFilename& data_file)
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfIkonosRpcModel::parseRpcData(data_file): entering..."
         << std::endl;
   }
      
   if( !data_file.exists() )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfIkonosRpcModel::parseRpcData(data_file) WARN:"
            << "\nrpc data file <" << data_file << ">. "<< "doesn't exist..."
            << std::endl;
      }
      ++theErrorStatus;
      return;
   }
   
   //***
   // The Ikonos RPC data file is conveniently formatted as KWL file:
   //***
   rspfKeywordlist kwl (data_file);
   if (kwl.getErrorStatus())
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << "ERROR rspfIkonosRpcModel::parseRpcData(data_file): Could not open RPC data file <" << data_file << ">. " << "Aborting..." << std::endl;
      ++theErrorStatus;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
         << "returning with error..." << std::endl;
      return;
   }

   const char* buf;
   const char* keyword;
   
   //***
   // Parse data from KWL:
   //***
   keyword = LINE_OFF_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   theLineOffset = atof(buf);
      
   keyword = SAMP_OFF_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   theSampOffset = atof(buf);

   keyword = LAT_OFF_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
      
   theLatOffset = atof(buf);
   
   keyword = LONG_OFF_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   theLonOffset = atof(buf);

   keyword = HEIGHT_OFF_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
      
   theHgtOffset = atof(buf);

   keyword = LINE_SCALE_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   theLineScale = atof(buf);
   
   keyword = SAMP_SCALE_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   theSampScale = atof(buf);
   
   keyword = LAT_SCALE_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   else
     {
       // copy rspfIkonosMetada-sensor into rspfIkonosRpcModel-sensorId
       theSensorID = theSupportData->getSensorID();
     }
   

   theLatScale = atof(buf);
   
   keyword = LONG_SCALE_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
   theLonScale = atof(buf);
   
   keyword = HEIGHT_SCALE_KW;
   buf = kwl.find(keyword);
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                          << "\nAborting construction. Error looking up keyword: "
                                          << keyword << std::endl;
      return;
   }
      
   theHgtScale = atof(buf);
   
   char kwbuf[32];
   keyword = kwbuf;
   for(int i=1; i<=20; i++)
   {
      sprintf(kwbuf, "%s%d", LINE_NUM_COEFF_KW, i);
      buf = kwl.find(keyword);
      if (!buf)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                             << "\nAborting construction. Error looking up keyword: "
                                             << keyword << std::endl;
         return;
      }
      
      theLineNumCoef[i-1] = atof(buf);
      
      sprintf(kwbuf, "%s%d", LINE_DEN_COEFF_KW, i);
      buf = kwl.find(keyword);
      if (!buf)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                             << "\nAborting construction. Error looking up keyword: "
                                             << keyword << std::endl;
         return;
      }
      theLineDenCoef[i-1] = atof(buf);
      
      sprintf(kwbuf, "%s%d", SAMP_NUM_COEFF_KW, i);
      buf = kwl.find(keyword);
      if (!buf)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                             << "\nAborting construction. Error looking up keyword: "
                                             << keyword << std::endl;
         return;
      }
      theSampNumCoef[i-1] = atof(buf);
      
      sprintf(kwbuf, "%s%d", SAMP_DEN_COEFF_KW, i);
      buf = kwl.find(keyword);
      if (!buf)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfIkonosRpcModel::parseRpcData(data_file):"
                                             << "\nAborting construction. Error looking up keyword: "
                                             << keyword << std::endl;
         return;
      }
      theSampDenCoef[i-1] = atof(buf);
   }

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfIkonosRpcModel::parseRpcData(data_file): returning..." << std::endl;
   return;

   theErrorStatus++;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfIkonosRpcModel::parseRpcData(data_file): returning with error..." << std::endl;
   
   return;
}

//*****************************************************************************
//  METHOD: rspfIkonosRpcModel::writeGeomTemplate()
//  
//   Writes a template of an rspfIkonosRpcModel geometry file.
//  
//*****************************************************************************
void rspfIkonosRpcModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::writeGeomTemplate(os): entering..." << std::endl;

   os <<
      "//**************************************************************\n"
      "// Template for Ikonos RPC geometry keywordlist\n"
      "//\n"
      "// NOTE: It is preferable to select the full RPC geometry KWL \n"
      "//       that should have been created with the first use of the\n"
      "//       derived model type rspfIkonosRpcModel. Using this KWL \n"
      "//       implies that an initial geometry is being constructed \n"
      "//       with all adjustable parameters initialized to 0. \n"
      "//**************************************************************\n"
      << rspfKeywordNames::TYPE_KW << ": " << MODEL_TYPE << endl;
   os << META_DATA_FILE << ": <string>\n"
      << RPC_DATA_FILE  << ": <string>\n" << endl;

   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcModel::writeGeomTemplate(os): returning..." << std::endl;
   return;
}

bool rspfIkonosRpcModel::saveState(rspfKeywordlist& kwl,
				    const char* prefix)const
{
   if(theSupportData.valid())
   {
      rspfString supportPrefix = rspfString(prefix) + "support_data.";
      // copy rspfIkonosMetada-sensor into rspfIkonosRpcModel-sensorId
      theSupportData->saveState(kwl, supportPrefix);
   }

   rspfRpcModel::saveState(kwl, prefix);

  // this model just sets the base class values so
  // we do not need to re-construct this model so 
  // specify the type as the base class type
  //
//   kwl.add(prefix,
// 	  rspfKeywordNames::TYPE_KW,
// 	  STATIC_TYPE_NAME(rspfRpcModel),
// 	  true);

  // cout << "kwl:\n" << kwl << std::endl;

  return true;
}

bool rspfIkonosRpcModel::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   if(theSupportData.valid())
   {
      rspfString supportPrefix = rspfString(prefix) + "support_data.";
      theSupportData->loadState(kwl, supportPrefix);
   }

   return rspfRpcModel::loadState(kwl, prefix);
}

bool rspfIkonosRpcModel::parseFile(const rspfFilename& file)
{
      return parseTiffFile(file);
}

bool rspfIkonosRpcModel::parseTiffFile(const rspfFilename& filename)
{
   bool result = false;
   
   rspfRefPtr<rspfTiffTileSource> tiff = new rspfTiffTileSource();

   if ( tiff->open(filename) )
   {
      if ( !theSupportData )
      {
         theSupportData = new rspfIkonosMetaData();
      }

      if ( theSupportData->open(filename) == false )
      {
         if(traceDebug())
         {
            // Currently not required by model so we will not error out here.
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "WARNING: rspfIkonosMetaData::open returned false.\n"
               << std::endl;
         }
      }
      else
      {
         // copy rspfIkonosMetada-sensor into rspfIkonosRpcModel-sensorId
         theSensorID = theSupportData->getSensorID();
      }

      //convert file to rpc filename and hdr filename so we can get some info
      rspfFilename rpcfile = filename.noExtension();
      rpcfile += "_rpc.txt";
      
      rspfFilename hdrfile = filename;
      hdrfile.setExtension(rspfString("hdr"));
      
      if( parseHdrData(hdrfile) )
      {
         // parseRpcData sets the error status on error.
         parseRpcData (rpcfile);
         if ( !getErrorStatus() ) //check for errors in parsing rpc data
         {
            finishConstruction();
            
            //---
            // Save current state in RPC model format:
            //---
            rspfString drivePart;
            rspfString pathPart;
            rspfString filePart;
            rspfString extPart;
            filename.split(drivePart,
                           pathPart,
                           filePart,
                           extPart);
            
            rspfFilename init_rpc_geom;
            init_rpc_geom.merge(drivePart,
                                pathPart,
                                INIT_RPC_GEOM_FILENAME,
                                "");

            rspfKeywordlist kwl (init_rpc_geom);
            saveState(kwl);

            // If we get here set the return status to true.
            result = true;

         } // matches: if ( !getErrorStatus() )
   
      } // matches: if( parseHdrData(hdrfile) )

   } // matches:  if ( tiff->open(filename) )
   
   if ( traceExec() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "return status: " << (result?"true\n":"false\n")
         << "DEBUG rspfIkonosRpcModel parseTiffFile: returning..."
         << std::endl;
   }

   return result;
}

bool rspfIkonosRpcModel::isNitf(const rspfFilename& filename)
{
   std::ifstream in(filename.c_str(), ios::in|ios::binary);
   
   if(in)
   {
      char nitfFile[4];
      in.read((char*)nitfFile, 4);

      return (rspfString(nitfFile,
                          nitfFile+4) == "NITF");
   }

   return false;
}
