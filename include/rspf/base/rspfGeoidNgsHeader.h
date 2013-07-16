//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
//*******************************************************************
//  $Id: rspfGeoidNgsHeader.h 11496 2007-08-06 09:18:28Z dburken $
#ifndef rspfGeoidNgsHeader_HEADER
#define rspfGeoidNgsHeader_HEADER
#include <rspf/base/rspfFilename.h>

#include <iostream>

class RSPFDLLEXPORT rspfGeoidNgsHeader
{
public:
   friend std::ostream& operator << (std::ostream &out, const rspfGeoidNgsHeader &data);
   
   rspfGeoidNgsHeader();
   rspfGeoidNgsHeader(const rspfFilename &fileName,
                       rspfByteOrder byteOrder=RSPF_LITTLE_ENDIAN);

   bool initialize(const rspfFilename &fileName,
                   rspfByteOrder byteOrder=RSPF_LITTLE_ENDIAN);
   
   double southernMostLat()const{return theSouthernMostLatitude;}
   double westernMostLon()const{return theWesternMostLongitude;}
   double latDelta()const{return theLatDelta;}
   double lonDelta()const{return theLonDelta;}
   int    rows()const{return theRows;}
   int    cols()const{return theCols;}
   int    dataType()const{return theDataType;}
   bool   pointWithin(double lat, double lon)const;
   const rspfFilename& filename()const{return theFilename;}
   
   int headerSize()const{return 44;}
   int dataTypeSize()const{return theDataType==1?4:0;}

   /** @return Height delta or rspf::nan() if not found. */
   double getHeightDelta(double lat, double lon)const;
   
private:
   rspfFilename theFilename;
   rspfByteOrder theByteOrder;
   double theSouthernMostLatitude;

   /*!
    * Important:  The geoid.bin files expresses this as a possitive value.
    *
    * example: -90.0 is 270 degrees.
    */
   double theWesternMostLongitude;

   /*!
    * Specifies the spacing of the latitude direction.
    */
   double theLatDelta;

   /*!
    * longitude spacing.
    */
   double theLonDelta;

   /*!
    * theRows specifies the number of lat increments
    */
   int    theRows;

   /*!
    * theCols specifies the number of lon increments
    */
   int    theCols;

   /*!
    * Specifies the datatype.  if this is 1 then the data that follows
    * is 4 byte floats.
    */
   int    theDataType;
};

#endif
