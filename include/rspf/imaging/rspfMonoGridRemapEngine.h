//*****************************************************************************
// FILE: rspfMonoGridRemapEngine.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class rspfMonoGridRemapEngine
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfMonoGridRemapEngine.h 9094 2006-06-13 19:12:40Z dburken $

#ifndef rspfMonoGridRemapEngine_HEADER
#define rspfMonoGridRemapEngine_HEADER

#include <rspf/imaging/rspfGridRemapEngine.h>

/*!****************************************************************************
 *
 * CLASS: rspfMonoGridRemapEngine 
 *
 *****************************************************************************/
class rspfMonoGridRemapEngine : public rspfGridRemapEngine
{
public:
   rspfMonoGridRemapEngine()
      : rspfGridRemapEngine(1, 1) {}
   
   virtual rspfObject* dup() const;
   
   virtual void remapTile(const rspfDpt& origin_point,
                          rspfGridRemapSource* remapper,
                          rspfRefPtr<rspfImageData>& tile);

   virtual void assignRemapValues(std::vector<rspfAtbPointSource*>& sources);

   virtual void computeSourceValue(rspfRefPtr<rspfImageData>& source,
                                   void* result);

protected:
   virtual void computeRemapNode(rspfAtbPointSource*  point_source,
                                 void*                 source_value,
                                 void*                 target_value);

   TYPE_DATA
};

#endif
