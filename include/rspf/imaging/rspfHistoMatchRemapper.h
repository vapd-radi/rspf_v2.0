//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfHistoMatchRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfHistoMatchRemapper_HEADER
#define rspfHistoMatchRemapper_HEADER
#include <vector>
using namespace std;

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfHistoMatchRemapper: public rspfImageSourceFilter
{
public:
   rspfHistoMatchRemapper();
   rspfHistoMatchRemapper(rspfImageSource* inputSource,
                           const vector<double>& targetMeanPerBand  = vector<double>(),
                           const vector<double>& targetSigmaPerBand = vector<double>(),
                           const vector<double>& inputMeanPerBand   = vector<double>(),
                           const vector<double>& inputSigmaPerBand  = vector<double>()
                           );

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();
   
   const vector<double>& getInputMeanValues()const;
   const vector<double>& getInputSigmaValues()const;
   const vector<double>& getTargetMeanValues()const;
   const vector<double>& getTargetSigmaValues()const;

   void setInputMeanValues(const vector<double>& newValues);
   void setInputSigmaValues(const vector<double>& newValues);
   void setTargetMeanValues(const vector<double>& newValues);
   void setTargetSigmaValues(const vector<double>& newValues);
   

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL);

protected:
   virtual ~rspfHistoMatchRemapper();
   
   rspfRefPtr<rspfImageData> theBlankTile;
   
   vector<double> theTargetMeanPerBand;
   vector<double> theTargetSigmaPerBand;
   vector<double> theInputMeanPerBand;
   vector<double> theInputSigmaPerBand;

   /**
    * transLean
    * @param vIn input value to be transformed
    * @param vBias bias value to be removed
    * @param vTarget value that will replace bias
    * @param vMin minimum valid value of vIn (inclusive)
    * @param vMax maximum valid value of vIn (inclusive)
    * @return vOut
    */
   double transLean(double vIn,
                    double vBias,
                    double vTarget,
                    double vMin,
                    double vMax);

   /**
    * transLeanStretch
    * @param vin Input value to be transformed.
    * @param vBias Bias value to be removed.
    * @param vBiasStretch dispersion (+/-) about vBias
    * @param vTarget value that will replace bias
    * @param vTargetStretch dispersion (+/-) about vTarget
    * @param vMin minimum valid value of vIn (inclusive)
    * @param vMax maximum valid value of vIn (inclusive)
    * @return vOut
    */
   double transLeanStretch(double vIn,
                           double vBias,
                           double vBiasStretch,
                           double vTarget,
                           double vTargetStretch,
                           double vMin,
                           double vMax);
   
};

#endif /* #ifndef rspfHistoMatchRemapper_HEADER */
