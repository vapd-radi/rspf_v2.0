//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfDiscreteConvolutionKernel.cpp 12912 2008-05-28 15:05:54Z gpotts $
#include <rspf/imaging/rspfDiscreteConvolutionKernel.h>

 
rspfDiscreteConvolutionKernel::rspfDiscreteConvolutionKernel(long width,
                                                               long height,
                                                               bool doWeightedAverage)
   : theKernel(0),
     theWidth(width),
     theHeight(height),
     theComputeWeightedAverageFlag(doWeightedAverage)
{
   theWidth  = theWidth <0?1:theWidth;
   theHeight = theHeight <0?1:theHeight;
   
   theKernel = new NEWMAT::Matrix(theHeight, theWidth);
   *theKernel = (1.0/(theHeight*theWidth));
}

rspfDiscreteConvolutionKernel::rspfDiscreteConvolutionKernel(const NEWMAT::Matrix& kernel,
                                                               bool doWeightedAverage)
   :theKernel(new NEWMAT::Matrix(kernel)),
   theComputeWeightedAverageFlag(doWeightedAverage)
{
   theWidth  = theKernel->Ncols();
   theHeight = theKernel->Nrows();
}

rspfDiscreteConvolutionKernel::~rspfDiscreteConvolutionKernel()
{
   if(theKernel)
   {
      delete theKernel;
      theKernel = 0;
   }
}

void rspfDiscreteConvolutionKernel::buildSymmetric(const std::vector<float>& coefficients,
                                                    NEWMAT::Matrix& result)
{
   rspf_uint32 size = (rspf_uint32)coefficients.size();
   if(size > 0)
   {

      NEWMAT::RowVector row(size);
      NEWMAT::ColumnVector col(size);
      for(rspf_uint32 index = 0; index < size; ++index)
      {
         row[index] = coefficients[index];
         col[index] = coefficients[index];
      }
      result = col*row;
   }
}

void rspfDiscreteConvolutionKernel::buildSymmetric(float *coefficients,
                                                    long size,
                                                    NEWMAT::Matrix& result)
{
   if(size > 0)
   {
      NEWMAT::RowVector    row(size);
      NEWMAT::ColumnVector col(size);
      for(long index = 0; index < size; ++index)
      {
         row[index] = coefficients[index];
         col[index] = coefficients[index];
      }
      result = col*row;
   }
}

void rspfDiscreteConvolutionKernel::setKernel(const NEWMAT::Matrix& kernel)
{
   if(theKernel)
   {
      theKernel = new NEWMAT::Matrix;
   }
   *theKernel = kernel;
   theWidth  = theKernel->Ncols();
   theHeight = theKernel->Nrows();   
}

void rspfDiscreteConvolutionKernel::convolve(const float* data,
                                              double& result,
                                              float nullPixel)const
{
   result = 0;

   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
   }
}

void rspfDiscreteConvolutionKernel::convolveSubImage(const float* data,
                                                      long dataWidth,
                                                      double& result,
                                                      float nullPixel)const
{
   const float* dataStart = 0;
   result = 0;
   
   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
   }
}

void rspfDiscreteConvolutionKernel::convolve(const unsigned char* data,
                                              double& result,
                                              rspf_uint8 nullPixel)const
{
   result = 0;

   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
   }
}

void rspfDiscreteConvolutionKernel::convolveSubImage(const unsigned char* data,
                                                      long dataWidth,
                                                      double& result,
                                                      rspf_uint8 nullPixel)const
{
   const rspf_uint8* dataStart = 0;
   result = 0;
   
   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
   }
}


void rspfDiscreteConvolutionKernel::convolve(const unsigned short* data,
                                              double& result,
                                              rspf_uint16 nullPixel)const
{
   result = 0;

   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
   }
}

void rspfDiscreteConvolutionKernel::convolveSubImage(const unsigned short* data,
                                                      long dataWidth,
                                                      double& result,
                                                      rspf_uint16 nullPixel)const
{
   const rspf_uint16* dataStart = 0;
   result = 0;
   
   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
   }
}

void rspfDiscreteConvolutionKernel::convolve(const short* data,
                                              double& result,
                                              rspf_sint16 nullPixel)const
{
   result = 0;

   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
   }
}

void rspfDiscreteConvolutionKernel::convolveSubImage(const short* data,
                                                      long dataWidth,
                                                      double& result,
                                                      rspf_sint16 nullPixel)const
{
   const rspf_sint16* dataStart = 0;
   result = 0;
   
   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
   }
}


void rspfDiscreteConvolutionKernel::convolve(const double* data,
                                              double& result,
                                              double nullPixel)const
{
   result = 0;

   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         for(long col=0; col < theWidth; ++col)
         {
            if(*data != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*data));
            }
            ++data;
         }
      }
   }
}

void rspfDiscreteConvolutionKernel::convolveSubImage(const double* data,
                                                      long dataWidth,
                                                      double& result,
                                                      double nullPixel)const
{
   const double* dataStart = 0;
   result = 0;
   
   if(theComputeWeightedAverageFlag)
   {
      double divisor = 0.0;
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               divisor += (*theKernel)[row][col];
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
      if(divisor > 0)
      {
         result /= divisor;
      }
   }
   else
   {
      for(long row=0; row < theHeight; ++row)
      {
         dataStart=data;
         for(long col=0; col < theWidth; ++col)
         {
            if(*dataStart != nullPixel)
            {
               result  += ((*theKernel)[row][col] * (*dataStart));
            }
            ++dataStart;
         }
         data+=dataWidth;
      }
   }
}
