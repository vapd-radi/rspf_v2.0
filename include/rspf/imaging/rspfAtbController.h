//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains declaration of class rspfAtbController.
//   This is the class for the top-level object controlling the automated
//   tonal balancing (ATB) function. The controller accepts a mosaic.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAtbController.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfAtbController_HEADER
#define rspfAtbController_HEADER

#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfProcessInterface.h>
#include <vector>

class rspfImageCombiner;
class rspfKeywordlist;
class rspfAtbMatchPoint;
class rspfGridRemapSource;
class rspfGridRemapEngine;
class rspfConnectableContainer;

/*****************************************************************************
 *
 * CLASS: rspfAtbController 
 *
 *****************************************************************************/
class rspfAtbController :
   public rspfConnectableObject,
   public rspfProcessInterface
{
public:
   /*!
    * default Constructor initializes the member combiner pointer to NULL.
    */
   rspfAtbController();
   
   /*!
    * Constructor with Combiner reference. This combiner is used for the ATB.
    */
   rspfAtbController(rspfImageCombiner*   combiner,
                      rspfGridRemapEngine* engine=0);

   /*!
    * Constructor with a specified remap engine.
    */
   rspfAtbController(rspfGridRemapEngine* engine);

   ~rspfAtbController();

  virtual rspfObject* getObject()
  {
    return this;
  }
  virtual const rspfObject* getObject()const
  {
    return this;
  }
   /*!
    * Provide alternate means of initialization in case default constructor
    * used.
    */
   void initializeWithCombiner(rspfImageCombiner*   combiner,
			       rspfGridRemapEngine* engine=0);
   
   /*!
    * Enable an image in the mosaic for the tonal balancing process
    */
   bool enableImage (unsigned int index);

   /*!
    * Disnable an image in the mosaic from the tonal balancing process
    */
   bool disableImage(unsigned int index);

   /*!
    * Locks a particular source for adjustment. A locked image
    * effectively defines the target values for all overlapping imagery. 
    */
   bool lockImage      (unsigned int index);

   /*!
    * Unlock a particular source for adjustment. 
    */
   bool unlockImage    (unsigned int index);

   /*!
    * Implementation of rspfProcessInterface pure virtual method.
    */
   virtual bool execute();
   virtual void abort();

   /*!
    * Hook to set the size of the kernel used by all point sources in computing
    * their mean pixel value. The kernels will be resized to NxN.
    */
   void setKernelSize(int side_size);

   /*!
    * Sets the grid spacing used in gridding the mosaic's bounding rectangle.
    */
   void setGridSpacing(const rspfIpt& spacing);

   /*!
    * Sets the ATB remap engine. This object implements the specific tonal
    * balancing algorithm. The dependency on number of bands and color space,
    * is limited only to this engine. This facilitates the implementation of
    * various grid-based image matching schemes.
    */
   void setGridRemapEngine(rspfGridRemapEngine* engine);
   
   /*!
    * Implementations of rspfStateInterface pure virtuals
    */
   virtual bool saveState(rspfKeywordlist& kwl) const;
   virtual bool loadState(const rspfKeywordlist& kwl);
   
   /*!
    * Implementation of rspfConnectableObject pure virtual.
    */
   virtual bool canConnectMyInputTo(rspf_int32 myInputIndex,
                                    const rspfConnectableObject* object)const;

   
protected:
   void initializeRemappers();

   rspfDrect                       theBoundingRect;
   rspfRefPtr<rspfConnectableContainer>       theContainer;
   rspfGridRemapEngine*            theGridRemapEngine;
   std::vector<rspfRefPtr<rspfAtbMatchPoint> > theMatchPoints;
   rspfIpt                         theGridSpacing;

   TYPE_DATA
};

#endif
