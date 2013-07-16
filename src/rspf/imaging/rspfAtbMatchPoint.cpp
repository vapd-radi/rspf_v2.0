//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains implementation of class 
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAtbMatchPoint.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfAtbMatchPoint.h>
#include <rspf/imaging/rspfAtbPointSource.h>
#include <rspf/imaging/rspfGridRemapEngine.h>
#include <rspf/imaging/rspfGridRemapSource.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfAtbMatchPoint:exec");
static rspfTrace traceDebug ("rspfAtbMatchPoint:debug");

//*****************************************************************************
//  DESTRUCTOR: ~rspfAtbMatchPoint()
//  
//  Need to delete each instance of an rspfAtbPointSource in thePointSourceList
//  
//*****************************************************************************
rspfAtbMatchPoint::~rspfAtbMatchPoint()
{
   thePointSourceList.clear();
}
   
//*****************************************************************************
//  METHOD: rspfAtbMatchPoint::addImage()
//
//  Adds a new contribution to the sample set.
//
//*****************************************************************************
void rspfAtbMatchPoint::addImage(rspfGridRemapSource* remapper)
{
   static const char MODULE[] = "rspfAtbMatchPoint::addImage()";
   if (traceExec())  CLOG << "entering..." << endl;

   //***
   // Assure that this image contains the view point corresponding to this
   // matchpoint:
   //***
   if (!remapper->getBoundingRect().pointWithin(theViewPoint))
   {
      if (traceExec())  CLOG << "returning..." << endl;
      return;
   }

   //***
   // Instantiate a point source for this image at this view point and
   // save it in the list:
   //***
   rspfAtbPointSource* point_source = new rspfAtbPointSource(remapper,
                                                               theViewPoint);
   thePointSourceList.push_back(point_source);

   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfAtbMatchPoint::assignRemapValues()
//
//  The target parameter value computed given all contributions. The target
//  value is then used to establish the remap parameters for each image at this
//  points location. 
//
//*****************************************************************************
bool rspfAtbMatchPoint::assignRemapValues()
{
   static const char MODULE[] = "rspfAtbMatchPoint::assignRemapValues()";
   if (traceExec())  CLOG << "entering..." << endl;

   //***
   // Determine the number of contributors. We require minimum of two:
   //***
   rspf_uint32 num_contributors = (rspf_uint32)thePointSourceList.size();
   if (num_contributors < 2)
      return false;

   //***
   // Hand off the computation of the target pixel to the ATB engine being
   // used. The engine implements the methods for computing targets, as these
   // will vary according to algorithm being used:
   //***
   theGridRemapEngine->assignRemapValues(thePointSourceList);

   if (traceExec())  CLOG << "returning..." << endl;
   return true;
}

   
//*****************************************************************************
//  METHOD:  rspfAtbMatchPoint::setKernelSize(N)
//  
//  Hook to set the size of the kernel used by all point sources in computing
//  their mean pixel value. The kernels will be resized to NxN.
//  
//*****************************************************************************
void rspfAtbMatchPoint::setKernelSize(int side_size)
{
   static const char MODULE[] = "rspfAtbController::setKernelSize(N)";
   if (traceExec())  CLOG << "entering..." << endl;

   vector<rspfAtbPointSource*>::iterator psi = thePointSourceList.begin();
   while (psi != thePointSourceList.end())
      (*psi)->setKernelSize(side_size);

   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfAtbMatchPoint::setAtbRemapEngine
//  
//*****************************************************************************
void rspfAtbMatchPoint::setGridRemapEngine(rspfGridRemapEngine* engine)
{
   static const char MODULE[] = "rspfAtbMatchPoint::setAtbRemapEngine";
   if (traceExec())  CLOG << "entering..." << endl;

   theGridRemapEngine = engine;
   
   //***
   // Need to communicate this change of engine to the point sources that use
   // it to arrive at a "source value":
   //***
   vector<rspfAtbPointSource*>::iterator source = thePointSourceList.begin();
   while (source != thePointSourceList.end())
   {
      (*source)->setGridRemapEngine(engine);
      source++;
   }

   if (traceExec())  CLOG << "returning..." << endl;
   return;
}
