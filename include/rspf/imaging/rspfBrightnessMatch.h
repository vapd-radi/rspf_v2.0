//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfBrightnessMatch.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfBrightnessMatch_HEADER
#define rspfBrightnessMatch_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfBrightnessContrastSource.h>

class rspfBrightnessMatch : public rspfImageSourceFilter
{
public:
   rspfBrightnessMatch();



   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);


   virtual void initialize();
   /*---------------------- PROPERTY INTERFACE ---------------------------*/
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /*--------------------- PROPERTY INTERFACE END ------------------------*/

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
protected:
   virtual ~rspfBrightnessMatch();
   
   double          theTargetBrightness;
   double          theInputBrightness;
   rspfRefPtr<rspfImageData> theNormTile;

   rspfRefPtr<rspfBrightnessContrastSource> theBrightnessContrastSource;
   
   rspfRefPtr<rspfImageData> runBrightnessMatch();

   void computeInputBrightness();
TYPE_DATA
};

#endif
