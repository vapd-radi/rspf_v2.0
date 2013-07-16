//*****************************************************************************
// FILE: rspfElevCellHandler.cc
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains implementation of class rspfElevCellHandler. This is the base
//   class for all DEM file readers including DTED. Each file shall have its
//   corresponding rspfElevCellHandler. When more than one file is accessed,
//   an rspfElevCombiner is used which owns multiple instances of this class.
//
// SOFTWARE HISTORY:
//>
//   19Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
// $Id: rspfElevCellHandler.cpp 21214 2012-07-03 16:20:11Z dburken $

#include <rspf/elevation/rspfElevCellHandler.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfGpt.h>

RTTI_DEF1(rspfElevCellHandler, "rspfElevCellHandler" , rspfElevSource)

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfElevCellHandler:exec");
static rspfTrace traceDebug ("rspfElevCellHandler:debug");

static const rspfKeyword DEM_FILENAME_KW ("dem_filename",
                                           "Name of DEM file to load.");
static const rspfIpt ZERO_SIZE_IPT (0, 0);

rspfElevCellHandler::rspfElevCellHandler ()
   : rspfElevSource(),
     theFilename(),
     theMeanSpacing(0.0),
     theAbsLE90(0.0),
     theAbsCE90(0.0)
{
}

rspfElevCellHandler::rspfElevCellHandler (const rspfElevCellHandler& src)
   : rspfElevSource(src),
     theFilename    (src.theFilename),
     theMeanSpacing (src.theMeanSpacing),
     theAbsLE90     (src.theAbsLE90),
     theAbsCE90     (src.theAbsCE90)
{
}

rspfElevCellHandler::rspfElevCellHandler(const char* filename)
   : rspfElevSource(),
     theFilename(filename),
     theMeanSpacing(0.0),
     theAbsLE90(0.0),
     theAbsCE90(0.0)
{}

rspfElevCellHandler::~rspfElevCellHandler()
{}

const rspfElevCellHandler& rspfElevCellHandler::operator=(
   const rspfElevCellHandler& rhs)
{
   if (this != &rhs)
   {
      theFilename     = rhs.theFilename;
      theMeanSpacing  = rhs.theMeanSpacing;
      theAbsLE90      = rhs.theAbsLE90;
      theAbsCE90      = rhs.theAbsCE90;
      
      rspfElevSource::operator=( *(const rspfElevSource*)&rhs);
   }
   
   return *this;
}


rspfFilename rspfElevCellHandler::getFilename() const
{
   return theFilename;
}

double rspfElevCellHandler::getMeanSpacingMeters() const
{
   return theMeanSpacing;
}

bool rspfElevCellHandler::getAccuracyInfo(rspfElevationAccuracyInfo& info,
                                           const rspfGpt& /* gpt*/ ) const
{
  info.m_confidenceLevel = .9;
  info.m_absoluteLE = theAbsLE90;
  info.m_absoluteCE = theAbsCE90;

  return info.hasValidAbsoluteError();
}

bool rspfElevCellHandler::canConnectMyInputTo(
   rspf_int32 /* inputIndex */,
   const rspfConnectableObject* /* object */) const
{         
   return false;
}

