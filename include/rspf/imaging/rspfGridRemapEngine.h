//*****************************************************************************
// FILE: rspfGridRemapEngine.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class 
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGridRemapEngine.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfGridRemapEngine_HEADER
#define rspfGridRemapEngine_HEADER

#include <vector>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>

class rspfDpt;
class rspfGridRemapSource;
class rspfImageData;
class rspfAtbPointSource;

/*!****************************************************************************
 *
 * CLASS: rspfGridRemapEngine 
 *
 *****************************************************************************/
class rspfGridRemapEngine : public rspfObject
{
public:
   virtual rspfObject* dup() const = 0;
   
   virtual void remapTile(const rspfDpt&       origin_point,
                          rspfGridRemapSource* remapper,
                          rspfRefPtr<rspfImageData>& tile) = 0;

   virtual void assignRemapValues(std::vector<rspfAtbPointSource*>& sources)=0;

   virtual void computeSourceValue(rspfRefPtr<rspfImageData>& source,
                                   void* result) = 0;

   int  getNumberOfParams() const { return theNumberOfParams; }
   
   int  getNumberOfBands()  const { return theNumberOfBands; }
   
protected:
   rspfGridRemapEngine(int numParams, int numBands)
      : theNumberOfParams(numParams), theNumberOfBands(numBands) {}

   rspfGridRemapEngine() {}
   virtual ~rspfGridRemapEngine() {}

   virtual void computeRemapNode(rspfAtbPointSource*  point_source,
                                 void*                 source_value,
                                 void*                 target_value) = 0;
   int  theNumberOfParams;
   int  theNumberOfBands;

   TYPE_DATA
};

#endif
