//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Walt Bunch
//
// Description: Nitf support class for USE00A -
// Exploitation Usability extension.
// 
//********************************************************************
// $Id: rspfNitfUse00aTag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <iostream>
#include <iomanip>
#include <rspf/support_data/rspfNitfUse00aTag.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfCommon.h>

RTTI_DEF1(rspfNitfUse00aTag, "rspfNitfUse00aTag", rspfNitfRegisteredTag);

static const rspfString ANGLETONORTH_KW = "ANGLETONORTH";
static const rspfString MEANGSD_KW = "MEANGSD";
static const rspfString DYNAMICRANGE_KW = "DYNAMICRANGE";
static const rspfString OBLANG_KW = "OBLANG";
static const rspfString ROLLANG_KW = "ROLLANG";
static const rspfString NREF_KW = "NREF";
static const rspfString REVNUM_KW = "REVNUM";
static const rspfString NSEG_KW = "NSEG";
static const rspfString MAXLPSEG_KW = "MAXLPSEG";
static const rspfString SUNEL_KW = "SUNEL";
static const rspfString SUNAZ_KW = "SUNAZ";


rspfNitfUse00aTag::rspfNitfUse00aTag()
   : rspfNitfRegisteredTag(std::string("USE00A"), 107) 
{
   clearFields();
}

rspfNitfUse00aTag::~rspfNitfUse00aTag()
{
}

void rspfNitfUse00aTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(theAngleToNorth, ANGLE_TO_NORTH_SIZE);
   in.read(theMeanGsd, MEAN_GSD_SIZE);
   in.read(theField3, FIELD3_SIZE);
   in.read(theDynamicRange, DYNAMIC_RANGE_SIZE);
   in.read(theField5, FIELD5_SIZE);
   in.read(theField6, FIELD6_SIZE);
   in.read(theField7, FIELD7_SIZE);
   in.read(theOblAng, OBL_ANG_SIZE);
   in.read(theRollAng, ROLL_ANG_SIZE);
   in.read(theField10, FIELD10_SIZE);
   in.read(theField11, FIELD11_SIZE);
   in.read(theField12, FIELD12_SIZE);
   in.read(theField13, FIELD13_SIZE);
   in.read(theField14, FIELD14_SIZE);
   in.read(theField15, FIELD15_SIZE);
   in.read(theField16, FIELD16_SIZE);
   in.read(theNRef, N_REF_SIZE);
   in.read(theRevNum, REV_NUM_SIZE);
   in.read(theNSeg, N_SEG_SIZE);
   in.read(theMaxLpSeg, MAX_LP_SEG_SIZE);
   in.read(theField20, FIELD20_SIZE);
   in.read(theField21, FIELD21_SIZE);
   in.read(theSunEl, SUN_EL_SIZE);
   in.read(theSunAz, SUN_AZ_SIZE);
}

void rspfNitfUse00aTag::writeStream(std::ostream& out)
{
   out.write(theAngleToNorth, ANGLE_TO_NORTH_SIZE);
   out.write(theMeanGsd, MEAN_GSD_SIZE);
   out.write(theField3, FIELD3_SIZE);
   out.write(theDynamicRange, DYNAMIC_RANGE_SIZE);
   out.write(theField5, FIELD5_SIZE);
   out.write(theField6, FIELD6_SIZE);
   out.write(theField7, FIELD7_SIZE);
   out.write(theOblAng, OBL_ANG_SIZE);
   out.write(theRollAng, ROLL_ANG_SIZE);
   out.write(theField10, FIELD10_SIZE);
   out.write(theField11, FIELD11_SIZE);
   out.write(theField12, FIELD12_SIZE);
   out.write(theField13, FIELD13_SIZE);
   out.write(theField14, FIELD14_SIZE);
   out.write(theField15, FIELD15_SIZE);
   out.write(theField16, FIELD16_SIZE);
   out.write(theNRef, N_REF_SIZE);
   out.write(theRevNum, REV_NUM_SIZE);
   out.write(theNSeg, N_SEG_SIZE);
   out.write(theMaxLpSeg, MAX_LP_SEG_SIZE);
   out.write(theField20, FIELD20_SIZE);
   out.write(theField21, FIELD21_SIZE);
   out.write(theSunEl, SUN_EL_SIZE);
   out.write(theSunAz, SUN_AZ_SIZE);
}

void rspfNitfUse00aTag::clearFields()
{
   memset(theAngleToNorth, ' ', ANGLE_TO_NORTH_SIZE);
   memset(theMeanGsd, ' ', MEAN_GSD_SIZE);
   memset(theField3, ' ', FIELD3_SIZE);
   memset(theDynamicRange, ' ', DYNAMIC_RANGE_SIZE);
   memset(theField5, ' ', FIELD5_SIZE);
   memset(theField6, ' ', FIELD6_SIZE);
   memset(theField7, ' ', FIELD7_SIZE);
   memset(theOblAng, ' ', OBL_ANG_SIZE);
   memset(theRollAng, ' ', ROLL_ANG_SIZE);
   memset(theField10, ' ', FIELD10_SIZE);
   memset(theField11, ' ', FIELD11_SIZE);
   memset(theField12, ' ', FIELD12_SIZE);
   memset(theField13, ' ', FIELD13_SIZE);
   memset(theField14, ' ', FIELD14_SIZE);
   memset(theField15, ' ', FIELD15_SIZE);
   memset(theField16, ' ', FIELD16_SIZE);
   memset(theNRef, ' ', N_REF_SIZE);
   memset(theRevNum, ' ', REV_NUM_SIZE);
   memset(theNSeg, ' ', N_SEG_SIZE);
   memset(theMaxLpSeg, ' ', MAX_LP_SEG_SIZE);
   memset(theField20, ' ', FIELD20_SIZE);
   memset(theField21, ' ', FIELD21_SIZE);
   memset(theSunEl, ' ', SUN_EL_SIZE);
   memset(theSunAz, ' ', SUN_AZ_SIZE);

   theAngleToNorth[ANGLE_TO_NORTH_SIZE] = '\0';
   theMeanGsd[MEAN_GSD_SIZE] = '\0';
   theField3[FIELD3_SIZE] = '\0';
   theDynamicRange[DYNAMIC_RANGE_SIZE] = '\0';
   theField5[FIELD5_SIZE] = '\0';
   theField6[FIELD6_SIZE] = '\0';
   theField7[FIELD7_SIZE] = '\0';
   theOblAng[OBL_ANG_SIZE] = '\0';
   theRollAng[ROLL_ANG_SIZE] = '\0';
   theField10[FIELD10_SIZE] = '\0';
   theField11[FIELD11_SIZE] = '\0';
   theField12[FIELD12_SIZE] = '\0';
   theField13[FIELD13_SIZE] = '\0';
   theField14[FIELD14_SIZE] = '\0';
   theField15[FIELD15_SIZE] = '\0';
   theField16[FIELD16_SIZE] = '\0';
   theNRef[N_REF_SIZE] = '\0';
   theRevNum[REV_NUM_SIZE] = '\0';
   theNSeg[N_SEG_SIZE] = '\0';
   theMaxLpSeg[MAX_LP_SEG_SIZE] = '\0';
   theField20[FIELD20_SIZE] = '\0';
   theField21[FIELD21_SIZE] = '\0';
   theSunEl[SUN_EL_SIZE] = '\0';
   theSunAz[SUN_AZ_SIZE] = '\0';
}

rspfString rspfNitfUse00aTag::getAngleToNorth()const
{
   return theAngleToNorth;
}

void rspfNitfUse00aTag::setAngleToNorth(const rspfString& angleToNorth)
{
   memset(theAngleToNorth, ' ', ANGLE_TO_NORTH_SIZE);
   memcpy(theAngleToNorth, angleToNorth.c_str(), std::min((size_t)ANGLE_TO_NORTH_SIZE, angleToNorth.length()));
}

rspfString rspfNitfUse00aTag::getMeanGsd()const
{
   return theMeanGsd;
}

rspf_float64 rspfNitfUse00aTag::getMeanGsdInMeters() const
{
   rspf_float64 gsd = 0.0;
   
   rspfString s = theMeanGsd;
   if (s.size())
   {
      rspf_float64 d = s.toFloat64();
      if ( (d != 0.0) && (rspf::isnan(d) == false) )
      {
         gsd = d / 12.0 * MTRS_PER_FT;
      }
   }
   
   return gsd;
}

void rspfNitfUse00aTag::setMeanGsd(const rspfString& meanGsd)
{
   memset(theMeanGsd, ' ', MEAN_GSD_SIZE);
   memcpy(theMeanGsd, meanGsd.c_str(), std::min((size_t)MEAN_GSD_SIZE, meanGsd.length()));
}

rspfString rspfNitfUse00aTag::getField3()const
{
   return theField3;
}

void rspfNitfUse00aTag::setField3(const rspfString& field3)
{
   memset(theField3, ' ', FIELD3_SIZE);
   memcpy(theField3, field3.c_str(), std::min((size_t)FIELD3_SIZE, field3.length()));
}

rspfString rspfNitfUse00aTag::getDynamicRange()const
{
   return theDynamicRange;
}

void rspfNitfUse00aTag::setDynamicRange(const rspfString& dynamicRange)
{
   memset(theDynamicRange, ' ', DYNAMIC_RANGE_SIZE);
   memcpy(theDynamicRange, dynamicRange.c_str(), std::min((size_t)DYNAMIC_RANGE_SIZE, dynamicRange.length()));
}

rspfString rspfNitfUse00aTag::getField5()const
{
   return theField5;
}

void rspfNitfUse00aTag::setField5(const rspfString& field5)
{
   memset(theField5, ' ', FIELD5_SIZE);
   memcpy(theField5, field5.c_str(), std::min((size_t)FIELD5_SIZE, field5.length()));
}

rspfString rspfNitfUse00aTag::getField6()const
{
   return theField6;
}

void rspfNitfUse00aTag::setField6(const rspfString& field6)
{
   memset(theField6, ' ', FIELD6_SIZE);
   memcpy(theField6, field6.c_str(), std::min((size_t)FIELD6_SIZE, field6.length()));
}

rspfString rspfNitfUse00aTag::getField7()const
{
   return theField7;
}

void rspfNitfUse00aTag::setField7(const rspfString& field7)
{
   memset(theField7, ' ', FIELD7_SIZE);
   memcpy(theField7, field7.c_str(), std::min((size_t)FIELD7_SIZE, field7.length()));
}

rspfString rspfNitfUse00aTag::getOblAng()const
{
   return theOblAng;
}

void rspfNitfUse00aTag::setOblAng(const rspfString& oblAng)
{
   memset(theOblAng, ' ', OBL_ANG_SIZE);
   memcpy(theOblAng, oblAng.c_str(), std::min((size_t)OBL_ANG_SIZE, oblAng.length()));
}

rspfString rspfNitfUse00aTag::getRollAng()const
{
   return theRollAng;
}

void rspfNitfUse00aTag::setRollAng(const rspfString& rollAng)
{
   memset(theRollAng, ' ', ROLL_ANG_SIZE);
   memcpy(theRollAng, rollAng.c_str(), std::min((size_t)ROLL_ANG_SIZE, rollAng.length()));
}

rspfString rspfNitfUse00aTag::getField10()const
{
   return theField10;
}

void rspfNitfUse00aTag::setField10(const rspfString& field10)
{
   memset(theField10, ' ', FIELD10_SIZE);
   memcpy(theField10, field10.c_str(), std::min((size_t)FIELD10_SIZE, field10.length()));
}

rspfString rspfNitfUse00aTag::getField11()const
{
   return theField11;
}

void rspfNitfUse00aTag::setField11(const rspfString& field11)
{
   memset(theField11, ' ', FIELD11_SIZE);
   memcpy(theField11, field11.c_str(), std::min((size_t)FIELD11_SIZE, field11.length()));
}

rspfString rspfNitfUse00aTag::getField12()const
{
   return theField12;
}

void rspfNitfUse00aTag::setField12(const rspfString& field12)
{
   memset(theField12, ' ', FIELD12_SIZE);
   memcpy(theField12, field12.c_str(), std::min((size_t)FIELD12_SIZE, field12.length()));
}

rspfString rspfNitfUse00aTag::getField13()const
{
   return theField13;
}

void rspfNitfUse00aTag::setField13(const rspfString& field13)
{
   memset(theField13, ' ', FIELD13_SIZE);
   memcpy(theField13, field13.c_str(), std::min((size_t)FIELD13_SIZE, field13.length()));
}

rspfString rspfNitfUse00aTag::getField14()const
{
   return theField14;
}

void rspfNitfUse00aTag::setField14(const rspfString& field14)
{
   memset(theField14, ' ', FIELD14_SIZE);
   memcpy(theField14, field14.c_str(), std::min((size_t)FIELD14_SIZE, field14.length()));
}

rspfString rspfNitfUse00aTag::getField15()const
{
   return theField15;
}

void rspfNitfUse00aTag::setField15(const rspfString& field15)
{
   memset(theField15, ' ', FIELD15_SIZE);
   memcpy(theField15, field15.c_str(), std::min((size_t)FIELD15_SIZE, field15.length()));
}

rspfString rspfNitfUse00aTag::getField16()const
{
   return theField16;
}

void rspfNitfUse00aTag::setField16(const rspfString& field16)
{
   memset(theField16, ' ', FIELD16_SIZE);
   memcpy(theField16, field16.c_str(), std::min((size_t)FIELD16_SIZE, field16.length()));
}

rspfString rspfNitfUse00aTag::getNRef()const
{
   return theNRef;
}

void rspfNitfUse00aTag::setNRef(const rspfString& nRef)
{
   memset(theNRef, ' ', N_REF_SIZE);
   memcpy(theNRef, nRef.c_str(), std::min((size_t)N_REF_SIZE, nRef.length()));
}

rspfString rspfNitfUse00aTag::getRevNum()const
{
   return theRevNum;
}

void rspfNitfUse00aTag::setRevNum(const rspfString& revNum)
{
   memset(theRevNum, ' ', REV_NUM_SIZE);
   memcpy(theRevNum, revNum.c_str(), std::min((size_t)REV_NUM_SIZE, revNum.length()));
}

rspfString rspfNitfUse00aTag::getNSeg()const
{
   return theNSeg;
}

void rspfNitfUse00aTag::setNSeg(const rspfString& nSeg)
{
   memset(theNSeg, ' ', N_SEG_SIZE);
   memcpy(theNSeg, nSeg.c_str(), std::min((size_t)N_SEG_SIZE, nSeg.length()));
}

rspfString rspfNitfUse00aTag::getMaxLpSeg()const
{
   return theMaxLpSeg;
}

void rspfNitfUse00aTag::setMaxLpSeg(const rspfString& maxLpSeg)
{
   memset(theMaxLpSeg, ' ', MAX_LP_SEG_SIZE);
   memcpy(theMaxLpSeg, maxLpSeg.c_str(), std::min((size_t)MAX_LP_SEG_SIZE, maxLpSeg.length()));
}

rspfString rspfNitfUse00aTag::getField20()const
{
   return theField20;
}

void rspfNitfUse00aTag::setField20(const rspfString& field20)
{
   memset(theField20, ' ', FIELD20_SIZE);
   memcpy(theField20, field20.c_str(), std::min((size_t)FIELD20_SIZE, field20.length()));
}

rspfString rspfNitfUse00aTag::getField21()const
{
   return theField21;
}

void rspfNitfUse00aTag::setField21(const rspfString& field21)
{
   memset(theField21, ' ', FIELD21_SIZE);
   memcpy(theField21, field21.c_str(), std::min((size_t)FIELD21_SIZE, field21.length()));
}

rspfString rspfNitfUse00aTag::getSunEl()const
{
   return theSunEl;
}

void rspfNitfUse00aTag::setSunEl(const rspfString& sunEl)
{
   memset(theSunEl, ' ', SUN_EL_SIZE);
   memcpy(theSunEl, sunEl.c_str(), std::min((size_t)SUN_EL_SIZE, sunEl.length()));
}

rspfString rspfNitfUse00aTag::getSunAz()const
{
   return theSunAz;
}

void rspfNitfUse00aTag::setSunAz(const rspfString& sunAz)
{
   memset(theSunAz, ' ', SUN_AZ_SIZE);
   memcpy(theSunAz, sunAz.c_str(), std::min((size_t)SUN_AZ_SIZE, sunAz.length()));
}

std::ostream& rspfNitfUse00aTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "ANGLETONORTH:" << theAngleToNorth << "\n" 
       << pfx << std::setw(24) << "MEANGSD:"      << theMeanGsd << "\n"
       << pfx << std::setw(24) << "DYNAMICRANGE:" << theDynamicRange << "\n"
       << pfx << std::setw(24) << "OBLANG:"       << theOblAng << "\n"
       << pfx << std::setw(24) << "ROLLANG:"      << theRollAng << "\n"
       << pfx << std::setw(24) << "NREF:"         << theNRef << "\n"
       << pfx << std::setw(24) << "REVNUM:"       << theRevNum << "\n"
       << pfx << std::setw(24) << "NSEG:"         << theNSeg << "\n"
       << pfx << std::setw(24) << "MAXLPSEG:"     << theMaxLpSeg << "\n"
       << pfx << std::setw(24) << "SUNEL:"        << theSunEl << "\n"
       << pfx << std::setw(24) << "SUNAZ:"        << theSunAz << "\n";

   return out; 
}
   
void rspfNitfUse00aTag::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfNitfRegisteredTag::setProperty(property);
}

rspfRefPtr<rspfProperty> rspfNitfUse00aTag::getProperty(const rspfString& name)const
{
   rspfProperty* result = 0;

   if(name == ANGLETONORTH_KW)
   {
      result = new rspfStringProperty(name, theAngleToNorth);
   }
   else if(name == MEANGSD_KW)
   {
      result = new rspfStringProperty(name, theMeanGsd);
   }
   else if(name == DYNAMICRANGE_KW)
   {
      result = new rspfStringProperty(name, theDynamicRange);
   }
   else if(name == OBLANG_KW)
   {
      result = new rspfStringProperty(name, theOblAng);
   }
   else if(name == ROLLANG_KW)
   {
      result = new rspfStringProperty(name, theRollAng);
   }
   else if(name == NREF_KW)
   {
      result = new rspfStringProperty(name, theNRef);
   }
   else if(name == REVNUM_KW)
   {
      result = new rspfStringProperty(name, theRevNum);
   }
   else if(name == NSEG_KW)
   {
      result = new rspfStringProperty(name, theNSeg);
   }
   else if(name == MAXLPSEG_KW)
   {
      result = new rspfStringProperty(name, theMaxLpSeg);
   }
   else if(name == SUNEL_KW)
   {
      result = new rspfStringProperty(name, theSunEl);
   }
   else if(name == SUNAZ_KW)
   {
      result = new rspfStringProperty(name, theSunAz);
   }
   else
   {
      return rspfNitfRegisteredTag::getProperty(name);
   }

   return result;
}

void rspfNitfUse00aTag::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfNitfRegisteredTag::getPropertyNames(propertyNames);

   propertyNames.push_back(ANGLETONORTH_KW);
   propertyNames.push_back(MEANGSD_KW);
   propertyNames.push_back(DYNAMICRANGE_KW);
   propertyNames.push_back(OBLANG_KW);
   propertyNames.push_back(ROLLANG_KW);
   propertyNames.push_back(NREF_KW);
   propertyNames.push_back(REVNUM_KW);
   propertyNames.push_back(NSEG_KW);
   propertyNames.push_back(MAXLPSEG_KW);
   propertyNames.push_back(SUNEL_KW);
   propertyNames.push_back(SUNAZ_KW);
}
