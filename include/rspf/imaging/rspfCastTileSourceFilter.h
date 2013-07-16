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
// $Id: rspfCastTileSourceFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfCastTileSourceFilter_HEADER
#define rspfCastTileSourceFilter_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPFDLLEXPORT rspfCastTileSourceFilter : public rspfImageSourceFilter
{
public:
   rspfCastTileSourceFilter(rspfImageSource* inputSource=NULL,
			     rspfScalarType scalarType=RSPF_UCHAR);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();
   virtual rspfScalarType getOutputScalarType() const;

   virtual rspfString getOutputScalarTypeString()const;

   /**
    *  Sets the output scalar type.
    */
   virtual void setOutputScalarType(rspfScalarType scalarType);
   virtual void setOutputScalarType(rspfString scalarType);
   
   /**
    *  Sets the current resolution level.  Returns true on success, false
    *  on error.
    */
   virtual double getNullPixelValue(rspf_uint32 band)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   rspfRefPtr<rspfImageData> applyCast(rspfRefPtr<rspfImageData> input);
   
   /**
    *  Method to save the state of an object to a keyword list.
    *  Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);

   /**
    *   Override base class so that a disableSource event does not
    *   reinitialize the object and enable itself.
    */
   virtual void propertyEvent(rspfPropertyEvent& event);
   virtual void refreshEvent(rspfRefreshEvent& event);   

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
     
protected:
   virtual ~rspfCastTileSourceFilter();

   void allocate(const rspfRefPtr<rspfImageData> inputTile);
   
   rspfRefPtr<rspfImageData> theTile;
   rspfScalarType             theOutputScalarType;

   template<class inType>
   void castInputToOutput(inType **inBuffer,
                          const double  *nullPix,
                          bool   inPartialFlag);
   
   template<class inType, class outType>
   void castInputToOutput(inType *inBuffer[],
                          const double  *nullInPix,
                          bool   inPartialFlag,
                          outType *outBuffer[],
                          const double  *nullOutPix,
                          rspf_uint32   numberOfBands);
TYPE_DATA
};

#endif /* #ifndef rspfCastTileSourceFilter_HEADER */
