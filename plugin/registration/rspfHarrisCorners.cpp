#include "rspfHarrisCorners.h"
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/imaging/rspfCastTileSourceFilter.h>
#include <rspf/imaging/rspfConvolutionFilter1D.h>
#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/imaging/rspfEquationCombiner.h>
#include "rspfExtremaFilter.h"
#include "rspfDensityReducer.h"
#include "rspfSquareFunction.h"
#include "rspfMultiplier.h"

RTTI_DEF1(rspfHarrisCorners, "rspfHarrisCorners", rspfImageSourceFilter);

//! property names
static const char* PROPERTYNAME_K             = "K";
static const char* PROPERTYNAME_GAUSSSTD      = "GaussStd";
static const char* PROPERTYNAME_MINCORNERNESS = "MinCornerness";
static const char* PROPERTYNAME_DENSITY       = "Density";

static double DER_KERNEL[3] = {-0.5,0.0,0.5};

rspfHarrisCorners::rspfHarrisCorners()
   : rspfImageSourceFilter(),
     theK(0.05),
     theGaussStd(1.0),
     theMinCornerness(0.0),
     theDensity(0.003)
{
   //create sub-processes
   // ingredients: 
   // 2x  3x3ConvolutionFilter
   // 3x  EquationCombiner - for 2nd order products  (a square and a product)
   // 3x  EquationCombiner - for gaussian smoothing  (clones)
   // 1x  EquationCombiner - for combining everything
   // 1x  ExtremaFilter    - for local maxima, 8 neighbours, strict
   // 1x  DensityReducer   - for spatially filtering local maxima
   
   theCastP       = new rspfCastTileSourceFilter();
   theDxP         = new rspfConvolutionFilter1D();
   theDyP         = new rspfConvolutionFilter1D();
   theProductP    = new rspfMultiplier();
   theSquaresP.push_back(new rspfSquareFunction);
   theSquaresP.push_back(new rspfSquareFunction);
   theSmoothP.push_back(new rspfImageGaussianFilter);
   theSmoothP.push_back(new rspfImageGaussianFilter);
   theSmoothP.push_back(new rspfImageGaussianFilter);
   theCornernessP = new rspfEquationCombiner();
   theLocalMaxP   = new rspfExtremaFilter();
   theFilterP     = new rspfDensityReducer();

   //initialize parameters for sub-processes
   theCastP->setOutputScalarType(RSPF_FLOAT64); //float64 or double??

   theDxP->setKernel(vector<rspf_float64>(DER_KERNEL,DER_KERNEL+3)); //hard coded
   theDxP->setCenterOffset(1); //hard coded
   theDxP->setIsHorizontal(true); //hard coded
   theDxP->setStrictNoData(true);

   theDyP->setKernel(vector<rspf_float64>(DER_KERNEL,DER_KERNEL+3));
   theDyP->setCenterOffset(1); //hard coded
   theDyP->setIsHorizontal(false); //hard coded
   theDyP->setStrictNoData(true);
      
   for(int i=0;i<3;++i)
   {
     theSmoothP[i]->setGaussStd(theGaussStd);
     theSmoothP[i]->setStrictNoData(true);
   }

   theCornernessP->setEquation(getCornernessEquation());
   theCornernessP->setOutputScalarType(RSPF_FLOAT64);

   theLocalMaxP->setIsMaximum(true);
   theLocalMaxP->setIsStrict(true);

   theFilterP->setMaxDensity(getDensity());

   //tie them all up
   theDxP->connectMyInputTo(0,theCastP.get());
   theDyP->connectMyInputTo(0,theCastP.get());

   theSquaresP[0]->connectMyInputTo(0,theDxP.get());
   theSquaresP[1]->connectMyInputTo(0,theDyP.get());

   theProductP->connectMyInputTo(0,theDxP.get());
   theProductP->connectMyInputTo(1,theDyP.get());

   theSmoothP[0]->connectMyInputTo(0,theSquaresP[0].get());
   theSmoothP[1]->connectMyInputTo(0,theProductP.get());
   theSmoothP[2]->connectMyInputTo(0,theSquaresP[1].get());

   for (int i=0;i<3;++i)
   {   
      theCornernessP->connectMyInputTo(i,theSmoothP[i].get());
   }

   theLocalMaxP->connectMyInputTo(0,theCornernessP.get());

   theFilterP->connectMyInputTo(0,theLocalMaxP.get());
}

rspfHarrisCorners::~rspfHarrisCorners()
{
   if(theCastP.valid())
   {
      rspf_uint32 idx = 0;
      theCastP->disconnect();
      theDxP->disconnect();
      theDyP->disconnect();
      theProductP->disconnect();
      for(idx = 0; idx < theSquaresP.size(); ++idx)
      {
         theSquaresP[idx]->disconnect();
         theSquaresP[idx] = 0;
      }
      for(idx = 0; idx < theSmoothP.size(); ++idx)
      {
         theSmoothP[idx]->disconnect();
         theSmoothP[idx] = 0;
      }
      theCornernessP->disconnect();
      theLocalMaxP->disconnect();
      theFilterP->disconnect();
      theSmoothP.clear();
      theCastP = 0;
      theDxP = 0;
      theDyP = 0;
      theCornernessP = 0;
      theLocalMaxP = 0;
      theFilterP = 0;
   }
}

void rspfHarrisCorners::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if (property->getName() == PROPERTYNAME_K) {
      rspfNumericProperty* sProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(sProperty)
      {
         setK(sProperty->asFloat64());
      }
   } else if (property->getName() == PROPERTYNAME_GAUSSSTD) {
      rspfNumericProperty* sProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(sProperty)
      {
         setGaussStd(sProperty->asFloat64());
      }
   } else if(property->getName() == PROPERTYNAME_MINCORNERNESS) {
      rspfNumericProperty* sProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(sProperty)
      {
         setMinCornerness(sProperty->asFloat64());
      }
   } else if(property->getName() == PROPERTYNAME_DENSITY) {
      rspfNumericProperty* sProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(sProperty)
      {
         setDensity(sProperty->asFloat64());
      }
   } else {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfHarrisCorners::getProperty(const rspfString& name)const
{
   if(name == PROPERTYNAME_K) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString(getK()),0,0.25);
      property->setCacheRefreshBit();
      return property;
   } else if (name == PROPERTYNAME_GAUSSSTD) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString(getGaussStd()),0.001,15);
      property->setCacheRefreshBit();
      return property;
   } else if (name == PROPERTYNAME_MINCORNERNESS) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString(getMinCornerness()));
      property->setCacheRefreshBit();
      return property;
   } else if (name == PROPERTYNAME_DENSITY) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString(getDensity()),0.0,1.0);
      property->setCacheRefreshBit();
      return property;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfHarrisCorners::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(PROPERTYNAME_K);
   propertyNames.push_back(PROPERTYNAME_GAUSSSTD);
   propertyNames.push_back(PROPERTYNAME_MINCORNERNESS);
   propertyNames.push_back(PROPERTYNAME_DENSITY);
}

bool rspfHarrisCorners::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{   
   kwl.add(prefix,
           PROPERTYNAME_K,
           getK(),
           true);
   kwl.add(prefix,
          PROPERTYNAME_GAUSSSTD,
           getGaussStd(),
           true);
   kwl.add(prefix,
          PROPERTYNAME_MINCORNERNESS,
           getMinCornerness(),
           true); 
   kwl.add(prefix,
          PROPERTYNAME_DENSITY,
           getDensity(),
           true); 

   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfHarrisCorners::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* k = kwl.find(prefix, PROPERTYNAME_K);
   if(k)
   {      
      setK(rspfString(k).toDouble());
   } else {
      cerr<<"rspfHarrisCorners::loadState : missing property "<< PROPERTYNAME_K<<endl;
   }

   const char* gs = kwl.find(prefix, PROPERTYNAME_GAUSSSTD);
   if(gs)
   {      
      setGaussStd(rspfString(gs).toDouble());
   } else {
      cerr<<"rspfHarrisCorners::loadState : missing property "<< PROPERTYNAME_GAUSSSTD<<endl;
   }

   const char* mc = kwl.find(prefix, PROPERTYNAME_MINCORNERNESS);
   if(mc)
   {      
      setMinCornerness(rspfString(mc).toDouble());
   } else {
      cerr<<"rspfHarrisCorners::loadState : missing property "<< PROPERTYNAME_MINCORNERNESS<<endl;
   }
   const char* dd = kwl.find(prefix, PROPERTYNAME_DENSITY);
   if(dd)
   {      
      setDensity(rspfString(dd).toDouble());
   } else {
      cerr<<"rspfHarrisCorners::loadState : missing property "<< PROPERTYNAME_DENSITY<<endl;
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfHarrisCorners::setK(const rspf_float64& v)
{
   theK=v;
   //update processes
   theCornernessP->setEquation(getCornernessEquation());
}

void rspfHarrisCorners::setGaussStd(const rspf_float64& v)
{
   theGaussStd = v;
   //update processes
   for(int i=0;i<3;++i)
   {
      theSmoothP[i]->setGaussStd(theGaussStd);
   }
}

void rspfHarrisCorners::setMinCornerness(const rspf_float64& v)
{
   theMinCornerness = v;
   //TBC : no impact, not used
}

void rspfHarrisCorners::setDensity(const rspf_float64& v)
{
   theDensity = v;
   //update process
   theFilterP->setMaxDensity(getDensity());
}

void
rspfHarrisCorners::initialize()
{
   rspfImageSourceFilter::initialize();
   initializeProcesses();
}

rspfRefPtr<rspfImageData>
rspfHarrisCorners::getTile(const rspfIrect &tileRect,rspf_uint32 resLevel)
{
    if(theInputConnection)
    {
       if(isSourceEnabled())
       {
         return theFilterP->getTile(tileRect, resLevel);
       }
       return theInputConnection->getTile(tileRect, resLevel);
    }
    return rspfRefPtr<rspfImageData>();
}

void
rspfHarrisCorners::initializeProcesses()
{
   theCastP->initialize();
   theDxP->initialize();
   theDyP->initialize();
   theProductP->initialize();
   theSquaresP[0]->initialize();
   theSquaresP[1]->initialize();
   for (int i=0;i<3;++i)
   {
      theSmoothP[i]->initialize();
   }
   theCornernessP->initialize();
   theLocalMaxP->initialize();
   theFilterP->initialize();
}

void
rspfHarrisCorners::connectInputEvent(rspfConnectionEvent &event)
{
    rspfImageSourceFilter::connectInputEvent(event);
    if(getInput())
    {
       theCastP->connectMyInputTo(0, getInput());
       initializeProcesses();
    }
    else
    {
       theCastP->disconnectMyInput(0, false, false);
       initializeProcesses();
    }
}


void
rspfHarrisCorners::disconnectInputEvent(rspfConnectionEvent &event)
{
    rspfImageSourceFilter::disconnectInputEvent(event);
    if(getInput())
    {
       theCastP->connectMyInputTo(0, getInput());
       initializeProcesses();
    }
    else
    {
       theCastP->disconnectMyInput(0, false, false);
       initializeProcesses();
    }
}

rspfString
rspfHarrisCorners::getCornernessEquation()const
{
   return rspfString("(in[0]*in[2]-in[1]^2)-") + rspfString::toString(theK) + "*(in[0]+in[2])^2";
}

void
rspfHarrisCorners::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   theFilterP->getOutputBandList(bandList);
}

rspfScalarType
rspfHarrisCorners::getOutputScalarType() const
{
   return theFilterP->getOutputScalarType();
}

double
rspfHarrisCorners::getNullPixelValue(rspf_uint32 band)const
{
   return theFilterP->getNullPixelValue(band);
}

double
rspfHarrisCorners::getMinPixelValue(rspf_uint32 band)const
{
   return theFilterP->getMinPixelValue(band);
}

double
rspfHarrisCorners::getMaxPixelValue(rspf_uint32 band)const
{
   return theFilterP->getMaxPixelValue(band);
}
