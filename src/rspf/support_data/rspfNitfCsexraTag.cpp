//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: CSEXRA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0006-NCDRD Table 3.5-16 for more info.
// 
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <rspf/support_data/rspfNitfCsexraTag.h>


RTTI_DEF1(rspfNitfCsexraTag, "rspfNitfCsexraTag", rspfNitfRegisteredTag);

rspfNitfCsexraTag::rspfNitfCsexraTag()
   : rspfNitfRegisteredTag(std::string("CSEXRA"), 132)
{
   clearFields();
}

rspfNitfCsexraTag::~rspfNitfCsexraTag()
{
}

void rspfNitfCsexraTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(theSensor, 6);
   in.read(theTileFirstLine, 12);
   in.read(theImageTimeDuration, 12);
   in.read(theMaxGsd, 5);
   in.read(theAlongScanGsd, 5);
   in.read(theCrossScanGsd, 5);
   in.read(theGeoMeanGsd, 5);
   in.read(theAlongScanVertGsd, 5);
   in.read(theCrossScanVertGsd, 5);
   in.read(theGeoMeanVertGsd, 5);
   in.read(theGeoBetaAngle, 5);
   in.read(theDynamicRange, 5);
   in.read(theLine, 7);
   in.read(theSamples, 5);
   in.read(theAngleToNorth, 7);
   in.read(theObliquityAngle, 6);
   in.read(theAzOfObliquity, 7);
   in.read(theGrdCover, 1);
   in.read(theSnowDepthCategory, 1);
   in.read(theSunAzimuth, 7);
   in.read(theSunElevation, 7);
   in.read(thePredictedNiirs, 3);
   in.read(theCircularError, 3);
   in.read(theLinearError, 3);
}

void rspfNitfCsexraTag::writeStream(std::ostream& out)
{
   out.write(theSensor, 6);
   out.write(theTileFirstLine, 12);
   out.write(theImageTimeDuration, 12);
   out.write(theMaxGsd, 5);
   out.write(theAlongScanGsd, 5);
   out.write(theCrossScanGsd, 5);
   out.write(theGeoMeanGsd, 5);
   out.write(theAlongScanVertGsd, 5);
   out.write(theCrossScanVertGsd, 5);
   out.write(theGeoMeanVertGsd, 5);
   out.write(theGeoBetaAngle, 5);
   out.write(theDynamicRange, 5);
   out.write(theLine, 7);
   out.write(theSamples, 5);
   out.write(theAngleToNorth, 7);
   out.write(theObliquityAngle, 6);
   out.write(theAzOfObliquity, 7);
   out.write(theGrdCover, 1);
   out.write(theSnowDepthCategory, 1);
   out.write(theSunAzimuth, 7);
   out.write(theSunElevation, 7);
   out.write(thePredictedNiirs, 3);
   out.write(theCircularError, 3);
   out.write(theLinearError, 3);
}

void rspfNitfCsexraTag::clearFields()
{
   //---
   // No attempt made to set to defaults.
   // BCS-N's to '0's, BCS-A's to ' '(spaces)
   //---

   memset(theSensor, ' ',  6);
   memset(theTileFirstLine, '0', 12);
   memset(theImageTimeDuration, '0', 12);
   memset(theMaxGsd, '0', 5);
   memset(theAlongScanGsd, ' ', 5);
   memset(theCrossScanGsd, ' ', 5);
   memset(theGeoMeanGsd, ' ', 5);
   memset(theAlongScanVertGsd, ' ', 5);
   memset(theCrossScanVertGsd, ' ', 5);
   memset(theGeoMeanVertGsd, ' ', 5);
   memset(theGeoBetaAngle, ' ', 5);
   memset(theDynamicRange, '0', 5);
   memset(theLine, '0', 7);
   memset(theSamples, '0', 5);
   memset(theAngleToNorth, '0', 7);
   memset(theObliquityAngle, '0', 6);
   memset(theAzOfObliquity, '0', 7);
   memset(theGrdCover, '0', 1);
   memset(theSnowDepthCategory, '0', 1);
   memset(theSunAzimuth, '0', 7);
   memset(theSunElevation, '0', 7);
   memset(thePredictedNiirs, ' ', 3);
   memset(theCircularError, '0', 3);
   memset(theLinearError, '0', 3);

   theSensor[6] = '\0';
   theTileFirstLine[12] = '\0';
   theImageTimeDuration[12] = '\0';
   theMaxGsd[5] = '\0';
   theAlongScanGsd[5] = '\0';
   theCrossScanGsd[5] = '\0';
   theGeoMeanGsd[5] = '\0';
   theAlongScanVertGsd[5] = '\0';
   theCrossScanVertGsd[5] = '\0';
   theGeoMeanVertGsd[5] = '\0';
   theGeoBetaAngle[5] = '\0';
   theDynamicRange[5] = '\0';
   theLine[7] = '\0';
   theSamples[5] = '\0';
   theAngleToNorth[7] = '\0';
   theObliquityAngle[6] = '\0';
   theAzOfObliquity[7] = '\0';
   theGrdCover[1] = '\0';
   theSnowDepthCategory[1] = '\0';
   theSunAzimuth[7] = '\0';
   theSunElevation[7] = '\0';
   thePredictedNiirs[3] = '\0';
   theCircularError[3] = '\0';
   theLinearError[3] = '\0';
   
}

std::ostream& rspfNitfCsexraTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"
       << getTagLength() << "\n"
       << pfx << std::setw(24) << "SENSOR:"
       << theSensor << "\n"
       << pfx << std::setw(24) << "TIME_FIRST_LINE_IMAGE:"
       << theTileFirstLine << "\n"
       << pfx << std::setw(24) << "TIME_IMAGE_DURATION:"
       << theImageTimeDuration << "\n"
       << pfx << std::setw(24) << "MAX_GSD:"
       << theMaxGsd << "\n"
       << pfx << std::setw(24) << "ALONG_SCAN_GSD:"
       << theAlongScanGsd << "\n"
       << pfx << std::setw(24) << "CROSS_SCAN_GSD:"
       << theCrossScanGsd << "\n"
       << pfx << std::setw(24) << "GEO_MEAN_GSD:"
       << theGeoMeanGsd << "\n"
       << pfx << std::setw(24) << "A_S_VERT_GSD:"
       << theAlongScanVertGsd << "\n"
       << pfx << std::setw(24) << "C_S_VERT_GSD:"
       << theCrossScanVertGsd << "\n"
       << pfx << std::setw(24) << "GEO_MEAN_VERT_GSD:"
       << theGeoMeanVertGsd << "\n"
       << pfx << std::setw(24) << "GEO_BETA_ANGLE:"
       << theGeoBetaAngle << "\n"
       << pfx << std::setw(24) << "DYNAMIC_RANGE:"
       << theDynamicRange << "\n"
       << pfx << std::setw(24) << "NUM_LINES:"
       << theLine << "\n"
       << pfx << std::setw(24) << "NUM_SAMPLES:"
       << theSamples << "\n"
       << pfx << std::setw(24) << "ANGLE_TO_NORTH:"
       << theAngleToNorth << "\n"
       << pfx << std::setw(24) << "OBLIQUITY_ANGLE:"
       << theObliquityAngle << "\n"
       << pfx << std::setw(24) << "AZ_OF_OBLIQUITY:"
       << theAzOfObliquity << "\n"
       << pfx << std::setw(24) << "GRD_COVER:"
       << theGrdCover << "\n"
       << pfx << std::setw(24) << "SNOW_DEPTH_CAT:"
       << theSnowDepthCategory << "\n"
       << pfx << std::setw(24) << "SUN_AZIMUTH:"
       << theSunAzimuth << "\n"
       << pfx << std::setw(24) << "SUN_ELEVATION:"
       << theSunElevation << "\n"
       << pfx << std::setw(24) << "PREDICTED_NIIRS:"
       << thePredictedNiirs << "\n"
       << pfx << std::setw(24) << "CIRCL_ERR:"
       << theCircularError << "\n"
       << pfx << std::setw(24) << "LINEAR_ERR:"
       << theLinearError<< "\n";
   
   return out;
}

rspfString rspfNitfCsexraTag::getSensor() const
{
   return rspfString(theSensor);
}
   
rspfString rspfNitfCsexraTag::getTimeFirstLineImage() const
{
   return rspfString(theTileFirstLine);
}
   
rspfString rspfNitfCsexraTag::getTimeImageDuration() const
{
   return rspfString(theImageTimeDuration);
}
   
rspfString rspfNitfCsexraTag::getMaxGsd() const
{
   return rspfString(theMaxGsd);
}
   
rspfString rspfNitfCsexraTag::getAlongScanGsd() const
{
   return rspfString(theAlongScanGsd);
}
   
rspfString rspfNitfCsexraTag::getCrossScanGsd() const
{
   return rspfString(theCrossScanGsd);
}
   
rspfString rspfNitfCsexraTag::getGeoMeanGsd() const
{
   return rspfString(theGeoMeanGsd);
}
   
rspfString rspfNitfCsexraTag::getAlongScanVerticalGsd() const
{
   return rspfString(theAlongScanVertGsd);
}
   
rspfString rspfNitfCsexraTag::getCrossScanVerticalGsd() const
{
   return rspfString(theCrossScanVertGsd);
}

rspfString rspfNitfCsexraTag::getGeoMeanVerticalGsd() const
{
   return rspfString(theGeoMeanVertGsd);
}

rspfString rspfNitfCsexraTag::getGeoBetaAngle() const
{
   return rspfString(theGeoBetaAngle);
}

rspfString rspfNitfCsexraTag::getDynamicRange() const
{
   return rspfString(theDynamicRange);
}
   
rspfString rspfNitfCsexraTag::getNumLines() const
{
   return rspfString(theLine);
}
   
rspfString rspfNitfCsexraTag::getNumSamples() const
{
   return rspfString(theSamples);
}
   
rspfString rspfNitfCsexraTag::getAngleToNorth() const
{
   return rspfString(theAngleToNorth);
}

rspfString rspfNitfCsexraTag::getObliquityAngle() const
{
   return rspfString(theObliquityAngle);
}

rspfString rspfNitfCsexraTag::getAzimuthOfObliquity() const
{
   return rspfString(theAzOfObliquity);
}
   
rspfString rspfNitfCsexraTag::getGroundCover() const
{
   return rspfString(theGrdCover);
}

rspfString rspfNitfCsexraTag::getSnowDepth() const
{
   return rspfString(theSnowDepthCategory);
}
   
rspfString rspfNitfCsexraTag::getSunAzimuth() const
{
   return rspfString(theSunAzimuth);
}

rspfString rspfNitfCsexraTag::getSunElevation() const
{
   return rspfString(theSunElevation);
}

rspfString rspfNitfCsexraTag::getPredictedNiirs() const
{
   return rspfString(thePredictedNiirs);
}
   
rspfString rspfNitfCsexraTag::getCE90() const
{
   return rspfString(theCircularError);
}

rspfString rspfNitfCsexraTag::getLE90() const
{
   return rspfString(theLinearError);
}
