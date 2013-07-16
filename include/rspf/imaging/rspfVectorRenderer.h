//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
// 
// Description:
//
// Abstract class for rendering vector data.  Derived classes should implement
// the "rasterizeVectorData" data method.
//               
//*******************************************************************
// $Id: rspfVectorRenderer.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfVectorRenderer_HEADER
#define rspfVectorRenderer_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfDpt3d.h>

class rspfImageViewTransform;
class rspfImageData;

class rspfVectorRenderer : public rspfImageSourceFilter
{
public:
   rspfVectorRenderer();
   rspfVectorRenderer(rspfImageSource* inputSource,
                       rspfImageViewTransform *transform = NULL);
      
   /**
    * Renders the vector data to a data object.  The area which is
    * imaged is the area of interest.  The scale is determined by the output
    * view projection of the ImageViewTransform.
    */
   virtual rspfImageData* renderVectorData() const = 0;

   /**
    * Returns vector of rspfDpt3d points representing the data object at
    * the index passed in.  Implementors should set an error and return an
    * empty vector if the index is out of range.
    */
   virtual vector<rspfDpt3d> getObject(rspf_uint32 index) const = 0;
   
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Sets the ImageViewTransform to be used for converting vector data.
    */
   virtual void setImageViewTransform(rspfImageViewTransform* transform);

   /**
    * Sets the area of interest.  This will be used by the renderVectorData.
    */
   virtual void setAreaOfInterest(const rspfDrect& aoi);

   /**
    * Returns the current area of interest as an rspfDrect.
    */
   virtual rspfDrect getAreaOfInterest() const;

   /**
    * Returns the full bounding rectangle of current vector data loaded.
    */
   virtual rspfDrect getBoundingRect(long resLevel=0)const;

   /**
    * Returns the number of vector objects in the area of interest.
    */
   virtual rspf_int32 getNumberOfAoiObjects() const;

   /**
    * Returns the total number of vector objects in the full bounding
    * rectangle.
    */
   virtual rspf_int32 getNumberOfObjects() const;

protected:
   virtual ~rspfVectorRenderer();
   
   /**
    * Sets the bounding rectangle of current vector data loaded.
    */
   virtual void setBoundingRect(rspfDrect& rect);

   /**
    * Sets the number of vector objects in the area of interest.
    */
   virtual void setNumberOfAoiObjects(rspf_int32 number_of_objects);
 
   /**
    * Sets the number of vector objects in the full bounding rectangle.
    */
   virtual void setNumberOfObjects(rspf_int32 number_of_objects);
   
private:

   rspfImageViewTransform*  theImageViewTransform;
   rspfDrect                theBoundingRect;
   rspfDrect                theAreaOfInterest;
   rspf_int32               theNumberOfAoiObjects;
   rspf_int32               theNumberOfObjects;
   
TYPE_DATA
};

#endif
