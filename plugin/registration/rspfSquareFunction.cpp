
#include "rspfSquareFunction.h"
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfMatrixProperty.h>

RTTI_DEF1(rspfSquareFunction, "rspfSquareFunction", rspfImageSourceFilter);

rspfSquareFunction::rspfSquareFunction(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theTile(NULL)
{
}

rspfSquareFunction::~rspfSquareFunction()
{
}

rspfRefPtr<rspfImageData> rspfSquareFunction::getTile(
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

   rspfRefPtr<rspfImageData> data = theInputConnection->getTile(tileRect,
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
      case RSPF_UINT8:
      { 
         applyFunction(static_cast<rspf_uint8>(0), data, theTile);
         break;
      }
      case RSPF_SINT8:
      { 
         applyFunction(static_cast<rspf_sint8>(0), data, theTile);
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         applyFunction(static_cast<rspf_uint16>(0), data, theTile);         
         break;
      }
      case RSPF_SINT16:
      {
         applyFunction(static_cast<rspf_sint16>(0), data, theTile);         
         break;
      }
      case RSPF_UINT32:
      {
         applyFunction(static_cast<rspf_uint32>(0), data, theTile);         
         break;
      }
      case RSPF_SINT32:
      {
         applyFunction(static_cast<rspf_sint32>(0), data, theTile);         
         break;
      }
      case RSPF_FLOAT32: 
      case RSPF_NORMALIZED_FLOAT:
      {
         applyFunction(static_cast<rspf_float32>(0), data, theTile);         
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         applyFunction(static_cast<rspf_float64>(0), data, theTile);
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfSquareFunction::getTile WARNING:\n"
            << "Scalar type = " << theTile->getScalarType()
            << " Not supported by rspfSquareFunction" << endl;
         break;
      }
   }
   theTile->validate();
   
   return theTile;
}


template<class T> 
void
rspfSquareFunction::applyFunction( T,
   rspfRefPtr<rspfImageData> inputData,
   rspfRefPtr<rspfImageData> outputData)
{
   // let's set up some temporary variables so we don't
   // have to call the functions in loops.  Iknow that compilers
   // typically optimize this out but if we are in debug mode 
   // with no optimization it will still run fast
   //  
   double v;
   rspf_uint32 outputW = outputData->getWidth();
   rspf_uint32 outputH = outputData->getHeight();
   for(rspf_uint32 band = 0; band < getNumberOfInputBands(); ++band)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(band));
      T* outputBuf = static_cast<T*>(outputData->getBuf(band));
      T maxPix     = static_cast<T>(getMaxPixelValue(band));
      T minPix     = static_cast<T>(getMinPixelValue(band));     
      T nullPix    = static_cast<T>(inputData->getNullPix(band));
      T oNullPix   = static_cast<T>(getNullPixelValue(band));
      
      if(inputBuf&&outputBuf)
      {
         for(rspf_uint32 row = 0; row < outputW; ++row)
         {
            for(rspf_uint32 col = 0; col < outputH; ++col)
            {

               if(*inputBuf != nullPix)
               {
                  v = *inputBuf * (*inputBuf);
                  if(v > maxPix)
                  {
                     *outputBuf = maxPix;
                  }
                  else if(v < minPix)
                  {
                     *outputBuf = minPix;
                  }
                  else
                  {
                     *outputBuf = static_cast<T>(v);
                  }
               }
               else {
                  *outputBuf = oNullPix;
               }
               ++inputBuf;
               ++outputBuf;
            }
         }
      }
   }
}


void rspfSquareFunction::initialize()
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

void rspfSquareFunction::allocate()
{   
   if(theInputConnection)
   {
      rspfImageDataFactory* idf = rspfImageDataFactory::instance();
      
      theTile = idf->create(this,
                            this);
      
      theTile->initialize();
   }
}

double rspfSquareFunction::getNullPixelValue(rspf_uint32 band)const
{
   if( isSourceEnabled() && (band < theMinPixValue.size()) )
   {
      return theNullPixValue[band];
   }
   return rspfImageSource::getNullPixelValue(band);
}

double rspfSquareFunction::getMinPixelValue(rspf_uint32 band)const
{
   if( isSourceEnabled() && (band < theMinPixValue.size()) )
   {
      return theMinPixValue[band];
   }   
   return rspfImageSource::getMinPixelValue(band);
}

double rspfSquareFunction::getMaxPixelValue(rspf_uint32 band)const
{
   if( isSourceEnabled() && (band < theMaxPixValue.size()) )
   {
      return theMaxPixValue[band];
   }
   return rspfImageSource::getMaxPixelValue(band);
}

void rspfSquareFunction::clearNullMinMax()
{
   theNullPixValue.clear();
   theMinPixValue.clear();
   theMaxPixValue.clear();
}

void rspfSquareFunction::computeNullMinMax()
{
   const rspf_uint32 BANDS = getNumberOfOutputBands();

   theNullPixValue.resize(BANDS);
   theMinPixValue.resize(BANDS);
   theMaxPixValue.resize(BANDS);

   rspf_float64 defaultNull = rspf::defaultNull(getOutputScalarType());
   rspf_float64 defaultMin = rspf::defaultMin(getOutputScalarType());
   rspf_float64 defaultMax = rspf::defaultMax(getOutputScalarType());
  
   rspfImageSource* input  = PTR_CAST(rspfImageSource, getInput(0));
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      if(theInputConnection)
      {
         double span[2] = { input->getMinPixelValue(band)  , input->getMaxPixelValue(band)  };
         double v;
         double minv = defaultMax; //crossed min-max
         double maxv = defaultMin;
         for (int l=0;l<2;++l)
         {
            for (int r=0;r<2;++r)
            {
               v = span[l] * span[r];
               if (v < minv) minv=v;
               if (v > maxv) maxv=v;
            }
         }         
         theMinPixValue[band] = (minv>defaultMin) ? minv : defaultMin ;
         theMaxPixValue[band] = (maxv<defaultMax) ? maxv : defaultMax ;
      }
      else // No input connection...
      {    
         theMinPixValue[band]  = defaultMin;
         theMaxPixValue[band]  = defaultMax;
      }
      //always default for null
      theNullPixValue[band] = defaultNull;
      
   } // End of band loop.
}

