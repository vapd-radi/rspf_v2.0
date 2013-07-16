//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
// Description:
//
// Class to annotate or draw things on tiles like text, ellipses and so on.
//
// Notes:
// 1) The output scalar type of this object is ALWAYS 8 bit or RSPF_UINT8 so
//    if your input connection is something other than 8 bit it will be
//    remapped to 8 bit.
// 2) This can handle any number of input bands; however, it will never draw
//    to more than three.  So if you have an input connection of four bands
//    the fourth band will not be drawn to.
//
//*************************************************************************
// $Id: rspfAnnotationSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationSource_HEADER
#define rspfAnnotationSource_HEADER

#include <vector>
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/imaging/rspfAnnotationObject.h>

class rspfKeywordlist;

class RSPFDLLEXPORT rspfAnnotationSource : public rspfImageSourceFilter
{
public:
   typedef std::vector<rspfRefPtr<rspfAnnotationObject> > AnnotationObjectListType;
   rspfAnnotationSource(rspfImageSource* inputSource=0);
   virtual ~rspfAnnotationSource();

   virtual rspf_uint32 getNumberOfOutputBands() const;

   virtual rspfScalarType getOutputScalarType() const;

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel = 0);
   
   /*!
    * Will allow this source to initialize its data objects.  If it needs to
    * it should call its connection to get what it needs.
    */
   virtual void initialize();

   /*!
    * Will return the bounding rectangle.
    */
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

   /*!
    * @param bands This can be any number of bands; however, the
    * annotator will only write to a max of 3, so if you have a 4 band
    * image the fourth band will not be annotated.
    */
   virtual void setNumberOfBands(rspf_uint32 bands);
   
   /*!
    * Will add an object to the list.
    * it will own the object added.
    */
   virtual bool addObject(rspfAnnotationObject* anObject);
   
   /*!
    * Will delete an object from the list.
    * Note: the passed in pointer is no long
    *       valid.
    */
   virtual bool deleteObject(rspfAnnotationObject* anObject);

   /*!
    * Saves the current state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual void computeBoundingRect();

   /*!
    * Calls the isPointWithin all Annotation
    * Objects.  Note:  Do not delete these objects.
    */
   AnnotationObjectListType pickObjects(const rspfDpt& imagePoint);

   /*!
    * Note: do not delete the objects returned.
    */
   AnnotationObjectListType pickObjects(const rspfDrect& imageRect);

   const AnnotationObjectListType& getObjectList()const;

   AnnotationObjectListType& getObjectList();

   /*!
     Clears theAnnotationObjectList vector.  All elements are deleted.
   */
   void deleteAll();
   
   virtual void drawAnnotations(rspfRefPtr<rspfImageData> tile);
   
protected:
   
   void allocate(const rspfIrect& rect);
   void destroy();
   
   rspfAnnotationSource(const rspfAnnotationSource& rhs):rspfImageSourceFilter(rhs){}

   /*!
    * What is the size of the image.  This class
    * is an image producer and it needs to know about
    * its bounds. We will default the rectangle to an
    * empty region.
    */
   rspfDrect theRectangle;
   
   /*!
    * The output band can be set to 1 or 3 for
    * grey or RGB scale. We will default this to
    * a 1 band image.
    */
   rspf_uint32 theNumberOfBands;

   /*!
    * This has all the routines we need for drawing
    * lines, circles and polygons.
    */
   rspfRefPtr<rspfRgbImage> theImage;
   
   rspfRefPtr<rspfImageData> theTile;

   /*!
    * These will be all your primitives in
    * Example: lines, circles, text ...
    */
   AnnotationObjectListType theAnnotationObjectList;   

TYPE_DATA
};

#endif /* #ifndef rspfAnnotationSource_HEADER */

