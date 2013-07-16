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
//  $Id: rspfConvolutionSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfConvolutionSource_HEADER
#define rspfConvolutionSource_HEADER
#include <vector>
#include <iostream>
#include <rspf/matrix/newmat.h>
using namespace std;

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfTilePatch;
class rspfDiscreteConvolutionKernel;

class rspfConvolutionSource : public rspfImageSourceFilter
{
public:
   rspfConvolutionSource();
   rspfConvolutionSource(rspfImageSource* inputSource,
                          const NEWMAT::Matrix& convolutionMatix);
   rspfConvolutionSource(rspfImageSource* inputSource,
                          const vector<NEWMAT::Matrix>& convolutionList);
   
   virtual void setConvolution(const double* kernel, int nrows, int ncols, bool doWeightedAverage=false);
   virtual void setConvolution(const NEWMAT::Matrix& convolutionMatrix, bool doWeightedAverage=false);
   virtual void setConvolutionList(const vector<NEWMAT::Matrix>& convolutionList, bool doWeightedAverage=false);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);

   rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect, rspf_uint32 resLevel=0);
   
   virtual void initialize();
   
protected:
   virtual ~rspfConvolutionSource();

   void allocate();
   
   rspfRefPtr<rspfImageData> theTile;
   rspf_int32                 theMaxKernelWidth;
   rspf_int32                 theMaxKernelHeight;
   
   std::vector<rspfDiscreteConvolutionKernel* > theConvolutionKernelList;
   virtual void setKernelInformation();
   virtual void deleteConvolutionList();

   template<class T>
   void convolve(T dummyVariable,
                 rspfRefPtr<rspfImageData> inputTile,
                 rspfDiscreteConvolutionKernel* kernel);
   

TYPE_DATA
};

#endif /* #ifndef rspfConvolutionSource_HEADER */
