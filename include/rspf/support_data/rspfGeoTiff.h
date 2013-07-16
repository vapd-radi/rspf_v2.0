//***************************************************************************
// FILE: rspfGeoTiff.h
//
// License:  See top level LICENSE.txt file.
//
// Description:
// 
// Class declaration for rspfGeoTiff which is designed to read and hold tag
// information.
//
//***************************************************************************
// $Id: rspfGeoTiff.h 21023 2012-05-29 20:32:22Z dburken $

#ifndef rspfGeoTiff_HEADER
#define rspfGeoTiff_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfString.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfRefPtr.h>

#include <vector>

#include <tiffio.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

class rspfFilename;
class rspfKeywordlist;
class rspfPrivateGtifDef;
class rspfProjection;
class rspfTieGptSet;

class RSPF_DLL rspfGeoTiff : public rspfErrorStatusInterface
{
public:
   /** default constructor */
   rspfGeoTiff();

   rspfGeoTiff(const rspfFilename& file, rspf_uint32 entryIdx=0);

   ~rspfGeoTiff();
  
   enum
   {
      UNDEFINED                        = 0,
      PIXEL_IS_AREA                    = 1,
      PIXEL_IS_POINT                   = 2,
      GT_MODEL_TYPE_GEO_KEY            = 1024,
      GT_RASTER_TYPE_GEO_KEY           = 1025,
      GEOGRAPHIC_TYPE_GEO_KEY          = 2048,
      GEOG_GEODETIC_DATUM_GEO_KEY      = 2050,
      GEOG_ANGULAR_UNITS_GEO_KEY       = 2054,
      PROJECTED_CS_TYPE_GEO_KEY        = 3072,
      PCS_CITATION_GEO_KEY             = 3073,
      PROJECTION_GEO_KEY               = 3074,
      PROJ_COORD_TRANS_GEO_KEY         = 3075,
      LINEAR_UNITS_GEO_KEY             = 3076,
      PROJ_STD_PARALLEL1_GEO_KEY       = 3078,
      PROJ_STD_PARALLEL2_GEO_KEY       = 3079,
      PROJ_NAT_ORIGIN_LONG_GEO_KEY     = 3080,
      PROJ_NAT_ORIGIN_LAT_GEO_KEY      = 3081,
      PROJ_FALSE_EASTING_GEO_KEY       = 3082,
      PROJ_FALSE_NORTHING_GEO_KEY      = 3083,
      PROJ_CENTER_LONG_GEO_KEY         = 3088,
      PROJ_CENTER_LAT_GEO_KEY          = 3089,
      PROJ_SCALE_AT_NAT_ORIGIN_GEO_KEY = 3092,
      LINEAR_METER                     = 9001,
      LINEAR_FOOT                      = 9002,
      LINEAR_FOOT_US_SURVEY            = 9003,
      ANGULAR_DEGREE                   = 9102,
      ANGULAR_ARC_MINUTE               = 9103,
      ANGULAR_ARC_SECOND               = 9104,
      ANGULAR_GRAD                     = 9105,
      ANGULAR_GON                      = 9106,
      ANGULAR_DMS                      = 9107,
      ANGULAR_DMS_HEMISPHERE           = 9108,
      PCS_BRITISH_NATIONAL_GRID        = 27700,
      USER_DEFINED                     = 32767
   };

/*    enum CompressType */
/*    { */
/*       NOT_COMPRESSED = 0, */
/*       COMPRESSED     = 1 */
/*    }; */
 
/*    enum PhotoInterpretation */
/*    { */
/*       PHOTO_MINISWHITE  = 0,   // min value is white  */
/*       PHOTO_MINISBLACK  = 1,   // min value is black  */
/*       PHOTO_RGB         = 2,   // RGB color model  */
/*       PHOTO_PALETTE     = 3,   // color map indexed  */
/*       PHOTO_MASK        = 4,   // $holdout mask  */
/*       PHOTO_SEPARATED   = 5,   // !color separations  */
/*       PHOTO_YCBCR       = 6,   // !CCIR 601  */
/*       PHOTO_CIELAB      = 8    // !1976 CIE L*a*b* */
/*    }; */
   enum ModelType
   {
      UNKNOWN               = 0,
      MODEL_TYPE_PROJECTED  = 1,  // Projection Coordinate System
      MODEL_TYPE_GEOGRAPHIC = 2,  // Geographic latitude-longitude System 
      MODEL_TYPE_GEOCENTRIC = 3
   };

   static int getPcsUnitType(rspf_int32 pcsCode);

   static bool writeTags(TIFF* tiffOut,
                         const rspfRefPtr<rspfMapProjectionInfo> projectionInfo,
                         bool imagineNad27Flag=false);

   /**
    * @brief Writes a geotiff box to a buffer.
    *
    * This will write a degenerate GeoTIFF file to a temp file, copy file to
    * the buffer and then delete the temp file.
    *
    * @param tmpFile The temporary filename.
    * @param rect The output image rect.
    * @param proj Pointer to output projection.
    * @param buf The buffer to stuff with data.
    * @param pixelType RSPF_PIXEL_IS_POINT(0) or RSPF_PIXEL_IS_AREA(1)
    * @return true on success, false on error.
    */
   static bool writeJp2GeotiffBox(const rspfFilename& tmpFile,
                                  const rspfIrect& rect,
                                  const rspfProjection* proj,
                                  std::vector<rspf_uint8>& buf,
                                  rspfPixelType pixelType);
   
   /**
    *  Reads tags.
    *  Returns true on success, false on error.
    */
   bool readTags(const rspfFilename& file, rspf_uint32 entryIdx=0);

   /**
    * @brief Method to parse the tiff file from an open tiff pointer for a
    * given index.
    *
    * This will initialize this container for a subsequent call to
    * addImageGeometry.
    * 
    * @param tiff The opened TIFF* to read from.
    * 
    * @param entryIdx Entry (tiff directory) to read.
    *
    * @param ownTiffPtrFlag If true the tiff pointer will be deleted by this
    * object; else, it will simply zero out the pointer at the end of method.
    * This allows for external code to pass in their open tiff pointer
    * without this object closing it.
    *
    * @return true on success, false on error.
    */
   bool readTags(TIFF* tiff, rspf_uint32 entryIdx, bool ownTiffPtrFlag);

   /**
    *  Returns the map zone as an interger.
    */
   int mapZone() const;

   /**
    *  Add geometry info from tags to keword list.
    *  Returns true on success, false on error.
    */
   bool addImageGeometry(rspfKeywordlist& kwl,
                         const char* prefix=0) const;

   /**
    *  Returns an rspfString representing the rspf projection name.
    *  Returns "unknown" if it can't find a match.
    */
   rspfString getOssimProjectionName() const;

   /**
    *  Attempts to set the rspf projection name from keys read.
    */
   void setOssimProjectionName();

   /**
    *  Returns an rspfString representing the rspf datum name code.
    *  Returns "unknown" if it can't find a match.
    */
   rspfString getOssimDatumName() const;

   /**
    *  Attempts to set the rspf datum code.
    */
   void setOssimDatumName();

   void getScale(std::vector<double>& scale) const;
   void getTiePoint(std::vector<double>& tie_point) const;
   void getModelTransformation(std::vector<double>& transform) const;

   const std::vector<double>& getTiePoint() const;
   const std::vector<double>& getModelTransformation() const;
   const std::vector<double>& getScale() const;
   rspfPixelType getRasterType() const;

   int getWidth() const;
   int getLength() const;
/*    int getSamplesPerPixel() const; */

   /** Prints data members. */
   virtual std::ostream& print(std::ostream& out) const;
   
private:

   // Disallow ...
   rspfGeoTiff(const rspfGeoTiff* /* rhs */ ) {}
   rspfGeoTiff& operator=(const rspfGeoTiff& /*rhs*/) { return *this; }

   //! Initializes data members given a projection code. Returns TRUE if valid PCS code specified.
   //! Resets the PCS code to 0 if invalid.
   bool  parsePcsCode();

   //! Initializes data members given a projection. Returns TRUE if successful.
   bool parseProjection(rspfMapProjection* map_proj);

   /**
    *  Converts double passed in to meters if needed.  The conversion is
    *  base on "theLiniarUnitsCode".
    */
   double convert2meters(double d) const;

   /**
    * @return true if conditions are present to use model transform; false
    * if not.
    */
   bool usingModelTransform() const;

   /**
    * Initializes tieSet from theTiePoints.  Has logic to shift one based
    * ties to be zero based like the rest of the code.
    */
   void getTieSet(rspfTieGptSet& tieSet) const;

   /**
    * Attempts to detect if tie points are one or zero based.
    * @return true if one base, false if not.
    */
   bool hasOneBasedTiePoints() const;

   TIFF*                 theTiffPtr;
   rspf_uint32          theGeoKeyOffset;
   int                   theGeoKeyLength;
   bool                  theGeoKeysPresentFlag;
   int                   theZone;
   rspfString           theHemisphere;
   int                   theDoubleParamLength;
   int                   theAsciiParamLength;
   rspfString           theProjectionName;
   rspfString           theDatumName;
   
   std::vector<double>   theScale;                         // tag 33550
   std::vector<double>   theTiePoint;                      // tag 33922
   std::vector<double>   theModelTransformation;           // tag 34264
   std::vector<double>   theDoubleParam;                   // tag 34736
   rspfString           theAsciiParam;                    // tag 34737
   
   rspf_uint32          theWidth;                         // tag 256
   rspf_uint32          theLength;                        // tag 257
   rspf_uint16          theBitsPerSample;                 // tag 258

   rspf_uint16          theModelType;                     // key 1024
   rspf_uint16          theRasterType;                    // key 1025 undefined=0, area=1, point=2
   rspf_uint16          theGcsCode;                       // key 2048
   rspf_uint16          theDatumCode;                     // key 2050
   rspf_uint16          theAngularUnits;                  // key 2054
   rspf_uint32          thePcsCode;                       // key 3072
   rspfString           thePcsCitation;                   // key 3073
   rspf_uint16          theCoorTransGeoCode;              // key 3075
   rspf_uint16          theLinearUnitsCode;               // key 3076
   double                theStdPar1;                       // key 3078 
   double                theStdPar2;                       // key 3079
   mutable double        theOriginLon;                     // key 3080
   mutable double        theOriginLat;                     // key 3081
   double                theFalseEasting;                  // key 3082
   double                theFalseNorthing;                 // key 3083
   double                theScaleFactor;                   // key 3092
   
   rspfPrivateGtifDef*  thePrivateDefinitions;
   
   static OpenThreads::Mutex theMutex;
};

#endif
