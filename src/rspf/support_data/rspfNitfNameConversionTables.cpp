//*******************************************************************
// Copyright (C) 2004 Garrett Potts.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfNitfNameConversionTables.cpp 13219 2008-07-23 19:27:49Z dburken $

#include <cstddef>
#include <rspf/support_data/rspfNitfNameConversionTables.h>
#include <rspf/base/rspfString.h>

struct rspfNitfMapProjectionCodeToOssimName
{
   const char* nitfProjCode; // nitf code name
   const char* projectionName; // rspf projection name
};

struct rspfNitfMapProjectionCodeToNitfName
{
   const char* nitfProjCode; // nitf code name
   const char* projectionName; // rspf projection name
};

static const rspfNitfMapProjectionCodeToOssimName mapProjectiontable[]=
{
   {"AC", "rspfAlbersProjection"},
   {"AL", "rspfAzimEquDistProjection"},
   {"BF", "rspfBonneProjection"},
   {"CP", "rspfEquDistCylProjection"},
   {"CP", "rspfLlxyProjection"},
   {"CS", "rspfCassiniProjection"},
   {"ED", "rspfEckert6Projection"},
   {"EF", "rspfEckert4Projection"},
   {"GN", "rspfGnomonicProjection"},
   {"LE", "rspfLambertConformalConicProjection"},
   {"LI", "rspfCylEquAreaProjection"},
   {"MC", "rspfMercatorProjection"},
   {"MH", "rspfMillerProjection"},
   {"MP", "rspfMollweidProjection"},
   {"NT", "rspfNewZealandMapGridProjection"},
   {"OD", "rspfOrthoGraphicProjection"},
   {"PG", "rspfPolarStereoProjection"},
   {"PH", "rspfPolyconicProjection"},
   {"SA", "rspfSinusoidalProjection"},
   {"SX", "rspfSpaceObliqueMercatorProjection"},
   {"TC", "rspfTransMercatorProjection"},
   {"TX", "rspfTransCylEquAreaProjection"},
   {"VA", "rspfVanDerGrintenProjection"},
   { 0, 0 }
};

static const rspfNitfMapProjectionCodeToNitfName nitfMapProjectiontable[]=
{
   {"AC", "Albers Equal-Area Conic"},
   {"AK", "(Lambert) Azimuthal Equal-Area"},
   {"AL", "Azimuthal Equidistant"},
   {"BF", "Bonne"},
   {"CC", "Equidistant Conic with 1 Standard Parallel"},
   {"CP", "Equirectangular"},
   {"CS", "Cassini-Soldner"},
   {"ED", "Eckert VI"},
   {"EF", "Eckert IV"},
   {"GN", "Gnomonic"},
   {"HX", "Hotine Oblique Mercator based on 2 Points"},
   {"KA", " Equidistant Conic with 2 Standard Parallels"},
   {"LA", "Laborde"},
   {"LE", "Lambert Conformal Conic"},
   {"LI", "Cylindrical Equal Area"},
   {"LJ", " Lambert Equal-Area Meridional"},
   {"MC", "Mercator"},
   {"MH", "Miller Cylindrical"},
   {"MJ", "French Lambert"},
   {"MP", "Mollweide"},
   {"NT", "New Zealand Map Grid"},
   {"OC", "Oblique Mercator"},
   {"OD", "Orthographic"},
   {"PG", "Polar Stereographic"},
   {"PH", "Polyconic"},
   {"RS", "Hotine Oblique Mercator (Rectified Skew Orthomorphic)"},
   {"RB", "Hotine Oblique Mercator (Rectified Skew Orthomorphic)"},
   {"RX", "Robinson"},
   {"SA", "Sinusoidal"},
   {"SD", "Oblique Stereographic"},
   {"SX", "Space Oblique Mercator"},
   {"TC", " Transverse Mercator"},
   {"TX", "Transverse Cylindrical Equal Area"},
   {"VA", "Van der Grinten"},
   {"VX", "General Vertical NearSide Perspective"},
   { 0, 0 }
};

rspfNitfNameConversionTables::rspfNitfNameConversionTables()
{
}

rspfString rspfNitfNameConversionTables::convertMapProjectionNameToNitfCode(const rspfString& mapProjectionName)const
{
   rspf_uint32 idx = 0;

   while(mapProjectiontable[idx].nitfProjCode)
   {
      if(mapProjectionName == mapProjectiontable[idx].projectionName)
      {
         return mapProjectiontable[idx].nitfProjCode;
      }
      ++idx;
   }

   return "";
}

rspfString rspfNitfNameConversionTables::convertNitfCodeToOssimProjectionName(const rspfString& nitfProjectionCode)const
{
   rspf_uint32 idx = 0;

   while(mapProjectiontable[idx].nitfProjCode)
   {
      if(nitfProjectionCode == mapProjectiontable[idx].nitfProjCode)
      {
         return mapProjectiontable[idx].projectionName;
      }
      ++idx;
   }

   return "";
}

rspfString rspfNitfNameConversionTables::convertNitfCodeToNitfProjectionName(const rspfString& nitfProjectionCode)const
{
   rspf_uint32 idx = 0;

   while(nitfMapProjectiontable[idx].nitfProjCode)
   {
      if(nitfProjectionCode == nitfMapProjectiontable[idx].nitfProjCode)
      {
         return nitfMapProjectiontable[idx].projectionName;
      }
      ++idx;
   }

   return "";
}

rspfString rspfNitfNameConversionTables::convertNitfProjectionNameToNitfCode(const rspfString& nitfProjectionName)const
{
   rspf_uint32 idx = 0;

   while(nitfMapProjectiontable[idx].nitfProjCode)
   {
      if(nitfProjectionName ==  nitfMapProjectiontable[idx].projectionName)
      {
         return nitfMapProjectiontable[idx].nitfProjCode;
      }
      ++idx;
   }

   return "";
}
