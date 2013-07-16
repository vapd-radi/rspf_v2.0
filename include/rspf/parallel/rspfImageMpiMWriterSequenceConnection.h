//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
// This class is specific to the Master connection and is optimized
// for the writer sequence and batch processing chain.  For the Slave
// implementation look for the rspfImageMpiSWriterSequenceConnection.cc and .h
// files.  The Master connection is currently implemented to allways do
// a recieve and does no processing itself.  The slave connection does
// all the actual work and processing.
//
//*******************************************************************
//  $Id: rspfImageMpiMWriterSequenceConnection.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfImageMpiMWriterSequenceConnection_HEADER
#define rspfImageMpiMWriterSequenceConnection_HEADER
#include <rspf/imaging/rspfImageSourceSequencer.h>
class rspfImageData;
class rspfImageMpiMWriterSequenceConnection : public rspfImageSourceSequencer
{
public:
   rspfImageMpiMWriterSequenceConnection(rspfObject* owner=NULL);
   rspfImageMpiMWriterSequenceConnection(rspfImageSource* inputSource,
                                          rspfObject* owner=NULL);
  virtual ~rspfImageMpiMWriterSequenceConnection();

   /*!
    * This is a virtual method that can be overriden
    * by derived classes (MPI, or PVM or other
    * parallel connections.  This method is to indicate
    * whether or not this connection is the master/
    * controlling connection.
    */
   virtual void initialize();
   virtual void setToStartOfSequence();

   virtual bool isMaster()const
      {
         return true;
      }
   /*!
    * Will allow you to get the next tile in the sequence.
    */
   virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32 resLevel=0);
protected:
   int theNumberOfProcessors;
   int theRank;
   bool theNeedToSendRequest;
   rspfRefPtr<rspfImageData> theOutputTile;

TYPE_DATA
};

#endif
