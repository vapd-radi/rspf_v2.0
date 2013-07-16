//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfHistogramWriter.cpp 20118 2011-10-05 13:50:55Z dburken $
#include <rspf/base/rspfProcessListener.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfImageSourceSequencer.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfHistogramWriter:debug");
class rspfHistogramWriterProcessListener : public rspfProcessListener
{
public:
   rspfHistogramWriterProcessListener(rspfConnectableObject* passon)
      :thePassonTo(passon)
      {
      }
   virtual void processProgressEvent(rspfProcessProgressEvent& event)
      {
         if(thePassonTo)
         {
            event.setObject(thePassonTo);
            thePassonTo->fireEvent(event);
         }
      }

   rspfConnectableObject* thePassonTo;  
   
};

RTTI_DEF3(rspfHistogramWriter,
          "rspfHistogramWriter",
          rspfOutputSource,
          rspfProcessInterface,
          rspfConnectableObjectListener);

rspfHistogramWriter::rspfHistogramWriter(rspfImageSource* inputSource,
                                           rspfObject* owner)
   : rspfOutputSource(owner,
                       1,
                       0,
                       true,
                       false),
     theAreaOfInterest(),
     theFilename(),
     theFileStream(0),
     theProcessListener(0),
     theHistogramSource(0)
{
   theProcessListener = new rspfHistogramWriterProcessListener(this);
   
   theAreaOfInterest.makeNan();
   connectMyInputTo(0, inputSource);
}

rspfHistogramWriter::~rspfHistogramWriter()
{
   if(isOpen())
   {
      close();
   }
   if(theProcessListener)
   {
      delete theProcessListener;
      theProcessListener = 0;
   }

   // This does not need to be deleted here.  Simply stored for abort.
   if (theHistogramSource)
   {
      theHistogramSource = 0;
   }
}

void rspfHistogramWriter::setAreaOfInterest(const rspfIrect& rect)
{
   theAreaOfInterest = rect;
}

rspfObject* rspfHistogramWriter::getObject()
{
   return this;
}

bool rspfHistogramWriter::isOpen()const
{
   return (theFileStream != 0);
}

bool rspfHistogramWriter::open()
{
   if(isOpen())
   {
      close();
   }
   
   theFileStream = new std::ofstream(theFilename.c_str());
   
   return theFileStream->good();
}

bool rspfHistogramWriter::open(const rspfFilename& filename)
{
   return rspfOutputSource::open(filename);
}

void rspfHistogramWriter::close()
{
   delete theFileStream;
   theFileStream = 0;
}

void rspfHistogramWriter::setOutputName(const rspfString& outputName)
{
   rspfOutputSource::setOutputName(outputName);
   setFilename(outputName);
}

void rspfHistogramWriter::setFilename(const rspfFilename& filename)
{
   theFilename = filename;
}
   
bool rspfHistogramWriter::canConnectMyInputTo(
   rspf_int32 inputIndex, const rspfConnectableObject* object)const
{
   return (object&&(inputIndex == 0)&& 
           (PTR_CAST(rspfHistogramSource, object)||
            PTR_CAST(rspfImageSource, object)));
}

const rspfObject* rspfHistogramWriter::getObject()const
{
   return this;
}

void rspfHistogramWriter::processProgressEvent(
   rspfProcessProgressEvent& event)
{
   // we will raise the event if its coming from our
   // input.  This means that we are connected to a
   // rspfHistogramSource.
   //
   if(event.getObject() != this)
   {
      rspfProcessInterface::setCurrentMessage(event.getMessage());
      setPercentComplete(event.getPercentComplete());
   }
}

void rspfHistogramWriter::connectInputEvent(
   rspfConnectionEvent& event)
{
   if(event.getObject() == this)
   {
      if(event.getOldObject())
      {
         event.getOldObject()->removeListener((rspfProcessListener*)this);
      }
      if(getInput(0)&&
         PTR_CAST(rspfHistogramSource, getInput(0)))
      {
         getInput(0)->addListener( (rspfProcessListener*)this);
      }
   }
}

void rspfHistogramWriter::disconnectInputEvent(
   rspfConnectionEvent& event)
{
   if(event.getOldObject()&&
      PTR_CAST(rspfHistogramSource, getInput(0)))
   {
      event.getOldObject()->removeListener((rspfProcessListener*)this);
   }
}

bool rspfHistogramWriter::saveState(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   bool result = rspfOutputSource::saveState(kwl, prefix);


   return result;
}

bool rspfHistogramWriter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   bool result = rspfOutputSource::loadState(kwl, prefix);

   if(result)
   {
      if(!getNumberOfInputs())
      {
         setNumberOfInputs(1);
      }
   }

   return result;
}

bool rspfHistogramWriter::execute()
{
   writeHistogram();
   
   return true;
}

void rspfHistogramWriter::writeHistogram()
{
   if(!getInput(0))
   {
      cerr << "rspfHistogramWriter::writeHistogram is not connected" << endl;
      return;
   }
   rspfHistogramSource* histoSource = PTR_CAST(rspfHistogramSource, getInput(0));
   bool deleteHistoSource = false;
   if(!histoSource)
   {
      histoSource = new rspfImageHistogramSource;
      histoSource->connectMyInputTo(0, getInput(0));
      histoSource->enableSource();
      deleteHistoSource = true;

      //---
      // Capture the pointer for abort call.  Note a rspfHistogramSource has
      // no abort so the abort will only work through the
      // rspfImageHistogramSource pointer.
      //---
      theHistogramSource = (rspfImageHistogramSource*)histoSource;
   }

   // Capture the pointer for abort call.
   theHistogramSource = histoSource;
   
   histoSource->addListener( theProcessListener);
   
   rspfRefPtr<rspfMultiResLevelHistogram> histo = histoSource->getHistogram();

   // Don't write histogram if abort flag was set...
   if(histo.valid() && !isAborted() )
   {
      rspfKeywordlist kwl;
      histo->saveState(kwl);
      if(!isOpen())
      {
         open();
         if(!isOpen())
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN) << "unable to open file " << theFilename << endl;
            }
         }
      }
      if( isOpen() )
      {
         kwl.writeToStream(*theFileStream);
      }
   }
   histoSource->removeListener(theProcessListener);
   
   if(deleteHistoSource)
   {
      delete histoSource;
      histoSource = 0;
   }
   theHistogramSource = 0;
   
   close();
}

void rspfHistogramWriter::abort()
{
   //---
   // Call base abort. This sets the status to PROCESS_STATUS_ABORT_REQUEST so the processor
   // knows to abort.
   //---
   rspfProcessInterface::abort();

   // Propagate to histo source.
   if (theHistogramSource)
   {
      rspfImageHistogramSource* histoSource =
         PTR_CAST(rspfImageHistogramSource, theHistogramSource);
      if (histoSource)
      {
         histoSource->abort();
      }
   }

   //---
   // Now set status to "ABORTED" so rspfProcessInterface::isAborted returns true so the
   // writeHistogram method doesn't write the file if the process was aborted.
   //---
   setProcessStatus(PROCESS_STATUS_ABORTED);
}
