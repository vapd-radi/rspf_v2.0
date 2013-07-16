//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfImageMpiSWriterSequenceConnection.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfImageMpiSWriterSequenceConnection_HEADER
#define rspfImageMpiSWriterSequenceConnection_HEADER
#include <rspf/imaging/rspfImageSourceSequencer.h>
class rspfImageData;
class rspfImageMpiSWriterSequenceConnection : public rspfImageSourceSequencer
{
public:
   rspfImageMpiSWriterSequenceConnection(rspfObject* owner=NULL,
                                          long numberOfTilesToBuffer = 2);
   
   rspfImageMpiSWriterSequenceConnection(rspfImageSource* inputSource,
                                          rspfObject* owner=NULL,
                                          long numberOfTilesToBuffer = 2);
   
  virtual ~rspfImageMpiSWriterSequenceConnection();
   virtual bool isMaster()const
      {
         return false;
      }

   virtual void initialize();
   virtual void setToStartOfSequence();
   /*!
    * Will allow you to get the next tile in the sequence.
    */
   virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32 resLevel=0);

   virtual void slaveProcessTiles();
   
protected:
   int theNumberOfProcessors;
   int theRank;
   int theNumberOfTilesToBuffer;
   
   rspfRefPtr<rspfImageData>* theOutputTile;

   void deleteOutputTiles();

TYPE_DATA
};

#endif
