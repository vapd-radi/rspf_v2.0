//-----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Class declaration of rspfImageModel.
//
//-----------------------------------------------------------------------------
// $Id$

#ifndef rspfImageModel_HEADER
#define rspfImageModel_HEADER 1

#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfRtti.h>

class rspfDrect;
class rspfImageHandler;

/**
 * @brief Class to handle transforming image points, rectangles, and offsets
 * from one reduced resolution data set (rrds) level to another.
 *
 * Derived from rspfObject only so users can pass via the
 * rspfViewInterface::setView method.
 */
class RSPF_DLL rspfImageModel : public rspfObject
{
public:
   
   /** @brief default constructor */
   rspfImageModel();


   /**
    * @brief Method to initialize class from an image handler.
    *
    * @param ih Image handler.
    */
   virtual void initialize(const rspfImageHandler& ih);

   /**
    * @brief Get r0 point from rn point.
    * 
    * @param rrds Source (rnPt) reduced resolution data set.
    *
    * @param rnPt The image point to tranform.
    *
    * @param r0Pt the Point to initialize.
    *
    * @note Throws rspfException on out of range rrds.
    */
   void rnToR0(rspf_uint32 rrds,
               const rspfDpt& rnPt,
               rspfDpt& r0Pt) const;

   /**
    * @brief Get r0 point from rn point.
    *
    * This requires calling setTargetRrds(rspf_uint32 rrds) to the level
    * for rnPt.
    * 
    * @param rnPt The image point to tranform.
    *
    * @param r0Pt the Point to initialize.
    *
    * @note Throws rspfException on out of range rrds.
    *
    * @see setTargetRrds
    */
   void rnToR0(const rspfDpt& rnPt, rspfDpt& r0Pt) const;
   
   /**
    * @brief Get rn point from r0 point.
    * 
    * @param rrds Target (rnPt) reduced resolution data set.
    *
    * @param r0Pt The image point to tranform.
    *
    * @param rnPt the Point to initialize.
    *
    * @note Throws rspfException on out of range rrds.
    */
   void r0ToRn(rspf_uint32 rrds,
               const rspfDpt& r0Pt,
               rspfDpt& rnPt) const;

   /**
    * @brief Get rn point from r0 point.
    *
    * This requires calling setTargetRrds(rspf_uint32 rrds) to the level
    * for rnPt.
    * 
    * @param r0Pt The image point to tranform.
    *
    * @param rnPt the Point to initialize.
    *
    * @note Throws rspfException on out of range rrds.
    *
    * @see setTargetRrds
    */
   void r0ToRn(const rspfDpt& r0Pt,
               rspfDpt& rnPt) const;

   /**
    * @brief Get the sub image offset for a given resolution level.
    * 
    * @param rrds The reduced resolution data set.
    *
    * @param offset the Point to initialize.
    *
    * @note Throws rspfException on out of range rrds.
    */
   void getSubImageOffset(rspf_uint32 rrds, rspfDpt& offset) const;

   /**
    * @brief Gets the zero-based image rectangle for a given reduced resolution
    * data set.
    *
    * @param rrds The reduced resolution data set.
    *
    * @param rect Initialized with image rectangle for rrds.
    *
    * @note Throws rspfException on out of range rrds.
    */
   void getImageRectangle(rspf_uint32 rrds, rspfDrect& rect) const; 
   
   /**
    * @brief Gets the model-based image rectangle for a given reduced
    * resolution data set.
    *
    * If this image is a sub image the offset is applied.  So if the image
    * has a sub image offset of (1024, 1024), and has 1024 lines and 1024
    * samples the rectangle for r0 will be:  (1024, 1024) (2047, 2047)
    *
    * @param rrds The reduced resolution data set.
    *
    * @param rect Initialized with image rectangle for rrds.
    *
    * @note Throws rspfException on out of range rrds.
    */
   void getBoundingRectangle(rspf_uint32 rrds, rspfDrect& rect) const;

   /**
    * @return This returns the total number of decimation levels.
    */
   rspf_uint32 getNumberOfDecimationLevels()const;

   /**
    * @brief Set theTargetRrds data member.
    *
    * This is used by methods rnToR0 and r0ToRn that do not take a rrds
    * argument.
    *
    * @param rrds Target reduced resolution data set.
    */
   void setTargetRrds(rspf_uint32 rrds);

   /**
    * @return The target reduced resolution data set.
    */
   rspf_uint32 getTargetRrds() const;
   
 

protected:
   /** @brief virtual destructor */
   virtual ~rspfImageModel();

   /** Offset from the full image. */
   rspfDpt theSubImageOffset;

   /** Decimation factors for each rrds level. */
   std::vector<rspfDpt> theDecimationFactors;

   rspf_uint32 theLines;
   rspf_uint32 theSamples;
   rspf_uint32 theTargetRrds;

TYPE_DATA
};

#endif /* #ifndef rspfImageModel_HEADER */
