//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
// 
// Copied from rspfQuickbirdNitfTileSource written by Garrett Potts.
//
// Description:
//
// Class declaration for specialized image handler to pick up offsets from
// Quick Bird ".TIL" files.
// 
//----------------------------------------------------------------------------
// $Id: rspfQuickbirdTiffTileSource.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfQuickbirdTiffTileSource_HEADER
#define rspfQuickbirdTiffTileSource_HEADER

#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/base/rspf2dTo2dShiftTransform.h>

class RSPF_DLL rspfQuickbirdTiffTileSource : public rspfTiffTileSource
{
public:
   virtual bool open();
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   
protected:
   rspfFilename m_tileInfoFilename;
   
TYPE_DATA   
};
#endif
