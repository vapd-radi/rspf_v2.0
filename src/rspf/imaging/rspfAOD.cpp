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
//
//*************************************************************************
// $Id: rspfAOD.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <math.h>
#include <rspf/imaging/rspfAOD.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfU8ImageData.h>


RTTI_DEF1(rspfAOD, "rspfAOD", rspfImageSourceFilter)

static rspfTrace traceDebug("rspfAOD:debug");

rspfAOD::rspfAOD(rspfObject* owner)
   :
      rspfImageSourceFilter  (owner),  // base class
      theTile                 (NULL)
{
   //***
   // Set the base class "theEnableFlag" to off since no adjustments have been
   // made yet.
   //***
   disableSource();

   // Construction not complete.
}

rspfAOD::rspfAOD(rspfImageSource* inputSource)
   :
      rspfImageSourceFilter  (NULL, inputSource),  // base class
      theTile                 (NULL)
{
   //***
   // Set the base class "theEnableFlag" to off since no adjustments have been
   // made yet.
   //***
   disableSource();

   if (inputSource == NULL)
   {
      setErrorStatus();
      cerr << "rspfAOD::rspfAOD ERROR:"
           << "\nNull input source passed to constructor!" << endl;
      return;
   }

   initialize();
}


rspfAOD::rspfAOD(rspfObject* owner,
                                         rspfImageSource* inputSource)
   :
      rspfImageSourceFilter  (owner, inputSource),  // base class     
      theTile                 (NULL)
{
   //***
   // Set the base class "theEnableFlag" to off since no adjustments have been
   // made yet.
   //***
   disableSource();

   if (inputSource == NULL)
   {
      setErrorStatus();
      cerr << "rspfAOD::rspfAOD ERROR:"
           << "\nNull input source passed to constructor!" << endl;
      return;
   }

   initialize();
}

rspfAOD::~rspfAOD()
{
}

rspfRefPtr<rspfImageData> rspfAOD::getTile(const rspfIrect& tile_rect,
                                              rspf_uint32 resLevel)
{
   if (!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }
      
   // Fetch tile from pointer from the input source.
   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(tile_rect,
                                                                       resLevel);


   if (!inputTile.valid())  // Just in case...
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfAOD::getTile ERROR:"
         << "\nReceived null pointer to tile from input source!"
         << "\nReturning blank tile."
         << endl;
      return inputTile;
   }

   rspfDataObjectStatus tile_status = inputTile->getDataObjectStatus();

   if ( !theEnableFlag ||
        (tile_status == RSPF_NULL) ||
        (tile_status == RSPF_EMPTY) )
   {
      return inputTile;
   }
   
   if (!theTile.valid())
   {
      allocate();
      if (!theTile.valid())
      {
         return inputTile;
      }
   }

   rspf_uint32 w     = tile_rect.width();
   rspf_uint32 h     = tile_rect.height();
   rspf_uint32 tw    = theTile->getWidth();
   rspf_uint32 th    = theTile->getHeight();
   // rspf_uint32 bands = theTile->getNumberOfBands();

   // Set the origin of the output tile.
   theTile->setOrigin(tile_rect.ul());

   if(w*h != tw*th)
   {
      theTile->setWidthHeight(w, h);
      theTile->initialize();
   }
   return theTile;
}

void rspfAOD::initialize()
{
   rspfImageSourceFilter::initialize();
}

void rspfAOD::allocate()
{
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this,
                                                          theInputConnection);
      theTile->initialize();
      setInitializedFlag(true);
      clearErrorStatus();
   }
   else
   {
      setInitializedFlag(false);
      setErrorStatus();
      cerr << "rspfAOD::initialize ERROR:"
           << "\nCannot call method when input connection is NULL!"
           << endl;
   };

   verifyEnabled();
}

bool rspfAOD::loadState(const rspfKeywordlist& /* kwl */, const char* /* prefix */)
{
   static const char MODULE[] = "rspfAOD::loadState()";

   if (traceDebug())  CLOG << "entering..." << endl;

   if (!theTile)
   {
      cerr << MODULE << " ERROR:"
           << "Not initialized..." << endl;
      return false;
   }

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << *this
           << "\nreturning..."
           << endl;
   }
   
   return true;
}


void rspfAOD::verifyEnabled()
{
   // Check all the pointers...
   if ( !theInputConnection    || !theTile )
   {
      disableSource();
      return;
   }

   enableSource();
}

ostream& rspfAOD::print(ostream& os) const
{
   os << "rspfAOD:"
      << "\ntheEnableFlag:  " << (theEnableFlag?"enabled":"disabled")
      << endl;

   return os;
}

ostream& operator<<(ostream& os, const rspfAOD& hr)
{
   return hr.print(os);
}

void rspfAOD::writeTemplate(ostream& /* os */)
{
}

rspfString rspfAOD::getShortName() const
{
   return rspfString("Aerosol Optical Depth");
}
