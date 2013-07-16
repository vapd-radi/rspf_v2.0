//*******************************************************************
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//*************************************************************************
// $Id: rspfConvolutionFilter1D.cpp 15833 2009-10-29 01:41:53Z eshirschorn $


#include <rspf/imaging/rspfConvolutionFilter1D.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfMatrixProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfNumericProperty.h>

RTTI_DEF1(rspfConvolutionFilter1D, "rspfConvolutionFilter1D", rspfImageSourceFilter);

#define PROPNAME_KERNELSIZE "KernelSize"
#define PROPNAME_KERNEL "Kernel"
#define PROPNAME_ISHZ   "Horizontal"
#define PROPNAME_CENTEROFFSET "CenterOffset"
#define PROPNAME_STRICTNODATA "StrictNoData"

rspfConvolutionFilter1D::rspfConvolutionFilter1D(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theCenterOffset(0),
    theTile(NULL),
    theIsHz(true),
    theStrictNoData(true),
    theNullPixValue(0),
    theMinPixValue(0),
    theMaxPixValue(0)
{
   //sets up an identity convolution
   theKernel.push_back(1.0);
}

rspfConvolutionFilter1D::~rspfConvolutionFilter1D()
{
}

rspfRefPtr<rspfImageData> rspfConvolutionFilter1D::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return theTile;
   }

   if(!isSourceEnabled())
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }

   //---
   // We have a 1xn or nx1 matrix, + a center offset
   // so stretch the input rect out to cover the required pixels
   //---
   rspfIrect  newRect;
   rspf_uint32 kl = (rspf_uint32)theKernel.size(); //kernel length
   if (theIsHz)
   {
      //horizontal kernel (row kernel)
      newRect = rspfIrect(rspfIpt(tileRect.ul().x - theCenterOffset,
                                    tileRect.ul().y),
                           rspfIpt(tileRect.lr().x - theCenterOffset + kl -1,
                                    tileRect.lr().y));
   } else {
      //vertical kernel
      newRect = rspfIrect(rspfIpt(tileRect.ul().x,
                                    tileRect.ul().y - theCenterOffset),
                           rspfIpt(tileRect.lr().x,
                                    tileRect.lr().y - theCenterOffset + kl -1));
   }
   rspfRefPtr<rspfImageData> data = theInputConnection->getTile(newRect,
                                                                  resLevel);

   if(!data.valid() || !data->getBuf())
   {
      return data;
   }

    // First time through or after an initialize()...
   if (!theTile.valid())
   {
      allocate();
      if (!theTile.valid()) // Should never happen!
      {
         return data;
      }
   }

   // First time through, after an initialize() or a setKernel()...
   if (!theNullPixValue.size())
   {
      computeNullMinMax();
      if (!theNullPixValue.size()) // Should never happen!
      {
         return data;
      }
   }

   theTile->setImageRectangle(tileRect);
   theTile->makeBlank();
   
   switch(data->getScalarType())
   {
      case RSPF_UCHAR:
      {
         if(data->getDataObjectStatus() == RSPF_FULL)
         {
            convolveFull(static_cast<rspf_uint8>(0), data, theTile);
         }
         else
         {
            convolvePartial(static_cast<rspf_uint8>(0), data, theTile);
         }
         break;
      }
      case RSPF_FLOAT: 
      case RSPF_NORMALIZED_FLOAT:
      {
         if(data->getDataObjectStatus() == RSPF_FULL)
         {
            convolveFull(static_cast<float>(0), data, theTile);
         }
         else
         {
            convolvePartial(static_cast<float>(0), data, theTile);
         }
         break;
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         if(data->getDataObjectStatus() == RSPF_FULL)
         {
            convolveFull(static_cast<rspf_uint16>(0), data, theTile);
         }
         else
         {
            convolvePartial(static_cast<rspf_uint16>(0), data, theTile);
         }
         break;
      }
      case RSPF_SSHORT16:
      {
         if(data->getDataObjectStatus() == RSPF_FULL)
         {
            convolveFull(static_cast<rspf_sint16>(0), data, theTile);
         }
         else
         {
            convolvePartial(static_cast<rspf_sint16>(0), data, theTile);
         }
         break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         if(data->getDataObjectStatus() == RSPF_FULL)
         {
            convolveFull(static_cast<double>(0), data, theTile);
      }
      else
      {
         convolvePartial(static_cast<double>(0), data, theTile);
      }
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfConvolutionFilter1D::getTile WARNING:\n"
            << "Scalar type = " << theTile->getScalarType()
            << " Not supported by rspfConvolutionFilter1D" << endl;
         break;
      }
   }
   theTile->validate();
   
   return theTile;
}


template<class T> void rspfConvolutionFilter1D::convolvePartial(
   T,
   rspfRefPtr<rspfImageData> inputData,
   rspfRefPtr<rspfImageData> outputData)
{
   // there may be NULL pixels in the input
   rspf_int32 inputW = static_cast<rspf_int32>(inputData->getWidth());
   rspf_uint32 outputW       = outputData->getWidth();
   rspf_uint32 outputH       = outputData->getHeight();
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
   
   rspf_uint32 klength=(rspf_uint32)theKernel.size(),k;
   rspf_int32 incrK = theIsHz ? 1 : inputW; //kernel increment in input : one col or one row
   rspf_int32 iRow  = inputW - outputW; //jump to go to next row

   double sum=0.0;
   T* iKP; //kernel covered input pointer

   rspf_int32 iCo = incrK*theCenterOffset; //center  pixel offset within the input buffer

   for(rspf_uint32 band = 0; band < numberOfBands; ++band)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(band));
      T* outputBuf = static_cast<T*>(outputData->getBuf(band));
      T maxPix     = static_cast<T>(getMaxPixelValue(band));
      T minPix     = static_cast<T>(getMinPixelValue(band));
      T nullPix    = static_cast<T>(inputData->getNullPix(band));
      T oNullPix   = static_cast<T>(getNullPixelValue(band));
      
      if(inputBuf&&outputBuf)
      {
         for(rspf_uint32 row = 0; row < outputH; ++row)
         {
            for(rspf_uint32 col = 0; col < outputW; ++col)
            {
               //center pixel may not be NULL
               if (inputBuf[iCo] != nullPix)
               {
                  iKP = inputBuf; 
                  sum = 0.0;
                  for(k=0;k<klength;++k)
                  {
                     if (*iKP != nullPix) //just sum on non-NULL pixels (potential normailzation issue)
                     {
                        sum += theKernel[k] * (double)(*iKP);                   
                     } else if (theStrictNoData)
                     {
                        break;
                     }                     
                     iKP += incrK;
                  }
                  if (k==klength)
                  {
                     if(sum > maxPix)
                     {
                        *outputBuf = maxPix;
                     }
                     else if(sum < minPix)
                     {
                        *outputBuf = minPix;
                     }
                     else
                     {
                        *outputBuf = static_cast<T>(sum);
                     }
                  } else {
                     //theStrictNoData
                     *outputBuf = oNullPix;
                  }
               } else {
                  *outputBuf = oNullPix;
               }
               // move curent center position
               inputBuf  += 1;
               outputBuf += 1;
            }
            //move current center position
            inputBuf  += iRow;
            //outputBuf += 0;
         }
      }
   }
    
   
}

template<class T> void rspfConvolutionFilter1D::convolveFull(
   T,
   rspfRefPtr<rspfImageData> inputData,
   rspfRefPtr<rspfImageData> outputData)
{
   //inputData must be displaced according to the Kernel size and theCenterOffset +isHz
   rspf_int32 inputW = static_cast<rspf_int32>(inputData->getWidth());
   rspf_uint32 outputW       = outputData->getWidth();
   rspf_uint32 outputH       = outputData->getHeight();
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
   
   rspf_uint32 klength=(rspf_uint32)theKernel.size(), k;
   rspf_int32 incrK = theIsHz ? 1 : inputW; //kernel increment in input : one col or one row
   rspf_int32 iRow  = inputW - outputW; //jump to go to next row

   double sum=0.0;
   T* iKP; //kernel covered input pointer

   for(rspf_uint32 band = 0; band < numberOfBands; ++band)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(band));
      T* outputBuf = static_cast<T*>(outputData->getBuf(band));
      T maxPix     = static_cast<T>(getMaxPixelValue(band));
      T minPix     = static_cast<T>(getMinPixelValue(band));
      
      if(inputBuf&&outputBuf)
      {
         for(rspf_uint32 row = 0; row < outputH; ++row)
         {
            for(rspf_uint32 col = 0; col < outputW; ++col)
            {
               sum = 0.0;
               iKP = inputBuf;
               for(k=0;k<klength;++k)
               {
                  sum += theKernel[k] * (double)(*iKP);
                  iKP += incrK;
               }
               
               if(sum > maxPix)
               {
                  *outputBuf = maxPix;
               }
               else if(sum < minPix)
               {
                  *outputBuf = minPix;
               }
               else
               {
                  *outputBuf = static_cast<T>(sum);
               }
               // move curent center position
               inputBuf  += 1;
               outputBuf += 1;
            }
            //move current center position
            inputBuf  += iRow;
            //outputBuf += 0;
         }
      }
   }
}

void rspfConvolutionFilter1D::initialize()
{
   //---
   // NOTE:
   // Since initialize get called often sequentially we will wipe things slick
   // but not reallocate to avoid multiple delete/allocates.
   //
   // On the first getTile call things will be reallocated/computed.
   //---
   theTile = NULL;
   clearNullMinMax();
}

void rspfConvolutionFilter1D::allocate()
{   
   if(theInputConnection)
   {
      rspfImageDataFactory* idf = rspfImageDataFactory::instance();
      
      theTile = idf->create(this,
                            this);
      
      theTile->initialize();
   }
}

void rspfConvolutionFilter1D::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if(property->getName() == PROPNAME_KERNEL)
   {
      rspfMatrixProperty* matrixProperty = PTR_CAST(rspfMatrixProperty,
                                                     property.get());
      if(matrixProperty)
      {
         rspf_uint32 nc=matrixProperty->getNumberOfCols();
         theKernel.resize(nc);
         for(rspf_uint32 i=0;i<nc;++i)
         {
             theKernel[i] = (*matrixProperty)(0,i);
         }
         clearNullMinMax();
      }
      else
      {
         rspfImageSourceFilter::setProperty(property);
      }
   } else if (property->getName() == PROPNAME_ISHZ)  {
      rspfBooleanProperty* booleanProperty = PTR_CAST(rspfBooleanProperty,
                                                     property.get());
      if(booleanProperty)
      {
         theIsHz = booleanProperty->getBoolean();
      }
   } else if (property->getName() == PROPNAME_STRICTNODATA)  {
      rspfBooleanProperty* booleanProperty = PTR_CAST(rspfBooleanProperty,
                                                     property.get());
      if(booleanProperty)
      {
         theStrictNoData = booleanProperty->getBoolean();
      }
   } else if (property->getName() == PROPNAME_CENTEROFFSET)  {
      rspfNumericProperty* numProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(numProperty)
      {
         theCenterOffset = numProperty->asInt32();
      }
   } else if (property->getName() == PROPNAME_KERNELSIZE)  {
      rspfNumericProperty* numProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(numProperty)
      {
         theKernel.resize(numProperty->asUInt32());
         clearNullMinMax();
      }
   } else {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfConvolutionFilter1D::getProperty(const rspfString& name)const
{
   if(name == PROPNAME_KERNEL)
   {
      rspfMatrixProperty* property = new rspfMatrixProperty(name);
      property->resize(1,(int)theKernel.size());
      for(rspf_uint32 i=0;i<theKernel.size();++i)
      {
        (*property)(0,i) = theKernel[i];
      }
      property->setCacheRefreshBit();
      return property;
   } else if (name==PROPNAME_ISHZ) {
      rspfBooleanProperty* property = new rspfBooleanProperty(name,theIsHz);
      property->setCacheRefreshBit();
      return property;
   } else if (name==PROPNAME_STRICTNODATA) {
      rspfBooleanProperty* property = new rspfBooleanProperty(name,theStrictNoData);
      property->setCacheRefreshBit();
      return property;
   } else if (name==PROPNAME_CENTEROFFSET) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString(theCenterOffset));
      property->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT); //restrict to int type
      property->setCacheRefreshBit();
      return property;
   } else if (name == PROPNAME_KERNELSIZE) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString((rspf_uint32)theKernel.size()),1.0,100.0);
      property->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT); //restrict to int type
      property->setCacheRefreshBit();
      return property;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfConvolutionFilter1D::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(PROPNAME_KERNELSIZE);
   propertyNames.push_back(PROPNAME_KERNEL);
   propertyNames.push_back(PROPNAME_CENTEROFFSET);
   propertyNames.push_back(PROPNAME_ISHZ);
   propertyNames.push_back(PROPNAME_STRICTNODATA);
}

bool rspfConvolutionFilter1D::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{   
   kwl.add(prefix,
           PROPNAME_KERNELSIZE,
           static_cast<rspf_uint32>(theKernel.size()),
           true);
   kwl.add(prefix,
          PROPNAME_CENTEROFFSET,
           theCenterOffset,
           true);
   kwl.add(prefix,
          PROPNAME_ISHZ,
           theIsHz?"true":"false", //use string instead of boolean
           true);
   kwl.add(prefix,
          PROPNAME_STRICTNODATA,
           theStrictNoData?"true":"false", //use string instead of boolean
           true);

   for(rspf_uint32 row = 0; row < theKernel.size(); ++row)
   {
         rspfString newPrefix = "k" + rspfString::toString(row);
         kwl.add(prefix,
                 newPrefix,
                 theKernel[row],
                 true);          
   }
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfConvolutionFilter1D::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   //find kernel size
   rspf_uint32 iks=0;
   const char* ks = kwl.find(prefix, PROPNAME_KERNELSIZE);
   if(ks)
   {
      iks = rspfString(ks).toUInt32();
      if (iks<1)
      {
         cerr<<"rspfConvolutionFilter1D : warning bad "<< PROPNAME_KERNELSIZE <<" in state"<<endl;
         iks=0;
      }
   } else {
      iks=0;
      cerr<<"rspfConvolutionFilter1D : warning no "<< PROPNAME_KERNELSIZE <<" in state"<<endl;
   }

   //load other props
   const char* co = kwl.find(prefix, PROPNAME_CENTEROFFSET);
   if(co)
   {      
      setCenterOffset(rspfString(co).toUInt32());
   } else {
      cerr<<"rspfConvolutionFilter1D : warning no "<< PROPNAME_CENTEROFFSET<<" in state"<<endl;
   }
   const char* ih = kwl.find(prefix, PROPNAME_ISHZ);
   if(ih)
   {      
      setIsHorizontal(rspfString(ih).toBool());
   } else {
      cerr<<"rspfConvolutionFilter1D : warning no "<<PROPNAME_ISHZ<<" in state"<<endl;
   }
   const char* sn = kwl.find(prefix, PROPNAME_STRICTNODATA);
   if(sn)
   {      
      setStrictNoData(rspfString(sn).toBool());
   } else {
      cerr<<"rspfConvolutionFilter1D : warning no "<<PROPNAME_STRICTNODATA<<" in state"<<endl;
   }
   //load kernel values
   rspfString newPrefix = prefix;
   newPrefix += rspfString("k");
   theKernel.resize(iks);
   for(rspf_uint32 r=0; r<iks; ++r)
   {      
      rspfString value = rspfString::toString(r);
      
      const char* v = kwl.find(newPrefix.c_str(),
                               value.c_str());
      if(v)
      {
         theKernel[r] = rspfString(v).toDouble();
      } else {
         cerr<<"rspfConvolutionFilter1D : warning no value for "<<newPrefix<<" in state"<<endl;
         theKernel[r] = 0.0;
      }
   }
   clearNullMinMax();
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfConvolutionFilter1D::setKernel(const std::vector<rspf_float64>& aKernel)
{
   theKernel = aKernel;
   // Will be recomputed first getTile call.
   clearNullMinMax();
}

double rspfConvolutionFilter1D::getNullPixelValue(rspf_uint32 band)const
{
   if( isSourceEnabled() && (band < theNullPixValue.size()) )
   {
      return theNullPixValue[band];
   }

   return rspf::defaultNull(getOutputScalarType());
}

double rspfConvolutionFilter1D::getMinPixelValue(rspf_uint32 band)const
{
   if( isSourceEnabled() && (band < theMinPixValue.size()) )
   {
      return theMinPixValue[band];
   }
   
   return rspfImageSource::getMinPixelValue(band);
}

double rspfConvolutionFilter1D::getMaxPixelValue(rspf_uint32 band)const
{
   if( isSourceEnabled() && (band < theMaxPixValue.size()) )
   {
      return theMaxPixValue[band];
   }

   return rspfImageSource::getMaxPixelValue(band);
}

void rspfConvolutionFilter1D::clearNullMinMax()
{
   theNullPixValue.clear();
   theMinPixValue.clear();
   theMaxPixValue.clear();
}

void rspfConvolutionFilter1D::computeNullMinMax()
{
   const rspf_uint32 BANDS = getNumberOfOutputBands();

   theNullPixValue.resize(BANDS);
   theMinPixValue.resize(BANDS);
   theMaxPixValue.resize(BANDS);

   rspf_float64 defaultNull = rspf::defaultNull(getOutputScalarType());
   rspf_float64 defaultMin = rspf::defaultMin(getOutputScalarType());
   rspf_float64 defaultMax = rspf::defaultMax(getOutputScalarType());
  
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      if(theInputConnection)
      {
         rspf_float64 inputNull = theInputConnection->getNullPixelValue(band);
         rspf_float64 inputMin  = theInputConnection->getMinPixelValue(band);
         rspf_float64 inputMax  = theInputConnection->getMaxPixelValue(band);
         rspf_float64 tempMin   = 0.0;
         rspf_float64 tempMax   = 0.0;
         rspf_float64 k         = 0.0;
         for(rspf_uint32 i=0;i<theKernel.size();++i)
         {          
               k=theKernel[i];
               tempMin += (k<0.0) ? k*inputMax : k*inputMin;
               tempMax += (k>0.0) ? k*inputMax : k*inputMin;          
         }
         
         if((tempMin >= defaultMin) && (tempMin <= defaultMax))
         {
            theMinPixValue[band] = tempMin;
         }
         else
         {
            theMinPixValue[band] = defaultMin;
         }

         if((tempMax >= defaultMin) && (tempMax <= defaultMax))
         {
            theMaxPixValue[band] = tempMax;
         }
         else
         {
            theMaxPixValue[band] = defaultMax;
         }
         if((inputNull < theMinPixValue[band]) ||
            (inputNull > theMaxPixValue[band]))
         {
            theNullPixValue[band] = inputNull;
         }
         else
         {
            theNullPixValue[band] = defaultNull;
         }
         
      }
      else // No input connection...
      {
         theNullPixValue[band] = defaultNull;
         theMinPixValue[band]  = defaultMin;
         theMaxPixValue[band]  = defaultMax;
      }
      
   } // End of band loop.
}
