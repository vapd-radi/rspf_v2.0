//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Garrett Potts (gpotts@imagelinks.com)  
//         Oscar Kramer (oscar@krameranalytic.com)
//
// DESCRIPTION: Contains declaration of rspfImageViewProjectionTransform.
//    This class provides an image to view transform that utilizes two
//    independent 2D-to-3D projections. Intended for transforming view to
//    geographic "world" space to input image space.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfImageViewProjectionTransform.h 20352 2011-12-12 17:24:52Z dburken $

#ifndef rspfImageViewProjectionTransform_HEADER
#define rspfImageViewProjectionTransform_HEADER 1

#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/imaging/rspfImageGeometry.h>

class RSPFDLLEXPORT rspfImageViewProjectionTransform : public rspfImageViewTransform
{
public:
   rspfImageViewProjectionTransform(rspfImageGeometry* imageGeometry=0,
                                     rspfImageGeometry* viewGeometry=0);

   //! copy constructor 
   rspfImageViewProjectionTransform(const rspfImageViewProjectionTransform& src);

   virtual rspfObject* dup() const { return new rspfImageViewProjectionTransform(*this); }
   virtual ~rspfImageViewProjectionTransform();

   //! Satisfies base class pure virtual. Returns TRUE if both input and output geoms exist.
   virtual bool isValid() const { return (m_imageGeometry.valid() && m_viewGeometry.valid()); }

   //! Returns TRUE if both input and output geometries are identical. Presently implemented as
   //! limited compare of geometry pointers
   virtual bool isIdentity() const { return (m_imageGeometry == m_viewGeometry); }

   //! Assigns the geometry to use for output view. This object does NOT own the geometry.
   void setViewGeometry(rspfImageGeometry* g) { m_viewGeometry = g; }   

   //! Assigns the geometry to use for input image. This object does NOT own the geometry.
   void setImageGeometry(rspfImageGeometry* g) { m_imageGeometry = g; }  

   //! Workhorse of the object. Converts image-space to view-space.
   virtual void imageToView(const rspfDpt& imagePoint, rspfDpt& viewPoint) const;

   //! Other workhorse of the object. Converts view-space to image-space.
   virtual void viewToImage(const rspfDpt& viewPoint, rspfDpt& imagePoint) const;

   //! Dumps contents to stream
   virtual std::ostream& print(std::ostream& out) const;
   
   rspfImageGeometry* getImageGeometry()  { return m_imageGeometry.get(); }
   rspfImageGeometry* getViewGeometry()   { return m_viewGeometry.get(); }
   const rspfImageGeometry* getImageGeometry()const  { return m_imageGeometry.get(); }
   const rspfImageGeometry* getViewGeometry()const   { return m_viewGeometry.get(); }
   
   //! OLK: Not sure where this is used, but needed to satisfy rspfViewInterface base class.
   //! The ownership flag is ignored.
   virtual bool setView(rspfObject* baseObject);
   virtual       rspfObject* getView()       { return m_viewGeometry.get(); }
   virtual const rspfObject* getView() const { return m_viewGeometry.get(); }

   //! Returns the GSD of input image.
   virtual rspfDpt getInputMetersPerPixel()const;

   //! Returns the GSD of the output view.
   virtual rspfDpt getOutputMetersPerPixel() const;

   //! Gets the image bounding rect in view-space coordinates
   virtual rspfDrect getImageToViewBounds(const rspfDrect& imageRect)const;
   
   //! After rewrite for incorporating rspfImageGeometry: No longer needed.  
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix =0);
   
   //! After rewrite for incorporating rspfImageGeometry: No longer needed.  
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix = 0)const;

protected:

   /**
    * @brief Initializes the view geometry image size from image geometry
    * bounding rect.
    *
    * This is needed for the rspfImageGeometry::worldToLocal if the underlying
    * projection is geographic to handle images that cross the date line.
    * 
    * @param Input image rectangle.
    * @return true on success, false on error.
    */
   bool initializeViewSize();  
   
   rspfRefPtr<rspfImageGeometry> m_imageGeometry;
   rspfRefPtr<rspfImageGeometry> m_viewGeometry;
   
TYPE_DATA
};

#endif
