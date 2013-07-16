//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains declaration of class rspfAtbPointSource.
//   This object provides the statistics associated with a given point on a
//   given image corresponding to a matchpoint. A matchpoint contains a
//   collection of these point sources, one for each contributing image.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAtbPointSource.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfAtbPointSource_HEADER
#define rspfAtbPointSource_HEADER

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>

class rspfImageSource;
class rspfImageData;
class rspfGridRemapSource;
class rspfGridRemapEngine;

/*!****************************************************************************
 *
 * CLASS: rspfAtbPointSource 
 *
 *****************************************************************************/
class rspfAtbPointSource : public rspfReferenced
{
public:
   rspfAtbPointSource();
   rspfAtbPointSource(rspfGridRemapSource* remap_source,
                       const rspfDpt&       view_point);


   /*!
    * Sets the pointer of the source of pixels used to compute the stats.
    */
   void setRemapSource(rspfGridRemapSource* image_source);

   /*!
    * Returns the reference to the remapper feeding this source.
    */
   rspfGridRemapSource* getRemapSource() { return theRemapSource.get(); }

   /*!
    * Sets the view coordinates corresponding to this point.
    */
   void setViewPoint(const rspfDpt& view_point);

   /*!
    * Returns the view point corresponding to this point source.
    */
   const rspfDpt& getViewPoint() const { return theViewPoint; }

   /*!
    * Returns the computed value vector corresponding to the region about the
    * view point. The ATB Remap engine performs the actual computation, since
    * the definition of "value" is algorithm dependent (the value may be in a
    * different color space).
    */
   void getSourceValue(void* value);

   /*!
    * Method to set the kernel size used in computing statistics. The kernel
    * will be resized to NxN.
    */
   void setKernelSize(int side_size);

   /*!
    * Sets the ATB remap engine reference owned by the controller. This engine
    * is the only object that understands how to compute remap parameters from
    * pixel data. This permits easily modifying the ATB algorithm without
    * the support classes such as this one.
    */
   void setGridRemapEngine(rspfGridRemapEngine* engine)
      { theGridRemapEngine = engine; }
protected:
   virtual ~rspfAtbPointSource();

private:
   
   rspfRefPtr<rspfGridRemapSource> theRemapSource;
   rspfDpt              theViewPoint;
   rspfRefPtr<rspfGridRemapEngine> theGridRemapEngine;
   int                   theKernelSize;
   bool                  theViewPointIsValid;
};

#endif
