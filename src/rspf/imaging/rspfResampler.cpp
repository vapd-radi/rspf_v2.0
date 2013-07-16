//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//
// Description:
//
// Contains class definition for rspfResampler.
// 
//*******************************************************************
//  $Id: rspfResampler.cpp 20204 2011-11-04 15:12:28Z dburken $


#include <iostream>
using namespace std;
#include <rspf/imaging/rspfResampler.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfFilter.h>
#include <rspf/imaging/rspfTilePatch.h>
#include <rspf/imaging/rspfImageHandler.h>

RTTI_DEF1(rspfResampler, "rspfResampler", rspfConnectableObject)


static const char* RESAMPLER_CONVOLUTION_TYPE_KW = "convolution_type";
static const char* RESAMPLER_SCALE_X_KW          = "scale_x";
static const char* RESAMPLER_SCALE_Y_KW          = "scale_y";
static const char* RESAMPLER_CUBIC_PARAMETER_KW  = "cubic_parameter";

rspfResampler::rspfResampler()
   :rspfConnectableObject(NULL,0,0,true, false),
    theOutputToInputRatio(1,1),
    theResamplerType(rspfResampler_NONE),
    theTableWidthX(0),
    theTableWidthY(0),
    theTableHeight(0),
    theKernelWidth(0),
    theKernelHeight(0),
    theWeightTableX(NULL),
    theWeightTableY(NULL),
    theCubicAdjustableParameter(-.5)
{
}

rspfResampler::~rspfResampler()
{
   deleteWeightTable();
}

void rspfResampler::resample(rspfImageData* input,
                              rspfImageData* output)
{
   if(!input||
      !output ||
      !input->getBuf() ||
      !output->getBuf())
   {
      return;
   }

   if(theWeightTableX&&theWeightTableY)
   {
      rspfScalarType scalarType = input->getScalarType();
      switch(scalarType)
      {
         case RSPF_UINT8:
         {
            resampleTile(rspf_uint8(0), // dummy template variable
                         input,
                         output);
            break;
         }
         case RSPF_UINT16:
         case RSPF_USHORT11:
         {
            resampleTile(rspf_uint16(0), // dummy template variable
                         input,
                         output);
            break;
         }
         case RSPF_SSHORT16:
         {
            resampleTile(rspf_sint16(0), // dummy template variable
                         input,
                         output);
            break;
         }      
         case RSPF_UINT32:
         {
            resampleTile(rspf_uint32(0), // dummy template variable
                         input,
                         output);
            break;
         }      
         case RSPF_FLOAT32:
         case RSPF_NORMALIZED_FLOAT:
         {
            resampleTile(rspf_float32(0.0), // dummy template variable
                         input,
                         output);
            break;
         }
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_DOUBLE:
         {
            resampleTile(rspf_float64(0.0), // dummy template variable
                         input,
                         output);
            break;
         }
         default:
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfResampler::resample error: unknown scalar type:  "
               << scalarType
               << endl;
            
         }
      }
   }      
   // convolve the input and copy to output.
}

void rspfResampler::resample(rspfImageData* input,
                              rspfImageData* output,
                              const rspfDpt& ul,
                              const rspfDpt& ur,
                              const rspfDpt& deltaUl,
                              const rspfDpt& deltaUr,
                              const rspfDpt& length)
{
   resample(input,
            output,
            output->getImageRectangle(),
            ul,
            ur,
            deltaUl,
            deltaUr,
            length);
}

void rspfResampler::resample(rspfImageData* input,
                              rspfImageData* output,
                              const rspfIrect& outputSubRect,
                              const rspfDpt& ul,
                              const rspfDpt& ur,
                              const rspfDpt& deltaUl,
                              const rspfDpt& deltaUr,
                              const rspfDpt& length)
{
   if(!input||
      !output ||
      !input->getBuf() ||
      !output->getBuf())
   {
      return;
   }
   
   if(theWeightTableX&&theWeightTableY)
   {
      rspfScalarType scalarType = input->getScalarType();
      switch(scalarType)
      {
         case RSPF_UINT8:
         {
            resampleTile(rspf_uint8(0), // dummy template variable
                         input,
                         output,
                         outputSubRect,
                         ul,
                         ur,
                         deltaUl,
                         deltaUr,
                         length);
            break;
         }
         case RSPF_UINT16:
         case RSPF_USHORT11:
         {
            resampleTile(rspf_uint16(0), // dummy template variable
                         input,
                         output,
                         outputSubRect,
                         ul,
                         ur,
                         deltaUl,
                         deltaUr,
                         length);
            break;
         }
         case RSPF_SINT16:
         {
            resampleTile(rspf_sint16(0), // dummy template variable
                         input,
                         output,
                         outputSubRect,
                         ul,
                         ur,
                         deltaUl,
                         deltaUr,
                         length);
            break;
         }      
         case RSPF_UINT32:
         {
            resampleTile(rspf_uint32(0), // dummy template variable
                         input,
                         output,
                         outputSubRect,
                         ul,
                         ur,
                         deltaUl,
                         deltaUr,
                         length);
            break;
         }      
         case RSPF_FLOAT32:
         case RSPF_NORMALIZED_FLOAT:
         {
            resampleTile(rspf_float32(0.0), // dummy template variable
                         input,
                         output,
                         outputSubRect,
                         ul,
                         ur,
                         deltaUl,
                         deltaUr,
                         length);
            break;
         }
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_DOUBLE:
         {
            resampleTile(rspf_float64(0.0), // dummy template variable
                         input,
                         output,
                         outputSubRect,
                         ul,
                         ur,
                         deltaUl,
                         deltaUr,
                         length);
            break;
         }
         default:
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfResampler::resample error: unknown scalar type:  "
               << scalarType
               << endl;
         }
      }
   }
   
}

void rspfResampler::resampleNearestNeighbor(rspfImageData* input,
                                             rspfImageData* output,
                                             const rspfIrect& outputSubRect,
                                             const rspfDpt& ul,
                                             const rspfDpt& ur,
                                             const rspfDpt& deltaUl,
                                             const rspfDpt& deltaUr,
                                             const rspfDpt& length)
{
   if(!input||
      !output ||
      !input->getBuf() ||
      !output->getBuf())
   {
      return;
   }
   
   rspfScalarType scalarType = input->getScalarType();
   switch(scalarType)
   {
      case RSPF_UINT8:
      {
         resampleTileNearestNeighbor(rspf_uint8(0),// dummy template variable
                                     input,
                                     output,
                                     outputSubRect,
                                     ul,
                                     ur,
                                     deltaUl,
                                     deltaUr,
                                     length);
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         resampleTileNearestNeighbor(rspf_uint16(0),// dummy template variable
                                     input,
                                     output,
                                     outputSubRect,
                                     ul,
                                     ur,
                                     deltaUl,
                                     deltaUr,
                                     length);
         break;
      }
      case RSPF_SINT16:
      {
         resampleTileNearestNeighbor(rspf_sint16(0),// dummy template variable
                                     input,
                                     output,
                                     outputSubRect,
                                     ul,
                                     ur,
                                     deltaUl,
                                     deltaUr,
                                     length);
         break;
      }      
      case RSPF_UINT32:
      {
         resampleTileNearestNeighbor(rspf_uint32(0),// dummy template variable
                                     input,
                                     output,
                                     outputSubRect,
                                     ul,
                                     ur,
                                     deltaUl,
                                     deltaUr,
                                     length);
         break;
      }      
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         resampleTileNearestNeighbor(rspf_float32(0.0),
                                     input,
                                     output,
                                     outputSubRect,
                                     ul,
                                     ur,
                                     deltaUl,
                                     deltaUr,
                                     length);
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         resampleTileNearestNeighbor(rspf_float64(0.0),
                                     input,
                                     output,
                                     outputSubRect,
                                     ul,
                                     ur,
                                     deltaUl,
                                     deltaUr,
                                     length);
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfResampler::resample error: unknown scalar type:  "
            << scalarType
            << endl;
      }
   }   
}

void rspfResampler::resampleNearestNeighbor(rspfImageData* input,
                                             rspfImageData* output,
                                             const rspfDpt& ul,
                                             const rspfDpt& ur,
                                             const rspfDpt& deltaUl,
                                             const rspfDpt& deltaUr,
                                             const rspfDpt& length)
{
   resampleNearestNeighbor(input,
                           output,
                           output->getImageRectangle(),
                           ul,
                           ur,
                           deltaUl,
                           deltaUr,
                           length);
}


template <class T>
void rspfResampler::resampleTile(T,// not used
                                  rspfImageData* input,
                                  rspfImageData* output)
{
   switch(input->getDataObjectStatus())
   {
      case RSPF_FULL:
      {
         resampleFullTile((T)0, input, output);
         break;
      }
      case RSPF_PARTIAL:
      {      
         resamplePartialTile((T)0, input, output);
         break;
      }
      default:
      {
         break;
      }
   }
}

template <class T>
void rspfResampler::resamplePartialTile(T,// not used
                                         rspfImageData* input,
                                         rspfImageData* output)
{   
   rspf_int32 maxInputSize  = std::max(input->getWidth(),  input->getHeight());
   rspf_int32 maxOutputSize = std::max(output->getWidth(), output->getHeight());
   rspf_int32 k;
   double *h  = new double[maxInputSize];
   int    *Lx = new int[maxOutputSize];
   int    *Ly = new int[maxOutputSize];
   rspf_int32 out_height = output->getHeight();
   rspf_int32 out_width  = output->getWidth();
   rspf_int32 in_width   = input->getWidth();
   rspf_int32 in_height  = input->getHeight();
   rspf_int32 j;
   rspf_int32 l;

   rspfIpt origin=output->getOrigin();
   rspfIpt scaledOriginShift(input->getOrigin()*-1);

   rspf_int32 evenKernelW = (rspf_int32)(!(theKernelWidth%2));
   rspf_int32 evenKernelH = (rspf_int32)(!(theKernelHeight%2));
   rspf_int32 kernelHorizontalShift = (rspf_int32)(-theKernelWidth/2.0+evenKernelW);
   rspf_int32 kernelVerticalShift   = (rspf_int32)(-theKernelHeight/2.0+evenKernelH);
   
   for (k = 0; k < maxOutputSize; k++)
   {
      Lx[k] = (rspf_int32)(scaledOriginShift.x+((k+origin.x)/
                                           theOutputToInputRatio.x));
   }
   for (k = 0; k < maxOutputSize; k++)
   {
      Ly[k] = (rspf_int32)(scaledOriginShift.y+((k+origin.y)/
                                           theOutputToInputRatio.y));
   }
   for(rspf_int32 band = 0; band < (rspf_int32)input->getNumberOfBands();++band)
   {
      const T* inputBuf  = static_cast<T*>(input->getBuf(band));
      T*       outputBuf = static_cast<T*>(output->getBuf(band));
      T        minPix    = static_cast<T>(input->getMinPix(band));
      T        maxPix    = static_cast<T>(input->getMaxPix(band));
      T        np        = static_cast<T>(input->getNullPix(band));
      
      for (k = 0; k < out_height; k++)
      {
         rspf_int32 indexMod = (rspf_int32)fmod((k+origin.y), theOutputToInputRatio.y);
         if(indexMod >= theTableWidthY) indexMod = theTableWidthY - 1;
         if(indexMod <0) indexMod = 0;
         for (j = 0; j < in_width; j++)
         {
            h[j] = 0.0;
            rspf_int32 count = 0;
            double lastValue = rspf::nan();
            for (l = 0; l < theKernelHeight; l++)
            {
               rspf_int32 index = Ly[k] + l + kernelVerticalShift;
               if ((index >= 0) && (index < in_height))
               {
                  rspf_int32 offset = index*in_width +j;
                  if(!input->isNull(offset))
                  {
                     lastValue = (double)inputBuf[offset];
                     h[j] += lastValue *
                             theWeightTableY[theKernelHeight - l-1][indexMod];
                     ++count;
                  }
                  // I think instead of skipping to next value I'll just assume 0
                  
                  // this was code to skip to next non-null value and use it
                  // 
//                   else
//                   {
//                      for(rspf_int32 templ = l; templ < theKernelHeight; ++templ)
//                      {
//                         index = Ly[k] + templ + kernelVerticalShift;
//                         offset = index*in_width +j;
//                         if(!input->isNull(offset))
//                         {
//                            lastValue = (double)inputBuf[offset];
//                            break;
//                         }
//                      }
//                      if(rspf::isnan(lastValue) == false)
//                      {
//                         h[j] += lastValue *
//                                 theWeightTableY[theKernelHeight - l-1][indexMod];
                        
//                         ++count;
//                      }
//                      else
//                      {
//                         break;
//                      }
//                   }
               }
            }
            if(!count)
            {
               h[j] = rspf::nan();
            }
         }
         for (rspf_int32 m = 0; m < out_width; m++)
         {
            double x = 0.0;
            rspf_int32 indexMod = (rspf_int32)fmod((m+origin.x), theOutputToInputRatio.x);
            if(indexMod >= theTableWidthX) indexMod = theTableWidthX-1;
            if(indexMod <0) indexMod = 0;
            
            if(input->isNull(rspfIpt(Lx[m]-scaledOriginShift.x, Ly[k]-scaledOriginShift.y)))
            {
               outputBuf[k*out_width +m] = np;
            }
            else
            {
               for (l = 0; l < theKernelWidth; l++)
               {
                  rspf_int32 index = Lx[m] + l + kernelHorizontalShift;
                  if ((index >= 0) && (index < in_width))
                  {
                     if(rspf::isnan(h[index]) == false)
                     {
                        x += h[index] * theWeightTableX[theKernelWidth - l-1][indexMod];
                     }

                     // I am commenting out the code that searches for the next
                     // non null value with the kernel range.  This will have the
                     // same effect as multiplying by 0.0 for the NULL value.
                     //
//                      else
//                      {
//                         for(rspf_int32 templ = l; templ < l; ++templ)
//                         {
//                            rspf_int32 index = Lx[m] + templ + kernelHorizontalShift;
//                            if(rspf::isnan(h[index]) == false))
//                            {
//                               lastValue = h[index];
//                               break;
//                            }
//                         }
//                         if(rspf::isnan(lastValue) == false)
//                         {
//                            x += lastValue * theWeightTableX[theKernelWidth - l-1][indexMod];
//                         }
//                      }
                  }
               }
               if (x < minPix)
               {
                  outputBuf[k*out_width +m] = static_cast<T>(minPix);
               }
               else if (x > maxPix)
               {
                  outputBuf[k*out_width +m] = static_cast<T>(maxPix);
               }
               else
               {
                  outputBuf[k*out_width +m] = static_cast<T>(x);
               }
            }
         }
      }
   }
   
   delete [] h;
   delete [] Lx;   
   delete [] Ly;
}

template <class T>
void rspfResampler::resampleFullTile(T,// not used
                                      rspfImageData* input,
                                      rspfImageData* output)
{
   rspf_int32 maxInputSize  = std::max(input->getWidth(),  input->getHeight());
   rspf_int32 maxOutputSize = std::max(output->getWidth(), output->getHeight());
   rspf_int32 k;
   double *h     = new double[maxInputSize];
   rspf_int32 *Lx        = new rspf_int32[maxOutputSize];
   rspf_int32 *Ly        = new rspf_int32[maxOutputSize];
   rspf_int32 out_height = output->getHeight();
   rspf_int32 out_width  = output->getWidth();
   rspf_int32 in_width   = input->getWidth();
   rspf_int32 in_height  = input->getHeight();
   rspf_int32 j;
   rspf_int32 l;
   
   rspfIpt origin=output->getOrigin();
   rspfIpt scaledOriginShift(input->getOrigin()*-1);

   rspf_int32 evenKernelW = (rspf_int32)(!(theKernelWidth%2));
   rspf_int32 evenKernelH = (rspf_int32)(!(theKernelHeight%2));
   rspf_int32 kernelHorizontalShift = (rspf_int32)(-theKernelWidth/2.0+evenKernelW);
   rspf_int32 kernelVerticalShift   = (rspf_int32)(-theKernelHeight/2.0+evenKernelH);
   
   for (k = 0; k < maxOutputSize; k++)
   {
      Lx[k] = (rspf_int32)(scaledOriginShift.x+((k+origin.x)/
                                           theOutputToInputRatio.x));
   }
   for (k = 0; k < maxOutputSize; k++)
   {
      Ly[k] = (rspf_int32)(scaledOriginShift.y+((k+origin.y)/
                                           theOutputToInputRatio.y));
   }
   for(rspf_int32 band = 0; band < (rspf_int32)input->getNumberOfBands();++band)
   {
      const T* inputBuf  = (const T*)(input->getBuf(band));
      T*       outputBuf = (T*)(output->getBuf(band));
      double  minPix    = static_cast<T>(input->getMinPix()[band]);
      double  maxPix    = static_cast<T>(input->getMaxPix()[band]);
      
      for (k = 0; k < out_height; k++)
      {
         int indexMod = (int)fmod((k+origin.y), theOutputToInputRatio.y);
         if(indexMod >= theTableWidthY) indexMod = theTableWidthY - 1;
         if(indexMod <0) indexMod = 0;
         for (j = 0; j < in_width; j++)
         {
            h[j] = 0.0;
            for (l = 0; l < theKernelHeight; l++)
            {
               rspf_int32 index = Ly[k] + l + kernelVerticalShift;
               if ((index >= 0) && (index < in_height))
               {
                  h[j] += ((double)inputBuf[index*in_width +j]) *
                          theWeightTableY[theKernelHeight - l-1][indexMod];
               }
            }
         }
         for (rspf_int32 m = 0; m < out_width; m++)
         {
            double x = 0.0;
            int indexMod = (int)fmod((m+origin.x), theOutputToInputRatio.x);
            if(indexMod >= theTableWidthX) indexMod = theTableWidthX-1;
            if(indexMod <0) indexMod = 0;
            
            for (l = 0; l < theKernelWidth; l++)
            {
               rspf_int32 index = Lx[m] + l + kernelHorizontalShift;
               if ((index >= 0) && (index < in_width))
               {
         
                  x += h[index] * theWeightTableX[theKernelWidth - l-1][indexMod];
               }
            }
            if (x < minPix)
            {
               outputBuf[k*out_width +m] = static_cast<T>(minPix);
            }
            else if (x > maxPix)
            {
               outputBuf[k*out_width +m] = static_cast<T>(maxPix);
            }
            else
            {
               outputBuf[k*out_width +m] = static_cast<T>(x);
            }
         }
      }
   }
   
   delete [] h;
   delete [] Lx;   
   delete [] Ly;
}



template <class T>
void rspfResampler::resampleTile(T,// not used
                                  rspfImageData* input,
                                  rspfImageData* output,
                                  const rspfIrect& outputSubRect,
                                  const rspfDpt& inputUl,
                                  const rspfDpt& inputUr,
                                  const rspfDpt& deltaUl,
                                  const rspfDpt& deltaUr,
                                  const rspfDpt& outLength)
{
   rspfRefPtr<rspfImageData> dupIn = input;
   rspfDpt origin = input->getOrigin();
   rspfDpt newInputUl = inputUl;
   rspfDpt newInputUr = inputUr;
   rspfDpt newDeltaUl = deltaUl;
   rspfDpt newDeltaUr = deltaUr;

   if(theResamplerType != rspfResampler_NEAREST_NEIGHBOR)
   {
      newInputUl = rspfDpt(inputUl.x * theOutputToInputRatio.x,
                            inputUl.y * theOutputToInputRatio.y);
      newInputUr = rspfDpt(inputUr.x * theOutputToInputRatio.x,
                            inputUr.y * theOutputToInputRatio.y);
      newDeltaUl = rspfDpt(deltaUl.x * theOutputToInputRatio.x,
                            deltaUl.y * theOutputToInputRatio.y);
      newDeltaUr = rspfDpt(deltaUr.x * theOutputToInputRatio.x,
                            deltaUr.y * theOutputToInputRatio.y);

      rspfDpt newLl = newInputUl + newDeltaUl*outLength.y;
      rspfDpt newLr = newInputUr + newDeltaUr*outLength.y;
      
      rspfDrect newBoundingRect(newInputUl,
                                 newInputUr,
                                 newLl,
                                 newLr);
      

      newBoundingRect = rspfDrect(newBoundingRect.ul() - rspfDpt(theKernelWidth, theKernelHeight),
                                   newBoundingRect.lr() + rspfDpt(theKernelWidth, theKernelHeight));
      rspfIrect roundedRect = newBoundingRect;
      origin = roundedRect.ul();

      dupIn = new rspfImageData(NULL,
                                 input->getScalarType(),
                                 input->getNumberOfBands(),
                                 roundedRect.width(),
                                 roundedRect.height());
      dupIn->setOrigin(roundedRect.ul());
      dupIn->initialize();
      resampleTile(static_cast<T>(0), input, dupIn.get());
   }
   long    inWidth        = dupIn->getWidth();
   long    inHeight       = dupIn->getHeight();
   double  stepSizeWidth  = 1.0/outLength.x;
   double  stepSizeHeight = 1.0/outLength.y;   
   rspfIrect rect = dupIn->getImageRectangle();
   
   rspfDpt startSave(newInputUl.x - rect.ul().x,
                      newInputUl.y - rect.ul().y );
   rspfDpt endSave(newInputUr.x - rect.ul().x,
                    newInputUr.y - rect.ul().y);

   rspfIrect outputRect = output->getImageRectangle();
   rspfIpt subRectUl = outputSubRect.ul();
   long     subRectH  = outputSubRect.height();
   long     subRectW  = outputSubRect.width();
   
   rspfIpt outputRectUl    = outputRect.ul();
   long outputRectW         = outputRect.width();
   long resultOffset = (subRectUl.y - outputRectUl.y)*outputRectW + (subRectUl.x - outputRectUl.x);

   for(rspf_uint32 band = 0; band < input->getNumberOfBands(); band++)
   {
      T* resultBuf       = static_cast<T*>(output->getBuf(band))+resultOffset;
      const T *sourceBuf = static_cast<T*>(dupIn->getBuf(band));
      rspfDpt start = startSave;
      rspfDpt end   = endSave;
      T np = (T)output->getNullPix(band);
                        
      for(long y = 0; y < subRectH; y++)
      {
         double deltaX = (end.x - start.x)*stepSizeWidth;
         double deltaY = (end.y - start.y)*stepSizeHeight;
         rspfDpt pointXY = start;
         
         for(long x = 0; x < subRectW; x++)
         {
            int xPixel = pointXY.x<0?(int)floor(pointXY.x):(int)pointXY.x;
            int yPixel = pointXY.y<0?(int)floor(pointXY.y):(int)pointXY.y;

            
            if( (xPixel >=0) && (xPixel < inWidth) &&
                (yPixel >=0) && (yPixel < inHeight))
            {
               resultBuf[x] = sourceBuf[yPixel*inWidth + xPixel];
            }
            else
            {
               resultBuf[x] = np;
            }
            
            pointXY.y += deltaY;
            pointXY.x += deltaX;
         }
         resultBuf += outputRectW;
         
         start.x += newDeltaUl.x;
         start.y += newDeltaUl.y;
         end.x   += newDeltaUr.x;
         end.y   += newDeltaUr.y;
      }
   }
   dupIn = 0;
}

template <class T>
void rspfResampler::resampleTileNearestNeighbor(T, // dummy template variable
                                                 rspfImageData* input,
                                                 rspfImageData* output,
                                                 const rspfDpt& ul,
                                                 const rspfDpt& ur,
                                                 const rspfDpt& deltaUl,
                                                 const rspfDpt& deltaUr,
                                                 const rspfDpt& length)
{
   resampleTileNearestNeighbor((T)0,
                               input,
                               output,
                               output->getImageRectangle(),
                               ul,
                               ur,
                               deltaUl,
                               deltaUr,
                               length);
}

template <class T>
void rspfResampler::resampleTileNearestNeighbor(T, // dummy template variable
                                                 rspfImageData* input,
                                                 rspfImageData* output,
                                                 const rspfIrect& outputSubRect,
                                                 const rspfDpt& ul,
                                                 const rspfDpt& ur,
                                                 const rspfDpt& deltaUl,
                                                 const rspfDpt& deltaUr,
                                                 const rspfDpt& length)
{   
   long    inWidth        = input->getWidth();
   long    inHeight       = input->getHeight();
   double  stepSizeWidth  = 1.0/length.x;
   double  stepSizeHeight = 1.0/length.y;
   rspfIrect rect        = input->getImageRectangle();
   
   rspfDpt startSave(ul.x - rect.ul().x,
                      ul.y - rect.ul().y );
   
   rspfDpt endSave(ur.x - rect.ul().x,
                    ur.y - rect.ul().y);

   rspfIrect outputRect = output->getImageRectangle();
   rspfIpt subRectUl = outputSubRect.ul();
//   rspfIpt subRectUl((int)outputSubRect.ul().x,
//                      (int)outputSubRect.ul().y);
   long     subRectH  = outputSubRect.height();
   long     subRectW  = outputSubRect.width();
   
   rspfIpt outputRectUl    = outputRect.ul();
   long outputRectW         = outputRect.width();

   long resultOffset = (subRectUl.y - outputRectUl.y)*outputRectW + (subRectUl.x - outputRectUl.x);
   for(rspf_uint32 band = 0; band < input->getNumberOfBands(); band++)
   {
      T* resultBuf       = static_cast<T*>(output->getBuf(band))+resultOffset;
      const T *sourceBuf = static_cast<T*>(input->getBuf(band));
      rspfDpt start = startSave;
      rspfDpt end   = endSave;
      T inNp  = (T)input->getNullPix(band);
      T outNp = (T)output->getNullPix(band);
                        
      for(long y = 0; y < subRectH; y++)
      {
         double deltaX = (end.x - start.x)*stepSizeWidth;
         double deltaY = (end.y - start.y)*stepSizeHeight;
         rspfDpt pointXY = start;
         
         for(long x = 0; x < subRectW; x++)
         {
            int xPixel = pointXY.x<0?(int)floor(pointXY.x):(int)pointXY.x;
            int yPixel = pointXY.y<0?(int)floor(pointXY.y):(int)pointXY.y;

            
             if( (xPixel >=0) && (xPixel < inWidth) &&
                 (yPixel >=0) && (yPixel < inHeight))
             {
                T value = sourceBuf[yPixel*inWidth + xPixel];

                if(value != inNp)
                {
                   resultBuf[x] = value;
                }
                else
                {
                   resultBuf[x] = outNp;
                }
            }
            else
            {
               resultBuf[x] = outNp;
            }
            
            pointXY.y += deltaY;
            pointXY.x += deltaX;
         }
         resultBuf += outputRectW;
         
         start.x += deltaUl.x;
         start.y += deltaUl.y;
         end.x   += deltaUr.x;
         end.y   += deltaUr.y;
      }
   }
}

rspf_int32 rspfResampler::getKernelWidth()const
{
   return theKernelWidth;
}

rspf_int32 rspfResampler::getKernelHeight()const
{
   return theKernelHeight;
}

void rspfResampler::deleteWeightTable()
{
   if(theWeightTableX)
   {
      for(rspf_int32 index = 0; index < theTableHeight; ++index)
      {
         delete [] theWeightTableX[index];
      }
      delete [] theWeightTableX;

      theWeightTableX = NULL;
   }
   if(theWeightTableY)
   {
      for(rspf_int32 index = 0; index < theTableHeight; ++index)
      {
         delete [] theWeightTableY[index];
      }
      delete [] theWeightTableY;

      theWeightTableY = NULL;
   }
}

void rspfResampler::allocateWeightTable()//uint32 outWidth)
{
   if(theWeightTableX||
      theWeightTableY)
   {
      deleteWeightTable();
   }
   
   switch(theResamplerType)
   {
   case rspfResampler_BICUBIC:
   {
      theTableHeight = 4;
      break;
   }
   case rspfResampler_BILINEAR:
   {
      theTableHeight = 2;
      break;
   }
   case rspfResampler_NEAREST_NEIGHBOR:
   {
      theTableHeight = 1;
      break;
   }
   case rspfResampler_NONE:
     {
       theResamplerType = rspfResampler_NEAREST_NEIGHBOR;
      theTableHeight = 1;
      break;
     }
   }
   theTableWidthX = (rspf_int32)rspf::round<int>(theOutputToInputRatio.x);
   theTableWidthY = (rspf_int32)rspf::round<int>(theOutputToInputRatio.y);
   if(theTableWidthX&&theTableHeight)
   {
      theWeightTableX = new double*[theTableHeight];
      
      for(rspf_int32 index = 0; index < theTableHeight; ++index)
      {
         theWeightTableX[index] = new double[theTableWidthX];
      }
   }
   if(theTableWidthY&&theTableHeight)
   {
      theWeightTableY = new double*[theTableHeight];
      
      for(rspf_int32 index = 0; index < theTableHeight; ++index)
      {
         theWeightTableY[index] = new double[theTableWidthY];
      }
   }
}

void rspfResampler::generateWeightTable()
{
   if(theWeightTableX&&
      theWeightTableY)
   {
//      rspf_int32 d = theOutputToInputRatio.theDen;
//      rspf_int32 n = theOutputToInputRatio.theNum;
      rspf_int32 i = 0;
      
      double x = 0.0;
      
      switch(theResamplerType)
      {
   case rspfResampler_NONE:
     {
       theResamplerType = rspfResampler_NEAREST_NEIGHBOR;
       for (i = 0; i < theTableWidthY; i++)
         {
	   theWeightTableY[0][i] = 1;
         }
       for (i = 0; i < theTableWidthX; i++)
         {
	   theWeightTableX[0][i] = 1;
         }
      break;
     }
      case rspfResampler_NEAREST_NEIGHBOR:
      {         
         for (i = 0; i < theTableWidthY; i++)
         {
            theWeightTableY[0][i] = 1;
         }
         for (i = 0; i < theTableWidthX; i++)
         {
            theWeightTableX[0][i] = 1;
         }
         break;
      }
      case rspfResampler_BILINEAR:
      {
         for (i = 0; i < theTableWidthX; i++)
         {
            x = (double)i/(double)(theTableWidthX);
            theWeightTableX[0][i] = x;
            theWeightTableX[1][i] = 1-x;
         }
         for (i = 0; i < theTableWidthY; i++)
         {
            x = (double)i/(double)(theTableWidthY);
            theWeightTableY[0][i] = x;
            theWeightTableY[1][i] = 1-x;
         }
         break;
      }
      case rspfResampler_BICUBIC:
      {         
         for (i = 0; i < theTableWidthX; i++)
         {
            x = (double)i/(double)(theTableWidthX);
            theWeightTableX[0][i] = getCubicC0(x);
            theWeightTableX[1][i] = getCubicC1(x);
            theWeightTableX[2][i] = getCubicC2(x);
            theWeightTableX[3][i] = getCubicC3(x);               
         }
         for (i = 0; i < theTableWidthY; i++)
         {
            x = (double)i/(double)(theTableWidthY);
            theWeightTableY[0][i] = getCubicC0(x);
            theWeightTableY[1][i] = getCubicC1(x);
            theWeightTableY[2][i] = getCubicC2(x);
            theWeightTableY[3][i] = getCubicC3(x);               
         }
         break;
      }
      }
   }
}

void rspfResampler::setResamplerType(rspfResLevelResamplerType type)
{
   if(theResamplerType != type)
   {
      theResamplerType = type;
      
      switch(theResamplerType)
      {
      case rspfResampler_NONE:
      {
	theResamplerType = rspfResampler_NEAREST_NEIGHBOR;
         theKernelWidth  = 1;
         theKernelHeight = 1;

         break;
      }
      case rspfResampler_NEAREST_NEIGHBOR:
      {
         theKernelWidth  = 1;
         theKernelHeight = 1;

         break;
      }
      case rspfResampler_BILINEAR:
      {
         theKernelWidth  = 2;
         theKernelHeight = 2;

         break;
      }
      case rspfResampler_BICUBIC:
      {
         theKernelWidth  = 4;
         theKernelHeight = 4;
         
         break;
      }
      }
      
      allocateWeightTable();
      generateWeightTable();
   }
}

void rspfResampler::setRatio(double outputToInputRatio)
{
   // make it square
   setRatio(rspfDpt(outputToInputRatio, outputToInputRatio));
}

void rspfResampler::setRatio(const rspfDpt& outputToInputRatio)
{
   // make it square
   theOutputToInputRatio.x = (outputToInputRatio.x);
   theOutputToInputRatio.y = (outputToInputRatio.y);

   if((theTableWidthX != rspf::round<int>(outputToInputRatio.x))||
      (theTableWidthY != rspf::round<int>(outputToInputRatio.y)))
   {
      allocateWeightTable();
      generateWeightTable();
   }
}

double rspfResampler::getCubicC0(double t)const
{
   return ((-theCubicAdjustableParameter * t * t * t) +
           (theCubicAdjustableParameter * t * t));
}

double rspfResampler::getCubicC1(double t)const
{
   return (-(theCubicAdjustableParameter + 2.0) * t * t * t +
           (2.0 * theCubicAdjustableParameter + 3.0) * t * t -
           theCubicAdjustableParameter * t);
}

double rspfResampler::getCubicC2(double t)const
{
   return ((theCubicAdjustableParameter + 2.0) * t * t * t -
           (theCubicAdjustableParameter + 3.0) * t * t + 1.0);
}

double rspfResampler::getCubicC3(double t)const
{
   return ((theCubicAdjustableParameter * t * t * t) -
           (2.0f * theCubicAdjustableParameter * t * t) +
           (theCubicAdjustableParameter * t));
}

bool rspfResampler::loadState(const rspfKeywordlist& kwl,
                               const char* prefix)
{   
   const char* resamplerType = kwl.find(prefix, RESAMPLER_CONVOLUTION_TYPE_KW);
   const char* scaleX        = kwl.find(prefix, RESAMPLER_SCALE_X_KW);
   const char* scaleY        = kwl.find(prefix, RESAMPLER_SCALE_Y_KW);
   const char* cubicParameter= kwl.find(prefix, RESAMPLER_CUBIC_PARAMETER_KW);
   
   if(cubicParameter)
   {
      theCubicAdjustableParameter = rspfString(cubicParameter).toDouble();
      if(theCubicAdjustableParameter < -1) theCubicAdjustableParameter = -1;
      if(theCubicAdjustableParameter > 0)  theCubicAdjustableParameter = 0;
   }
   else
   {
      theCubicAdjustableParameter = -.5;
   }
   if(resamplerType)
   {
      rspfString test =rspfString(resamplerType).upcase().trim();
      
      if( test == "BICUBIC")
      {
         setResamplerType(rspfResampler::rspfResampler_BICUBIC);
      }
      else if( test == "BILINEAR")
      {
         setResamplerType(rspfResampler::rspfResampler_BILINEAR);
      }
      else
      {
         setResamplerType(rspfResampler::rspfResampler_NEAREST_NEIGHBOR);
      }
   }
   if(scaleX&&scaleY)
   {
      setRatio(rspfDpt(rspfString(scaleX).toDouble(),
                        rspfString(scaleY).toDouble()));
   }
   allocateWeightTable();
   generateWeightTable();
   
   return rspfConnectableObject::loadState(kwl, prefix);
}

bool rspfResampler::saveState(rspfKeywordlist& kwl,
                               const char* prefix)const
{
   rspfString resamplerType;
   if(getResamplerType() == rspfResampler_BICUBIC)
   {
      resamplerType = "BICUBIC";
   }
   else if(getResamplerType() == rspfResampler_BILINEAR)
   {
      resamplerType = "BILINEAR";
   }
   else 
   {
      resamplerType = "NEAREST_NEIGHBOR";
   }
   kwl.add(prefix,
           RESAMPLER_CONVOLUTION_TYPE_KW,
           resamplerType.c_str(),
           true);

   kwl.add(prefix,
           RESAMPLER_SCALE_X_KW,
           theOutputToInputRatio.x,
           true);

   kwl.add(prefix,
           RESAMPLER_SCALE_Y_KW,
           theOutputToInputRatio.y,
           true);

   kwl.add(prefix,
           RESAMPLER_CUBIC_PARAMETER_KW,
           theCubicAdjustableParameter,
           true);
   
   return rspfConnectableObject::saveState(kwl, prefix);
}
