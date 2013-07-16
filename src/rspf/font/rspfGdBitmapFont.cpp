#include <rspf/font/rspfGdBitmapFont.h>
#include "string.h"
RTTI_DEF1(rspfGdBitmapFont, "rspfGdBitmapFont", rspfFont);
rspfGdBitmapFont::rspfGdBitmapFont(const rspfString& familyName,
                                     const rspfString& styleName,
                                     const rspfGdFont* gdFontPtr)
   :rspfFont(familyName,
              styleName,
              (int)gdFontPtr->w,
              (int)gdFontPtr->h),
    theGdFontPtr(gdFontPtr),
    theBufferSize(0),
    theOutputBuffer(NULL)
{
}
rspfGdBitmapFont::rspfGdBitmapFont(const rspfGdBitmapFont& rhs)
   :rspfFont(rhs),
    theGdFontPtr(rhs.theGdFontPtr),
    theBufferSize(rhs.theBufferSize),
    theOutputBuffer(NULL)    
{
   if(theBufferSize)
   {
      theOutputBuffer = new rspf_uint8[theBufferSize];
      memcpy(theOutputBuffer, rhs.theOutputBuffer, theBufferSize);
   }
}
rspfGdBitmapFont::~rspfGdBitmapFont()
{
   if(theOutputBuffer)
   {
      delete [] theOutputBuffer;
      theOutputBuffer = NULL;
   }
}
void rspfGdBitmapFont::getBoundingBox(rspfIrect& box)
{
   if(theStringToRasterize.size())
   {
      box = rspfIrect(0,0,
                       (int)ceil(theStringToRasterize.size()*theHorizontalPixelSize*theHorizontalScale)-1,
                       (int)ceil(theVerticalPixelSize*theVerticalScale)-1);
   }
   else
   {
      box = rspfIrect(0,
                       0,
                       0,
                       0);
   }
#if 0
   if(theGdFontPtr)
   {
      if((theRotation == 0.0)&&
         (theHorizontalScale == 1.0)&&
         (theVerticalScale   == 1.0))
      {
         box = rspfIrect(0,0, theGdFontPtr->w-1, theGdFontPtr->h-1);
      }
      else
      {
         rspfIrect localBox(0,0, theGdFontPtr->w-1, theGdFontPtr->h-1);
         double ulx, uly;
         double llx, lly;
         double lrx, lry;
         double urx, ury;
         
         double min_x, min_y, max_x, max_y;
         ulx = 0.0;
         uly = 0.0;
         llx = (theAffineTransform[0][0]*localBox.ll().x +
                theAffineTransform[0][1]*localBox.ll().y);
         lly = theAffineTransform[1][0]*localBox.ll().x +
               theAffineTransform[1][1]*localBox.ll().y;
         
         lrx = (theAffineTransform[0][0]*localBox.lr().x +
                theAffineTransform[0][1]*localBox.lr().y);
         lry = theAffineTransform[1][0]*localBox.lr().x +
               theAffineTransform[1][1]*localBox.lr().y;
         urx = (theAffineTransform[0][0]*localBox.ur().x +
                theAffineTransform[0][1]*localBox.ur().y);
         ury = theAffineTransform[1][0]*localBox.ur().x +
               theAffineTransform[1][1]*localBox.ur().y;
         
         min_x = std::min(ulx, std::min(llx, (std::min(lrx, urx))));
         max_x = std::max(ulx, std::max(llx, (std::max(lrx, urx))));
         min_y = std::min(uly, std::min(lly, (std::min(lry, ury))));
         max_y = std::max(uly, std::max(lly, (std::max(lry, ury))));
         
         box = rspfIrect((int)floor(min_x), (int)floor(min_y),
                          (int)ceil(max_x), (int)ceil(max_y));
      }
   }
#endif
}
const rspf_uint8* rspfGdBitmapFont::rasterize()
{
   allocateBuffer();
   if(theOutputBuffer)
   {
      memset(theOutputBuffer, 0, theBufferSize);
      rasterizeNormal();
   }
   return theOutputBuffer;
}
void rspfGdBitmapFont::rasterizeNormal()
{
   rspfIrect outBox;
   rspfIrect inBox(0,0,
                    (rspf_int32)theStringToRasterize.length()*theGdFontPtr->w-1,
                    theGdFontPtr->h-1);
   
   getBoundingBox(outBox);
   
   long charOffset = 0;
   long bufOffset  = 0;
   long inW    = inBox.width();
   long inH    = inBox.height();
   long outW = outBox.width();
   long outH = outBox.height();
   rspf_uint8* inBuf = new rspf_uint8[inBox.width()*inBox.height()];
      
   for(std::string::size_type character = 0;
       character < theStringToRasterize.size();
       ++character)
   {
      charOffset = theStringToRasterize[character];
      
      if(charOffset < 0)
      {
         charOffset += 256;
      }
      bufOffset = (long)character*theGdFontPtr->w;
      
      charOffset  = charOffset*theGdFontPtr->w*theGdFontPtr->h;
      
      for(long row = 0; row < (theGdFontPtr->h); ++row)
      {         
         for(long col = 0; col < theGdFontPtr->w; ++col)
         {
            if(theGdFontPtr->data[charOffset])
            {
               inBuf[bufOffset+col] = 255;
            }
            ++charOffset;
         }
         bufOffset += inW;
      }
   }
   if((inW == outW)&&(inH==outH))
   {
      memcpy(theOutputBuffer, inBuf, inW*inH);
   }
   else
   {
      long *xLut = new long[outW];
      long *yLut = new long[outH];
      long i = 0;
      for(i = 0; i < outW; ++i)
      {
         xLut[i] = (int)(i*((double)inW/(double)outW));
      }
      for(i = 0; i < outH; ++i)
      {
         yLut[i] = (int)(i*((double)inH/(double)outH));
      }
      
      rspf_uint8* outBuf = theOutputBuffer;
      for(long row= 0; row < outH; ++row)
      {
         long inYOffset = yLut[row]*inW;
         for(long col=0; col < outW; ++col)
         {
            *outBuf = inBuf[inYOffset + xLut[col]];
            ++outBuf;
         }
      }
      delete [] xLut;
      delete [] yLut;
   }
   delete [] inBuf;
}
void rspfGdBitmapFont::allocateBuffer()
{   
   rspfIrect box;
   
   getBoundingBox(box);
   long bufferSize = box.width()*box.height();
   if(bufferSize != theBufferSize)
   {
      theBufferSize = bufferSize;
      
      if(theOutputBuffer)
      {
         delete theOutputBuffer;
         theOutputBuffer = NULL;
      }
      if(!theOutputBuffer&&theBufferSize)
      {
         theOutputBuffer = new rspf_uint8[theBufferSize];
            
      }
   }   
}
void rspfGdBitmapFont::getFixedSizes(vector<rspfIpt>& sizes)const
{
   sizes.push_back(rspfIpt(theHorizontalPixelSize,
                            theVerticalPixelSize));
}
