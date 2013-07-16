#include <cstdlib>
#include <fstream>
#include <time.h>
using namespace std;
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfUnitTypeLut.h>
static rspfTrace traceDebug("rspfMapProjectionInfo::debug");
const char* rspfMapProjectionInfo::OUTPUT_US_FT_INFO_KW =
"viewinfo.output_readme_in_us_ft_flag";
const char* rspfMapProjectionInfo::PIXEL_TYPE_KW = "viewinfo.pixel_type";
const char* rspfMapProjectionInfo::README_IMAGE_STRING_KW =
"viewinfo.readme_image_string";
rspfMapProjectionInfo::rspfMapProjectionInfo(const rspfMapProjection* proj,
                                               const rspfDrect& output_rect)
   :
      theProjection               (proj),
      theErrorStatus              (false),
      theLinesPerImage            (0),
      thePixelsPerLine            (0),
      theCornerGroundPt           (),
      theCornerEastingNorthingPt  (),
      theCenterGroundPt           (),
      theCenterEastingNorthingPt  (0.0, 0.0),
      thePixelType                (RSPF_PIXEL_IS_POINT),
      theOutputInfoInFeetFlag     (false),
      theImageInfoString          ()
{
   if (!theProjection)
   {
      theErrorStatus = true;
      rspfNotify(rspfNotifyLevel_FATAL)
         << "FATAL rspfMapProjectionInfo::rspfMapProjectionInfo: "
         << "Null projection pointer passed to constructor!"
         << "\nError status has been set.  Returning..."
         << std::endl;
      return;
   }
   
   initializeMembers(output_rect);
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfMapProjectionInfo::rspfMapProjectionInfo:\n"
         << "output_rect:  " << output_rect << "\n"
         << *this << std::endl;
   }
}
rspfMapProjectionInfo::~rspfMapProjectionInfo()
{
}
bool rspfMapProjectionInfo::errorStatus() const
{
   return theErrorStatus;
}
void rspfMapProjectionInfo::initializeMembers(const rspfDrect& rect)
{
   theBoundingRect = rect;
   theLinesPerImage  = rspf::round<int>(rect.height());
   thePixelsPerLine  = rspf::round<int>(rect.width());
   theProjection->lineSampleToWorld(rect.ul(),
                                    theCornerGroundPt[0]);
   
   theProjection->lineSampleToWorld(rect.ur(),
                                    theCornerGroundPt[1]);
   
   theProjection->lineSampleToWorld(rect.lr(), 
                                    theCornerGroundPt[2]);
   
   
   theProjection->lineSampleToWorld(rect.ll(), 
                                    theCornerGroundPt[3]);
   theProjection->lineSampleToEastingNorthing(rect.ul(),
                                              theCornerEastingNorthingPt[0]);
   theProjection->lineSampleToEastingNorthing(rect.ur(), 
                                              theCornerEastingNorthingPt[1]);
   
   theProjection->lineSampleToEastingNorthing(rect.lr(),
                                              theCornerEastingNorthingPt[2]);
   
   theProjection->lineSampleToEastingNorthing(rect.ll(), 
                                              theCornerEastingNorthingPt[3]);
   
   theCenterEastingNorthingPt.x = (theCornerEastingNorthingPt[0].x +
                                   theCornerEastingNorthingPt[1].x +
                                   theCornerEastingNorthingPt[2].x +
                                   theCornerEastingNorthingPt[3].x) / 4.0;
   
   theCenterEastingNorthingPt.y = (theCornerEastingNorthingPt[0].y +
                                   theCornerEastingNorthingPt[1].y +
                                   theCornerEastingNorthingPt[2].y +
                                   theCornerEastingNorthingPt[3].y) / 4.0;
   
   theCenterGroundPt = theProjection->inverse(theCenterEastingNorthingPt);
}
void rspfMapProjectionInfo::getGeom(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   theProjection->saveState(kwl, prefix);
   if(theProjection->isGeographic())
   {
      rspfGpt gpt = ulGroundPt();
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_XY_KW,
              rspfDpt(gpt).toString().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_UNITS_KW,
              rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES),
              true);
   }
   else
   {
      rspfDpt dpt = ulEastingNorthingPt();
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_XY_KW,
              dpt.toString().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_UNITS_KW,
              rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS),
              true);
   }
}
std::ostream& rspfMapProjectionInfo::print(std::ostream& os) const
{
   if (!os)
   {
      return os;
   }
   os << setiosflags(ios::left)
      << setiosflags(ios::fixed)
      << "rspfMapProjectionInfo Data Members:\n"
      << "Projection name:  " << theProjection->getProjectionName()
      << setw(30) << "\nOutput pixel type:"
      << ((getPixelType() == RSPF_PIXEL_IS_POINT) ? "pixel is point" :
          "pixel is area")
      << setw(30) << "\nMeters per pixel:"
      << getMetersPerPixel()
      << setw(30) << "\nUS survey feet per pixel:"
      << getUsSurveyFeetPerPixel()
      << setw(30) << "\nDecimal degrees per pixel:"
      << getDecimalDegreesPerPixel()
      << setw(30) << "\nNumber of lines:"
      << theLinesPerImage
      << setw(30) << "\nNumber of pixels:"
      << thePixelsPerLine
      << setw(30) << "\nUpper left ground point:"
      << theCornerGroundPt[0]
      << setw(30) << "\nUpper right ground point:"
      << theCornerGroundPt[1]
      << setw(30) << "\nLower right ground point:"
      << theCornerGroundPt[2]
      << setw(30) << "\nLower left ground point:"
      << theCornerGroundPt[3]
      << setw(30) << "\nUpper left easting_northing:"
      << theCornerEastingNorthingPt[0]
      << setw(30) << "\nUpper right easting_northing:"
      << theCornerEastingNorthingPt[1]
      << setw(30) << "\nLower right easting_northing:"
      << theCornerEastingNorthingPt[2]
      << setw(30) << "\nLower left easting_northing:"
      << theCornerEastingNorthingPt[3]
      << setw(30) << "\nCenter ground point:"
      << theCenterGroundPt
      << setw(30) << "\nCenter easting_northing:"
      << theCenterEastingNorthingPt
      << "\nMap Projection dump:";
   theProjection->print(os);
   return os;
}
void rspfMapProjectionInfo::setPixelType (rspfPixelType type)
{
   thePixelType = type;
}
rspfPixelType rspfMapProjectionInfo::getPixelType () const
{
   return thePixelType;
}
void rspfMapProjectionInfo::setOutputFeetFlag (bool flag)
{
   theOutputInfoInFeetFlag = flag;
}
bool rspfMapProjectionInfo::unitsInFeet() const
{
   return theOutputInfoInFeetFlag;
}
rspfString rspfMapProjectionInfo::getImageInfoString () const
{
   return theImageInfoString;
}
void rspfMapProjectionInfo::setImageInfoString (const rspfString& string)
{
   theImageInfoString = string;
}
const rspfMapProjection* rspfMapProjectionInfo::getProjection() const
{
   return theProjection;
}
rspfDpt rspfMapProjectionInfo::ulEastingNorthingPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt mpp = getMetersPerPixel();
      rspfDpt pt;
      pt.x = theCornerEastingNorthingPt[0].x - (mpp.x / 2.0);
      pt.y = theCornerEastingNorthingPt[0].y + (mpp.y / 2.0);
      return pt;
   }
   else
   {
      return theCornerEastingNorthingPt[0];
   }
}
rspfDpt rspfMapProjectionInfo::urEastingNorthingPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt mpp = getMetersPerPixel();
      rspfDpt pt;
      pt.x = theCornerEastingNorthingPt[1].x + (mpp.x / 2.0);
      pt.y = theCornerEastingNorthingPt[1].y + (mpp.y / 2.0);
      return pt;      
   }
   else
   {
      return theCornerEastingNorthingPt[1];
   }
}
rspfDpt rspfMapProjectionInfo::lrEastingNorthingPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt mpp = getMetersPerPixel();
      rspfDpt pt;
      pt.x = theCornerEastingNorthingPt[2].x + (mpp.x / 2.0);
      pt.y = theCornerEastingNorthingPt[2].y - (mpp.y / 2.0);
      return pt;      
   }
   else
   {
      return theCornerEastingNorthingPt[2];
   }
}
rspfDpt rspfMapProjectionInfo::llEastingNorthingPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt mpp = getMetersPerPixel();
      rspfDpt pt;
      pt.x = theCornerEastingNorthingPt[3].x - (mpp.x / 2.0);
      pt.y = theCornerEastingNorthingPt[3].y - (mpp.y / 2.0);
      return pt;      
   }
   else
   {
      return theCornerEastingNorthingPt[3];
   }
}
rspfGpt rspfMapProjectionInfo::ulGroundPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt ddpp = getDecimalDegreesPerPixel();
      rspfGpt gpt;
      gpt.latd(theCornerGroundPt[0].latd() + (ddpp.y / 2.0));
      gpt.lond(theCornerGroundPt[0].lond() - (ddpp.x / 2.0));
      return gpt;
   }
   else
   {
      return theCornerGroundPt[0];
   } 
}
rspfGpt rspfMapProjectionInfo::urGroundPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt ddpp = getDecimalDegreesPerPixel();
      rspfGpt gpt;
      gpt.latd(theCornerGroundPt[1].latd() + (ddpp.y / 2.0));
      gpt.lond(theCornerGroundPt[1].lond() + (ddpp.x / 2.0));
      return gpt;
   }
   else
   {
      return theCornerGroundPt[1];
   }
}
rspfGpt rspfMapProjectionInfo::lrGroundPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt ddpp = getDecimalDegreesPerPixel();
      rspfGpt gpt;
      gpt.latd(theCornerGroundPt[2].latd() - (ddpp.y / 2.0));
      gpt.lond(theCornerGroundPt[2].lond() + (ddpp.x / 2.0));
      return gpt;
   }
   else
   {
      return theCornerGroundPt[2];
   }
}
rspfGpt rspfMapProjectionInfo::llGroundPt( ) const
{
   if (getPixelType() == RSPF_PIXEL_IS_AREA)
   {
      rspfDpt ddpp = getDecimalDegreesPerPixel();
      rspfGpt gpt;
      gpt.latd(theCornerGroundPt[3].latd() - (ddpp.y / 2.0));
      gpt.lond(theCornerGroundPt[3].lond() - (ddpp.x / 2.0));
      return gpt;
   }
   else
   {
      return theCornerGroundPt[3];
   }   
}
rspfGpt rspfMapProjectionInfo::centerGroundPt( ) const
{
   return theCenterGroundPt;
}
rspfDpt rspfMapProjectionInfo::centerEastingNorthingPt() const
{
   return theCenterEastingNorthingPt;
}
rspf_int32 rspfMapProjectionInfo::linesPerImage() const
{
   return theLinesPerImage;
}
rspf_int32 rspfMapProjectionInfo::pixelsPerLine() const
{
   return thePixelsPerLine;
}
bool rspfMapProjectionInfo::loadState(const rspfKeywordlist& kwl,
                                       const char* )
{
   const char* lookupReturn = kwl.find(README_IMAGE_STRING_KW);
   if (lookupReturn) theImageInfoString = lookupReturn;
   
   lookupReturn = kwl.find(rspfKeywordNames::PIXEL_TYPE_KW);
   if (lookupReturn)
   {
      rspfString tmp = lookupReturn;
      tmp.downcase();
      if (tmp.contains("area"))
      {
         thePixelType = RSPF_PIXEL_IS_AREA;
      }
      else
      {
         thePixelType = RSPF_PIXEL_IS_POINT;
      }
   }
   lookupReturn = kwl.find(OUTPUT_US_FT_INFO_KW);
   if (lookupReturn)
   {
      int tmp = atoi(lookupReturn);
      if (tmp)
      {
         theOutputInfoInFeetFlag = true;
      }
      else
      {
         theOutputInfoInFeetFlag = false;
      }
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfMapProjectionInfo::loadState:"
         << "\ntheImageInfoString:       " << theImageInfoString
         << "\nthePixelType:             " << int(thePixelType)
         << "\ntheOutputInfoInFeetFlag:  " << theOutputInfoInFeetFlag
         << endl;
   }
   return true;
}
bool rspfMapProjectionInfo::saveState(rspfKeywordlist& kwl,
                                       const char* ) const 
{
   kwl.add(README_IMAGE_STRING_KW,
           theImageInfoString.chars());
   
   rspfString tmp;
   
   if (thePixelType == RSPF_PIXEL_IS_POINT)
   {
      tmp = "point";
   }
   else
   {
      tmp = "area";
   }
   
   kwl.add(PIXEL_TYPE_KW,
           tmp.chars());
   kwl.add(OUTPUT_US_FT_INFO_KW,
           int(theOutputInfoInFeetFlag));
   
   return true;
}
rspfDpt rspfMapProjectionInfo::ulEastingNorthingPtInFt() const
{
   rspfDpt pt = ulEastingNorthingPt();
   
   pt.x = rspf::mtrs2usft(pt.x);
   
   pt.y = rspf::mtrs2usft(pt.y);
   
   return pt;
}
rspfDpt rspfMapProjectionInfo::urEastingNorthingPtInFt() const
{
   rspfDpt pt = urEastingNorthingPt();
   pt.x = rspf::mtrs2usft(pt.x);
   pt.y = rspf::mtrs2usft(pt.y);
   return pt;
}
rspfDpt rspfMapProjectionInfo::lrEastingNorthingPtInFt() const
{
   rspfDpt pt = lrEastingNorthingPt();
   pt.x = rspf::mtrs2usft(pt.x);
   pt.y = rspf::mtrs2usft(pt.y);
   return pt;
}
rspfDpt rspfMapProjectionInfo::llEastingNorthingPtInFt() const
{
   rspfDpt pt = llEastingNorthingPt();
   pt.x = rspf::mtrs2usft(pt.x);
   pt.y = rspf::mtrs2usft(pt.y);
   return pt;
}
rspfDpt rspfMapProjectionInfo::getMetersPerPixel() const
{
   return theProjection->getMetersPerPixel();
}
rspfDpt rspfMapProjectionInfo::getUsSurveyFeetPerPixel() const
{
   rspfDpt pt = getMetersPerPixel();
   
   pt.x = rspf::mtrs2usft(pt.x);
   pt.y = rspf::mtrs2usft(pt.y);
   return pt;
}
rspfDpt rspfMapProjectionInfo::getDecimalDegreesPerPixel() const
{
   return theProjection->getDecimalDegreesPerPixel();
}
