//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer (rspf port by D. Burken)
//
// Description:
//
// Contains definition of class rspfSpotDimapSupportData.
//
//*****************************************************************************
// $Id: rspfSpotDimapSupportData.cpp 20609 2012-02-27 12:05:13Z gpotts $

#include <rspf/support_data/rspfSpotDimapSupportData.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfXmlAttribute.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>

// Define Trace flags for use within this file:
static rspfTrace traceDebug ("rspfSpotDimapSupportData:debug");

static const rspf_uint32  LAGRANGE_FILTER_SIZE = 8; // num samples considered

rspfSpotDimapSupportData::rspfSpotDimapSupportData ()
   :
   rspfErrorStatusInterface(),
   theMetadataVersion(RSPF_SPOT_METADATA_VERSION_UNKNOWN),
   theImageID(),
   theMetadataFile(),
   theProductionDate(),
   theInstrument(),
   theInstrumentIndex(0),
   theSunAzimuth(0.0),
   theSunElevation(0.0),
   theIncidenceAngle(0.0),
   theViewingAngle(0.0),
   theSceneOrientation(0.0),
   theImageSize(0.0, 0.0),
   theRefGroundPoint(0.0, 0.0, 0.0),
   theRefImagePoint(0.0, 0.0),
   theSubImageOffset(0.0, 0.0),
   theRefLineTime(0.0),
   theRefLineTimeLine(0.0),
   theLineSamplingPeriod(0.0),
   theDetectorCount(0),
   thePixelLookAngleX(),
   thePixelLookAngleY(),
   theAttitudeSamples(),
   theAttSampTimes(),
   thePosEcfSamples(),
   theVelEcfSamples(),
   theEphSampTimes(),
   theStarTrackerUsed(false),
   theSwirDataFlag(false),
   theNumBands(0),
   theAcquisitionDate(),
   theStepCount(0),
   theUlCorner(),
   theUrCorner(),
   theLrCorner(),
   theLlCorner(),
   theGeoPosImagePoints(),
   theGeoPosGroundPoints()
{
}
rspfSpotDimapSupportData::rspfSpotDimapSupportData(const rspfSpotDimapSupportData& rhs)
   :rspfErrorStatusInterface(rhs),
    theMetadataVersion(rhs.theMetadataVersion),
    theImageID(rhs.theImageID),
    theMetadataFile (rhs.theMetadataFile),
    theProductionDate(rhs.theProductionDate),
    theInstrument(rhs.theInstrument),
    theInstrumentIndex(rhs.theInstrumentIndex),
    theSunAzimuth(rhs.theSunAzimuth),
    theSunElevation(rhs.theSunElevation),  
    theIncidenceAngle(rhs.theIncidenceAngle),
    theViewingAngle(rhs.theViewingAngle),
    theSceneOrientation(rhs.theSceneOrientation),
    theImageSize(rhs.theImageSize),
    theRefGroundPoint(rhs.theRefGroundPoint),
    theRefImagePoint(rhs.theRefImagePoint),
    theSubImageOffset(rhs.theSubImageOffset),
    theRefLineTime(rhs.theRefLineTime),
    theRefLineTimeLine(rhs.theRefLineTimeLine),
    theLineSamplingPeriod(rhs.theLineSamplingPeriod),
    theDetectorCount(rhs.theDetectorCount),
    thePixelLookAngleX(rhs.thePixelLookAngleX),
    thePixelLookAngleY(rhs.thePixelLookAngleY),
    theAttitudeSamples(rhs.theAttitudeSamples),
    theAttSampTimes(rhs.theAttSampTimes),
    thePosEcfSamples(rhs.thePosEcfSamples),
    theVelEcfSamples(rhs.theVelEcfSamples),
    theEphSampTimes(rhs.theEphSampTimes),
    theStarTrackerUsed(rhs.theStarTrackerUsed),
    theSwirDataFlag (rhs.theSwirDataFlag),
    theNumBands(rhs.theNumBands),
    theAcquisitionDate(rhs.theAcquisitionDate),
    theStepCount(rhs.theStepCount),
    theUlCorner(rhs.theUlCorner),
    theUrCorner(rhs.theUrCorner),
    theLrCorner(rhs.theLrCorner),
    theLlCorner(rhs.theLlCorner),
    theGeoPosImagePoints(rhs.theGeoPosImagePoints),
    theGeoPosGroundPoints(rhs.theGeoPosGroundPoints)
{
}

rspfSpotDimapSupportData::rspfSpotDimapSupportData (const rspfFilename& dimapFile, bool  processSwir)
   :
   rspfErrorStatusInterface(),
   theMetadataVersion(RSPF_SPOT_METADATA_VERSION_UNKNOWN),
   theImageID(),
   theMetadataFile (dimapFile),
   theProductionDate(),
   theInstrument(),
   theInstrumentIndex(0),
   theSunAzimuth(0.0),
   theSunElevation(0.0),
   theIncidenceAngle(0.0),
   theViewingAngle(0.0),
   theSceneOrientation(0.0),
   theImageSize(0.0, 0.0),
   theRefGroundPoint(0.0, 0.0, 0.0),
   theRefImagePoint(0.0, 0.0),
   theSubImageOffset(0.0, 0.0),
   theRefLineTime(0.0),
   theRefLineTimeLine(0.0),
   theLineSamplingPeriod(0.0),
   theDetectorCount(0),
   thePixelLookAngleX(),
   thePixelLookAngleY(),
   theAttitudeSamples(),
   theAttSampTimes(),
   thePosEcfSamples(),
   theVelEcfSamples(),
   theEphSampTimes(),
   theStarTrackerUsed(false),
   theSwirDataFlag (processSwir),
   theNumBands(0),
   theAcquisitionDate(),
   theStepCount(0),
   theUlCorner(),
   theUrCorner(),
   theLrCorner(),
   theLlCorner(),
   theGeoPosImagePoints(),
   theGeoPosGroundPoints()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSpotDimapSupportData::rspfSpotDimapSupportData: entering..."
         << std::endl;
   }

   loadXmlFile(dimapFile, processSwir);

   // Finished successful parse:
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSpotDimapSupportData::rspfSpotDimapSupportData: leaving..."
         << std::endl;
   }
}

rspfSpotDimapSupportData::~rspfSpotDimapSupportData ()
{
}

rspfObject* rspfSpotDimapSupportData::dup()const
{
   return new rspfSpotDimapSupportData(*this);
}

void rspfSpotDimapSupportData::clearFields()
{
   clearErrorStatus();
   //theSensorID="Spot 5";
   theSensorID="";
   theMetadataVersion = RSPF_SPOT_METADATA_VERSION_UNKNOWN;
   theImageID = "";
   theMetadataFile = "";
   theProductionDate = "";
   theInstrument = "";
   theInstrumentIndex = 0;
   theSunAzimuth = 0.0;
   theSunElevation = 0.0;
   theIncidenceAngle = 0.0;
   theViewingAngle = 0.0;
   theSceneOrientation = 0.0;
   theImageSize.makeNan();
   theRefGroundPoint.makeNan();
   theRefImagePoint.makeNan();
   theSubImageOffset.makeNan();
   theRefLineTime = rspf::nan();
   theRefLineTimeLine = rspf::nan();
   theLineSamplingPeriod = rspf::nan();
   theDetectorCount = 0;
   thePixelLookAngleX.clear();
   thePixelLookAngleY.clear();
   theAttitudeSamples.clear(); // x=pitch, y=roll, z=yaw
   theAttSampTimes.clear();
   thePosEcfSamples.clear();
   theVelEcfSamples.clear();
   theEphSampTimes.clear();
   theStarTrackerUsed = false;
   theSwirDataFlag = false;
   theNumBands = 0;
   theAcquisitionDate = "";
   theStepCount = 0;

   //---
   // Corner points:
   //---
   theUlCorner.makeNan();
   theUrCorner.makeNan();
   theLrCorner.makeNan();
   theLlCorner.makeNan();

   //---
   // Geoposition Points:
   //---
   theGeoPosImagePoints.clear();
   theGeoPosGroundPoints.clear();

}

bool rspfSpotDimapSupportData::loadXmlFile(const rspfFilename& file,
                                            bool processSwir)
{
   static const char MODULE[] = "rspfSpotDimapSupportData::loadXmlFile";

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:"
         << "\nFile: " << file << std::endl;
   }
   clearFields();
   theSwirDataFlag = processSwir;
   theMetadataFile = file;

   rspf_int64 fileSize = file.fileSize();
   std::ifstream in(file.c_str(), std::ios::binary|std::ios::in);
   std::vector<char> fullBuffer;
   rspfString bufferedIo;
   if(in.good()&&(fileSize > 0))
   {
      char buf[100];
      fullBuffer.resize(fileSize);
      in.read(buf, rspf::min((rspf_int64)100, fileSize));
      if(!in.fail())
      {
         rspfString testString = rspfString(buf,
                                              buf + in.gcount());
         if(testString.contains("xml"))
         {
            in.seekg(0);
            in.read(&fullBuffer.front(), (std::streamsize)fullBuffer.size());
            if(!in.fail())
            {
               bufferedIo = rspfString(fullBuffer.begin(),
                                        fullBuffer.begin()+in.gcount());
            }
         }
      }
   }
   else
   {
      return false;
   }
   //---
   // Instantiate the XML parser:
   //---
   rspfRefPtr<rspfXmlDocument> xmlDocument;

   if(bufferedIo.empty())
   {
      xmlDocument = new rspfXmlDocument(file);
   }
   else
   {

      xmlDocument = new rspfXmlDocument;
      std::istringstream inStringStream(bufferedIo.string());
      if(!xmlDocument->read(inStringStream))
      {
         return false;
      }
   }
   if (xmlDocument->getErrorStatus())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << MODULE << " DEBUG:"
            << "rspfSpotDimapSupportData::loadXmlFile:"
            << "\nUnable to parse xml file" << std::endl;
      }
      setErrorStatus();
      return false;
   }

   //---
   // Check that it is a SPOT DIMAP file format
   //---
   vector<rspfRefPtr<rspfXmlNode> > xml_nodes;
   xml_nodes.clear();
   rspfString xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/MISSION";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
	 rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\n Not a SPOT DIMAP file format."<< std::endl;
      }
      return false;
   }
   if ( xml_nodes[0]->getText() != "SPOT" && xml_nodes[0]->getText() != "Spot" && xml_nodes[0]->getText() != "spot" )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\n Not a SPOT DIMAP file format."<< std::endl;
      }
      return false;
   }


   //---
   // Get the version string.  This must be performed first as it is used
   // as a key for parsing different versions.
   //---
   if (initMetadataVersion(xmlDocument) == false)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << MODULE << " DEBUG:"
            << "rspfSpotDimapSupportData::loadXmlFile:"
            << "\nMetadata initialization failed.  Returning false"
            << std::endl;
      }
      return false;
   }

   // Get the image id.
   if (initImageId(xmlDocument) == false)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << MODULE << " DEBUG:"
            << "rspfSpotDimapSupportData::loadXmlFile:"
            << "\nImageId initialization failed.  Returning false"
            << std::endl;
      }
      return false;
   }

   // Get data from "Scene_Source" section.
   if (initSceneSource(xmlDocument)  == false)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " DEBUG:"
         << "rspfSpotDimapSupportData::loadXmlFile:"
         << "\nScene source initialization failed.  Returning false"
         << std::endl;

      return false;
   }

   if (initFramePoints(xmlDocument)  == false)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " DEBUG:"
         << "rspfSpotDimapSupportData::loadXmlFile:"
         << "\nFrame point initialization failed.  Returning false"
         << std::endl;
      return false;
   }

   if (parsePart1(xmlDocument) == false)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " DEBUG:"
         << "rspfSpotDimapSupportData::loadXmlFile:"
         << "\nPart 1 initialization failed.  Returning false"
         << std::endl;
      return false;
   }

   if (parsePart2(xmlDocument) == false)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " DEBUG:"
         << "rspfSpotDimapSupportData::loadXmlFile:"
         << "\nPart 2 initialization failed.  Returning false"
         << std::endl;
      return false;
   }

   if (parsePart3(xmlDocument) == false)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " DEBUG:"
         << "rspfSpotDimapSupportData::loadXmlFile:"
         << "\nPart 3 initialization failed.  Returning false"
         << std::endl;
      return false;
   }

   if (parsePart4(xmlDocument) == false)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " DEBUG:"
         << "rspfSpotDimapSupportData::loadXmlFile:"
         << "\nPart 4 initialization failed.  Returning false"
         << std::endl;
      return false;
   }

   if (traceDebug())
   {
      printInfo(rspfNotify(rspfNotifyLevel_DEBUG));

      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: exited..."
         << std::endl;
   }

   return true;
}

void rspfSpotDimapSupportData::getPositionEcf(rspf_uint32 sample,
                                               rspfEcefPoint& pe)  const
{
   pe.makeNan();

   if (thePosEcfSamples.size() < theDetectorCount)
   {
      if(theImageSize.samp > 0)
      {
         double t = 0.0;
         double tempIdx = 0.0;
         double tempIdxFraction = 0.0;
         t = static_cast<double>(sample)/
            static_cast<double>(theDetectorCount-1);
         tempIdx = (thePosEcfSamples.size()-1)*t;
         tempIdxFraction = tempIdx - (rspf_int32)tempIdx;
         rspf_uint32 idxStart = (rspf_uint32)tempIdx;
         rspf_uint32 idxEnd = (rspf_uint32)ceil(tempIdx);
         if(idxEnd >= thePosEcfSamples.size())
         {
            idxEnd = (rspf_uint32)thePosEcfSamples.size()-1;
         }
         if(idxStart > idxEnd)
         {
            idxStart = idxEnd;
         }
         pe = rspfEcefPoint(thePosEcfSamples[idxStart].x +tempIdxFraction*( thePosEcfSamples[idxEnd].x - thePosEcfSamples[idxStart].x),
                             thePosEcfSamples[idxStart].y +tempIdxFraction*( thePosEcfSamples[idxEnd].y - thePosEcfSamples[idxStart].y),
                             thePosEcfSamples[idxStart].z +tempIdxFraction*( thePosEcfSamples[idxEnd].z - thePosEcfSamples[idxStart].z));

      }
   }
   else if(thePosEcfSamples.size() == theDetectorCount)
   {
      pe = rspfEcefPoint(thePosEcfSamples[sample].x,
                          thePosEcfSamples[sample].y,
                          thePosEcfSamples[sample].z);
   }
}

void rspfSpotDimapSupportData::getPositionEcf(const rspf_float64& time,
                                               rspfEcefPoint& pe)  const
{
   rspfDpt3d tempPt;

   if((thePosEcfSamples.size() < 8)||
      (theEphSampTimes.size() < 8))
   {
      getBilinearInterpolation(time, thePosEcfSamples, theEphSampTimes, tempPt);
   }
   else
   {
      getLagrangeInterpolation(time, thePosEcfSamples, theEphSampTimes, tempPt);
   }

   pe = rspfEcefPoint(tempPt.x,
                       tempPt.y,
                       tempPt.z);
}

void rspfSpotDimapSupportData::getVelocityEcf(rspf_uint32 sample, rspfEcefPoint& ve)  const
{
   ve.makeNan();

   if (theVelEcfSamples.size() < theDetectorCount)
   {
      if(theImageSize.samp > 0)
      {
         double t = 0.0;
         double tempIdx = 0.0;
         double tempIdxFraction = 0.0;
         t = static_cast<double>(sample)/
            static_cast<double>(theDetectorCount-1);
         tempIdx = (theVelEcfSamples.size()-1)*t;
         tempIdxFraction = tempIdx - (rspf_int32)tempIdx;
          rspf_uint32 idxStart = (rspf_uint32)tempIdx;
         rspf_uint32 idxEnd = (rspf_uint32)ceil(tempIdx);
         if(idxEnd >= theVelEcfSamples.size())
         {
            idxEnd = (rspf_uint32)theVelEcfSamples.size()-1;
         }
         if(idxStart > idxEnd)
         {
            idxStart = idxEnd;
         }
         ve = rspfEcefPoint(theVelEcfSamples[idxStart].x +tempIdxFraction*( theVelEcfSamples[idxEnd].x - theVelEcfSamples[idxStart].x),
                             theVelEcfSamples[idxStart].y +tempIdxFraction*( theVelEcfSamples[idxEnd].y - theVelEcfSamples[idxStart].y),
                             theVelEcfSamples[idxStart].z +tempIdxFraction*( theVelEcfSamples[idxEnd].z - theVelEcfSamples[idxStart].z));

      }

   }
   else if(theVelEcfSamples.size() == theDetectorCount)
   {
      ve = rspfEcefPoint(theVelEcfSamples[sample].x,
                          theVelEcfSamples[sample].y,
                          theVelEcfSamples[sample].z);
   }
}

void rspfSpotDimapSupportData::getVelocityEcf(const rspf_float64& time,
                                               rspfEcefPoint& ve)  const
{
   rspfDpt3d tempPt;

   if((theVelEcfSamples.size() < 8) ||
      (theEphSampTimes.size() < 8))
   {
      getBilinearInterpolation (time, theVelEcfSamples, theEphSampTimes, tempPt);
   }
   else
   {
      getLagrangeInterpolation (time, theVelEcfSamples, theEphSampTimes, tempPt);
   }

   ve = rspfEcefPoint(tempPt.x,
                       tempPt.y,
                       tempPt.z);
}

void rspfSpotDimapSupportData::getEphSampTime(rspf_uint32 sample,
                                               rspf_float64& et)  const
{
   et = rspf::nan();
   if(theEphSampTimes.size() < theImageSize.samp)
   {
      if(theImageSize.samp > 0)
      {
         double t = 0.0;
         double tempIdx = 0.0;
         double tempIdxFraction = 0.0;
         t = (double)sample/(double)(theImageSize.samp-1);
         tempIdx = (theEphSampTimes.size()-1)*t;
         tempIdxFraction = tempIdx - (rspf_int32)tempIdx;
         rspf_uint32 idxStart = (rspf_uint32)tempIdx;
         rspf_uint32 idxEnd = (rspf_uint32)ceil(tempIdx);
         if(idxEnd >= theEphSampTimes.size())
         {
            idxEnd = (rspf_uint32)theEphSampTimes.size()-1;
         }
         if(idxStart > idxEnd)
         {
            idxStart = idxEnd;
         }
         et = (theEphSampTimes[idxStart] +tempIdxFraction*(theEphSampTimes[idxEnd] -
                                                           theEphSampTimes[idxStart]));
      }
   }
   else if(theEphSampTimes.size() == theImageSize.samp)
   {
      et = theEphSampTimes[sample];
   }
}

void rspfSpotDimapSupportData::getAttitude(rspf_uint32 sample,
                                            rspfDpt3d& at)  const
{
   if (sample >= theAttitudeSamples.size())
   {
      at.makeNan();
      return;
   }

   at = theAttitudeSamples[sample];
}

void rspfSpotDimapSupportData::getAttitude(const rspf_float64& time,
                                            rspfDpt3d& at)  const
{
   if (theAttSampTimes.empty())
   {
     at.makeNan();
     return;
   }

   if ((time <  theAttSampTimes.front()) ||
       (time >= theAttSampTimes.back() ))
   {
      extrapolateAttitude(time, at);
      return;
   }

   //***
   // Search the attitude sampling time array for surrounding samples:
   //***
   int i=0;
   while ((i < (int)theAttSampTimes.size()) &&
          (theAttSampTimes[i] < time)) ++i;
   --i;

   //***
   // Linearly interpolate attitudes angles:
   //***
   rspf_float64 dt1   = time - theAttSampTimes[i];
   rspf_float64 dt0   = theAttSampTimes[i+1] - time;
   rspf_float64 dt    = theAttSampTimes[i+1] - theAttSampTimes[i];

   at = (theAttitudeSamples[i+1]*dt1 + theAttitudeSamples[i]*dt0)/dt;
}

void rspfSpotDimapSupportData::extrapolateAttitude(const rspf_float64& time, rspfDpt3d& at) const
{
   at.makeNan();
   int last_samp = (int) theAttSampTimes.size() - 1;
   if (last_samp < 1)
      return;

   rspfDpt3d dAtt, dAtt_dt;
   double dt, delta_t;

   // Determine whether extrapolating at the front or the back of the range:
   if (time < theAttSampTimes.front())
   {
      dt = theAttSampTimes[1] - theAttSampTimes[0];
      dAtt = theAttitudeSamples[1] - theAttitudeSamples[0];
      dAtt_dt = dAtt/dt;
      delta_t = time - theAttSampTimes[0];
      at = theAttitudeSamples[0] + (dAtt_dt*delta_t);
   }
   else if (time >= theAttSampTimes.back())
   {
      dt = theAttSampTimes[last_samp] - theAttSampTimes[last_samp-1];
      dAtt = theAttitudeSamples[last_samp] - theAttitudeSamples[last_samp-1];
      dAtt_dt = dAtt/dt;
      delta_t = time - theAttSampTimes[last_samp];
      at = theAttitudeSamples[last_samp] + (dAtt_dt*delta_t);
   }

   return;
}

void rspfSpotDimapSupportData::getAttSampTime(rspf_uint32 sample, rspf_float64& at)  const
{
   if (sample >= theAttSampTimes.size())
   {
      at = rspf::nan();
      return;
   }

   at = theAttSampTimes[sample];
}

void rspfSpotDimapSupportData::getPixelLookAngleX(rspf_uint32 sample,
                                                   rspf_float64& pa) const
{
   if (sample >= thePixelLookAngleX.size())
   {
      setErrorStatus();
      pa = rspf::nan();

      return;
   }

   pa = thePixelLookAngleX[sample];
}

void rspfSpotDimapSupportData::getPixelLookAngleX(const rspf_float64& sample,
                                                   rspf_float64& pa) const
{
   rspf_uint32 s = static_cast<rspf_uint32>(sample);
   getInterpolatedLookAngle(s, thePixelLookAngleX, pa);
}

void rspfSpotDimapSupportData::getPixelLookAngleY(rspf_uint32 sample,
                                                   rspf_float64& pa) const
{
   if (sample >= thePixelLookAngleY.size())
   {
      setErrorStatus();
      pa = rspf::nan();
      return;
   }

   pa = thePixelLookAngleY[sample];
}

void rspfSpotDimapSupportData::getPixelLookAngleY(const rspf_float64& sample,
                                                   rspf_float64& pa) const
{
   rspf_uint32 s = static_cast<rspf_uint32>(sample);
   getInterpolatedLookAngle(s, thePixelLookAngleY, pa);
}

void rspfSpotDimapSupportData::getInterpolatedLookAngle(
   const rspf_float64& p,
   const std::vector<rspf_float64>& angles,
   rspf_float64& la) const
{
   if ((p < 0.0) || (p >= (rspf_float64) angles.size()))
   {
      setErrorStatus();
      la = rspf::nan();
      return;
   }

   rspf_float64 p0 = floor(p);
   rspf_float64 p1 = ceil (p);

   if (p0 == p1)
   {
      la = angles[(int) p0];
   }
   else
   {
      rspf_float64 angle_0 = angles[(int) p0];
      rspf_float64 angle_1 = angles[(int) p1];

      la = (angle_0*(p1-p) + angle_1*(p-p0))/(p1-p0);
   }
}

void rspfSpotDimapSupportData::getBilinearInterpolation(
   const rspf_float64& time,
   const std::vector<rspfDpt3d>& V,
   const std::vector<rspf_float64>& T,
   rspfDpt3d& li) const
{
   rspf_uint32 samp0 = 0;
   while ((samp0 < T.size()) && (T[samp0] < time)) ++samp0;

   if(samp0==0)
   {
      li = V[0];
   }
   else if(samp0 == T.size())
   {
      li = V[1];
   }
   else
   {
      double t = (T[samp0-1]-time)/(T[samp0-1] - T[samp0]);

      li = V[samp0-1] + (V[samp0]-V[samp0-1])*t;
   }
}

void rspfSpotDimapSupportData::getLagrangeInterpolation(
   const rspf_float64& time,
   const std::vector<rspfDpt3d>& V,
   const std::vector<rspf_float64>& T,
   rspfDpt3d& li) const

{
//    std::cout << "V size = " << V.size() << std::endl
//              << "T size = " << T.size() << std::endl;

   rspf_uint32 filter_size = 8;
   //
   // Verify that t is within allowable range:
   //
   rspf_uint32 lagrange_half_filter = 4;

   if(T.size() <= filter_size)
   {
      filter_size = (rspf_uint32)T.size()/2;
      lagrange_half_filter = filter_size/2;
   }
   if ((time <  T[lagrange_half_filter]) ||
       (time >= T[T.size()-lagrange_half_filter] ))
   {
      setErrorStatus();
      li.makeNan();

      return;
   }

   //***
   // Search the sampling time array for surrounding samples:
   //***
   rspf_uint32 samp0 = lagrange_half_filter;
   while ((samp0 < T.size()) && (T[samp0] < time)) ++samp0;

   //***
   // Do not use sample if it falls in neighborhood of desired time:
   //***
   rspf_uint32 bump = 0;
   if (fabs(T[samp0] - time) < theLineSamplingPeriod/2.0)
      bump = 1;

   samp0 -= lagrange_half_filter; // adjust to first sample in window

   //***
   // Outer summation loop:
   //***
   rspfDpt3d S (0, 0, 0);
   for (rspf_uint32 j=samp0; j<(samp0+filter_size+bump); ++j)
   {
      rspf_float64 numerator   = 1.0;
      rspf_float64 denominator = 1.0;

      //***
      // Skip this sample if too close to desired time:
      //***
      if (bump && (j == (samp0+lagrange_half_filter) ))
         ++j;

      //***
      // Inner loop for product series:
      //***
      for (rspf_uint32 i=samp0; i<(samp0+filter_size+bump); ++i)
      {
         //***
         // Skip this sample if too close to desired time:
         //***
         if (bump && (i == (samp0+lagrange_half_filter) ))
            ++i;

         if (i != j)
         {
            numerator   *= time - T[i];
            denominator *= T[j] - T[i];
         }
      }

      rspfDpt3d p = V[j];
      p = p * numerator;
      p = p / denominator;
      S += p;
   }

   li = S;
}

rspf_float64 rspfSpotDimapSupportData::convertTimeStamp(const rspfString& time_stamp) const
{
   double ti;
   convertTimeStamp(time_stamp, ti);
   return ti;
}

void rspfSpotDimapSupportData::convertTimeStamp(const rspfString& time_stamp,
                                                 rspf_float64& ti) const
{
   int    year, month, day, hour, minute;
   double second;

   //***
   // Time stamps are in the format: "yyyy-mm-ddThh:mm:ss.ssssss"
   //***
   int converted = sscanf(time_stamp,
                          "%4d-%2d-%2dT%2d:%2d:%9lf",
                          &year, &month, &day,
                          &hour, &minute, &second);

   if (converted != 6)
   {
      setErrorStatus();
      ti = rspf::nan();
   }
   else
   {
      ti = (((((year-2002.0)*12.0 + month - 1.0)*365.0 + day - 1.0)*24.0
             + hour)*60.0 + minute)*60.0 + second;
   }
}

void rspfSpotDimapSupportData::getGeoPosPoint (rspf_uint32 point,
                                                rspfDpt& ip,
                                                rspfGpt& gp) const
{
   if (point < theGeoPosImagePoints.size())
   {
      ip = theGeoPosImagePoints [point];
      gp = theGeoPosGroundPoints[point];
   }
}

void rspfSpotDimapSupportData::printInfo(ostream& os) const
{
   rspfString corr_att = "NO";
   if (theStarTrackerUsed)
      corr_att = "YES";

   os << "\n----------------- Info on SPOT5 Image -------------------"
      << "\n  "
      << "\n  Job Number (ID):      " << theImageID
      << "\n  Acquisition Date:     " << theAcquisitionDate
      << "\n  Instrument:           " << theInstrument
      << "\n  Instrument Index:     " << theInstrumentIndex
      << "\n  Production Date:      " << theProductionDate
      << "\n  Number of Bands:      " << theNumBands
      << "\n  Geo Center Point:     " << theRefGroundPoint
      << "\n  Detector count:       " << theDetectorCount
      << "\n  Image Size:           " << theImageSize
      << "\n  Incidence Angle:      " << theIncidenceAngle
      << "\n  Viewing Angle:        " << theViewingAngle      
      << "\n  Scene Orientation:    " << theSceneOrientation 
      << "\n  Corrected Attitude:   " << corr_att
      << "\n  Sun Azimuth:          " << theSunAzimuth
      << "\n  Sun Elevation:        " << theSunElevation
      << "\n  Sub image offset:     " << theSubImageOffset
      << "\n  Step Count:           " << theStepCount
      << "\n  PixelLookAngleX size: " << thePixelLookAngleX.size()
      << "\n  thePosEcfSamples size:" << thePosEcfSamples.size()
      << "\n  Corner Points:"
      << "\n     UL: " << theUlCorner
      << "\n     UR: " << theUrCorner
      << "\n     LR: " << theLrCorner
      << "\n     LL: " << theLlCorner
      << "\n"
      << "\n---------------------------------------------------------"
      << "\n  " << std::endl;
}

rspfString rspfSpotDimapSupportData::getSensorID() const
{
  return theSensorID;
}

rspfString   rspfSpotDimapSupportData::getMetadataVersionString() const
{
   if (theMetadataVersion == RSPF_SPOT_METADATA_VERSION_1_1)
   {
      return rspfString("1.1");
   }
   else if (theMetadataVersion == RSPF_SPOT_METADATA_VERSION_1_0)
   {
      return rspfString("1.0");
   }
   return rspfString("unknown");
}

rspfString rspfSpotDimapSupportData::getAcquisitionDate() const
{
   return theAcquisitionDate;
}

rspfString rspfSpotDimapSupportData::getProductionDate() const
{
   return theProductionDate;
}

rspfString rspfSpotDimapSupportData::getImageID() const
{
   return theImageID;
}

rspfFilename rspfSpotDimapSupportData::getMetadataFile() const
{
   return theMetadataFile;
}

rspfString rspfSpotDimapSupportData::getInstrument() const
{
   return theInstrument;
}

rspf_uint32 rspfSpotDimapSupportData::getInstrumentIndex() const
{
   return theInstrumentIndex;
}

void rspfSpotDimapSupportData::getSunAzimuth(rspf_float64& az) const
{
   az = theSunAzimuth;
}

void rspfSpotDimapSupportData::getSunElevation(rspf_float64& el) const
{
   el = theSunElevation;
}

void rspfSpotDimapSupportData::getImageSize(rspfDpt& sz) const
{
   sz = theImageSize;
}

void rspfSpotDimapSupportData::getLineSamplingPeriod(rspf_float64& pe) const
{
   pe = theLineSamplingPeriod;
}

bool rspfSpotDimapSupportData::isStarTrackerUsed() const
{
   return theStarTrackerUsed;
}

bool rspfSpotDimapSupportData::isSwirDataUsed() const
{
   return theSwirDataFlag;
}

rspf_uint32 rspfSpotDimapSupportData::getNumberOfBands() const
{
   return theNumBands;
}

rspf_uint32 rspfSpotDimapSupportData::getStepCount() const
{
   return theStepCount;
}

void rspfSpotDimapSupportData::getIncidenceAngle(rspf_float64& ia) const
{
   ia = theIncidenceAngle;
}

void rspfSpotDimapSupportData::getViewingAngle(rspf_float64& va) const
{
   va = theViewingAngle;
}

void rspfSpotDimapSupportData::getSceneOrientation(rspf_float64& so) const
{
   so = theSceneOrientation;
}

void rspfSpotDimapSupportData::getRefGroundPoint(rspfGpt& gp) const
{
   gp = theRefGroundPoint;
}

void rspfSpotDimapSupportData::getRefImagePoint(rspfDpt& rp) const
{
   rp = theRefImagePoint;
}

void rspfSpotDimapSupportData::getRefLineTime(rspf_float64& rt) const
{
   rt = theRefLineTime;
}

void rspfSpotDimapSupportData::getRefLineTimeLine(rspf_float64& rtl) const
{
   rtl = theRefLineTimeLine;
}

rspf_uint32 rspfSpotDimapSupportData::getNumEphSamples() const
{
   return (rspf_uint32)theEphSampTimes.size();
}

rspf_uint32 rspfSpotDimapSupportData::getNumAttSamples() const
{
   return (rspf_uint32)theAttSampTimes.size();
}

rspf_uint32 rspfSpotDimapSupportData::getNumGeoPosPoints() const
{
   return (rspf_uint32)theGeoPosImagePoints.size();
}

void rspfSpotDimapSupportData::getUlCorner(rspfGpt& pt) const
{
   pt = theUlCorner;
}

void rspfSpotDimapSupportData::getUrCorner(rspfGpt& pt) const
{
   pt = theUrCorner;
}

void rspfSpotDimapSupportData::getLrCorner(rspfGpt& pt) const
{
   pt = theLrCorner;
}

void rspfSpotDimapSupportData::getLlCorner(rspfGpt& pt) const
{
   pt = theLlCorner;
}

void rspfSpotDimapSupportData::getImageRect(rspfDrect& rect)const
{
   rect = rspfDrect(0.0, 0.0, theImageSize.x-1.0, theImageSize.y-1.0);
}

void rspfSpotDimapSupportData::getSubImageOffset(rspfDpt& offset) const
{
   offset = theSubImageOffset;
}

bool rspfSpotDimapSupportData::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfSpotDimapSupportData",
           true);

   kwl.add(prefix,
           "metadata_file",
           theMetadataFile,
           true);

   kwl.add(prefix,
           rspfKeywordNames::AZIMUTH_ANGLE_KW,
           theSunAzimuth,
           true);

   kwl.add(prefix,
           rspfKeywordNames::ELEVATION_ANGLE_KW,
           theSunElevation,
           true);

   //---
   // Note: since this is a new keyword, use the point.toString as there is
   // no backwards compatibility issues.
   //---
   kwl.add(prefix,
           "detector_count",
           theDetectorCount,
           true);

   kwl.add(prefix,
           "image_size",
           rspfString::toString(theImageSize.x) + " " +
           rspfString::toString(theImageSize.y),
           true);

   kwl.add(prefix,
           "reference_ground_point",
           rspfString::toString(theRefGroundPoint.latd()) + " " +
           rspfString::toString(theRefGroundPoint.lond()) + " " +
           rspfString::toString(theRefGroundPoint.height()) + " " +
           theRefGroundPoint.datum()->code(),
           true);

   kwl.add(prefix,
           "reference_image_point",
           rspfString::toString(theRefImagePoint.x) + " " +
           rspfString::toString(theRefImagePoint.y),
           true);

   kwl.add(prefix,
           "sub_image_offset",
           rspfString::toString(theSubImageOffset.x) + " " +
           rspfString::toString(theSubImageOffset.y),
           true);

   kwl.add(prefix,
           "reference_line_time",
           theRefLineTime,
           true);

   kwl.add(prefix,
           "reference_line_time_line",
           theRefLineTimeLine,
           true);

   kwl.add(prefix,
           "line_sampling_period",
           theLineSamplingPeriod,
           true);

   rspfString tempString;
   rspf_uint32 idx = 0;

   tempString = "";
   for(idx = 0; idx < thePixelLookAngleX.size(); ++idx)
   {
      tempString += (rspfString::toString(thePixelLookAngleX[idx]) + " ");
   }

   kwl.add(prefix,
           "pixel_lookat_angle_x",
           tempString,
           true);

   kwl.add(prefix,
           "number_of_pixel_lookat_angle_x",
           static_cast<rspf_uint32>(thePixelLookAngleX.size()),
           true);

   tempString = "";
   for(idx = 0; idx < thePixelLookAngleY.size(); ++idx)
   {
      tempString += (rspfString::toString(thePixelLookAngleY[idx]) + " ");
   }
   kwl.add(prefix,
           "pixel_lookat_angle_y",
           tempString,
           true);
   kwl.add(prefix,
           "number_of_pixel_lookat_angle_y",
           static_cast<rspf_uint32>(thePixelLookAngleY.size()),
           true);


   tempString = "";
   for(idx = 0; idx < theAttitudeSamples.size(); ++idx)
   {
      tempString += (rspfString::toString(theAttitudeSamples[idx].x) + " " +
                     rspfString::toString(theAttitudeSamples[idx].y) + " " +
                     rspfString::toString(theAttitudeSamples[idx].z) + " ");
   }
   kwl.add(prefix,
           "attitude_samples",
           tempString,
           true);
   kwl.add(prefix,
           "number_of_attitude_samples",
           static_cast<rspf_uint32>(theAttitudeSamples.size()),
           true);

   tempString = "";
   for(idx = 0; idx < theAttSampTimes.size(); ++idx)
   {
      tempString += (rspfString::toString(theAttSampTimes[idx]) + " ");
   }
   kwl.add(prefix,
           "attitude_sample_times",
           tempString,
           true);
   kwl.add(prefix,
           "number_of_attitude_sample_times",
           static_cast<rspf_uint32>(theAttSampTimes.size()),
           true);

   tempString = "";
   for(idx = 0; idx < thePosEcfSamples.size(); ++idx)
   {
      tempString += (rspfString::toString(thePosEcfSamples[idx].x) + " " +
                     rspfString::toString(thePosEcfSamples[idx].y) + " " +
                     rspfString::toString(thePosEcfSamples[idx].z) + " ");
   }
   kwl.add(prefix,
           "position_ecf_samples",
           tempString,
           true);
   kwl.add(prefix,
           "number_of_position_ecf_samples",
           static_cast<rspf_uint32>(thePosEcfSamples.size()),
           true);

   tempString = "";
   for(idx = 0; idx < theVelEcfSamples.size(); ++idx)
   {
      tempString += (rspfString::toString(theVelEcfSamples[idx].x) + " " +
                     rspfString::toString(theVelEcfSamples[idx].y) + " " +
                     rspfString::toString(theVelEcfSamples[idx].z) + " ");
   }
   kwl.add(prefix,
           "velocity_ecf_samples",
           tempString,
           true);
   kwl.add(prefix,
           "number_of_velocity_ecf_samples",
           static_cast<rspf_uint32>(thePosEcfSamples.size()),
           true);

   tempString = "";
   for(idx = 0; idx < theEphSampTimes.size(); ++idx)
   {
      tempString += (rspfString::toString(theEphSampTimes[idx]) + " ");
   }

   kwl.add(prefix,
           "ephemeris_sample_times",
           tempString,
           true);
   kwl.add(prefix,
           "number_of_ephemeris_sample_times",
           static_cast<rspf_uint32>(theEphSampTimes.size()),
           true);

   kwl.add(prefix,
           "star_tracker_used_flag",
           static_cast<rspf_uint32>(theStarTrackerUsed),
           true);

   kwl.add(prefix,
           "swir_data_flag",
           static_cast<rspf_uint32>(theSwirDataFlag),
           true);

   kwl.add(prefix,
           rspfKeywordNames::NUMBER_BANDS_KW,
           theNumBands,
           true);

   kwl.add(prefix,
           "image_id",
           theImageID,
           true);

   kwl.add(prefix,
           "instrument",
           theInstrument,
           true);

   kwl.add(prefix,
           "instrument_index",
           theInstrumentIndex,
           true);

   kwl.add(prefix,
           rspfKeywordNames::IMAGE_DATE_KW,
           theAcquisitionDate,
           true);

   kwl.add(prefix,
           "production_date",
           theProductionDate,
           true);

   kwl.add(prefix,
           "incident_angle",
           theIncidenceAngle,
           true);

   kwl.add(prefix,
           "viewing_angle",
           theViewingAngle,
           true);

   kwl.add(prefix,
           "scene_orientation",
           theSceneOrientation,
           true);
                      
   kwl.add(prefix,
           "step_count",
           theStepCount,
           true);
           
   kwl.add(prefix,
           "ul_ground_point",
           rspfString::toString(theUlCorner.latd()) + " " +
           rspfString::toString(theUlCorner.lond()) + " " +
           rspfString::toString(theUlCorner.height()) + " " +
           theUlCorner.datum()->code(),
           true);

   kwl.add(prefix,
           "ur_ground_point",
           rspfString::toString(theUrCorner.latd()) + " " +
           rspfString::toString(theUrCorner.lond()) + " " +
           rspfString::toString(theUrCorner.height()) + " " +
           theUrCorner.datum()->code(),
           true);

   kwl.add(prefix,
           "lr_ground_point",
           rspfString::toString(theLrCorner.latd()) + " " +
           rspfString::toString(theLrCorner.lond()) + " " +
           rspfString::toString(theLrCorner.height()) + " " +
           theLrCorner.datum()->code(),
           true);

   kwl.add(prefix,
           "ll_ground_point",
           rspfString::toString(theLlCorner.latd()) + " " +
           rspfString::toString(theLlCorner.lond()) + " " +
           rspfString::toString(theLlCorner.height()) + " " +
           theLlCorner.datum()->code(),
           true);

   kwl.add(prefix,
           "sensorID",
           theSensorID,
           true);


   tempString = "";
   for(idx = 0; idx < thePhysicalBias.size(); ++idx)
   {
     tempString += (rspfString::toString(thePhysicalBias[idx]) + " ");
   }
   kwl.add(prefix,
           "physical_bias",
           tempString,
           true);

   tempString = "";
   for(idx = 0; idx < thePhysicalGain.size(); ++idx)
   {
     tempString += (rspfString::toString(thePhysicalGain[idx]) + " ");
   }
   kwl.add(prefix,
           "physical_gain",
           tempString,
           true);

   tempString = "";
   for(idx = 0; idx < theSolarIrradiance.size(); ++idx)
   {
     tempString += (rspfString::toString(theSolarIrradiance[idx]) + " ");
   }

   kwl.add(prefix,
           "solar_irradiance",
           tempString,
           true);

   return true;
}

bool rspfSpotDimapSupportData::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   clearFields();

   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);

   if(type != "rspfSpotDimapSupportData")
   {
      return false;
   }
   theMetadataFile = kwl.find(prefix, "metadata_file");

   theSunAzimuth   = rspfString(kwl.find(prefix, rspfKeywordNames::AZIMUTH_ANGLE_KW)).toDouble();
   theSunElevation = rspfString(kwl.find(prefix, rspfKeywordNames::ELEVATION_ANGLE_KW)).toDouble();

   const char* lookup = kwl.find(prefix, "detector_count");
   if (lookup)
   {
      theDetectorCount = rspfString(lookup).toUInt32();
   }

   theImageSize      = createDpt(kwl.find(prefix, "image_size"));
   theRefGroundPoint = createGround(kwl.find(prefix, "reference_ground_point"));
   theRefImagePoint  = createDpt(kwl.find(prefix, "reference_image_point"));
   theSubImageOffset = createDpt(kwl.find(prefix, "sub_image_offset"));

   theRefLineTime    = rspfString(kwl.find(prefix, "reference_line_time")).toDouble();

   lookup = kwl.find(prefix, "reference_line_time_line");
   if (lookup)
   {
      theRefLineTimeLine = rspfString(lookup).toDouble();
   }

   theLineSamplingPeriod = rspfString(kwl.find(prefix, "line_sampling_period")).toDouble();


   rspf_uint32 idx = 0;
   rspf_uint32 total =  rspfString(kwl.find(prefix,"number_of_pixel_lookat_angle_x")).toUInt32();
   rspfString tempString;

   thePixelLookAngleX.resize(total);
   tempString = kwl.find(prefix,"pixel_lookat_angle_x");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < thePixelLookAngleX.size();++idx)
      {
         in >> tempValue.string();
         thePixelLookAngleX[idx] = tempValue.toDouble();
      }
   }

   total =  rspfString(kwl.find(prefix,"number_of_pixel_lookat_angle_y")).toUInt32();
   thePixelLookAngleY.resize(total);
   tempString = kwl.find(prefix,"pixel_lookat_angle_y");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < thePixelLookAngleY.size();++idx)
      {
         in >> tempValue.string();
         thePixelLookAngleY[idx] = tempValue.toDouble();
      }
   }

   total =  rspfString(kwl.find(prefix,"number_of_attitude_samples")).toUInt32();
   theAttitudeSamples.resize(total);
   tempString = kwl.find(prefix,"attitude_samples");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString x, y, z;
      for(idx = 0; idx < theAttitudeSamples.size();++idx)
      {
         in >> x.string() >> y.string() >> z.string();
         theAttitudeSamples[idx] =rspfDpt3d(x.toDouble(), y.toDouble(), z.toDouble());
      }
   }

   total =  rspfString(kwl.find(prefix,"number_of_attitude_sample_times")).toUInt32();
   theAttSampTimes.resize(total);
   tempString = kwl.find(prefix,"attitude_sample_times");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < theAttSampTimes.size();++idx)
      {
         in >> tempValue.string();
         theAttSampTimes[idx] = tempValue.toDouble();
      }
   }

   total =  rspfString(kwl.find(prefix,"number_of_position_ecf_samples")).toUInt32();
   thePosEcfSamples.resize(total);
   tempString = kwl.find(prefix,"position_ecf_samples");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString x, y, z;
      for(idx = 0; idx < thePosEcfSamples.size();++idx)
      {
         in >> x.string() >> y.string() >> z.string();
         thePosEcfSamples[idx] = rspfDpt3d(x.toDouble(), y.toDouble(), z.toDouble());
      }
   }

   total =  rspfString(kwl.find(prefix,"number_of_velocity_ecf_samples")).toUInt32();
   theVelEcfSamples.resize(total);
   tempString = kwl.find(prefix,"velocity_ecf_samples");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString x, y, z;
      for(idx = 0; idx < theVelEcfSamples.size();++idx)
      {
         in >> x.string() >> y.string() >> z.string();
         theVelEcfSamples[idx] = rspfDpt3d(x.toDouble(), y.toDouble(), z.toDouble());
      }
   }

   total =  rspfString(kwl.find(prefix,"number_of_ephemeris_sample_times")).toUInt32();
   theEphSampTimes.resize(total);
   tempString = kwl.find(prefix,"ephemeris_sample_times");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < theEphSampTimes.size();++idx)
      {
         in >> tempValue.string();
         theEphSampTimes[idx] = tempValue.toDouble();
      }
   }

   tempString = "";
   for(idx = 0; idx < theEphSampTimes.size(); ++idx)
   {
      tempString += (rspfString::toString(theEphSampTimes[idx]) + " ");
   }

   theStarTrackerUsed = rspfString(kwl.find(prefix, "star_tracker_used_flag")).toBool();
   theSwirDataFlag    = rspfString(kwl.find(prefix, "swir_data_flag")).toBool();
   theNumBands        = rspfString(kwl.find(prefix, rspfKeywordNames::NUMBER_BANDS_KW)).toUInt32();
   theAcquisitionDate = kwl.find(prefix, rspfKeywordNames::IMAGE_DATE_KW);
   theProductionDate  = kwl.find(prefix, "production_date");
   theImageID         = kwl.find(prefix, "image_id");
   theInstrument      = kwl.find(prefix, "instrument");
   theInstrumentIndex = rspfString(kwl.find(prefix, "instrument_index")).toUInt32();
   theStepCount       = rspfString(kwl.find(prefix, "step_count")).toInt32();
   
   theIncidenceAngle  = rspfString(kwl.find(prefix, "incident_angle")).toDouble();
   theViewingAngle    = rspfString(kwl.find(prefix, "viewing_angle")).toDouble();
   theSceneOrientation= rspfString(kwl.find(prefix, "scene_orientation")).toDouble();
   
   theUlCorner =createGround( kwl.find(prefix, "ul_ground_point"));
   theUrCorner =createGround( kwl.find(prefix, "ur_ground_point"));
   theLrCorner =createGround( kwl.find(prefix, "lr_ground_point"));
   theLlCorner =createGround( kwl.find(prefix, "ll_ground_point"));

   theSensorID = rspfString(kwl.find(prefix, "sensorID"));

   thePhysicalBias.resize(theNumBands);
   tempString = kwl.find(prefix,"physical_bias");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < thePhysicalBias.size();++idx)
      {
         in >> tempValue.string();
         thePhysicalBias[idx] = tempValue.toDouble();
      }
   }

   thePhysicalGain.resize(theNumBands);
   tempString = kwl.find(prefix,"physical_gain");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < thePhysicalGain.size();++idx)
      {
         in >> tempValue.string();
         thePhysicalGain[idx] = tempValue.toDouble();
      }
   }

   theSolarIrradiance.resize(theNumBands);
   tempString = kwl.find(prefix,"solar_irradiance");
   if(tempString != "")
   {
      std::istringstream in(tempString.string());
      rspfString tempValue;
      for(idx = 0; idx < theSolarIrradiance.size();++idx)
      {
         in >> tempValue.string();
         theSolarIrradiance[idx] = tempValue.toDouble();
      }
   }

   return true;
}

rspfGpt rspfSpotDimapSupportData::createGround(const rspfString& s)const
{
   std::istringstream in(s.string());
   rspfString lat, lon, height;
   rspfString code;

   in >> lat.string() >> lon.string() >> height.string() >> code.string();

   return rspfGpt(lat.toDouble(),
                   lon.toDouble(),
                   height.toDouble(),
                   rspfDatumFactory::instance()->create(code));

}

rspfDpt rspfSpotDimapSupportData::createDpt(const rspfString& s)const
{
   std::istringstream in(s.string());
   rspfString x, y;
   rspfString code;

   in >> x.string() >> y.string();

   return rspfDpt(x.toDouble(), y.toDouble());

}

bool rspfSpotDimapSupportData::parsePart1(
   rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   static const char MODULE[] = "rspfSpotDimapSupportData::parsePart1";

   rspfString xpath;
   vector<rspfRefPtr<rspfXmlNode> > xml_nodes;


   //---
   // Fetch the ImageSize:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Raster_Dimensions/NCOLS";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theImageSize.samp = xml_nodes[0]->getText().toDouble();

   xml_nodes.clear();
   xpath = "/Dimap_Document/Raster_Dimensions/NROWS";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theImageSize.line = xml_nodes[0]->getText().toDouble();

   if (theSwirDataFlag)
   {
      theImageSize.line /= 2.0;
      theImageSize.samp /= 2.0;
   }

   //---
   // We will make the RefImagePoint the zero base center of the image.  This
   // is used by the rspfSensorModel::worldToLineSample iterative loop as
   // the starting point.  Since the rspfSensorModel does not know of the
   // sub image we make it zero base.
   //---
   theRefImagePoint.line = theImageSize.line / 2.0;
   theRefImagePoint.samp = theImageSize.samp / 2.0;

   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Sensor_Configuration/Time_Stamp/SCENE_CENTER_LINE";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }

   // Relative to full image frame.
   theRefLineTimeLine = xml_nodes[0]->getText().toDouble() - 1.0;

   // See if there's a sub image offset...
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Processing/Regions_Of_Interest/Region_Of_Interest/COL_MIN";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      theSubImageOffset.samp = 0.0;
   }
   else
   {
      theSubImageOffset.samp = xml_nodes[0]->getText().toDouble() - 1.0;
   }

   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Processing/Regions_Of_Interest/Region_Of_Interest/ROW_MIN";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      theSubImageOffset.line = 0.0;
   }
   else
   {
      theSubImageOffset.line = xml_nodes[0]->getText().toDouble() - 1.0;
   }


   if (theSwirDataFlag)
   {
      theRefImagePoint.line /= 2.0;
      theRefImagePoint.samp /= 2.0;
   }

   //---
   // Fetch the RefLineTime:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Sensor_Configuration/Time_Stamp/SCENE_CENTER_TIME";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theAcquisitionDate = xml_nodes[0]->getText();
   convertTimeStamp(theAcquisitionDate, theRefLineTime);

   //---
   // Fetch the ProductionDate:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Production/DATASET_PRODUCTION_DATE";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theProductionDate = xml_nodes[0]->getText();
   
   //---
   // Fetch the Instrument:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/INSTRUMENT";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theInstrument = xml_nodes[0]->getText();

   //---
   // Fetch the Instrument Index:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/INSTRUMENT_INDEX";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theInstrumentIndex = xml_nodes[0]->getText().toUInt32();

   return true;
}

bool rspfSpotDimapSupportData::parsePart2(
   rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   static const char MODULE[] = "rspfSpotDimapSupportData::parsePart2";

   rspfString xpath;
   std::vector<rspfRefPtr<rspfXmlNode> > xml_nodes;
   std::vector<rspfRefPtr<rspfXmlNode> > sub_nodes;
   std::vector<rspfRefPtr<rspfXmlNode> >::iterator node;
   unsigned int band_index;

   //---
   // Fetch the LineSamplingPeriod:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Sensor_Configuration/Time_Stamp/LINE_PERIOD";

   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theLineSamplingPeriod = xml_nodes[0]->getText().toDouble();

   if (theSwirDataFlag)
   {
      theLineSamplingPeriod *= 2.0;
   }

   //---
   // Fetch number of bands
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Raster_Dimensions/NBANDS";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theNumBands = atoi(xml_nodes[0]->getText());

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:"
         << "\nNumber of bands: " << theNumBands
         << std::endl;

   }

   if (theNumBands == 1)
   {
      if (theSwirDataFlag)
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << MODULE << " DEBUG:"
               << "\nSWIR band error..."
               << std::endl;
         }
         return false;
      }
      band_index = 0;
   }
   else if (theNumBands == 3)
   {
      band_index = 0; // using green band for PSI angles
   }
   else if (theNumBands == 4)
   {
      if (theSwirDataFlag)
      {
         band_index = 3;
      }
      else
      {
         band_index = 1; // using green band for PSI angles
      }
   }
   else
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nBand ERROR!"
            << std::endl;
      }
      return false;
   }

   //---
   // Fetch the PixelLookAngleX and PixelLookAngleY arrays. If MS, then the
   // green band PSI angles are used unless SWIR requested:
   //---
   thePixelLookAngleX.clear();
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Sensor_Configuration/"
      "Instrument_Look_Angles_List/Instrument_Look_Angles/";
   xmlDocument->findNodes(xpath, xml_nodes);
//   if (xml_nodes.size() != theNumBands)
//   {
      if(xml_nodes.size() == 0)
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << MODULE << " DEBUG:"
               << "\nCould not find: " << xpath
               << std::endl;
         }
         return false;
      }
//       else
//       {
//       }

//   }

   xpath = "Look_Angles_List/Look_Angles/PSI_X";
   sub_nodes.clear();
   xml_nodes[band_index]->findChildNodes(xpath, sub_nodes);

   theDetectorCount = (rspf_uint32)sub_nodes.size();

   if (theMetadataVersion == RSPF_SPOT_METADATA_VERSION_1_1)
   {
      for (rspf_uint32 i=0; i<theDetectorCount; ++i)
      {
         thePixelLookAngleX.push_back(sub_nodes[i]->getText().toDouble());
      }
   }
   else if (sub_nodes.size() != theImageSize.samp)
   {
      // theFullImageSize.samp = sub_nodes.size();
//       if ((theSubImageOffset.samp + theImageSize.samp - 1)<= sub_nodes.size())
//       {
//          rspf_uint32 i = theSubImageOffset.samp;
//          for (rspf_uint32 idx 0; idx<theImageSize.samp; ++idx)
//          {
//             thePixelLookAngleX.push_back(sub_nodes[i]->getText().toDouble());
//             ++i;
//          }
//       }
//       else
//       {
      std::vector<double> tempV(sub_nodes.size());
      double t = 0.0;
      double tempIdx = 0.0;
      double tempIdxFraction = 0.0;
      rspf_int32 idxStart = 0;
      rspf_int32 idxEnd = 0;
      rspf_uint32 idx = 0;
      for(idx = 0; idx < sub_nodes.size();++idx)
      {
         tempV[idx] = sub_nodes[idx]->getText().toDouble();
      }
      for(idx = 0; idx < theImageSize.samp; ++idx)
      {
         t = (double)idx/(double)(theImageSize.samp);
         tempIdx = (sub_nodes.size()-1)*t;
         tempIdxFraction = tempIdx - (rspf_int32)tempIdx;
         idxStart = (rspf_int32)tempIdx;
         idxEnd = (rspf_int32)ceil(tempIdx);
         if(idxEnd >= (rspf_int32)sub_nodes.size())
         {
            idxEnd = (rspf_int32)sub_nodes.size()-1;
         }

         thePixelLookAngleX.push_back(tempV[idxStart] + tempIdxFraction*(tempV[idxEnd] - tempV[idxStart]));
      }
   }
   else
   {
      for (rspf_uint32 i=0; i<theImageSize.samp; ++i)
      {
         thePixelLookAngleX.push_back(sub_nodes[i]->getText().toDouble());
      }
   }

   thePixelLookAngleY.clear();
   xpath = "Look_Angles_List/Look_Angles/PSI_Y";
   sub_nodes.clear();
   xml_nodes[band_index]->findChildNodes(xpath, sub_nodes);

   if (theMetadataVersion == RSPF_SPOT_METADATA_VERSION_1_1)
   {
      for (rspf_uint32 i=0; i<theDetectorCount; ++i)
      {
         thePixelLookAngleY.push_back(sub_nodes[i]->getText().toDouble());
      }
   }
   else if (sub_nodes.size() != theImageSize.samp)
   {
//       if ((theSubImageOffset.samp + theImageSize.samp - 1)<= sub_nodes.size())
//       {
//          rspf_uint32 i = theSubImageOffset.samp;
//          for (rspf_uint32 idx 0; idx<theImageSize.samp; ++idx)
//          {
//             thePixelLookAngleX.push_back(sub_nodes[i]->getText().toDouble());
//             ++i;
//          }
//       }
      std::vector<double> tempV(sub_nodes.size());
      double t = 0.0;
      double tempIdx = 0.0;
      double tempIdxFraction = 0.0;
      rspf_int32 idxStart = 0;
      rspf_int32 idxEnd = 0;
      rspf_uint32 idx = 0;
      for(idx = 0; idx < sub_nodes.size();++idx)
      {
         tempV[idx] = sub_nodes[idx]->getText().toDouble();
      }
      for(idx = 0; idx < theImageSize.samp; ++idx)
      {
         t = (double)idx/(double)(theImageSize.samp-1);
         tempIdx = (sub_nodes.size()-1)*t;
         tempIdxFraction = tempIdx - (rspf_int32)tempIdx;
         idxStart = (rspf_int32)tempIdx;
         idxEnd = (rspf_int32)ceil(tempIdx);
         if(idxEnd >= (rspf_int32)sub_nodes.size())
         {
            idxEnd = (rspf_int32)sub_nodes.size()-1;
         }
         if(idxStart > idxEnd)
         {
            idxStart = idxEnd;
         }
         thePixelLookAngleY.push_back(tempV[idxStart] + tempIdxFraction*(tempV[idxEnd] - tempV[idxStart]));
      }
   }
   else
   {
      for (rspf_uint32 i=0; i<theImageSize.samp; ++i)
      {
         thePixelLookAngleY.push_back(sub_nodes[i]->getText().toDouble());
      }
   }

   //---
   // Fetch the Attitude Samples:
   //---
   theAttitudeSamples.clear();
   theAttSampTimes.clear();
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Satellite_Attitudes/Corrected_Attitudes/"
      "Corrected_Attitude/Angles";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      xpath = "/Dimap_Document/Data_Strip/Satellite_Attitudes/Raw_Attitudes/Aocs_Attitude/Angles_List/Angles";

      xmlDocument->findNodes(xpath, xml_nodes);
      if (xml_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
   }
   node = xml_nodes.begin();
   while (node != xml_nodes.end())
   {
      rspfDpt3d V;
      sub_nodes.clear();
      xpath = "OUT_OF_RANGE";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      if (sub_nodes[0]->getText() == "N")
      {
         sub_nodes.clear();
         xpath = "PITCH";
         (*node)->findChildNodes(xpath, sub_nodes);
         if (sub_nodes.size() == 0)
         {
            setErrorStatus();
            return false;
         }
         V.x = sub_nodes[0]->getText().toDouble();

         sub_nodes.clear();
         xpath = "ROLL";
         (*node)->findChildNodes(xpath, sub_nodes);
         if (sub_nodes.size() == 0)
         {
            setErrorStatus();
            return false;
         }
         V.y = sub_nodes[0]->getText().toDouble();

         sub_nodes.clear();
         xpath = "YAW";
         (*node)->findChildNodes(xpath, sub_nodes);
         if (sub_nodes.size() == 0)
         {
            setErrorStatus();
            return false;
         }
         V.z = sub_nodes[0]->getText().toDouble();

         theAttitudeSamples.push_back(V);

         sub_nodes.clear();
         xpath = "TIME";
         (*node)->findChildNodes(xpath, sub_nodes);
         if (sub_nodes.size() == 0)
         {
            setErrorStatus();
            return false;
         }
         theAttSampTimes.push_back(convertTimeStamp(sub_nodes[0]->getText()));
      }
      ++node;
   }

   return true;
}

bool rspfSpotDimapSupportData::parsePart3(
   rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   rspfString xpath;
   std::vector<rspfRefPtr<rspfXmlNode> > xml_nodes;
   std::vector<rspfRefPtr<rspfXmlNode> > sub_nodes;
   std::vector<rspfRefPtr<rspfXmlNode> >::iterator node;

   //---
   // Fetch the ephemeris samples:
   //---
   thePosEcfSamples.clear();
   theVelEcfSamples.clear();
   theEphSampTimes.clear();
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Ephemeris/Points/Point";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      return false;
   }
   node = xml_nodes.begin();

   while (node != xml_nodes.end())
   {
      rspfDpt3d VP;
      sub_nodes.clear();
      xpath  = "Location/X";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      VP.x = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath  = "Location/Y";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      VP.y = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath  = "Location/Z";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      VP.z = sub_nodes[0]->getText().toDouble();

      thePosEcfSamples.push_back(VP);

      rspfDpt3d VV;
      sub_nodes.clear();
      xpath = "Velocity/X";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      VV.x = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath = "Velocity/Y";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      VV.y = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath = "Velocity/Z";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      VV.z = sub_nodes[0]->getText().toDouble();

      theVelEcfSamples.push_back(VV);

      sub_nodes.clear();
      xpath  = "TIME";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      theEphSampTimes.push_back(convertTimeStamp(sub_nodes[0]->getText()));

      ++node;
   }

   //---
   // Fetch the star tracker flag:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Satellite_Attitudes/Corrected_Attitudes/"
      "STAR_TRACKER_USED";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
         theStarTrackerUsed = false;
//       setErrorStatus();
//       return false;
   }
   else
   {
      if (xml_nodes[0]->getText() == "Y")
         theStarTrackerUsed = true;
      else
         theStarTrackerUsed = false;
   }

   //---
   // Geoposition points:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Geoposition/Geoposition_Points/Tie_Point";
   xmlDocument->findNodes(xpath, xml_nodes);
   node = xml_nodes.begin();
   while (node != xml_nodes.end())
   {
      rspfGpt gpt;
      rspfDpt  ipt;

      sub_nodes.clear();
      xpath = "TIE_POINT_DATA_Y";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      ipt.line = sub_nodes[0]->getText().toDouble() - 1.0;

      sub_nodes.clear();
      xpath = "TIE_POINT_DATA_X";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      ipt.samp = sub_nodes[0]->getText().toDouble() - 1.0;

      sub_nodes.clear();
      xpath = "TIE_POINT_CRS_Y";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      gpt.lat = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath = "TIE_POINT_CRS_X";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      gpt.lon = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath = "TIE_POINT_CRS_Z";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      gpt.hgt = sub_nodes[0]->getText().toDouble();

      theGeoPosImagePoints.push_back(ipt);
      theGeoPosGroundPoints.push_back(gpt);

      ++node;
   }

   return true;
}

bool rspfSpotDimapSupportData::parsePart4(rspfRefPtr<rspfXmlDocument> xmlDocument)
{
  rspfString xpath;
  std::vector<rspfRefPtr<rspfXmlNode> > xml_nodes;
  std::vector<rspfRefPtr<rspfXmlNode> > sub_nodes;
  std::vector<rspfRefPtr<rspfXmlNode> >::iterator node;

  //---
  // Fetch the gain and bias for each spectral band:
  //---

  thePhysicalGain.assign(theNumBands, 1.000);
  thePhysicalBias.assign(theNumBands, 0.000);

  xml_nodes.clear();
  xpath = "/Dimap_Document/Image_Interpretation/Spectral_Band_Info";
  xmlDocument->findNodes(xpath, xml_nodes);
  node = xml_nodes.begin();
  while (node != xml_nodes.end())
  {
    sub_nodes.clear();
    xpath = "BAND_INDEX";
    (*node)->findChildNodes(xpath, sub_nodes);
    if (sub_nodes.size() == 0)
    {
      setErrorStatus();
      return false;
    }

    rspf_int32 bandIndex = sub_nodes[0]->getText().toInt32() - 1;

    if( (bandIndex >= static_cast<int>(theNumBands) ) || (bandIndex<0) )
    {
       rspfNotify(rspfNotifyLevel_WARN) << "rspfSpotDimapSupportData::parsePart4 ERROR: Band index outside of range\n";
       return false;
    }

    sub_nodes.clear();
    xpath = "PHYSICAL_BIAS";
    (*node)->findChildNodes(xpath, sub_nodes);
    if (sub_nodes.size() == 0)
    {
      setErrorStatus();
      return false;
    }
    thePhysicalBias[bandIndex] = sub_nodes[0]->getText().toDouble();

    sub_nodes.clear();
    xpath = "PHYSICAL_GAIN";
    (*node)->findChildNodes(xpath, sub_nodes);
    if (sub_nodes.size() == 0)
    {
      setErrorStatus();
      return false;
    }
    thePhysicalGain[bandIndex] = sub_nodes[0]->getText().toDouble();

    ++node;
  }

  theSolarIrradiance.assign(theNumBands, 0.000);
  xml_nodes.clear();
  xpath = "/Dimap_Document/Data_Strip/Sensor_Calibration/Solar_Irradiance/Band_Solar_Irradiance";
  xmlDocument->findNodes(xpath, xml_nodes);
  node = xml_nodes.begin();
  while (node != xml_nodes.end())
  {
    sub_nodes.clear();
    xpath = "BAND_INDEX";
    (*node)->findChildNodes(xpath, sub_nodes);
    if (sub_nodes.size() == 0)
    {
      setErrorStatus();
      return false;
    }

    rspf_int32 bandIndex = sub_nodes[0]->getText().toInt32() - 1;
    
    if((bandIndex >= static_cast<rspf_int32>(theNumBands) ) || (bandIndex<0))
    {
       rspfNotify(rspfNotifyLevel_WARN) << "rspfSpotDimapSupportData::parsePart4 ERROR: Band index outside of range\n";
       return false;
    }
    
    sub_nodes.clear();
    xpath = "SOLAR_IRRADIANCE_VALUE";
    (*node)->findChildNodes(xpath, sub_nodes);
    if (sub_nodes.size() == 0)
    {
      setErrorStatus();
      return false;
    }
    theSolarIrradiance[bandIndex] = sub_nodes[0]->getText().toDouble();

    ++node;
  }


  return true;
}

bool rspfSpotDimapSupportData::initMetadataVersion(rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   rspfString xpath;
   std::vector<rspfRefPtr<rspfXmlNode> > xml_nodes;

   //---
   // Get the version string which can be used as a key for parsing.
   //---
   xpath = "/Dimap_Document/Metadata_Id/METADATA_FORMAT";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << endl;
      }
      return false;
   }

   rspfString attribute = "version";
   rspfString value;
   xml_nodes[0]->getAttributeValue(value, attribute);
   if (value == "1.0")
   {
      theMetadataVersion = RSPF_SPOT_METADATA_VERSION_1_0;
   }
   else if (value == "1.1")
   {
      theMetadataVersion = RSPF_SPOT_METADATA_VERSION_1_1;
   }

   if (theMetadataVersion == RSPF_SPOT_METADATA_VERSION_UNKNOWN)
   {
      setErrorStatus();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "WARNING:  metadata version not found!"
            << std::endl;
      }
      return false;
   }
   return true;
}

bool rspfSpotDimapSupportData::initImageId(
   rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   rspfString xpath;
   vector<rspfRefPtr<rspfXmlNode> > xml_nodes;

   //---
   // Fetch the Image ID:
   //---
   xpath = "/Dimap_Document/Production/JOB_ID";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << endl;
      }
      return false;
   }
   theImageID = xml_nodes[0]->getText();
   return true;
}

bool rspfSpotDimapSupportData::initSceneSource(
   rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   rspfString xpath;
   vector<rspfRefPtr<rspfXmlNode> > xml_nodes;

  //---
  // Fetch the mission index (Spot 1, 4 or 5):
  // and generate theSensorID
  //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/MISSION_INDEX";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
     setErrorStatus();
     if(traceDebug())
       {
	 rspfNotify(rspfNotifyLevel_DEBUG)
	   << "DEBUG:\nCould not find: " << xpath
	   << std::endl;
       }
     return false;
   }
   if (xml_nodes[0]->getText() == "1")
     theSensorID = "Spot 1";
   if (xml_nodes[0]->getText() == "2")
     theSensorID = "Spot 2";
   if (xml_nodes[0]->getText() == "4")
     theSensorID = "Spot 4";
   if (xml_nodes[0]->getText() == "5")
     theSensorID = "Spot 5";

   //---
   // Fetch the Sun Azimuth:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/SUN_AZIMUTH";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theSunAzimuth = xml_nodes[0]->getText().toDouble();

   //---
   // Fetch the Sun Elevation:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/SUN_ELEVATION";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theSunElevation = xml_nodes[0]->getText().toDouble();

   //---
   // Fetch incidence angle:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/INCIDENCE_ANGLE";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theIncidenceAngle = xml_nodes[0]->getText().toDouble();

   //---
   // Fetch viewing angle:
   //
   // From the SPOT Dimap documentation (Dimap Generic 1.0), VIEWING_ANGLE
   // (the scene instrumental viewing angle) is ONLY available for SPOT5 data.
   // FROM SPOT: You can use use incidence angle to calculate viewing angle
   // (called look direction as well).
   // FIX (see below): need theSatelliteAltitude and theIncidenceAngle. The
   // equation is shown below where RT is the mean value of WGS84 ellipsoid 
   // semi-axis.
   //---
   if(this->theSensorID == "Spot 5")
   {
      xml_nodes.clear();
      xpath = "/Dimap_Document/Dataset_Sources/Source_Information/Scene_Source/VIEWING_ANGLE";
      xmlDocument->findNodes(xpath, xml_nodes);
      if (xml_nodes.size() == 0)
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "DEBUG:\nCould not find: " << xpath
               << std::endl;
         }
         return false;
      }
      theViewingAngle = xml_nodes[0]->getText().toDouble();
   }
   else
   {
      xml_nodes.clear();
      xpath = "/Dimap_Document/Data_Strip/Ephemeris/SATELLITE_ALTITUDE";
      
      theViewingAngle = -1.0;
      xmlDocument->findNodes(xpath, xml_nodes);
      if (xml_nodes.size() == 0)
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "DEBUG:\nCould not find: " << xpath
               << std::endl;
         }
         return false;
      }
      //compute VIEWING_ANGLE
      double theSatelliteAltitude =  xml_nodes[0]->getText().toDouble();
      double RT = 63710087714.0;
      theViewingAngle = asin((RT/(RT+theSatelliteAltitude))*sin(theIncidenceAngle));
   }

   //---
   // Fetch Step Count:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Data_Strip/Sensor_Configuration/Mirror_Position/STEP_COUNT";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theStepCount = xml_nodes[0]->getText().toInt();
   
   return true;
}

bool rspfSpotDimapSupportData::initFramePoints(
   rspfRefPtr<rspfXmlDocument> xmlDocument)
{
   rspfString xpath;
   vector<rspfRefPtr<rspfXmlNode> > xml_nodes;

   //---
   // Corner points:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Frame/Vertex";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() != 4)
   {
      setErrorStatus();
      return false;
   }
   std::vector<rspfRefPtr<rspfXmlNode> >::iterator node = xml_nodes.begin();
   while (node != xml_nodes.end())
   {
      rspfGpt gpt;
      rspfDpt ipt;

      std::vector<rspfRefPtr<rspfXmlNode> > sub_nodes;
      xpath = "FRAME_LAT";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      gpt.lat = sub_nodes[0]->getText().toDouble();

      sub_nodes.clear();
      xpath = "FRAME_LON";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      gpt.lon = sub_nodes[0]->getText().toDouble();
      gpt.hgt = 0.0; // assumed

      sub_nodes.clear();
      xpath = "FRAME_ROW";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      ipt.line = sub_nodes[0]->getText().toDouble() - 1.0;

      sub_nodes.clear();
      xpath = "FRAME_COL";
      (*node)->findChildNodes(xpath, sub_nodes);
      if (sub_nodes.size() == 0)
      {
         setErrorStatus();
         return false;
      }
      ipt.samp = sub_nodes[0]->getText().toDouble() - 1.0;

      if (ipt.line < 1.0)
         if (ipt.samp < 1.0)
            theUlCorner = gpt;
         else
            theUrCorner = gpt;
      else
         if (ipt.samp < 1.0)
            theLlCorner = gpt;
         else
            theLrCorner = gpt;

      ++node;
   }

   //---
   // Center of frame.
   //---
   theRefGroundPoint.hgt = 0.0; // needs to be looked up

   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Frame/Scene_Center/FRAME_LON";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() != 1)
   {
      setErrorStatus();
      return false;
   }
   theRefGroundPoint.lon = xml_nodes[0]->getText().toDouble();

   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Frame/Scene_Center/FRAME_LAT";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() != 1)
   {
      setErrorStatus();
      return false;
   }
   theRefGroundPoint.lat = xml_nodes[0]->getText().toDouble();

  
   //---
   // Fetch scene orientation:
   //---
   xml_nodes.clear();
   xpath = "/Dimap_Document/Dataset_Frame/SCENE_ORIENTATION";
   xmlDocument->findNodes(xpath, xml_nodes);
   if (xml_nodes.size() == 0)
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG:\nCould not find: " << xpath
            << std::endl;
      }
      return false;
   }
   theSceneOrientation = xml_nodes[0]->getText().toDouble();  

   return true;
}

