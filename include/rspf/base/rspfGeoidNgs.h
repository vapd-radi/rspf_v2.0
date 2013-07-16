//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: rspfGeoidNgs.h 11496 2007-08-06 09:18:28Z dburken $
#ifndef rspfGeoidNgs_HEADER
#define rspfGeoidNgs_HEADER

#include <rspf/base/rspfGeoid.h>
#include <rspf/base/rspfGeoidNgsHeader.h>

class RSPFDLLEXPORT rspfGeoidNgs : public rspfGeoid
{

public:
   rspfGeoidNgs();
   rspfGeoidNgs(const rspfFilename& dir,
                 rspfByteOrder byteOrder = RSPF_LITTLE_ENDIAN);
   virtual ~rspfGeoidNgs();

   virtual rspfString getShortName()const;
   
   virtual bool open(const rspfFilename& dir,
                     rspfByteOrder byteOrder = RSPF_LITTLE_ENDIAN);

   virtual bool addFile(const rspfFilename& file,
                        rspfByteOrder byteOrder = RSPF_LITTLE_ENDIAN);
   /**
    *  @return The offset from the ellipsoid to the geoid or rspf::nan()
    *  if grid does not contain the point.
    */
   virtual double offsetFromEllipsoid(const rspfGpt&) const;

   /**
    *  @return Geoid to ellipsoid height or rspf::nan()
    *  if grid does not contain the point.
    */
   double geoidToEllipsoidHeight(double lat,
                                 double lon,
                                 double geoidHeight) const;

   /**
    *  @return Ellipsoid to geoid height or rspf::nan()
    *  if grid does not contain the point.
    */
   double ellipsoidToGeoidHeight(double lat,
                                 double lon,
                                 double ellipsoidHeight) const;
protected:
   void fixLatLon(double &lat, double &lon) const;
   double deltaHeight(double lat, double lon)const;
   
   std::vector<rspfGeoidNgsHeader> theGridVector;
   rspfString theShortName;

   TYPE_DATA
};

#endif
