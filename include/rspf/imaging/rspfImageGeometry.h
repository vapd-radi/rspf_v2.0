//***********************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer (oscarkramer@yahoo.com)
//
// Description: Class declaration of rspfImageGeometry.
//
//***********************************************************************************************
// $Id: rspfImageGeometry.h 3102 2012-01-18 15:30:20Z oscar.kramer $

#ifndef rspfImageGeometry_HEADER
#define rspfImageGeometry_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspf2dTo2dTransform.h>
#include <vector>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfAdjustableParameterInterface.h>

// Forward class declarations:
class rspfDrect;
class rspfIrect;

//***********************************************************************************************
//! Container class that holds both 2D transform and 3D projection information for an image
//! Only one instance of this is alive per image. This is  the object that is returned from
//! a call to rspfImageSource::getImageGeometry()
//! 
//! All images in RSPF have at least a 2D transformation from local (relative to start of file)
//! to full-image coordinates. Often this transform is an identity transform (does nothing). In 
//! some cases however, the first pixel in the file does not correspond to the upper-left pixel of
//! the original, full image. Since the image's sensor model may only apply to full-image pixel 
//! coordinates, it is necessary to reference the latter when making computing ground point 
//! location.  Also the local image could be a reduced resolution level of the full image.  So
//! the image could be a decimated sub image of the full image.
//!
//! Additionally, images typically are accompanied by some form of metadata that defines the mapping
//! from 2D image coordinates to 3D world coordinates. This mapping may be in the form of a map
//! projection for orthorectified images, or a perspective projection such as a sensor model.
//! 
//! This object class maintains both 2D transform and 3D projection information for the associated
//! image. This object will typically be created and "owned" by the image handler object. Therefore,
//! only one copy per image will exist at a time.
//!
//! Notes:
//! 1) Shifts are relative to "full image space".  So if you have a sub image from r2 the shift
//!    given to the transform should be relative to "full image space".
//
//! 2) A decimation of 1.0 is the full image.  This may or may not be r0 as r0 can be decimated.
//!
//***********************************************************************************************
class RSPF_DLL rspfImageGeometry : public rspfObject
{
public:
   typedef std::vector<rspf_float64> DeltaParamList;
   
   //! Default constructor defaults to unity transform with no projection.
   rspfImageGeometry();
   virtual ~rspfImageGeometry();
   
   //! Copy constructor -- performs a deep copy. This is needed when an imageSource in the chain
   //! needs to modify the image geometry at that point in the chain. The image geometry to the 
   //! left of that imageSource must remain unchanged. This constructor permits that imageSource to 
   //! maintain its own geometry based on the input geometry. All chained objects to the right
   //! would see this alternate geometry. See rspfScaleFilter for an example of this case.
   rspfImageGeometry(const rspfImageGeometry& copy_this);

   //! Constructs with projection and transform objects available for referencing. Either pointer
   //! can be NULL -- the associated mapping would be identity.
   rspfImageGeometry(rspf2dTo2dTransform* transform, rspfProjection* projection);

   //! rnToRn is a utility method that takes a rn resolution image point and maps it to the another
   //! rn resolution image point.
   //!
   //! @param inRnPt Is a point in resolution n.
   //! @param inResolutionLevel Is the resolution of the point inRnPt.
   //! @param outResolutionLevel Is the resolution of the point outRnPt.
   //! @param outRnPt Is the result of the transform.
   //!
   void rnToRn(const rspfDpt& inRnPt, rspf_uint32 inResolutionLevel,
               rspf_uint32 outResolutionLevel,rspfDpt& outRnPt) const;
   
   //! rnToFull is a utility method that takes a rn resolution image point and maps it to the full
   //! image point.
   //!
   //! @param rnPt Is a point in resolution n.
   //! @param resolutionLevel Is the resolution of the point rnPt.  a value of 0 is the local image
   //! @param fullPt Is the result of the transform
   //!
   void rnToFull(const rspfDpt& rnPt, rspf_uint32 resolutionLevel, rspfDpt& fullPt) const;

   //! @brief fullToRn is a utility method that takes a full image point and maps it to a rn
   //! resolution image point.
   //! 
   //! @param fullPt Is a point in full image space.
   //! @param resolutionLevel Is the resolution of the point rnPt. A value of 0 is the local image.
   //! @param fullPt Is the result of the transform
   void fullToRn(const rspfDpt& fullPt, rspf_uint32 resolutionLevel, rspfDpt& rnPt) const;

   //! rnToWorld is a utility method that takes a rn resolution image point and maps it to the 
   //! world point.
   //!
   //! @param rnPt Is a point in resolution n.
   //! @param resolutionLevel Is the resolution of the point rnPt.  a value of 0 is the local image
   //! @param wpt Is the result of the transform
   //!
   void rnToWorld(const rspfDpt& rnPt, rspf_uint32 resolutionLevel, rspfGpt& wpt) const;
   
   //! worldToRn is a utility method that takes a world point allows one to transform all the way back to
   //! an rn point.
   //!
   //! @param wpt Ground point.
   //! @param resolutionLevel Is the resolution of the point rnPt.  a value of 0 is the local image
   //! @param rnPt Is the resoltion point.
   //!
   void worldToRn(const rspfGpt& wpt, rspf_uint32 resolutionLevel, rspfDpt& rnPt) const;

   //! Exposes the 3D projection from image to world coordinates. The caller should verify that
   //! a valid projection exists before calling this method. Returns TRUE if a valid ground point
   //! is available in the ground_pt argument. This method depends on the existence of elevation
   //! information. If no DEM is available, the results will be incorrect or inaccurate.
   bool localToWorld(const rspfDpt& local_pt, rspfGpt& world_pt) const;

   //! Exposes the 3D projection from image to world coordinates given a constant height above 
   //! ellipsoid. The caller should verify that a valid projection exists before calling this
   //! method. Returns TRUE if a valid ground point is available in the ground_pt argument.
   bool localToWorld(const rspfDpt& local_pt, const double& h_ellipsoid, rspfGpt& world_pt) const;

   //! Exposes the 3D world-to-local image coordinate reverse projection. The caller should verify 
   //! that a valid projection exists before calling this method. Returns TRUE if a valid image 
   //! point is available in the local_pt argument.
   bool worldToLocal(const rspfGpt& world_pt, rspfDpt& local_pt) const;

   //! Sets the transform to be used for local-to-full-image coordinate transformation
   void setTransform(rspf2dTo2dTransform* transform);

   //! Sets the projection to be used for local-to-world coordinate transformation
   void setProjection(rspfProjection* projection);

   //! Access methods for transform (may be NULL pointer).
   const rspf2dTo2dTransform* getTransform() const { return m_transform.get(); }
   rspf2dTo2dTransform*       getTransform()       { return m_transform.get(); }

   //! Access methods for projection (may be NULL pointer).
   const rspfProjection* getProjection() const { return m_projection.get(); }
   rspfProjection*       getProjection()       { return m_projection.get(); }

   /**
    * @return const rspfMapProjection* or NULL if projection not set or not
    * derived from rspfMapProjection.
    */
   const rspfMapProjection* getAsMapProjection() const
   { return dynamic_cast<const rspfMapProjection*>( m_projection.get() ); }

   /**
    * @return rspfMapProjection* or NULL if projection not set or not
    * derived from rspfMapProjection.
    */
   rspfMapProjection*       getAsMapProjection()
   { return dynamic_cast<rspfMapProjection*>( m_projection.get() ); }

   //! Returns TRUE if valid projection defined
   bool hasProjection() const { return m_projection.valid(); }
   
   //! Returns TRUE if valid transform defined
   bool hasTransform() const { return m_transform.valid(); }

   //! Returns TRUE if this geometry is sensitive to elevation
   bool isAffectedByElevation() const;

   //! Returns the GSD associated with this image in the active projection. Note that this only
   //! makes sense if there is a projection associated with the image. Returns NaNs if no 
   //! projection defined.
   rspfDpt getMetersPerPixel() const;

   /**
    * @brief Get the ground sample distance(GSD) associated with this image
    * in the active projection.
    *
    * GSD is taken from projection if there is no transform set; else,
    * taken from three localToWorld calls (one pixel apart) at the image
    * center.  If the projection or the image size(if needed) is not set the
    * point will be set to NaNs.
    * 
    * @param gsd Point to intialize with GSD.
    * 
    * @note This only makes sense if there is a projection associated with
    * the image.
    *
    * @note Result should be checked for NaNs after call.
    */
   void getMetersPerPixel( rspfDpt& gsd ) const;

   //! Returns the resolution of this image in degrees/pixel. Note that this only
   //! makes sense if there is a projection associated with the image. Returns NaNs if no 
   //! projection defined.
   rspfDpt getDegreesPerPixel() const;

   /**
    * @brief Get the resolution of this image in degrees/pixel.
    *
    * Degrees/pixel is taken from projection if there is no transform set;
    * else, taken from three localToWorld calls (one pixel apart) at the image
    * center.  If the projection or the image size(if needed) is not set the
    * point will be set to NaNs.
    * 
    * @param dpp Point to intialize with degrees/pixel.
    * 
    * @note This only makes sense if there is a projection associated with
    * the image.
    *
    * @note Result should be checked for NaNs after call.
    */
   void getDegreesPerPixel( rspfDpt& dpp ) const;

   //! Assigns the rspfGpts with the ground coordinates of the four corresponding image 
   //! corner points. Returns true if points are valid.
   bool getCornerGpts(rspfGpt& ul, rspfGpt& ur, rspfGpt& lr, rspfGpt& ll) const;

   /**
    * @brief Get the latitude, longitude of the tie point.
    *
    * This is the bounding upper left point of the image which is not
    * necessarily the image (0, 0) point.
    * 
    * @param tie rspfGpt to assign.
    *
    * @param edge If true the tie point is shifted up and to the right by
    * half the gsd.
    *
    * @note Requires projection and image size to be initialized or the
    * latitude and longitue will be set to nan.
    *
    * @note The height parameter of the rspfGpt is not touched by this method.
    */
   void getTiePoint(rspfGpt& tie, bool edge) const;

   /**
    * @brief Get the easting, northing of the tie point.
    *
    * This is the bounding upper left point of the image which is not
    * necessarily the image (0, 0) point.
    * 
    * @param tie rspfDpt to assign.
    *
    * @param edge If true the tie point is shifted up and to the right by
    * half the gsd.
    * 
    * @note Requires projection and image size to be initialized or the
    * easting and northing will be set to nan.
    */
   void getTiePoint(rspfDpt& tie, bool edge) const;

   //! Prints contents to output stream.
   std::ostream& print(std::ostream& out) const;

   //! Returns TRUE if argument geometry has exactly the same instances for transform and 
   //! m_projection. This should be expanded to permit different instances of same transforms
   bool operator==(const rspfImageGeometry& compare_this) const;

   //! Replaces any existing transform and projection with those in the copy_this object
   const rspfImageGeometry& operator=(const rspfImageGeometry& copy_this);

   //! Returns the decimation factor from R0 for the resolution level specified. For r_index=0, the 
   //! decimation factor is by definition 1.0. For the non-discrete case, r_index=1 returns a 
   //! decimation of 0.5. If the vector of discrete decimation factors (m_decimationFactors) is 
   //! empty, the factor will be computed as f=1/2^n
   rspfDpt decimationFactor(rspf_uint32 r_index) const;

   /**
    * @brief Method to get the decimation factor for a given resolution
    * level.
    *
    * If the array of decimations is not initialized by owner, the default is:
    * r_index=0 is by definition 1.0.
    * r_index=n Where n is some level the factor will be computed as f=1/2^n.
    *
    * @param resLevel Reduced resolution set for requested decimation.
    *
    * @param result rspfDpt to initialize with requested decimation.
    */
   void decimationFactor(rspf_uint32 r_index, rspfDpt& result) const;

   /**
    * @brief Gets array of all decimation levels.
    * @param decimations Array to initialiaze.
    */
   void decimationFactors(std::vector<rspfDpt>& decimations) const;

   //! Sets the decimation scheme to a discrete list of decimation factors.
   void setDiscreteDecimation(const std::vector<rspfDpt>& decimation_list) 
      { m_decimationFactors = decimation_list; }

   //! @return The number of decimation factors
   rspf_uint32 getNumberOfDecimations()const
   {
      return (rspf_uint32)m_decimationFactors.size();
   }
   void setImageSize(const rspfIpt& size)
   {
      m_imageSize = size;
   }
   const rspfIpt& getImageSize()const
   {
      return m_imageSize;
   }

   /**
    * @brief Get the bounding rect of (0, 0) to (imageSize.x-1, imageSize.y-1).
    *
    * Relies on image size being initialized.
    *
    * @param bounding_rect Initialized by this method.  Will do a
    * rspfIrect::makeNan() if the image size is not initialized.
    */
   void getBoundingRect(rspfIrect& bounding_rect) const;
   void getBoundingRect(rspfDrect& bounding_rect) const; // Overloaded for drect.
   
   //! Creates a new instance of rspfImageGeometry with the same transform and projection.
   //! Overrides base-class version requiring loadState() and saveState() (not implemented yet)
   virtual rspfObject* dup() const { return new rspfImageGeometry(*this); }

   //! Attempts to initialize a transform and a projection given the KWL. Returns TRUE if no
   //! error encountered.
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   //! Saves the transform (if any) and projection (if any) states to the KWL.
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0) const;

   /**
    * @brief Set m_targetRrds data member.
    *
    * This is used by methods worldToRn and localToWorld that do not take a rrds
    * argument and methods rnToWorld and worldToRn.  If the target rrds is set to 2,
    * then the resulting point from worldToLocal is relative to reduced
    * resolution level 2.  Vice versa for localToWorld it is assumed the local
    * point is relative to the target
    * reduced resolution data set.
    *
    * @param rrds Target (zero based) reduced resolution data set.
    */
   void setTargetRrds(rspf_uint32 rrds);

   /**
    * @return The target zero based reduced resolution data set used for localToWorld and
    * worldToLocal.
    * @see setTargetRrds 
    */
   rspf_uint32 getTargetRrds() const;

   //! @brief  Changes the GSD and image size to reflect the scale provided.
   //! @param scale  The scale to be applied in x and y directions
   //! @param recenterTiePoint If true the will adjust the tie point by shifting the original tie 
   //! to the upper left corner, applying scale, then shifting back by half of either the new
   //! theDeltaLat/lon or theMetersPerPixel depending on if underlying projection isGeographic.
   void applyScale(const rspfDpt& scale, bool recenterTiePoint);

   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType = RSPF_COMPARE_FULL)const;



   // If we have an adjustable parameter interface return one.
   //
   // @return a pointer to an rspfAdjustableParameterInterface or NULL if no
   //         adjustable paramters exist
   //
   virtual rspfAdjustableParameterInterface* getAdjustableParameterInterface();
   virtual const rspfAdjustableParameterInterface* getAdjustableParameterInterface()const;
   
   
   // Compute partials from image to ground with respect to the adjustable parameters.
   //
   //
   bool computeImageToGroundPartialsWRTAdjParam(rspfDpt& result,
                                                          const rspfGpt& gpt,
                                                          rspf_uint32 idx,
                                                          rspf_float64 paramDelta=1.0);
   bool computeImageToGroundPartialsWRTAdjParams(NEWMAT::Matrix& result,
                                                           const rspfGpt& gpt,
                                                           rspf_float64 paramDelta=1.0);
   bool computeImageToGroundPartialsWRTAdjParams(NEWMAT::Matrix& result,
                                                           const rspfGpt& gpt,
                                                           const DeltaParamList& deltas);
   
   bool computeGroundToImagePartials(NEWMAT::Matrix& result,
                                     const rspfGpt& gpt,
                                     const rspfDpt3d& deltaLlh);
   
   bool computeGroundToImagePartials(NEWMAT::Matrix& result,
                                     const rspfGpt& gpt);

   /**
    * @return Returns the angle to "up is up" in decimal degrees, 0.0 if image
    * is not affected by elevation, rspf::nan on error.
    */
   rspf_float64 upIsUpAngle() const;

   /**
    * @return Returns the angle to "north up" in decimal degrees, rspf::nan
    * on error.
    */
   rspf_float64 northUpAngle() const;
   
protected:
   //! @brief Method to back out decimation of a point.
   //! @param rnPt Is a point in resolution n.
   //! @param resolutionLevel Is the resolution of the point rnPt.
   //! @param outPt Is the result of the transform a non-decimated point.
   void undecimatePoint(const rspfDpt& rnPt,
                        rspf_uint32 resolutionLevel,
                        rspfDpt& outPt) const;

   //! @brief Method to apply decimation of a point.
   //! @param inPt Is a point with no decimation.
   //! @param resolutionLevel Is the resolution of the point rnPt.
   //! @param rnPt Is the result of the transform
   void decimatePoint(const rspfDpt& inPt,
                      rspf_uint32 resolutionLevel,
                      rspfDpt& rnPt) const;

   rspfRefPtr<rspf2dTo2dTransform> m_transform;   //!< Maintains local_image-to-full_image transformation 
   rspfRefPtr<rspfProjection>      m_projection;  //!< Maintains full_image-to-world_space transformation
   std::vector<rspfDpt>             m_decimationFactors; //!< List of decimation factors for R-levels
   rspfIpt                          m_imageSize; // Image width and height

   /** @brief Target rrds for localToWorld and worldToLocal methods. */
   rspf_uint32                      m_targetRrds; 

   TYPE_DATA
};

#endif /* #ifndef rspfImageGeometry_HEADER */
