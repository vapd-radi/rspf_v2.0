//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
//
// Author:  Oscar Kramer (rspf port by D. Burken)
//
// Description:
//
// Contains declaration of class rspfSpotDimapSupportData.
//
//*****************************************************************************
// $Id: rspfSpotDimapSupportData.h 17814 2010-08-03 12:44:02Z dburken $
#ifndef rspfSpotDimapSupportData_HEADER
#define rspfSpotDimapSupportData_HEADER

#include <vector>
#include <iostream>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefPoint.h>

class rspfKeywordlist;
// class rspfRefPtr;
class rspfXmlDocument;

class rspfSpotDimapSupportData : public rspfObject,
                                  public rspfErrorStatusInterface
{
public:

   /** metadata.dim format version */
   enum rspfSpotMetadataVersion
   {
      RSPF_SPOT_METADATA_VERSION_UNKNOWN = 0,
      RSPF_SPOT_METADATA_VERSION_1_0 = 1,
      RSPF_SPOT_METADATA_VERSION_1_1 = 2
   };

   rspfSpotDimapSupportData();


   rspfSpotDimapSupportData(const rspfSpotDimapSupportData& rhs);
   rspfSpotDimapSupportData(const rspfFilename& dimapFile,
                             bool  processSwir=false);

   virtual rspfObject* dup()const;

   void clearFields();
   bool loadXmlFile(const rspfFilename& file,
                    bool processSwir=false);

   rspfString   getSensorID()                            const;
   rspfString   getMetadataVersionString()               const;
   rspfString   getAcquisitionDate()                     const;
   rspfString   getProductionDate()                      const;
   rspfString   getImageID()                             const;
   rspfString   getInstrument()                          const;
   rspf_uint32  getInstrumentIndex()                     const;
   rspfFilename getMetadataFile()                        const;
   void          getSunAzimuth(rspf_float64& az)         const;
   void          getSunElevation(rspf_float64& el)       const;
   void          getImageSize(rspfDpt& sz)               const;
   void          getLineSamplingPeriod(rspf_float64& pe) const;
   void          getIncidenceAngle(rspf_float64& ia)     const;
   void          getViewingAngle(rspf_float64& va)       const;
   void          getSceneOrientation(rspf_float64& so)   const;
   rspf_uint32  getNumberOfBands()                       const;
   rspf_uint32  getStepCount()                           const;
   bool          isStarTrackerUsed()                      const;
   bool          isSwirDataUsed()                         const;

   //---
   // Image center point:
   //---

   /** Center of frame, sub image if there is one. */
   void getRefGroundPoint(rspfGpt& gp)         const;

   /** zero base center point */
   void getRefImagePoint(rspfDpt& rp)          const;

   void getRefLineTime(rspf_float64& rt)       const;

   /** relative to full frame. */
   void getRefLineTimeLine(rspf_float64& rtl)  const;

   /** Zero based image rectangle, sub image if there is one. */
   void getImageRect(rspfDrect& rect)const;

   //---
   // Sub image offset:
   //---
   void getSubImageOffset(rspfDpt& offset) const;

   //---
   // Ephemeris (m & m/s):
   //---
   void getPositionEcf(rspf_uint32 sample, rspfEcefPoint& pe) const;
   void getPositionEcf(const rspf_float64& time, rspfEcefPoint& pe) const;
   void getVelocityEcf(rspf_uint32 sample, rspfEcefPoint& ve) const;
   void getVelocityEcf(const rspf_float64& time, rspfEcefPoint& ve) const;
   void getEphSampTime(rspf_uint32 sample, rspf_float64& et) const;

   rspf_uint32 getNumEphSamples() const;

   //---
   // Attitude Angles in RADIANS:
   //---
   void getAttitude(rspf_uint32 sample, rspfDpt3d& at)  const;
   void getAttitude(const rspf_float64& time, rspfDpt3d& at) const;
   void getAttSampTime(rspf_uint32 sample, rspf_float64& at)  const;
   rspf_uint32 getNumAttSamples() const;

   //---
   // Pixel Pointing/Mirror tilt  Angles in RADIANS:
   //---
   void getPixelLookAngleX (rspf_uint32 sample, rspf_float64& pa) const;
   void getPixelLookAngleX (const rspf_float64& sample,
                            rspf_float64& pa) const;
   void getPixelLookAngleY (rspf_uint32 sample, rspf_float64& pa) const;
   void getPixelLookAngleY (const rspf_float64& sample,
                            rspf_float64& pa) const;

   //---
   // Geoposition points provided in the file (most likely just corner points):
   //---
   rspf_uint32 getNumGeoPosPoints() const;
   void getGeoPosPoint (rspf_uint32 point, rspfDpt& ip, rspfGpt& gp) const;

   //---
   // Corner points:
   //---
   void getUlCorner(rspfGpt& pt) const;
   void getUrCorner(rspfGpt& pt) const;
   void getLrCorner(rspfGpt& pt) const;
   void getLlCorner(rspfGpt& pt) const;

   //---
   // Convenient method to print important image info:
   //---
   void  printInfo (ostream& os) const;

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
protected:
   virtual ~rspfSpotDimapSupportData();

private:
   void getLagrangeInterpolation(const rspf_float64& t,
                                 const std::vector<rspfDpt3d>& V,
                                 const std::vector<rspf_float64>& T,
                                 rspfDpt3d& li )const;

   void getBilinearInterpolation(const rspf_float64& t,
                                 const std::vector<rspfDpt3d>& V,
                                 const std::vector<rspf_float64>& T,
                                 rspfDpt3d& li )const;

   void getInterpolatedLookAngle(const rspf_float64& p,
                                 const std::vector<rspf_float64>& angles,
                                 rspf_float64& la) const;

   rspf_float64 convertTimeStamp(const rspfString& time_stamp) const;

   void convertTimeStamp(const rspfString& time_stamp,
                         rspf_float64& ti) const;

   /**
    * Initializes theMetadataVersion.
    * @return true on success, false if not found.
    */
   bool initMetadataVersion(rspfRefPtr<rspfXmlDocument> xmlDocument);

   /**
    * Initializes theImageId.
    * @return true on success, false if not found.
    */
   bool initImageId(rspfRefPtr<rspfXmlDocument> xmlDocument);

   /**
    * From xml section:
    * /Dimap_Document/Dataset_Sources/Source_Information/Scene_Source
    *
    * Initializes:
    * theSunAzimuth
    * theSunElevation
    * theIncidenceAngle
    * @return true on success, false if not found.
    */
   bool initSceneSource(rspfRefPtr<rspfXmlDocument> xmlDocument);

   /**
    * Frame points:
    *
    * From xml section:
    * /Dimap_Document/Dataset_Frame/
    *
    * Initializes:
    * theRefGroundPoint
    * theUlCorner
    * theUrCorner
    * theLrCorner
    * theLlCorner
    * theViewingAngle
    *
    * Note that the theRefImagePoint will be the zero based center of the
    * frame.
    * @return true on success, false if not found.
    */
   bool initFramePoints(rspfRefPtr<rspfXmlDocument> xmlDocument);

   // Extrapolates the attitude for imaging times outside the defined range:
   void extrapolateAttitude(const rspf_float64& time, rspfDpt3d& at) const;


   rspfString                 theSensorID;
   rspfSpotMetadataVersion    theMetadataVersion;
   rspfString                 theImageID;
   rspfFilename               theMetadataFile;
   rspfString                 theProductionDate;
   rspfString                 theInstrument;
   rspf_uint32                theInstrumentIndex;

   /*
    * From xml section:
    * /Dimap_Document/Dataset_Sources/Source_Information/
    * Scene_Source
    */
   rspf_float64               theSunAzimuth;
   rspf_float64               theSunElevation;
   rspf_float64               theIncidenceAngle;
   rspf_float64               theViewingAngle;
   rspf_float64               theSceneOrientation;   
   
   rspfDpt                    theImageSize;

   /** Center of frame on ground, if sub image it's the center of that. */
   rspfGpt                    theRefGroundPoint;

   /** Zero based center of frame. */
   rspfDpt                    theRefImagePoint;

   rspfDpt                    theSubImageOffset;

   /** relative to full image */
   rspf_float64               theRefLineTime;

   /** relative to full image */
   rspf_float64               theRefLineTimeLine;

   rspf_float64               theLineSamplingPeriod;

   /** holds the size of thePixelLookAngleX/Y */
   rspf_uint32                theDetectorCount;
   std::vector<rspf_float64>  thePixelLookAngleX;
   std::vector<rspf_float64>  thePixelLookAngleY;
   std::vector<rspfDpt3d>     theAttitudeSamples; // x=pitch, y=roll, z=yaw
   std::vector<rspf_float64>  theAttSampTimes;
   std::vector<rspfDpt3d>     thePosEcfSamples;
   std::vector<rspfDpt3d>     theVelEcfSamples;
   std::vector<rspf_float64>  theEphSampTimes;
   bool                        theStarTrackerUsed;
   bool                        theSwirDataFlag;
   rspf_uint32                theNumBands;
   rspfString                 theAcquisitionDate;
   rspf_uint32                theStepCount;
   

   //---
   // Corner points:
   //---
   rspfGpt theUlCorner;
   rspfGpt theUrCorner;
   rspfGpt theLrCorner;
   rspfGpt theLlCorner;

   //---
   // Geoposition Points:
   //---
   std::vector <rspfDpt> theGeoPosImagePoints;
   std::vector <rspfGpt> theGeoPosGroundPoints;

   rspfGpt createGround(const rspfString& s)const;
   rspfDpt createDpt(const rspfString& s)const;

   /** callibration information for radiometric corrections*/

   std::vector<rspf_float64> thePhysicalBias;
   std::vector<rspf_float64> thePhysicalGain;

   std::vector<rspf_float64> theSolarIrradiance;

   /**
    * Private parse methods called by loadXml.
    *
    * These "parse" methods were made simply to split the loadXmlMethod
    * into pieces.  This was an attempt to troubleshoot what was thought to
    * be a compiler issue, which turned out to be changed metadata and bad
    * error handling within this code.  Since there were lots of debug
    * statements added I am committing this to cvs.
    * DRB 16 Oct. 2005
    */
   bool parsePart1(rspfRefPtr<rspfXmlDocument> xmlDocument);
   bool parsePart2(rspfRefPtr<rspfXmlDocument> xmlDocument);
   bool parsePart3(rspfRefPtr<rspfXmlDocument> xmlDocument);
   bool parsePart4(rspfRefPtr<rspfXmlDocument> xmlDocument);
};

#endif /* #ifndef rspfSpotDimapSupportData_HEADER */
