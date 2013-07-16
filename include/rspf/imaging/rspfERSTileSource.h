//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Chong-Ket Chuah
//
// Description:
//
// Contains class definition for the class "rspfERSTileSource".
//
//*******************************************************************
//  $Id: rspfERSTileSource.h 17932 2010-08-19 20:34:35Z dburken $

#ifndef rspfERSTileSource_HEADER
#define rspfERSTileSource_HEADER

#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/support_data/rspfERS.h>


class RSPF_DLL rspfERSTileSource : public rspfGeneralRasterTileSource
{
public:
   rspfERSTileSource();

   rspfERSTileSource(const rspfKeywordlist& kwl, const char* prefix=0);
   

   virtual rspfString getShortName() const;
   virtual rspfString getLongName() const;
   virtual rspfString className() const;

   virtual bool open(const rspfFilename& filename);
  
   //! Returns the image geometry object associated with this tile source or NULL if not defined.
   //! The geometry contains full-to-local image transform as well as projection (image-to-world)
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = NULL);
protected:
   virtual ~rspfERSTileSource();

private:

   void openHeader(const rspfFilename& file);
   
   rspfERS* theHdr;

   TYPE_DATA
};

#endif
