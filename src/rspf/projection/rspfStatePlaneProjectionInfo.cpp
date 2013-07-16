#include <rspf/projection/rspfStatePlaneProjectionInfo.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfLambertConformalConicProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfUnitTypeLut.h>
rspfStatePlaneProjectionInfo::rspfStatePlaneProjectionInfo(
   const std::string&  name,
   int                 pcsCode,
   const std::string&  projName,
   const std::string&  param1,
   const std::string&  param2,
   const std::string&  param3,
   const std::string&  param4,
   double              falseEast,
   double              falseNorth,
   const std::string&  units,
   const std::string&  datumName)
   :
      thePcsCode        (pcsCode),
      theName           (name),
      theProjectionName (projName),
      theDatum          (0),
      theOriginLat      (param1),
      theOriginLon      (param2),
      theOrigin         (),
      theParameter3     (0.0),
      theParameter4     (0.0),
      theFalseEasting   (falseEast),
      theFalseNorthing  (falseNorth),
      theScaleFactor    (0.0),
      theUnits          (RSPF_UNIT_UNKNOWN)
      
{
   if (projName == "tm")
   {
      theProjectionName = "rspfTransMercatorProjection";
   }
   else if (projName == "lcc")
   {
      theProjectionName = "rspfLambertConformalConicProjection";
   }
   if (theProjectionName == "rspfTransMercatorProjection")
   {
      theParameter3 = rspfString::toDouble(param3);
      if(fabs(theParameter3) > FLT_EPSILON)
      {
         theScaleFactor = 1.0 - (1.0 / theParameter3);
      }
      else
      {
         theScaleFactor = 1.0;
      }
   }
   else if (theProjectionName == "rspfLambertConformalConicProjection")
   {
      theParameter3 = rspfDms(param3).getDegrees();
      theParameter4 = rspfDms(param4).getDegrees();
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfStatePlaneProjectionInfo unhandled projection type: "
         << projName << std::endl;
   }
   theDatum = rspfDatumFactoryRegistry::instance()->create(datumName);
   if (!theDatum)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfStatePlaneProjectionInfo unhandled datum type: "
         << datumName << std::endl;
   }
   theOrigin = rspfGpt(theOriginLat.getDegrees(),
                        theOriginLon.getDegrees(),
                        0,
                        theDatum);
   theUnits = static_cast<rspfUnitType>(
      rspfUnitTypeLut::instance()->getEntryNumber(units.c_str()));
   if ( (theUnits != RSPF_METERS) &&
        (theUnits != RSPF_US_SURVEY_FEET) &&
        (theUnits != RSPF_FEET) )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfStatePlaneProjectionInfo unhandled unit type: "
         << units.c_str()
         << std::endl;
   }
}
int rspfStatePlaneProjectionInfo::code() const
{
   return thePcsCode;
}
const rspfString& rspfStatePlaneProjectionInfo::name() const
{
   return theName;
}
const rspfString& rspfStatePlaneProjectionInfo::projName() const
{
   return theProjectionName;
}
double rspfStatePlaneProjectionInfo::originLat() const
{
   return theOriginLat.getDegrees();
}
double rspfStatePlaneProjectionInfo::originLon() const
{
   return theOriginLon.getDegrees();
}
const rspfDatum* rspfStatePlaneProjectionInfo::datum() const
{
   return theDatum;
}
rspfGpt rspfStatePlaneProjectionInfo::origin() const
{
   return theOrigin;
}
double rspfStatePlaneProjectionInfo::parallel1() const
{
   return theParameter3;
}
double rspfStatePlaneProjectionInfo::parallel2() const
{
   return theParameter4;
}
double rspfStatePlaneProjectionInfo::falseEasting() const
{
   return theFalseEasting;
}
double rspfStatePlaneProjectionInfo::falseNorthing() const
{
   return theFalseNorthing;
}
double rspfStatePlaneProjectionInfo::scaleFactor() const
{
   return theScaleFactor;
}
rspfString rspfStatePlaneProjectionInfo::units() const
{
   return rspfUnitTypeLut::instance()->getEntryString(theUnits);
}
bool rspfStatePlaneProjectionInfo::isSameCode(int pcsCode) const
{
   if(pcsCode == thePcsCode)
   {
      return true;
   }
   return false;
}
double rspfStatePlaneProjectionInfo::falseEastingInMeters() const
{
   double result = theFalseEasting;
   switch (theUnits)
   {
      case RSPF_US_SURVEY_FEET:
         result *= US_METERS_PER_FT;
         break;
      case RSPF_FEET:
         result *= MTRS_PER_FT;
         break;
      default:
         break;
   }
   return result;
}
double rspfStatePlaneProjectionInfo::falseNorthingInMeters() const
{
   double result = theFalseNorthing;
   switch (theUnits)
   {
      case RSPF_US_SURVEY_FEET:
         result *= US_METERS_PER_FT;
         break;
      case RSPF_FEET:
         result *= MTRS_PER_FT;
         break;
      default:
         break;
   }
   return result;
}
void rspfStatePlaneProjectionInfo::populateProjectionKeywords(
   rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::PCS_CODE_KW,
           thePcsCode,
           true);
  
   kwl.add(prefix,
           rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
           (rspfUnitTypeLut::instance()->getEntryString(theUnits)),
           true);
   rspfDpt pt(theFalseEasting, theFalseNorthing);
           
   kwl.add(prefix,
           rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
           pt.toString().c_str(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::ORIGIN_LATITUDE_KW,
           origin().latd(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::CENTRAL_MERIDIAN_KW,
           origin().lond(),
           true);
   
   kwl.add(prefix,
      rspfKeywordNames::TYPE_KW,
      projName(),
      true);
   
   if(theDatum)
   {
      
      kwl.add(prefix,
              rspfKeywordNames::DATUM_KW,
              theDatum->code(),
              true);
   }
   if (projName()== STATIC_TYPE_NAME(rspfLambertConformalConicProjection))
   {
      kwl.add(prefix,
              rspfKeywordNames::STD_PARALLEL_1_KW,
              parallel1(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::STD_PARALLEL_2_KW,
              parallel2(),
              true);
   }
   else
   {
      kwl.add(prefix,
              rspfKeywordNames::SCALE_FACTOR_KW,
              theScaleFactor,
              true);
   }
}
rspfUnitType rspfStatePlaneProjectionInfo::getUnitType() const
{
   return theUnits;
}
bool rspfStatePlaneProjectionInfo::matchesProjection(
   const rspfMapProjection* proj) const
{
   if (!proj)
   {
      return false;
   }
#if 0
   cout << "\ntheProjectionName: " << theProjectionName
           << "\nclassName: " << proj->getClassName()
           << "\ntheOrigin: " << theOrigin
           << "\nproj->origin(): "<< proj->origin()
           << "\nfalseEastingInMeters(): "<< falseEastingInMeters()
           << "\nproj->getFalseEasting(): " << proj->getFalseEasting()
           << "\nfalseNorthingInMeters(): " << falseNorthingInMeters()
           << "\nproj->getFalseNorthing(): " << proj->getFalseNorthing()
           << "\ndatum: " << theDatum->code() 
           << endl;
#endif
   
   if ( (theProjectionName != proj->getClassName()) ||
        ( (theOrigin == proj->origin()) == false) ||
        (falseEastingInMeters() != proj->getFalseEasting()) ||
        (falseNorthingInMeters() != proj->getFalseNorthing()) )
   {
      return false;
   }
   const double ALLOWABLE_VARIANCE = 0.00000000000001;
   
   if (theProjectionName == "rspfTransMercatorProjection")
   {
      rspfTransMercatorProjection* tm = PTR_CAST(rspfTransMercatorProjection,
                                                  proj);
      if (!tm)
      {
         return false;
      }
      double d1 = fabs(theScaleFactor - tm->getScaleFactor());
      if (d1 < ALLOWABLE_VARIANCE)
      {
         return true;
      }
   }
   else // Lambert.
   {
      rspfLambertConformalConicProjection* lam =
         PTR_CAST(rspfLambertConformalConicProjection, proj);
      if (!lam)
      {
         return false;
      }
      double d1 = fabs(parallel1() - lam->getStandardParallel1());
      double d2 = fabs(parallel2() - lam->getStandardParallel2());
      if ( (d1 < ALLOWABLE_VARIANCE) && (d2 < ALLOWABLE_VARIANCE) )
      {
         return true;
      }
   }
   
   return false;
}
std::ostream& operator<<(std::ostream& os,
                         const rspfStatePlaneProjectionInfo& thePlane)
{
   if (thePlane.projName()== STATIC_TYPE_NAME(rspfTransMercatorProjection))
   {
      return  os << "rspfStatePlaneProjection: "
                 << "\nName:    " << thePlane.name()
                 << "\nPcsCode: " << thePlane.code()
                 << "\norigin:  " << thePlane.origin()
                 << "\nscale factor:  " << setprecision(12)
                 << thePlane.scaleFactor()
                 << "\nfalseEasting:   " << thePlane.falseEasting()
                 << "\nfalseNorthing:  " << thePlane.falseNorthing()
                 << endl;
   }
   else  // Lambert
   {
      return  os << "rspfStatePlaneProjection: "
                 << "\nName:    " << thePlane.name()
                 << "\nPcsCode: " << thePlane.code()
                 << "\norigin:  " << thePlane.origin()
                 << "\nphi1:    "
                 << setprecision(12) << thePlane.parallel1()
                 << "\nphi2:    "
                 << setprecision(12) << thePlane.parallel2()
                 << "\nfalseEasting:   " << thePlane.falseEasting()
                 << "\nfalseNorthing:  " << thePlane.falseNorthing()
                 << endl;
   }
}
