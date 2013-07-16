//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfImageSourceHistogramFilter.cpp 13312 2008-07-27 01:26:52Z gpotts $
#include <rspf/imaging/rspfImageSourceHistogramFilter.h>
#include <rspf/base/rspfHistogramSource.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfKeywordlist.h>

static const rspfKeyword PROPRIETARY_FILENAME_KW("proprietary_filename",
                                                  "imports a proprietary file");

static const rspfKeyword HISTOGRAM_FILENAME_KW("histogram_filename",
                                                "read an RSPF histogram file");


RTTI_DEF1(rspfImageSourceHistogramFilter, "rspfImageSourceHistogramFilter", rspfImageSourceFilter)
   
rspfImageSourceHistogramFilter::rspfImageSourceHistogramFilter()
   :rspfImageSourceFilter(),
    theCurrentResLevel(0),
    theHistogram(NULL),
    theFilename("")
{
   setNumberOfInputs(2);
   theInputListIsFixedFlag = true;
}

rspfImageSourceHistogramFilter::rspfImageSourceHistogramFilter(rspfImageSource* inputSource,
                                                                 rspfRefPtr<rspfMultiResLevelHistogram> histogram)
   : rspfImageSourceFilter(inputSource),
     theCurrentResLevel(0),
     theHistogram(histogram),
     theFilename("")
{
   setNumberOfInputs(2);
   theInputListIsFixedFlag = true;
}

rspfImageSourceHistogramFilter::~rspfImageSourceHistogramFilter()
{
}


void rspfImageSourceHistogramFilter::setHistogram(rspfRefPtr<rspfMultiResLevelHistogram> histogram)
{
   theHistogram = histogram;
   
   if(theHistogram.valid())
   {
      theHistogram->setBinCount(0, 0);
   }
}

bool rspfImageSourceHistogramFilter::setHistogram(const rspfFilename& filename)
{
   rspfRefPtr<rspfMultiResLevelHistogram> histogram = new rspfMultiResLevelHistogram;

   bool result = histogram->importHistogram(filename);

   if(result)
   {
      theFilename = filename;
      setHistogram(histogram);
   }
   
   return result;
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfImageSourceHistogramFilter::getHistogram()
{
   if(!getInput(1))
   {
      return theHistogram;
   }
   else
   {
      rspfHistogramSource* histoSource = PTR_CAST(rspfHistogramSource, getInput(1));
      if(histoSource)
      {
         return histoSource->getHistogram();
      }
   }

   return (rspfMultiResLevelHistogram*)0;
}

const rspfRefPtr<rspfMultiResLevelHistogram> rspfImageSourceHistogramFilter::getHistogram()const
{
   if(!getInput(1))
   {
      return theHistogram;
   }
   else
   {
      rspfHistogramSource* histoSource = PTR_CAST(rspfHistogramSource, getInput(1));
      if(histoSource)
      {
         return histoSource->getHistogram();
      }
   }

   return (rspfMultiResLevelHistogram*)0;
}

bool rspfImageSourceHistogramFilter::canConnectMyInputTo(rspf_int32 inputIndex,
                                                          const rspfConnectableObject* object)const
{
   if(object)
   {
      if(PTR_CAST(rspfImageSource, object) && ( inputIndex == 0 ))
      {
         return true;
      }
      else if(PTR_CAST(rspfHistogramSource, object) && ( inputIndex == 1 ))
      {
         return true;
      }
   }

   return false;
}

void rspfImageSourceHistogramFilter::connectInputEvent(rspfConnectionEvent& event)
{
   theInputConnection = PTR_CAST(rspfImageSource, getInput(0));
   if(PTR_CAST(rspfHistogramSource, event.getNewObject()))
   {
      // only initialize if it's a new object
      initialize();
   }
}

bool rspfImageSourceHistogramFilter::saveState(rspfKeywordlist& kwl,
                                                const char* prefix)const
{
   bool result = rspfImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,
           HISTOGRAM_FILENAME_KW,
           theFilename,
           true);
   
   return result;
}

bool rspfImageSourceHistogramFilter::loadState(const rspfKeywordlist& kwl,
                                                const char* prefix)
{
   const char* proprietaryName = kwl.find(prefix, PROPRIETARY_FILENAME_KW);
   const char* rspfName       = kwl.find(prefix, HISTOGRAM_FILENAME_KW);
   bool result = true;
   rspfFilename file;

   if(proprietaryName)
   {
       theFilename = rspfFilename(proprietaryName);
   }
   else if(rspfName)
   {
      if(!theHistogram)
      {
         theHistogram = new rspfMultiResLevelHistogram;
      }
      theFilename = rspfFilename(rspfName);
      
   }

   if(theFilename.exists()&&(theFilename!=""))
   {
      if(!theHistogram)
      {
         theHistogram = new rspfMultiResLevelHistogram;
      }
      result = theHistogram->importHistogram(theFilename);
   }
   if(theHistogram.valid())
   {
      theHistogram->setBinCount(0, 0);
   }
   
   result =  rspfImageSourceFilter::loadState(kwl, prefix);

   setNumberOfInputs(2);
   theInputListIsFixedFlag = true;
   
   return result;
}

std::ostream& rspfImageSourceHistogramFilter::print(std::ostream& out) const
{

   out << "rspfImageSourceHistogramFilter::print:\n"
       << "theFilename:  " << endl;

   return rspfImageSourceFilter::print(out);
}
