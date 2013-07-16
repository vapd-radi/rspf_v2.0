//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Class definition for mpi slave sequencer for building
// overview files.
// 
//----------------------------------------------------------------------------
// $Id: rspfMpiSlaveOverviewSequencer.h 17194 2010-04-23 15:05:19Z dburken $
#ifndef rspfMpiSlaveOverviewSequencer_HEADER
#define rspfMpiSlaveOverviewSequencer_HEADER

#include <rspf/imaging/rspfOverviewSequencer.h>
#include <rspf/base/rspfConstants.h>

/**
 * @class MPI slave sequencer for building overview files.
 */
class RSPF_DLL rspfMpiSlaveOverviewSequencer : public rspfOverviewSequencer
{
public:
   
   /** default constructor */
   rspfMpiSlaveOverviewSequencer();
   

   /**
    * @return Always false for this implementation.
    */
   virtual bool isMaster()const;

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
    * @brief This is a trigger to start resampling tiles.
    */
   virtual void slaveProcessTiles();
   
protected:
   /** virtual destructor */
   virtual ~rspfMpiSlaveOverviewSequencer();

   int  m_rank;
   int  m_numberOfProcessors;
};

#endif /* #ifndef rspfMpiSlaveOverviewSequencer_HEADER */
