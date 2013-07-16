#ifndef rspfMapProjectionInfo_HEADER
#define rspfMapProjectionInfo_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfString.h>
class rspfMapProjection;
class rspfImageChain;
class RSPFDLLEXPORT rspfMapProjectionInfo : public rspfObject
{
public:
   rspfMapProjectionInfo(const rspfMapProjection* proj,
                          const rspfDrect& output_rect);
   virtual ~rspfMapProjectionInfo();
   /**
    *  Returns true on error false if ok.
    */
   bool errorStatus() const;
   /**
    * Adds tie point to keyword list.  This will be latitude, longitude if
    * projection is geographic; else, easting, northing.
    */
   void getGeom(rspfKeywordlist& kwl, const char* prefix=0)const;
   /**
    *  Returns the pixel size in meters.
    *  x = sample direction
    *  y = line direction
    */
   rspfDpt getMetersPerPixel() const;
   
   /**
    *  Returns the pixel size in U.S. Survey feet.
    *  x = sample direction
    *  y = line direction
    *
    *  NOTE:
    *  The US survey feet per meter converion (0.3048006096 feet per meter)
    *  is NOT the same as the standard feet per meter conversion
    *  (0.3048 feet per meter).
    */
   rspfDpt getUsSurveyFeetPerPixel() const;
   /**
    *  Returns the decimal degrees per pixel.
    *  x = sample direction
    *  y = line direction
    */
   rspfDpt getDecimalDegreesPerPixel() const;
   
   /**
    *  Returns the upper left easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v". Units are in meters.
    */
   rspfDpt ulEastingNorthingPt() const;
   /**
    *  Returns the upper right easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v". Units are in meters.
    */
   rspfDpt urEastingNorthingPt() const;
   /**
    *  Returns the lower right easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v". Units are in meters.
    */
   rspfDpt lrEastingNorthingPt() const;
   /**
    *  Returns the lower left easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v". Units are in meters.
    */
   rspfDpt llEastingNorthingPt() const;
   
   /**
    *  Returns the upper left easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v".
    *  Units are in U.S. Survey feet.
    */
   rspfDpt ulEastingNorthingPtInFt() const;
   /**
    *  Returns the upper right easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v".
    *  Units are in U.S. Survey feet.
    */
   rspfDpt urEastingNorthingPtInFt() const;
   /**
    *  Returns the lower right easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v".
    *  Units are in U.S. Survey feet.
    */
   rspfDpt lrEastingNorthingPtInFt() const;
   /**
    *  Returns the lower left easting and northing as a rspfDpt.
    *  The easting is "u" and the northing is "v". 
    *  Units are in U.S. Survey feet.
    */
   rspfDpt llEastingNorthingPtInFt() const;
   
   /**
    * Returns the upper left ground point.
    */
   rspfGpt ulGroundPt() const;
   /**
    * Returns the upper right ground point.
    */
   rspfGpt urGroundPt() const;
   /**
    * Returns the lower right ground point.
    */
   rspfGpt lrGroundPt() const;
 
   /**
    * Returns the lower left ground point.
    */
   rspfGpt llGroundPt() const;
   /**
    * Returns the center ground point.
    */
   rspfGpt centerGroundPt() const;
   /** @return The center ground point. */
   rspfDpt centerEastingNorthingPt() const;
   /** @return The lines per image. */
   rspf_int32 linesPerImage() const;
   /** @return The pixels per line (samples). */
   rspf_int32 pixelsPerLine() const;
   /**
    * Formatted print of data members.  Used by operator <<.
    */
   std::ostream& print(std::ostream& os) const;
   /**
    *  Sets the data member "thePixelType".
    *  NOTE:
    *  PIXEL_IS_POINT (ennumerated to a "0"):
    *  Corner coordinates are relative to the center of pixel.
    *  PIXEL_IS_AREA (ennumerated to a "1"):
    *  Corner coordinates are relative to the outer edge of the pixel.
    */
   void setPixelType (rspfPixelType type);
   /**
    *  Returns data member "thePixelType".
    *  NOTE:
    *  RSPF_PIXEL_IS_POINT (ennumerated to a "0"):
    *  Corner coordinates are relative to the center of pixel.
    *  RSPF_PIXEL_IS_AREA (ennumerated to a "1"):
    *  Corner coordinates are relative to the outer of the pixel.
    */
   rspfPixelType  getPixelType () const;
   /**
    * Sets the data member "theOutputInfoInFeetFlag".
    */
   void setOutputFeetFlag (bool flag);
   /**
    *  Returns the data member "theOutputInfoInFeetFlag".
    *  True = feet, false = meters.
    */
   bool unitsInFeet() const;
   /**
    * Returns theImageInfoString.
    */
   rspfString getImageInfoString () const;
   /**
    *  Used to set data member "theImageInfoString".  This will be output
    *  in the "Image: " field of the README file.
    */
   void setImageInfoString (const rspfString& string);
   /**
    * Returns reference to "theProjection".
    */
   const rspfMapProjection* getProjection() const;
   static const char* README_IMAGE_STRING_KW;
   static const char* PIXEL_TYPE_KW;
   static const char* OUTPUT_US_FT_INFO_KW;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0) const;
   
private:
   rspfMapProjectionInfo();
   rspfMapProjectionInfo& operator=(const rspfMapProjectionInfo&);
   
   void initializeMembers(const rspfDrect& output_rect);
   
   const rspfMapProjection* theProjection;
   mutable bool              theErrorStatus;
   int                       theLinesPerImage;
   int                       thePixelsPerLine;
   rspfGpt                  theCornerGroundPt[4];
   rspfDpt                  theCornerEastingNorthingPt[4];
   rspfGpt                  theCenterGroundPt;
   rspfDpt                  theCenterEastingNorthingPt;
   rspfDrect                theBoundingRect;
   
   /**
    *   "thePixelType" defines whether the tie point coordinates are
    *   relative to the center of the pixel(which is PIXEL_IS_POINT) or
    *   the upper left corner of the pixel(PIXEL_IS_AREA).  The default
    *   pixle type is PIXEL_IS_POINT.
    */
   rspfPixelType                 thePixelType;
   /**
    *   If "theOutputInfoInFeetFlag" is true the readme file tie point
    *   info will be output in both meters and U.S. feet.
    *   NOTE:
    *   U.S. feet conversion is "meters/0.3048006096" not the standard
    *   "meters/0.3048".
    *   Reference for U.S. feet value:
    *   U.S.G.S. "Map Projections - A Working Manual" USGS Professional
    *   paper 1395 page 51.
    */
   bool                      theOutputInfoInFeetFlag;
   /**
    *   "theImageInfoString" goes in the README file "Image:" field
    *   to identify the image.
    */
   rspfString               theImageInfoString;
};
#endif /* End of #ifndef rspfMapProjectionInfo_HEADER */
