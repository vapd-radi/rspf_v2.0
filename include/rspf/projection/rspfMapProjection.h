//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Base class for all map projections.
// 
//*******************************************************************
//  $Id: rspfMapProjection.h 20133 2011-10-12 19:03:47Z oscarkramer $

#ifndef rspfMapProjection_HEADER
#define rspfMapProjection_HEADER 1

#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfDrect.h>
#include <iostream>

#include <rspf/base/rspfMatrix4x4.h>

class rspfKeywordlist;

class RSPFDLLEXPORT rspfMapProjection : public rspfProjection
{
public:
   rspfMapProjection(const rspfEllipsoid& ellipsoid = rspfEllipsoid(),
                      const rspfGpt& origin=rspfGpt());

   rspfMapProjection(const rspfMapProjection& src);

   virtual rspfGpt origin()const;

   /**
    * All map projections will convert the world coordinate to an easting
    * northing (Meters).
    */
   virtual rspfDpt forward(const rspfGpt &worldPoint) const = 0;

   /**
    * Will take a point in meters and convert it to ground.
    */
   virtual rspfGpt inverse(const rspfDpt &projectedPoint)const = 0;

   virtual rspfDpt worldToLineSample(const rspfGpt &worldPoint)const;
   virtual void     worldToLineSample(const rspfGpt &worldPoint,
                                      rspfDpt&       lineSample)const;
   
   virtual rspfGpt lineSampleToWorld(const rspfDpt &projectedPoint)const;
   virtual void     lineSampleToWorld(const rspfDpt &projectedPoint,
                                      rspfGpt& gpt)const;
   /**
    * This is the pure virtual that projects the image point to the given
    * elevation above ellipsoid, thereby bypassing reference to a DEM. Useful
    * for projections that are sensitive to elevation.
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                        const double&   heightAboveEllipsoid,
                                        rspfGpt&       worldPt) const;

   virtual void lineSampleToEastingNorthing(const rspfDpt& liineSample,
                                            rspfDpt& eastingNorthing)const;

   virtual void eastingNorthingToLineSample(const rspfDpt& eastingNorthing,
                                            rspfDpt&       lineSample)const;

   virtual void eastingNorthingToWorld(const rspfDpt& eastingNorthing,
                                       rspfGpt&       worldPt)const;

   
   /** @return The false easting. */
   virtual double getFalseEasting() const;

   /** @return The false northing. */
   virtual double getFalseNorthing() const;
   
   /**
    * Derived classes should implement as needed.
    * This implementation returns 0.0.
    * 
    * @return The first standard parallel.
    */
   virtual double getStandardParallel1() const;

   /**
    * Derived classes should implement as needed.
    * This implementation returns 0.0.
    * 
    * @return The second standard parallel.
    */
   virtual double getStandardParallel2() const;
   
   virtual void update();

   virtual void setPcsCode(rspf_uint32 pcsCode);

   //! Returns the EPSG PCS code or 32767 if the projection is a custom (non-EPSG) projection
   virtual rspf_uint32 getPcsCode()const;

   /**
    *  Returns the projection name.
    */
   virtual rspfString getProjectionName() const;
   
   /**
    * ACCESS METHODS: 
    */
   virtual double getA() const;
   virtual double getB() const;
   virtual double getF() const;
   
   virtual rspfDpt getMetersPerPixel() const;

   /**
    * Returns decimal degrees per pixel as an rspfDpt with
    * "x" representing longitude and "y" representing latitude.
    */
   virtual const rspfDpt& getDecimalDegreesPerPixel() const;
   
   virtual const rspfDpt&   getUlEastingNorthing() const;
   virtual const rspfGpt&   getUlGpt() const;
   virtual const rspfDatum* getDatum() const;
   const rspfEllipsoid&     getEllipsoid() const { return theEllipsoid; }
   const rspfGpt& getOrigin() const;
   virtual bool isGeographic()const;

   /**
    * Applies scale to theDeltaLonPerPixel, theDeltaLatPerPixel and
    * theMetersPerPixel data members (eg: theDeltaLonPerPixel *= scale.x).
    *
    * @param scale Multiplier to be applied to theDeltaLonPerPixel,
    * theDeltaLatPerPixel and theMetersPerPixel
    *
    * @param recenterTiePoint If true the will adjust the tie point by
    * shifting the original tie to the upper left corner, applying scale,
    * then shifting back by half of either the new
    * theDeltaLat/lon or theMetersPerPixel depending on if projection
    * isGeographic.
    */
   virtual void applyScale(const rspfDpt& scale, bool recenterTiePoint);
   
   /**
    * SET METHODS: 
    */
   virtual void setEllipsoid(const rspfEllipsoid& ellipsoid);
   virtual void setAB(double a, double b);

   /**
    * Sets theDatum to datum.
    * Sets theEllipsiod to datum's ellipsiod.
    * Calls update().
    */
   virtual void setDatum(const rspfDatum* datum);

   /**
    * Sets theOrigin to origin.
    * Sets theDatum to origin's datum.
    * Sets theEllipsiod to datum's ellipsiod.
    * Calls update().
    *
    * @param origin The projection origin.
    */
   virtual void setOrigin(const rspfGpt& origin);
   
   virtual void setMetersPerPixel(const rspfDpt& gsd);
   virtual void setDecimalDegreesPerPixel(const rspfDpt& gsd);
   virtual void setUlTiePoints(const rspfGpt& gpt);
   virtual void setUlTiePoints(const rspfDpt& eastingNorthing);
   virtual void setUlEastingNorthing(const  rspfDpt& ulEastingNorthing);
   virtual void setUlGpt(const  rspfGpt& ulGpt);
   virtual void assign( const  rspfProjection &aProjection);
   
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Prints data members to stream.  Returns stream&.
    */
   virtual std::ostream& print(std::ostream& out) const;

   //! Compares this to arg projection and returns TRUE if the same. 
   //! NOTE: As currently implemented in RSPF, map projections also contain image geometry 
   //! information like tiepoint and scale. This operator is only concerned with the map 
   //! specification and ignores image geometry differences.
   virtual bool operator==(const rspfProjection& projection) const;

   //! Computes the approximate resolution in degrees/pixel
   virtual void computeDegreesPerPixel();

   
   /**
    * This will go from the ground point and give
    * you an approximate meters per pixel. the Delta Lat
    * and delta lon will be in degrees.
    */
   virtual void computeMetersPerPixel();

   void setMatrix(double rotation,
                  const rspfDpt& scale,
                  const rspfDpt& translation);
   
   void setMatrixScale(const rspfDpt& scale);
   
   void setMatrixRotation(double rotation);

   void setMatrixTranslation(const rspfDpt& translation);

   /**
    * Utility method to snap the tie point to some multiple.
    *
    * This is used to overcome inaccuracies in formats like NITF's rpc tag
    * witch has a four digit decimal degree precision for the lat/lon offset.
    *
    * @param multiple Value to snap tie point to like 1.0.
    *
    * @param unitType RSPF_METERS or RSPF_DEGREES.
    *
    * EXAMPLE1:
    * projection:    rspfUtmProjection
    * tie point is:  551634.88, 4492036.16
    * multiple:      1.0
    * unitType:      RSPF_METERS
    * new tie point: 551635.00, 4492036.00
    *
    * EXAMPLE2:
    * projection:    rspfEquDistCylProjection
    * tie point is:  -90.3246343      42.0297589
    * multiple:      0.0001
    * unitType:      RSPF_DEGREES
    * new tie point: -90.3246 42.0298
    */
   void snapTiePointTo(rspf_float64 multiple, rspfUnitType unitType);
   void snapTiePointToOrigin();
                      
   void setElevationLookupFlag(bool flag);
   bool getElevationLookupFlag()const;
   rspfUnitType getModelTransformUnitType()const
   {
      return theModelTransformUnitType;
   }
   void setModelTransformUnitType(rspfUnitType unit)
   {
      theModelTransformUnitType = unit;
   }
   bool hasModelTransform()const
   {
      return (theModelTransformUnitType != RSPF_UNIT_UNKNOWN);
   }

   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return false.
    */
   virtual bool isAffectedByElevation() const { return false; }
   
   void setProjectionUnits(rspfUnitType units) { theProjectionUnits = units; }
   rspfUnitType getProjectionUnits() const { return theProjectionUnits; }
   
   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType = RSPF_COMPARE_FULL)const;


protected:
   
   virtual ~rspfMapProjection();

   //---
   // If theModelTransform is set this updates:
   // theDegreesPerPixel
   // theMetersPerPixel
   // theUlEastingNorthing
   // theUlGpt
   //---
   void updateFromTransform();

   /**
    * This method verifies that the projection parameters match the current
    * pcs code.  If not this will set the pcs code to 0.
    *
    * @return true if match, false if not.
    */
   //bool verifyPcsCodeMatches() const;
   
   rspfEllipsoid   theEllipsoid;
   rspfGpt         theOrigin;

   /**
    * This is only set if we want to have built in datum shifting
    */
   const rspfDatum* theDatum;
   
   /**
    * Holds the number of meters per pixel.
    * x = easting
    * y = northing
    */
   rspfDpt          theMetersPerPixel;

   /**
    * Hold the decimal degrees per pixel.
    * x = longitude
    * y = latitude
    */
   rspfDpt          theDegreesPerPixel;

   /** Hold tie point in decimal degrees. */
   rspfGpt          theUlGpt;

   /**
    * Hold tie point as easting northing.
    * x = easting,
    * y = northing
    */
   rspfDpt          theUlEastingNorthing;

   /**
    * Hold the false easting northing.
    * x = false easting,
    * y = false northing
    */
   rspfDpt          theFalseEastingNorthing;

   //! Projection Coordinate System(PCS) code. Mutable because they may update automatically
   //! after the projection is initialized, even in a const method. Normally set to 0 until the
   //! corresponding EPSG code can be determined. If the projection is NOT represented by any
   //! EPSG code, then the PCS is set to 32767.
   mutable rspf_uint32      thePcsCode;

   bool              theElevationLookupFlag;

   // Will always be a 4x4 matrix.
   // note:  only the first 2 dimensions will be used.
   // if the size is 0 then it will not be used
   //
   rspfMatrix4x4 theModelTransform; // goes from image to model
   rspfMatrix4x4 theInverseModelTransform; //goes from model back to image

   // Output Units of the transform
   //
   rspfUnitType theModelTransformUnitType;

   //! Linear units of the projection as indicated in the projection's specification:
   rspfUnitType theProjectionUnits;

TYPE_DATA
};


#endif
