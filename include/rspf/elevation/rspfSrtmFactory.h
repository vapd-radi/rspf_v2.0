//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Shuttle Radar Topography Mission (SRTM) factory to return an
// rspfSrtmElevSource given a ground point.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmFactory.h 13269 2008-07-25 14:27:36Z dburken $
#ifndef rspfSrtmFactory_HEADER
#define rspfSrtmFactory_HEADER

#include <rspf/elevation/rspfElevSourceFactory.h>

/**
 * @class rspfSrtmFactory Used by the elevation manager, this class returns
 * an rspfSrtmElevSource given a ground point and some directory with srtm
 * files in it.
 */
class RSPF_DLL rspfSrtmFactory : public rspfElevSourceFactory
{
public:

   /** default constructor */
   rspfSrtmFactory();

   /** Constructor that takes a directory name. */
   rspfSrtmFactory(const rspfFilename& dir);

   /** destructor */
   virtual ~rspfSrtmFactory();

   /**
    * Open the appropriate rspfSrtmElevSource that covers given a
    * ground point.
    *
    * @param gpt Ground point that an elevation source is need for.
    *
    * @return Returns a pointer to an rspfElevSource if an srtm file is found
    * that can cover the ground point.  Returns NULL if no cell is found
    * for the point.
    */
   virtual rspfElevSource* getNewElevSource(const rspfGpt& gpt) const;
   
protected:

TYPE_DATA
};

#endif /* End of "#ifndef rspfSrtmFactory_HEADER" */
