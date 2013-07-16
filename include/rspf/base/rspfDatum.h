//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// Contains class declaration for OssimDatum.  This is a class wrapper
// for Geotrans datum.  For more thorough description of each function
// look at the datum.h file.
//*******************************************************************
//  $Id: rspfDatum.h 19793 2011-06-30 13:26:56Z gpotts $
#ifndef rspfDatum_HEADER
#define rspfDatum_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfDatum : public rspfObject
{
   friend class rspfDatumFactory;

public:
   /**
    *   @param alpha_code     new RSPF/Geotrans datum code.                (input)
    *   @param name           Name of the new datum                         (input)
    *   @param SigmaX         Standard error in X in meters                 (input)
    *   @param SigmaY         Standard error in Y in meters                 (input)
    *   @param SigmaZ         Standard error in Z in meters                 (input)
    *   @param southLatitude  Southern edge of validity rectangle in radians(input)
    *   @param northLatitude  Northern edge of validity rectangle in radians(input)
    *   @param westLongitude  Western edge of validity rectangle in radians (input)
    *   @param eastLongitude  Eastern edge of validity rectangle in radians (input)
    */
    rspfDatum(const rspfString &alpha_code, const rspfString &name,
              const rspfEllipsoid* anEllipsoid,
              rspf_float64 sigmaX, rspf_float64 sigmaY, rspf_float64 sigmaZ,
              rspf_float64 westLongitude, rspf_float64 eastLongitude,
              rspf_float64 southLatitude, rspf_float64 northLatitude);

   // Argument holds the source point and datum.  Returns another
   // point with this datum.
   //
   virtual rspfGpt shift(const rspfGpt    &aPt)const=0;
   //utility functions to shift to and from the standard coordinates.
   //Users should use the shift instead!!!
   //
   virtual rspfGpt       shiftToWgs84(const rspfGpt &aPt)const = 0;
   virtual rspfGpt       shiftFromWgs84(const rspfGpt &aPt)const = 0;
   

   virtual bool  isTheSameAs(const rspfDatum *aDatum)const
      {return this == aDatum;}
   virtual const rspfString& code()const{return theCode;}
   virtual const rspfString& name()const{return theName;}
   virtual rspf_uint32 epsgCode()const{return theEpsgCode;}
   virtual const rspfEllipsoid* ellipsoid()const{return theEllipsoid;}
   virtual rspf_float64 sigmaX()const{return theSigmaX;}
   virtual rspf_float64 sigmaY()const{return theSigmaY;}
   virtual rspf_float64 sigmaZ()const{return theSigmaZ;}

   virtual rspf_float64 westLongitude()const{return theWestLongitude;}
   virtual rspf_float64 eastLongitude()const{return theEastLongitude;}
   virtual rspf_float64 southLatitude()const{return theSouthLatitude;}
   virtual rspf_float64 northLatitude()const{return theNorthLatitude;}
   
   virtual rspf_float64 param1()const=0;
   virtual rspf_float64 param2()const=0;
   virtual rspf_float64 param3()const=0;
   virtual rspf_float64 param4()const=0;
   virtual rspf_float64 param5()const=0;
   virtual rspf_float64 param6()const=0;
   virtual rspf_float64 param7()const=0;

   virtual bool isInside(rspf_float64 latitude, rspf_float64 longitude)const
   {
      return ((theSouthLatitude <= latitude) &&
              (latitude <= theNorthLatitude) &&
              (theWestLongitude <= longitude) &&
              (longitude <= theEastLongitude));
   }

   bool operator==(const rspfDatum& rhs) const;

   bool operator!=(const rspfDatum& rhs)const
   {
      return (!(*this == rhs));
   }
   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType=RSPF_COMPARE_FULL)const;

protected:
   //! Only friend rspfDatumFactory is permitted to delete
   virtual ~rspfDatum(){};

   /*!
    * This is directly from Geotrans:
    * Begin Molodensky_Shift
    * This function shifts geodetic coordinates using the Molodensky method.
    *
    *    a         : Semi-major axis of source ellipsoid in meters  (input)
    *    da        : Destination a minus source a                   (input)
    *    f         : Flattening of source ellipsoid                 (input)
    *    df        : Destination f minus source f                   (input)
    *    dx        : X coordinate shift in meters                   (input)
    *    dy        : Y coordinate shift in meters                   (input)
    *    dz        : Z coordinate shift in meters                   (input)
    */
   virtual void    molodenskyShift( double a,
                                    double da,
                                    double f,
                                    double df,
                                    double dx,
                                    double dy,
                                    double dz,
                                    double Lat_in,
                                    double Lon_in,
                                    double Hgt_in,
                                   double &Lat_out,
                                   double &Lon_out,
                                   double &Hgt_out)const;
   bool withinMolodenskyRange(const rspfGpt& pt)const
      {
         double lat = pt.latd();
         
         return ((lat < 89.75) && (lat > -89.75));
      }

   
protected:
   rspfString           theCode;
   rspf_uint32          theEpsgCode;
   rspfString           theName;
   const rspfEllipsoid *theEllipsoid;
   
   rspf_float64        theSigmaX;
   rspf_float64        theSigmaY;
   rspf_float64        theSigmaZ;

   rspf_float64        theWestLongitude;
   rspf_float64        theEastLongitude;
   rspf_float64        theSouthLatitude;
   rspf_float64        theNorthLatitude; 
   
TYPE_DATA;
};

#endif
