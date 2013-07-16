//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
// Description:
//
// SevenParamDatum. This is a base class for all
// seven param datums.
//*******************************************************************
//  $Id: rspfSevenParamDatum.h 19795 2011-06-30 15:04:48Z gpotts $

#ifndef rspfSevenParamDatum_HEADER
#define rspfSevenParamDatum_HEADER
#include <rspf/base/rspfDatum.h>

class rspfEllipsoid;

class RSPF_DLL rspfSevenParamDatum : public rspfDatum
{
public:
   /**
    *   @param code           new datum code.                               (input)
    *   @param name           Name of the new datum                         (input)
    *   @param SigmaX         Standard error in X in meters                 (input)
    *   @param SigmaY         Standard error in Y in meters                 (input)
    *   @param SigmaZ         Standard error in Z in meters                 (input)
    *   @param southLatitude  Southern edge of validity rectangle in radians(input)
    *   @param northLatitude  Northern edge of validity rectangle in radians(input)
    *   @param westLongitude  Western edge of validity rectangle in radians (input)
    *   @param eastLongitude  Eastern edge of validity rectangle in radians (input)
    *   @param param1         X translation
    *   @param param2         Y translation
    *   @param param3         Z translation
    *   @param param4         X Rotation
    *   @param param5         Y Rotation
    *   @param param6         Z Rotation
    *   @param param7         x,y,z Scale
    */
   rspfSevenParamDatum(const rspfString &code, const rspfString &name,
                        const rspfEllipsoid* anEllipsoid,
                        rspf_float64 sigmaX,
                        rspf_float64 sigmaY,
                        rspf_float64 sigmaZ,
                        rspf_float64 westLongitude,
                        rspf_float64 eastLongitude,
                        rspf_float64 southLatitude,
                        rspf_float64 northLatitude,
                        rspf_float64 aParam1,
                        rspf_float64 aParam2,
                        rspf_float64 aParam3,
                        rspf_float64 aParam4,
                        rspf_float64 aParam5,
                        rspf_float64 aParam6,
                        rspf_float64 aParam7)
      : rspfDatum(code, name, anEllipsoid,
                   sigmaX, sigmaY, sigmaZ,
                   westLongitude, eastLongitude,
                   southLatitude, northLatitude),
        theParam1(aParam1),
        theParam2(aParam2),
        theParam3(aParam3),
        theParam4(aParam4),
        theParam5(aParam5),
        theParam6(aParam6),
        theParam7(aParam7)
      {}
   

   /*!
    * Will shift the ground point relative to this datum.
    */
   rspfGpt         shift(const rspfGpt    &aPt)const;
   virtual rspfGpt shiftToWgs84(const rspfGpt &aPt)const;
   virtual rspfGpt shiftFromWgs84(const rspfGpt &aPt)const;
   
   virtual rspf_float64 param1()const{return theParam1;}
   virtual rspf_float64 param2()const{return theParam2;}
   virtual rspf_float64 param3()const{return theParam3;}
   virtual rspf_float64 param4()const{return theParam4;}
   virtual rspf_float64 param5()const{return theParam5;}
   virtual rspf_float64 param6()const{return theParam6;}
   virtual rspf_float64 param7()const{return theParam7;}
   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType=RSPF_COMPARE_FULL)const;
   
private:
   rspf_float64 theParam1;
   rspf_float64 theParam2;
   rspf_float64 theParam3;
   rspf_float64 theParam4;
   rspf_float64 theParam5;
   rspf_float64 theParam6;
   rspf_float64 theParam7;
   
   TYPE_DATA;
};

#endif
