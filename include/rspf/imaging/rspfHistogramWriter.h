//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfHistogramWriter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfHistogramWriter_HEADER
#define rspfHistogramWriter_HEADER
#include <rspf/base/rspfOutputSource.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfProcessListener.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfConnectionEvent.h>
#include <rspf/base/rspfHistogramSource.h>
#include <rspf/imaging/rspfImageSource.h>

#include <fstream>

class rspfImageSource;
class rspfHistogramWriterProcessListener;
class rspfImageHistogramSource;

class RSPFDLLEXPORT rspfHistogramWriter : public rspfOutputSource,
                                            public rspfProcessInterface,
                                            public rspfConnectableObjectListener
{
public:
   friend class rspfHistogramWriterProcessListener;
   
   rspfHistogramWriter(rspfImageSource* inputSource=0,
                        rspfObject* owner=0);

   void setAreaOfInterest(const rspfIrect& rect);

   virtual rspfObject* getObject();

   virtual const rspfObject* getObject()const;

   virtual bool execute();
   
   virtual bool isOpen()const;

   /*!
    * open the histogram file for output.  Note if the file was
    * previously open then it will close it and re-opens the
    * file.
    */
   virtual bool open();

   virtual bool open(const rspfFilename& filename);

   virtual void close();

   /*!
    * Overrides the base class and maps the outputName to a filename.
    */
   virtual void setOutputName(const rspfString& outputName);
   
   virtual void setFilename(const rspfFilename& filename);
   
   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object)const;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual void processProgressEvent(rspfProcessProgressEvent& event);

   virtual void connectInputEvent(rspfConnectionEvent& event);

   virtual void disconnectInputEvent(rspfConnectionEvent& event);

   /**
    * Overrides base to pass on abort to theHistogramSource.
    */ 
   virtual void abort();
   
protected:
   virtual ~rspfHistogramWriter();
   virtual void writeHistogram();
   rspfIrect                           theAreaOfInterest;
   rspfFilename                        theFilename;
   std::ofstream*                       theFileStream;
   rspfHistogramWriterProcessListener* theProcessListener;

   /** stored for abort call only. */
   rspfHistogramSource*                theHistogramSource;
   
TYPE_DATA
};

#endif
