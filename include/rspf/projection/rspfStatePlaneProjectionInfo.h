#ifndef rspfStatePlaneProjectionInfo_HEADER
#define rspfStatePlaneProjectionInfo_HEADER
#include <ostream>
#include <rspf/base/rspfConstants.h> /* for RSPF_DLL macro */
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfString.h>
class rspfDatum;
class rspfMapProjection;
class rspfKeywordlist;
class RSPF_DLL rspfStatePlaneProjectionInfo
{
   friend std::ostream& operator<<(std::ostream&,
                                   const rspfStatePlaneProjectionInfo&);
public:
   rspfStatePlaneProjectionInfo(const std::string&  name,
                                 int                 pcsCode,
                                 const std::string&  projCode,
                                 const std::string&  param1,
                                 const std::string&  param2,
                                 const std::string&  param3,
                                 const std::string&  param4,
                                 double              falseEast,
                                 double              falseNorth,
                                 const std::string&  units,
                                 const std::string&  datumName);
  
   int                code()                  const;
   const rspfString& name()                  const;
   const rspfString& projName()              const;
   double             originLat()             const;
   double             originLon()             const;
   const rspfDatum*  datum()                 const;
   rspfGpt           origin()                const;
   double             parallel1()             const;
   double             parallel2()             const;
   double             falseEasting()          const;
   double             falseNorthing()         const;
   double             falseEastingInMeters()  const;
   double             falseNorthingInMeters() const;
   double             scaleFactor()           const;
   /** Not stored as string; hence, returned by value. */
   rspfString        units()                 const;
   /**
    * 
    * @return rspfUnitType
    */
   rspfUnitType getUnitType() const;
   /**
    * Checks parameters of projection against this.
    *
    * @return true if projection parameters match this, false if not.
    */
   bool matchesProjection(const rspfMapProjection* proj) const;
   
   bool               isSameCode( int Code )  const;
   void populateProjectionKeywords(rspfKeywordlist& kwl,
                                   const char* prefix=0)const;
private:
   
   int                  thePcsCode;       // The Geotiff code 
   rspfString          theName;          // Descriptive PCS name
   
   /*!
    * rspfLambertConformalConicProjection or
    * rspfTransMercatorProjection
    */
   rspfString          theProjectionName;
   
   const rspfDatum*    theDatum; // Either NAD83 or NAD27  based on name
   
   rspfDms             theOriginLat;
   rspfDms             theOriginLon;
   rspfGpt             theOrigin;
   
   double               theParameter3;      // parallel 1   or   Scale factor
   double               theParameter4;      // parallel 2   or      0
   
   double               theFalseEasting;
   double               theFalseNorthing;
   
   double               theScaleFactor;
   
   rspfUnitType        theUnits;        // "us_survey_ft" "feet" or "meters"
};
#endif /* #ifndef rspfStatePlaneProjectionInfo_HEADER */
