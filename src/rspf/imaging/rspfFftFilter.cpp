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
//  $Id: rspfFftFilter.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <rspf/imaging/rspfFftFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/base/rspfStringProperty.h>

RTTI_DEF1(rspfFftFilter, "rspfFftFilter", rspfImageSourceFilter);

rspfFftFilter::rspfFftFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theTile(0),
    theDirectionType(rspfFftFilterDirectionType_FORWARD),
    theScalarRemapper(new rspfScalarRemapper())
{
   theScalarRemapper->setOutputScalarType(RSPF_DOUBLE);
}

rspfFftFilter::rspfFftFilter(rspfImageSource* inputSource)
   :rspfImageSourceFilter(inputSource),
    theTile(0),
    theDirectionType(rspfFftFilterDirectionType_FORWARD),
    theScalarRemapper(new rspfScalarRemapper())
{
   theScalarRemapper->setOutputScalarType(RSPF_DOUBLE);
}

rspfFftFilter::rspfFftFilter(rspfObject* owner,
                               rspfImageSource* inputSource)
   :rspfImageSourceFilter(owner, inputSource),
    theTile(0),
    theDirectionType(rspfFftFilterDirectionType_FORWARD),
    theScalarRemapper(new rspfScalarRemapper())
{
   theScalarRemapper->setOutputScalarType(RSPF_DOUBLE);
}

rspfFftFilter::~rspfFftFilter()
{
   if(theScalarRemapper.valid())
   {
      theScalarRemapper->disconnect();
      theScalarRemapper = 0;
   }
}

rspfRefPtr<rspfImageData> rspfFftFilter::getTile(const rspfIrect& rect,
                                                    rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(rect, resLevel);
   }
   
   rspfIrect tempRequest = rect;

   rspf_uint32 w = rect.width();
   rspf_uint32 h = rect.height();
   
   if(w & 1)
   {
      ++w;
   }
   if(h&1)
   {
      ++h;
   }

   tempRequest = rspfIrect(rect.ul().x,
                            rect.ul().y,
                            rect.ul().x + (w-1),
                            rect.ul().y + (h-1));
   
   rspfRefPtr<rspfImageData> input;
   input  = theScalarRemapper->getTile(tempRequest, resLevel);

   if(!input.valid()) return input;
   
   if(!theTile.valid()) initialize();
   if(!theTile.valid()) return theTile;
   
   theTile->setImageRectangle(rect);
   rspfRefPtr<rspfImageData> tempTile = theTile;
   
   if(!input->getBuf())
   {
      return theTile;
   }

   if(rect != tempRequest)
   {
      tempTile = (rspfImageData*)theTile->dup();
      tempTile->setImageRectangle(tempRequest);
   }

   runFft((rspf_float64)0, input, tempTile);
          
   if(tempTile != theTile)
   {
      theTile->loadTile(tempTile.get());
   }
   
   theTile->validate();

   return theTile;
}

void rspfFftFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   theTile = rspfImageDataFactory::instance()->create(this, this);
   
   if(theTile.valid())
   {
      theTile->initialize();
   }
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      theScalarRemapper->setOutputScalarType(RSPF_NORMALIZED_DOUBLE);
   }
   else
   {
      theScalarRemapper->setOutputScalarType(RSPF_DOUBLE);
   }
   theScalarRemapper->connectMyInputTo(0, getInput());
}

rspfScalarType rspfFftFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getOutputScalarType();
   }
   
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      return RSPF_DOUBLE;
   }
   
   return RSPF_NORMALIZED_DOUBLE;
}

double rspfFftFilter::getNullPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getNullPixelValue(band);
   }
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      return rspf::nan();
   }

   // it will invert to a normalized float output
   return 0.0;
}

double rspfFftFilter::getMinPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getMinPixelValue(band);
   }
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      return RSPF_DEFAULT_MIN_PIX_DOUBLE;
   }
   return RSPF_DEFAULT_MIN_PIX_NORM_DOUBLE;
}

double rspfFftFilter::getMaxPixelValue(rspf_uint32 band)const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getMaxPixelValue(band);
   }
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      return RSPF_DEFAULT_MAX_PIX_DOUBLE;
   }
   return RSPF_DEFAULT_MAX_PIX_NORM_DOUBLE;
}

rspf_uint32 rspfFftFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getNumberOfOutputBands();
   }
   rspf_uint32 bands = rspfImageSourceFilter::getNumberOfOutputBands();
   
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      bands *=2;
   }
   else 
   {
      bands /=2;
   }

   return bands;
}

void rspfFftFilter::setForward()
{
   theDirectionType = rspfFftFilterDirectionType_FORWARD;
}

void rspfFftFilter::setInverse()
{
   theDirectionType = rspfFftFilterDirectionType_INVERSE;
}

rspfString rspfFftFilter::getDirectionTypeAsString()const
{
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      return "Forward";
   }

   return "Inverse";
}

void rspfFftFilter::setDirectionType(const rspfString& directionType)
{
   rspfString tempDirectionType = directionType;
   tempDirectionType = tempDirectionType.downcase();
   
   if(tempDirectionType.contains("forward"))
   {
      setDirectionType(rspfFftFilterDirectionType_FORWARD);
   }
   else
   {
      setDirectionType(rspfFftFilterDirectionType_INVERSE);
   }
}

void rspfFftFilter::setDirectionType(rspfFftFilterDirectionType directionType)
{
   theDirectionType = directionType;
   if(theTile.valid())
   {
      theTile = NULL;
   }
}

rspfRefPtr<rspfProperty> rspfFftFilter::getProperty(const rspfString& name)const
{
   if(name == "FFT Direction")
   {
      std::vector<rspfString> filterNames;
      filterNames.push_back("Forward");
      filterNames.push_back("Inverse");
      rspfStringProperty* stringProp = new rspfStringProperty("FFT Direction",
								getDirectionTypeAsString(),
								false,
								filterNames);
      stringProp->setCacheRefreshBit();

      return stringProp;
   }

   return rspfImageSourceFilter::getProperty(name);
}

void rspfFftFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;
   
   if(property->getName() == "FFT Direction")
   {
      if(theTile.valid())
      {
         theTile = NULL;
      }
      setDirectionType(property->valueToString());
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

void rspfFftFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("FFT Direction");
}


template <class T>
void rspfFftFilter::runFft(T /* dummy */,
                            rspfRefPtr<rspfImageData>& input,
                            rspfRefPtr<rspfImageData>& output)
{
   // T*             inputBand  = 0;

   NEWMAT::Matrix* realIn = new NEWMAT::Matrix(input->getHeight(),
                                               input->getWidth());
   NEWMAT::Matrix* imgIn = new NEWMAT::Matrix(input->getHeight(),
                                              input->getWidth());
   NEWMAT::Matrix* realOut = new NEWMAT::Matrix(input->getHeight(),
                                                input->getWidth());
   NEWMAT::Matrix* imgOut = new NEWMAT::Matrix(input->getHeight(),
                                               input->getWidth());
   rspf_uint32 bandIdx = 0;
   rspf_uint32 w = input->getWidth();
   rspf_uint32 h = input->getHeight();
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   if(theDirectionType == rspfFftFilterDirectionType_FORWARD)
   {
      rspf_uint32 bands = input->getNumberOfBands();
      for(bandIdx = 0; bandIdx < bands; ++bandIdx)
      {
         rspf_float64* bandReal = 0;
         rspf_float64* bandImg  = 0;
         fillMatrixForward((T*)input->getBuf(bandIdx),
                           (T)input->getNullPix(bandIdx),
                           *realIn,
                           *imgIn);
         NEWMAT::FFT2(*realIn, *imgIn, *realOut, *imgOut);
         bandReal = (rspf_float64*)output->getBuf(2*bandIdx);
         bandImg  = (rspf_float64*)output->getBuf(2*bandIdx + 1);
         if(bandReal&&bandImg)
         {
            for(y = 0; y < h; ++y)
            {
               for(x = 0; x < w; ++x)
               {
                  *bandReal = (rspf_float64)((*realOut)[y][x]);
                  *bandImg  = (rspf_float64)((*imgOut)[y][x]);
                  ++bandReal;
                  ++bandImg;
               }
            }
         }
      }
   }
   else
   {
      rspf_float64* bandReal = 0;
      rspf_uint32 bands = input->getNumberOfBands();
      for(bandIdx = 0; bandIdx < bands; bandIdx+=2)
      {
         bandReal = (rspf_float64*)output->getBuf(bandIdx/2);
         if(input->getBuf(bandIdx)&&
            input->getBuf(bandIdx+1))
         {
            fillMatrixInverse((T*)input->getBuf(bandIdx),
                              (T*)input->getBuf(bandIdx+1),
                              *realIn,
                              *imgIn);
            NEWMAT::FFT2I(*realIn, *imgIn, *realOut, *imgOut);
            for(y = 0; y < h; ++y)
            {
               for(x = 0; x < w; ++x)
               {
                  *bandReal = (rspf_float64)((*realOut)[y][x]);
                  if(*bandReal > 1.0)
                  {
                     *bandReal = 1.0;
               }
                  if(*bandReal < 0.0)
                  {
                     *bandReal = 0.0;
                  }
                  ++bandReal;
               }
            }
         }
      }
   }
   
   delete realIn;
   delete imgIn;
   delete realOut;
   delete imgOut;
}

template <class T>
void rspfFftFilter::fillMatrixForward(T *realPart,
                                       T nullPix,
                                       NEWMAT::Matrix& real,
                                       NEWMAT::Matrix& img)const
{
   rspf_uint32 w = real.Ncols();
   rspf_uint32 h = real.Nrows();
   rspf_uint32 yIdx = 0;
   rspf_uint32 xIdx = 0;
   
   for(yIdx = 0; yIdx < h; ++yIdx)
   {
      for(xIdx = 0; xIdx < w; ++xIdx)
      {
         if((double)(*realPart) != nullPix)
         {
            real[yIdx][xIdx] = (double)(*realPart);
         }
         else
         {
            real[yIdx][xIdx] = 0.0;
         }
         
         img[yIdx][xIdx]  = 0.0;
         
         ++realPart;
      }
   }
}

template <class T>
void rspfFftFilter::fillMatrixInverse(T *realPart,
                                       T *imgPart,
                                       NEWMAT::Matrix& real,
                                       NEWMAT::Matrix& img)const
{
   rspf_uint32 w = real.Ncols();
   rspf_uint32 h = real.Nrows();
   rspf_uint32 yIdx = 0;
   rspf_uint32 xIdx = 0;
   
   for(yIdx = 0; yIdx < h; ++yIdx)
   {
      for(xIdx = 0; xIdx < w; ++xIdx)
      {
         real[yIdx][xIdx] = (double)(*realPart);
         img[yIdx][xIdx]  = (double)(*imgPart);
         
         ++realPart;
         ++imgPart;
      }
   }
}


bool rspfFftFilter::loadState(const rspfKeywordlist& kwl,
                               const char* prefix)
{
   const char* direction = kwl.find(prefix, "fft_direction");
   if(direction)
   {
      setDirectionType(rspfString(direction));
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfFftFilter::saveState(rspfKeywordlist& kwl,
                               const char* prefix)const
{
   kwl.add(prefix,
           "fft_direction",
           getDirectionTypeAsString(),
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}
