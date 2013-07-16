//----------------------------------------------------------------------------
// FILE: rspfElevSourceFactory.cc
//
// Copyright (C) 2002 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Class declaration for rspfElevSourceFactory.
//
// This is the base class interface for elevation source factories which main
// function is to return an elevation source that covers the ground point
// passed to the "getElevSource" method.
//
// Note the caller of "getElevSource" is responsible for managing the memory
// new'd from the method.
// 
//----------------------------------------------------------------------------
// $Id: rspfElevSourceFactory.h 13269 2008-07-25 14:27:36Z dburken $

#ifndef rspfElevSourceFactory_HEADER
#define rspfElevSourceFactory_HEADER

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfFilename.h>

class rspfElevSource;
class rspfGpt;

/*!****************************************************************************
 * CLASS:  rspfElevSourceFactory
 *
 *****************************************************************************/
class RSPF_DLL rspfElevSourceFactory : public rspfObject
{
public:

   /** default constructor */
   rspfElevSourceFactory();

   /** virtual destructor */
   virtual ~rspfElevSourceFactory();

   /**
    *  Pure virtual method.
    *  return an elevation source pointer that has elevation coverage for the
    *  ground point passed in.
    *  Note the caller of this method is responsible for managing the memory
    *  allocated.
    */
   virtual rspfElevSource* getNewElevSource(const rspfGpt& gpt)const=0;

   /**
    * @return The directory the factory returns data from.
    */
   virtual rspfFilename getDirectory() const;

   /**
    * @param directory The directory to return data from.
    */
   virtual void setDirectory(const rspfFilename& directory);

protected:
   rspfFilename theDirectory;
   
TYPE_DATA
}; 

#endif /* End of "#ifndef rspfElevSourceFactory_HEADER" */
