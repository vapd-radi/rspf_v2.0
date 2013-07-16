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
// $Id: rspfFeatherMosaic.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfFeatherMosaic.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfLine.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfFeatherMosaic:debug");

RTTI_DEF1(rspfFeatherMosaic, "rspfFeatherMosaic", rspfImageMosaic);

rspfFeatherMosaic::rspfFeatherMosaic()
   :rspfImageMosaic(),
    theInputFeatherInformation(NULL),
    theAlphaSum(NULL),
    theResult(NULL),
    theFeatherInfoSize(0)
{
}

rspfFeatherMosaic::rspfFeatherMosaic(rspfConnectableObject::ConnectableObjectList& inputSources)
   :rspfImageMosaic(inputSources),
    theInputFeatherInformation(NULL),
    theAlphaSum(NULL),
    theResult(NULL),
    theFeatherInfoSize(0)
{
   initialize();
}

rspfFeatherMosaic::~rspfFeatherMosaic()
{
   if(theInputFeatherInformation)
   {
      delete [] theInputFeatherInformation;
      theInputFeatherInformation = NULL;
   }
   theFeatherInfoSize = 0;
}

rspfRefPtr<rspfImageData> rspfFeatherMosaic::getTile(const rspfIrect& tileRect,
                                            rspf_uint32 resLevel)
{
   long w = tileRect.width();
   long h = tileRect.height();
   rspfIpt origin = tileRect.ul();
   
   if(!isSourceEnabled())
   {
      return rspfImageMosaic::getTile(tileRect, resLevel);
   }
   if(!theTile||!theAlphaSum||!theResult||!theInputFeatherInformation)
   {
      initialize();

      if(!theTile||!theAlphaSum||!theResult||!theInputFeatherInformation)
      {
         return rspfImageMosaic::getTile(tileRect, resLevel);
      }
   }
   rspf_uint32 size = getNumberOfInputs();
   theAlphaSum->setImageRectangle(tileRect);
   theResult->setImageRectangle(tileRect);
   
   if(size == 0)
   {
      return rspfRefPtr<rspfImageData>();
   }

   if(size == 1)
   {
      return rspfImageMosaic::getTile(tileRect, resLevel);
   }

   long tileW = theTile->getWidth();
   long tileH = theTile->getHeight();
   if((w != tileW)||
      (h != tileH))
   {
      theTile->setWidth(w);
      theTile->setHeight(h);
      if((w*h)!=(tileW*tileH))
      {
         theTile->initialize();
      }
   }
   theTile->setOrigin(origin);
   theTile->makeBlank();
      
   switch(theTile->getScalarType())
   {
      case RSPF_UCHAR:
      {
         return combine(static_cast<rspf_uint8>(0),
                        tileRect, resLevel);
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         return combine(static_cast<rspf_uint16>(0),
                        tileRect, resLevel);
      }
      case RSPF_SSHORT16:
      {
         return combine(static_cast<rspf_sint16>(0),
                        tileRect, resLevel);
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         return combine(static_cast<double>(0),
                        tileRect, resLevel);
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         return combine(static_cast<float>(0),
                        tileRect, resLevel);
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfFeatherMosaic::getTile: error, unknown scalar type!!!"
            << std::endl;
      }
   }
   return rspfRefPtr<rspfImageData>();
}


template <class T> rspfRefPtr<rspfImageData> rspfFeatherMosaic::combine(
   T,
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> currentImageData;
   rspf_uint32 band;
   long upperBound = theTile->getWidth()*theTile->getHeight();
   long offset = 0;
   long row    = 0;
   long col    = 0;
   long numberOfTilesProcessed = 0;
   float *sumBand       = static_cast<float*>(theAlphaSum->getBuf());
   float         *bandRes = NULL;
   rspfIpt point;
   
   theAlphaSum->fill(0.0);
   theResult->fill(0.0);
   
   T** srcBands  = new T*[theLargestNumberOfInputBands];
   T** destBands = new T*[theLargestNumberOfInputBands];
   rspf_uint32 layerIdx = 0;
   currentImageData  = getNextTile(layerIdx,
                                   0,
                                   tileRect,
                                   resLevel);
   if(!currentImageData.valid())
   {
      delete [] srcBands;
      delete [] destBands;
      return currentImageData;
   }
   rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
   for(band = 0; band < minNumberOfBands; ++band)
   {
      srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
   }
   // if the src is smaller than the destination in number
   // of bands we will just duplicate the last band.
   for(;band < theLargestNumberOfInputBands; ++band)
   {
      srcBands[band]  = srcBands[minNumberOfBands - 1];
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
   }

   // most of the time we will not overlap so let's
   // copy the first tile into destination and check later.
   //
   for(band = 0; band < theTile->getNumberOfBands();++band)
   {
      T* destBand = destBands[band];
      T* srcBand  = srcBands[band];
      if(destBand&&srcBand)
      {
         for(offset = 0; offset < upperBound;++offset)
         {
            *destBand = *srcBand;
            ++srcBand; ++destBand;
         }
      }
   }
   theTile->setDataObjectStatus(currentImageData->getDataObjectStatus());

   while(currentImageData.valid())
   {
      rspfDataObjectStatus currentStatus     = currentImageData->getDataObjectStatus();
      point = currentImageData->getOrigin();
      long h = (long)currentImageData->getHeight();
      long w = (long)currentImageData->getWidth();
      if( (currentStatus != RSPF_EMPTY) &&
          (currentStatus != RSPF_NULL))
      {
         ++numberOfTilesProcessed;
         offset = 0;
         minNumberOfBands = currentImageData->getNumberOfBands();
         for(band = 0; band < minNumberOfBands; ++band)
         {
            srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
         }
         // if the src is smaller than the destination in number
         // of bands we will just duplicate the last band.
         for(;band < theLargestNumberOfInputBands; ++band)
         {
            srcBands[band]  = srcBands[minNumberOfBands - 1];
         }
          if(currentStatus == RSPF_PARTIAL)
          {
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                  if(!currentImageData->isNull(offset))
                  {
                     double weight = computeWeight(layerIdx,
                                                   rspfDpt(point.x+col,
                                                            point.y+row));
                     
                     for(band = 0; band < theLargestNumberOfInputBands; ++band)
                     {
                        bandRes = static_cast<float*>(theResult->getBuf(band));
                        bandRes[offset] += (srcBands[band][offset]*weight);
                     }
                     sumBand[offset] += weight;
                  }
                  ++offset;
               }
            }
         }
         else
         {
            offset = 0;
            
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                     double weight = computeWeight(layerIdx,
                                                   rspfDpt(point.x+col,
                                                            point.y+row));
                     
                     for(band = 0; band < theLargestNumberOfInputBands; ++band)
                     {
                        bandRes     = static_cast<float*>(theResult->getBuf(band));
                        
                        bandRes[offset] += (srcBands[band][offset]*weight);
                     }
                     sumBand[offset] += weight;
                     ++offset;
               }
            }
         }
      }
      currentImageData = getNextTile(layerIdx, tileRect, resLevel);
   }
   upperBound = theTile->getWidth()*theTile->getHeight();

   if(numberOfTilesProcessed > 1)
   {
      const double* minPix = theTile->getMinPix();
      const double* maxPix = theTile->getMaxPix();
      const double* nullPix= theTile->getNullPix();
      for(offset = 0; offset < upperBound;++offset)
      {
         for(band = 0; band < theTile->getNumberOfBands();++band)
         {
            T* destBand      = static_cast<T*>(theTile->getBuf(band));
            float* weightedBand = static_cast<float*>(theResult->getBuf(band));

            // this should be ok to test 0.0 instead of
            // FLT_EPSILON range for 0 since we set it.
            if(sumBand[offset] != 0.0)
            {            
               weightedBand[offset] = (weightedBand[offset])/sumBand[offset];
               if(weightedBand[offset]<minPix[band])
               {
                  weightedBand[offset] = minPix[band];   
               }
               else if(weightedBand[offset] > maxPix[band])
               {
                  weightedBand[offset] = maxPix[band];                   
               }
            }
            else
            {
               weightedBand[offset] = nullPix[band];
            }
            destBand[offset] = static_cast<T>(weightedBand[offset]);
         }
      }
      theTile->validate();
   }

   delete [] srcBands;
   delete [] destBands;

   return theTile;
}

double rspfFeatherMosaic::computeWeight(long index,
                                         const rspfDpt& point)const
{
   rspfFeatherInputInformation& info = theInputFeatherInformation[index];
   double result = 0.0;
   rspfDpt delta = point-info.theCenter;
   
   double length1 = fabs(delta.x*info.theAxis1.x + delta.y*info.theAxis1.y)/info.theAxis1Length;
   double length2 = fabs(delta.x*info.theAxis2.x + delta.y*info.theAxis2.y)/info.theAxis2Length;

   if(length1 > length2)
   {
      result = (1.0 - length1);
   }
   else
   {
      result = (1.0 - length2);
   }
   if(result < 0) result = 0;
   
   return result;
}

void rspfFeatherMosaic::initialize()
{
   rspfImageMosaic::initialize();

   allocate();
   if(theTile.valid())
   {
      theAlphaSum = new rspfImageData(this,
                                       RSPF_FLOAT,
                                       1,
                                       theTile->getWidth(),
                                       theTile->getHeight());
      theResult = new rspfImageData(this,
                                     RSPF_FLOAT,
                                     theLargestNumberOfInputBands,
                                     theTile->getWidth(),
                                     theTile->getHeight());
      theAlphaSum->initialize();
      theResult->initialize();
   }
   std::vector<rspfIpt> validVertices;
   if(!getNumberOfInputs()) return;

   if(theInputFeatherInformation)
   {
      delete [] theInputFeatherInformation;
      theInputFeatherInformation = NULL;
   }
   theFeatherInfoSize = getNumberOfInputs();
   theInputFeatherInformation = new rspfFeatherInputInformation[theFeatherInfoSize];
   for(long index = 0; index < theFeatherInfoSize; ++ index)
   {
      validVertices.clear();
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(index));
      if(temp)
      {
         temp->getValidImageVertices(validVertices, RSPF_CLOCKWISE_ORDER);
         theInputFeatherInformation[index].setVertexList(validVertices);
      }
   }
}


rspfFeatherMosaic::rspfFeatherInputInformation::rspfFeatherInputInformation(const std::vector<rspfIpt>& validVertices)
{
   setVertexList(validVertices);
}


void rspfFeatherMosaic::rspfFeatherInputInformation::setVertexList(const std::vector<rspfIpt>& validVertices)
{
   const char* MODULE = "rspfFeatherMosaic::rspfFeatherInputInformation::setVertexList()";
   
   theValidVertices = validVertices;

   theCenter       = rspfDpt(0,0);
   theAxis1        = rspfDpt(1, 0);
   theAxis2        = rspfDpt(0, 1);
   theAxis1Length  = 1;
   theAxis2Length  = 1;

   double xSum=0.0, ySum=0.0;
   rspf_uint32 upperBound = (rspf_uint32)validVertices.size();
   if(upperBound)
   {
      for(rspf_uint32 index = 0; index < upperBound; ++index)
      {
         xSum += validVertices[index].x;
         ySum += validVertices[index].y;
      }

      theCenter.x = xSum/upperBound;
      theCenter.y = ySum/upperBound;

      // for now we just want a quick implementation of something
      // and we know that we have 4 vertices for the bounding valid
      // vertices.
      //
      if(upperBound == 4)
      {
         rspfDpt edgeDirection1 = validVertices[1] - validVertices[0];
         rspfDpt edgeDirection2 = validVertices[2] - validVertices[1];

         theAxis1 = rspfDpt(-edgeDirection1.y, edgeDirection1.x);
         
         theAxis2 = rspfDpt(-edgeDirection2.y, edgeDirection2.x);

         theAxis1 = theAxis1/theAxis1.length();
         theAxis2 = theAxis2/theAxis2.length();

         rspfLine line1(theCenter,
                         theCenter + theAxis1*2);
         rspfLine line2(validVertices[1],
                         validVertices[0]);
         rspfLine line3(theCenter,
                         theCenter + theAxis2*2);
         rspfLine line4(validVertices[2],
                         validVertices[1]);
         
         rspfDpt intersectionPoint1 = line1.intersectInfinite(line2);
         rspfDpt intersectionPoint2 = line3.intersectInfinite(line4);

         
         theAxis1Length = rspf::round<int>((theCenter-intersectionPoint1).length());
         theAxis2Length = rspf::round<int>((theCenter-intersectionPoint2).length());

          if(traceDebug())
          {
             CLOG << "theAxis1Length:       " << theAxis1Length << endl
                  << "theAxis2Length:       " << theAxis2Length << endl
                  << "center:               " << theCenter      << endl;
          }
      }
   }
}

ostream& operator<<(ostream& out,
                    const rspfFeatherMosaic::rspfFeatherInputInformation& data)
{
   out << "center: " << data.theCenter << endl
       << "axis1:  " << data.theAxis1  << endl
       << "axis2:  " << data.theAxis2  << endl
       << "axis1_length: " << data.theAxis1Length << endl
       << "axis2_length: " << data.theAxis2Length << endl
       << "valid vertices: " << endl;
   std::copy(data.theValidVertices.begin(),
             data.theValidVertices.end(),
             std::ostream_iterator<rspfDpt>(out, "\n"));
   return out;
}
