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
// Gamma remapper.
//
//*************************************************************************
// $Id: rspfGammaRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGammaRemapper_HEADER
#define rspfGammaRemapper_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfGammaRemapper : public rspfImageSourceFilter
{
public:

   rspfGammaRemapper(rspfObject* owner=NULL);
   rspfGammaRemapper(rspfImageSource* inputSource);
   rspfGammaRemapper(rspfObject* owner, rspfImageSource* inputSource);


   virtual rspfString getShortName()const;

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
   friend ostream& operator<< (ostream& os,  const rspfGammaRemapper& hr);

   void setMinMaxPixelValues(const vector<double>& v_min,
                             const vector<double>& v_max);

   virtual void enableSource();
   
protected:
   virtual ~rspfGammaRemapper();

   /*!
    * Method to set unset the enable flag.
    */

   void verifyEnabled();

   rspfRefPtr<rspfImageData> theTile;
   double*         theBuffer;
   vector<double>  theMinPixelValue;
   vector<double>  theMaxPixelValue;
   vector<double>  theGamma;
   bool            theUserDisabledFlag;

   TYPE_DATA
};

#endif /* #ifndef rspfGammaRemapper_HEADER */
