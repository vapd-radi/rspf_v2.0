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
//  $Id: rspfFftFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfFftFilter_HEADER
#define rspfFftFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfScalarRemapper;

class rspfFftFilter : public rspfImageSourceFilter
{
public:
   enum rspfFftFilterDirectionType
      {
         rspfFftFilterDirectionType_FORWARD = 0,
         rspfFftFilterDirectionType_INVERSE
      };
   rspfFftFilter(rspfObject* owner=NULL);
   rspfFftFilter(rspfImageSource* inputSource);
   rspfFftFilter(rspfObject* owner,
                  rspfImageSource* inputSource);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                   rspf_uint32 resLevel=0);
   virtual void initialize();
   virtual rspf_uint32 getNumberOfOutputBands() const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
  /*!
   * Returns the max pixel of the band.
   */
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
  /*!
   * Each band has a null pixel associated with it.  The null pixel 
   * represents an invalid value.
   */ 
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   
   virtual rspfScalarType getOutputScalarType() const;
   
   void setForward();
   void setInverse();
   rspfString getDirectionTypeAsString()const;
   void setDirectionType(const rspfString& directionType);
   void setDirectionType(rspfFftFilterDirectionType directionType);
      
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;
protected:
   virtual ~rspfFftFilter();
   rspfRefPtr<rspfImageData> theTile;
   rspfFftFilterDirectionType theDirectionType;
   rspfRefPtr<rspfScalarRemapper>        theScalarRemapper;
   template <class T>
   void runFft(T dummy,
               rspfRefPtr<rspfImageData>& input,
               rspfRefPtr<rspfImageData>& output);

   template <class T>
   void fillMatrixForward(T *realPart,
                          T nullPix,
                          NEWMAT::Matrix& real,
                          NEWMAT::Matrix& img)const;
   template <class T>
   void fillMatrixInverse(T *realPart,
                          T *imgPart,
                          NEWMAT::Matrix& real,
                          NEWMAT::Matrix& img)const;


TYPE_DATA
};

#endif
