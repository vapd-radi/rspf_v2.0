// class rspfChipMatch implementation
// REQUIRES FFTW version 3.x (Fast Fourier Transform)

#include "rspfChipMatch.h"
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfIrect.h>
#include "rspfRunningSum.h"
#include "rspfNCC_FFTW.h"
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>

#include <fftw3.h>

#include <iostream> //TBR

RTTI_DEF1( rspfChipMatch, "rspfChipMatch", rspfImageCombiner );

// matrix to get the 2nd order x,y best fit polynomial (least mean squares)
// -order of values (inputs) : from top left to bottom right along rows (normal image scan)
// -order of coefficients (results) : 1 x y xy xx yy
// uniform weighting for least mean squares

double rspfChipMatch::theLMS[6*9] = {
-1.1111111111111116e-001,2.2222222222222210e-001,-1.1111111111111116e-001,2.2222222222222210e-001,5.5555555555555536e-001,2.2222222222222210e-001,-1.1111111111111116e-001,2.2222222222222210e-001,-1.1111111111111116e-001,
-1.6666666666666666e-001,0.0000000000000000e+000,1.6666666666666666e-001,-1.6666666666666666e-001,0.0000000000000000e+000,1.6666666666666666e-001,-1.6666666666666666e-001,0.0000000000000000e+000,1.6666666666666666e-001,
-1.6666666666666666e-001,-1.6666666666666666e-001,-1.6666666666666666e-001,0.0000000000000000e+000,0.0000000000000000e+000,0.0000000000000000e+000,1.6666666666666666e-001,1.6666666666666666e-001,1.6666666666666666e-001,
2.5000000000000000e-001,0.0000000000000000e+000,-2.5000000000000000e-001,0.0000000000000000e+000,0.0000000000000000e+000,0.0000000000000000e+000,-2.5000000000000000e-001,0.0000000000000000e+000,2.5000000000000000e-001,
1.6666666666666669e-001,-3.3333333333333331e-001,1.6666666666666669e-001,1.6666666666666674e-001,-3.3333333333333326e-001,1.6666666666666674e-001,1.6666666666666669e-001,-3.3333333333333331e-001,1.6666666666666669e-001,
1.6666666666666669e-001,1.6666666666666674e-001,1.6666666666666669e-001,-3.3333333333333331e-001,-3.3333333333333326e-001,-3.3333333333333331e-001,1.6666666666666669e-001,1.6666666666666674e-001,1.6666666666666669e-001
};

rspfChipMatch::rspfChipMatch()
   :rspfImageCombiner(),
   theSlaveAccuracy(7.0), //TBC
   theMRadius(5), //TBC
   theBias(0.0,0.0),
   theMinNCC(0.75),
   theNCCengine(NULL),
   theTile(NULL)
{
}

rspfChipMatch::rspfChipMatch(rspfObject *owner,         
                               rspfImageSource *centers,
                               rspfImageSource *master, 
                               rspfImageSource *slave   )
   :rspfImageCombiner(owner,
   3,
   0,
   true,
   false),
   theSlaveAccuracy(7.0), //TBC: set to 0
   theMRadius(5), //TBC
   theBias(0.0,0.0),
   theMinNCC(0.75),
   theNCCengine(NULL),
   theTile(NULL)
{
   connectMyInputTo(0, centers);
   connectMyInputTo(1, master);
   connectMyInputTo(2, slave);
}

rspfChipMatch::~rspfChipMatch()
{
   //no need to erase theTile (automatic)
   if (theNCCengine!=NULL)
   {
      delete theNCCengine;
      theNCCengine=NULL;
   }
}

bool
rspfChipMatch::canConnectMyInputTo(rspf_int32 index,const rspfConnectableObject* object)const
{
   //check that inputs just have one band
   //max 3 inputs
   //TBD: check scalar types
   switch(index)
   {
   case 0:
   case 1:
   case 2:
      {
      rspfImageSource* imagep=PTR_CAST(rspfImageSource, object);
      if (imagep)
      {
         //return (imagep->getNumberOfOutputBands()==1);
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

rspfScalarType
rspfChipMatch::getOutputScalarType()const
{
   return RSPF_NORMALIZED_DOUBLE;
}

rspfRefPtr<rspfImageData>
rspfChipMatch::getTile(const rspfIrect &rect, rspf_uint32 resLevel)
{
   if(!theTile)
   {
      initialize();
   }
   long w = rect.width();
   long h = rect.height();
   long tw = theTile->getWidth();
   long th = theTile->getHeight();

   theTile->setImageRectangle(rect);
      
   if(w*h != tw*th)
   {
     theTile->initialize();
   }
   else
   {
      theTile->makeBlank();
   }
   
   if (runMatch(rect, resLevel))
   {
      //rasterize tie-points
      rspf_float64* tilebuf = static_cast<rspf_float64*>(theTile->getBuf(0));
      vector<rspfTDpt>::const_iterator it = theTies.begin();
      int xc,yc;
      double x0=rect.ul().x;
      double y0=rect.ul().y;
      for (;it!=theTies.end();++it)
      {
         //remove upperleft corner and round
         xc=(int)std::floor(it->x - x0 + 0.5); //TBC : overflow possible?
         yc=(int)std::floor(it->y - y0 + 0.5);
         tilebuf[ w*yc + xc] = it->score;
      }
      theTile->validate();
   }
   return theTile;
}

// returned vector is destroyed after subsequent calls to getFeatures
const vector<rspfTDpt>&
rspfChipMatch::getFeatures(const rspfIrect &rect, rspf_uint32 resLevel)
{
   runMatch(rect, resLevel);
   return theTies;
}

void 
rspfChipMatch::initialize()
{
   rspfImageCombiner::initialize();
   theTile = rspfImageDataFactory::instance()->create(this, getOutputScalarType(), getNumberOfOutputBands(), getTileWidth(), getTileHeight());
   theTile->initialize();
   //TBD: change tile creation so it isn't within initialize 
}

double
rspfChipMatch::getNullPixelValue(rspf_uint32 /*band*/)const
{
   return rspf::defaultNull(getOutputScalarType());
}

double
rspfChipMatch::getMinPixelValue(rspf_uint32 /*band*/)const
{
   return rspf::defaultMin(getOutputScalarType());
}

double 
rspfChipMatch::getMaxPixelValue(rspf_uint32 /*band*/)const
{
      return rspf::defaultMax(getOutputScalarType());
}

void 
rspfChipMatch::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(property->getName() == RSPF_CHIPMATCH_PIXELRADIUS_PROPNAME)
   {
      //take ceiling of floating point value
      setMasterRadius((rspf_uint32)(property->valueToString().toDouble()));//std::ceil(dynamic_cast<const rspfNumericProperty*>(property.get())->asFloat64())));
   } 
   else if (property->getName() == RSPF_CHIPMATCH_SLAVEACCURACY_PROPNAME)
   {
      setSlaveAccuracy(property->valueToString().toDouble());//dynamic_cast<const rspfNumericProperty*>(property.get())->asFloat64());
   }
   else if (property->getName() == RSPF_CHIPMATCH_MINNCC_PROPNAME)
   {
      setMinNCC(property->valueToString().toDouble());//property.get())->asFloat64());
   }
   else
   {
      rspfImageCombiner::setProperty(property);
   }
}

rspfRefPtr<rspfProperty>
rspfChipMatch::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> resultProperty;
   if(name == RSPF_CHIPMATCH_PIXELRADIUS_PROPNAME)
   {
      rspfNumericProperty* prProperty = new rspfNumericProperty(name,rspfString::toString(getMasterRadius()));
      prProperty->setConstraints(0.0,50.0); //TBD: change to constants
      prProperty->setCacheRefreshBit();
      resultProperty = prProperty;
   }
   else if (name == RSPF_CHIPMATCH_SLAVEACCURACY_PROPNAME)
   {
      rspfNumericProperty* prProperty = new rspfNumericProperty(name,rspfString::toString(getSlaveAccuracy()));
      prProperty->setConstraints(0.0,100000.0); //TBD: change to constants
      prProperty->setCacheRefreshBit();
      resultProperty = prProperty;
   }
   else if(name == RSPF_CHIPMATCH_MINNCC_PROPNAME)
   {
      rspfNumericProperty* prProperty = new rspfNumericProperty(name,rspfString::toString(getMinNCC()));
      prProperty->setConstraints(-1.0,1.0); //TBD: change to constants
      prProperty->setCacheRefreshBit();
      resultProperty = prProperty;
   }
   else
   {
      resultProperty = rspfImageCombiner::getProperty(name);
   }
   return resultProperty;
}

void 
rspfChipMatch::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageCombiner::getPropertyNames(propertyNames);
   
   propertyNames.push_back(RSPF_CHIPMATCH_PIXELRADIUS_PROPNAME);
   propertyNames.push_back(RSPF_CHIPMATCH_SLAVEACCURACY_PROPNAME);
   propertyNames.push_back(RSPF_CHIPMATCH_MINNCC_PROPNAME);
}

bool rspfChipMatch::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   
   kwl.add(prefix,
           RSPF_CHIPMATCH_SLAVEACCURACY_PROPNAME,
           rspfString::toString(getSlaveAccuracy()).c_str(),
           true);

   kwl.add(prefix,
           RSPF_CHIPMATCH_PIXELRADIUS_PROPNAME,
           rspfString::toString(getMasterRadius()).c_str(),
           true);

   kwl.add(prefix,
           RSPF_CHIPMATCH_MINNCC_PROPNAME,
           rspfString::toString(getMinNCC()).c_str(),
           true);
   
   return rspfImageCombiner::saveState(kwl,
                                        prefix);
}

bool rspfChipMatch::loadState(const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   const char* sa = kwl.find(prefix, RSPF_CHIPMATCH_SLAVEACCURACY_PROPNAME);
   if(sa)
   {
      setSlaveAccuracy(rspfString(sa).toDouble());
   }

   const char* pr = kwl.find(prefix, RSPF_CHIPMATCH_PIXELRADIUS_PROPNAME);
   if(pr)
   {      
      setMasterRadius(rspfString(pr).toUInt32());
   }

   const char* mn = kwl.find(prefix, RSPF_CHIPMATCH_MINNCC_PROPNAME);
   if(mn)
   {
      setMinNCC(rspfString(mn).toDouble());
   }

   bool result = rspfImageCombiner::loadState(kwl,
                                               prefix);
   
   return result;
}

rspf_uint32
rspfChipMatch::getNumberOfOutputBands()const
{
   return 1;
}

rspfIrect
rspfChipMatch::getBoundingRect(rspf_uint32 resLevel)const
{
   //intersect master+slave and add buffer (accuracy)
   rspfImageSource* master = PTR_CAST(rspfImageSource, getInput(1)); //use corner input only
   rspfImageSource* slave  = PTR_CAST(rspfImageSource, getInput(2)); //use corner input only
   
   rspfIrect res;

   if (master && slave)
   {
     //Intersect rectangles
     res = master->getBoundingRect(resLevel);
     res.clipToRect(slave->getBoundingRect(resLevel));
     //buffer with accuracy in pixels
     //accuracy independent from resLevel
     rspf_int32 iacc = (rspf_int32)ceil(getSlaveAccuracy());
     rspfIpt delta_ur(iacc, -iacc); //image axes used

     res.set_ur(res.ur() + delta_ur);
     res.set_ll(res.ll() - delta_ur);

   } else {
     res.makeNan();
   }
   return res;
}

rspf_uint32
rspfChipMatch::getTileWidth() const
{
   rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(0));
   if(input)
   {
      return input->getTileWidth();
   }   
   return 0;
}

rspf_uint32
rspfChipMatch::getTileHeight() const
{
   rspfImageSource* input = PTR_CAST(rspfImageSource, getInput(0));
   if(input)
   {
      return input->getTileHeight();
   }   
   return 0;
}

void
rspfChipMatch::getDecimationFactor(rspf_uint32 resLevel,
                                          rspfDpt& result)const
{
   rspfImageSource* input = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(input)
   {
      input->getDecimationFactor(resLevel, result);
   }
}

void
rspfChipMatch::getDecimationFactors(vector<rspfDpt>& decimations)const
{
   rspfImageSource* input = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(input)
   {
      input->getDecimationFactors(decimations);
   }     
}

rspf_uint32
rspfChipMatch::getNumberOfDecimationLevels()const
{
   rspfImageSource* input = PTR_CAST(rspfImageSource,
                                               getInput(0));
   if(input)
   {
      return input->getNumberOfDecimationLevels();
   } 
   return 0;
}

bool
rspfChipMatch::runMatch(const rspfIrect &rect, rspf_uint32 resLevel)
{
   //erase stored tie points
   theTies.clear();

   //get Inputs
   rspfImageSource* corner = PTR_CAST(rspfImageSource, getInput(0));
   rspfImageSource* master = PTR_CAST(rspfImageSource, getInput(1));
   rspfImageSource* slave  = PTR_CAST(rspfImageSource, getInput(2));
   if (!corner || !master || !slave)
   {
      return false;
   }

   long w = rect.width();
   long h = rect.height();
   
   //get corner data tile (same size as inner tile)
   rspfRefPtr<rspfImageData> cornerData = corner->getTile(rect, resLevel);   
   if(!cornerData.valid() || !isSourceEnabled())
   {
      return false;
   }
   
   //TBD: use pixel size in meters to change delta_lr according to zoom

   if((cornerData->getDataObjectStatus() != RSPF_NULL) && (cornerData->getDataObjectStatus() != RSPF_EMPTY))
   {                  
      //loop on corners (<>NULL & >=2 TBC)
      rspf_uint32 coff=0; //offset (speedup)
      rspf_int32 ci=0;
      rspf_int32 cj=0;
      //chip image radii
      rspfIpt delta_mr(getMasterRadius(), getMasterRadius());
      rspfIpt delta_lr(getMasterRadius() + (rspf_int32)(ceil(theSlaveAccuracy)),
                        getMasterRadius() + (rspf_int32)(ceil(theSlaveAccuracy)) );

      for(cj=0;cj<h;++cj) //rows
      {
         for(ci=0;ci<w;++ci) //cols
         {
            if (!cornerData->isNull(coff,0))
            {
               //get master data for specified center + radius
               // radius doesn't change with resLevel
               rspfIpt delta_mc(ci,cj);
               rspfIrect mrect(rect.ul()+delta_mc-delta_mr, rect.ul()+delta_mc+delta_mr); //square, size 2*radius+1 pixels
               rspfRefPtr<rspfImageData> masterData = master->getTile(mrect, resLevel); //same resLevel?? TBC
               if ((masterData != NULL)
                   && (masterData->getDataObjectStatus() != RSPF_EMPTY) 
                   && (masterData->getDataObjectStatus() != RSPF_PARTIAL))
               {
                  //get slave data with bias & extended frame (use accuracy)
                  //bias & extension change with scale
                  rspfIpt delta_sc(ci+(rspf_int32)floor(theBias.x+0.5),cj+(rspf_int32)floor(theBias.y+0.5)); //biased center : TBD : convert unit to pixels
                  rspfIrect srect(rect.ul()+delta_sc-delta_lr, rect.ul()+delta_sc+delta_lr); //square, size 2*(radius+accuracy)+1 pixels
                  rspfRefPtr<rspfImageData> slaveData = slave->getTile(srect, resLevel); //same resLevel?? TBC
                  if ((slaveData != NULL) 
                      && (slaveData->getDataObjectStatus() != RSPF_EMPTY)
                      && (slaveData->getDataObjectStatus() != RSPF_PARTIAL))
                  {
                     //find normalized cross-correlation maximum
                     //TBD: assuming floating point input
                     double dx=0.0;
                     double dy=0.0;
                     double ncor=0.0;

                     getMaxCorrelation(masterData, slaveData, &dx, &dy, &ncor);
                     
                     //filter on NCC value
                     if (ncor >= theMinNCC)
                     {
                        //create tie point & store
                        theTies.push_back(rspfTDpt( rect.ul()+delta_mc, rspfDpt(dx,dy), ncor ));
                     }
                  }
               }
            }
            ++coff;
         }
      }
      return true;
   }
   return false;
}

void
rspfChipMatch::getMaxCorrelation(rspfRefPtr<rspfImageData> Mchip, rspfRefPtr<rspfImageData> Schip, 
                                  double* pdispx, double* pdispy, double* pcor)
{
   //use FFTW 3.0.1
   //assume displacement between center of master to center of slave buffer
   // Mchip must smaller than Schip (Schip incorporates error buffer)

   rspf_uint32 sx = Schip->getWidth();
   rspf_uint32 sy = Schip->getHeight();
   rspf_uint32 mx = Mchip->getWidth();
   rspf_uint32 my = Mchip->getHeight();
   //cout<<"mx="<<mx<<" my="<<my<<" sx="<<sx<<" sy="<<sy<<endl; //TBR

   int cx=sx+mx-1;
   int cy=sy+my-1;

   if (theNCCengine!=NULL)
   {
      //check correlation size
      if (!theNCCengine->sameDims(cy,cx))
      {
         //re build NCC engine //TBD : use wisdom
         delete theNCCengine;
         theNCCengine=NULL;
      }
   }
   if (theNCCengine==NULL)
   {
      //build a new NCC engine //TBD : use wisdom
      theNCCengine = new rspfNCC_FFTW(cy,cx);
   }

   theNCCengine->ingestMaster(my,mx,Mchip->getDoubleBuf());
   theNCCengine->ingestSlave(sy,sx,Schip->getDoubleBuf());

   if (!theNCCengine->calculateNCC())
   {
      // TBD err mngt
      if (pcor) *pcor=0.0;
      if (pdispx) *pdispx = 0.0;
      if (pdispy) *pdispy = 0.0;
      cout<<"Error in NCC calculation"<<endl;
      return;
   }
   int mj          = theNCCengine->getMaxCorrX(); 
   int mi          = theNCCengine->getMaxCorrY();
   double bestcorr = theNCCengine->getMaxCorr();
   int oj = (cx-1)/2;//we know that cx and cy are odd!!
   int oi = (cy-1)/2;
   int deltaj = (sx-mx)/2; //we know that sx-mx is even
   int deltai = (sy-my)/2;

   //original best shift (integer shift for for max value)
   double dmcx = mj - oj;
   double dmcy = mi - oi;

   //find maximum, sub-pixel precision
   //use least-square fit on 2nd order polynomial
   if ((mj > oj-deltaj) && (mj < oj+deltaj) && (mi > oi-deltai) && (mi < oi+deltai))
   {
      //then there's a 3x3 neighborhood we can use to get better precision
      vector<double> p2c(6); //2nd order x y polynomial coefficents (see theLMS comments)
      vector<double>::iterator it = p2c.begin();
      double* pm = theLMS;
      const rspfNCC_FFTW::cMatrix& corrmat = theNCCengine->getNcc();
      //matrix product with values of 3x3 neighborhood
      for (int k=0;k<6;++k)
      {
         *it = 0.0;
         for(int i=-1;i<=1;++i)
         {
            for(int j=-1;j<=1;++j)
            {
               *it += *(pm++) * corrmat(mi+i, mj+j);
            }
         }
         ++it;
      }
      //check convexity (det>0) + downwards orientation (trace<0)
      double trace = p2c[4] + p2c[5];
      if (trace<-1e-13) //TBC : -epsilon
      {
         double det = p2c[4]*p2c[5] - 0.25*p2c[3]*p2c[3];
         if (det>1e-13) //TBC : epsilon
         {
            //ok : convex + downwards
            //find maximum position
            double optx = (p2c[3]*p2c[2] - 2.0 * p2c[5]*p2c[1]) / det * 0.25;
            double opty = (p2c[3]*p2c[1] - 2.0 * p2c[4]*p2c[2]) / det * 0.25;
            //limit new position to center pixel square
            //TBD : need to find better model for NCC subpixel
            if ((fabs(optx)<=0.501) && (fabs(opty)<=0.501))
            {
               dmcx+=optx;
               dmcy+=opty;
              //change corelation max value (dangerous) : TBD ? TBC
            }
         }
      }
   }

   //give results back  
   if (pcor)   *pcor   = bestcorr;
   if (pdispx) *pdispx = dmcx;
   if (pdispy) *pdispy = dmcy;
}

