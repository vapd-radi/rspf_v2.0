//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Implementation for Base fusion.
//              This base class assumes two inputs where the second input is
//              the intensity source.
//
//*************************************************************************
// $Id: rspfFusionCombiner.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfFusionCombiner_HEADER
#define rspfFusionCombiner_HEADER

#include <rspf/imaging/rspfImageCombiner.h>

class RSPFDLLEXPORT rspfFusionCombiner : public rspfImageCombiner
{
public:
  rspfFusionCombiner();
  rspfFusionCombiner(rspfObject* owner);
  virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0) const;
  bool canConnectMyInputTo(rspf_int32 inputIndex,
			   const rspfConnectableObject* object)const;
   rspfScalarType getOutputScalarType() const;
   
  virtual void initialize();
protected:
   virtual ~rspfFusionCombiner();   
  rspfRefPtr<rspfImageData> theTile;
  rspfRefPtr<rspfImageData> theNormTile;
  rspfRefPtr<rspfImageData> theNormIntensity;
  rspfImageSource*  theInputConnection;
  rspfImageSource*  theIntensityConnection;
  
  rspfRefPtr<rspfImageData> getNormIntensity(const rspfIrect& rect,
                                               rspf_uint32 resLevel);
  rspfRefPtr<rspfImageData> getNormTile(const rspfIrect& rect,
                                          rspf_uint32 resLevel);
TYPE_DATA  
};

#endif
