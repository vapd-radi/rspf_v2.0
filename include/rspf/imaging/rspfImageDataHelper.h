//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfImageDataHelper.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfImageDataHelper_HEADER
#define rspfImageDataHelper_HEADER

#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/base/rspfRgbVector.h>

class rspfImageDataHelper
{
public:
   /*!
    * This must be a valid image data object
    */
   rspfImageDataHelper(rspfImageData* imageData=0);

   void setImageData(rspfImageData* imageData);

   void fill(const double* values,
             const rspfIrect& rect,
             bool clipPoly=true);

   void fill(const double* values,
             std::vector<rspfPolygon>& regionList,
             bool clipPoly=true);
   
   void fill(const rspfRgbVector& color,
             std::vector<rspfPolygon>& regionList,
             bool clipPoly=true);
   
   void fill(const rspfRgbVector& color,
             const rspfIrect& rect,
             bool clipPoly=true);
   /*!
    * Values must be of the same type as the image data that
    * we are writing to.
    */
   void fill(const double*  values,
             const rspfPolygon& region,
             bool clipPoly=true);

   void fill(const rspfRgbVector& color,
             const rspfPolygon& region,
             bool clipPoly=true);
   /*!
    * The input is assumed to have the same origin, size and scalar
    * as this object.  This will implement a selection
    */
   void copyInputToThis(const void* input,
                        const rspfPolygon& region,
                        bool clipPoly=true);

   rspfImageData* theImageData;
   

protected:
   rspfIpt        theOrigin;
   rspfIrect      theImageRectangle;
   rspfPolyArea2d thePolyImageRectangle;
   
   template <class T>
   void fill(T dummyVariable,
             const double* values,
             const rspfPolygon& region,
             bool clipPoly);

   template <class T>
   void fill(T dummyVariable,
             const double* values,
             const rspfIrect& region,
             bool clipPoly);
   
   template <class T>
   void copyInputToThis(const T* inputBuf,
                        const rspfPolygon& region,
                        bool clipPoly);

   template <class T>
   void fill(T dummyVariable,
             const double* values,
             const rspfPolygon& region);
   template <class T>
   void fill(T dummyVariable,
             const double* values,
             const rspfIrect& region);
   
   template <class T>
   void copyInputToThis(const T* inputBuf,
                        const rspfPolygon& region);

   
};

#endif
