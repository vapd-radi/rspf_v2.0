//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGrect.h 22197 2013-03-12 02:00:55Z dburken $
#ifndef rspfGrect_HEADER
#define rspfGrect_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfCommon.h>

#include <vector>
#include <iostream>

class RSPF_DLL rspfGrect
{
public:
   friend RSPF_DLL std::ostream& operator<<(std::ostream& os, const rspfGrect& rect);

   /**
    * Will default to 0,0,0,0.
    */
    rspfGrect()
      :
         theUlCorner(0.0, 0.0, 0.0),
         theUrCorner(0.0, 0.0, 0.0),
         theLrCorner(0.0, 0.0, 0.0),
         theLlCorner(0.0, 0.0, 0.0)
      {}

   /**
    * Copies the passed in rectangle to this
    * object.
    */
   rspfGrect(const rspfGrect& rect)
      :
         theUlCorner(rect.ul()),
         theUrCorner(rect.ur()),
         theLrCorner(rect.lr()),
         theLlCorner(rect.ll())
      {}

   /**
    * WIll take two ground points and fill the
    * bounding rect appropriately.
    */
   rspfGrect(const rspfGpt& ul,
              const rspfGpt& lr)
      : 
         theUlCorner(ul),
         theUrCorner(ul.latd(), lr.lond(),0,ul.datum()),
         theLrCorner(lr),
         theLlCorner(lr.latd(), ul.lond(), 0, ul.datum())
      {
      }

   /**
    * Takes the upper left and lower right ground
    * points
    */
   rspfGrect(double ulLat,
              double ulLon,
              double lrLat,
              double lrLon,
              const rspfDatum* aDatum=rspfDatumFactory::instance()->wgs84())
      : 
         theUlCorner(ulLat, ulLon,0, aDatum),
         theUrCorner(ulLat, lrLon, 0, aDatum),
         theLrCorner(lrLat, lrLon, 0, aDatum),
         theLlCorner(lrLat, ulLon, 0, aDatum)
      {}
   rspfGrect(const rspfGpt& point,
              double latSpacingInDegrees,
              double lonSpacingInDegrees)
      :
         theUlCorner(point),
         theUrCorner(point),
         theLrCorner(point),
         theLlCorner(point)
      {
         std::vector<rspfGrect> v;
         computeEvenTiles(v, latSpacingInDegrees, lonSpacingInDegrees);
         if(v.size())
            *this = v[0];
      }
   rspfGrect(std::vector<rspfGpt>& points);
   rspfGrect(const rspfGpt& p1,
              const rspfGpt& p2,
              const rspfGpt& p3,
              const rspfGpt& p4);
              

   const rspfGrect& operator=(const rspfGrect& rect)
      {
         theUlCorner = rect.ul();
         theUrCorner = rect.ur();
         theLrCorner = rect.lr();
         theLlCorner = rect.ll();
         return *this;
      }

   inline rspfGpt midPoint()const;
   
   /**
    * Returns the height of a rectangle.
    */
   inline rspf_float64 height() const;

   /**
    * Returns the width of a rectangle.
    */
   inline rspf_float64 width()  const;
   
   inline const rspfGpt& ul()const;
   inline const rspfGpt& ur()const;
   inline const rspfGpt& ll()const;
   inline const rspfGpt& lr()const;
   
   inline rspfGpt& ul();
   inline rspfGpt& ur();
   inline rspfGpt& ll();
   inline rspfGpt& lr();
	
   inline void makeNan();
   
   inline bool isLonLatNan()const;
	
   inline bool hasNans()const;
   
   inline bool isNan()const;
   
   /*!
    * Returns true if "this" rectangle is contained completely within the
    * input rectangle "rect".
    */
   bool completely_within(const rspfGrect& rect) const;
	
   /*!
    * Returns true if any portion of an input rectangle "rect" intersects
    * "this" rectangle.  
    */
   bool intersects(const rspfGrect& rect) const;

   inline rspfGrect clipToRect(const rspfGrect& rect)const;

   inline rspfGrect combine(const rspfGrect& rect)const;

   /**
    * METHOD: pointWithin(rspfGpt)
    *
    * @param gpt Point to test for withinness.
    * 
    * @return true if argument is inside of horizontal rectangle
    *
    * @note Height is not considered and there is no datum shift applied if
    * gpt is of a different datum than this datum.
    */
   inline bool pointWithin(const rspfGpt& gpt) const; //inline below

   rspfGrect stretchToEvenBoundary(double latSpacingInDegrees,
                                    double lonSpacingInDegrees)const;
   
   void computeEvenTiles(std::vector<rspfGrect>& result,
                         double latSpacingInDegrees,
                         double lonSpacingInDegrees,
                         bool clipToGeographicBounds = true)const;
private:
   rspfGpt theUlCorner;
   rspfGpt theUrCorner;
   rspfGpt theLrCorner;
   rspfGpt theLlCorner;
  
};

//==================== BEGIN INLINE DEFINITIONS ===============================

//*****************************************************************************
//  INLINE METHOD: rspfGrect::midPoint()
//*****************************************************************************
inline rspfGpt rspfGrect::midPoint()const
{
   return rspfGpt((ul().latd() + ur().latd() + ll().latd() + lr().latd())*.25,
                   (ul().lond() + ur().lond() + ll().lond() + lr().lond())*.25,
                   (ul().height()+ur().height()+ll().height()+
                    lr().height())*.25,
                   ul().datum() );
}

//*****************************************************************************
//  INLINE METHOD: rspfGrect::clipToRect()
//*****************************************************************************
inline rspfGrect rspfGrect::clipToRect(const rspfGrect& rect)const
{
    double     ulx, uly, lrx, lry;

    ulx = rspf::max<rspf_float64>(rect.ul().lond(),ul().lond());
    uly = rspf::min<rspf_float64>(rect.ul().latd(),ul().latd());
    lrx = rspf::min<rspf_float64>(rect.lr().lond(),lr().lond());
    lry = rspf::max<rspf_float64>(rect.lr().latd(),lr().latd());

    if( lrx < ulx || lry > uly )
    {
       return rspfGrect(rspfGpt(0,0,0),rspfGpt(0,0,0));
    }
    else
    {
       return rspfGrect(rspfGpt(uly, ulx, 0, rect.ul().datum()),
                         rspfGpt(lry, lrx, 0, rect.ul().datum()));
    }
}

//*******************************************************************
// Inline Method: rspfDrect::combine(const rspfDrect& rect)
//*******************************************************************
inline rspfGrect rspfGrect::combine(const rspfGrect& rect)const
{
   rspfGpt ulCombine;
   rspfGpt lrCombine;
	
	ulCombine.lon = ((ul().lon <= rect.ul().lon)?ul().lon:rect.ul().lon);
   ulCombine.lat = ((ul().lat >= rect.ul().lat)?ul().lat:rect.ul().lat);
   lrCombine.lon = ((lr().lon >= rect.lr().lon)?lr().lon:rect.lr().lon);
   lrCombine.lat = ((lr().lat <= rect.lr().lat)?lr().lat:rect.lr().lat);
	
   return rspfGrect(ulCombine, lrCombine);
}

//*****************************************************************************
//  INLINE METHOD: rspfGrect::pointWithin()
//*****************************************************************************
inline bool rspfGrect::pointWithin(const rspfGpt& gpt) const
{
   return ((gpt.lat <= theUlCorner.lat) && (gpt.lat >= theLrCorner.lat) &&
           (gpt.lon >= theUlCorner.lon) && (gpt.lon <= theLrCorner.lon));
}

inline rspf_float64 rspfGrect::height() const
{
   return fabs(theLlCorner.latd() - theUlCorner.latd());
}

inline rspf_float64 rspfGrect::width() const
{
   return fabs(theLrCorner.lond() - theLlCorner.lond());
}

inline const rspfGpt& rspfGrect::ul() const
{
   return theUlCorner;
}

inline const rspfGpt& rspfGrect::ur() const
{
   return theUrCorner;
}

inline const rspfGpt& rspfGrect::ll() const
{
   return theLlCorner;
}

inline const rspfGpt& rspfGrect::lr() const
{
   return theLrCorner;
}
   
inline rspfGpt& rspfGrect::ul()
{
   return theUlCorner;
}

inline rspfGpt& rspfGrect::ur()
{
   return theUrCorner;
}

inline rspfGpt& rspfGrect::ll()
{
   return theLlCorner;
}

inline rspfGpt& rspfGrect::lr()
{
   return theLrCorner;
}
	
inline void rspfGrect::makeNan()
{
   theUlCorner.makeNan();
   theLlCorner.makeNan();
   theLrCorner.makeNan();
   theUrCorner.makeNan();
}

inline bool rspfGrect::isLonLatNan() const
{
   return ( rspf::isnan(theUlCorner.lat) ||
            rspf::isnan(theUlCorner.lon) ||
            rspf::isnan(theUrCorner.lat) ||
            rspf::isnan(theUrCorner.lon) ||
            rspf::isnan(theLrCorner.lat) ||
            rspf::isnan(theLrCorner.lon) ||
            rspf::isnan(theLlCorner.lat) ||
            rspf::isnan(theLlCorner.lon) );
}

inline bool rspfGrect::hasNans() const
{
   return ( theUlCorner.hasNans() ||
            theLlCorner.hasNans() ||
            theLrCorner.hasNans() ||
            theUrCorner.hasNans() );
}

inline bool rspfGrect::isNan()const
{
   return ( theUlCorner.hasNans() &&
            theLlCorner.hasNans() &&
            theLrCorner.hasNans() &&
            theUrCorner.hasNans() );
}

#endif /* End of "#ifndef rspfGrect_HEADER" */
