//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Class definition for mpi master sequencer for building
// overview files.
// 
//----------------------------------------------------------------------------
// $Id: rspfMpiMasterOverviewSequencer.h 17194 2010-04-23 15:05:19Z dburken $
#ifndef rspfMpiMasterOverviewSequencer_HEADER
#define rspfMpiMasterOverviewSequencer_HEADER

#include <rspf/imaging/rspfOverviewSequencer.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageData.h>


/**
 * @class MPI master sequencer for building overview files.
 */
class RSPF_DLL rspfMpiMasterOverviewSequencer : public rspfOverviewSequencer
{
public:
   
   /** default constructor */
   rspfMpiMasterOverviewSequencer();
   
   /**
    * This must be called.  We can only initialize this
    * object completely if we know all connections
    * are valid.  Some other object drives this and so the
    * connection's initialize will be called after.  The job
    * of this connection is to set up the sequence.  It will
    * default to the bounding rect.  The area of interest can be
    * set to some other rectagle (use setAreaOfInterest).
    */
   virtual void initialize();

   /**
    * @brief Will set the internal pointers to the upperleft
    * tile number.  To go to the next tile in the sequence
    * just call getNextTile.
    */
   virtual void setToStartOfSequence();

   /**
    * Will allow you to get the next tile in the sequence.
    * Note the last tile returned will be an invalid
    * rspfRefPtr<rspfImageData>.  Callers should be able to do:
    * 
    * rspfRefPtr<rspfImageData> id = sequencer->getNextTile();
    * while (id.valid())
    * {
    *    doSomething;
    *    id = sequencer->getNextTile();
    * }
    * 
    */
   virtual rspfRefPtr<rspfImageData> getNextTile();
   
protected:
   /** virtual destructor */
   virtual ~rspfMpiMasterOverviewSequencer();

   int  m_rank;
   int  m_numberOfProcessors;
};

#endif /* #ifndef rspfMpiMasterOverviewSequencer_HEADER */
