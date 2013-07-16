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
// $Id: rspfImageToPlaneNormalFilter.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfImageToPlaneNormalFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfBooleanProperty.h>

static const char* SMOOTHNESS_FACTOR_KW="smoothness_factor";

RTTI_DEF1(rspfImageToPlaneNormalFilter, "rspfImageToPlaneNormalFilter", rspfImageSourceFilter);

rspfImageToPlaneNormalFilter::rspfImageToPlaneNormalFilter()
   :rspfImageSourceFilter(),
    theTile(NULL),
    theBlankTile(NULL),
    theTrackScaleFlag(true),
    theXScale(1.0),
    theYScale(1.0),
    theSmoothnessFactor(1.0)
{
}

rspfImageToPlaneNormalFilter::rspfImageToPlaneNormalFilter(rspfImageSource* inputSource)
   :rspfImageSourceFilter(inputSource),
    theTile(NULL),
    theBlankTile(NULL),
    theTrackScaleFlag(true),
    theXScale(1.0),
    theYScale(1.0),
    theSmoothnessFactor(1.0)
{
}

rspfRefPtr<rspfImageData> rspfImageToPlaneNormalFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!isSourceEnabled()||!theInputConnection)
   {
      return rspfImageSourceFilter::getTile(tileRect, resLevel);
   }

   if(!theTile.valid())
   {
      initialize();
   }

   if(!theTile.valid())
   {
      return rspfImageSourceFilter::getTile(tileRect, resLevel);
   }

   theTile->setImageRectangle(tileRect);

   rspfIrect requestRect(tileRect.ul().x - 1,
                          tileRect.ul().y - 1,
                          tileRect.lr().x + 1,
                          tileRect.lr().y + 1);

   rspfRefPtr<rspfImageData> input =
      theInputConnection->getTile(requestRect, resLevel);

   if(!input||(input->getDataObjectStatus()==RSPF_EMPTY)||!input->getBuf())
   {
      if(tileRect.completely_within(theInputBounds))
      {
         initializeTile();
         theTile->validate();
         return theTile.get();
      }
      theBlankTile->setImageRectangle(tileRect);
      return theBlankTile;
   }

   double oldScaleX = theXScale;
   double oldScaleY = theYScale;

   if(resLevel > 0)
   {
      rspfDpt scaleFactor;
      theInputConnection->getDecimationFactor(resLevel, scaleFactor);

      if(!scaleFactor.hasNans())
      {
         theXScale *= scaleFactor.x;
         theYScale *= scaleFactor.y;
      }
   }

   computeNormals(input, theTile);

   theXScale = oldScaleX;
   theYScale = oldScaleY;

   theTile->validate();

   return theTile;
}

void rspfImageToPlaneNormalFilter::initializeTile()
{
   double* x = static_cast<double*>(theTile->getBuf(0));
   double* y = static_cast<double*>(theTile->getBuf(1));
   double* z = static_cast<double*>(theTile->getBuf(2));
   
   if(x) std::fill(x, x+theTile->getSizePerBand(), 0.0);
   if(y) std::fill(y, y+theTile->getSizePerBand(), 0.0);
   if(z) std::fill(z, z+theTile->getSizePerBand(), 1.0);
}

void rspfImageToPlaneNormalFilter::initialize()
{
   if(theInputConnection)
   {
      theInputConnection->initialize();
      
      theInputBounds = theInputConnection->getBoundingRect();
      theTile      = rspfImageDataFactory::instance()->create(this, this);
      theBlankTile = (rspfImageData*)(theTile->dup());
      theTile->initialize();

      if(theTrackScaleFlag)
      {
         rspfRefPtr<rspfImageGeometry> geom = theInputConnection->getImageGeometry();
         if( geom.valid() )
         {
            rspfDpt pt = geom->getMetersPerPixel();
            if(!pt.hasNans())
            {
               theXScale = 1.0/pt.x;
               theYScale = 1.0/pt.y;
            }
         }
      }
   }
}

void rspfImageToPlaneNormalFilter::computeNormals(
   rspfRefPtr<rspfImageData>& inputTile,
   rspfRefPtr<rspfImageData>& outputTile)
{
   switch(inputTile->getScalarType())
   {
      case RSPF_SSHORT16:
      {
         computeNormalsTemplate((rspf_sint16)0,
                                inputTile,
                                outputTile);
         break;
      }
      case RSPF_UCHAR:
      {
         computeNormalsTemplate((rspf_uint8)0,
                                inputTile,
                                outputTile);
         break;
      }
      case RSPF_USHORT11:
      case RSPF_USHORT16:
      {
         computeNormalsTemplate((rspf_uint16)0,
                                inputTile,
                                outputTile);
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_DOUBLE:
      {
         computeNormalsTemplate((rspf_float64)0,
                                inputTile,
                                outputTile);
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT:
      {
         computeNormalsTemplate((rspf_float32)0,
                                inputTile,
                                outputTile);
         break;
      }
      default:
         break;
   }
}

template <class T> void rspfImageToPlaneNormalFilter::computeNormalsTemplate(
   T /* inputScalarTypeDummy */,
   rspfRefPtr<rspfImageData>& inputTile,
   rspfRefPtr<rspfImageData>& outputTile)
{
   T inputNull = (T)inputTile->getNullPix(0);
   T* inbuf = (T*)inputTile->getBuf();

   double* normX = (double*)outputTile->getBuf(0);
   double* normY = (double*)outputTile->getBuf(1);
   double* normZ = (double*)outputTile->getBuf(2);
   rspf_int32 inbuf_width = inputTile->getWidth();
   rspf_int32 normbuf_width = outputTile->getWidth();
   rspf_int32 normbuf_height = outputTile->getHeight();
   rspfColumnVector3d normal;

   for (rspf_int32 y=0; y<normbuf_height; y++)
   {
      // Establish offsets into the image and output normals buffers given row:
      rspf_uint32 n = y*normbuf_width;
      rspf_uint32 i = (y+1)*inbuf_width + 1;
      
      // Loop to compute the gradient (normal) vector [dh/dx, dh/dy, 1]:
      for (rspf_int32 x=0; x<normbuf_width; x++)
      {
         // Default in case of null inputs is a flat earth:
         normal[0] = 0;
         normal[1] = 0;
         normal[2] = 1.0;

         // Compute the x-direction differential:
         if (inbuf[i+1] != inputNull)
         {
            if (inbuf[i-1] != inputNull)
               normal[0] = theXScale*theSmoothnessFactor*(inbuf[i+1] - inbuf[i-1]) / 2.0;
            else if (inbuf[i] != inputNull)
               normal[0] = theXScale*theSmoothnessFactor*(inbuf[i+1] - inbuf[i]);
         }
         else if ((inbuf[i] != inputNull) && (inbuf[i-1] != inputNull))
         {
            normal[0] = theXScale*theSmoothnessFactor*(inbuf[i] - inbuf[i-1]);
         }

         // Compute the y-direction differential:
         if (inbuf[i+inbuf_width] != inputNull)
         {
            if (inbuf[i-inbuf_width] != inputNull)
               normal[1] = theYScale*theSmoothnessFactor*(inbuf[i+inbuf_width] - inbuf[i-inbuf_width]) / 2.0;
            else if (inbuf[i] != inputNull)
               normal[1] = theYScale*theSmoothnessFactor*(inbuf[i+inbuf_width] - inbuf[i]);
         }
         else if ((inbuf[i] != inputNull) && (inbuf[i-inbuf_width] != inputNull))
         {
            normal[1] = theYScale*theSmoothnessFactor*(inbuf[i] - inbuf[i-inbuf_width]);
         }

         // Stuff the normalized gradient vector into the output buffers:
         normal = normal.unit();
         normX[n] = normal[0];
         normY[n] = normal[1];
         normZ[n] = normal[2];
         
         ++n;
         ++i;
      }
   }
}

bool rspfImageToPlaneNormalFilter::loadState(const rspfKeywordlist& kwl,
                                              const char* prefix)
{
   rspfString scaleX     = kwl.find(prefix, rspfKeywordNames::SCALE_PER_PIXEL_X_KW);
   rspfString scaleY     = kwl.find(prefix, rspfKeywordNames::SCALE_PER_PIXEL_Y_KW);
   rspfString trackFlag  = kwl.find(prefix, "track_scale_flag");
   rspfString smoothness = kwl.find(prefix, SMOOTHNESS_FACTOR_KW);

   if(scaleX != "")
   {
      theXScale = scaleX.toDouble();
   }
   if(scaleY != "")
   {
      theYScale = scaleY.toDouble();
   }
   if(trackFlag != "")
   {
      theTrackScaleFlag = trackFlag.toBool();
   }
   if(smoothness!="")
   {
      theSmoothnessFactor = smoothness.toDouble();
   }

   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfImageToPlaneNormalFilter::saveState(rspfKeywordlist& kwl,
                                              const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::SCALE_PER_PIXEL_X_KW,
           theXScale,
           true);

   kwl.add(prefix,
           rspfKeywordNames::SCALE_PER_PIXEL_Y_KW,
           theXScale,
           true);

   kwl.add(prefix,
           "track_scale_flag",
           (rspf_uint32)theTrackScaleFlag,
           true);

   kwl.add(prefix,
           SMOOTHNESS_FACTOR_KW,
           theSmoothnessFactor,
           true);

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

double rspfImageToPlaneNormalFilter::getMinPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getMinPixelValue(band);
   }
   return -1;
}

double rspfImageToPlaneNormalFilter::getMaxPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getMaxPixelValue(band);
   }
   return 1.0;
}

rspfScalarType rspfImageToPlaneNormalFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      return RSPF_DOUBLE;
   }
   
   return rspfImageSourceFilter::getOutputScalarType();
}

rspf_uint32 rspfImageToPlaneNormalFilter::getNumberOfOutputBands() const
{
   if(isSourceEnabled())
   {
      return 3;
   }
   return rspfImageSourceFilter::getNumberOfOutputBands();
}

void rspfImageToPlaneNormalFilter::setXScale(const double& scale)
{
   theXScale = scale;
}

double rspfImageToPlaneNormalFilter::getXScale()const
{
   return theXScale;
}

double rspfImageToPlaneNormalFilter::getYScale()const
{
   return theYScale;
}

void rspfImageToPlaneNormalFilter::setYScale(const double& scale)
{
   theYScale = scale;
}

void rspfImageToPlaneNormalFilter::setTrackScaleFlag(bool flag)
{
   theTrackScaleFlag = flag;
}

bool rspfImageToPlaneNormalFilter::getTrackScaleFlag()const
{
   return theTrackScaleFlag;
}

void rspfImageToPlaneNormalFilter::setSmoothnessFactor(double value)
{
   theSmoothnessFactor = value;
}

double rspfImageToPlaneNormalFilter::getSmoothnessFactor()const
{
   return theSmoothnessFactor;
}

void rspfImageToPlaneNormalFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfString name = property->getName();
   if((name == "smoothnessFactor")||
      (name == "gain"))
   {
      theSmoothnessFactor = property->valueToString().toDouble();
      initialize();
   }
   else if(name == "xscale")
   {
      theXScale = property->valueToString().toDouble();
      initialize();
   }
   else if(name == "yscale")
   {
      theYScale = property->valueToString().toDouble();
      initialize();
   }
   else if(name == "autoTrackScaleFlag")
   {
      theTrackScaleFlag = property->valueToString().toDouble();
      initialize();
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfImageToPlaneNormalFilter::getProperty(const rspfString& name)const
{
   if((name == "smoothnessFactor")||
      (name == "gain"))
   {
      rspfNumericProperty* prop = new rspfNumericProperty("gain", rspfString::toString(theSmoothnessFactor), .0001, 40);
      prop->setCacheRefreshBit();
      return prop;
   }
   else if(name == "xscale")
   {
      rspfNumericProperty* prop = new rspfNumericProperty(name, rspfString::toString(theXScale), .0001, 50000);
      prop->setCacheRefreshBit();
      return prop;
   }
   else if(name == "yscale")
   {
      rspfNumericProperty* prop = new rspfNumericProperty(name, rspfString::toString(theYScale), .0001, 50000);
      prop->setCacheRefreshBit();
      return prop;
   }
   else if(name == "autoTrackScaleFlag")
   {
      rspfBooleanProperty* prop = new rspfBooleanProperty(name, theTrackScaleFlag);
      prop->setCacheRefreshBit();
      return prop;
   }
   
   return rspfImageSourceFilter::getProperty(name);
}

void rspfImageToPlaneNormalFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("gain");
   propertyNames.push_back("xscale");
   propertyNames.push_back("yscale");
   propertyNames.push_back("autoTrackScaleFlag");
}
