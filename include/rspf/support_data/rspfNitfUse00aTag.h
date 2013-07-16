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
// $Id: rspfNitfUse00aTag.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef rspfNitfUse00aTag_HEADER
#define rspfNitfUse00aTag_HEADER

#include <rspf/support_data/rspfNitfRegisteredTag.h>

class RSPF_DLL rspfNitfUse00aTag : public rspfNitfRegisteredTag
{
public:
   
   enum
   {
      ANGLE_TO_NORTH_SIZE  = 3,
      MEAN_GSD_SIZE        = 5,
      FIELD3_SIZE          = 1,
      DYNAMIC_RANGE_SIZE   = 5,
      FIELD5_SIZE          = 3,
      FIELD6_SIZE          = 1,
      FIELD7_SIZE          = 3,
      OBL_ANG_SIZE         = 5,
      ROLL_ANG_SIZE        = 6,
      FIELD10_SIZE         = 12,
      FIELD11_SIZE         = 15,
      FIELD12_SIZE         = 4,
      FIELD13_SIZE         = 1,
      FIELD14_SIZE         = 3,
      FIELD15_SIZE         = 1,
      FIELD16_SIZE         = 1,
      N_REF_SIZE           = 2,
      REV_NUM_SIZE         = 5,
      N_SEG_SIZE           = 3,
      MAX_LP_SEG_SIZE      = 6,
      FIELD20_SIZE         = 6,
      FIELD21_SIZE         = 6,
      SUN_EL_SIZE          = 5,
      SUN_AZ_SIZE          = 5
      //                  -----
      //                  107 bytes
   };
   
   rspfNitfUse00aTag();
   virtual ~rspfNitfUse00aTag();

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual void clearFields();
   
  // The set methods below taking rspfString args will truncate and
  // pad with spaces, as necessary, to match enumed size
   void setAngleToNorth(const rspfString& angleToNorth);
   rspfString getAngleToNorth()const;
   void setMeanGsd(const rspfString& meanGsd);
   rspfString getMeanGsd()const;

   /**
    * @return MEANGSD field as a double converted to meters.
    *
    * @note Conversion used: gsd_in_meters = gsd / 12.0 * MTRS_PER_FT
    */
   rspf_float64 getMeanGsdInMeters() const;

   void setField3(const rspfString& field3);
   rspfString getField3()const;
   void setDynamicRange(const rspfString& dynamicRange);
   rspfString getDynamicRange()const;
   void setField5(const rspfString& field5);
   rspfString getField5()const;
   void setField6(const rspfString& field6);
   rspfString getField6()const;
   void setField7(const rspfString& field7);
   rspfString getField7()const;
   void setOblAng(const rspfString& oblAng);
   rspfString getOblAng()const;
   void setRollAng(const rspfString& rollAng);
   rspfString getRollAng()const;
   void setField10(const rspfString& field10);
   rspfString getField10()const;
   void setField11(const rspfString& field11);
   rspfString getField11()const;
   void setField12(const rspfString& field12);
   rspfString getField12()const;
   void setField13(const rspfString& field13);
   rspfString getField13()const;
   void setField14(const rspfString& field14);
   rspfString getField14()const;
   void setField15(const rspfString& field15);
   rspfString getField15()const;
   void setField16(const rspfString& field16);
   rspfString getField16()const;
   void setNRef(const rspfString& nRef);
   rspfString getNRef()const;
   void setRevNum(const rspfString& revNum);
   rspfString getRevNum()const;
   void setNSeg(const rspfString& nSeg);
   rspfString getNSeg()const;
   void setMaxLpSeg(const rspfString& maxLpSeg);
   rspfString getMaxLpSeg()const;
   void setField20(const rspfString& field20);
   rspfString getField20()const;
   void setField21(const rspfString& field21);
   rspfString getField21()const;
   void setSunEl(const rspfString& sunEl);
   rspfString getSunEl()const;
   void setSunAz(const rspfString& sunAz);
   rspfString getSunAz()const;

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
protected:
   
   /**
    * FIELD: ANGLETONORTH
    *
    */
   char theAngleToNorth[ANGLE_TO_NORTH_SIZE+1];
   
   /**
    * FIELD: MEANGSD
    *
    */
   char theMeanGsd[MEAN_GSD_SIZE+1];
   
   /**
    * FIELD: FIELD3
    *
    */
   char theField3[FIELD3_SIZE+1];

   /**
    * FIELD: DYNAMICRANGE
    *
    */
   char theDynamicRange[DYNAMIC_RANGE_SIZE+1];

   /**
    * FIELD: FIELD5
    *
    */
   char theField5[FIELD5_SIZE+1];

   /**
    * FIELD: FIELD6
    *
    */
   char theField6[FIELD6_SIZE+1];

   /**
    * FIELD: FIELD7
    *
    */
   char theField7[FIELD7_SIZE+1];

   /**
    * FIELD: OBLANG
    *
    */
   char theOblAng[OBL_ANG_SIZE+1];

   /**
    * FIELD: ROLLANG
    *
    */
   char theRollAng[ROLL_ANG_SIZE+1];

   /**
    * FIELD: FIELD10
    *
    */
   char theField10[FIELD10_SIZE+1];

   /**
    * FIELD: FIELD11
    *
    */
   char theField11[FIELD11_SIZE+1];

   /**
    * FIELD: FIELD12
    *
    */
   char theField12[FIELD12_SIZE+1];

   /**
    * FIELD: FIELD13
    *
    */
   char theField13[FIELD13_SIZE+1];

   /**
    * FIELD: FIELD14
    *
    */
   char theField14[FIELD14_SIZE+1];

   /**
    * FIELD: FIELD15
    *
    */
   char theField15[FIELD15_SIZE+1];

   /**
    * FIELD: FIELD16
    *
    */
   char theField16[FIELD16_SIZE+1];

   /**
    * FIELD: NREF
    *
    */
   char theNRef[N_REF_SIZE+1];

   /**
    * FIELD: REVNUM
    *
    */
   char theRevNum[REV_NUM_SIZE+1];

   /**
    * FIELD: NSEG
    *
    */
   char theNSeg[N_SEG_SIZE+1];

   /**
    * FIELD: MAXLPSEG
    *
    */
   char theMaxLpSeg[MAX_LP_SEG_SIZE+1];

   /**
    * FIELD: FIELD20
    *
    */
   char theField20[FIELD20_SIZE+1];

   /**
    * FIELD: FIELD21
    *
    */
   char theField21[FIELD21_SIZE+1];

   /**
    * FIELD: SUNEL
    *
    */
   char theSunEl[SUN_EL_SIZE+1];

   /**
    * FIELD: SUNAZ
    *
    */
   char theSunAz[SUN_AZ_SIZE+1];

TYPE_DATA   
};

#endif
