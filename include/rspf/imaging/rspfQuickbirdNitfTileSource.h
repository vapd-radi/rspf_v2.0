//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
//*******************************************************************
//  $Id: rspfQuickbirdNitfTileSource.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfQuickbirdNitfTileSource_HEADER
#define rspfQuickbirdNitfTileSource_HEADER
#include <rspf/imaging/rspfNitfTileSource.h>
#include <rspf/base/rspf2dTo2dTransform.h>

class rspfQuickbirdNitfTileSource : public rspfNitfTileSource
{
public:
   virtual bool open();

   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   
protected:
   rspfRefPtr<rspf2dTo2dTransform> m_transform;
TYPE_DATA   
};
#endif
