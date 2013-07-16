//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Frank Warmerdam (warmerdam@pobox.com)
//
// Description:
//
// Contains class declaration for rspfImageWriter.
//*******************************************************************
//  $Id: rspfImageWriter.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/imaging/rspfImageWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/imaging/rspfTiffOverviewBuilder.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHandler.h>

RTTI_DEF1(rspfImageWriter, "rspfImageWriter", rspfOutputSource);

rspfImageWriter::rspfImageWriter(rspfObject* owner)
   : rspfOutputSource(owner,
                       1,
                       0,
                       true,
                       true)
{
   theAreaOfInterest.makeNan();
}

rspfImageWriter::rspfImageWriter(rspfObject* owner,
                    rspf_uint32 numberOfInputs,
                    rspf_uint32 numberOfOutputs,
                    bool inputListIsFixed,
                    bool outputListIsFixed)
   :rspfOutputSource(owner,
                      numberOfInputs,
                      numberOfOutputs,
                      inputListIsFixed,
                      outputListIsFixed)
{
   theAreaOfInterest.makeNan();
}

rspfImageWriter::~rspfImageWriter()
{
}

bool rspfImageWriter::setViewingRect(const rspfIrect &aRect)
{
   setAreaOfInterest(aRect);
   
   return true;
}

rspfIrect rspfImageWriter::getAreaOfInterest() const
{
   return theAreaOfInterest;
}
 
void rspfImageWriter::setAreaOfInterest(const rspfIrect& inputAreaOfInterest)
{
   theAreaOfInterest = inputAreaOfInterest;
}

bool rspfImageWriter::saveState(rspfKeywordlist& kwl,
                                 const char* prefix)const
{
   return rspfOutputSource::saveState(kwl, prefix);
}

bool rspfImageWriter::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   return rspfOutputSource::loadState(kwl, prefix);
}
   
