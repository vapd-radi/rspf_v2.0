
#include "rspfDensityReducer.h"
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfNumericProperty.h>

#include <algorithm>

using namespace std;

RTTI_DEF1(rspfDensityReducer, "rspfDensityReducer", rspfImageSourceFilter);

static const char* PROP_MAXDENSITY  = "MaxDensity";

//functor for decreasing comparison (strict weak ordering, like greater), for pairs on the first element
template<class T> class greaterFirst
{
   public:
      bool operator()(const T& x, const T& y)
      {
         return x.first > y.first;
      }
};

rspfDensityReducer::rspfDensityReducer(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theTile(NULL),
    theMaxDensity(1.0)
{
}

rspfDensityReducer::rspfDensityReducer(rspfImageSource* inputSource)
   :rspfImageSourceFilter(inputSource),
    theTile(NULL),
    theMaxDensity(1.0)
{
}

rspfDensityReducer::rspfDensityReducer(rspfObject* owner,
                                   rspfImageSource* inputSource)
   :rspfImageSourceFilter(owner, inputSource),
    theTile(NULL),
    theMaxDensity(1.0)
{
}

rspfRefPtr<rspfImageData> 
rspfDensityReducer::getTile(const rspfIrect& rect, rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(rect, resLevel);
   }

   rspfRefPtr<rspfImageData> inputData =
      rspfImageSourceFilter::getTile(rect, resLevel);

   if(!inputData.valid() || (!inputData->getBuf()) || (inputData->getDataObjectStatus() ==  RSPF_EMPTY) || (getMaxDensity()>=1.0))
   {
      return inputData;
   }

   if(!theTile.valid()) initialize();
   if(!theTile.valid()) return theTile;
   
   theTile->setImageRectangleAndBands(rect, inputData->getNumberOfBands());
   
   switch(theTile->getScalarType())
   {
   case RSPF_UCHAR:
   {
      runFilter((rspf_uint8)0,
                inputData);
      break;
   }
   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
   {
      runFilter((rspf_float32)0,
                inputData);
      break;
   }
   case RSPF_USHORT16:
   case RSPF_USHORT11:
   {
      runFilter((rspf_uint16)0,
                inputData);
      break;
   }
   case RSPF_SSHORT16:
   {
      runFilter((rspf_sint16)0,
                inputData);
      break;
   }
   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
   {
      runFilter((rspf_float64)0,
                inputData);
      break;
   }
   default:
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfDensityReducer::getTile WARN: Scalar type = " << theTile->getScalarType()
                                         << " Not supported by rspfDensityReducer" << std::endl;
      break;
   }
   }

   return theTile;
}

void rspfDensityReducer::initialize()
{
   rspfImageSourceFilter::initialize();

   theTile = NULL;

   if(!isSourceEnabled())
   {
      return;
   }
   
   theTile = rspfImageDataFactory::instance()->create(this, this);
   if(theTile.valid())
   {
      theTile->initialize();
   }

}

void rspfDensityReducer::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if (property->getName() == PROP_MAXDENSITY)  {
      rspfNumericProperty* numProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(numProperty)
      {
         setMaxDensity(numProperty->asFloat64());
      }
   }  else {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfDensityReducer::getProperty(const rspfString& name)const
{
   if (name == PROP_MAXDENSITY) {
      rspfNumericProperty* property = new rspfNumericProperty(name, rspfString::toString(getMaxDensity()),0.0,1.0);
      property->setCacheRefreshBit();
      return property;
   } 
   return rspfImageSourceFilter::getProperty(name);
}

void rspfDensityReducer::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back(PROP_MAXDENSITY);
}


template <class T>
void rspfDensityReducer::runFilter(T dummyVariable,
                                rspfRefPtr<rspfImageData> inputData)
{
   // so far neighborhood is 3x3
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
  
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_uint32 tsize  = width * height; //surely a tile cannot hold more than 4 billion pixels?

   //max number of pixels
   if (getMaxDensity()<=0)
   {
      //nullify tile
      theTile->makeBlank();
      return;
   }
   //we always want at least one point (if density > 0)
   rspf_uint32 maxp = (rspf_uint32)ceil(getMaxDensity() * tsize);
   if (maxp >= tsize)
   {
      //return everything
      theTile = inputData; //TBC: should we make a deep copy?
      return;
   }

   //initially nullify output tile
   theTile->makeBlank();

   //init a storage vector for non null pixels : pre-allocate maximum size
   // stores value and offset from start
   // will be used for all bands
   vector< pair< T , rspf_uint32 > > sortv(tsize);

   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(inputData->getNullPix(bandIdx)); //changed to input Null            
      if(inputBuf&&outputBuf)
      {
         //get only non null pixels, and store offset
         typename vector< pair< T , rspf_uint32 > >::iterator it = sortv.begin();
         rspf_uint32 o;
         for(o = 0; o < tsize; ++o,++it)
         {
            if (inputBuf[o] != np)
            {
               it->first  = inputBuf[o] ;
               it->second = o ;
            }
         }

         //o is now the number of valid elemnts in the vector
         //arrange vector so that the maxp first elements have larger value than the others
         if (maxp<o)
         {
            nth_element< typename vector< pair< T , rspf_uint32 > >::iterator , greaterFirst< pair< T , rspf_uint32 > > >(
               sortv.begin(),
               sortv.begin()+maxp, 
               sortv.begin()+o, 
               greaterFirst< pair< T , rspf_uint32 > >());
            o=maxp;
         } //else : density below max, nothing to do

         //only display elements between offsets maxp and o
         typename vector< pair< T , rspf_uint32 > >::iterator nit = sortv.begin();
         for (rspf_uint32 p=0; p<o; ++p,++nit)
         {
            outputBuf[nit->second] = nit->first;
         }

      }
   }
   theTile->validate();
}

bool rspfDensityReducer::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{  
   kwl.add(prefix,
          PROP_MAXDENSITY,
          rspfString::toString(getMaxDensity()),
          true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfDensityReducer::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* md = kwl.find(prefix,PROP_MAXDENSITY );
   if(md)
   {
      setMaxDensity(rspfString::toDouble(md));
   } else {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfDensityReducer::loadState WARN: no "<<PROP_MAXDENSITY<<" found" << std::endl;
   }  
   return rspfImageSourceFilter::loadState(kwl, prefix);
}
