//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfUsgsQuad.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef  rspfUsgsQuad_HEADER
#define rspfUsgsQuad_HEADER
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfErrorStatusInterface.h>

#include <vector>

class RSPFDLLEXPORT  rspfUsgsQuad
{
public:
   rspfUsgsQuad(const rspfGpt& lrGpt);
   rspfUsgsQuad(const rspfString& name,
                 const rspfDatum* datum);
//                 double paddingInMeters = 100);

   void setQuadName(const rspfString& name);
   /*!  Returns the bounding rectangle of the quarter quad segment.
    *   A segment is 1/4 of a quarter quad.
    *   Segment assingments:  upper_left=A, upper_right=B, lower_left=C,
    *   lower_right=D   NOTE:  padding is added in.
    */
   rspfGrect quarterQuadSegRect(char seg);

   /*!  Returns the bounding rectangle of the quarter quad segment
    *   in the "area.lat_lon" keyword format:
    *   start_lat, start_lon, stop_lat, stop_lon
    *   NOTE:  padding is added in.
    */
   rspfString quarterQuadSegKwRect(char seg);

   /*!  Returns the bounding rectangle of the quarter quad (3.75 minutes).  
    *   NOTE:  padding is added in.
    */
   rspfGrect quarterQuadRect() const;

   /*!  Returns the bounding rectangle of the quarter quad (3.75 minutes)  
    *   in the "area.lat_lon" keyword format:
    *   start_lat, start_lon, stop_lat, stop_lon
    *   NOTE:  padding is added in.
    */
   rspfString quarterQuadKwRect() const;

   /*!  Returns the bounding rectangle of the quarter quad (7.5 minutes).  
    *   NOTE:  padding is added in.
    */
   rspfGrect quadRect() const;

   /*!  Returns the bounding rectangle of the quarter quad (7.5 minutes)
    *   in the "area.lat_lon" keyword format:
    *   start_lat, start_lon, stop_lat, stop_lon
    *   NOTE:  padding is added in.
    */
   rspfString quadKwRect() const;

   /*!
    * Returns the USGS quarter quad name.
    */
   rspfString quarterQuadName() const
      {
         return rspfString(theName.begin(),
                            theName.begin()+7);
      }

   rspfString quarterQuadSegName() const
      {
         return theName;
      }

   
   /*!
    *  Returns the lower right hand corner of quad with no padding.
    */
   rspfGpt lrQuadCorner() const { return theQuadLowerRightCorner ; }
   
   /*!
    * Returns the lower right hand corner of quarter quad with no padding.
    */
   rspfGpt lrQuarterQuadCorner() const
      { return theQuarterQuadLowerRightCorner; }

   static void getQuadList(std::vector<rspfUsgsQuad>& result,
                           const rspfGrect& rect);
   
protected:
   rspfString         theName;
   rspfGpt            theQuadLowerRightCorner; // Lower right corner of quad.
   rspfGpt            theQuarterQuadLowerRightCorner;// quarter quad lr corner.
//   rspfDpt            thePaddingInDegrees;
   char                theQuarterQuadSegment; // A, B, C, D
};

#endif
