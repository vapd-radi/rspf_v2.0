
#include "rspfExtremaFilter.h"
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfBooleanProperty.h>

RTTI_DEF1(rspfExtremaFilter, "rspfExtremaFilter", rspfImageSourceFilter);

static const char* PROP_ISSTRICT  = "IsStrict";
static const char* PROP_ISMAXIMUM = "IsMaximum";

rspfExtremaFilter::rspfExtremaFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theTile(NULL),
    theIsMaximum(true),
    theIsStrict(true)
{
}

rspfExtremaFilter::rspfExtremaFilter(rspfImageSource* inputSource)
   :rspfImageSourceFilter(inputSource),
    theTile(NULL),
    theIsMaximum(true),
    theIsStrict(true)
{
}

rspfExtremaFilter::rspfExtremaFilter(rspfObject* owner,
                                   rspfImageSource* inputSource)
   :rspfImageSourceFilter(owner, inputSource),
    theTile(NULL),
    theIsMaximum(true),
    theIsStrict(true)
{
}

rspfRefPtr<rspfImageData> 
rspfExtremaFilter::getTile(const rspfIrect& rect, rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(rect, resLevel);
   }

   // expand the rect out to cover the 3x3 horizontal and vertical kernel.
   rspfIrect requestRect = rect;

   adjustRequestRect(requestRect);
   
   rspfRefPtr<rspfImageData> inputData =
      rspfImageSourceFilter::getTile(requestRect, resLevel);

   if(!inputData.valid() || (!inputData->getBuf()))
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
      rspfNotify(rspfNotifyLevel_WARN) << "rspfExtremaFilter::getTile WARN: Scalar type = " << theTile->getScalarType()
                                         << " Not supported by rspfExtremaFilter" << std::endl;
      break;
   }
   }

   return theTile;
}

void rspfExtremaFilter::initialize()
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

void rspfExtremaFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if (property->getName() == PROP_ISMAXIMUM)  {
      rspfBooleanProperty* booleanProperty = PTR_CAST(rspfBooleanProperty,
                                                     property.get());
      if(booleanProperty)
      {
         setIsMaximum(booleanProperty->getBoolean());
      }
   } else if (property->getName() == PROP_ISSTRICT)  {
      rspfBooleanProperty* booleanProperty = PTR_CAST(rspfBooleanProperty,
                                                     property.get());
      if(booleanProperty)
      {
         setIsStrict(booleanProperty->getBoolean());
      }
   } else {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfExtremaFilter::getProperty(const rspfString& name)const
{
   if (name == PROP_ISMAXIMUM) {
      rspfBooleanProperty* property = new rspfBooleanProperty(name, isMaximum());
      property->setCacheRefreshBit();
      return property;
   } else if (name == PROP_ISSTRICT) {
      rspfBooleanProperty* property = new rspfBooleanProperty(name, isStrict());
      property->setCacheRefreshBit();
      return property;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfExtremaFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back(PROP_ISMAXIMUM);
   propertyNames.push_back(PROP_ISSTRICT);
}


void rspfExtremaFilter::adjustRequestRect(rspfIrect& requestRect)const
{
   requestRect = rspfIrect(requestRect.ul().x - 1,
                            requestRect.ul().y - 1,
                            requestRect.lr().x + 1,
                            requestRect.lr().y + 1);
}

template <class T>
void rspfExtremaFilter::runFilter(T dummyVariable,
                                rspfRefPtr<rspfImageData> inputData)
{
   // so far neighborhood is 3x3
   rspf_uint32 bandIdx = 0;
   rspf_uint32 numberOfBands = inputData->getNumberOfBands();
  
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 width  = theTile->getWidth();
   rspf_uint32 height = theTile->getHeight();
   rspf_int32 rowIncrement  = inputData->getWidth();
   rspf_int32 rowIncrement2 = 2*inputData->getWidth(); 
         
   T centerv;
   T extremum;
   bool center_better;
   for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
   {
      //inputBuf has a 1 pixel edge compared to outputBuf
      T* inputBuf  = static_cast<T*>(inputData->getBuf(bandIdx));
      T* outputBuf = static_cast<T*>(theTile->getBuf(bandIdx));
      T np         = static_cast<T>(inputData->getNullPix(bandIdx)); //changed to input Null            
      if(inputBuf&&outputBuf)
      {
         //one pass: maybe faster if changed to two passes
         T* outB;
         T* inB;
         
      	outB = outputBuf;         
         inB  = inputBuf;         
         for(y = 0; y < height; ++y)
         {
            for(x = 0; x < width; ++x)
            {
               centerv = inB[1+rowIncrement];
               if (centerv != np)
               {
                  if (isMaximum())
                  {
                     extremum = max<T>(
                           max<T>(
                            max<T>(inB[0],inB[1]),
                            max<T>(inB[2],inB[rowIncrement])),
                           max<T>(
                            max<T>(inB[rowIncrement+2],inB[rowIncrement2]),
                            max<T>(inB[rowIncrement2+1],inB[rowIncrement2+2])
                            ));
                     if (isStrict())
                     {
                        center_better = centerv > extremum;
                     } else {
                        center_better = centerv >= extremum;
                     }
                  } else {
                     extremum = min<T>(
                           min<T>(
                            min<T>(inB[0],inB[1]),
                            min<T>(inB[2],inB[rowIncrement])),
                           min<T>(
                            min<T>(inB[rowIncrement+2],inB[rowIncrement2]),
                            min<T>(inB[rowIncrement2+1],inB[rowIncrement2+2])
                            ));
                     if (isStrict())
                     {
                        center_better = centerv < extremum;
                     } else {
                        center_better = centerv <= extremum;
                     }
                  }
                  //nullify if not extremum
                  *outB = center_better ? centerv : np ;
               }
               else
               {
                  *outB = np;
               }
               ++outB;
               ++inB;
            }
            inB+=2; //go to next line, jump due to edge
         }       
      }      
   }
   theTile->validate();
}

bool rspfExtremaFilter::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{  
   kwl.add(prefix,
          PROP_ISMAXIMUM,
          rspfString::toString(isStrict()),
          true);
   kwl.add(prefix,
          PROP_ISSTRICT,
           rspfString::toString(isStrict()),
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfExtremaFilter::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* im = kwl.find(prefix,PROP_ISMAXIMUM );
   if(im)
   {
      setIsMaximum(rspfString::toBool(im));
   } else {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfExtremaFilter::loadState WARN: no "<<PROP_ISMAXIMUM<<" found" << std::endl;
   }
   const char* is = kwl.find(prefix,PROP_ISSTRICT );
   if(is)
   {
      setIsStrict(rspfString::toBool(is));
   } else {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfExtremaFilter::loadState WARN: no "<<PROP_ISSTRICT<<" found" << std::endl;
   }
        
   return rspfImageSourceFilter::loadState(kwl, prefix);
}
