//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfMpi.cpp 13602 2008-09-25 14:23:32Z gpotts $

#include <rspf/parallel/rspfMpi.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <time.h>
#if RSPF_HAS_MPI
#  include <mpi.h>
#endif

#if RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfMpi.cpp 13602 2008-09-25 14:23:32Z gpotts $";
#endif

static rspfTrace traceDebug = rspfTrace("rspfMpi:debug");

rspfMpi* rspfMpi::theInstance=0;

rspfMpi* rspfMpi::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfMpi;
   }
   
   return theInstance;
}

bool rspfMpi::isEnabled()const
{
   return theEnabledFlag;
}

void rspfMpi::barrier()
{
#if RSPF_HAS_MPI
   if(theEnabledFlag)
   {
      // Blocks until all processes have reached this routine.
      MPI_Barrier( MPI_COMM_WORLD );
   }
#endif
}

rspf_float64 rspfMpi::getTime()const
{
#if RSPF_HAS_MPI
   if(theEnabledFlag)
   {
      return MPI_Wtime();
   }
   else
   {
      return time(0);
   }
#else
   return time(0);
#endif
}

void rspfMpi::initialize(int* argc, char*** argv)
{
#if RSPF_HAS_MPI
   int success=0;
   success = MPI_Init(argc, argv);
   if(success == MPI_SUCCESS)
   {
      theEnabledFlag = true;
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfMpi::initialize\n"
            << "MPI is initialized and running with "
            << getNumberOfProcessors()
            << " processors..."
            << std::endl;
      }
   }
   else
   {
      theEnabledFlag = false;
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfMpi::initialize:  MPI is not initialized."
            << std::endl;
      }
   }
#else
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfMpi::initialize: Not compiled with mpi!"
         << std::endl;
   }
#endif
}
void rspfMpi::setEnabledFlag(bool flag)
{
   theEnabledFlag = flag;
}

void rspfMpi::finalize()
{
#if RSPF_HAS_MPI
   if(theEnabledFlag)
   {
      int success;
      success = MPI_Finalize();
      if(traceDebug())
      {
         if(success == MPI_SUCCESS)
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "DEBUG rspfMpi::finalize(): mpi is finalized" << std::endl;
         }
         else
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "DEBUG rspfMpi::finalize(): "
               << "mpi is not finalized successfully" << std::endl;
         }
      }
   }
#endif
}

int rspfMpi::getRank()const
{
   int result = 0;

#if RSPF_HAS_MPI
   if(theEnabledFlag)
   {
      MPI_Comm_rank(MPI_COMM_WORLD, &result);
   }
#endif
   
   return result;
}

int rspfMpi::getNumberOfProcessors()const
{
   int result = 1;

#if RSPF_HAS_MPI
   if(theEnabledFlag)
   {
      MPI_Comm_size(MPI_COMM_WORLD, &result);
   }
#endif

   return result;
}

rspfMpi::rspfMpi()
   : theEnabledFlag(false)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfMpi::rspfMpi entered..."
         << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID = " << RSPF_ID << std::endl;
#endif
   }
}

rspfMpi::rspfMpi(const rspfMpi& obj)
   : theEnabledFlag(obj.theEnabledFlag)
{}

void rspfMpi::operator=(const rspfMpi& rhs)
{
   theEnabledFlag = rhs.theEnabledFlag; 
}
