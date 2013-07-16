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
// $Id: rspfHistogramSource.cpp 11721 2007-09-13 13:19:34Z gpotts $

#include <rspf/base/rspfHistogramSource.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>

RTTI_DEF1(rspfHistogramSource, "rspfHistogramSource", rspfSource);


rspfHistogramSource::rspfHistogramSource(rspfObject* owner,
                                           rspf_uint32 numberOfInputs,
                                           rspf_uint32 numberOfOutputs,
                                           bool inputListFixedFlag,
                                           bool outputListFixedFlag)
   : rspfSource(owner,
                 numberOfInputs,
                 numberOfOutputs,
                 inputListFixedFlag,
                 outputListFixedFlag),
     theHistogram(0),
     theFilename()
{
}

rspfHistogramSource::~rspfHistogramSource()
{
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfHistogramSource::getHistogram()
{
   return theHistogram;
}

bool rspfHistogramSource::loadState(const rspfKeywordlist& kwl,
				     const char* prefix)
{   
   theHistogram = 0;
   const char* externalFile = kwl.find(prefix,
                                       rspfKeywordNames::FILENAME_KW);

   theHistogram = new rspfMultiResLevelHistogram;
   if(externalFile)
   {
      if(!theHistogram->importHistogram(rspfFilename(externalFile)))
      {
         theHistogram = 0;
         theFilename = "";
      }
      theFilename = externalFile;
   }
   else
   {
      rspfString newPrefix = rspfString(prefix) + "histogram.";
      if(!theHistogram->loadState(kwl, newPrefix))
      {
         theHistogram = 0;
      }
   }
   
   return rspfSource::loadState(kwl, prefix);
}

bool rspfHistogramSource::saveState(rspfKeywordlist& kwl,
				     const char* prefix)const
{
   if(theHistogram.valid())
   {
      if(!theFilename.empty())      {
         rspfKeywordlist kwl2;
         
         if(theHistogram->saveState(kwl2))
         {
            kwl2.write(theFilename.c_str());
            kwl.add(prefix,
                    rspfKeywordNames::FILENAME_KW,
                    theFilename.c_str(),
                    true);
         }
      }
      else
      {
         rspfString newPrefix = rspfString(prefix) + "histogram.";
         theHistogram->saveState(kwl, newPrefix);
      }
   }
   
   return rspfSource::saveState(kwl, prefix);
}

// Hidden from use.
rspfHistogramSource::rspfHistogramSource(const rspfHistogramSource&)
   :
   theHistogram(0),
   theFilename()
{
}
