//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
//         Orginally written by Jamie Moyers (jmoyers@geeks.com)
//         Adapted from the package KDEM.
// Description: This class parses a DEM header.
//
//********************************************************************
// $Id: rspfDemHeader.cpp 15327 2009-09-01 20:31:16Z dburken $

#include <fstream>
#include <iostream>
#include <iomanip>
#include <rspf/support_data/rspfDemHeader.h>
#include <rspf/support_data/rspfDemUtil.h>

#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordNames.h>

static const char* PROCESS_CODE[]
= { "Autocorrelation resample simple bilinear",
    "Manual profile GRIDEM simple bilinear",
    "DLG/hypsography CTOG 8-direction bilinear",
    "Interpolation from photogrammetic system contours DCASS",
    "direction bilinear",
    "DLG/hypsography LINETRACE, LT4X complex linear",
    "DLG/hypsography CPS-3, ANUDEM, GRASS complex polynomial",
    "Electronic imaging (non-photogrammetric), active or passive",
    "sensor systems" };

static const int MAX_PROCESS_CODE_INDEX = 8;

static const char* GROUND_REF_SYSTEM[]
= { "Geographic",
    "UTM",
    "State Plane" };

static const int MAX_GROUND_REF_SYSTEM_INDEX = 2;

static const char* GROUND_REF_SYSTEM_UNITS[]
= { "Radians",
    "Feet",
    "Meters",
    "Arc Seconds" };

static const int MAX_GROUND_REF_SYSTEM_UNITS_INDEX = 3;


static const char* VERTICAL_DATUM[]
= { "local mean sea level",
    "National Geodetic Vertical Datum 1929",
    "North American Vertical Datum 1988" };

static const int MAX_VERTICAL_DATUM_INDEX = 2;

static const char* HORIZONTAL_DATUM[]
= { "North American Datum 1927 (NAD 27)",
    "World Geoditic System 1972 (WGS 72)",
    "WGS 84",
    "NAD 83",
    "Old Hawaii Datum",
    "Puerto Rico Datum" };

static const int MAX_HORIZONTAL_DATUM_INDEX = 5;


rspfDemHeader::rspfDemHeader()
   : _seGeoCornerX(0.0),
     _seGeoCornerY(0.0),
     _processCode(0),
     _levelCode(0),
     _elevPattern(0),
     _groundRefSysCode(0),
     _groundRefSysZone(0),
     _groundRefSysUnits(0),
     _elevUnits(0),
     _numPolySides(0),
     _counterclockAngle(0.0),
     _elevAccuracyCode(0),
     _minElevation(0),
     _maxElevation(0),
     _spatialResX(0.0),
     _spatialResY(0.0),
     _spatialResZ(0.0),
     _profileRows(0),
     _profileColumns(0),
     _largeContInt(0),
     _maxSourceUnits(0),
     _smallContInt(0),
     _minSourceUnits(0),
     _sourceDate(0),
     _inspRevDate(0),
     _valFlag(0),
     _suspectVoidFlg(0),
     _vertDatum(0),
     _horizDatum(0),
     _dataEdition(0),
     _perctVoid(0),
     _westEdgeFlag(0),
     _northEdgeFlag(0),
     _eastEdgeFlag(0),
     _southEdgeFlag(0),
     _vertDatumShift(0.0)
{
}
   
std::string const&
rspfDemHeader::getQuadName() const
{
   return _quadName;
}

std::string const&
rspfDemHeader::getProcessInfo() const
{
   return _processInfo;
}

double
rspfDemHeader::getSEGeoCornerX() const
{
   return _seGeoCornerX;
}

double
rspfDemHeader::getSEGeoCornerY() const
{
   return _seGeoCornerY;
}

rspf_int32
rspfDemHeader::getProcessCode() const
{
   return _processCode;
}

std::string const&
rspfDemHeader::getSectionIndicator() const
{
   return _sectionIndicator;
}

std::string const&
rspfDemHeader::getMappingCenterCode() const
{
   return _mapCenterCode;
}

rspf_int32
rspfDemHeader::getLevelCode() const
{
   return _levelCode;
}
         
rspf_int32
rspfDemHeader::getElevPattern() const
{
   return _elevPattern;
}

rspf_int32
rspfDemHeader::getGroundRefSysCode() const
{
   return _groundRefSysCode;
}
    
rspf_int32
rspfDemHeader::getGroundRefSysZone() const
{
   return _groundRefSysZone;
}

rspf_int32
rspfDemHeader::getGroundRefSysUnits() const
{
   return _groundRefSysUnits;
}

rspf_int32
rspfDemHeader::getElevationUnits() const
{
   return _elevUnits;
}

rspf_int32
rspfDemHeader::getNumPolySides() const
{
   return _numPolySides;
}

rspfDemPointVector const&
rspfDemHeader::getDEMCorners() const
{
   return _demCorners;
}

double
rspfDemHeader::getCounterclockAngle() const
{
   return _counterclockAngle;
}

rspf_int32
rspfDemHeader::getElevAccuracyCode() const
{
   return _elevAccuracyCode;
}

double
rspfDemHeader::getMinimumElev() const
{
   return _minElevation;
}

double
rspfDemHeader::getMaximumElev() const
{
   return _maxElevation;
}

double
rspfDemHeader::getSpatialResX() const
{
   return _spatialResX;
}

double
rspfDemHeader::getSpatialResY() const
{
   return _spatialResY;
}

double
rspfDemHeader::getSpatialResZ() const
{
   return _spatialResZ;
}

rspf_int32
rspfDemHeader::getProfileRows() const
{
   return _profileRows;
}

rspf_int32
rspfDemHeader::getProfileColumns() const
{
   return _profileColumns;
}

rspf_int32
rspfDemHeader::getLargeContInt() const
{
   return _largeContInt;
}

rspf_int32
rspfDemHeader::getMaxSourceUnits() const
{
   return _maxSourceUnits;
}

rspf_int32
rspfDemHeader::getSmallContInt() const
{
   return _smallContInt;
}

rspf_int32
rspfDemHeader::getMinSourceUnits() const
{
   return _minSourceUnits;
}

rspf_int32
rspfDemHeader::getSourceDate() const
{
   return _sourceDate;
}

rspf_int32
rspfDemHeader::getInspRevDate() const
{
   return _inspRevDate;
}

std::string const&
rspfDemHeader::getInspFlag() const
{
   return _inspFlag;
}

rspf_int32
rspfDemHeader::getDataValFlag() const
{
   return _valFlag;
}

rspf_int32
rspfDemHeader::getSuspectVoidFlag() const
{
   return _suspectVoidFlg;
}

rspf_int32
rspfDemHeader::getVertDatum () const
{
   return _vertDatum;
}

rspf_int32
rspfDemHeader::getHorizDatum() const
{
   return _horizDatum;
}

rspf_int32
rspfDemHeader::getDataEdition() const
{
   return _dataEdition;
}

rspf_int32
rspfDemHeader::getPerctVoid() const
{
   return _perctVoid;
}

rspf_int32
rspfDemHeader::getWestEdgeFlag() const
{
   return _westEdgeFlag;
}

rspf_int32
rspfDemHeader::getNorthEdgeFlag() const
{
   return _northEdgeFlag;
}

rspf_int32
rspfDemHeader::getEastEdgeFlag() const
{
   return _eastEdgeFlag;
}

rspf_int32
rspfDemHeader::getSouthEdgeFlag() const
{
   return _southEdgeFlag;
}

double
rspfDemHeader::getVertDatumShift() const
{
   return _vertDatumShift;
}

std::ostream&
operator<<(std::ostream& s, const rspfDemHeader& header)
{
   return header.print(s);
}

bool rspfDemHeader::open(const rspfFilename& file)
{
   bool result = rspfDemUtil::isUsgsDem(file);
   std::ifstream is(file.c_str(), std::ios_base::in | std::ios_base::binary);
   if ( is.good() )
   {
      open(is);
      is.close();
   }
   else
   {
      result = false;
   }
   return result;
}

std::istream& rspfDemHeader::open(std::istream& is)
{
   if ( is.good() )
   {
      char* bufstr = new char[1024];
      char* temp   = new char[1024];
      rspf_int32 i;
      
      rspfDemUtil::getRecord(is, bufstr);
      
      strncpy(temp, bufstr, 40);
      temp[40] = '\0';
      _quadName = temp;
      
      strncpy(temp,bufstr+40,40);
      temp[40] = '\0';
      _processInfo = temp;
      
      rspfDemUtil::getDouble(bufstr, 109, 13, _seGeoCornerX);
      rspfDemUtil::getDouble(bufstr, 122, 13, _seGeoCornerY);
      _processCode = rspfDemUtil::getLong(bufstr, 135, 1);
      
      strncpy(temp,bufstr+137,3);
      temp[3] = '\0';
      _sectionIndicator = temp;
      
      strncpy(temp,bufstr+140,4);
      temp[4] = '\0';
      _mapCenterCode = temp;
      
      _levelCode = rspfDemUtil::getLong(bufstr, 144, 6);
      _elevPattern = rspfDemUtil::getLong(bufstr, 150, 6);
      _groundRefSysCode = rspfDemUtil::getLong(bufstr, 156, 6);
      _groundRefSysZone = rspfDemUtil::getLong(bufstr, 162, 6);
      _groundRefSysUnits = rspfDemUtil::getLong(bufstr, 528, 6);
      _elevUnits = rspfDemUtil::getLong(bufstr, 534, 6);
      _numPolySides = rspfDemUtil::getLong(bufstr, 540, 6);
      
      for (i = 0; i < 4; i++)
      {
         double x,y;
         rspf_int32 pos = 546 + (i * 48);
         rspfDemUtil::getDouble(bufstr, pos, 24, x);
         rspfDemUtil::getDouble(bufstr, pos + 24, 24, y);
      _demCorners.push_back(rspfDemPoint(x,y));
      }
      
      rspfDemUtil::getDouble(bufstr, 738, 24, _minElevation);
      rspfDemUtil::getDouble(bufstr, 762, 24, _maxElevation);
      rspfDemUtil::getDouble(bufstr, 786, 24, _counterclockAngle );
      _elevAccuracyCode = rspfDemUtil::getLong(bufstr, 810, 6);
      rspfDemUtil::getDouble(bufstr, 816, 12, _spatialResX);
      rspfDemUtil::getDouble(bufstr, 828, 12, _spatialResY);
      rspfDemUtil::getDouble(bufstr, 840, 12, _spatialResZ);
      _profileRows = rspfDemUtil::getLong(bufstr, 852, 6);
      _profileColumns = rspfDemUtil::getLong(bufstr, 858, 6);
      _largeContInt = rspfDemUtil::getLong(bufstr, 864, 5);
      _maxSourceUnits = rspfDemUtil::getLong(bufstr, 869, 1);
      _smallContInt = rspfDemUtil::getLong(bufstr, 870, 5);
      _minSourceUnits = rspfDemUtil::getLong(bufstr, 875, 1);
      _sourceDate = rspfDemUtil::getLong(bufstr, 876, 4);
      _inspRevDate = rspfDemUtil::getLong(bufstr, 880, 4);
      
      strncpy(temp, bufstr+884,1);
      temp[1]='\0';
      _inspFlag = temp;
      
      _valFlag = rspfDemUtil::getLong(bufstr, 885, 1);
      _suspectVoidFlg = rspfDemUtil::getLong(bufstr, 886, 2);
      _vertDatum = rspfDemUtil::getLong(bufstr, 888, 2);
      _horizDatum = rspfDemUtil::getLong(bufstr, 890, 2);
      if (_horizDatum == 0)
         _horizDatum = 1;   // Default to NAD27
      
      _dataEdition = rspfDemUtil::getLong(bufstr, 892, 4);
      _perctVoid = rspfDemUtil::getLong(bufstr, 896, 4);
      _westEdgeFlag = rspfDemUtil::getLong(bufstr, 900, 2);
      _northEdgeFlag = rspfDemUtil::getLong(bufstr, 902, 2);
      _eastEdgeFlag = rspfDemUtil::getLong(bufstr, 904, 2);
      _southEdgeFlag = rspfDemUtil::getLong(bufstr, 906, 2);
      rspfDemUtil::getDouble(bufstr, 908, 7, _vertDatumShift);

      delete [] bufstr;
      delete [] temp;
      bufstr = 0;
      temp = 0;
   }
   return is;
}


std::ostream& rspfDemHeader::print(std::ostream& out) const
{
   const int W = 24; // format width
   const int CW = W-8; // format corner width
   
   // Capture the original flags then set float output to full precision.
   std::ios_base::fmtflags f = out.flags();

   // Note:  This is only a partial print for now...
   std::string prefix = "usgs_dem.";
   
   out << std::setiosflags(std::ios_base::fixed|std::ios_base::left)
       << std::setprecision(10)
      
       << prefix << std::setw(W)
       << "quadrangle_name:" << getQuadName().c_str() << "\n"
       << prefix << std::setw(W)
       << "process_info:" << getProcessInfo().c_str() << "\n"
       << prefix << std::setw(W)
       << "se_geo_corner_x:" << getSEGeoCornerX() << "\n"
       << prefix << std::setw(W)
       << "se_geo_corner_y:" << getSEGeoCornerY() << "\n"
       << prefix << std::setw(W)
       << "process_code:";
   
   rspf_int32 tmpl = getProcessCode() - 1;
   if ( tmpl >= 0 && tmpl <= MAX_PROCESS_CODE_INDEX)
   {
      out << PROCESS_CODE[tmpl] << "\n";
   }
   else
   {
      out << "unknown" << "\n";
   }

   out << prefix << std::setw(W)
       << "section_indicator: " << getSectionIndicator().c_str() << "\n"
       << prefix << std::setw(W)
       << "mapping_center_code: " << getMappingCenterCode().c_str() << "\n"
       << prefix << std::setw(W)
       << "level_code: " << getLevelCode() << "\n"
       << prefix << std::setw(W)
       << "elev_pattern: " << getElevPattern() << "\n"
       << prefix << std::setw(W) << "ground_ref_sys: ";
   
   tmpl = getGroundRefSysCode();
   if ( tmpl >= 0 && tmpl <= MAX_GROUND_REF_SYSTEM_INDEX)
   {
      out << GROUND_REF_SYSTEM[tmpl] << "\n";
   }
   else
   {
      out << "unknown\n";
   }
   
   out << prefix << std::setw(W)
       << "ground_ref_sys_zone: " << getGroundRefSysZone() << "\n"
       << prefix << std::setw(W)
       << "ground_ref_sys_units:";
   tmpl = getGroundRefSysUnits();
   if ( tmpl >= 0 && tmpl <= MAX_GROUND_REF_SYSTEM_UNITS_INDEX)
   {
      out << GROUND_REF_SYSTEM_UNITS[tmpl] << "\n";
   }
   else
   {
      out << "unknown\n";
   }
         
   out << prefix << std::setw(W)
       << "elevation_units: ";
   tmpl = getElevationUnits();
   if ( tmpl >= 0 && tmpl <= MAX_GROUND_REF_SYSTEM_UNITS_INDEX)
   {
      out << GROUND_REF_SYSTEM_UNITS[tmpl] << "\n";
   }
   else
   {
      out << "unknown\n";
   }

   out << prefix << std::setw(W)
       << "number_poly_sides: " << getNumPolySides() << "\n"
       << prefix << std::setw(W)
       << "counterclock_angle: " << getCounterclockAngle()<< "\n"
       << prefix << std::setw(W)
       << "elev_accuracy_code: " << getElevAccuracyCode()<< "\n"
       << prefix << std::setw(W)
       << "minimum_elevation: " << getMinimumElev() << "\n"
       << prefix << std::setw(W)
       << "maximum_elevation: " << getMaximumElev() << "\n"
       << prefix << std::setw(W)
       << "spatial_res_x: " << getSpatialResX() << "\n"
       << prefix << std::setw(W)
       << "spatial_res_y: " << getSpatialResY() << "\n"
       << prefix << std::setw(W)
       << "spatial_res_z:" << getSpatialResZ() << "\n"
       << prefix << std::setw(W)
       << "profile_rows: " << getProfileRows() << "\n"
       << prefix << std::setw(W)
       << "profile_columns:" << getProfileColumns() << "\n"
       << prefix << std::setw(W)
       << "source_date:" << getSourceDate() << "\n"
       << prefix << std::setw(W)
       << "revision_date:" << getInspRevDate() << "\n"
       << prefix  << std::setw(W)
       << "vertical_datum:";
   
   tmpl = getVertDatum() - 1;
   if ( tmpl >= 0 && tmpl <= MAX_VERTICAL_DATUM_INDEX)
   {
      out << VERTICAL_DATUM[tmpl] << "\n";
   }
   else
   {
      out << "unknown\n";
   }

   out << prefix << std::setw(W)
       << "vertical_datum_shift:" << getVertDatumShift() << "\n"
       << prefix << std::setw(W)
       << "horizontal_datum:";
   tmpl = getHorizDatum() - 1;
   if ( tmpl >= 0 && tmpl <= MAX_HORIZONTAL_DATUM_INDEX)
   {
      out << HORIZONTAL_DATUM[tmpl] << "\n";
   }
   else
   {
      out << "unknown\n";
   }

   const rspfDemPointVector CORNERS = getDEMCorners();
   for (unsigned int i=0; i < CORNERS.size(); ++i)
   {
      out << prefix << "corner[" << i
          << std::setw(CW)<< "].x:  " << CORNERS[i].getX() << "\n"
          << prefix << "corner[" << i
          << std::setw(CW)<< "].y:  " << CORNERS[i].getY() << "\n"
          << std::endl;
   }

   out << std::endl;

   // Reset flags.
   out.setf(f);   

   return out;
}

std::istream& operator>>(std::istream& s, rspfDemHeader& header)
{
   return header.open(s);
}

bool rspfDemHeader::getImageGeometry(rspfKeywordlist& kwl,
                                      const char* prefix) const
{
   rspfString type = getProjectionType();
   rspfString datum = getHorizDatumCode();
   if ( (type == "unknown") || (datum == "unknown") )
   {
      return false;
   }

   if (_demCorners.size() != 4)
   {
      return false;
   }

   double tieX   = _demCorners[1].getX();
   double tieY   = _demCorners[1].getY();
   double scaleX = getSpatialResX();
   double scaleY = getSpatialResX();
   
   // Add these for all projections.
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           type);
   
   kwl.add(prefix,
           rspfKeywordNames::DATUM_KW,
           datum);

   if (getGroundRefSysCode() == 0) // Geographic.
   {
      // ESH 10/2008 -- The Dem ground units can be either radians or 
      // arc seconds, so we have to convert parameters in these units 
      // to degrees which is what RSPF is assuming.
      bool bIsArcSecs = (getGroundRefSysUnits() == 3) ? true : false;
      bool bIsRadians = (getGroundRefSysUnits() == 0) ? true : false;

      double convertFactor = 1.0;
      if ( bIsArcSecs == true )
      {
         convertFactor = 1.0 / 3600;
      }
      else if ( bIsRadians == true )
      {
         convertFactor = 180.0 / M_PI;
      }

      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_LON_KW,
              (tieX * convertFactor) );
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_LAT_KW,
              (tieY * convertFactor) );
      kwl.add(prefix,
              rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
              (scaleX * convertFactor) );
      kwl.add(prefix,
              rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
              (scaleY * convertFactor) );
   }
   else if (getGroundRefSysCode() == 1) // UTM
   {
      // Get the zone.
      kwl.add(prefix,
              rspfKeywordNames::ZONE_KW,
              abs(getGroundRefSysZone()));

      // Set the hemisphere.
      kwl.add(prefix,
              rspfKeywordNames::HEMISPHERE_KW,
              (getGroundRefSysZone()<0?"S":"N"));

      // Set the tie.
      if (getGroundRefSysUnits() == 1) // Feet...
      {
         tieX = rspf::usft2mtrs(tieX);
         tieY = rspf::usft2mtrs(tieY);
         scaleX = rspf::usft2mtrs(scaleX);
         scaleY = rspf::usft2mtrs(scaleY);
         
      }
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_EASTING_KW,
              tieX);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_NORTHING_KW,
              tieY);
      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_X_KW,
              scaleX);
      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_Y_KW,
              scaleY);
   }
   else if (getGroundRefSysCode() == 2) // State Plane
   {
      return false; // Need to code...
   }

   
   return true;
}

rspfString rspfDemHeader::getHorizDatumCode() const
{
   rspfString code;
   switch(getHorizDatum())
   {
      case 1:
         code = "NAS-C";
         break;
      case 2:
         code = "WGD";
         break;
      case 3:
         code = "WGE";
        break;         
      case 4:
         code = "NAR-C";
        break;         
      case 5:
         code = "OHA-M";
        break;         
      case 6:
         code = "PUR";
         break;
      default:
         code = "unknown";
        break;         
   }
   return code;
}
         
rspfString rspfDemHeader::getProjectionType() const
{
   rspfString type;
   switch(getGroundRefSysCode())
   {
      case 0:
         type = "rspfEquDistCylProjection";
         break;
      case 1:
         type = "rspfUtmProjection";
         break;
      case 2:
      default:
         type = "unknown";
         break;
   }
   return type;
}
