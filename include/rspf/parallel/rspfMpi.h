//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfMpi.h 13602 2008-09-25 14:23:32Z gpotts $
#ifndef rspfMpi_HEADER
#define rspfMpi_HEADER

#include <rspf/rspfConfig.h>
#include <rspf/base/rspfConstants.h>

class RSPF_DLL rspfMpi
{
public:
   static rspfMpi* instance();

   rspf_float64 getTime()const;

   /*!
    * Typically called only one time
    */
   void initialize(int* argc, char*** argv);

   /*!
    * Called only once when we would like to
    * shutdown mpi
    */
   void finalize();
   int getRank()const;
   int getNumberOfProcessors()const;
   void setEnabledFlag(bool flag);
   bool isEnabled()const;

   /**
    * @brief MPI Barrier method.
    * Blocks until all processes have reached this routine.
    *
    * If MPI is enabled this will call:  MPI_Barrier( MPI_COMM_WORLD );
    * else it will simply return.
    */
   void barrier();
   
protected:
   rspfMpi(); // hide 
   rspfMpi(const rspfMpi&); // hide
   void operator =(const rspfMpi&); // hide

   /*!
    * We will have only a single instance of
    * this class.
    */
   static rspfMpi* theInstance;
   bool theEnabledFlag;
};

#endif
