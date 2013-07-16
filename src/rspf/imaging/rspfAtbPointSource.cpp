//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains implementation of class rspfAtbPointSource.
//   This object provides the statistics associated with a given point on a
//   given image corresponding to a matchpoint. A matchpoint contains a
//   collection of these point sources, one for each contributing image.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAtbPointSource.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfGridRemapSource.h>
#include <rspf/imaging/rspfGridRemapEngine.h>
#include <rspf/imaging/rspfAtbPointSource.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfImageSource:exec");
static rspfTrace traceDebug ("rspfImageSource:debug");

static const int DEFAULT_KERNEL_SIZE = 18;  // recommend odd number

//*****************************************************************************
//  DEFAULT CONSTRUCTOR: rspfAtbPointSource
//  
//*****************************************************************************
rspfAtbPointSource::rspfAtbPointSource()
   : theRemapSource (0),
     theGridRemapEngine (0),
     theKernelSize (DEFAULT_KERNEL_SIZE),
     theViewPointIsValid (false)
{
   static const char MODULE[] = "rspfAtbPointSource Default Constructor";
   if (traceExec())  CLOG << "entering..." << endl;

   if (traceExec())  CLOG << "returning..." << endl;
}
      

//*****************************************************************************
//  CONSTRUCTOR: rspfAtbPointSource(image_source)
//  
//*****************************************************************************
rspfAtbPointSource::rspfAtbPointSource(rspfGridRemapSource* source,
                                         const rspfDpt&    view_point)
   : theRemapSource (source),
     theGridRemapEngine (source->getRemapEngine()),
     theKernelSize (DEFAULT_KERNEL_SIZE),
     theViewPointIsValid (false)
{
   static const char MODULE[] = "rspfAtbPointSource Default Constructor";
   if (traceExec())  CLOG << "entering..." << endl;

   setViewPoint(view_point);
   
   if (traceExec())  CLOG << "returning..." << endl;
}


//*****************************************************************************
//  DESTRUCTOR: ~rspfAtbPointSource()
//  
//*****************************************************************************
rspfAtbPointSource::~rspfAtbPointSource()
{
}

//*****************************************************************************
//  METHOD: rspfAtbPointSource::setRemapSource()
//  
//  Sets the pointer to the source of pixels used to compute the stats.
//  
//*****************************************************************************
void rspfAtbPointSource::setRemapSource(rspfGridRemapSource* source)
{
   static const char MODULE[] = "rspfAtbPointSource::setRemapSource()";
   if (traceExec())  CLOG << "entering..." << endl;

   theRemapSource = source;

   //***
   // Also fetch the remap engine which should be a derived rspfAtbRemapEngine:
   //***
   theGridRemapEngine = theRemapSource->getRemapEngine();
   
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfAtbPointSource::setViewPoint()
//  
//  Sets the view coordinates corresponding to this point.
//  
//*****************************************************************************
void rspfAtbPointSource::setViewPoint(const rspfDpt& view_point)
{
   static const char MODULE[] = "rspfAtbPointSource::setViewPoint()";
   if (traceExec())  CLOG << "entering..." << endl;

   theViewPoint = view_point;
   
   if ((theRemapSource.valid()) &&
       (theRemapSource->getBoundingRect().pointWithin(theViewPoint)))
      theViewPointIsValid = true;
   else
      theViewPointIsValid = false;
         
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfAtbPointSource::getSourceValue()
//  
//  Returns the computed value vector corresponding to the region about the
//  view point. The definition of "value" is implemented by theGridRemapEngine.
//
//*****************************************************************************
void rspfAtbPointSource::getSourceValue(void* value_vector)
{
   static const char MODULE[] = "rspfAtbPointSource::getSourceValue()";
   if (traceExec())  CLOG << "entering..." << endl;

   //***
   // Verify that members are initialized before processing:
   //***
   if ((!theViewPointIsValid) || (!theRemapSource) || (!theGridRemapEngine))
   {
      CLOG <<"ERROR: This object was not properly initialized before attempting"
           <<" to compute target value! Returning usassigned vector..."<<endl;
      return;
   }

   //***
   // Extract the data kernel from the image chain:
   //***
   rspfIpt kernel_2d_size (theKernelSize, theKernelSize);
   rspfIpt kernel_ul (theViewPoint - kernel_2d_size/2.0);
   rspfIpt kernel_lr (kernel_ul.x + kernel_2d_size.x - 1,
                       kernel_ul.y + kernel_2d_size.y - 1);
   rspfIrect kernel_rect (kernel_ul, kernel_lr);
   rspfRefPtr<rspfImageData> kernel_data =
      theRemapSource->getTile(kernel_rect);

   //***
   // Fetch the value of the data kernel. Note: this is not necessarily in
   // the same color space as the pixels. It depends on which engine is being
   // utilized:
   //***
   theGridRemapEngine->computeSourceValue(kernel_data, value_vector);

//   delete kernel_data;
   
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfAtbPointSource:: setKernelSize(N)
//
//  Method to set the kernel size used in computing statistics. The kernel
//  will be resized to NxN.
//
//*****************************************************************************
void rspfAtbPointSource::setKernelSize(int side_size)
{
   static const char MODULE[] = "rspfAtbPointSource::setKernelSize()";
   if (traceExec())  CLOG << "entering..." << endl;

   theKernelSize = side_size;
   
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}

 
