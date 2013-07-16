//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfBlendMosaic.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfBlendMosaic_HEADER
#define rspfBlendMosaic_HEADER
#include <rspf/imaging/rspfImageMosaic.h>

class RSPFDLLEXPORT rspfBlendMosaic : public rspfImageMosaic
{
public:
   rspfBlendMosaic();
   rspfBlendMosaic(rspfConnectableObject::ConnectableObjectList& inputSources);
   virtual void initialize();

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);

   void setWeight(rspf_uint32 index, double weight);
   void setWeights(const std::vector<double>& weights);
   
   double getWeight(rspf_uint32 index) const;
   const std::vector<double>& getWeights()const;


   void setNumberOfWeights(rspf_uint32 numberOfWeights);
   void setAllWeightsTo(double value);
   
   /**
    * Will find the min and max values and then
    * normalize all weights to be from 0 to 1
    */
   void normalizeWeights();
   void findMinMax(double& minValue, double& maxValue)const;
   
protected:
   virtual ~rspfBlendMosaic();
   vector<double>              theWeights;
   rspfRefPtr<rspfImageData> theNormResult;
   /**
    * If our inputs have output of different scalar
    * types then we must normalize so we can blend
    * different scalar data together.
    */
   

   template <class T>
      rspfRefPtr<rspfImageData> combine(T,
                                          const rspfIrect& tileRect,
                                          rspf_uint32);
   template <class T>
      rspfRefPtr<rspfImageData> combineNorm(T,
                                              const rspfIrect& tileRect,
                                              rspf_uint32);
TYPE_DATA
};

#endif /* #ifndef rspfBlendMosaic_HEADER */
