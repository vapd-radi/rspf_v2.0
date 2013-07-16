// class rspfMultiplier implementation
// REQUIRES FFTW version 3.x (Fast Fourier Transform)

#include "rspfMultiplier.h"
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfIrect.h>

RTTI_DEF1( rspfMultiplier, "rspfMultiplier", rspfImageCombiner );

rspfMultiplier::rspfMultiplier()
   :
   rspfImageCombiner()
{
}

rspfMultiplier::rspfMultiplier(rspfObject *owner,         
                                 rspfImageSource *left,
                                 rspfImageSource *right)
   :rspfImageCombiner(owner,
   2,
   0,
   true,
   false)
{
   connectMyInputTo(0, left);
   connectMyInputTo(1, right);
}

rspfMultiplier::~rspfMultiplier()
{ 
}

bool
rspfMultiplier::canConnectMyInputTo(rspf_int32 index,const rspfConnectableObject* object)const
{
   //check that inputs just have one band
   //max 3 inputs
   //TBD: check scalar types
   switch(index)
   {
   case 0:
   case 1:
      {
      rspfImageSource* imagep=PTR_CAST(rspfImageSource, object);
      if (imagep)
      {
         return true;
      } else {
         return false;
      }
      }
      break;
   default:
      return false;
   }
}

rspfRefPtr<rspfImageData>
rspfMultiplier::getTile(const rspfIrect &rect, rspf_uint32 resLevel)
{
   if ( (!theNormTile) || (!theMin.size()) )
   {
      initialize();
   }

   long w = rect.width();
   long h = rect.height();
   long tw = theNormTile->getWidth();
   long th = theNormTile->getHeight();

   theNormTile->setImageRectangle(rect);

   if(w*h != tw*th)
   {
     theNormTile->initialize();
   }
   else
   {
      theNormTile->makeBlank();
   }

   switchTypeRun(rect, resLevel);
   
   return theNormTile;
}

//switchTypeRun: updates theNormTile
void
rspfMultiplier::switchTypeRun(const rspfIrect &tileRect, rspf_uint32 resLevel)
{   
   //get scalar type & check
   if (hasDifferentInputs())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "All input scalar types must be identical"<< endl;
      return;
   }
   //type template
   switch(theNormTile->getScalarType())
   {
      case RSPF_UINT8:
      {
            operate(static_cast<rspf_uint8>(0),
                               tileRect,
                               resLevel);
            break;
      }
      case RSPF_SINT8:
      {
            operate(static_cast<rspf_sint8>(0),
                               tileRect,
                               resLevel);
            break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
           operate(static_cast<rspf_uint16>(0),
                               tileRect,
                               resLevel);
           break;
      }
      case RSPF_SINT16:
      {
           operate(static_cast<rspf_sint16>(0),
                               tileRect,
                               resLevel);
           break;
      }
      case RSPF_SINT32:
      {
            operate(static_cast<rspf_sint32>(0),
                               tileRect,
                               resLevel);
            break;
      }
      case RSPF_UINT32:
      {
            operate(static_cast<rspf_uint32>(0),
                               tileRect,
                               resLevel);
            break;
      }
      case RSPF_FLOAT: 
      case RSPF_NORMALIZED_FLOAT:
      {
           operate(static_cast<rspf_float32>(0),
                               tileRect,
                               resLevel);
           break;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
            operate(static_cast<rspf_float64>(0),
                               tileRect,
                               resLevel);
            break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Scalar type = " << theNormTile->getScalarType()
            << " Not supported by rspfMultiplier" << endl;
         break;
      }
   }
   theNormTile->validate();
}

template <class T>
void
rspfMultiplier::operate( T,// dummy template variable 
                             const rspfIrect& tileRect,
                             rspf_uint32 resLevel)
{
   rspfImageSource* left  = PTR_CAST(rspfImageSource, getInput(0));
   rspfImageSource* right = PTR_CAST(rspfImageSource, getInput(1));
   double v;
   if (left && right)
   {
      rspfRefPtr<rspfImageData> leftD  = left->getTile(tileRect, resLevel);
      rspfRefPtr<rspfImageData> rightD = right->getTile(tileRect, resLevel);
      if ((leftD.valid()) && (rightD.valid()) && (leftD->getDataObjectStatus() != RSPF_EMPTY) && (rightD->getDataObjectStatus() != RSPF_EMPTY))
      {
         rspf_uint32 upperBound = theNormTile->getWidth() * theNormTile->getHeight();
         for(rspf_uint32 band = 0; band < theNormTile->getNumberOfBands(); ++band)
         {
            T* leftBand      = static_cast<T*>(leftD->getBuf(band));
            T* rightBand     = static_cast<T*>(rightD->getBuf(band));
            T* destBand      = static_cast<T*>(theNormTile->getBuf(band));
            double minv      = theMin[band];
            double maxv      = theMax[band];
            double leftNull  = leftD->getNullPix(band);
            double rightNull = rightD->getNullPix(band);

            if(leftBand&&rightBand)
            {
               for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
               {
                  if (((double)*leftBand != leftNull) && ((double)*rightBand != rightNull))
                  {
                     v = *leftBand * (*rightBand);
                     //must still check for "overflows"
                     if (v > maxv) v = maxv;
                     if (v < minv) v = minv;
                  
                     *destBand = static_cast<T>(v);
                  } //else : destination already blank
                  
                  ++leftBand; ++rightBand; ++destBand;
               }
            }
         }
      }
   }
}

void 
rspfMultiplier::initialize()
{
   rspfImageCombiner::initialize();
   theNormTile = rspfImageDataFactory::instance()->create(this, this);
   theNormTile->initialize();  //TBD: change tile creation so it isn't within initialize 
   computeMinMax(); //TBC : is it the right place?
}

double
rspfMultiplier::getNullPixelValue(rspf_uint32 band)const
{
   return rspf::defaultNull(getOutputScalarType());
}

double
rspfMultiplier::getMinPixelValue(rspf_uint32 band)const
{
   if (band<theMin.size())
   {
      return theMin[band];
   } else {
      return rspf::defaultMin(getOutputScalarType());
   }
}

double 
rspfMultiplier::getMaxPixelValue(rspf_uint32 band)const
{
   if (band<theMax.size())
   {
      return theMax[band];
   } else {
      return rspf::defaultMax(getOutputScalarType());
   }
}

void
rspfMultiplier::computeMinMax()
{
   // use monotony of (x,y) |-> x*y on lines and rows
   // => test only four corners and keep min/max
   rspfImageSource* left  = PTR_CAST(rspfImageSource, getInput(0));
   rspfImageSource* right = PTR_CAST(rspfImageSource, getInput(1));

   //maximum span: default values
   double defMin = rspf::defaultMin(getOutputScalarType());
   double defMax = rspf::defaultMax(getOutputScalarType());

   theMin.clear();
   theMax.clear();

   //new values
   if (left && right)
   {
      for (rspf_uint32 band=0;band<getNumberOfOutputBands();++band)
      {
         double leftspan[2] = { left->getMinPixelValue(band)  , left->getMaxPixelValue(band)  };
         double rightspan[2]= { right->getMinPixelValue(band) , right->getMaxPixelValue(band) };
         double v;
         double minv = defMax; //crossed min-max
         double maxv = defMin;
         for (int l=0;l<2;++l)
         {
            for (int r=0;r<2;++r)
            {
               v = leftspan[l] * rightspan[r];
               if (v < minv) minv=v;
               if (v > maxv) maxv=v;
            }
         }         
         theMin.push_back( (minv>defMin) ? minv : defMin );
         theMax.push_back( (maxv<defMax) ? maxv : defMax );
      }
   }
}

rspf_uint32
rspfMultiplier::getNumberOfOutputBands()const
{   
   rspfImageSource* left  = PTR_CAST(rspfImageSource, getInput(0));
   rspfImageSource* right = PTR_CAST(rspfImageSource, getInput(1));
   if (left && right)
   {
      return std::min( left->getNumberOfOutputBands(), right->getNumberOfOutputBands() );
   } else {
      return 0;
   }
}

rspfIrect
rspfMultiplier::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfImageSource* left  = PTR_CAST(rspfImageSource, getInput(0));
   rspfImageSource* right = PTR_CAST(rspfImageSource, getInput(1));
   rspfIrect res;
   if (left && right)
   {
     //Intersect rectangles
     res = left->getBoundingRect(resLevel);
     res.clipToRect(right->getBoundingRect(resLevel));
   } 
   else
   {
      res.makeNan();
   }
   return res;
}

