//*****************************************************************************
// FILE: rspfElevSource.h
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer
//
// DESCRIPTION:
//   Contains declaration of class rspfElevSource. This is the base class for
//   all objects providing height information given horizontal ground position.
//
//*****************************************************************************
// $Id: rspfElevSource.h 21380 2012-07-25 13:18:31Z gpotts $

#ifndef rspfElevSource_HEADER
#define rspfElevSource_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGeoid.h>
#include <rspf/elevation/rspfElevationAccuracyInfo.h>
class rspfEcefRay;
class rspfKeywordlist;

/******************************************************************************
 * CLASS:  rspfElevSource
 *
 *****************************************************************************/
class RSPF_DLL rspfElevSource : public rspfSource
{
public:

   virtual const rspfElevSource& operator = (const rspfElevSource& src);
   /**
    * Height access methods:
    */
   virtual double getHeightAboveMSL(const rspfGpt&) = 0;
   virtual double getHeightAboveEllipsoid(const rspfGpt&);

   /**
    *  METHOD: intersectRay()
    *  
    *  Service method for intersecting a ray with the elevation surface to
    *  arrive at a ground point. The ray is expected to originate ABOVE the
    *  surface and pointing down.
    *
    *  NOTE: the gpt argument is expected to be initialized with the desired
    *  datum, including ellipsoid, for the proper intersection point to be
    *  computed.
    *
    *  LIMITATION: This release supports only single valued solutions, i.e., it
    *  is possible a ray passing through one side of a mountain and out the
    *  other will return an intersection with the far side. Eventually,
    *  a more robust algorithm will be employed.
    *
    *  Returns true if good intersection found.
    */
   bool intersectRay(const rspfEcefRay& ray, rspfGpt& gpt, double defaultElevValue = 0.0);
   
   /**
    * Access methods for the bounding elevations:
    */
   virtual double getMinHeightAboveMSL() const;
   virtual double getMaxHeightAboveMSL() const;

   /**
    * Special numeric quantities as defined by this source:
    */
   double getNullHeightValue() const;
   double getSeaLevelValue()   const;

   /**
    * METHOD: pointHasCoverage(gpt)
    * Returns TRUE if coverage exists over gpt:
    */
   virtual bool pointHasCoverage(const rspfGpt&) const = 0;

   /**
    * METHOD: meanSpacingMeters()
    * This method returns the post spacing in the region of gpt:
    */
   virtual double getMeanSpacingMeters() const = 0;


   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const =0;



   /**
    * METHODS: accuracyLE90(), accuracyCE90()
    * Returns the vertical and horizontal accuracy (90% confidence) in the
    * region of gpt:
    */
   //virtual double getAccuracyLE90(const rspfGpt& gpt) const = 0;
   //virtual double getAccuracyCE90(const rspfGpt& gpt) const = 0;


   /**
    *  METHOD:  getBoundingGndRect
    *  
    *  @return  The bounding geograpic rectangle in decimal degrees.
    */
   virtual const rspfGrect& getBoundingGndRect() const;
   
   virtual bool canConnectMyInputTo(rspf_int32 inputIndex,
                                    const rspfConnectableObject* object)const;
   
   /**
    * Implement rspfSource base-class pure virtuals:
    */
   virtual void initialize();

//   virtual rspfFilename getFilename() const;

   /**
    * Opens a stream to an elevation cell.
    *
    * @return Returns true on success, false on error.
    *
    * @note This implementation always returns true.  This should be
    * overridden in derived classes that have streams to files.
    */
//   virtual bool open();
   
   /**
    * Closes the stream to the file.
    *
    * @note This implementation does nothing.  This should be
    * overridden in derived classes that have streams to files.
    */
//   virtual void close();

   /**
    * @return Returns the flag which controls whether or not statistics will
    * be computed upon opening a cell and not finding a corresponding
    * statistics file.
    */
   virtual bool getComputeStatisticsFlag() const;

   /**
    * Sets the flag which controls whether or not statistics will
    * be computed upon opening a cell and not finding a corresponding
    * statistics file.
    *
    * @param flag  true to enable, false to disable.
    *
    * @note This flag can also be controlled by user via the rspf preferences
    * keyword = "elevation.compute_statistics.enabled".
    */
   virtual void setComputeStatisticsFlag(bool flag);


protected:
   virtual ~rspfElevSource();
   /**
    * Constructors/Destructors:
    */
   rspfElevSource();
   rspfElevSource(const rspfElevSource& src);

   /**
    * Data members:
    */
   double     theMinHeightAboveMSL;
   double     theMaxHeightAboveMSL;
   double     theNullHeightValue;
   double     theSeaLevelValue;
   rspfGrect theGroundRect;

   //> Controls statistics gathering.  Defaulted to true.
   bool       theComputeStatsFlag;


private:

TYPE_DATA
};

#endif
