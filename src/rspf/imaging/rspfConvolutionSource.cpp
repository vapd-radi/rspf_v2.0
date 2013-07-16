// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfConvolutionSource.cpp 18602 2010-12-28 05:49:36Z gpotts $

#include <rspf/imaging/rspfConvolutionSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfDiscreteConvolutionKernel.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeyword.h>

static const rspfKeyword NUMBER_OF_MATRICES = rspfKeyword("number_of_matrices", "");
static const rspfKeyword NUMBER_OF_ROWS = rspfKeyword("rows", "");
static const rspfKeyword NUMBER_OF_COLS = rspfKeyword("cols", "");

RTTI_DEF1(rspfConvolutionSource, "rspfConvolutionSource", rspfImageSourceFilter);

rspfConvolutionSource::rspfConvolutionSource()
   : rspfImageSourceFilter(),
     theTile(NULL)
{
}

rspfConvolutionSource::rspfConvolutionSource(rspfImageSource* inputSource,
                                               const NEWMAT::Matrix& convolutionMatrix)
   : rspfImageSourceFilter(inputSource),
     theTile(NULL)
{
   theConvolutionKernelList.push_back(new rspfDiscreteConvolutionKernel(convolutionMatrix));
   setKernelInformation();
   initialize();
}

rspfConvolutionSource::rspfConvolutionSource(rspfImageSource* inputSource,
                                               const vector<NEWMAT::Matrix>& convolutionList)
   : rspfImageSourceFilter(inputSource),
     theTile(NULL)
{
   setConvolutionList(convolutionList);
}

rspfConvolutionSource::~rspfConvolutionSource()
{
   deleteConvolutionList();
}

void rspfConvolutionSource::setConvolution(const double* kernel,
                                            int nrows,
                                            int ncols,
                                            bool doWeightedAverage)
{
   NEWMAT::Matrix m(nrows, ncols);
   const double* tempPtr = kernel;
   for(int row = 0; row < nrows; ++row)
   {
      for(int col = 0; col < ncols; ++col)
      {
         m[row][col] =*tempPtr;
         ++tempPtr;
      }
   }
   
   setConvolution(m, doWeightedAverage);
}

void rspfConvolutionSource::setConvolutionList(const vector<NEWMAT::Matrix>& convolutionList,
                                                bool doWeightedAverage)
{
   deleteConvolutionList();
   rspf_uint32 idx;
   for(idx = 0; idx < convolutionList.size(); ++idx)
   {
      theConvolutionKernelList.push_back(new rspfDiscreteConvolutionKernel(convolutionList[idx],
                                                                            doWeightedAverage));
   }

   setKernelInformation();
}

rspfRefPtr<rspfImageData> rspfConvolutionSource::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection) return rspfRefPtr<rspfImageData>();

   if((!isSourceEnabled())||
      (theConvolutionKernelList.size() < 1))
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }
   
   if(!theTile.valid())
   {
      allocate();
      if(!theTile.valid())  // Throw exception???
      {
         return theInputConnection->getTile(tileRect, resLevel);
      }
   }
   
   rspf_uint32 w  = tileRect.width();
   rspf_uint32 h  = tileRect.height();
   rspf_uint32 tw = theTile->getWidth();
   rspf_uint32 th = theTile->getHeight();
   theTile->setWidth(w);
   theTile->setHeight(h);
   if((w*h)!=(tw*th))
   {
      theTile->initialize();
      theTile->makeBlank();
   }
   else
   {
      theTile->makeBlank();
   }
   theTile->setOrigin(tileRect.ul());
   long offsetX = (theMaxKernelWidth)/2;
   long offsetY = (theMaxKernelHeight)/2;
   rspfIrect requestRect(tileRect.ul().x - offsetX,
                          tileRect.ul().y - offsetY,
                          tileRect.lr().x + offsetX,
                          tileRect.lr().y + offsetY);
   
   rspfRefPtr<rspfImageData> input = theInputConnection->getTile(requestRect,
                                                                   resLevel);

   if(!input.valid() ||
      (input->getDataObjectStatus() == RSPF_NULL)||
      (input->getDataObjectStatus() == RSPF_EMPTY))
   {
      return input;
   }
   switch(theTile->getScalarType())
   {
   case RSPF_UCHAR:
   {
      if(theConvolutionKernelList.size() == 1)
      {
         convolve(static_cast<rspf_uint8>(0),
                  input,
                  theConvolutionKernelList[0]);
      }
      else
      {
         rspf_uint32 upperBound = (rspf_uint32)theConvolutionKernelList.size();
         rspf_uint32 idx;
         for(idx = 0; idx < upperBound; ++idx)
         {
            convolve(static_cast<rspf_uint8>(0),
                     input,
                     theConvolutionKernelList[idx]);
            input->loadTile(theTile.get());
         }
      }
      break;
   }
   case RSPF_USHORT16:
   case RSPF_USHORT11:
   {
      if(theConvolutionKernelList.size() == 1)
      {
         convolve(static_cast<rspf_uint16>(0),
                  input,
                  theConvolutionKernelList[0]);
      }
      else
      {
         rspf_uint32 upperBound = (rspf_uint32)theConvolutionKernelList.size();
         rspf_uint32 idx;
         for(idx = 0; idx < upperBound; ++idx)
         {
            convolve(static_cast<rspf_uint16>(0),
                     input,
                     theConvolutionKernelList[idx]);
            input->loadTile(theTile.get());
         }
      }
      break;
   }
   case RSPF_SSHORT16:
   {
      if(theConvolutionKernelList.size() == 1)
      {
         convolve(static_cast<rspf_sint16>(0),
                  input,
                  theConvolutionKernelList[0]);
      }
      else
      {
         rspf_uint32 upperBound = (rspf_uint32)theConvolutionKernelList.size();
         rspf_uint32 idx;
         for(idx = 0; idx < upperBound; ++idx)
         {
            convolve(static_cast<rspf_sint16>(0),
                     input,
                     theConvolutionKernelList[idx]);
            input->loadTile(theTile.get());
         }
      }
      break;
   }
   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
   {
      if(theConvolutionKernelList.size() == 1)
      {
         convolve(static_cast<float>(0),
                  input,
                  theConvolutionKernelList[0]);
      }
      else
      {
         rspf_uint32 upperBound = (rspf_uint32)theConvolutionKernelList.size();
         rspf_uint32 idx;
         for(idx = 0; idx < upperBound; ++idx)
         {
            convolve(static_cast<float>(0),
                     input,
                     theConvolutionKernelList[idx]);
            input->loadTile(theTile.get());
         }
      }
      break;
   }
   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
   {
      if(theConvolutionKernelList.size() == 1)
      {
         convolve(static_cast<double>(0),
                  input,
                  theConvolutionKernelList[0]);
      }
      else
      {
         rspf_uint32 upperBound = (rspf_uint32)theConvolutionKernelList.size();
         rspf_uint32 idx;
         for(idx = 0; idx < upperBound; ++idx)
         {
            convolve(static_cast<double>(0),
                     input,
                     theConvolutionKernelList[idx]);
            input->loadTile(theTile.get());
         }
      }
      break;
   }
   default:
   {
      theTile->loadTile(input.get());
   }
   }
   theTile->validate();
   return theTile;
}
   
template <class T>
void rspfConvolutionSource::convolve(T /* dummyVariable */,
                                      rspfRefPtr<rspfImageData> inputTile,
                                      rspfDiscreteConvolutionKernel* kernel)
{
   rspfIpt startOrigin   = theTile->getOrigin();

   // Make sure that the patch is not empty or NULL
   //
   rspfIpt startDelta(startOrigin.x - inputTile->getOrigin().x,
                       startOrigin.y - inputTile->getOrigin().y);

   rspfDataObjectStatus status = inputTile->getDataObjectStatus();
   // let's setup some variables that we will need to do the
   // convolution algorithm.
   //
   rspfIrect patchRect   = inputTile->getImageRectangle();
   long tileHeight        = theTile->getHeight();
   long tileWidth         = theTile->getWidth();
   long outputBands       = theTile->getNumberOfBands();
   long convolutionWidth  = kernel->getWidth();
   long convolutionHeight = kernel->getHeight();
   long convolutionOffsetX= convolutionWidth/2;
   long convolutionOffsetY= convolutionHeight/2;
   long patchWidth        = patchRect.width();
   long convolutionTopLeftOffset = 0;
   long convolutionCenterOffset  = 0;
   
   long outputOffset          = 0;
   T np = 0;
   
   const double minPix  = rspf::defaultMin(getOutputScalarType());
   const double maxPix  = rspf::defaultMax(getOutputScalarType());
//   const double* maxPix  = inputTile->getMaxPix();
   const double* nullPix = inputTile->getNullPix();
   double convolveResult = 0;
   
   if(status == RSPF_PARTIAL) // must check for NULLS
   {
      for(long y = 0; y <tileHeight; y++)
      {
         convolutionCenterOffset  = patchWidth*(startDelta.y + y) + startDelta.x;
         convolutionTopLeftOffset = patchWidth*(startDelta.y + y - convolutionOffsetY) + startDelta.x-convolutionOffsetX;
         
         for(long x =0; x < tileWidth; x++)
         {
            if(!inputTile->isNull(convolutionCenterOffset))
            {
               for(long b = 0; b < outputBands; ++b)
               {
                  T* buf    = (T*)(inputTile->getBuf(b)) + convolutionTopLeftOffset;
                  T* outBuf = (T*)(theTile->getBuf(b));
                  kernel->convolveSubImage(buf,
                                           patchWidth,
                                           convolveResult,
                                           (T)nullPix[b]);

                  convolveResult = convolveResult < minPix? minPix:convolveResult;
                  convolveResult = convolveResult > maxPix? maxPix:convolveResult;
                  
                  outBuf[outputOffset] = (T)convolveResult;
               }
            }
            else
            {
               theTile->setNull(outputOffset);
               
            }
            ++convolutionCenterOffset;
            ++convolutionTopLeftOffset;
            ++outputOffset;
         }
      }
   }
   else  // do not need to check for nulls here.
   {
      for(long b = 0; b < outputBands; ++b)
      {                  
         double convolveResult = 0;
         const T* buf = (const T*)inputTile->getBuf(b);
         T* outBuf    = (T*)(theTile->getBuf(b));
         np =(T)nullPix[b];
         outputOffset = 0;
         
         for(long y = 0; y <tileHeight; y++)
         {
            convolutionTopLeftOffset = patchWidth*(startDelta.y + y - convolutionOffsetY) + startDelta.x-convolutionOffsetX;
            
            for(long x =0; x < tileWidth; x++)
            {
               kernel->convolveSubImage(&buf[convolutionTopLeftOffset],
                                        patchWidth,
                                        convolveResult,
                                        np);
// NOT SURE IF I WANT TO CLAMP IN A CONVOLUTION SOURCE  
// seems better to clamp to a scalar range instead of an input min max
               convolveResult = convolveResult < minPix? (T)minPix:convolveResult;
               convolveResult = convolveResult > maxPix?(T)maxPix:convolveResult;
               outBuf[outputOffset] = (T)convolveResult;
               ++outputOffset;
               ++convolutionTopLeftOffset;
            }
         }
      }
   }
}

void rspfConvolutionSource::initialize()
{
   rspfImageSourceFilter::initialize();
   theTile = NULL;
}

void rspfConvolutionSource::allocate()
{
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this,
                                                          theInputConnection);
      theTile->initialize();
   }
}

bool rspfConvolutionSource::saveState(rspfKeywordlist& kwl,
                                       const char* prefix)const
{
   rspf_uint32 numberOfMatrices = 0;
   for(rspf_uint32 m = 0; m < theConvolutionKernelList.size();++m)
   {
      if(theConvolutionKernelList[m])
      {
         ++numberOfMatrices;
         const NEWMAT::Matrix& kernel = theConvolutionKernelList[m]->getKernel();
         rspfString mPrefix = "m" +
                               rspfString::toString(numberOfMatrices) +
                               ".";
         kwl.add(prefix,
                 (mPrefix + "rows").c_str(),
                 kernel.Nrows(),
                 true);
         
         kwl.add(prefix,
                 (mPrefix + "cols").c_str(),
                 kernel.Ncols(),
                 true);
         
         for(rspf_int32 row = 0; row < kernel.Nrows(); ++row)
         {
            for(rspf_int32 col =0; col < kernel.Ncols(); ++col)
            {
               rspfString newPrefix = mPrefix +
                                       rspfString::toString(row+1) + "_" +
                                       rspfString::toString(col+1);
               kwl.add(prefix,
                       newPrefix,
                       kernel[row][col],
                       true);          
            }
         }
      }
   }

   
   kwl.add(prefix,
           NUMBER_OF_MATRICES,
           numberOfMatrices,
           true);
      
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfConvolutionSource::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   deleteConvolutionList();

   const char* numberOfMatrices = kwl.find(prefix, NUMBER_OF_MATRICES);

   rspf_int32 matrixCount = rspfString(numberOfMatrices).toLong();
   rspf_int32 numberOfMatches = 0;
   rspf_int32 index = 0;
   while(numberOfMatches < matrixCount)
   {
      rspfString newPrefix = prefix;
      newPrefix += rspfString("m");
      newPrefix += rspfString::toString(index);
      newPrefix += rspfString(".");

      const char* rows = kwl.find((newPrefix+NUMBER_OF_ROWS.key()).c_str());
      const char* cols = kwl.find((newPrefix+NUMBER_OF_COLS.key()).c_str());

      if(rows&&cols)
      {
         ++numberOfMatches;
         rspf_int32 numberOfRows = rspfString(rows).toLong();
         rspf_int32 numberOfCols = rspfString(cols).toLong();
         NEWMAT::Matrix convolutionMatrix(numberOfRows, numberOfCols);
         
         
         for(rspf_int32 r = 1; r <= numberOfRows; r++)
         {
            for(rspf_int32 c = 1; c <= numberOfCols; c++)
            {
               convolutionMatrix[r-1][c-1] = 0.0;
               
               rspfString value = rspfString::toString(r);
               value += "_";
               value += rspfString::toString(c);

               const char* v = kwl.find(newPrefix.c_str(),
                                        value.c_str());
               if(v)
               {
                  convolutionMatrix[r-1][c-1] = rspfString(v).toDouble();
               }
            }
         }
         theConvolutionKernelList.push_back(new rspfDiscreteConvolutionKernel(convolutionMatrix));
      }
      ++index;
   }
   setKernelInformation();
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfConvolutionSource::setKernelInformation()
{
   rspf_uint32 index;
   
   if(theConvolutionKernelList.size() > 0)
   {
      theMaxKernelWidth  = theConvolutionKernelList[0]->getWidth();
      theMaxKernelHeight = theConvolutionKernelList[0]->getHeight();
      
      for(index = 1; index < theConvolutionKernelList.size(); ++index)
      {
         rspf_int32 w = theConvolutionKernelList[index]->getWidth();
         rspf_int32 h = theConvolutionKernelList[index]->getHeight();
         theMaxKernelWidth  = theMaxKernelWidth < w?w:theMaxKernelWidth;
         theMaxKernelHeight = theMaxKernelHeight < h?h:theMaxKernelHeight;
      }
   }
}

void rspfConvolutionSource::deleteConvolutionList()
{
   for(rspf_int32 index = 0; index < (rspf_int32)theConvolutionKernelList.size(); ++index)
   {
      delete theConvolutionKernelList[index];
   }

   theConvolutionKernelList.clear();
}

void rspfConvolutionSource::setConvolution(const NEWMAT::Matrix& convolutionMatrix, bool doWeightedAverage)
{
   std::vector<NEWMAT::Matrix> m;
   
   m.push_back(convolutionMatrix);
   
   setConvolutionList(m, doWeightedAverage);
}
