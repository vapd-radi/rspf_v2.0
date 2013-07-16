//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Utility class to encapsulate parsing projection info in the xml file.
// 
//----------------------------------------------------------------------------
// $Id: rspfFgdcXmlDoc.cpp 2063 2011-01-19 19:38:12Z ming.su $

#include <rspf/support_data/rspfFgdcXmlDoc.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfEpsgProjectionFactory.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfUtmProjection.h>

#include <cmath> /* for std::fabs */

// Static trace for debugging
static rspfTrace traceDebug("rspfFgdcXmlDoc:debug");

rspfFgdcXmlDoc::rspfFgdcXmlDoc()
   : m_xmlFilename(rspfFilename::NIL),
     m_xmlDocument(0),
     m_projection(0),
     m_boundInDegree(false)
{
}

rspfFgdcXmlDoc::~rspfFgdcXmlDoc()
{
}

bool rspfFgdcXmlDoc::open(const rspfFilename& xmlFileName)
{
   bool result = false;
   if ( isOpen() )
   {
      close();
   }
   
   if (xmlFileName.size())
   {
      m_xmlDocument = new rspfXmlDocument(xmlFileName);
      if ( m_xmlDocument.valid() )
      {
         // May want to add a test for valid FGDC here??? (drb)
         m_xmlFilename = xmlFileName;
         result = true;
      }
   }
   return result;
}

void rspfFgdcXmlDoc::close()
{
   m_xmlFilename = rspfFilename::NIL;
   m_xmlDocument = 0;
   m_projection  = 0;
}

rspfRefPtr<rspfProjection> rspfFgdcXmlDoc::getProjection()
{
   if ( (m_projection.valid() == false) && isOpen() )
   {
      rspfString projName;
      if ( getProjCsn(projName) )
      {
         if (!projName.empty())
         {
            m_projection = rspfEpsgProjectionFactory::instance()->createProjection(projName);
            
            if ( m_projection.valid() )
            {
               rspfDpt gsd;
               gsd.makeNan();
               rspf_float64 d;
               if ( getXRes(d) )
               {
                  gsd.x = std::fabs(d);
               }
               if ( getYRes(d) )
               {
                  gsd.y = std::fabs(d);
               }
               rspfMapProjection* mapProj = dynamic_cast<rspfMapProjection*>(m_projection.get());
               if (mapProj)
               {
                  rspfDrect rect;
                  getBoundingBox(rect);
                  if (mapProj->isGeographic())
                  {
                     rspfGpt tie(rect.ul().lat, rect.ul().lon);
                     mapProj->setUlTiePoints(tie);
                     mapProj->setDecimalDegreesPerPixel(gsd);
                  }
                  else
                  {
                     rspfDpt tie(rect.ul().x, rect.ul().y);
                     rspfUnitType unitType = getUnitType();
                     if ( unitType == RSPF_US_SURVEY_FEET)
                     {
                        gsd = gsd * US_METERS_PER_FT;
                        tie = tie * US_METERS_PER_FT;
                     }
                     else if ( unitType == RSPF_FEET )
                     {
                        gsd = gsd * MTRS_PER_FT;
                        tie = tie * MTRS_PER_FT;
                     }
                        
                     mapProj->setUlTiePoints(tie);
                     mapProj->setMetersPerPixel(gsd);
                  }
               }
            }
         }
      }
   }
   return m_projection;
}

rspfUnitType rspfFgdcXmlDoc::getUnitType() const
{
   rspfUnitType result = RSPF_METERS; // default
   rspfString units;
   if ( getPlanarDistantUnits(units) )
   {
      units.downcase();
      if ( units.contains("feet") )
      {
         if ( units.contains("international") )
         {
            result = RSPF_FEET;
         }
         else
         {
            result = RSPF_US_SURVEY_FEET; // Default for feet.
         }
      }
   }
   return result;
}

bool rspfFgdcXmlDoc::isOpen() const
{
   return m_xmlDocument.valid();
}

bool rspfFgdcXmlDoc::getGeoCsn(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/cordsysn/geogcsn";
   return getPath(xpath, s);
}

bool rspfFgdcXmlDoc::getProjCsn(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/cordsysn/projcsn";
   return getPath(xpath, s);
}

bool rspfFgdcXmlDoc::getAltitudeDistantUnits(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/vertdef/altsys/altunits";
   return getPath(xpath, s);
}

bool rspfFgdcXmlDoc::getGridCoordinateSystem(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/planar/gridsys/gridsysn";
   return getPath(xpath, s);
}

rspfRefPtr<rspfProjection> rspfFgdcXmlDoc::getGridCoordSysProjection()
{
   static const char M[] = "rspfFgdcXmlDoc::getGridCoordSysProjection";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered...\n";
   }

   if ( m_projection.valid() == false )
   {
      rspfString s;
      if ( getGridCoordinateSystem(s) )
      {
         rspfString gridsysn = s.downcase();
         if ( getHorizontalDatum(s) )
         {
            rspfString horizdn = s.downcase();
            const rspfDatum* datum = createOssimDatum(s); // throws exception
            
            if ( gridsysn == "universal transverse mercator" )
            {
               // Get the zone:
               if ( getUtmZone(s) )
               {
                  rspf_int32 zone = s.toInt32();

                  //---
                  // Note: Contruct with an origin with our datum.
                  // "proj->setDatum" does not change the origin's datum.
                  // This ensures therspfEpsgProjectionDatabase::findProjectionCode
                  // sets the psc code correctly down the line.
                  //---
                  rspfRefPtr<rspfUtmProjection> utmProj =
                     new rspfUtmProjection( *(datum->ellipsoid()), rspfGpt(0.0,0.0,0.0,datum) );
                  utmProj->setDatum(datum);
                  utmProj->setZone(zone);
                  
                  // Hemisphere( North false easting = 0.0, South = 10000000):
                  bool tmpResult = getUtmFalseNorthing(s);
                  if ( tmpResult && ( s != "0.0" ) )
                  {
                     utmProj->setHemisphere('S');
                  }
                  else
                  {
                     utmProj->setHemisphere('N');
                  }
                  utmProj->setPcsCode(0);

                  rspf_float64 xRes = 0.0;
                  rspf_float64 yRes = 0.0;
                  if (getXRes(xRes) && getYRes(yRes))
                  {
                     rspfDrect rect;
                     getBoundingBox(rect);

                     rspfDpt gsd(std::fabs(xRes), std::fabs(yRes));
                     rspfUnitType unitType = getUnitType();
                   
                     if (m_boundInDegree)
                     {
                        rspfGpt tieg(rect.ul().lat, rect.ul().lon);
                        utmProj->setUlTiePoints(tieg);
                     }
                     else
                     {
                        rspfDpt tie(rect.ul().x, rect.ul().y);
                        if ( unitType == RSPF_US_SURVEY_FEET)
                        {
                           tie = tie * US_METERS_PER_FT;
                        }
                        else if ( unitType == RSPF_FEET )
                        {
                           tie = tie * MTRS_PER_FT;
                        }
                        utmProj->setUlTiePoints(tie);
                     }

                     if ( unitType == RSPF_US_SURVEY_FEET)
                     {
                        gsd = gsd * US_METERS_PER_FT;
                     }
                     else if ( unitType == RSPF_FEET )
                     {
                        gsd = gsd * MTRS_PER_FT;
                     }
                     utmProj->setMetersPerPixel(gsd);
                  }
                  m_projection = utmProj.get(); // Capture projection.
               }
               else
               {
                  std::string errMsg = M;
                  errMsg += " ERROR: Could not determine utm zone!";
                  throw rspfException(errMsg);
               }
            }
         }
      }
   }
   
   if ( traceDebug() )
   {
      if ( m_projection.valid() )
      {
         m_projection->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exiting...\n";
   }
   return m_projection;
}


bool rspfFgdcXmlDoc::getHorizontalDatum(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/geodetic/horizdn";
   return getPath(xpath, s);
}

bool rspfFgdcXmlDoc::getPlanarDistantUnits(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/planar/planci/plandu";
   return getPath(xpath, s);
}

bool rspfFgdcXmlDoc::getUtmFalseNorthing(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/planar/gridsys/utm/transmer/fnorth";
   return getPath(xpath, s); 
}

bool rspfFgdcXmlDoc::getUtmZone(rspfString& s) const
{
   rspfString xpath = "/metadata/spref/horizsys/planar/gridsys/utm/utmzone";
   return getPath(xpath, s); 
}

bool rspfFgdcXmlDoc::getXRes(rspf_float64& v) const
{
   rspfString xpath = "/metadata/spdoinfo/rastinfo/rastxsz";
   if (getPath(xpath, v) == false)
   {
      xpath = "/metadata/spref/horizsys/planar/planci/coordrep/absres";
   }
   return getPath(xpath, v);
}

bool rspfFgdcXmlDoc::getYRes(rspf_float64& v) const
{
   rspfString xpath = "/metadata/spdoinfo/rastinfo/rastysz";
   if (getPath(xpath, v) == false)
   {
      xpath = "/metadata/spref/horizsys/planar/planci/coordrep/ordres";
   }
   return getPath(xpath, v);
}

bool rspfFgdcXmlDoc::getImageSize(rspfIpt& size) const
{
   bool result = false;
   if (  m_xmlDocument.valid() )
   {
      rspfString xpath = "/metadata/spatRepInfo/GridSpatRep/axDimProps/Dimen/dimSize";
      vector<rspfRefPtr<rspfXmlNode> > xml_nodes;
      m_xmlDocument->findNodes(xpath, xml_nodes);
      if (xml_nodes.size() > 1)
      {
         size.x = xml_nodes[0]->getText().toInt32();
         size.y = xml_nodes[1]->getText().toInt32();
         result = true;
      }

      if (result == false)
      {
         xpath = "/metadata/spdoinfo/rastinfo/colcount";
         rspf_float64 x = 0.0;
         result = getPath(xpath, x);
         if (result)
         {
            size.x = (rspf_int32)x;
         }
         xpath = "/metadata/spdoinfo/rastinfo/rowcount";
         rspf_float64 y = 0.0;
         result = getPath(xpath, y);
         if (result)
         {
            size.y = (rspf_int32)y;
         }
      }
   }
   return result;
}

void rspfFgdcXmlDoc::getBoundingBox(rspfDrect& rect) const
{
   rect.makeNan();
   
   if (isOpen())
   {
      double ll_lat = 0.0;
      double ll_lon = 0.0;
      double lr_lat = 0.0;
      double lr_lon = 0.0;
      double ul_lat = 0.0;
      double ul_lon = 0.0;
      double ur_lat = 0.0;
      double ur_lon = 0.0;
     
      rspfString xpath = "/metadata/idinfo/spdom/lboundng/leftbc";
      bool result = getPath(xpath, ul_lon);
      if (!result)
      {
         xpath = "/metadata/idinfo/spdom/bounding/westbc";
         result = getPath(xpath, ul_lon);
         m_boundInDegree = true;
      }
      if (result)
      {
         ll_lon = ul_lon;
      }
      
      xpath = "/metadata/idinfo/spdom/lboundng/rightbc";
      result = getPath(xpath, ur_lon);
      if (!result)
      {
         xpath = "/metadata/idinfo/spdom/bounding/eastbc";
         result = getPath(xpath, ur_lon);
         m_boundInDegree = true;
      }
      if (result)
      {
         lr_lon = ur_lon;
      }

      xpath = "/metadata/idinfo/spdom/lboundng/bottombc";
      result = getPath(xpath, ll_lat);
      if (!result)
      {
         xpath = "/metadata/idinfo/spdom/bounding/southbc";
         result = getPath(xpath, ll_lat);
         m_boundInDegree = true;
      }
      if (result)
      {
         lr_lat = ll_lat;
      }

      xpath = "/metadata/idinfo/spdom/lboundng/topbc";
      result = getPath(xpath, ul_lat);
      if (!result)
      {
         xpath = "/metadata/idinfo/spdom/bounding/northbc";
         result = getPath(xpath, ul_lat);
         m_boundInDegree = true;
      }
      if (result)
      {
         ur_lat = ul_lat;
      }
      
      rect = rspfDrect(rspfDpt(ul_lon, ul_lat),
                        rspfDpt(ur_lon, ur_lat),
                        rspfDpt(lr_lon, lr_lat),
                        rspfDpt(ll_lon, ll_lat), RSPF_RIGHT_HANDED); 
   }
}

rspf_uint32 rspfFgdcXmlDoc::getNumberOfBands()
{
   rspf_uint32 numOfBands = 0;
   rspfString s;
   rspfString xpath = "/metadata/spdoinfo/rastinfo/vrtcount";
   if (getPath(xpath, s))
   {
      numOfBands = s.toInt();
   }
   return numOfBands;
}

bool rspfFgdcXmlDoc::getPath(const rspfString& path, rspfString& s) const
{
   static const char M[] = "rspfFgdcXmlDoc::getPath";
   
   bool result = false;
   if ( m_xmlDocument.valid() )
   {
      std::vector<rspfRefPtr<rspfXmlNode> > xnodes;
      m_xmlDocument->findNodes(path, xnodes);
      if ( xnodes.size() == 1 ) // Error if more than one.
      {
         if ( xnodes[0].valid() )
         {
            s = xnodes[0]->getText();
            result = true;
         }
         else
         {
            if(traceDebug())
            {
               
               rspfNotify(rspfNotifyLevel_WARN)
                  << M << " ERROR:\n"
                  << "Node not found: " << path
                  << std::endl;
            }
         }
      }
      else if ( xnodes.size() == 0 )
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN) << M << " ERROR:\nNode not found: " << path
               << std::endl;
         }
      }
      else
      {
         if(traceDebug())
         {
            
            rspfNotify(rspfNotifyLevel_WARN)
               << M << " ERROR:\nMultiple nodes found: " << path << std::endl;
         }
      }
   }
   if (!result)
   {
      s.clear();
   }
   return result;      
}
   
bool rspfFgdcXmlDoc::getPath(const rspfString& path, rspf_float64& v) const
{
   rspfString s;
   bool result = getPath(path, s);
   if ( result )
   {
      v = s.toFloat64();
   }
   return result;
}

const rspfDatum* rspfFgdcXmlDoc::createOssimDatum(const rspfString& s) const
{
   rspfString horizdn = s.downcase();
   rspfString datumCode;
   if ( horizdn == "north american datum of 1983" )
   {
      datumCode = "NAR-C";
   }
   else if ( horizdn == "north american datum of 1927" )
   {
      datumCode = "NAS-C";
   }
   else if ( horizdn == "wgs84")
   {
      datumCode = "WGE";
   }
   else
   {
      std::string errMsg = "rspfFgdcXmlDoc::createOssimDatum ERROR: Unhandled datum: ";
      errMsg += horizdn.string();
      throw rspfException(errMsg);
   }
   return rspfDatumFactoryRegistry::instance()->create(datumCode);   
}
