//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Matt Revelle
//          David Burken
//
// Description:
//
// Contains class declaration for rspfNitfProjectionFactory.
//
// $Id: rspfNitfProjectionFactory.h 18905 2011-02-16 13:30:11Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfNitfProjectionFactory_HEADER
#define rspfNitfProjectionFactory_HEADER 1

#include <vector>
#include <rspf/projection/rspfProjectionFactoryBase.h>

class rspfProjection;
class rspfString;
class rspfGpt;
class rspfNitfTileSource;
class rspfNitfImageHeader;
class rspfNitfFileHeader;
class rspfDpt;

class RSPFDLLEXPORT rspfNitfProjectionFactory : public rspfProjectionFactoryBase
{
public:

   /**
    * METHOD: instance()
    *
    * @return  Singleton instance of this class.
    */
   static rspfNitfProjectionFactory* instance();

   /**
    * destructor
    */
   virtual ~rspfNitfProjectionFactory();

   /**
    * @param filename This filename can be an image file or
    * it can also be a rspf .geom file.  It could be other
    * forms of geom files as well.  The factories job will be to
    * determine what parser to use and return a projection if
    * successful.
    *
    * @return rspfProjection* on success or NULL if not handled.
    */
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx)const;
   
   /**
    * @param name This would be a projection name.
    *
    * @return rspfProjection* on success or NULL if not handled.
    */
   virtual rspfProjection* createProjection(const rspfString& name)const;

   
   /**
    * @param kwl  Keyword list to instantiate projection from.
    *
    * @return rspfProjection* on success or NULL if not handled.
    */
   virtual rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                             const char* prefix = 0)const;
   
   virtual rspfProjection* createProjection(rspfImageHandler* handler)const;
   /**
    * Creates an object given a type name.
    *
    * @return rspfObject on success or NULL if not handled.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const;

   /**
    * Creates and object given a keyword list.
    *
    * @return rspfObject on success or NULL if not handled.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   /**
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    *
    * @param typeList List to append names to.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;

private:
   
   rspfProjection* createProjectionFromHeaders(
      rspfNitfFileHeader* fileHeader,
      rspfNitfImageHeader* imageHeader)const;
   
   
   /**
    * @param hdr The nitf image header.
    *
    * @param coordianteSystem The coordinate system as a string.
    *
    * @param geographicLocation This should contain the four corner strings.
    *
    * @return Either an  rspfEquDistCylProjection or rspfBilinearProjection
    * depending upon if the corner points line up evenly.
    */
   rspfProjection* makeGeographic(const rspfNitfImageHeader* hdr,
                                   const rspfString& coordinateSysetm)const;//,
                                   //const rspfFilename& filename) const;

   /**
    * @param hdr The nitf image header.
    *
    * @param coordianteSystem The coordinate system as a string.
    *
    * @param geographicLocation This should contain the four corner strings.
    *
    * @return rspfUtmProjection
    */
   rspfProjection* makeUtm(const rspfNitfImageHeader* hdr,
                            const rspfString& coordinateSysetm)const;//,
   
   /**
    * @param hdr The nitf image header.
    *
    * @param gpts Four geographic corner points.
    * 
    * @return rspfEquDistCylProjection* or null if a problem is found.
    */
   rspfProjection* makeEuiDistant(const rspfNitfImageHeader* hdr,
                                   const std::vector<rspfGpt>& gpts) const;
   
   /**
    * @return rspfBilinearProjection* or null if a problem is found.
    */
   rspfProjection* makeBilinear(const rspfNitfImageHeader* hdr,
                                 const std::vector<rspfGpt>& gpts) const;

   /**
    * Parses geographic stream.  This will initialize the vector of rspfGpt's
    * with the four corners.  If the string geographicLocation is not the
    * correct size the vector will be cleared.
    *
    * @param geographicLocation String containing corners.
    *
    * @param gpts Vector to initialize.
    */
   void parseGeographicString(const rspfString& geographicLocation,
                              std::vector<rspfGpt>& gpts) const;

   /**
    * Parses Decimal degree stream.  This will initialize the vector of rspfGpt's
    * with the four corners.  If the string geographicLocation is not the
    * correct size the vector will be cleared.
    *
    * @param geographicLocation String containing corners.
    *
    * @param gpts Vector to initialize.
    */
   void parseDecimalDegreesString(const rspfString& geographicLocation,
                                  std::vector<rspfGpt>& gpts) const;

   void parseUtmString(const rspfString& utmLocation,
                       rspf_uint32& zone,
                       std::vector<rspfDpt>& utmPoints)const;

   /**
    * @brief Gets UTM points, zone and hemisphere from IGEOLO field when ICORDS files = U which is
    * UTM expressed in MGRS.
    * @param mgrsLocationString String from IGEOLO field.
    * @param zone Initialized in method.
    * @param hemisphere Initialized in method.
    * @param utmPoints Initialized in method.
    * @return True on success, false on error.
    */
   bool parseMgrsString(const rspfString& mgrsLocationString,
                        rspf_uint32& zone,
                        char& hemisphere,
                        std::vector<rspfDpt>& utmPoints) const;
   
   /**
    * Computes the scale in decimal degrees with scale.x being longitude,
    * y being latitude.
    *
    * @param hdr The nitf image header from the currently opened nitf file.
    *
    * @param gpts Four geographic corner points.
    *
    * @param scale Scale to initialize.
    *
    * @note scale will be set to nan's on failure.
    */
   void computeScaleInDecimalDegrees(const rspfNitfImageHeader* hdr,
                                     const std::vector<rspfGpt>& gpts,
                                     rspfDpt& scale) const;

   /**
    * Computes the scale in meters with scale.x being easting,
    * y being northing.
    *
    * @param hdr The nitf image header from the currently opened nitf file.
    *
    * @param dpts Four utm corner points.
    *
    * @param scale Scale to initialize.
    *
    * @note scale will be set to nan's on failure.
    */
   void computeScaleInMeters(const rspfNitfImageHeader* hdr,
                             const std::vector<rspfDpt>& dpts,
                             rspfDpt& scale) const;
      
   /**
    * @return true if corner points are parallel to each other, else false.
    */
   bool isSkewed(const std::vector<rspfGpt>& gpts) const;

   /**
    * @return true if corner points are parallel to each other, else false.
    */
   bool isSkewed(const std::vector<rspfDpt>& gpts) const;

   /**
    * @param hdr The nitf image header from the currently opened nitf file.
    * 
    * @param gpts Ground points to initialize from BLOCKA tag.  This should
    * be an empty vector.
    *
    * @return true if BLOCKA tag was parsed.
    */
   bool getBlockaPoints(const rspfNitfImageHeader* hdr,
                        std::vector<rspfGpt>& gpts)const;//,
                        //const rspfFilename& filename) const;
   
   /**
    * Private constructor, users must go through instance() method.
    */
   rspfNitfProjectionFactory();

   /**
    * Private copy constructor to hide from use.
    */
   rspfNitfProjectionFactory(const rspfNitfProjectionFactory& obj);

   /**
    * Private operator = to hide from use.
    */
   rspfNitfProjectionFactory& operator=(const rspfNitfProjectionFactory& rhs);
   
   static rspfNitfProjectionFactory*  theInstance;

   bool isNitf(const rspfFilename& filename)const;
};

#endif /* End of "#ifndef rspfNitfProjectionFactory_HEADER" */
