//*******************************************************************
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// class rspfImageGaussianFilter : tile source
//*******************************************************************
// $Id: rspfImageGaussianFilter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/imaging/rspfImageData.h>
#include <cmath>

RTTI_DEF1(rspfImageGaussianFilter, "rspfImageGaussianFilter", rspfImageSourceFilter);

/** 
 * property names
 */
static const char* PROPERTYNAME_GAUSSSTD     = "GaussStd";
static const char* PROPERTYNAME_STRICTNODATA = "StrictNoData";

rspfImageGaussianFilter::rspfImageGaussianFilter()
   : rspfImageSourceFilter(),
     theGaussStd(0.5),
     theStrictNoData(true)
{
   // ingredients: 
   // 2x  ConvolutionFilter1D
   theHF=new rspfConvolutionFilter1D();
   theVF=new rspfConvolutionFilter1D();

   theHF->setIsHorizontal(true);
   theVF->setIsHorizontal(false);
   updateKernels();

   theHF->setStrictNoData(theStrictNoData);
   theVF->setStrictNoData(theStrictNoData);

   //tie them up
   theVF->connectMyInputTo(0,theHF.get());
}

rspfImageGaussianFilter::~rspfImageGaussianFilter()
{
   if(theHF.valid())
   {
      theHF->disconnect();
      theHF = 0;
   }
   if(theVF.valid())
   {
      theVF->disconnect();
      theVF = 0;
   }
}

void rspfImageGaussianFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;

   if (property->getName() == PROPERTYNAME_GAUSSSTD) {
      rspfNumericProperty* sProperty = PTR_CAST(rspfNumericProperty,
                                                     property.get());
      if(sProperty)
      {
         setGaussStd(sProperty->asFloat64());
      }
   } else if (property->getName() == PROPERTYNAME_STRICTNODATA) {
      rspfBooleanProperty* booleanProperty = PTR_CAST(rspfBooleanProperty,
                                                     property.get());
      if(booleanProperty)
      {
         setStrictNoData(booleanProperty->getBoolean());
      }
   } else {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfImageGaussianFilter::getProperty(const rspfString& name)const
{
   if (name == PROPERTYNAME_GAUSSSTD) {
      rspfNumericProperty* property = new rspfNumericProperty(name,rspfString::toString(getGaussStd()),1e-13,100.0);
      property->setCacheRefreshBit();
      return property;
   } else if (name == PROPERTYNAME_STRICTNODATA) {
      rspfBooleanProperty* property = new rspfBooleanProperty(name,isStrictNoData());
      property->setCacheRefreshBit();
      return property;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfImageGaussianFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(PROPERTYNAME_GAUSSSTD);
   propertyNames.push_back(PROPERTYNAME_STRICTNODATA);
}

bool rspfImageGaussianFilter::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{   
   kwl.add(prefix,
           PROPERTYNAME_GAUSSSTD,
           theGaussStd,
           true);
   kwl.add(prefix,
           PROPERTYNAME_STRICTNODATA,
           isStrictNoData()?"true":"false", //use string instead of boolean
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfImageGaussianFilter::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* gs = kwl.find(prefix, PROPERTYNAME_GAUSSSTD);
   if(gs)
   {      
      setGaussStd(rspfString(gs).toDouble());
   } else {
      cerr<<"rspfImageGaussianFilter : warning no "<< PROPERTYNAME_GAUSSSTD<<" in loadState"<<endl;
   }
   const char* sn = kwl.find(prefix, PROPERTYNAME_STRICTNODATA);
   if(sn)
   {      
      setStrictNoData(rspfString(sn).toBool());
   } else {
      cerr<<"rspfConvolutionFilter1D : warning no "<<PROPERTYNAME_STRICTNODATA<<" in state"<<endl;
   }
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfImageGaussianFilter::setGaussStd(const rspf_float64& v)
{
   theGaussStd = v;
   updateKernels();
}

void rspfImageGaussianFilter::setStrictNoData(bool aStrict)
{
   theStrictNoData = aStrict;
   theHF->setStrictNoData(aStrict);
   theVF->setStrictNoData(aStrict);
}

void
rspfImageGaussianFilter::initialize()
{
   rspfImageSourceFilter::initialize();
   initializeProcesses();
}

rspfRefPtr<rspfImageData>
rspfImageGaussianFilter::getTile(const rspfIrect &tileRect,rspf_uint32 resLevel)
{
    if(isSourceEnabled())
    {
       return theVF->getTile(tileRect, resLevel);
    }
    if(theInputConnection)
    {
       return theInputConnection->getTile(tileRect, resLevel);
    }

    return 0;
}

void
rspfImageGaussianFilter::initializeProcesses()
{
   theHF->initialize();
   theVF->initialize();
}

void
rspfImageGaussianFilter::connectInputEvent(rspfConnectionEvent &event)
{
    rspfImageSourceFilter::connectInputEvent(event);
    if(getInput())
    {
       theHF->connectMyInputTo(0, getInput());
       initializeProcesses();
    }
    else
    {
       theHF->disconnectMyInput(0, false, false);
       initializeProcesses();
    }
}

void
rspfImageGaussianFilter::disconnectInputEvent(rspfConnectionEvent &event)
{
    rspfImageSourceFilter::disconnectInputEvent(event);
    if(getInput())
    {
       theHF->connectMyInputTo(0, getInput());
       initializeProcesses();
    }
    else
    {
       theHF->disconnectMyInput(0, false, false);
       initializeProcesses();
    }
}
void
rspfImageGaussianFilter::updateKernels()
{
   //update kernels based on GaussStd value :2.5 sigma on each side
   // symetric kernel
   static const rspf_float64 sigmaN = 2.5;
   rspf_float64 sig22   = getGaussStd()*getGaussStd()*2.0;

   rspf_uint32 halfw   = (rspf_uint32)(std::floor(getGaussStd() * sigmaN + 0.5));
   rspf_uint32 supsize = 2*halfw + 1;

   //fill with sym. gaussian (unnormalized)
   vector<rspf_float64> newk(supsize);
   rspf_float64 sum=1.0;
   rspf_float64 v;
   newk[halfw] = 1.0;
   for(rspf_int32 i=(rspf_int32)halfw; i>0 ;--i) //reverse for summing
   {
      newk[halfw + i] = newk[halfw - i] = v = std::exp(-i*i/sig22);
      sum += 2.0 * v;
   }

   //normalize
   rspf_float64   invsum=1.0/sum;
   for(rspf_uint32 i=0; i<supsize ;++i)
   {
      newk[i] *= invsum;
   }

   //send to 1d conv filters
   theHF->setKernel(newk);
   theVF->setKernel(newk);
   theHF->setCenterOffset(halfw);
   theVF->setCenterOffset(halfw);
}
