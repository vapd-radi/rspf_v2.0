//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Mingjie Su
//
// Description: Utility class to encapsulate parsing projection info in the xml file.
// 
//----------------------------------------------------------------------------
// $Id: rspfFgdcXmlDoc.h 2063 2011-01-19 19:38:12Z ming.su $
#ifndef rspfFgdcXmlDoc_HEADER
#define rspfFgdcXmlDoc_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/projection/rspfProjection.h>

class rspfDatum;
class rspfDrect;

/** @brief Class for FGDC XML doc parsing. */
class RSPFDLLEXPORT rspfFgdcXmlDoc
{
public:
   /** @brief default constructor */
   rspfFgdcXmlDoc();
   
   /** @brief destructor */
   ~rspfFgdcXmlDoc();

   /**
    * @brief Open method.
    * @return true on success, false on error.
    */
   bool open(const rspfFilename& xmlFileName);

   /** @return true if m_xmlDocument is open; false if not. */
   bool isOpen() const;

   /** @brief close method. */
   void close();

   /** @brief Gets projection from document. */
   rspfRefPtr<rspfProjection> getProjection();

   /** @return Unit type. Note meters is the default if not found in doc. */
   rspfUnitType getUnitType() const;

   /**
    * @return text for path "/metadata/spref/horizsys/cordsysn/geogcsn"
    */
   bool getGeoCsn(rspfString& s) const;

   /**
    * @return text for path "/metadata/spref/horizsys/cordsysn/projcsn"
    */
   bool getProjCsn(rspfString& s) const;

   /**
    * @brief Get the Altitude Distance Units
    *
    * Path: "/metadata/spref/vertdef/altsys/altunits"
    *
    * Domain: "meters" "feet"
    * 
    * @return Text for path.
    */
   bool getAltitudeDistantUnits(rspfString& s) const;

   /**
    * @brief Get the Grid Coordinate system
    *
    * Path: "/metadata/spref/horizsys/planar/gridsys/gridsysn"
    *
    * @return Text for path.
    */
   bool getGridCoordinateSystem(rspfString& s) const;

   /**
    * @brief Gets projection from Grid Coordinate system node.
    *
    * Throws rspfException on error.
    *
    * @return Refptr with projection pointer.  Underlying pointer may be null.
    */
   rspfRefPtr<rspfProjection> getGridCoordSysProjection();

   /**
    * @brief Get the Grid Coordinate system
    *
    * Path: "/metadata/spref/horizsys/planar/gridsys/gridsysn"
    *
    * @return Text for path.
    */
   bool getHorizontalDatum(rspfString& s) const;

   /**
    * @brief Get the Planar Distant Units
    * 
    * Path: "/metadata/spref/horizsys/planar/planci/plandu"
    *
    * Domain: "meters" "international feet" "survey feet"
    *
    * @return Text for path.
    */
   bool getPlanarDistantUnits(rspfString& s) const;

   /**
    * @brief Get UTM false Northing.
    * 
    * Path: "/metadata/spref/horizsys/planar/gridsys/utm/transmer/fnorth"
    *
    * @return Text for path.
    */
   bool getUtmFalseNorthing(rspfString& s) const;
   
   /**
    * @brief Get UTM zone.
    * 
    * Path: "/metadata/spref/horizsys/planar/gridsys/utm/utmzone"
    *
    * Domain: 1 <= UTM Zone Number <= 60 for the northern hemisphere;
    * -60 <= UTM Zone Number <= -1 for the southern hemisphere
     *
    * @return Text for path.
    */
   bool getUtmZone(rspfString& s) const;

   /**
    * @brief Get Bands.
    * 
    * Path: "/metadata/spdoinfo/rastinfo/vrtcount"
    *
    * the maximum number of raster objects along the vertical (z) axis. 
    * For use with rectangular volumetric raster objects (voxels). 
     *
    * @return int for path.
    */
   rspf_uint32 getNumberOfBands();

    /**
    * @brief Gets path from doc and initializes string.
    *
    * This method errors if multiple nodes are found for path.
    * 
    * @param path Xml path to look for.
    * @param s String to initialize.
    * @return true on success and false if path is not found or if there
    * are more than one of path.
    */
   bool getPath(const rspfString& path, rspfString& s) const;

   bool getImageSize(rspfIpt& size) const;

private:

   bool getXRes(rspf_float64& v) const;
   
   bool getYRes(rspf_float64& v) const;

   void getBoundingBox(rspfDrect& rect) const;

   /**
    * @brief Gets path from doc and initializes string.
    * @param path Xml path to look for.
    * @param v Value to initialize.
    * @return true on success and false if path is not found or if there
    * are more than one of path.
    */
   bool getPath(const rspfString& path, rspf_float64& v) const;

   /**
    * Gets the rspfDatum from string.
    */
   const rspfDatum* createOssimDatum(const rspfString& s) const;
   
   rspfFilename                 m_xmlFilename;
   rspfRefPtr<rspfXmlDocument> m_xmlDocument;
   rspfRefPtr<rspfProjection>  m_projection;
   mutable bool                  m_boundInDegree;
};

#endif /* matches: #ifndef rspfFgdcXmlDoc */
