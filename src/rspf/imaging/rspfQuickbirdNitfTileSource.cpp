//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: rspfQuickbirdNitfTileSource.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfQuickbirdNitfTileSource.h>
#include <rspf/support_data/rspfQuickbirdTile.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspf2dTo2dShiftTransform.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/support_data/rspfNitfImageHeader.h>

RTTI_DEF1(rspfQuickbirdNitfTileSource, "rspfQuickbirdNitfTileSource", rspfNitfTileSource);

static const rspfTrace traceDebug("rspfQuickbirdNitfTileSource:debug");

bool rspfQuickbirdNitfTileSource::open()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfQuickbirdNitfTileSource::open(file) DEBUG: entered ..."
         << std::endl;
   }
   
   rspfFilename file = theImageFile;
   file = file.replaceAllThatMatch("_R[0-9]+C[0-9]+");
   rspfQuickbirdTile tileFile;
   bool openedTileFile = false;
   file.setExtension("TIL");
   
   if(!tileFile.open(file))
   {
      file.setExtension("TIL");
      if(tileFile.open(file))
      {
         openedTileFile = true;
      }
      else
      {
         file.setExtension("til");
         if(tileFile.open(file))
         {
            openedTileFile = true;
         }
      }
   }
   else
   {
      openedTileFile = true;
   }
   
   if(openedTileFile)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfQuickbirdNitfTileSource::open(file) DEBUG:"
            << "\nOpened tile file" << std::endl;
      }

      // Call the base class open...
      if(!rspfNitfTileSource::open())
      {
         return false;
      }
      
      rspfQuickbirdTileInfo info;
      rspfIrect tempBounds = getCurrentImageHeader()->getImageRect();
      
      
      tempBounds = rspfIrect(0,
                              0,
                              tempBounds.width() - 1,
                              tempBounds.height() - 1);
      
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfQuickbirdNitfTileSource::open(file) DEBUG:"
            << "\nheader rectangle = " << tempBounds << std::endl;
      }
      
      rspfIpt ulPt;
      rspfIpt urPt;
      rspfIpt lrPt;
      rspfIpt llPt;
      rspfDpt shift;
      if(tileFile.getInfo(info, theImageFile.file().upcase()))
      {
         ulPt.makeNan();
         urPt.makeNan();
         lrPt.makeNan();
         llPt.makeNan();
         
         if((info.theUlXOffset != RSPF_INT_NAN) &&
            (info.theUlYOffset != RSPF_INT_NAN))
         {
            shift = rspfIpt(info.theUlXOffset, info.theUlYOffset);
         }
         else
         {
            shift = rspfIpt(0,0);
         }
         m_transform = new rspf2dTo2dShiftTransform(shift);
      }
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfQuickbirdNitfTileSource::open(file) DEBUG:"
            << "\nUl = " << ulPt
            << "\nUr = " << urPt
            << "\nLr = " << lrPt
            << "\nLl = " << llPt
            << "\ntheImageRect:  " << getImageRectangle(0)
            << "\nExiting..." 
            << std::endl;
      }
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfQuickbirdNitfTileSource::open(file) DEBUG"
            << "Not opened..."
            << std::endl;
      }
   }
   
   return openedTileFile;
}

rspfRefPtr<rspfImageGeometry> rspfQuickbirdNitfTileSource::getImageGeometry()
{
   rspfRefPtr<rspfImageGeometry> result = rspfImageHandler::getImageGeometry();
   if( result.valid() )
   {
      if(!result->getTransform())
      {
         result->setTransform(m_transform.get());
      }
   }
   return result;
}
