//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Contains class definition for rspfGeoTiffCoordTransformsLut.
//*******************************************************************
//  $Id: rspfGeoTiffCoordTransformsLut.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <rspf/base/rspfGeoTiffCoordTransformsLut.h>
#include <rspf/base/rspfString.h>

static const int TABLE_SIZE = 27;

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfGeoTiffCoordTransformsLut::rspfGeoTiffCoordTransformsLut()
   :
      rspfLookUpTable(TABLE_SIZE)
{
   //***
   // Complete initialization of data member "theTable".
   // Note:  Scalar types defined in constants.h file.
   //***
   theTable[0].theKey    = CT_TransverseMercator;
   theTable[0].theValue  = "rspfTransMercatorProjection";
   theTable[1].theKey    = CT_TransvMercator_Modified_Alaska;
   theTable[1].theValue  = "unknown";
   theTable[2].theKey    = CT_ObliqueMercator;
   theTable[2].theValue  = "unknown";
   theTable[3].theKey    = CT_ObliqueMercator_Laborde;
   theTable[3].theValue  = "unknown";
   theTable[4].theKey    = CT_ObliqueMercator_Rosenmund;
   theTable[4].theValue  = "unknown";   
   theTable[5].theKey    = CT_ObliqueMercator_Spherical;
   theTable[5].theValue  = "unknown";   
   theTable[6].theKey    = CT_Mercator;
   theTable[6].theValue  = "rspfMercatorProjection";
   theTable[7].theKey    = CT_LambertConfConic_2SP;
   theTable[7].theValue  = "rspfLambertConformalConicProjection";
   theTable[8].theKey    = CT_LambertConfConic_Helmert;
   theTable[8].theValue  = "unknown";
   theTable[9].theKey    = CT_LambertAzimEqualArea;
   theTable[9].theValue  = "unknown";
   theTable[10].theKey   = CT_AlbersEqualArea;
   theTable[10].theValue = "rspfAlbersProjection";
   theTable[11].theKey   = CT_AzimuthalEquidistant;
   theTable[11].theValue = "unknown";
   theTable[12].theKey   = CT_EquidistantConic;
   theTable[12].theValue = "unknown";
   theTable[13].theKey   = CT_Stereographic;
   theTable[13].theValue = "unknown";
   theTable[14].theKey   = CT_PolarStereographic;
   theTable[14].theValue = "rspfPolarStereoProjection";
   theTable[15].theKey   = CT_ObliqueStereographic;
   theTable[15].theValue = "rspfPolarStereoProjection";
   theTable[16].theKey   = CT_Equirectangular;
   theTable[16].theValue = "rspfEquDistCylProjection";
   theTable[17].theKey   = CT_CassiniSoldner;
   theTable[17].theValue = "rspfCassiniProjection";
   theTable[18].theKey   = CT_Gnomonic;
   theTable[18].theValue = "unknown";
   theTable[19].theKey   = CT_MillerCylindrical;
   theTable[19].theValue = "rspfMillerProjection";
   theTable[20].theKey   = CT_Orthographic;
   theTable[20].theValue = "rspfOrthoGraphicProjection";
   theTable[21].theKey   = CT_Polyconic;
   theTable[21].theValue = "rspfPolyconicrojection";
   theTable[22].theKey   = CT_Robinson;
   theTable[22].theValue = "unkknown";
   theTable[23].theKey   = CT_Sinusoidal;
   theTable[23].theValue = "rspfSinusoidalProjection";
   theTable[24].theKey   = CT_VanDerGrinten;
   theTable[24].theValue = "rspfVanDerGrintenProjection";
   theTable[25].theKey   = CT_NewZealandMapGrid;
   theTable[25].theValue = "unknown";
   theTable[26].theKey   = CT_TransvMercator_SouthOriented;
   theTable[26].theValue = "unknown";
}
