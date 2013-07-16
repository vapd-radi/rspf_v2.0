//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description:  Height reference class.
//
//----------------------------------------------------------------------------

#include <rspf/elevation/rspfHgtRef.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/elevation/rspfElevationAccuracyInfo.h>


static rspfTrace traceDebug(rspfString("rspfHgtRef:debug"));

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfHgtRef.cpp 21399 2012-07-27 18:19:22Z gpotts $";
#endif



//*****************************************************************************
//  METHOD: rspfHgtRef::rspfHgtRef()
//  
//  Constructor for DEM reference.
//  
//*****************************************************************************
rspfHgtRef::rspfHgtRef(HeightRefType_t cRefType)
:
theCurrentHeightRefType(cRefType)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHgtRef::rspfHgtRef DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   
}


//*****************************************************************************
//  METHOD: rspfHgtRef::rspfHgtRef()
//  
//  Constructor for ellipsoid reference.
//  
//*****************************************************************************
rspfHgtRef::rspfHgtRef(HeightRefType_t cRefType, const rspf_float64& atHgt)
:
theCurrentHeightRefType(cRefType),
theCurrentRefHeight(atHgt)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHgtRef::rspfHgtRef DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   
}



rspfHgtRef::~rspfHgtRef()
{
}


//*****************************************************************************
//  METHOD: rspfHgtRef::getRefHeight()
//  
//  Get current reference height based on state of theCurrentHeightRefType.
//  
//*****************************************************************************
rspf_float64 rspfHgtRef::getRefHeight(const rspfGpt& pg) const
{
   rspf_float64 refHgt;
   
   switch (theCurrentHeightRefType)
   {
      case AT_HGT:
         refHgt = theCurrentRefHeight;
         break;
      case AT_DEM:
         refHgt = rspfElevManager::instance()->getHeightAboveEllipsoid(pg);
         break;
      default:
         refHgt = 0.0;
         break;
   }
         
   return refHgt;
}

//*****************************************************************************
//  METHOD: rspfHgtRef::getSurfaceInfo()
//  
//  Get reference surface information.
//  
//*****************************************************************************

#if 0
bool rspfHgtRef::getSurfaceInfo(const rspfGpt& pg,
                                       rspfElevationAccuracyInfo& info) const
{
   bool infoOK = false;

  // rspfElevManager::ElevationDatabaseListType dbList;  
  // std::vector<rspfFilename> cellList;


   //double hgt =  rspfElevManager::instance()->getHeightAboveEllipsoid(pg);


/*

   dbList = rspfElevManager::instance()->getElevationDatabaseList();
   rspf_uint32 idx;

   for(idx=0; idx < dbList.size(); ++idx)
   {
      double h = dbList[idx]->getHeightAboveEllipsoid(pg);
      if (!rspf::isnan(h))
      {
         hgt = h;
         break;
      }
   }
  */ 
   /*
   if (!rspf::isnan(hgt))
   {
      rspfElevManager::instance()->getOpenCellList(cellList);

      rspfRefPtr<rspfImageHandler> ih =
            rspfImageHandlerRegistry::instance()->open(cellList[idx]);
      rspfString hanType(ih->getClassName().c_str());

      rspfImageGeometry* geom = ih->getImageGeometry().get();
      gsd = geom->getMetersPerPixel();

      info = hanType;
   }
   else
   {
      info = "No Surface";
   }
   */
   infoOK = true;

   return infoOK;
}
#endif

//*****************************************************************************
//  METHOD: rspfHgtRef::getSurfaceCovMatrix()
//  
//  Get reference surface covariance matrix.
//  
//*****************************************************************************
bool rspfHgtRef::getSurfaceCovMatrix(const rspfGpt& pg, NEWMAT::Matrix& cov) const
{
   
   rspf_float64 refCE;
   rspf_float64 refLE;
   rspfString info;
   rspfDpt gsd;

   switch (theCurrentHeightRefType)
   {
      case AT_HGT:
         // Set the reference CE/LE
         //   Note: currently set to reflect no contribution
         refCE = 0.0;
         refLE = .01;
         
         break;
         
      case AT_DEM:
      {

         // Set the reference CE/LE
         //   Note: currently set to SRTM spec in METERS
         //    refCE = 20.0;
         //    refLE = 16.0;
         //        (ref: www2.jpl.nas.gov/srtm/statistics.html)
         //  "refCE = rspfElevManager::instance()->getAccuracyCE90(pg)" is
         //    the desirable operation here (if it is implemented)
         // ================================================
         //  This is one step closer to automatic
         //  access to elevation surface accuracy
         //   TODO...
         //     [1] load from RSPF_PREFERENCES?
         //     [2] does DTED header/metadata have info?
         // ================================================
         rspfElevationAccuracyInfo info;
         rspfElevManager::instance()->getAccuracyInfo(info, pg);

         if(info.hasValidAbsoluteError())
         {
            refCE = info.m_absoluteCE;
            refLE = info.m_absoluteLE;
         }
         else
         {
            refCE = 20.0;
            refLE = 16.0;
         }
#if 0
         if (getSurfaceInfo(pg, info, gsd))
         {
            if (info.contains("Srtm"))
            {
               if (gsd.x < 50.0)
               {
                  // SRTM 1 arc
                  refCE = 20.0;
                  refLE = 10.0;
               }
               else
               {
                  // SRTM 3 arc
                  refCE = 20.0;
                  refLE = 16.0;
               }
            }
            else if (info.contains("Dted"))
            {
               if (gsd.x < 50.0)
               {
                  // DTED level 2
                  refCE = 40.0;
                  refLE = 20.0;
               }
               else
               {
                  // DTED level 1
                  refCE = 50.0;
                  refLE = 30.0;
               }
            }
            else
            {
               // Other
               refCE = 20.0;
               refLE = 16.0;
            }
         }
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               //<< "DEBUG: info: " << info
               << " ref: " << refCE << "/" << refLE << endl;
         }
#endif

         break;
      }
      default:
         return false;
         break;
   }
   
   // Form the covariance matrix
   //  A circular error ellipse with no correlation must be assumed.
   cov = 0.0;
   cov(1,1) = refCE/2.146;
   cov(2,2) = refCE/2.146;
   cov(3,3) = refLE/1.6449;
   cov(1,1) *= cov(1,1);
   cov(2,2) *= cov(2,2);
   cov(3,3) *= cov(3,3);
   
   return true;
}


//*****************************************************************************
///  METHOD: rspfHgtRef::getSurfaceCovMatrix()
//  
//  Get reference surface covariance matrix.
// 
//*****************************************************************************
bool rspfHgtRef::getSurfaceCovMatrix(const rspf_float64   refCE, 
                                      const rspf_float64   refLE,
                                      NEWMAT::Matrix& cov) const
{
   rspf_float64 useCE;
   rspf_float64 useLE;
   
   if (refCE<0.0 || refLE<0.0)
   {
      return false;
   }
   
   switch (theCurrentHeightRefType)
   {
      case AT_HGT:
         // Set the reference CE/LE
         //   Note: currently set to reflect no contribution
         useCE = 0.0;
         useLE = .01;
         
         break;
         
      case AT_DEM:
         // Set the reference CE/LE
         useCE = refCE;
         useLE = refLE;
         
         break;
         
      default:
         return false;
         break;
   }
   
   // Form the covariance matrix
   //  A circular error ellipse with no correlation must be assumed.
   cov = 0.0;
   cov(1,1) = useCE/2.146;
   cov(2,2) = useCE/2.146;
   cov(3,3) = useLE/1.6449;
   cov(1,1) *= cov(1,1);
   cov(2,2) *= cov(2,2);
   cov(3,3) *= cov(3,3);
   
   return true;
}


//*****************************************************************************
//  METHOD: rspfHgtRef::getSurfaceNormalCovMatrix()
//  
//  Form surface normal ECF covariance matrix from ENU surface covariance.
//  
//*****************************************************************************
bool rspfHgtRef::
getSurfaceNormalCovMatrix
   (const rspfGpt&       pg, 
    const NEWMAT::Matrix& surfCov, 
          NEWMAT::Matrix& normCov) const
{
   // Set the local frame
   rspfLsrSpace enu(pg);

   // Propagate the reference covariance matrix to ECF
   NEWMAT::Matrix covEcf;
   covEcf = enu.lsrToEcefRotMatrix() * surfCov * enu.lsrToEcefRotMatrix().t();

   // Get surface normal vector in ENU
   rspfColumnVector3d sn = getLocalTerrainNormal(pg);
   NEWMAT::Matrix tnU(3,1);
   tnU << sn[0] << sn[1] << sn[2];

   // Rotate surface normal to ECF
   NEWMAT::Matrix tnUecf(3,1);
   tnUecf = enu.lsrToEcefRotMatrix() * tnU;

   // Propagate to terrain normal
   NEWMAT::Matrix ptn;
   NEWMAT::SymmetricMatrix ptns;
   ptn = tnUecf.t() * covEcf * tnUecf;
   ptns << ptn;

   // And back to ECF
   normCov = tnUecf * ptns * tnUecf.t();


   return true;
}


//*****************************************************************************
//  METHOD: rspfHgtRef::getLocalTerrainNormal()
//  
//  Get get local terrain normal unit vector.
//  
//*****************************************************************************
rspfColumnVector3d rspfHgtRef::getLocalTerrainNormal(const rspfGpt& pg) const
{
   rspfColumnVector3d tNorm;
   const rspf_float64 delta = 100.0;
   
   switch (theCurrentHeightRefType)
   {
      case AT_HGT:
         // Use ellipsoid tangent plane mormal
         tNorm = tNorm.zAligned();
         break;
         
      case AT_DEM:
         {
            // Use local 3X3 grid around point to get mean slope
            NEWMAT::Matrix h(3,3);
            rspfDpt mpd;
            mpd = pg.metersPerDegree();
            rspf_float64 dLon = delta/mpd.x;
            rspf_float64 dLat = delta/mpd.y;


            for (rspf_int32 lon=-1; lon<=1; ++lon)
            {
               rspf_float64 clon = pg.lond()+lon*dLon;
               for (rspf_int32 lat=-1; lat<=1; ++lat)
               {
                  rspf_float64 clat = pg.latd()+lat*dLat;
                  rspfGpt p(clat, clon, pg.height());
                  h(2-lat,lon+2) =
                     rspfElevManager::instance()->getHeightAboveEllipsoid(p);
               }
            }
            
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  <<"DEBUG: getLocalTerrainNormal...  3X3 grid"<<endl;
               for (rspf_int32 lat=-1; lat<=1; ++lat)
               {
                  for (rspf_int32 lon=-1; lon<=1; ++lon)
                    rspfNotify(rspfNotifyLevel_DEBUG)<<"  "<<h(lat+2,lon+2);
                  rspfNotify(rspfNotifyLevel_DEBUG)<<endl;
               }
            }

            rspf_float64 dz_dlon =
                  ((h(1,3)+2*h(2,3)+h(3,3))-(h(1,1)+2*h(2,1)+h(3,1)))/(8*delta);
            rspf_float64 dz_dlat =
                  ((h(1,1)+2*h(1,2)+h(1,3))-(h(3,1)+2*h(3,2)+h(3,3)))/(8*delta);
            tNorm[0] = -dz_dlon;
            tNorm[1] = -dz_dlat;
            tNorm[2] = 1.0 - sqrt(dz_dlon*dz_dlon+dz_dlat*dz_dlat);
         }
         
         // If error condition, return z-aligned vector to allow continuation
         if (rspf::isnan(tNorm[0]) ||
             rspf::isnan(tNorm[1]) ||
             rspf::isnan(tNorm[2]))
         {
            tNorm = tNorm.zAligned();
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "WARNING: rspfHgtRef::getLocalTerrainNormal(): "
                  << "\n   error... terrain normal set to vertical..."
                  << std::endl;
               
            }
         }
         break;
         
      default:
         tNorm = tNorm.zAligned();
         break;
   }

   return tNorm;
}
