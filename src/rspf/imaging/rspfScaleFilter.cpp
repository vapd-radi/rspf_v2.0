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
// $Id: rspfScaleFilter.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfScaleFilter.h>
#include <rspf/imaging/rspfFilter.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfDiscreteConvolutionKernel.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfKeywordNames.h>

RTTI_DEF1(rspfScaleFilter, "rspfScaleFilter", rspfImageSourceFilter);

//**************************************************************************************************
rspfScaleFilter::rspfScaleFilter()
   :rspfImageSourceFilter(),
    m_BlankTile(NULL),
    m_Tile(NULL),
    m_MinifyFilter(NULL),
    m_MagnifyFilter(NULL),
    m_MinifyFilterType(rspfScaleFilter_NEAREST_NEIGHBOR),
    m_MagnifyFilterType(rspfScaleFilter_NEAREST_NEIGHBOR),
    m_ScaleFactor(1.0, 1.0),
    m_InverseScaleFactor(1.0, 1.0),
    m_TileSize(64, 64),
    m_BlurFactor(1.0)
{
   m_InputRect.makeNan();
   m_MinifyFilter  = new rspfNearestNeighborFilter();
   m_MagnifyFilter = new rspfNearestNeighborFilter();
}

//**************************************************************************************************
rspfScaleFilter::rspfScaleFilter(rspfImageSource* inputSource,
                                   const rspfDpt& scaleFactor)
   :rspfImageSourceFilter(inputSource),
    m_BlankTile(NULL),
    m_Tile(NULL),
    m_MinifyFilter(NULL),
    m_MagnifyFilter(NULL),
    m_MinifyFilterType(rspfScaleFilter_NEAREST_NEIGHBOR),
    m_MagnifyFilterType(rspfScaleFilter_NEAREST_NEIGHBOR),
    m_ScaleFactor(scaleFactor),
    m_TileSize(64, 64),
    m_BlurFactor(1.0)
{
   m_InputRect.makeNan();
   m_MinifyFilter  = new rspfNearestNeighborFilter();
   m_MagnifyFilter = new rspfNearestNeighborFilter();
}

//**************************************************************************************************
rspfScaleFilter::~rspfScaleFilter()
{
   if(m_MinifyFilter)
   {
      delete m_MinifyFilter;
      m_MinifyFilter = NULL;
   }
   
   if(m_MagnifyFilter)
   {
      delete m_MagnifyFilter;
      m_MagnifyFilter = NULL;
   }
}

//**************************************************************************************************
rspfRefPtr<rspfImageData> rspfScaleFilter::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   
   if((!isSourceEnabled())||
      (!theInputConnection)||
      ((m_ScaleFactor.x == 1.0)&&
       (m_ScaleFactor.y == 1.0)&&
       (m_BlurFactor == 1.0)))
   {
      return rspfImageSourceFilter::getTile(tileRect, resLevel);
   }
   if(!m_Tile.valid())
   {
      allocate();
   }

   if(!m_Tile)
   {
      return rspfImageSourceFilter::getTile(tileRect, resLevel);
   }

   m_Tile->makeBlank();

                       
   rspfIrect imageRect = tileRect*m_InverseScaleFactor;

   m_Tile->setImageRectangle(tileRect);
   m_BlankTile->setImageRectangle(tileRect);


   double xSupport;
   double ySupport;

   getSupport(xSupport, ySupport);
   
   rspfIpt deltaPt;
   deltaPt.x = (rspf_int32)ceil(xSupport);
   deltaPt.y = (rspf_int32)ceil(ySupport);

   imageRect = rspfIrect(imageRect.ul().x - (deltaPt.x),
                          imageRect.ul().y - (deltaPt.y),
                          imageRect.lr().x + (deltaPt.x),
                          imageRect.lr().y + (deltaPt.y));

   
   runFilter(imageRect, tileRect);
   
   m_Tile->validate();
   
   return m_Tile;
}

//**************************************************************************************************
void rspfScaleFilter::runFilter(const rspfIrect& imageRect,
                                 const rspfIrect& viewRect)
{
   switch(m_Tile->getScalarType())
   {
      case RSPF_UINT8:
      {
         runFilterTemplate((rspf_uint8)0,
                           imageRect,
                           viewRect);
         break;
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         runFilterTemplate((rspf_uint16)0,
                           imageRect,
                           viewRect);
         break;
      }
      case RSPF_SINT16:
      {
         runFilterTemplate((rspf_sint16)0,
                           imageRect,
                           viewRect);
         break;
      }
      case RSPF_UINT32:
      {
         runFilterTemplate((rspf_uint32)0,
                           imageRect,
                           viewRect);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         runFilterTemplate((rspf_float64)0,
                           imageRect,
                           viewRect);
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         runFilterTemplate((rspf_float32)0,
                           imageRect,
                           viewRect);
         break;
      }
      default:
         break;
   }
}

//**************************************************************************************************
template <class T>
void rspfScaleFilter::runFilterTemplate(T dummy,
                                         const rspfIrect& imageRect,
                                         const rspfIrect& viewRect)
{
   rspfRefPtr<rspfImageData> inputData =
      theInputConnection->getTile(imageRect);

   if(!inputData.valid()   ||
      !inputData->getBuf() ||
      (inputData->getDataObjectStatus() == RSPF_EMPTY))
   {
      return;
   }

   rspf_int32 h = imageRect.height();
   rspfRefPtr<rspfImageData> tempData =
      rspfImageDataFactory::instance()->create(NULL,
                                                inputData->getScalarType(),
                                                inputData->getNumberOfBands(),
                                                viewRect.width(),
                                                h);
   tempData->setOrigin(rspfIpt(viewRect.ul().x,
                                imageRect.ul().y));
   
   tempData->initialize();
   
   if((m_ScaleFactor.x != 1.0)||
      (m_BlurFactor != 1.0))
   {
      runHorizontalFilterTemplate(dummy,
                                  inputData,
                                  tempData);
      tempData->validate();
   }
   else
   {
      tempData->loadTile(inputData.get());
   }
   
   if((m_ScaleFactor.y != 1.0)||
      (m_BlurFactor != 1.0))
   {
      runVerticalFilterTemplate(dummy,
                                tempData,
                                m_Tile);
   }
   else
   {
      m_Tile->loadTile(tempData.get());
   }
   
   m_Tile->validate();
}

//**************************************************************************************************
rspfIrect rspfScaleFilter::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfIrect result = rspfImageSourceFilter::getBoundingRect(resLevel);

   if(!result.hasNans())
   {
      result = rspfIrect(result.ul().x,
                          result.ul().y,
                          result.lr().x+1,
                          result.lr().y+1);
      result *= m_ScaleFactor;
   }
   
   return result;
}

//**************************************************************************************************
void rspfScaleFilter::setFilterType(rspfScaleFilterType filterType)
{
   setFilterType(filterType, filterType);
}


//**************************************************************************************************
void rspfScaleFilter::setFilterType(rspfScaleFilterType minifyFilterType,
                                     rspfScaleFilterType magnifyFilterType)
{
   if(m_MinifyFilter)
   {
      delete m_MinifyFilter;
      m_MinifyFilter = NULL;
   }
   if(m_MagnifyFilter)
   {
      delete m_MagnifyFilter;
      m_MagnifyFilter = NULL;
   }
   
   m_MinifyFilterType  = minifyFilterType;
   m_MagnifyFilterType = magnifyFilterType;
   
   m_MinifyFilter  = createNewFilter(minifyFilterType, m_MinifyFilterType);
   m_MagnifyFilter = createNewFilter(magnifyFilterType, m_MagnifyFilterType);
}

//**************************************************************************************************
rspfFilter* rspfScaleFilter::createNewFilter(rspfScaleFilterType filterType,
                                               rspfScaleFilterType& result)
{
   switch(filterType)
   {
   case rspfScaleFilter_NEAREST_NEIGHBOR:
   {
      return new rspfNearestNeighborFilter();
   }
   case rspfScaleFilter_BOX:
   {
      return new rspfBoxFilter();
   }
   case rspfScaleFilter_GAUSSIAN:
   {
      return new rspfGaussianFilter();
   }
   case rspfScaleFilter_CUBIC:
   {
      return new rspfCubicFilter();
   }
   case rspfScaleFilter_HANNING:
   {
      return new rspfHanningFilter();
   }
   case rspfScaleFilter_HAMMING:
   {
      return new rspfHammingFilter();
   }
   case rspfScaleFilter_LANCZOS:
   {
      return new rspfLanczosFilter();
   }
   case rspfScaleFilter_CATROM:
   {
      return new rspfCatromFilter();
   }
   case rspfScaleFilter_MITCHELL:
   {
      return new rspfMitchellFilter();
   }
   case rspfScaleFilter_BLACKMAN:
   {
      return new rspfBlackmanFilter();
   }
   case rspfScaleFilter_BLACKMAN_SINC:
   {
      return new rspfBlackmanSincFilter();
   }
   case rspfScaleFilter_BLACKMAN_BESSEL:
   {
      return new rspfBlackmanBesselFilter();
   }
   case rspfScaleFilter_QUADRATIC:
   {
      return new rspfQuadraticFilter();
   }
   case rspfScaleFilter_TRIANGLE:
   {
      return new rspfTriangleFilter();
   }
   case rspfScaleFilter_HERMITE:
   {
      return new rspfHermiteFilter();
   }
   
   }

   result = rspfScaleFilter_NEAREST_NEIGHBOR;
   return new rspfNearestNeighborFilter();
}

//**************************************************************************************************
void rspfScaleFilter::setScaleFactor(const rspfDpt& scale)
{
   m_ScaleFactor = scale;
   if(fabs(m_ScaleFactor.x) <= FLT_EPSILON)
   {
      m_ScaleFactor.x = 1.0;
   }
   if(fabs(m_ScaleFactor.y) <= FLT_EPSILON)
   {
      m_ScaleFactor.y = 1.0;
   }

   m_InverseScaleFactor.x = 1.0/m_ScaleFactor.x;
   m_InverseScaleFactor.y = 1.0/m_ScaleFactor.y;

   // A change in the scale factor implies a change to the image geometry. If one has been created
   // it needs to be modified:
   updateGeometry();
}


//**************************************************************************************************
template <class T> void rspfScaleFilter::runHorizontalFilterTemplate(
   T /* dummy */,
   const rspfRefPtr<rspfImageData>& input,
   rspfRefPtr<rspfImageData>& output)
{
   rspfIrect viewRect  = output->getImageRectangle();
   rspfIrect imageRect = input->getImageRectangle();
   rspf_int32 vw = viewRect.width();
   rspf_int32 vh = viewRect.height();
   rspf_int32 iw = imageRect.width();
   rspfIpt origin(viewRect.ul());
   rspfIpt imageOrigin(imageRect.ul());
   rspfIpt inputUl = m_InputRect.ul();
   rspfIpt inputLr = m_InputRect.lr();
   
   double scale = 0.0;
   double support = 0.0;
   rspf_int32 x = 0;
   rspf_int32 y = 0;
   rspf_int32 start = 0;
   rspf_int32 stop  = 0;
   rspf_int32 kernelIdx = 0;
   const rspfFilter* filter = getHorizontalFilter();
   rspf_float64 center = 0.0;
   rspf_int32 bandIdx = 0;
   rspf_int32 numberOfBands = m_Tile->getNumberOfBands();
   
   scale = m_BlurFactor*rspf::max(1.0/m_ScaleFactor.x, 1.0);
   
   support=scale*filter->getSupport();
   if (support <= 0.5)
   {
      support = 0.5 + FLT_EPSILON;
      scale = 1.0;
   }
   scale=1.0/scale;
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* imageBuf = (T*)input->getBuf(bandIdx);
      T* viewBuf  = (T*)output->getBuf(bandIdx);
      T np        = (T)input->getNullPix(bandIdx);
      T outNp     = (T)output->getNullPix(bandIdx);
      T outMinPix = (T)output->getMinPix(bandIdx);
      T outMaxPix = (T)output->getMaxPix(bandIdx);
      
      for(x = 0; x < vw; ++x)
      {
         center=(origin.x + x+ .5)/m_ScaleFactor.x;
         start=rspf::max((rspf_int32)rspf::round<int>(center-support), (rspf_int32)inputUl.x);
         stop=rspf::min((rspf_int32)rspf::round<int>(center+support), (rspf_int32)inputLr.x);
         rspf_int32 delta = stop-start;
         if (delta <= 0)
         {
            break;
         }
         vector<double> kernel(delta);
         double density=0.0;
         
         for(kernelIdx = 0; kernelIdx < delta; ++kernelIdx)
         {
            double t = scale*(start + kernelIdx -
                              center + .5);
            kernel[kernelIdx] = filter->filter(t,
                                               filter->getSupport());
            density += kernel[kernelIdx];
         }
         if ((density != 0.0) && (density != 1.0))
         {
            /*
              Normalize.
            */
            density=1.0/density;
            for (kernelIdx=0; kernelIdx < delta; kernelIdx++)
               kernel[kernelIdx]*=density;
         }
         rspf_int32 offset       = start  - imageOrigin.x;
         
         T* xptr         = imageBuf + offset;
         T* xCenterptr   = imageBuf + offset;
         T* outptr       = viewBuf  + x;
         
         for(y = 0; y < vh; ++y)
         {
            double result = 0.0;
            density = 0.0;
            if((*xCenterptr) == np)
            {
               *outptr = outNp;
            }
            else
            {
               for(kernelIdx = 0; kernelIdx < (int)kernel.size(); ++kernelIdx)
               {
                  if((*xptr != np)&&
                     (kernel[kernelIdx] != 0.0))
                  {
                     result  += ((double)(*(xptr+kernelIdx))*kernel[kernelIdx]);
                     density += kernel[kernelIdx];
                  }
               }
               if(density != 0.0)
               {
                  result /= density;
                  
                  if(result < outMinPix) result = outMinPix;
                  if(result > outMaxPix) result = outMaxPix;
                  
                  *outptr = (T)result;
               }
               else
               {
                  *outptr = outNp;
               }
            }
            xCenterptr += iw;
            xptr   += iw;
            outptr += vw;
         }
      }
   }
}

//**************************************************************************************************
template <class T> void rspfScaleFilter::runVerticalFilterTemplate(
   T /* dummy */,
   const rspfRefPtr<rspfImageData>& input,
   rspfRefPtr<rspfImageData>& output)
{
   rspfIrect viewRect  = output->getImageRectangle();
   rspfIrect imageRect = input->getImageRectangle();
   rspf_int32 vw = viewRect.width();
   rspf_int32 vh = viewRect.height();
   rspf_int32 iw = imageRect.width();
   rspfIpt origin(viewRect.ul());
   rspfIpt imageOrigin(imageRect.ul());
   rspfIpt inputUl = m_InputRect.ul();
   rspfIpt inputLr = m_InputRect.lr();
   double scale = 0.0;
   double support = 0.0;
   rspf_int32 x = 0;
   rspf_int32 y = 0;
   rspf_int32 start = 0;
   rspf_int32 stop  = 0;
   rspf_int32 kernelIdx = 0;
   const rspfFilter* filter = getVerticalFilter();
   rspf_float64 center = 0.0;
   rspf_int32 bandIdx = 0;
   rspf_int32 numberOfBands = m_Tile->getNumberOfBands();
   
   scale = m_BlurFactor*rspf::max(1.0/m_ScaleFactor.y, 1.0);
   
   support=scale*filter->getSupport();
   if (support <= 0.5)
   {
      support = .5 + FLT_EPSILON;
      scale = 1.0;
   }
   scale=1.0/scale;

   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* imageBuf = (T*)input->getBuf(bandIdx);
      T* viewBuf  = (T*)output->getBuf(bandIdx);
      T np        = (T)input->getNullPix(bandIdx);
      T outNp     = (T)output->getNullPix(bandIdx);
      T outMinPix = (T)output->getMinPix(bandIdx);
      T outMaxPix = (T)output->getMaxPix(bandIdx);
     
      for(y = 0; y < vh; ++y)
      {
         center=(double) ((y + origin.y+0.5)/m_ScaleFactor.y);
         start=rspf::max((rspf_int32)rspf::round<int>(center-support), (rspf_int32)inputUl.y);
         stop=rspf::min((rspf_int32)rspf::round<int>(center+support), (rspf_int32)inputLr.y);
         rspf_int32 delta = stop-start;
         if (delta <= 0)
         {
            break;
         }
         vector<double> kernel(delta);
         double density = 0.0;
         for(kernelIdx = 0; kernelIdx < delta; ++kernelIdx)
         {
            kernel[kernelIdx] = filter->filter(scale*(start + kernelIdx - center + .5),
                                               filter->getSupport());
            density += kernel[kernelIdx];
         }
         if ((density != 0.0) && (density != 1.0))
         {
            /*
              Normalize.
            */
            density=1.0/density;
            for (kernelIdx=0; kernelIdx < delta; kernelIdx++)
               kernel[kernelIdx]*=density;
         }

         rspf_int32 offset       = ((start  - imageOrigin.y)*iw);
         rspf_int32 offsetCenter = ((((rspf_int32)center) - imageOrigin.y)*iw);
        
         for(x = 0; x < vw; ++x)
         {
            T* yptr         = imageBuf + offset       + x;
            T* yCenterptr   = imageBuf + offsetCenter + x;
            double result = 0.0;
            density = 0.0;

            if((*yCenterptr) == np)
            {
               *viewBuf = outNp;
            }
            else
            {
               for(kernelIdx = 0; kernelIdx < delta; ++kernelIdx)
               {
                  if((*yptr != np)&&
                     (kernel[kernelIdx] != 0.0))
                  {
                     result  += ((*yptr)*kernel[kernelIdx]);
                     density += kernel[kernelIdx];
                  }
                  yptr += iw;
               }
               if(density != 0.0)
               {
                  result /= density;
                 
                  if(result < outMinPix) result = outMinPix;
                  if(result > outMaxPix) result = outMaxPix;
                 
                  *viewBuf = (T)result;
               }
               else
               {
                  *viewBuf = outNp;
               }
            }
            ++viewBuf;
         }
      }
   }
}

//**************************************************************************************************
void rspfScaleFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   // Force an allocate next getTile.
   m_Tile = NULL;
   m_BlankTile = NULL;
   m_InputRect.makeNan();
}

//**************************************************************************************************
void rspfScaleFilter::allocate()
{
   m_Tile      = NULL;
   m_BlankTile = NULL;
   m_InputRect.makeNan();

   if(theInputConnection&&isSourceEnabled())
   {
      m_Tile      = rspfImageDataFactory::instance()->create(this, this);
      m_BlankTile = rspfImageDataFactory::instance()->create(this, this);
      
      m_Tile->initialize();

      m_InputRect = theInputConnection->getBoundingRect();
   }
}

//**************************************************************************************************
// Returns a pointer reference to the active image geometry at this filter. The input source
// geometry is modified, so we need to maintain our own geometry object as a data member.
//**************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfScaleFilter::getImageGeometry()
{
   // Have we already defined our own geometry? Return it if so:
   if (m_ScaledGeometry.valid()) return m_ScaledGeometry;

   // Otherwise we'll need to establish a geometry based on the input connection:
   if(theInputConnection)
   {
      // Fetch the map projection of the input image if it exists:
      rspfRefPtr<rspfImageGeometry> inputGeom = theInputConnection->getImageGeometry();

      // If trivial case of identity scale, just pass along the input connection's geometry:
      if ((m_ScaleFactor.x == 1.0) && (m_ScaleFactor.y == 1.0))
         return inputGeom;

      // Need to create a copy of the input geom and modify it as our own, then pass that:
      if ( inputGeom.valid() )
      {
         m_ScaledGeometry = new rspfImageGeometry(*inputGeom);
         updateGeometry();

         // Return the modified geometry:
         return m_ScaledGeometry;
      }
   }

   // No geometry defined, return NULL pointer:
   return rspfRefPtr<rspfImageGeometry>();
}

//**************************************************************************************************
rspfIrect rspfScaleFilter::scaleRect(const rspfIrect input,
                                       const rspfDpt& scaleFactor)const
{
   rspfIpt origin(rspf::round<int>(input.ul().x*scaleFactor.x),
                   rspf::round<int>(input.ul().y*scaleFactor.y));
   rspf_int32 w = rspf::round<int>(input.width()*scaleFactor.x);
   rspf_int32 h = rspf::round<int>(input.height()*scaleFactor.y);

   if(w < 1) w = 1;
   if(h < 1) h = 1;
   
   return rspfIrect(origin.x,
                     origin.y,
                     origin.x + (w-1),
                     origin.y + (h-1));
}

//**************************************************************************************************
rspfString rspfScaleFilter::getFilterTypeAsString(rspfScaleFilterType type)const
{
   switch(type)
   {
   case rspfScaleFilter_NEAREST_NEIGHBOR:
   {
      return "nearest_neighbor";
   }
   case rspfScaleFilter_BOX:
   {
      return "box";
   }
   case rspfScaleFilter_GAUSSIAN:
   {
      return "gaussian";
   }
   case rspfScaleFilter_CUBIC:
   {
      return "cubic";
   }
   case rspfScaleFilter_HANNING:
   {
      return "hanning";
   }
   case rspfScaleFilter_HAMMING:
   {
      return "hamming";
   }
   case rspfScaleFilter_LANCZOS:
   {
      return "lanczos";
   }
   case rspfScaleFilter_MITCHELL:
   {
      return "mitchell";
   }
   case rspfScaleFilter_CATROM:
   {
      return "catrom";
   }
   case rspfScaleFilter_BLACKMAN:
   {
      return "blackman";
   }
   case rspfScaleFilter_BLACKMAN_SINC:
   {
      return "blackman_sinc";
   }
   case rspfScaleFilter_BLACKMAN_BESSEL:
   {
      return "blackman_bessel";
   }
   case rspfScaleFilter_QUADRATIC:
   {
      return "quadratic";
   }
   case rspfScaleFilter_TRIANGLE:
   {
      return "triangle";
   }
   case rspfScaleFilter_HERMITE:
   {
      return "hermite";
   }
   }

   return "nearest_neighbor";
}

//**************************************************************************************************
rspfScaleFilter::rspfScaleFilterType rspfScaleFilter::getFilterType(const rspfString& type)const
{
   rspfString typeUpper = type;
   typeUpper = typeUpper.upcase();

   if(typeUpper.contains("BOX"))
   {
      return rspfScaleFilter_BOX;
   }
   else if(typeUpper.contains("NEAREST_NEIGHBOR"))
   {
      return rspfScaleFilter_NEAREST_NEIGHBOR;
   }
   else if(typeUpper.contains("GAUSSIAN"))
   {
      return rspfScaleFilter_GAUSSIAN;
   }
   else if(typeUpper.contains("HANNING"))
   {
      return rspfScaleFilter_HANNING;
   }
   else if(typeUpper.contains("HAMMING"))
   {
      return rspfScaleFilter_HAMMING;
   }
   else if(typeUpper.contains("LANCZOS"))
   {
      return rspfScaleFilter_LANCZOS;
   }
   else if(typeUpper.contains("MITCHELL"))
   {
      return rspfScaleFilter_MITCHELL;
   }
   else if(typeUpper.contains("CATROM"))
   {
      return rspfScaleFilter_CATROM;
   }
   else if(typeUpper.contains("CUBIC"))
   {
      return rspfScaleFilter_CUBIC;
   }
   else if(typeUpper.contains("BLACKMAN_BESSEL"))
   {
      return rspfScaleFilter_BLACKMAN_BESSEL;
   }
   else if(typeUpper.contains("BLACKMAN_SINC"))
   {
      return rspfScaleFilter_BLACKMAN_SINC;
   }
   else if(typeUpper.contains("BLACKMAN"))
   {
      return rspfScaleFilter_BLACKMAN;
   }
   else if(typeUpper.contains("QUADRATIC"))
   {
      return rspfScaleFilter_QUADRATIC;
   }
   else if(typeUpper.contains("TRIANGLE"))
   {
      return rspfScaleFilter_TRIANGLE;
   }
   else if(typeUpper.contains("HERMITE"))
   {
      return rspfScaleFilter_HERMITE;
   }
   
   return rspfScaleFilter_NEAREST_NEIGHBOR;
}

//**************************************************************************************************
void rspfScaleFilter::getSupport(double& x, double& y)
{
   const rspfFilter* horizontalFilter = getHorizontalFilter();
   const rspfFilter* verticalFilter   = getVerticalFilter();
   
   x = m_BlurFactor*rspf::max(1.0/m_ScaleFactor.x, 1.0)*
       horizontalFilter->getSupport();
   y = m_BlurFactor*rspf::max(1.0/m_ScaleFactor.y, 1.0)*
       verticalFilter->getSupport();
}

//**************************************************************************************************
const rspfFilter* rspfScaleFilter::getHorizontalFilter()const
{
   if(m_ScaleFactor.x < 1)
   {
      return m_MinifyFilter;
   }

   return m_MagnifyFilter;
}

//**************************************************************************************************
const rspfFilter* rspfScaleFilter::getVerticalFilter()const
{
   if(m_ScaleFactor.y < 1)
   {
      return m_MinifyFilter;
   }

   return m_MagnifyFilter;
}



//**************************************************************************************************
bool rspfScaleFilter::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::SCALE_X_KW,
           m_ScaleFactor.x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::SCALE_Y_KW,
           m_ScaleFactor.y,
           true);
   kwl.add(prefix,
           "minify_type",
           getFilterTypeAsString(m_MinifyFilterType),
           true);
   kwl.add(prefix,
           "magnify_type",
           getFilterTypeAsString(m_MagnifyFilterType),
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

//**************************************************************************************************
bool rspfScaleFilter::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   rspfString scalex  = kwl.find(prefix,
                                 rspfKeywordNames::SCALE_X_KW);
   rspfString scaley  = kwl.find(prefix,
                                  rspfKeywordNames::SCALE_Y_KW);
   rspfString minify  = kwl.find(prefix,
                                  "minify_type");
   rspfString magnify = kwl.find(prefix,
                                  "magnify_type");
   
   m_ScaleFactor.x = scalex.toDouble();
   m_ScaleFactor.y = scaley.toDouble();

   if(fabs(m_ScaleFactor.x) <= FLT_EPSILON)
   {
      m_ScaleFactor.x = 1.0;
   }
   if(fabs(m_ScaleFactor.y) <= FLT_EPSILON)
   {
      m_ScaleFactor.y = 1.0;
   }

   m_InverseScaleFactor.x = 1.0/m_ScaleFactor.x;
   m_InverseScaleFactor.y = 1.0/m_ScaleFactor.y;
   
   setFilterType(getFilterType(minify),
                 getFilterType(magnify));
   
   // A change in the scale factor implies a change to the image geometry. If one has been created
   // it needs to be modified:
   updateGeometry();

   return rspfImageSourceFilter::loadState(kwl, prefix);
}

//**************************************************************************************************
//! If this object is maintaining an rspfImageGeometry, this method needs to be called after 
//! a scale change so that the geometry's projection is modified accordingly.
//**************************************************************************************************
void rspfScaleFilter::updateGeometry()
{
   if (m_ScaledGeometry.valid())
   {
      // Modify the image geometry's projection with the scale factor before returning geom:
      rspfProjection* proj = m_ScaledGeometry->getProjection();
      rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, proj);
      if(mapProj)
         mapProj->applyScale(m_InverseScaleFactor, true);
   }
}
