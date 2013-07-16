//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: BLOCKA tag class declaration.
//
//----------------------------------------------------------------------------
// $Id: rspfNitfBlockaTag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <cstring> /* for memcpy */
#include <sstream>
#include <iomanip>

#include <rspf/support_data/rspfNitfBlockaTag.h>
#include <rspf/support_data/rspfNitfCommon.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfDpt.h>

static const rspfTrace traceDebug(rspfString("rspfNitfBlockaTag:debug"));

RTTI_DEF1(rspfNitfBlockaTag, "rspfNitfBlockaTag", rspfNitfRegisteredTag);

rspfNitfBlockaTag::rspfNitfBlockaTag()
   : rspfNitfRegisteredTag(std::string("BLOCKA"), 123)
{
   clearFields();
}

void rspfNitfBlockaTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(theBlockInstance, BLOCK_INSTANCE_SIZE);
   in.read(theNGray,         N_GRAY_SIZE);
   in.read(theLLines,        L_LINES_SIZE);
   in.read(theLayoverAngle,  LAYOVER_ANGLE_SIZE);
   in.read(theShadowAngle,   SHADOW_ANGLE_SIZE);
   in.read(theField6,        FIELD_6_SIZE);
   in.read(theFrlcLoc,       FRLC_LOC_SIZE);
   in.read(theLrlcLoc,       LRLC_LOC_SIZE);
   in.read(theLrfcLoc,       LRFC_LOC_SIZE);
   in.read(theFrfcLoc,       FRFC_LOC_SIZE);
   in.read(theField11,       FIELD_11_SIZE);
   
}

void rspfNitfBlockaTag::writeStream(std::ostream& out)
{
   out.write(theBlockInstance, BLOCK_INSTANCE_SIZE);
   out.write(theNGray,         N_GRAY_SIZE);
   out.write(theLLines,        L_LINES_SIZE);
   out.write(theLayoverAngle,  LAYOVER_ANGLE_SIZE);
   out.write(theShadowAngle,   SHADOW_ANGLE_SIZE);
   out.write(theField6,        FIELD_6_SIZE);
   out.write(theFrlcLoc,       FRLC_LOC_SIZE);
   out.write(theLrlcLoc,       LRLC_LOC_SIZE);
   out.write(theLrfcLoc,       LRFC_LOC_SIZE);
   out.write(theFrfcLoc,       FRFC_LOC_SIZE);
   out.write(theField11,       FIELD_11_SIZE);
}

void rspfNitfBlockaTag::clearFields()
{
   memcpy(theBlockInstance, "01",    BLOCK_INSTANCE_SIZE);
   memset(theNGray,         '0',     N_GRAY_SIZE);
   memset(theLLines,        '0',     L_LINES_SIZE);
   memset(theLayoverAngle,  ' ',     LAYOVER_ANGLE_SIZE);
   memset(theShadowAngle,   ' ',     SHADOW_ANGLE_SIZE);
   memset(theField6,        ' ',     FIELD_6_SIZE);
   memset(theFrlcLoc,       ' ',     FRLC_LOC_SIZE);
   memset(theLrlcLoc,       ' ',     LRLC_LOC_SIZE);
   memset(theLrfcLoc,       ' ',     LRFC_LOC_SIZE);
   memset(theFrfcLoc,       ' ',     FRFC_LOC_SIZE);
   memcpy(theField11,       "010.0", FIELD_11_SIZE);
   
   theBlockInstance[BLOCK_INSTANCE_SIZE] = '\0';
   theNGray[N_GRAY_SIZE]                 = '\0';
   theLLines[L_LINES_SIZE]               = '\0';
   theLayoverAngle[LAYOVER_ANGLE_SIZE]   = '\0';
   theShadowAngle[SHADOW_ANGLE_SIZE]     = '\0';
   theField6[FIELD_6_SIZE]               = '\0';
   theFrlcLoc[FRLC_LOC_SIZE]             = '\0';
   theLrlcLoc[LRLC_LOC_SIZE]             = '\0';
   theLrfcLoc[LRFC_LOC_SIZE]             = '\0';
   theFrfcLoc[FRFC_LOC_SIZE]             = '\0';
   theField11[FIELD_11_SIZE]              = '\0';
}

std::ostream& rspfNitfBlockaTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   // Grab the corners parsed into points.
   rspfDpt ulPt;
   rspfDpt urPt;
   rspfDpt lrPt;
   rspfDpt llPt;
   getFrfcLoc(ulPt);
   getFrlcLoc(urPt);
   getLrlcLoc(lrPt);
   getLrfcLoc(llPt);
   
   out << setiosflags(ios::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "BLOCK_INSTANCE:" << theBlockInstance << "\n"
       << pfx << std::setw(24) << "N_GRAY:"         << theNGray << "\n"
       << pfx << std::setw(24) << "L_LINES:"        << theLLines << "\n"
       << pfx << std::setw(24) << "LAYOVER_ANGLE:"  << theLayoverAngle << "\n"
       << pfx << std::setw(24) << "SHADOW_ANGLE:"   << theShadowAngle << "\n"
       << pfx << std::setw(24) << "FIELD_6:"        << theField6 << "\n"
       << pfx << std::setw(24) << "FRLC_LOC:"       << theFrlcLoc << "\n"
       << pfx << std::setw(24) << "LRLC_LOC:"       << theLrlcLoc << "\n"
       << pfx << std::setw(24) << "LRFC_LOC:"       << theLrfcLoc << "\n"
       << pfx << std::setw(24) << "FRFC_LOC:"       << theFrfcLoc << "\n"
       << pfx << std::setw(24) << "FIELD_11:"       << theField11 << "\n"
       << pfx << std::setw(24) << "upper left:"     << ulPt << "\n"
       << pfx << std::setw(24) << "upper right:"    << urPt << "\n"
       << pfx << std::setw(24) << "lower right:"    << lrPt << "\n"
       << pfx << std::setw(24) << "lower left:"     << llPt << "\n";

   return out;
}

void rspfNitfBlockaTag::setBlockInstance(rspf_uint32 block)
{
   if ( (block > 0) && (block < 100) )
   {
      rspfString os =
         rspfNitfCommon::convertToIntString(block,
                                             BLOCK_INSTANCE_SIZE);
      memcpy(theBlockInstance, os.c_str(), BLOCK_INSTANCE_SIZE);
   }
}

rspfString rspfNitfBlockaTag::getBlockInstance() const
{
   return rspfString(theBlockInstance);
}

void rspfNitfBlockaTag::setNGray(rspf_uint32 grayCount)
{
   if (grayCount < 100000)
   {
      rspfString os =
         rspfNitfCommon::convertToUIntString(grayCount,
                                              N_GRAY_SIZE);
      memcpy(theNGray, os.c_str(), N_GRAY_SIZE);
   }
}

rspfString rspfNitfBlockaTag::getNGray() const
{
   return rspfString(theNGray);
}

void rspfNitfBlockaTag::setLLines(rspf_uint32 lines)
{
   if (lines < 100000)
   {
      rspfString os =
         rspfNitfCommon::convertToUIntString(lines, L_LINES_SIZE);
      memcpy(theLLines, os.c_str(), L_LINES_SIZE);
   }
}

rspfString rspfNitfBlockaTag::getLLines() const
{
   return rspfString(theLLines);
}

void rspfNitfBlockaTag::setLayoverAngle(rspf_uint32 angle)
{
   if (angle < 360)
   {
      rspfString os =
         rspfNitfCommon::convertToUIntString(angle, LAYOVER_ANGLE_SIZE);
      memcpy(theLayoverAngle, os.c_str(), LAYOVER_ANGLE_SIZE);
   }
}

rspfString rspfNitfBlockaTag::getLayoverAngle() const
{
   return rspfString(theLayoverAngle);
}

void rspfNitfBlockaTag::setShadowAngle(rspf_uint32 angle)
{
   if (angle < 360)
   {
      rspfString os =
         rspfNitfCommon::convertToUIntString(angle, SHADOW_ANGLE_SIZE);
      memcpy(theShadowAngle, os.c_str(), SHADOW_ANGLE_SIZE);
   }
}

rspfString rspfNitfBlockaTag::getShadowAngle() const
{
   return rspfString(theShadowAngle);
}

void rspfNitfBlockaTag::setFrlcLoc(const rspfDpt& pt)
{
   rspfString os = rspfNitfCommon::convertToDdLatLonLocString(pt, 6);
   memcpy(theFrlcLoc, os.c_str(), FRLC_LOC_SIZE);
}

rspfString rspfNitfBlockaTag::getFrlcLoc() const
{
   return rspfString(theFrlcLoc);
}

void rspfNitfBlockaTag::getFrlcLoc(rspfDpt& pt) const
{
   rspfString os = theFrlcLoc;
   converLocStringToPt(os, pt);
}

void rspfNitfBlockaTag::setLrlcLoc(const rspfDpt& pt)
{
   rspfString os = rspfNitfCommon::convertToDdLatLonLocString(pt, 6);
   memcpy(theLrlcLoc, os.c_str(), LRLC_LOC_SIZE);
}
   
rspfString rspfNitfBlockaTag::getLrlcLoc() const
{
   return rspfString(theLrlcLoc);
}

void rspfNitfBlockaTag::getLrlcLoc(rspfDpt& pt) const
{
   rspfString os = theLrlcLoc;
   converLocStringToPt(os, pt);
}

void rspfNitfBlockaTag::setLrfcLoc(const rspfDpt& pt)
{
   rspfString os = rspfNitfCommon::convertToDdLatLonLocString(pt, 6);
   memcpy(theLrfcLoc, os.c_str(), LRFC_LOC_SIZE);
}

rspfString rspfNitfBlockaTag::getLrfcLoc() const
{
   return rspfString(theLrfcLoc);
}

void rspfNitfBlockaTag::getLrfcLoc(rspfDpt& pt) const
{
   rspfString os = theLrfcLoc;
   converLocStringToPt(os, pt); 
}

void rspfNitfBlockaTag::setFrfcLoc(const rspfDpt& pt)
{
   rspfString os = rspfNitfCommon::convertToDdLatLonLocString(pt, 6);
   memcpy(theFrfcLoc, os.c_str(), FRFC_LOC_SIZE);
}

rspfString rspfNitfBlockaTag::getFrfcLoc() const
{
   return rspfString(theFrfcLoc);
}

void rspfNitfBlockaTag::getFrfcLoc(rspfDpt& pt) const
{
   rspfString os = theFrfcLoc;
   converLocStringToPt(os, pt);
}

void rspfNitfBlockaTag::converLocStringToPt(const rspfString& locationString,
                                             rspfDpt& pt) const
{
   if (locationString.size() != 21) return;

   std::string s = locationString;
   
   std::string lat(s, 0, 10);
   std::string lon(s, 10, 11);

   if ( (lat[0] == 'N') || (lat[0] == 'S') )
   {
      // Need to enter spaces for rspfDms to parse correctly
      std::ostringstream latStream;
      latStream << lat.substr(0, 3) << " " << lat.substr(3, 2)
                << " " << lat.substr(5);
      lat = latStream.str();
      
      // Use the dms class to parse.
      rspfDms dms(0.0, true);
      dms.setDegrees(lat.c_str());
      pt.y = dms.getDegrees();
   }
   else
   {
      rspfString os = lat;
      pt.y = os.toDouble();
   }
   if ( (lon[0] == 'E') || (lon[0] == 'W') )
   {
      // Need to enter spaces for rspfDms to parse correctly
      std::ostringstream lonStream;
      lonStream << lon.substr(0, 4) << " " << lon.substr(4, 2)
                << " " << lon.substr(6);
      lon = lonStream.str();
      
      // Use the dms class to parse.
      rspfDms dms(0.0, false);
      dms.setDegrees(lon.c_str());
      pt.x = dms.getDegrees();
   }
   else
   {
      rspfString os = lon;
      pt.x = os.toDouble();
   }
}
