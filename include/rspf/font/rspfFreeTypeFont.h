#ifndef rspfFreeTypeFont_HEADER
#define rspfFreeTypeFont_HEADER
#include <rspf/rspfConfig.h> /* To pick up RSPF_HAS_FREETYPE. */
#if RSPF_HAS_FREETYPE
#  include "rspfFont.h"
#  include <ft2build.h>
#  include FT_FREETYPE_H
#  include FT_MODULE_H // for FT_Done_Library
#  include FT_GLYPH_H
#  include "rspf/base/rspfFilename.h"
typedef struct  TGlyph_
{
   FT_UInt    glyph_index;         /* glyph index in face      */
   FT_Vector  pos;                 /* position of glyph origin */
   FT_Glyph   image;               /* glyph image              */
   
} TGlyph, *PGlyph;
class rspfFreeTypeFont: public rspfFont
{
public:
   rspfFreeTypeFont(const rspfFilename& fontFile);
   rspfFreeTypeFont(const rspfFreeTypeFont& rhs);
   virtual rspfObject* dup()const
      {
         return new rspfFreeTypeFont(*this);
      }
   virtual const rspf_uint8* getBuf()const
      {
         return theOutputBuffer;
      }
   
   void layoutGlyphs(vector<TGlyph>& glyphs,
                     const rspfString& s );
   
   virtual const rspf_uint8* rasterize();
   void getBoundingBox(rspfIrect& box);
   long getNumberOfFaces()const
      {
         if(theFontFace)
         {
            return theFontFace->num_faces;
         }
         return 0;
      }
   void setCurrentFace(long index);
   /*!
    * Fonts found in files .FNT and .PCF are fixed
    */
   bool isFixed()const
      {
         if(theFontFace)
         {
            return (theFontFace->num_fixed_sizes > 0);
         }
         return false;
      }
   
   virtual void getFixedSizes(vector<rspfIpt>& sizeArray)const;
   /*!
    * Will enable kerning support.  This is enabled
    * by default.  This is for true type fonts and
    * if its available for the current font this
    * will tell us to make sure we use it.  This
    * adjusts spacing between pairs of glyphs.
    */
   virtual void enableKerningSupport()
      {
         theKerningEnabledFlag = true;
         theBoundingRectIsValid = false;
      }
   /*!
    * Will disable kerning support.  
    */   
   virtual void disableKerningSupport()
      {
         theKerningEnabledFlag = false;
         theBoundingRectIsValid = false;
      }
   virtual void setHorizontalVerticalShear(double horizontalShear,
                                           double verticalShear)
      {
         rspfFont::setHorizontalVerticalShear(horizontalShear,
                                               verticalShear);
         theBoundingRectIsValid = false;
      }
   virtual void setScale(double horizontal,
                         double vertical)
      {
         rspfFont::setScale(horizontal,
                             vertical);
         theBoundingRectIsValid = false;
      }
   virtual void setPixelSize(int horizontalPixelSize,
                             int verticalPixelSize)
      {
         rspfFont::setPixelSize(horizontalPixelSize,
                                 verticalPixelSize);
         theBoundingRectIsValid = false;
      }
   virtual void setPixelSize(int pixelSize)
      {
         rspfFont::setPixelSize(pixelSize);
         theBoundingRectIsValid = false;         
      }
   
   virtual void setDeviceDpi(int horizontalDeviceUnits,
                             int verticalDeviceUnits)
      {
         rspfFont::setDeviceDpi(horizontalDeviceUnits,
                                 verticalDeviceUnits);
         theBoundingRectIsValid = false;
      }
   virtual void setString(const rspfString& s)
      {
         rspfFont::setString(s);
         theBoundingRectIsValid = false;
         theNeedToLayoutGlyphsFlag = true;
      }
  const FT_Face getFontFace()const
  {
    return theFontFace;
  }
protected:
   virtual ~rspfFreeTypeFont();
   FT_Face       theFontFace;
   FT_Library    theLibrary;
   FT_Matrix     theMatrix;
   
   rspfFilename theFontFile;
   rspf_uint8*  theOutputBuffer;
   long          theBufferSize;
   bool          theKerningEnabledFlag;
   bool          theNeedToLayoutGlyphsFlag;
   FT_Vector     theStringCenter;
   rspfIpt      theShift;
   /*!
    * Pre-compute all glyphs but don't render it
    */
   vector<TGlyph> theStringLayout;
   
   /*!
    * We will precompute the bounding rect only when we have to.
    * this will avoid re-computing on all rasterization calls.  We
    * will use the dirty flag to tell us if we need to re-compute.
    */
   mutable rspfIrect    thePrecomputedBoundingRect;
   /*!
    * Used to tell us when we need to recompute.
    */
   mutable bool          theBoundingRectIsValid;
   void layoutGlyphs(const rspfString& s);
   
   void setBaseClassInformation();
   void setupForRasterization();
   void allocateBuffer();
   void drawBitmap(FT_Bitmap* bitmap,
                   const rspfIrect& bufRect,
                   const rspfIrect& glyphBox);
   void computeGlyphBox(FT_Glyph,
                        rspfIrect& box)const;
   void deleteGlyphs(vector<TGlyph>& glyphs);
TYPE_DATA
};
#endif
#endif
