//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// rspfThreeParamDatum. This is a base class for all
// three param datums.
//*******************************************************************
//  $Id: rspfThreeParamDatum.h 19795 2011-06-30 15:04:48Z gpotts $

#ifndef rspfThreeParamDatum_HEADER
#define rspfThreeParamDatum_HEADER
#include <rspf/base/rspfDatum.h>

class RSPFDLLEXPORT rspfThreeParamDatum : public rspfDatum
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
    */
   rspfThreeParamDatum(const rspfString &code, const rspfString &name,
                        const rspfEllipsoid* anEllipsoid,
                        rspf_float64 sigmaX,
                        rspf_float64 sigmaY,
                        rspf_float64 sigmaZ,
                        rspf_float64 westLongitude,
                        rspf_float64 eastLongitude,
                        rspf_float64 southLatitude,
                        rspf_float64 northLatitude,
                        rspf_float64 param1,
                        rspf_float64 param2,
                        rspf_float64 param3)
      : rspfDatum(code, name, anEllipsoid,
              sigmaX, sigmaY, sigmaZ,
              westLongitude, eastLongitude,
              southLatitude, northLatitude),
        theParam1(param1),
        theParam2(param2),
        theParam3(param3)
      {}
   virtual ~rspfThreeParamDatum(){}

   /*!
    * Will shift a ground point to this datum.  If they are the same
    * datum then nothing happens.
    */
   virtual rspfGpt    shift(const rspfGpt    &aPt)const;
   
   virtual rspfGpt    shiftToWgs84(const rspfGpt &aPt)const;
   virtual rspfGpt    shiftFromWgs84(const rspfGpt &aPt)const;
   virtual rspf_float64 param1()const{return theParam1;}
   virtual rspf_float64 param2()const{return theParam2;}
   virtual rspf_float64 param3()const{return theParam3;}
   virtual rspf_float64 param4()const{return 0.0;}
   virtual rspf_float64 param5()const{return 0.0;}
   virtual rspf_float64 param6()const{return 0.0;}
   virtual rspf_float64 param7()const{return 1.0;}

   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType=RSPF_COMPARE_FULL)const;

protected:
   
   rspf_float64 theParam1;
   rspf_float64 theParam2;
   rspf_float64 theParam3;

   TYPE_DATA;
};


#endif
