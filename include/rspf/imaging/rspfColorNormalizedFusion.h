//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Color normalized fusion
//
//*************************************************************************
// $Id: rspfColorNormalizedFusion.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfColorNormalizedFusion_HEADER
#define rspfColorNormalizedFusion_HEADER
#include <rspf/imaging/rspfFusionCombiner.h>

class rspfColorNormalizedFusion : public rspfFusionCombiner
{
public:

   rspfColorNormalizedFusion();
   rspfColorNormalizedFusion(rspfObject* owner);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);

protected:
   virtual ~rspfColorNormalizedFusion();
TYPE_DATA
};

#endif
