//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfTilePatch.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <vector>
using namespace std;

#include <rspf/imaging/rspfTilePatch.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfErrorCodes.h>
rspfDiscreteNearestNeighbor rspfTilePatch::nearestNeighborKernel;

rspfTilePatch::rspfTilePatch(rspfImageSource* input)
{
   thePatchData = rspfImageDataFactory::instance()->create(NULL, input);
   thePatchData->initialize();
}

rspfTilePatch::rspfTilePatch(rspfScalarType scalarType,
                               long numberOfComponents,
                               long width,
                               long height)
   :thePatchData(NULL)
{
   thePatchData = rspfImageDataFactory::instance()->create(NULL,
                                                            scalarType,
                                                            numberOfComponents,
                                                            width,
                                                            height);

   thePatchData->initialize();
}

rspfTilePatch::~rspfTilePatch()
{
}

rspfIrect rspfTilePatch::getRect()const
{
   return thePatchData->getImageRectangle();
}

rspfDrect rspfTilePatch::findBoundingRect(const rspfDpt &p1,
                                            const rspfDpt &p2,
                                            const rspfDpt &p3,
                                            const rspfDpt &p4)
{
   vector<rspfDpt> points(4);
   
   points[0] = p1;
   points[1] = p2;
   points[2] = p3;
   points[3] = p4;

   return rspfDrect(points);
}

void rspfTilePatch::fillPatch(rspfImageSource* tileSource,
                               long resLevel)
{
   // long w = tileSource->getTileWidth();
   // long h = tileSource->getTileHeight();
   
   if(thePatchData.valid())
   {
      rspfIrect rect   = thePatchData->getImageRectangle();
      thePatchData->makeBlank();
      // rspfIpt origin(rect.ul());
      rspfRefPtr<rspfImageData> aTile = tileSource->getTile(rect, resLevel);
      thePatchData->loadTile(aTile.get());
   }
}

void rspfTilePatch::setData(rspfRefPtr<rspfImageData>& patchData)
{
   thePatchData = patchData;
}

void rspfTilePatch::convolve(rspfDiscreteConvolutionKernel* kernel)
{
   if(!kernel) return;

   long w = thePatchData->getWidth();
   long h = thePatchData->getHeight();

   long kW = kernel->getWidth();
   long kH = kernel->getHeight();
   rspfIpt origin = thePatchData->getOrigin();
   
   if( (w < kW) || (h < kH))
   {
      cerr << " Error rspfTilePatch::convolve(kernel): patch is smaller than kernel size" << endl;
      return;
   }
   rspfRefPtr<rspfImageData> kernelBuffer =(rspfImageData*) thePatchData->dup();

   // now let's make the buffer just fit the convolution filter
   //
   kernelBuffer->setWidth(w - kW);
   kernelBuffer->setHeight(h - kH);
   kernelBuffer->setOrigin(rspfDpt(origin.x + kW/2.0,
                                    origin.y + kH/2.0));
   kernelBuffer->initialize();

   fillTile(kernelBuffer, kernel);

   kernelBuffer->validate();
   // now copy the data back to the buffer
   //
   thePatchData->loadTile(kernelBuffer.get());

   // now make sure we validate the buffer.
   thePatchData->validate();
}

void rspfTilePatch::fillTile(rspfRefPtr<rspfImageData>& aTile)const
{
   if(aTile.valid() &&
      (aTile->getNumberOfBands()==thePatchData->getNumberOfBands())&&
      (aTile->getBuf())&&
      (thePatchData->getBuf())&&
      (thePatchData->getScalarType()==aTile->getScalarType()))
   {
      aTile->loadTile(thePatchData.get());
   }
}

void rspfTilePatch::fillTile(rspfRefPtr<rspfImageData>& result,
                              rspfDiscreteConvolutionKernel* kernel)const
{
   if(!kernel)
   {
      cerr << "Error: Kernel is NULL in  rspfTilePatch::fillTile(tile, kernel)!!" << endl;
   }
   if(thePatchData->getScalarType() != result->getScalarType())
   {
      //ERROR
      return;
   }
   else
   {
      switch(thePatchData->getScalarType())
      {
      case RSPF_UCHAR:
      {
         fillTileTemplate(static_cast<rspf_uint8>(0),
                          result,
                          kernel);
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         fillTileTemplate(static_cast<rspf_uint16>(0),
                          result,
                          kernel);
         break;
      }
      case RSPF_SSHORT16:
      {
         fillTileTemplate(static_cast<rspf_sint16>(0),
                          result,
                          kernel);
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         fillTileTemplate(static_cast<float>(0),
                          result,
                          kernel);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         fillTileTemplate(static_cast<double>(0),
                          result,
                          kernel);
         break;
      }
      default:
	{
	  rspfSetError("rspfTilePatch",
			rspfErrorCodes::RSPF_ERROR,
			"unhandled scalar type %d",
			(int)thePatchData->getScalarType());
	  break;
	}
      }
   }
}



void rspfTilePatch::fillTile(rspfRefPtr<rspfImageData>& result,
                              const rspfDpt   &ul,
                              const rspfDpt   &ur,
                              const rspfDpt   &deltaUl,
                              const rspfDpt   &deltaUr,
                              const rspfDpt   &length)const
{   
   result->setNullPix(thePatchData->getNullPix(), thePatchData->getNumberOfBands());
   result->setMinPix(thePatchData->getMinPix(), thePatchData->getNumberOfBands());
   result->setMaxPix(thePatchData->getMaxPix(), thePatchData->getNumberOfBands());
   result->makeBlank();
   if(thePatchData->getScalarType() != result->getScalarType())
   {
      //ERROR
      return;
   }
   else
   {
      switch(thePatchData->getScalarType())
      {
      case RSPF_UCHAR:
      {
         fillTileTemplate(static_cast<rspf_uint8>(0),
                          result,
                          ul,
                          ur,
                          deltaUl,
                          deltaUr,
                          length);
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         fillTileTemplate(static_cast<rspf_uint16>(0),
                          result,
                          ul,
                          ur,
                          deltaUl,
                          deltaUr,
                          length);
         break;
      }
      case RSPF_SSHORT16:
      {
         fillTileTemplate(static_cast<rspf_sint16>(0),
                          result,
                          ul,
                          ur,
                          deltaUl,
                          deltaUr,
                          length);
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         fillTileTemplate(static_cast<float>(0),
                          result,
                          ul,
                          ur,
                          deltaUl,
                          deltaUr,
                          length);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         fillTileTemplate(static_cast<double>(0),
                          result,
                          ul,
                          ur,
                          deltaUl,
                          deltaUr,
                          length);
         break;
      }
      default:
	{
	  rspfSetError("rspfTilePatch",
			rspfErrorCodes::RSPF_ERROR,
			"unhandled scalar type %d",
			(int)thePatchData->getScalarType());
	  break;
	}
      }
   }

}


template<class T>
void rspfTilePatch::fillTileTemplate(T /* dummyVariable */,
                                      rspfRefPtr<rspfImageData>& result,
                                      rspfDiscreteConvolutionKernel* kernel)const
{
   // get the origin.  The convolution could be
   // an offset into the patch.
   //
   rspfIpt startOrigin   = result->getOrigin();

   // Make sure that the patch is not empty or NULL
   //
   rspfDataObjectStatus status = thePatchData->getDataObjectStatus();
   if((status==RSPF_EMPTY)||
      (status == RSPF_NULL))
   {
      return;
   }
   rspfDpt startDelta(fabs((double)startOrigin.x - thePatchData->getOrigin().x),
                       fabs((double)startOrigin.y - thePatchData->getOrigin().y));
   
   // let's setup some variables that we will need to do the
   // convolution algorithm.
   //
   rspfIrect patchRect   = thePatchData->getImageRectangle();
   long tileHeight        = result->getHeight();
   long tileWidth         = result->getWidth();
   long outputBands       = result->getNumberOfBands();
   long convolutionWidth  = kernel->getWidth();
   long convolutionHeight = kernel->getHeight();
   long convolutionOffsetX= convolutionWidth/2;
   long convolutionOffsetY= convolutionHeight/2;
   long patchWidth        = patchRect.width();
   long patchCenterOffset = (long)(patchWidth*startDelta.y + startDelta.x);
   long patchConvolutionOffsetDelta = patchWidth*convolutionOffsetY + convolutionOffsetX;
   long patchLineStartOffset        = patchCenterOffset  - patchConvolutionOffsetDelta;
   long outputOffset                = 0;
   double min = 1.0;
   double max = 255.0;
   if(status == RSPF_PARTIAL) // must check for NULLS
   {
      for(long y = 0; y <tileHeight; y++)
      {
         patchCenterOffset = patchLineStartOffset;
         
         for(long x =0; x < tileWidth; x++)
         {  
            if(!thePatchData->isNull(patchCenterOffset))
            {
               double convolveResult = 0;
               for(long b = 0; b < outputBands; ++b)
               {                  
                  min=result->getMinPix(b);
                  max=result->getMaxPix(b);
                  T* buf    = (T*)(thePatchData->getBuf(b)) + patchCenterOffset;
                  T* outBuf = (T*)(result->getBuf(b));

                  kernel->convolveSubImage(buf,
                                           patchWidth,
                                           convolveResult);
                  convolveResult = convolveResult < min?min:convolveResult;
                  convolveResult = convolveResult > max?max:convolveResult;
                  outBuf[outputOffset] = (rspf_uint8)convolveResult;
               }
            }
            else
            {
               result->setNull(outputOffset);
            }
            ++outputOffset;
            ++patchCenterOffset;
         }
         patchLineStartOffset += patchWidth;
      }
    }
    else
    {
       for(long b = 0; b < outputBands; ++b)
       {                  
          double convolveResult = 0;
          const T* buf    = (const T*)thePatchData->getBuf(b);
          T* outBuf = (T*)(result->getBuf(b));
          outputOffset = 0;
          patchCenterOffset = (long)(patchWidth*startDelta.y + startDelta.x);
          patchLineStartOffset  = patchCenterOffset - patchConvolutionOffsetDelta;
          min=result->getMinPix(b);
          max=result->getMaxPix(b);
          
          for(long y = 0; y <tileHeight; y++)
          {
             patchCenterOffset = patchLineStartOffset;
             
             for(long x =0; x < tileWidth; x++)
             {
                kernel->convolveSubImage(&buf[patchCenterOffset],
                                         patchWidth,
                                         convolveResult);
                convolveResult = convolveResult < min? min:convolveResult;
                convolveResult = convolveResult > max?max:convolveResult;
                outBuf[outputOffset] = (T)convolveResult;
                ++outputOffset;
                ++patchCenterOffset;
             }
             patchLineStartOffset += patchWidth;
          }
       }
    }
}


template <class T>
void rspfTilePatch::fillTileTemplate(T /* dummyVariable */,
                              rspfRefPtr<rspfImageData>& result,
                              const rspfDpt   &ul,
                              const rspfDpt   &ur,
                              const rspfDpt   &deltaUl,
                              const rspfDpt   &deltaUr,
                              const rspfDpt   &length)const
{
   double stepSizeWidth  = 1.0/length.x;
   double stepSizeHeight = 1.0/length.y;
   long patchWidth = thePatchData->getWidth();
   long patchHeight = thePatchData->getHeight();

   
   rspfIrect rect = thePatchData->getImageRectangle();
   
   rspfDpt startSave(ul.x - rect.ul().x,
                      ul.y - rect.ul().y );
   rspfDpt endSave(ur.x - rect.ul().x,
                    ur.y - rect.ul().y);
   for(rspf_uint32 band = 0; band < thePatchData->getNumberOfBands(); ++band)
   {
      T *resultBuf = (T*)result->getBuf(band);
      const T *sourceBuf = (T*)thePatchData->getBuf(band);
      rspfDpt start = startSave;
      rspfDpt end   = endSave;
      T nullPix = static_cast<T>(result->getNullPix(band));
      for(long y = 0; y < length.y; y++)
      {
         double deltaX = (end.x - start.x)*stepSizeWidth;
         double deltaY = (end.y - start.y)*stepSizeHeight;
         rspfDpt pointXY = start;
         for(long x = 0; x < length.x; x++)
         {
            int xPixel = pointXY.x<0?(int)floor(pointXY.x):(int)pointXY.x;
            int yPixel = pointXY.y<0?(int)floor(pointXY.y):(int)pointXY.y;
            if( (xPixel >=0) && (xPixel < patchWidth) &&
                (yPixel >=0) && (yPixel < patchHeight))
            {              
               *resultBuf = sourceBuf[yPixel*patchWidth + xPixel];
            }
            else
            {
               *resultBuf = nullPix;
            }
            
            resultBuf++;
            pointXY.y += deltaY;
            pointXY.x += deltaX;
         }
         
         start.x += deltaUl.x;
         start.y += deltaUl.y;
         end.x   += deltaUr.x;
         end.y   += deltaUr.y;
      }
   }
}


void rspfTilePatch::setRect(const rspfDpt &p1,
                             const rspfDpt &p2,
                             const rspfDpt &p3,
                             const rspfDpt &p4,
                             const rspfDpt &tile_size,
                             const rspfDpt &padding)
{
   setRect(findBoundingRect(p1, p2, p3, p4),
           tile_size,
           padding);
}

void rspfTilePatch::setRect(const rspfDpt &center,
                             const rspfDpt &rect_size,
                             const rspfDpt &tile_size,
                             const rspfDpt &padding)
{
   rspfDpt ul (center - rect_size/2.0);
   rspfDpt lr (center + rect_size/2.0);
   rspfDrect rect (ul, lr);
   setRect(rect, tile_size, padding);
}

void rspfTilePatch::setRect(const rspfDrect& aRect,
                             const rspfDpt& /* tile_size*/,
                             const rspfDpt& padding)
{
   rspfDpt   ul(aRect.ul().x - padding.x,
                 aRect.ul().y - padding.y);
   rspfDpt   lr(aRect.lr().x + padding.x,
                 aRect.lr().y + padding.y);

   rspfDrect rect(ul, lr);
   rect.stretchOut();
   
//   rect   = alignRectToBoundary(rect, tile_size);

   if(thePatchData.valid())
   {
      if(  ((long)thePatchData->getWidth()  != (long)rect.width()) ||
           ((long)thePatchData->getHeight() != (long)rect.height()))
      {
         thePatchData->setWidth((long)rect.width());
         thePatchData->setHeight((long)rect.height());
      }
      thePatchData->setOrigin(rect.ul());
      thePatchData->initialize();
   }
}

