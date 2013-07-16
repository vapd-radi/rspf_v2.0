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
// $Id: rspfImageDataHelper.cpp 21184 2012-06-29 15:13:09Z dburken $
#include <rspf/imaging/rspfImageDataHelper.h>
#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/base/rspfLine.h>
#include <vector>
#include <algorithm>

rspfImageDataHelper::rspfImageDataHelper(rspfImageData* imageData)
{
   setImageData(imageData);
}


void rspfImageDataHelper::setImageData(rspfImageData* imageData)
{
   theImageData = imageData;
   if(theImageData)
   {
      theImageRectangle     = theImageData->getImageRectangle();
      thePolyImageRectangle = theImageRectangle;
//       thePolyImageRectangle = rspfPolyArea2d(theImageRectangle.ul(),
//                                               theImageRectangle.ur(),
//                                               theImageRectangle.lr(),
//                                               theImageRectangle.ll());
      theOrigin = theImageRectangle.ul();
   }
}

void rspfImageDataHelper::copyInputToThis(const void* input,
                                           const rspfPolygon& region,
                                           bool clipPoly)
{
   if(!theImageData) return;
   switch(theImageData->getScalarType())
   {
      case RSPF_UCHAR:
      {
         copyInputToThis(reinterpret_cast<const rspf_uint8*>(input),
                         region,
                         clipPoly);
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         copyInputToThis(reinterpret_cast<const rspf_uint16*>(input),
                         region,
                         clipPoly);
         break;
      }
      case RSPF_SSHORT16:
      {
         copyInputToThis(reinterpret_cast<const rspf_sint16*>(input),
                         region,
                         clipPoly);
         break;
      }
      case RSPF_SINT32:
      {
          copyInputToThis(reinterpret_cast<const rspf_sint32*>(input),
                          region,
                          clipPoly);
          break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         copyInputToThis(reinterpret_cast<const float*>(input),
                         region,
                         clipPoly);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         copyInputToThis(reinterpret_cast<const double*>(input),
                         region,
                         clipPoly);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         break;
      }
   }   
}

void rspfImageDataHelper::fill(const rspfRgbVector& color,
                                std::vector<rspfPolygon>& regionList,
                                bool clipPoly)
{
   int i = 0;

   for(i = 0; i < (int)regionList.size();++i)
   {
      fill(color,
           regionList[i],
           clipPoly);
   }
}


void rspfImageDataHelper::fill(const double* values,
                                std::vector<rspfPolygon>& regionList,
                                bool clipPoly)
{
   int i = 0;

   for(i = 0; i < (int)regionList.size();++i)
   {
      fill(values,
           regionList[i],
           clipPoly);
   }
}

void rspfImageDataHelper::fill(const rspfRgbVector& color,
                                const rspfPolygon& region,
                                bool clipPoly)
{
   double colorArray[3];

   colorArray[0] = color.getR();
   colorArray[1] = color.getG();
   colorArray[2] = color.getB();
   switch(theImageData->getScalarType())
   {
      case RSPF_UCHAR:
      {
         if(theImageData->getNumberOfBands() <= 3)
         {
            fill(static_cast<rspf_uint8>(0),
                 (const double*)colorArray,
                 region,
                 clipPoly);
            
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      case RSPF_SSHORT16:
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      default:
      {
         break;
      }
   }
}

void rspfImageDataHelper::fill(const rspfRgbVector& color,
                                const rspfIrect& region,
                                bool clipPoly)
{
   double colorArray[3];

   colorArray[0] = color.getR();
   colorArray[1] = color.getG();
   colorArray[2] = color.getB();

   switch(theImageData->getScalarType())
   {
      case RSPF_UCHAR:
      {
         if(theImageData->getNumberOfBands() <= 3)
         {
            fill(static_cast<rspf_uint8>(0),
                 colorArray,
                 region,
                 clipPoly);
            
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      case RSPF_SSHORT16:
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         break;
      }
      default:
         break;
   }
}

void rspfImageDataHelper::fill(const double*  values,
                                const rspfPolygon& region,
                                bool clipPoly)
{
   switch(theImageData->getScalarType())
   {
      case RSPF_UCHAR:
      {
         fill(static_cast<rspf_uint8>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         fill(static_cast<rspf_uint16>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_SSHORT16:
      {
         fill(static_cast<rspf_sint16>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         fill(static_cast<float>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         fill(static_cast<double>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         break;
      }
   }
}

void rspfImageDataHelper::fill(const double*  values,
                                const rspfIrect& region,
                                bool clipPoly)
{
   switch(theImageData->getScalarType())
   {
      case RSPF_UCHAR:
      {
         fill(static_cast<rspf_uint8>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         fill(static_cast<rspf_uint16>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_SSHORT16:
      {
         fill(static_cast<rspf_sint16>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         fill(static_cast<float>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         fill(static_cast<double>(0),
              values,
              region,
              clipPoly);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         break;
      }
   }
}



template <class T>
void rspfImageDataHelper::copyInputToThis(const T* inputBuf,
                                           const rspfPolygon& region,
                                           bool clipPoly)
{
   if(clipPoly)
   {
      rspfPolyArea2d polyArea(region);
      rspfPolyArea2d clipArea = polyArea&thePolyImageRectangle;
      
      vector<rspfPolygon> clipList;
//      clipArea.getAllVisiblePolygons(clipList);
      clipArea.getVisiblePolygons(clipList);
      int i = 0;
            
      for(i = 0; i < (int)clipList.size();++i)
      {
         copyInputToThis(inputBuf,
                         clipList[i]);
      }
   }
   else
   {
      copyInputToThis(inputBuf,
                      region);
   }
}

template <class T>
void rspfImageDataHelper::copyInputToThis(const T* inputBuf,
                                           const rspfPolygon& region)
{ 
   int n;
   int i;
   int y;
   rspf_int32 miny, maxy, minx, maxx;
   int x1, y1;
   int x2, y2;
   int ind1, ind2;
   // int ints;
   vector<int> polyInts;
   T* buf = reinterpret_cast<T*>(theImageData->getBuf());
   
   rspf_int32 blockLength=theImageData->getWidth()*theImageData->getHeight();
   rspf_int32 bandOffset = 0;
   
   region.getIntegerBounds(minx, miny, maxx, maxy);
   rspf_int32 rowOffset = (miny-theOrigin.y)*theImageData->getWidth();
   n = region.getVertexCount();
   /* Fix in 1.3: count a vertex only once */
   for (y = miny; (y <= maxy); y++)
   {
      polyInts.clear();
      // ints = 0;
      for (i = 0; (i < n); i++)
      {
         if (!i)
         {
            ind1 = n - 1;
            ind2 = 0;
         }
         else
         {
            ind1 = i - 1;
            ind2 = i;
         }
         y1 = rspf::round<int>(region[ind1].y);
         y2 = rspf::round<int>(region[ind2].y);
         if (y1 < y2)
         {
            x1 = rspf::round<int>(region[ind1].x);
            x2 = rspf::round<int>(region[ind2].x);
         }
         else if (y1 > y2)
         {
            y2 = rspf::round<int>(region[ind1].y);
            y1 = rspf::round<int>(region[ind2].y);
            x2 = rspf::round<int>(region[ind1].x);
            x1 = rspf::round<int>(region[ind2].x);
         }
         else
         {
            continue;
         }
         if ((y >= y1) && (y < y2))
         {
            polyInts.push_back((y - y1) * (x2 - x1) / (y2 - y1) + x1);
         }
         else if ((y == maxy) && (y > y1) && (y <= y2))
         {
            polyInts.push_back((y - y1) * (x2 - x1) / (y2 - y1) + x1);
         }
      }
      std::sort(polyInts.begin(), polyInts.end());
      
      for (i = 0; (i < (int)polyInts.size()); i += 2)
      {
         rspf_int32 startX = polyInts[i]-theOrigin.x;
         rspf_int32 endX   = polyInts[i+1]-theOrigin.x;
         while(startX <= endX)
         {
            bandOffset = 0;
	    int band;
            for(band = 0; band < (rspf_int32)theImageData->getNumberOfBands();++band)
            {
               buf[rowOffset+bandOffset+startX] = (inputBuf[rowOffset+bandOffset+startX]);
               bandOffset += blockLength;
            }
            ++startX;
         }
      }
      rowOffset += theImageData->getWidth();
   }
}

template <class T>
void rspfImageDataHelper::fill(T dummyVariable,
                                const double* values,
                                const rspfPolygon& region,
                                bool clipPoly)
{
   if(clipPoly)
   {
      rspfPolyArea2d polyArea(region);
      rspfPolyArea2d clipArea = polyArea&thePolyImageRectangle;
      
      vector<rspfPolygon> clipList;
//      clipArea.getAllVisiblePolygons(clipList);
      clipArea.getVisiblePolygons(clipList);
            
      int i = 0;
      for(i = 0; i < (int)clipList.size();++i)
      {
         fill(dummyVariable,
              values,
              clipList[i]);
      }
   }
   else
   {
      fill(dummyVariable,
           values,
           region);
   }
}

template <class T>
void rspfImageDataHelper::fill(T dummyVariable,
                                const double* values,
                                const rspfIrect& region,
                                bool clipPoly)
{
   if(clipPoly)
   {
      fill(dummyVariable,
           values,
           region.clipToRect(theImageData->getImageRectangle()));
   }
   else
   {
      fill(dummyVariable,
           values,
           region);
   }
}

template <class T>
void rspfImageDataHelper::fill(T /* dummyVariable */,
                                const double* values,
                                const rspfPolygon& region)
{
   int n;
   int i;
   int y;
   rspf_int32 miny, maxy, minx, maxx;
   int x1, y1;
   int x2, y2;
   int ind1, ind2;
   // int ints;
   vector<int> polyInts;
   T* buf = reinterpret_cast<T*>(theImageData->getBuf());
   
   rspf_int32 blockLength=theImageData->getWidth()*theImageData->getHeight();
   rspf_int32 bandOffset = 0;
   
   region.getIntegerBounds(minx, miny, maxx, maxy);
   rspf_int32 rowOffset = (miny-theOrigin.y)*theImageData->getWidth();
   n = region.getVertexCount();
   /* Fix in 1.3: count a vertex only once */
   for (y = miny; (y <= maxy); y++)
   {
      polyInts.clear();
      // ints = 0;
      for (i = 0; (i < n); i++)
      {
         if (!i)
         {
            ind1 = n - 1;
            ind2 = 0;
         }
         else
         {
            ind1 = i - 1;
            ind2 = i;
         }
         y1 = rspf::round<int>(region[ind1].y);
         y2 = rspf::round<int>(region[ind2].y);
         if (y1 < y2)
         {
            x1 = rspf::round<int>(region[ind1].x);
            x2 = rspf::round<int>(region[ind2].x);
         }
            else if (y1 > y2)
            {
               y2 = rspf::round<int>(region[ind1].y);
               y1 = rspf::round<int>(region[ind2].y);
               x2 = rspf::round<int>(region[ind1].x);
               x1 = rspf::round<int>(region[ind2].x);
            }
         else
         {
            continue;
         }
         if ((y >= y1) && (y < y2))
         {
            polyInts.push_back((y - y1) * (x2 - x1) / (y2 - y1) + x1);
         }
         else if ((y == maxy) && (y > y1) && (y <= y2))
         {
            polyInts.push_back((y - y1) * (x2 - x1) / (y2 - y1) + x1);
         }
      }
      std::sort(polyInts.begin(), polyInts.end());
      
      for (i = 0; (i < (int)polyInts.size()); i += 2)
      {
         rspf_int32 startX = std::abs(polyInts[i]-theOrigin.x);
         rspf_int32 endX   = std::abs(polyInts[i+1]-theOrigin.x);
         while(startX <= endX)
         {
            bandOffset = 0;
	    int band;
            for(band = 0; band < (int)theImageData->getNumberOfBands();++band)
            {
               buf[rowOffset+bandOffset+startX] = (T)(values[band]);
               bandOffset += blockLength;
            }
            ++startX;
         }
      }
      rowOffset += theImageData->getWidth();
   }
}

template <class T>
void rspfImageDataHelper::fill(T /* dummyVariable */,
                                const double* values,
                                const rspfIrect& region)
{
   T* buf = reinterpret_cast<T*>(theImageData->getBuf());
   
   rspf_int32 blockLength=theImageData->getWidth()*theImageData->getHeight();
   rspf_int32 bandOffset = 0;
   rspf_int32 miny,maxy;
   rspf_int32 minx, maxx;
   rspf_int32 y = 0;
   miny = region.ul().y-theOrigin.y;
   maxy = region.lr().y-theOrigin.y;
   minx = region.ul().x-theOrigin.x;
   maxx = region.lr().x-theOrigin.x;
   
   rspf_int32 rowOffset = (miny)*theImageData->getWidth();
   rspf_int32 startX = minx;
   rspf_int32 endX   = maxx;
   rspf_int32 bands  = (rspf_int32)theImageData->getNumberOfBands();

   for (y = miny; (y <= maxy); ++y)
   {
      startX=minx;
      while(startX <= endX)
      {
         bandOffset = 0;
	 int band = 0;
         for(band = 0; band < bands;++band)
         {
            buf[rowOffset+bandOffset+startX] = (T)(values[band]);
            bandOffset += blockLength;
         }
         ++startX;
      }
      rowOffset += theImageData->getWidth();
   }
}
 
