#ifndef rspfGdBitmapFont_HEADER
#define rspfGdBitmapFont_HEADER
#include <rspf/font/rspfFont.h>
struct rspfGdFont
{
	/* # of characters in font */
	int nchars;
	/* First character is numbered... (usually 32 = space) */
	int offset;
	/* Character width and height */
	int w;
	int h;
	/* Font data; array of characters, one row after another.
		Easily included in code, also easily loaded from
		data files. */
	char *data;
};
typedef rspfGdFont* rspfGdFontPtr;
/*!
 *  This is a wrapper for the gd library fonts.
 *  Note that this class will currently only support scaling.  It will not support
 *  any other affine operation.
 */
class rspfGdBitmapFont : public rspfFont
{
public:
   
   rspfGdBitmapFont(const rspfString& familyName,
                     const rspfString& styleName,
                     const rspfGdFont* gdFontPtr);
   rspfGdBitmapFont(const rspfGdBitmapFont& rhs);
   virtual ~rspfGdBitmapFont();
   virtual rspfObject* dup()const
      {
         return new rspfGdBitmapFont(*this);
      }
   virtual const rspf_uint8* getBuf()const
      {
         return theOutputBuffer;
      }
    /*!
    * Will return an internal buffer for rasterizing and will
    */
   virtual const rspf_uint8* rasterize();
   virtual void getBoundingBox(rspfIrect& box);
   bool isFixed()const
      {
         return true;
      }
   
   virtual void getFixedSizes(vector<rspfIpt>& sizes)const;
protected:
   const rspfGdFont* theGdFontPtr;
   long         theBufferSize;
   rspf_uint8* theOutputBuffer;
   /*!
    * This will take into consideration the point size along
    * the horizontal and vertical and also the scale
    * values.  So the actual scale will be:
    *
    * Scale*(pointSize/fixedSize);
    */
   void getActualScale(rspfDpt& scales)const;
   void rasterizeNormal();
   void allocateBuffer();
TYPE_DATA
};
#endif
