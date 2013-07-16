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
// $Id: rspfImageSharpenFilter.cpp 18021 2010-09-01 12:11:05Z gpotts $
#include <cstdlib> /* for abs() */
#include <rspf/imaging/rspfImageSharpenFilter.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfNumericProperty.h>

RTTI_DEF1(rspfImageSharpenFilter, "rspfImageSharpenFilter", rspfImageSourceFilter);

static const char* KERNEL_WIDTH_KW = "kernel_width";
static const char* KERNEL_SIGMA_KW = "kernel_sigma";

rspfImageSharpenFilter::rspfImageSharpenFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
   theWidth(3),
   theSigma(.5)
{
   theConvolutionSource = new rspfConvolutionSource;
}

rspfImageSharpenFilter::~rspfImageSharpenFilter()
{
}

void rspfImageSharpenFilter::setWidthAndSigma(rspf_uint32 w, rspf_float64 sigma)
{
   theWidth = w;
   theWidth |= 1;
   if(theWidth < 3) theWidth = 3;
   theSigma = sigma;
   buildConvolutionMatrix();
}

rspfRefPtr<rspfImageData> rspfImageSharpenFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;
   }
   if(!isSourceEnabled())
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }
   return theConvolutionSource->getTile(tileRect, resLevel);   
}

void rspfImageSharpenFilter::initialize()
{
   rspfImageSourceFilter::initialize();
   if(theConvolutionSource->getInput()!=getInput())
   {
      theConvolutionSource->disconnectAllInputs();
      theConvolutionSource->connectMyInputTo(0, getInput());
   }
   buildConvolutionMatrix();   
}

void rspfImageSharpenFilter::buildConvolutionMatrix()
{
   std::vector<double> theKernel(theWidth*theWidth);
   double* kernel = &theKernel.front();
   //width = findWidth(sigma);
   rspf_int32 i = 0 ;
   rspf_int32 u = 0 ;
   rspf_int32 v = 0 ;
   double normalize = 0.0 ;
   rspf_int32 w2 = theWidth>>1;
   for (v = -w2; v <= w2; ++v)
   {
      for (u = -w2; u <= w2; ++u)
      {
         double value = laplacianOfGaussian(u, v, theSigma);
         kernel[i] = value;
         normalize += kernel[i] ;
         ++i ;
      }
   }
   if (fabs(normalize) <= 1e-6)
   {
      normalize = 1.0;
   }
   normalize=(1.0/normalize);
   rspf_int32 size = static_cast<rspf_int32>(theWidth*theWidth);
   for (i = 0; i < size; ++i)
   {
      kernel[i]=kernel[i]*normalize;
   } 
      
#if 0
   // print the kernel
   for (i = 0; i < theWidth*theWidth; ++i)
   {
      if((i%theWidth)==0)
      {
         std::cout << std::endl;
      }
      std::cout << kernel[i] <<", ";
   } 
   std::cout << std::endl;
#endif
   theConvolutionSource->setConvolution(kernel, theWidth, theWidth, false);
}

void rspfImageSharpenFilter::connectInputEvent(rspfConnectionEvent &event)
{
   rspfImageSourceFilter::connectInputEvent(event);
   theConvolutionSource->connectMyInputTo(0, getInput());
   buildConvolutionMatrix();
}

void rspfImageSharpenFilter::disconnectInputEvent(rspfConnectionEvent &event)
{
   rspfImageSourceFilter::disconnectInputEvent(event);
   theConvolutionSource->connectMyInputTo(0, getInput());
}

rspfString rspfImageSharpenFilter::getShortName() const
{
   return rspfString("Sharpen");
}

rspfString rspfImageSharpenFilter::getLongName() const
{
   return rspfString("Sharpens the input");
}

void rspfImageSharpenFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   //std::cout << "rspfImageSharpenFilter::setProperty with name = " << property->getName() << std::endl;
   if(!property) return;
   if(property->getName() == KERNEL_WIDTH_KW)
   {
      theWidth = property->valueToString().toUInt32();
      theWidth |=1;
      if(theWidth < 3) theWidth = 3;
      initialize();
   }
   else if(property->getName() == KERNEL_SIGMA_KW)
   {
      theSigma = property->valueToString().toDouble();
      if(theSigma < .1) theSigma = .1;
      initialize();
   }
   else
   {
      rspfImageSourceFilter::setProperty(property.get());
   }
}

rspfRefPtr<rspfProperty> rspfImageSharpenFilter::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> property = 0;
   if(name == KERNEL_WIDTH_KW)
   {
      property = new rspfNumericProperty(name,
                                          rspfString::toString(theWidth),
                                          3.0,
                                          64.0);
      property->setCacheRefreshBit();
      
   }
   else if(name == KERNEL_SIGMA_KW)
   {
      property = new rspfNumericProperty(name,
                                          rspfString::toString(theSigma),
                                          .1,
                                          32);
      property->setCacheRefreshBit();
   }
   else 
   {
      property = rspfImageSourceFilter::getProperty(name);
   }
   
   return property.get();
}

void rspfImageSharpenFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(KERNEL_WIDTH_KW);
   propertyNames.push_back(KERNEL_SIGMA_KW);
}

bool rspfImageSharpenFilter::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   bool result = rspfImageSourceFilter::loadState(kwl, prefix);
   rspfString kernelWidth = kwl.find(prefix, KERNEL_WIDTH_KW);
   rspfString kernelSigma = kwl.find(prefix, KERNEL_SIGMA_KW);
   kernelWidth = kernelWidth.trim();
   kernelSigma = kernelSigma.trim();
   if(!kernelWidth.empty())
   {
      theWidth = kernelWidth.toUInt32();
      theWidth |= 1; // make it odd and check for size
      if(theWidth < 3) theWidth = 3;
   }
   if(!kernelSigma.empty())
   {
      theSigma = kernelSigma.toFloat64();
   }
   buildConvolutionMatrix();
   return result;
}

bool rspfImageSharpenFilter::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   kwl.add(prefix, KERNEL_WIDTH_KW, theWidth, true);
   kwl.add(prefix, KERNEL_SIGMA_KW, theSigma, true);
   return rspfImageSourceFilter::saveState(kwl, prefix);
}
