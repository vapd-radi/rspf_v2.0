//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kathy Minear
//
// Description:
// 
// Class to compute Aerosol Optical Depth (AOD) for with atmospheric
// correction.
//
//*************************************************************************
// $Id: rspfAOD.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAOD_HEADER
#define rspfAOD_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfAOD : public rspfImageSourceFilter
{
public:

   rspfAOD(rspfObject* owner=NULL);
   rspfAOD(rspfImageSource* inputSource);
   rspfAOD(rspfObject* owner, rspfImageSource* inputSource);


   virtual rspfString getShortName() const;

   virtual void initialize();

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tile_rect,
                                               rspf_uint32 resLevel=0);
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual ostream& print(ostream& os) const;
   friend ostream& operator << (ostream& os,  const rspfAOD& hr);

   static void writeTemplate(ostream& os);

protected:
   virtual ~rspfAOD();

   void allocate();
   
   /*!
    * Method to set unset the enable flag.
    */
   void verifyEnabled();

   rspfRefPtr<rspfImageData> theTile;

   TYPE_DATA
};

#endif /* #ifndef rspfAOD_HEADER */

