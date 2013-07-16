//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// $Id: rspfImageMpiSWriterSequenceConnection.cpp 17206 2010-04-25 23:20:40Z dburken $
//----------------------------------------------------------------------------

#include <rspf/rspfConfig.h> /* To pick up RSPF_HAS_MPI. */

#ifdef RSPF_HAS_MPI
#  if RSPF_HAS_MPI
#    include <mpi.h>
#  endif
#endif

#include <rspf/parallel/rspfImageMpiSWriterSequenceConnection.h>
#include <rspf/parallel/rspfMpi.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfNotifyContext.h>

static rspfTrace traceDebug = rspfTrace("rspfImageMpiSWriterSequenceConnection:debug");

RTTI_DEF1(rspfImageMpiSWriterSequenceConnection, "rspfImageMpiSWriterSequenceConnection", rspfImageSourceSequencer)

rspfImageMpiSWriterSequenceConnection::rspfImageMpiSWriterSequenceConnection(rspfObject* owner,
                                                                               long numberOfTilesToBuffer)
   :rspfImageSourceSequencer(NULL,
                              owner),
    theNumberOfTilesToBuffer(numberOfTilesToBuffer),
    theOutputTile(NULL)
{
   theRank = 0;
   theNumberOfProcessors = 1;
   theNumberOfTilesToBuffer = ((theNumberOfTilesToBuffer>0)?theNumberOfTilesToBuffer:2);

#ifdef RSPF_HAS_MPI   
#  if RSPF_HAS_MPI
   MPI_Comm_rank(MPI_COMM_WORLD, &theRank);
   MPI_Comm_size(MPI_COMM_WORLD, &theNumberOfProcessors);
#  endif
#endif
   
   if(theRank!=0)
   {
      theCurrentTileNumber = theRank -1;
   }
   else
   {
      theCurrentTileNumber = 0;
   }
}

rspfImageMpiSWriterSequenceConnection::rspfImageMpiSWriterSequenceConnection(rspfImageSource* inputSource,
                                                                               rspfObject* owner,
                                                                               long numberOfTilesToBuffer)
   :rspfImageSourceSequencer(inputSource,
                                 owner),
    theNumberOfTilesToBuffer(numberOfTilesToBuffer),
    theOutputTile(NULL)
{
   theRank = 0;
   theNumberOfProcessors = 1;
   theNumberOfTilesToBuffer = ((theNumberOfTilesToBuffer>0)?theNumberOfTilesToBuffer:2);
#if RSPF_HAS_MPI
   MPI_Comm_rank(MPI_COMM_WORLD, &theRank);
   MPI_Comm_size(MPI_COMM_WORLD, &theNumberOfProcessors);
#endif
   if(theRank!=0)
   {
      theCurrentTileNumber = theRank -1;
   }
   else
   {
      theCurrentTileNumber = 0;
   }   
}

rspfImageMpiSWriterSequenceConnection::~rspfImageMpiSWriterSequenceConnection()
{   
   deleteOutputTiles();
}

void rspfImageMpiSWriterSequenceConnection::deleteOutputTiles()
{
   if(theOutputTile)
   {
      delete [] theOutputTile;
      theOutputTile = NULL;
   }
}

void rspfImageMpiSWriterSequenceConnection::initialize()
{
  rspfImageSourceSequencer::initialize();

  theCurrentTileNumber = theRank-1;
  if(theOutputTile)
  {
     deleteOutputTiles();
  }
  
  if(theInputConnection)
  {
     theOutputTile = new rspfRefPtr<rspfImageData>[theNumberOfTilesToBuffer];
     
     for(long index = 0; index < theNumberOfTilesToBuffer; ++index)
     {
        theOutputTile[index] = rspfImageDataFactory::instance()->create(this,
                                                                         this);
        theOutputTile[index]->initialize();
     }
  }
}

void rspfImageMpiSWriterSequenceConnection::setToStartOfSequence()
{
   rspfImageSourceSequencer::setToStartOfSequence();
   if(theRank != 0)
   {
      // we will subtract one since the masters job is just
      // writting and not issue getTiles.
      theCurrentTileNumber = theRank-1;

   }
   else
   {
      // the master will start at 0
      theCurrentTileNumber = 0;
   }
}

void rspfImageMpiSWriterSequenceConnection::slaveProcessTiles()
{
#ifdef RSPF_HAS_MPI 
#  if RSPF_HAS_MPI
   rspfEndian endian;
   rspf_uint32 numberOfTiles    = getNumberOfTiles();
   long currentSendRequest = 0;
   long numberOfTilesSent  = 0;
   int errorValue= 0;
   MPI_Request *requests   = new MPI_Request[theNumberOfTilesToBuffer];
   for (int i = 0; i < theNumberOfTilesToBuffer; ++i)
   {
      requests[i] = MPI_REQUEST_NULL;
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfImageMpiSWriterSequenceConnection::slaveProcessTiles(): entering slave and will look at " << numberOfTiles << " tiles" << std::endl;
   }
   while(theCurrentTileNumber < numberOfTiles)
   {
      rspfRefPtr<rspfImageData> data = rspfImageSourceSequencer::getTile(theCurrentTileNumber);

      // if the current send requests have looped around
      // make sure we wait to see if it was sent
      //
      errorValue = MPI_Wait(&requests[currentSendRequest], MPI_STATUS_IGNORE);
      requests[currentSendRequest] = MPI_REQUEST_NULL;
      if(data.valid() &&
         (data->getDataObjectStatus()!=RSPF_NULL)&&
         (data->getDataObjectStatus()!=RSPF_EMPTY))
      {
         theOutputTile[currentSendRequest]->setImageRectangle(data->getImageRectangle());
         theOutputTile[currentSendRequest]->initialize();
         
         theOutputTile[currentSendRequest]->loadTile(data.get());
         theOutputTile[currentSendRequest]->setDataObjectStatus(data->getDataObjectStatus());

         if(traceDebug())
         {
            if(data->getDataObjectStatus() == RSPF_EMPTY)
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG rspfImageMpiSWriterSequenceConnection::slaveProcessTiles(): In salve = "
                  << theRank << " tile is empty" << std::endl;
            }
         }
      }
      else
      {
         if(traceDebug())
         {
            if(!data)
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG rspfImageMpiSWriterSequenceConnection::slaveProcessTiles(): In slave = "
                  << theRank << " ptr is null " << std::endl;
            }
            else
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "DEBUG rspfImageMpiSWriterSequenceConnection::slaveProcessTiles(): In slave = " << theRank << " tile is empty" << std::endl;
            }
         }
         theOutputTile[currentSendRequest]->makeBlank();
      }

      void* buf = theOutputTile[currentSendRequest]->getBuf();
      if((endian.getSystemEndianType()!=RSPF_BIG_ENDIAN)&&
         (theOutputTile[currentSendRequest]->getScalarType()!=RSPF_UINT8))
      {
         endian.swap(theOutputTile[currentSendRequest]->getScalarType(),
                     buf,
                     theOutputTile[currentSendRequest]->getSize());
      }
      errorValue = MPI_Isend(buf,
                             theOutputTile[currentSendRequest]->getSizeInBytes(),
                             MPI_UNSIGNED_CHAR,
                             0,
                             0,
                             MPI_COMM_WORLD,
                             &requests[currentSendRequest]);
#if 0      
      switch(theOutputTile[currentSendRequest]->getScalarType())
      {
         case RSPF_UINT8:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_UNSIGNED_CHAR,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_SINT8:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_CHAR,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_UINT16:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_UNSIGNED_SHORT,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_SINT16:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_SHORT,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_UINT32:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_UNSIGNED_LONG,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_SINT32:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_LONG,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_FLOAT32:
         case RSPF_NORMALIZED_FLOAT:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_FLOAT,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_DOUBLE:
         {
            errorValue = MPI_Isend(buf,
                                    theOutputTile[currentSendRequest]->getSize(),
                                    MPI_DOUBLE,
                                    0,
                                    0,
                                    MPI_COMM_WORLD,
                                    &requests[currentSendRequest]);
            break;
         }
         default:
            break;
      }
#endif
      theCurrentTileNumber += (theNumberOfProcessors-1);
      numberOfTilesSent++;
      currentSendRequest++;
      currentSendRequest %= theNumberOfTilesToBuffer;
   }
   rspf_int32 tempCount = 0;
   // must wait in the correct order
   //
   while(tempCount < theNumberOfTilesToBuffer)
   {
      currentSendRequest++;
      currentSendRequest %= theNumberOfTilesToBuffer;
      
      errorValue = MPI_Wait(&requests[currentSendRequest], MPI_STATUS_IGNORE);
      ++tempCount;
   }
   
//   MPI_Waitall(theNumberOfTilesToBuffer,
//               requests,
//               MPI_STATUS_IGNORE);
   
   delete [] requests;
#  endif
#endif
}


rspfRefPtr<rspfImageData> rspfImageMpiSWriterSequenceConnection::getNextTile(
   rspf_uint32 /* resLevel */)
{
   rspfNotify(rspfNotifyLevel_FATAL)
      << "FATAL rspfImageMpiSWriterSequenceConnection::getNextTile(): "
      << "should not be called" << std::endl;
   return rspfRefPtr<rspfImageData>();
}

