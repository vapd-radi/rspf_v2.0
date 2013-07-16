//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts

// Contributor:
//         David A. Horner (DAH) http://dave.thehorners.com
//
//*************************************************************************
// $Id: rspfFilterResampler.cpp 20326 2011-12-07 13:48:18Z dburken $

#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/imaging/rspfFilterTable.h>
rspfFilterResampler::rspfFilterResampler()
   :theMinifyFilter(new rspfNearestNeighborFilter()),
    theMagnifyFilter(new rspfNearestNeighborFilter()),
    theMinifyFilterType(rspfFilterResampler_NEAREST_NEIGHBOR),
    theMagnifyFilterType(rspfFilterResampler_NEAREST_NEIGHBOR),
    theScaleFactor(1.0, 1.0),
    theInverseScaleFactor(1.0, 1.0),
    theBlurFactor(1.0)
{
   setScaleFactor(rspfDpt(1.0, 1.0));
   loadState(rspfPreferences::instance()->preferencesKWL(),"resampler.");

}

rspfFilterResampler::~rspfFilterResampler()
{
   if(theMinifyFilter)
   {
      delete theMinifyFilter;
      theMinifyFilter = NULL;
   }
   if(theMagnifyFilter)
   {
      delete theMagnifyFilter;
      theMagnifyFilter = NULL;
   }
}


void rspfFilterResampler::resample(const rspfRefPtr<rspfImageData>& input,
				    rspfRefPtr<rspfImageData>& output,
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

void rspfFilterResampler::resample(const rspfRefPtr<rspfImageData>& input,
				    rspfRefPtr<rspfImageData>& output,
				    const rspfIrect& outputSubRect,
				    const rspfDpt& ul,
				    const rspfDpt& ur,
				    const rspfDpt& deltaUl,
				    const rspfDpt& deltaUr,
				    const rspfDpt& length)
{
   if(!input.valid()   ||
      !output.valid()  ||
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
         resampleBilinearTile(rspf_uint8(0), // dummy template variable
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
      case RSPF_SINT8:
      {
         resampleBilinearTile(rspf_sint8(0), // dummy template variable
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
	 resampleBilinearTile(rspf_uint16(0), // dummy template variable
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
	 resampleBilinearTile(rspf_sint16(0), // dummy template variable
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
	 resampleBilinearTile(rspf_uint32(0), // dummy template variable
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
      case RSPF_SINT32:
      {
	 resampleBilinearTile(rspf_sint32(0), // dummy template variable
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
	 resampleBilinearTile(rspf_float32(0.0), // dummy template variable
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
	 resampleBilinearTile(rspf_float64(0.0), // dummy template variable
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
            << "rspfFilterResampler::resample error: unknown scalar type:  "
            << scalarType << endl;
      }
   }
}


rspfFilter* rspfFilterResampler::createNewFilter(
   rspfFilterResamplerType filterType,
   rspfFilterResamplerType& result)
{
   switch(filterType)
   {
      case rspfFilterResampler_NEAREST_NEIGHBOR:
      {
         return new rspfNearestNeighborFilter();
      }
      case rspfFilterResampler_BOX:
      {
         return new rspfBoxFilter();
      }
      case rspfFilterResampler_GAUSSIAN:
      {
         return new rspfGaussianFilter();
      }
      case rspfFilterResampler_CUBIC:
      {
         return new rspfCubicFilter();
      }
      case rspfFilterResampler_HANNING:
      {
         return new rspfHanningFilter();
      }
      case rspfFilterResampler_HAMMING:
      {
         return new rspfHammingFilter();
      }
      case rspfFilterResampler_LANCZOS:
      {
         return new rspfLanczosFilter();
      }
      case rspfFilterResampler_CATROM:
      {
         return new rspfCatromFilter();
      }
      case rspfFilterResampler_MITCHELL:
      {
         return new rspfMitchellFilter();
      }
      case rspfFilterResampler_BLACKMAN:
      {
         return new rspfBlackmanFilter();
      }
      case rspfFilterResampler_BLACKMAN_SINC:
      {
         return new rspfBlackmanSincFilter();
      }
      case rspfFilterResampler_BLACKMAN_BESSEL:
      {
         return new rspfBlackmanBesselFilter();
      }
      case rspfFilterResampler_QUADRATIC:
      {
         return new rspfQuadraticFilter();
      }
      case rspfFilterResampler_TRIANGLE:
      {
         return new rspfTriangleFilter();
      }
      case rspfFilterResampler_HERMITE:
      {
         return new rspfHermiteFilter();
      }
      case rspfFilterResampler_BELL:
      {
         return new rspfGaussianFilter();
      }
      case rspfFilterResampler_BSPLINE:
      {
         return new rspfBSplineFilter();
      }
   }

   result = rspfFilterResampler_NEAREST_NEIGHBOR;
   return new rspfNearestNeighborFilter();
}

void rspfFilterResampler::setScaleFactor(const rspfDpt& scale)
{
   theScaleFactor = scale;
   if(fabs(theScaleFactor.x) <= FLT_EPSILON)
   {
      theScaleFactor.x = 1.0;
   }
   if(fabs(theScaleFactor.y) <= FLT_EPSILON)
   {
      theScaleFactor.y = 1.0;
   }

   theInverseScaleFactor.x = 1.0/theScaleFactor.x;
   theInverseScaleFactor.y = 1.0/theScaleFactor.y;
}

template <class T> void rspfFilterResampler::resampleBilinearTile(
   T /* dummy */,
   const rspfRefPtr<rspfImageData>& input,
   rspfRefPtr<rspfImageData>& output,
   const rspfIrect& outputSubRect,
   const rspfDpt& inputUl,
   const rspfDpt& inputUr,
   const rspfDpt& deltaUl,
   const rspfDpt& deltaUr,
   const rspfDpt& outLength)
{
#if 0 /* Please leave for debug. */
   std::cout << "INPUT  = \n" << *input << std::endl
             << "OUTPUT = \n" << *output << std::endl
             << "inputUL= " << inputUl << std::endl
             << "inputUR= " << inputUr << std::endl
             << "deltaUL= " << deltaUl << std::endl
             << "deltaUr= " << deltaUr << std::endl
             << "outlength= " << outLength << std::endl;
#endif
   
   rspf_uint32  band, centerOffset;
   rspf_float64 tmpFlt64, stepSizeWidth;

   if(outLength.x>1) {
      stepSizeWidth  = 1.0/(outLength.x-1.0);
   } else {
      stepSizeWidth   = 1.0;
   }

   // INPUT INFORMATION
   rspf_uint32       inWidth      = input->getWidth();
   rspf_uint32       inBandSize   = input->getSizePerBand();  // fix for out-of-bounds check OLK 06/2005
   rspf_uint32       BANDS        = input->getNumberOfBands();
   rspfIrect         inputRect    = input->getImageRectangle();

   // OUTPUT INFORMATION
   const rspf_float64* NULL_PIX      = output->getNullPix(); 
   const rspf_float64* MIN_PIX       = output->getMinPix(); 
   const rspf_float64* MAX_PIX       = output->getMaxPix(); 
   rspfIrect           outputRect    = output->getImageRectangle();
   rspf_uint32         resultRectH   = outputSubRect.height();
   rspf_uint32         resultRectW   = outputSubRect.width();
   rspf_uint32         outputRectW   = outputRect.width();

   // calculate the offset into the data so we can refer to it at 0 index.
   rspf_uint32 resultOffset=(outputSubRect.ul().y - outputRect.ul().y)*outputRectW +
                             (outputSubRect.ul().x - outputRect.ul().x);

   // make a local copy of the band pointers (at resultOffset)
   rspf_float64 *densityvals=new rspf_float64[BANDS];
   rspf_float64 *pixelvals=new rspf_float64[BANDS];
   const T* *inputBuf  = new const T*[BANDS];
   T* *resultBuf = new T*[BANDS];
   if(!pixelvals||!inputBuf||!resultBuf)
   {
      return;
   }
   
   for(band = 0; band < BANDS; ++band)
   {
      inputBuf[band] = static_cast<const T*>(input->getBuf(band));
      resultBuf[band] = static_cast<T*>(output->getBuf(band))+resultOffset;
   }

   // FILTER INFORMAION
   rspf_uint32 xkernel_width  = theFilterTable.getWidth();;
   rspf_uint32 ykernel_height = theFilterTable.getHeight();
   double xkernel_half_width   = theFilterTable.getXSupport();
   double ykernel_half_height  = theFilterTable.getYSupport();

   double initialx  = inputUl.x-inputRect.ul().x;
   double initialy  = inputUl.y-inputRect.ul().y;
   double terminalx = inputUr.x-inputRect.ul().x;
   double terminaly = inputUr.y-inputRect.ul().y;
   double pointx,pointy,deltaX,deltaY;
   rspf_int32 starty,startx; 

   if(xkernel_width==0 || ykernel_height==0)
   {
      // USING NEAREST NEIGHBOR
      for(rspf_uint32 resultY = 0; resultY < resultRectH; ++resultY)
      {
//          deltaX = (terminalx-initialx) * stepSizeWidth;
//          deltaY = (terminaly-initialy) * stepSizeHeight;
         // this should be stepsize width for both since we are traversing horizontal
         deltaX = (terminalx-initialx) * stepSizeWidth;
         deltaY = (terminaly-initialy) * stepSizeWidth;
         pointx = initialx;
         pointy = initialy;
         for(rspf_uint32 resultX = 0; resultX < resultRectW; ++resultX)
         {
            // just sample center in input space.
            centerOffset = rspf::round<int>(pointy)*inWidth + rspf::round<int>(pointx);
            for(band=0;band<BANDS;++band)
            {
               resultBuf[band][resultX] = inputBuf[band][centerOffset];
            }
            pointy += deltaY;
            pointx += deltaX;
         } // End of loop in x direction.
         
         // increment pointers to where we are now.
         for(band=0;band<BANDS;++band)
         {
            resultBuf[band] += outputRectW;
         }
         initialx += deltaUl.x;
         initialy += deltaUl.y;
         terminalx  += deltaUr.x;
         terminaly  += deltaUr.y;
      } // End of loop in y direction.
      
   }
   else
   {
      // USING A KERNEL
      const double* kernel;
      rspf_uint32 iy,ix,sourceIndex,nullCount;
      for(rspf_uint32 resultY = 0; resultY < resultRectH; ++resultY)
      {
         deltaX = (terminalx-initialx) * stepSizeWidth;
         deltaY = (terminaly-initialy) * stepSizeWidth;
         pointx = initialx;
         pointy = initialy;
         for(rspf_uint32 resultX = 0; resultX < resultRectW; ++resultX)
         {
            starty  = rspf::round<int>(pointy - ykernel_half_height + .5);
            startx  = rspf::round<int>(pointx - xkernel_half_width + .5);
            centerOffset = rspf::round<int>(pointy)*inWidth + rspf::round<int>(pointx);
            sourceIndex = starty*inWidth+startx;

            // look at center pixel, make sure they aren't all null.
            nullCount=0;
            if(centerOffset<inBandSize)
            {
               for (band=0;band<BANDS;++band)
               {
                  if(inputBuf[band][centerOffset]==static_cast<T>(NULL_PIX[band]))
                  {
                     ++nullCount;
                  }
               }
               // the center of the kernel is outside the input space, just set null.
            }
            else
            {
               nullCount=BANDS;
            }

            // make sure we have non-null data and we fit within the inputBuf.
            if ( nullCount==BANDS || (sourceIndex>=inBandSize))
            {
               // we don't need to continue, just assign null!
               for (band=0;band<BANDS;++band)
               {
                  resultBuf[band][resultX] = static_cast<T>(NULL_PIX[band]);
               }
            }
            else
            {  
               kernel = theFilterTable.getClosestWeights(pointx,pointy);
               if(kernel)
               {
                  // reset the pixel/density sums for each band to zero.
                  memset(densityvals,'\0',sizeof(rspf_float64)*BANDS);
                  memset(pixelvals,'\0',sizeof(rspf_float64)*BANDS);

                  // apply kernel to input space.
                  for (iy=0;((iy<ykernel_height)&&(sourceIndex<inBandSize));++iy)
                  {
                     for (ix = 0;((ix<xkernel_width)&&(sourceIndex<inBandSize));++ix)
                     {
                        tmpFlt64=*kernel; // pixel weight;
                        for(band=0;band<BANDS;++band)
                        {
                           if(inputBuf[band][sourceIndex]!=NULL_PIX[band])
                           {
                              densityvals[band] += tmpFlt64;
                              pixelvals[band] += (inputBuf[band][sourceIndex]*tmpFlt64);
                           }
                        }
                        ++sourceIndex;
                        ++kernel;
                        if(sourceIndex>=inBandSize)
                        {
                           break;
                        }
                     }
                     sourceIndex+=(inWidth-xkernel_width);
                  }

                  // actually assign the value to the output
                  for (band = 0; band < BANDS; ++band)
                  {
                     if(densityvals[band]<=FLT_EPSILON)
                     {
                        //---
                        // Setting tempFlt64 to pixelvals[band] causing 0's where -32768
                        // should be when null check was skipped above.
                        // tmpFlt64 = pixelvals[band];
                        //---
                        tmpFlt64 = NULL_PIX[band];
                     }
                     else
                     {
                        // normalize
                        tmpFlt64 = pixelvals[band]/densityvals[band];
                     }
                     
                     // clamp
                     tmpFlt64 = (tmpFlt64>=MIN_PIX[band]?(tmpFlt64<MAX_PIX[band]?tmpFlt64:MAX_PIX[band]):MIN_PIX[band]); 
                     // set resultant pixel value.
                     resultBuf[band][resultX] = static_cast<T>(tmpFlt64);
                  }

                  // we didn't get a filter kernel, just set NULL in this disaster.
               }
               else
               {
                  for (band=0;band<BANDS;++band)
                  {
                     resultBuf[band][resultX] = static_cast<T>(NULL_PIX[band]);
                  }
               }                  
            }
            pointy += deltaY;
            pointx += deltaX;
         } // End of loop in x direction.

         // increment pointers to where we are now.
         for(band=0;band<BANDS;++band)
         {
            resultBuf[band] += outputRectW;
         }
         initialx += deltaUl.x;
         initialy += deltaUl.y;
         terminalx  += deltaUr.x;
         terminaly  += deltaUr.y;
      } // End of loop in y direction.
   } // USING A KERNEL END

   delete [] densityvals;
   delete [] pixelvals;
   delete [] resultBuf;
   delete [] inputBuf;
}

rspfString rspfFilterResampler::getFilterTypeAsString(rspfFilterResamplerType type)const
{
   switch(type)
   {
      case rspfFilterResampler_NEAREST_NEIGHBOR:
      {
         return "nearest neighbor";
      }
      case rspfFilterResampler_BOX:
      {
         return "box";
      }
      case rspfFilterResampler_GAUSSIAN:
      {
         return "gaussian";
      }
      case rspfFilterResampler_CUBIC:
      {
         return "cubic";
      }
      case rspfFilterResampler_HANNING:
      {
         return "hanning";
      }
      case rspfFilterResampler_HAMMING:
      {
         return "hamming";
      }
      case rspfFilterResampler_LANCZOS:
      {
         return "lanczos";
      }
      case rspfFilterResampler_MITCHELL:
      {
         return "mitchell";
      }
      case rspfFilterResampler_CATROM:
      {
         return "catrom";
      }
      case rspfFilterResampler_BLACKMAN:
      {
         return "blackman";
      }
      case rspfFilterResampler_BLACKMAN_SINC:
      {
         return "sinc";
      }
      case rspfFilterResampler_BLACKMAN_BESSEL:
      {
         return "bessel";
      }
      case rspfFilterResampler_QUADRATIC:
      {
         return "quadratic";
      }
      case rspfFilterResampler_TRIANGLE:
      {
         return "bilinear";
      }
      case rspfFilterResampler_HERMITE:
      {
         return "hermite";
      }
      case rspfFilterResampler_BELL:
      {
         return "gaussian";
      }
      case rspfFilterResampler_BSPLINE:
      {
         return "bspline";
      }
   }

   return "nearest neighbor";
}

void rspfFilterResampler::getFilterTypes(std::vector<rspfString>& filterTypes)const
{
  filterTypes.push_back("nearest neighbor");
  filterTypes.push_back("bilinear");
  filterTypes.push_back("cubic");
//  filterTypes.push_back("bell");
  filterTypes.push_back("bessel");
  filterTypes.push_back("blackman");
  filterTypes.push_back("box");
  filterTypes.push_back("bspline");
  filterTypes.push_back("catrom");
  filterTypes.push_back("gaussian");
  filterTypes.push_back("hanning");
  filterTypes.push_back("hamming");
  filterTypes.push_back("hermite");
  filterTypes.push_back("lanczos");
  filterTypes.push_back("mitchell");
  filterTypes.push_back("quadratic");
  filterTypes.push_back("sinc");
}


rspfFilterResampler::rspfFilterResamplerType rspfFilterResampler::getFilterType(const rspfString& type)const
{
   rspfString typeUpper = type;
   typeUpper = typeUpper.upcase();

   if(typeUpper.contains("BOX"))
   {
      return rspfFilterResampler_BOX;
   }
   else if(typeUpper.contains("NEAREST"))
   {
      return rspfFilterResampler_NEAREST_NEIGHBOR;
   }
   else if(typeUpper.contains("GAUSSIAN"))
   {
      return rspfFilterResampler_GAUSSIAN;
   }
   else if(typeUpper.contains("HANNING"))
   {
      return rspfFilterResampler_HANNING;
   }
   else if(typeUpper.contains("HAMMING"))
   {
      return rspfFilterResampler_HAMMING;
   }
   else if(typeUpper.contains("LANCZOS"))
   {
      return rspfFilterResampler_LANCZOS;
   }
   else if(typeUpper.contains("MITCHELL"))
   {
      return rspfFilterResampler_MITCHELL;
   }
   else if(typeUpper.contains("CATROM"))
   {
      return rspfFilterResampler_CATROM;
   }
   else if(typeUpper.contains("CUBIC"))
   {
      return rspfFilterResampler_CUBIC;
   }
   else if(typeUpper.contains("BESSEL"))
   {
      return rspfFilterResampler_BLACKMAN_BESSEL;
   }
   else if(typeUpper.contains("SINC"))
   {
      return rspfFilterResampler_BLACKMAN_SINC;
   }
   else if(typeUpper.contains("BLACKMAN"))
   {
      return rspfFilterResampler_BLACKMAN;
   }
   else if(typeUpper.contains("QUADRATIC"))
   {
      return rspfFilterResampler_QUADRATIC;
   }
   else if(typeUpper.contains("TRIANGLE"))
   {
      return rspfFilterResampler_TRIANGLE;
   }
   else if(typeUpper.contains("BILINEAR"))
   {
      return rspfFilterResampler_TRIANGLE;
   }
   else if(typeUpper.contains("HERMITE"))
   {
      return rspfFilterResampler_HERMITE;
   }
//    else if(typeUpper.contains("BELL"))
//    {
//       return rspfFilterResampler_BELL;
//    }
   else if(typeUpper.contains("BSPLINE"))
   {
      return rspfFilterResampler_BSPLINE;
   }
   
   return rspfFilterResampler_NEAREST_NEIGHBOR;
}

void rspfFilterResampler::getKernelSupport(double& x, double& y)const
{
   const rspfFilter* horizontalFilter = getHorizontalFilter();
   const rspfFilter* verticalFilter   = getVerticalFilter();

   if(!horizontalFilter)
   {
      x = 0.0;
   }
   else
   {
//       x = theBlurFactor*rspf::max(1.0/theScaleFactor.x, 1.0)*
//           horizontalFilter->getSupport();
      x = horizontalFilter->getSupport();
   }

   if(!verticalFilter)
   {
      y = 0.0;
   }
   else
   {
//       y = theBlurFactor*rspf::max(1.0/theScaleFactor.y, 1.0)*
//           verticalFilter->getSupport();
      y = verticalFilter->getSupport();
   }
}

const rspfFilter* rspfFilterResampler::getHorizontalFilter()const
{
   if(theScaleFactor.x < 1)
   {
      return theMinifyFilter;
   }

   return theMagnifyFilter;
}

const rspfFilter* rspfFilterResampler::getVerticalFilter()const
{
   if(theScaleFactor.y < 1)
   {
      return theMinifyFilter;
   }

   return theMagnifyFilter;
}

void rspfFilterResampler::setFilterType(const rspfString& type)
{
   setFilterType(type, type);
}

void rspfFilterResampler::setFilterType(rspfFilterResamplerType filterType)
{
   setFilterType(filterType, filterType);
}
void rspfFilterResampler::setFilterType(const rspfString& minifyType,
                                         const rspfString& magnifyType)
{
   setFilterType(getFilterType(minifyType),
                 getFilterType(magnifyType));
}

void rspfFilterResampler::setMinifyFilterType(const rspfString& minifyType)
{
   setMinifyFilterType(getFilterType(minifyType));
}

void rspfFilterResampler::setMagnifyFilterType(const rspfString& magnifyType)
{
   setMagnifyFilterType(getFilterType(magnifyType));
}

void rspfFilterResampler::setMinifyFilterType(rspfFilterResamplerType filterType)
{
   setFilterType(filterType,
                 theMagnifyFilterType);
}

void rspfFilterResampler::setMagnifyFilterType(rspfFilterResamplerType filterType)
{
   setFilterType(theMinifyFilterType,filterType);
}

rspfString rspfFilterResampler::getMinifyFilterTypeAsString()const
{
   return getFilterTypeAsString(theMinifyFilterType);
}

rspfString rspfFilterResampler::getMagnifyFilterTypeAsString()const
{
   return getFilterTypeAsString(theMagnifyFilterType);
}

void rspfFilterResampler::setFilterType(
   rspfFilterResamplerType minifyFilterType,
   rspfFilterResamplerType magnifyFilterType)
{
   if(theMinifyFilter)
   {
      delete theMinifyFilter;
      theMinifyFilter = NULL;
   }
   if(theMagnifyFilter)
   {
      delete theMagnifyFilter;
      theMagnifyFilter = NULL;
   }
   
   theMinifyFilterType  = minifyFilterType;
   theMagnifyFilterType = magnifyFilterType;
   
   theMinifyFilter  = createNewFilter(minifyFilterType, theMinifyFilterType);
   theMagnifyFilter = createNewFilter(magnifyFilterType, theMagnifyFilterType);
   computeTable();
}

rspf_float64 rspfFilterResampler::getBlurFactor()const
{
   return theBlurFactor;
}

void rspfFilterResampler::setBlurFactor(rspf_float64 blur)
{
   theBlurFactor = blur;
}

bool rspfFilterResampler::saveState(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::SCALE_X_KW,
           theScaleFactor.x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::SCALE_Y_KW,
           theScaleFactor.y,
           true);
   kwl.add(prefix,
           "minify_type",
           getFilterTypeAsString(theMinifyFilterType),
           true);
   kwl.add(prefix,
           "magnify_type",
           getFilterTypeAsString(theMagnifyFilterType),
           true);

   return true;
}

bool rspfFilterResampler::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   const char* lookup = 0;

   lookup = kwl.find(prefix, rspfKeywordNames::SCALE_X_KW);
   if (lookup)
   {
      theScaleFactor.x = rspfString(lookup).toDouble();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::SCALE_Y_KW);
   if (lookup)
   {
      theScaleFactor.y = rspfString(lookup).toDouble();
   }

   rspfString minify;
   lookup = kwl.find(prefix, "minify_type");
   if (lookup)
   {
      minify = lookup;
   }

   rspfString magnify;
   lookup = kwl.find(prefix, "magnify_type");
   if (lookup)
   {
      magnify = lookup;
   }

   if(fabs(theScaleFactor.x) <= FLT_EPSILON)
   {
      theScaleFactor.x = 1.0;
   }
   if(fabs(theScaleFactor.y) <= FLT_EPSILON)
   {
      theScaleFactor.y = 1.0;
   }

   theInverseScaleFactor.x = 1.0/theScaleFactor.x;
   theInverseScaleFactor.y = 1.0/theScaleFactor.y;
   
   setFilterType(getFilterType(minify),
                 getFilterType(magnify));

   return true;
}

void rspfFilterResampler::computeTable()
{
  theFilterTable.buildTable(32, *theMagnifyFilter);
}

