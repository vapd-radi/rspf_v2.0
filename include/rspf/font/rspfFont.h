#ifndef rspfFont_HEADER
#define rspfFont_HEADER
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfString.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfFontInformation.h>
#define RSPF_FONT_DEFAULT_SIZE_X 12
#define RSPF_FONT_DEFAULT_SIZE_Y 12
class rspfFont : public rspfObject, public rspfErrorStatusInterface
{
public:
   rspfFont()
      :theStringToRasterize(""),
       theFamilyName(""),
       theStyleName(""),
       theRotation(0),
       theHorizontalShear(0.0),
       theVerticalShear(0.0),
       theHorizontalScale(1.0),
       theVerticalScale(1.0),
       theHorizontalPixelSize(8),
       theVerticalPixelSize(8),
       theHorizontalDeviceUnits(72),
       theVerticalDeviceUnits(72),
       theAffineTransform(2,2)
      {
         theAffineTransform << 1.0 << 0.0
                            << 0.0 << 1.0;
      }
   
   rspfFont(const rspfString& familyName,
             const rspfString& styleName,
             int                horizontalPixelSize,
             int                verticalPixelSize,
             int                horizontalDeviceUnits=72,// 72 dpi is default,
             int                verticalDeviceUnits=72)// 72 dpi is default
      :theStringToRasterize(""),
       theFamilyName(familyName),
       theStyleName(styleName),
       theRotation(0),
       theHorizontalShear(0.0),
       theVerticalShear(0.0),
       theHorizontalScale(1.0),
       theVerticalScale(1.0),
       theHorizontalPixelSize(horizontalPixelSize),
       theVerticalPixelSize(verticalPixelSize),
       theHorizontalDeviceUnits(horizontalDeviceUnits),
       theVerticalDeviceUnits(verticalDeviceUnits),
       theAffineTransform(2,2)
      {
         theAffineTransform << 1.0 << 0.0
                            << 0.0 << 1.0;
      }
   rspfFont(const rspfFont& rhs)
      {
         theStringToRasterize      = rhs.theStringToRasterize;
         theFamilyName             = rhs.theFamilyName;
         theStyleName              = rhs.theStyleName;
         theRotation               = rhs.theRotation;
         theHorizontalShear        = rhs.theHorizontalShear;
         theVerticalShear          = rhs.theVerticalShear;
         theHorizontalScale        = rhs.theHorizontalScale;
         theVerticalScale          = rhs.theVerticalScale;
         theHorizontalPixelSize    = rhs.theHorizontalPixelSize;
         theHorizontalDeviceUnits  = rhs.theHorizontalDeviceUnits;
         theVerticalDeviceUnits    = rhs.theVerticalDeviceUnits;
         theAffineTransform        = rhs.theAffineTransform;
         theClippingBox            = rhs.theClippingBox;
         theLocalOrigin            = rhs.theLocalOrigin;
         theAffineTransform        = rhs.theAffineTransform;
      }
   virtual rspfObject* dup()const=0;
   /*!
    * Returns true if this font is a fixed size font
    */
   virtual bool isFixed()const=0;
   /*!
    * You can get a list of fixed sizes supported by
    * this font.
    */
   virtual void getFixedSizes(vector<rspfIpt>& sizes)const=0;
   /*!
    * Will return the previously rasterized buffer.
    */
   virtual const rspf_uint8* getBuf()const=0;
   
   /*!
    * Will return an internal buffer for rasterizing and will
    * also set the width, and height arguments of the returned buffer.
    */
   virtual const rspf_uint8* rasterize()=0;
   virtual const rspf_uint8* rasterize(const rspfString& s)
      {
         setString(s);
         return rasterize();
      }
   virtual void setString(const rspfString& s)
      {
         theStringToRasterize = s;
      }
   
   /*!
    * Will return the bounding box of the last string that was set
    * to be rasterized in pixel space
    */
   virtual void getBoundingBox(rspfIrect& box)=0;
   /*!
    * Specifies if its helvetica, times new roman ... etc.
    */ 
   virtual rspfString getFamilyName()const
      {
         return theFamilyName;
      }
   
   /*!
    * Specifies if its bold or italic .. etc
    */
   virtual rspfString getStyleName()const
      {
         return theStyleName;
      }
   /*!
    * Allow us to rotate a font.
    */
    virtual void setRotation(double rotationInDegrees);
    virtual double getRotation()const
      {
         return theRotation;
      }
   
   virtual void getRotation(double& rotation)const
      {
         rotation = theRotation;
      }
   virtual void setHorizontalVerticalShear(double horizontalShear,
                                           double verticalShear)
      {
         theVerticalShear   = verticalShear;
         theHorizontalShear = horizontalShear;
         computeMatrix();
      }
   virtual void setVerticalShear(double verticalShear)
      {
         setHorizontalVerticalShear(theHorizontalShear,
                                    verticalShear);
      }
   virtual void setHorizontalShear(double horizontalShear)
      {
         setHorizontalVerticalShear(horizontalShear,
                                    theVerticalShear);
      }
   virtual double getVerticalShear()const
      {
         return theVerticalShear;
      }
   virtual double getHorizontalShear()const
      {
         return theHorizontalShear;
      }
   /*!
    * Sets the scale of the font. I'll have code in place to perform
    * reflections.  This can be specified with a - scale along the direction
    * you wish to reflect.
    */
   virtual void setScale(double horizontal,
                         double vertical);
   
   virtual void getScale(double& horizontal,
                         double& vertical)
      {
         horizontal = theHorizontalScale;
         vertical   = theVerticalScale;
      }
   virtual void setPixelSize(int horizontalPixelSize,
                             int verticalPixelSize)
      {
         theHorizontalPixelSize = horizontalPixelSize;
         theVerticalPixelSize   = verticalPixelSize;
      }
   virtual void setPixelSize(int pixelSize)
      {
         setPixelSize(pixelSize, pixelSize);
      }
   
   virtual void getPixelSize(int& horizontal,
                             int& vertical)
      {
         horizontal = theHorizontalPixelSize;
         vertical   = theVerticalPixelSize;
      }
   virtual long getHorizontalPixelSize()const
      {
         return theHorizontalPixelSize;
      }
   virtual rspfIpt getPixelSize()const
      {
         return rspfIpt(theHorizontalPixelSize,
                         theVerticalPixelSize);
      }
   virtual long getVerticalPixelSize()const
      {
         return theVerticalPixelSize;
      }
   virtual void setDeviceDpi(int horizontalDeviceUnits,
                             int verticalDeviceUnits)
      {
         theHorizontalDeviceUnits = horizontalDeviceUnits;
         theVerticalDeviceUnits   = verticalDeviceUnits;
      }
   
   /*!
    * Will clip the clipping box with the bounding box of the string and this
    * is the viewable information.
    *
    * If the clipping box is set then this should be used to get the dimensions
    * of the buffer.
    */
   virtual void getBufferRect(rspfIrect& bufRect)
      {
         getBoundingClipBox(bufRect);
      }
   
   virtual rspfIrect getBufferRect()
      {
         return getBoundingClipBox();
      }
   
   virtual void getBoundingClipBox(rspfIrect& clippedBox);
   
   virtual rspfIrect getBoundingClipBox();
   
   virtual void setClippingBox(const rspfIrect& clipBox=rspfIrect(0,0,0,0))
      {
         theClippingBox = clipBox;
      }
   
   virtual rspfIrect getClippingBox()const
      {
         return theClippingBox;
      }
   bool isIdentityTransform()const
      {
         return ( (theAffineTransform[0][0] == 1.0) && (theAffineTransform[1][0] == 0.0)&&
                  (theAffineTransform[0][1] == 0.0) && (theAffineTransform[1][1] == 1.0));
      }
   virtual void getFontInformation(vector<rspfFontInformation>& fontInformationList)const
      {
         fontInformationList.push_back(rspfFontInformation(getFamilyName(),
                                                            getStyleName(),
                                                            getPixelSize(),
                                                            isFixed()));         
      }
protected:
   virtual ~rspfFont()
   {}
   rspfString theStringToRasterize;
   rspfString theFamilyName;
   rspfString theStyleName;
   double      theRotation;
   double      theHorizontalShear;
   double      theVerticalShear;
   double      theHorizontalScale;
   double      theVerticalScale;
   int         theHorizontalPixelSize;
   int         theVerticalPixelSize;
   int         theHorizontalDeviceUnits;
   int         theVerticalDeviceUnits;
   rspfIrect  theClippingBox;
   /*!
    * During the compution of the global bounding rect we will
    * Make sure that we save off the upper left since its not really 0,0.
    * We will make sure that the bounding rect is shifted to 0,0 but the
    * shift will be saved.  That way we can render the shape correctly
    * later.
    */
   mutable rspfIpt    theLocalOrigin;
   
   NEWMAT::Matrix  theAffineTransform;
   void computeMatrix();
TYPE_DATA
};
#endif
