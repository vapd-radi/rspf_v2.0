//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationEllipseObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationEllipseObject_HEADER
#define rspfAnnotationEllipseObject_HEADER

#include <rspf/imaging/rspfAnnotationObject.h>

class RSPFDLLEXPORT rspfAnnotationEllipseObject : public rspfAnnotationObject
{
public:
   rspfAnnotationEllipseObject(const rspfDpt& center = rspfDpt(0,0),
                                const rspfDpt& widthHeight = rspfDpt(1,1),
                                rspf_float64 azimuthInDegrees=0.0,
                                bool enableFill = false,
                                rspf_uint8 r = 255,
                                rspf_uint8 g = 255,
                                rspf_uint8 b = 255,
                                rspf_uint8 thickness = 1);
   rspfAnnotationEllipseObject(const rspfAnnotationEllipseObject& rhs);

   virtual rspfObject* dup()const;


   virtual void applyScale(double x, double y);

   virtual void draw(rspfRgbImage& anImage)const;

   virtual bool intersects(const rspfDrect& rect)const;

   virtual rspfAnnotationObject* getNewClippedObject(
      const rspfDrect& rect)const;
   
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();
  /*!
   */
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   virtual void setCenterWidthHeight(const rspfDpt& center,
                                     const rspfDpt& widthHeight);

   /**
    * @brief Sets the azimuth.
    * @param azimuth in degrees.
    */
   virtual void setAzimuth(rspf_float64 azimuth);

   /**
    * @brief Gets the azimuth.
    * @return The azimuth in decimal degrees.
    */
   virtual rspf_float64 getAzimuth() const;
   
   virtual void setFillFlag(bool enabled);

   /** @return theFillEnabled flag. */
   virtual bool getFillFlag() const;

   /**
    * @brief Turns on and off drawing of axes on the ellipse.
    * This flag is only used if fill is turned off.
    *
    * @param flag true to enable, false to disable.
    */
   virtual void setDrawAxesFlag(bool flag);

   /** @return The draw axes flag. */
   virtual bool getDrawAxesFlag() const;

   /**
    * Saves the current state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
protected:
   virtual ~rspfAnnotationEllipseObject();
   rspfDpt      theCenter;
   rspfDpt      theWidthHeight;
   rspf_float64 theAzimuthInDegrees;
   bool          theFillEnabled;
   bool          theDrawAxesFlag;
   rspfDrect    theBoundingRect;

TYPE_DATA
};

#endif
