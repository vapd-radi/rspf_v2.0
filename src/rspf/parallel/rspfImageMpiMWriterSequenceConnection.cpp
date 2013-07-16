//*******************************************************************
//
// License:  See top level LICENSE.txt file.
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
//  $Id: rspfImageMpiMWriterSequenceConnection.cpp 12099 2007-12-01 16:09:36Z dburken $


#include <rspf/rspfConfig.h> /* To pick up RSPF_HAS_MPI. */

#ifdef RSPF_HAS_MPI
#  if RSPF_HAS_MPI
#    include <mpi.h>
#  endif
#endif

#include <rspf/parallel/rspfImageMpiMWriterSequenceConnection.h>
#include <rspf/parallel/rspfMpi.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfEndian.h>

static rspfTrace traceDebug = rspfTrace("rspfImageMpiMWriterSequenceConnection:debug");

RTTI_DEF1(rspfImageMpiMWriterSequenceConnection, "rspfImageMpiMWriterSequenceConnection", rspfImageSourceSequencer)


rspfImageMpiMWriterSequenceConnection::rspfImageMpiMWriterSequenceConnection(
   rspfImageSource* inputSource,
   rspfObject* owner)
   :rspfImageSourceSequencer(inputSource, owner),
    theOutputTile(NULL)
{
   theRank = 0;
   theNumberOfProcessors = 1;

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
   theNeedToSendRequest = true;
}

rspfImageMpiMWriterSequenceConnection::rspfImageMpiMWriterSequenceConnection(rspfObject* owner)
   :rspfImageSourceSequencer(NULL, owner),
    theOutputTile(NULL)
{
   theRank = 0;
   theNumberOfProcessors = 1;
   
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
   theNeedToSendRequest = true;
}

rspfImageMpiMWriterSequenceConnection::~rspfImageMpiMWriterSequenceConnection()
{
}

void rspfImageMpiMWriterSequenceConnection::initialize()
{
  rspfImageSourceSequencer::initialize();

  theCurrentTileNumber = theRank;//-1;
  theOutputTile = NULL;
  
  if(theInputConnection)
  {
     theOutputTile = rspfImageDataFactory::instance()->create(this, this);
     theOutputTile->initialize();
  }
}

void rspfImageMpiMWriterSequenceConnection::setToStartOfSequence()
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

/*!
 * Will allow you to get the next tile in the sequence.
 */
rspfRefPtr<rspfImageData> rspfImageMpiMWriterSequenceConnection::getNextTile(rspf_uint32 resLevel)
{
#if RSPF_HAS_MPI
   rspfEndian endian;
   if(!theOutputTile)
   {
      initialize();
      if (!theOutputTile)
      {
         return theOutputTile;
      }
   }
   
   void* buf = theOutputTile->getBuf();

   rspf_uint32 numberOfTiles = getNumberOfTiles();
   int errorValue = 0;
   
   if(theCurrentTileNumber >= numberOfTiles)
   {
      return NULL;
   }
   errorValue = MPI_Recv(buf,
                         theOutputTile->getSizeInBytes(),
                         MPI_UNSIGNED_CHAR,
                         theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                         0,
                         MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
   if((endian.getSystemEndianType()!=RSPF_BIG_ENDIAN)&&
      (theOutputTile->getScalarType()!=RSPF_UINT8))
   {
      endian.swap(theOutputTile->getScalarType(),
                  buf,
                  theOutputTile->getSize());
   }
   
#if 0
   // Since MPI can run on different platforms
   // I am not sure if MPI has built in byte
   // swap.  If so I will name each datatype
   // so it can handle byte swap.  Does anyone
   // know if MPI does byte swapping.
   switch(theOutputTile->getScalarType())
   {
      case RSPF_UINT8:
      {
         // we add 1 to the rank so we don't wait on a message
         // to ourself (rank = 0).
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_UNSIGNED_CHAR,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_SINT8:
      {
         // we add 1 to the rank so we don't wait on a message
         // to ourself (rank = 0).
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_CHAR,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_UNSIGNED_SHORT,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_SINT16:
      {
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_SHORT,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_SINT32:
      {
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_LONG,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_UINT32:
      {
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_UNSIGNED_LONG,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_FLOAT,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         errorValue = MPI_Recv(buf,
                               theOutputTile->getSize(),
                               MPI_DOUBLE,
                               theCurrentTileNumber%(theNumberOfProcessors-1)+1,
                               0,
                               MPI_COMM_WORLD,
                               MPI_STATUS_IGNORE);
         break;
      }
      default:
      {
         break;
      }
   }
#endif
   rspfIpt origin;
   getTileOrigin(theCurrentTileNumber,
                 origin);
   theOutputTile->setOrigin(origin);
   theOutputTile->validate();
   ++theCurrentTileNumber;
   return theOutputTile;
#else
   return rspfImageSourceSequencer::getNextTile(resLevel);
#endif
   
}
