//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
// Description: Class implementation of rspfImageGeometry. See .h file for class documentation.
//
//**************************************************************************************************
// $Id$

#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspf2dTo2dTransformRegistry.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <cmath>

RTTI_DEF1(rspfImageGeometry, "rspfImageGeometry", rspfObject);

//**************************************************************************************************
// Default constructor defaults to unity transform with no projection  
//**************************************************************************************************
rspfImageGeometry::rspfImageGeometry()
:   
m_transform (0),
m_projection(0),
m_decimationFactors(0),
m_imageSize(),
m_targetRrds(0)
{
   m_imageSize.makeNan();
}

//**************************************************************************************************
// Copy constructor -- performs a deep copy. This is needed when an imageSource in the chain
// needs to modify the image geometry at that point in the chain. The image geometry to the left
// of that imageSource must remain unchanged. This constructor permits that imageSource to 
// maintain its own geometry based on the input geometry. All objects in the chain and to the right
// would see this alternate geometry. See rspfScaleFilter for an example of this case.
//**************************************************************************************************
rspfImageGeometry::rspfImageGeometry(const rspfImageGeometry& copy_this)
:
rspfObject(copy_this),
m_transform(copy_this.m_transform.valid()?(rspf2dTo2dTransform*)copy_this.m_transform->dup():((rspf2dTo2dTransform*)0)),
m_projection(copy_this.m_projection.valid()?(rspfProjection*)copy_this.m_projection->dup():(rspfProjection*)0),
m_decimationFactors(copy_this.m_decimationFactors),
m_imageSize(copy_this.m_imageSize),
m_targetRrds(copy_this.m_targetRrds)
{
}

//**************************************************************************************************
//! Constructs with projection and transform objects available for referencing. Either pointer
//! can be NULL -- the associated mapping would be identity.
//**************************************************************************************************
rspfImageGeometry::rspfImageGeometry(rspf2dTo2dTransform* transform, rspfProjection* proj)
:
rspfObject(),
m_transform(transform),
m_projection(proj),
m_decimationFactors(0),
m_imageSize(),
m_targetRrds(0)   
{
   m_imageSize.makeNan();
}

//**************************************************************************************************
// Destructor is hidden. Only accessible via rspfRefPtr centrally managed
//**************************************************************************************************
rspfImageGeometry::~rspfImageGeometry()
{
   // Nothing to do
}

void rspfImageGeometry::rnToRn(const rspfDpt& inRnPt, rspf_uint32 inResolutionLevel,
                                rspf_uint32 outResolutionLevel, rspfDpt& outRnPt) const
{
   if (inResolutionLevel != outResolutionLevel)
   {
      // Back out the decimation.
      rspfDpt pt;
      undecimatePoint(inRnPt, inResolutionLevel, pt);

      // Decimate to new level.
      decimatePoint(pt, outResolutionLevel, outRnPt);
   }
   else
   {
      outRnPt = inRnPt; // No transform required.
   }
}

void rspfImageGeometry::rnToFull(const rspfDpt& rnPt,
                                  rspf_uint32 resolutionLevel,
                                  rspfDpt& fullPt) const
{
   // Back out the decimation.
   rspfDpt localPt;
   undecimatePoint(rnPt, resolutionLevel, localPt);

   // Remove any shift/rotation.
   if ( m_transform.valid() && !localPt.hasNans() )
   {
      m_transform->forward(localPt, fullPt);
   }
   else
   {
      fullPt = localPt; // No transform (shift/rotation)
   }
}

void rspfImageGeometry::fullToRn(const rspfDpt& fullPt,
                                  rspf_uint32 resolutionLevel,
                                  rspfDpt& rnPt) const
{
   // Apply shift/rotation.
   rspfDpt localPt;
   if (m_transform.valid())
   {
      m_transform->inverse(fullPt, localPt);
   }
   else
   {
      localPt = fullPt; // No transform (shift/rotation)
   }

   // Apply the decimation.
   decimatePoint(localPt, resolutionLevel, rnPt);
}

void rspfImageGeometry::rnToWorld(const rspfDpt& rnPt,
                                   rspf_uint32 resolutionLevel,
                                   rspfGpt& wpt) const
{
   rspfDpt localPt;
   rnToRn(rnPt, resolutionLevel, m_targetRrds, localPt);
   localToWorld(localPt, wpt);
}

void rspfImageGeometry::worldToRn(const rspfGpt& wpt,
                                   rspf_uint32 resolutionLevel,
                                   rspfDpt& rnPt) const
{   
   rspfDpt localPt;
   worldToLocal(wpt, localPt);
   rnToRn(localPt, m_targetRrds, resolutionLevel, rnPt);
}

//**************************************************************************************************
//! Exposes the 3D projection from image to world coordinates. The caller should verify that
//! a valid projection exists before calling this method. Returns TRUE if a valid ground point
//! is available in the ground_pt argument. This method depends on the existence of elevation
//! information. If no DEM is available, the results will be incorrect or inaccurate.
//**************************************************************************************************
bool rspfImageGeometry::localToWorld(const rspfDpt& local_pt, rspfGpt& world_pt) const
{
   //! Return a NAN point of no projection is defined:
   if (!m_projection.valid())
   {
      world_pt.makeNan();
      return false;
   }

   // First transform local pixel to full-image pixel:
   rspfDpt full_image_pt;
   rnToFull(local_pt, m_targetRrds, full_image_pt);

   // Perform projection to world coordinates:
   m_projection->lineSampleToWorld(full_image_pt, world_pt);

    // Put longitude between -180 and +180 and latitude between -90 and +90 if not so. 
    world_pt.wrap();
    
   return true;
}

//**************************************************************************************************
//! Exposes the 3D projection from image to world coordinates given a constant height above 
//! ellipsoid. The caller should verify that a valid projection exists before calling this
//! method. Returns TRUE if a valid ground point is available in the ground_pt argument.
//**************************************************************************************************
bool rspfImageGeometry::localToWorld(const rspfDpt& local_pt, 
                                      const double& h_ellipsoid, 
                                      rspfGpt& world_pt) const
{
   //! Return a NAN point of no projection is defined:
   if (!m_projection.valid())
   {
      world_pt.makeNan();
      return false;
   }

   // First transform local pixel to full-image pixel:
   rspfDpt full_image_pt;
   rnToFull(local_pt, m_targetRrds, full_image_pt);

   // Perform projection to world coordinates:
   m_projection->lineSampleHeightToWorld(full_image_pt, h_ellipsoid, world_pt);

   // Put longitude between -180 and +180 and latitude between -90 and +90 if not so. 
   world_pt.wrap();
    
   return true;
}

//**************************************************************************************************
//! Exposes the 3D world-to-local image coordinate reverse projection. The caller should verify 
//! that a valid projection exists before calling this method. Returns TRUE if a valid image 
//! point is available in the local_pt argument.
//**************************************************************************************************
bool rspfImageGeometry::worldToLocal(const rspfGpt& world_pt, rspfDpt& local_pt) const
{
   bool result = true;
   
   if ( m_projection.valid() )
   {
      const rspfEquDistCylProjection* eqProj =
         dynamic_cast<const rspfEquDistCylProjection*>( m_projection.get() );
      
      rspfDpt full_image_pt;

      if ( eqProj && (m_imageSize.hasNans() == false) )
      {
         // Call specialized method to handle wrapping...
         eqProj->worldToLineSample( world_pt, m_imageSize, full_image_pt );
      }
      else if( isAffectedByElevation() )
      {
         rspfGpt copyPt( world_pt );
         if(world_pt.isHgtNan())
         {
            copyPt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(copyPt);
         }     

         // Perform projection from world coordinates to full-image space:
         m_projection->worldToLineSample(copyPt, full_image_pt);
      }
      else
      {
         // Perform projection from world coordinates to full-image space:
         m_projection->worldToLineSample(world_pt, full_image_pt);
      }
      
      // Transform to local space:
      fullToRn(full_image_pt, m_targetRrds, local_pt);
   }
   else // No projection set:
   {
      local_pt.makeNan();
      result = false;  
   }
   
   return result;
   
} // End: rspfImageGeometry::worldToLocal(const rspfGpt&, rspfDpt&)

//**************************************************************************************************
//! Sets the transform to be used for local-to-full-image coordinate transformation
//**************************************************************************************************
void rspfImageGeometry::setTransform(rspf2dTo2dTransform* transform) 
{ 
   m_transform = transform; 
}

//**************************************************************************************************
//! Sets the projection to be used for local-to-world coordinate transformation
//**************************************************************************************************
void rspfImageGeometry::setProjection(rspfProjection* projection) 
{ 
   m_projection = projection; 
}

//**************************************************************************************************
//! Returns TRUE if this geometry is sensitive to elevation
//**************************************************************************************************
bool rspfImageGeometry::isAffectedByElevation() const
{
    if (m_projection.valid())
        return m_projection->isAffectedByElevation();
    return false;
}

//*************************************************************************************************
//! Returns  GSD in meters in x- and y-directions. This may not be the same as the member 
//! projection's stated resolution since a transform may be involved.
//*************************************************************************************************
rspfDpt rspfImageGeometry::getMetersPerPixel() const
{
   rspfDpt gsd;
   getMetersPerPixel(gsd);
   return gsd;
}

//*************************************************************************************************
//! Computes GSD in meters in x- and y-directions. This may not be the same as the member 
//! projection's stated resolution since a transform may be involved.
//*************************************************************************************************
void rspfImageGeometry::getMetersPerPixel( rspfDpt& gsd ) const
{
   if (m_projection.valid() && !m_transform.valid())
   {
      // No transform present, so simply query the projection for GSD:
      gsd = m_projection->getMetersPerPixel();
   }
   else if (m_projection.valid() && (m_imageSize.hasNans() == false))
   {
      // A transform is involved, so need to use localToWorld call below:
      rspfDpt pL0 (m_imageSize/2);
      rspfDpt pLx (pL0.x+1, pL0.y);
      rspfDpt pLy (pL0.x, pL0.y+1);
      rspfGpt g0, gx, gy;

      localToWorld(pL0, g0);
      localToWorld(pLx, g0.height(), gx);
      localToWorld(pLy, g0.height(), gy);

      // Compute horizontal distance for one pixel:
      gsd.x = g0.distanceTo(gx);
      gsd.y = g0.distanceTo(gy);
   }
   else
   {
      // This object is not fully initialized:
      gsd.makeNan();
   }
}

//*************************************************************************************************
// Returns the resolution of this image in degrees/pixel. Note that this only
// makes sense if there is a projection associated with the image. Returns NaNs if no 
// projection defined.
//*************************************************************************************************
rspfDpt rspfImageGeometry::getDegreesPerPixel() const
{
   rspfDpt dpp;
   getDegreesPerPixel(dpp);
   return dpp;
}

//*************************************************************************************************
// Computes the resolution of this image in degrees/pixel. Note that this only
// makes sense if there is a projection associated with the image. Returns NaNs if no 
// projection defined.
//*************************************************************************************************
void rspfImageGeometry::getDegreesPerPixel( rspfDpt& dpp ) const
{
   const rspfMapProjection *map_proj = dynamic_cast<const rspfMapProjection *>(m_projection.get());
   if (map_proj && !m_transform.valid())
   {
      // No transform present, so simply query the projection for resolution:
      dpp = map_proj->getDecimalDegreesPerPixel();
   }
   else if (m_projection.valid() && (m_imageSize.hasNans() == false))
   {
      // A transform is involved, so need to use localToWorld call below:
      rspfDpt pL0 (m_imageSize/2);
      rspfDpt pLx (pL0.x+1, pL0.y);
      rspfDpt pLy (pL0.x, pL0.y+1);
      rspfGpt g0, gx, gy;

      localToWorld(pL0, g0);
      localToWorld(pLx, g0.height(), gx);
      localToWorld(pLy, g0.height(), gy);

      // Compute horizontal distance for one pixel:
      double dlatx = std::fabs(g0.lat - gx.lat);
      double dlaty = std::fabs(g0.lat - gy.lat);
      double dlonx = std::fabs(g0.lon - gx.lon);
      double dlony = std::fabs(g0.lon - gy.lon);
      dpp.lat = sqrt(dlatx*dlatx + dlaty*dlaty);
      dpp.lon = sqrt(dlonx*dlonx + dlony*dlony);
   }
   else
   {
      // This object is not fully initialized:
      dpp.makeNan();
   }
}

//**************************************************************************************************
//! Prints contents to output stream.
//**************************************************************************************************
std::ostream& rspfImageGeometry::print(std::ostream& out) const
{
   out << "type: rspfImageGeometry" << std::endl;
   if(m_transform.valid())
   {
      out << "  m_transform: ";
      m_transform->print(out);
   }
   else
   {
      out << "  No transform defined. Using identity transform.\n";
   }
   
   if(m_projection.valid())
   {
      out << "  m_projection: ";
      m_projection->print(out);
   }
   else
   {
      out << "  No projection defined. ";
   }

   for ( std::vector<rspfDpt>::size_type i = 0; i < m_decimationFactors.size(); ++i )
   {
      cout << "m_decimationFactors[" << i << "]: " << m_decimationFactors[i] << "\n";
   }

   out << "m_imageSize: " << m_imageSize
       << "\nm_targetRrds: " << m_targetRrds << "\n";

   return out;
}

//**************************************************************************************************
//! Returns TRUE if argument geometry has exactly the same instances for transform and 
//! m_projection. This may need to be expanded to permit different instances of same transforms
//**************************************************************************************************
bool rspfImageGeometry::operator==(const rspfImageGeometry& other) const
{
    return ((m_transform == other.m_transform) && (m_projection == other.m_projection) &&
            (decimationFactor(0) == other.decimationFactor(0)) );
}

//**************************************************************************************************
//! Returns the decimation factor from R0 for the resolution level specified. For r_index=0, the 
//! decimation factor is by definition 1.0. For the non-discrete case, r_index=1 returns a 
//! decimation of 0.5. If the vector of discrete decimation factors (m_decimationFactors) is 
//! empty, the factor will be computed as f=1/2^n
//**************************************************************************************************
rspfDpt rspfImageGeometry::decimationFactor(rspf_uint32 r_index) const
{
   rspf_uint32 size = (rspf_uint32)m_decimationFactors.size();
   if (size)
   {
      if (r_index < size)
      {
         return m_decimationFactors[r_index];
      }
      
      // Return the last defined decimation if the index requested exceeds list size:
      return m_decimationFactors[size-1];
   }

   // Compute the decimation factor:
   rspf_float64 factor = 1.0/(rspf_float64)(1 << r_index);
   
   return rspfDpt(factor, factor);
}

void rspfImageGeometry::decimationFactor(rspf_uint32 r_index,
                                          rspfDpt& result) const
{
   const rspf_uint32 SIZE = (rspf_uint32)m_decimationFactors.size();
   if (SIZE)
   {
      if (r_index < SIZE)
      {
         result = m_decimationFactors[r_index];
      }
      else
      {
         //---
         // Return the last defined decimation if the index requested
         // exceeds list size:
         //---
         result = m_decimationFactors[SIZE-1];
      }
   }
   else
   {
      // Compute the decimation factor:
      rspf_float64 factor = 1.0/(rspf_float64)(1 << r_index);
      result.x = factor;
      result.y = factor;
   }
}

void rspfImageGeometry::decimationFactors(
   std::vector<rspfDpt>& decimations) const
{
   decimations = m_decimationFactors;
}

//*****************************************************************************
//! Attempts to initialize a transform and a projection given the KWL.
//! Returns TRUE if no error encountered.
//*****************************************************************************
bool rspfImageGeometry::loadState(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   const char* lookup = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if (lookup)
   {
      if ( rspfString(lookup) == STATIC_TYPE_NAME(rspfImageGeometry) )
      {
         rspfObject::loadState(kwl, prefix);

         // m_transform
         rspfString transformPrefix = rspfString(prefix) + "transform.";
         rspfRefPtr<rspf2dTo2dTransform> transform = rspf2dTo2dTransformRegistry::instance()->
            createNativeObjectFromRegistry(kwl, transformPrefix.c_str());
         if( transform.valid() )
         {
            m_transform = transform;
         }

         // m_projection:
         rspfString projectionPrefix = rspfString(prefix) + "projection.";
         rspfRefPtr<rspfProjection> projection = rspfProjectionFactoryRegistry::instance()->
            createProjection(kwl, projectionPrefix.c_str());
         if( projection.valid() )
         {
            m_projection = projection;
         }

         // m_decimationFactors:
         rspfString decimations = kwl.find(prefix, "decimations");
         if( decimations.size() )
         {
            m_decimationFactors.clear();
            rspf::toVector(m_decimationFactors, decimations);
         }

         // m_imageSize:
         rspfString imageSize = kwl.find(prefix, "image_size");
         if( imageSize.size() )
         {
            m_imageSize.toPoint(imageSize);
         }

         // m_targetRrds:
         rspfString targetRrds = kwl.find(prefix, "target_rrds");
         if ( targetRrds.size() )
         {
            m_targetRrds = rspfString(targetRrds).toUInt32();
         }
      }
      else
      {
         // Now look for projection spec (for backwards compatibility):
         rspfProjection* projection = rspfProjectionFactoryRegistry::instance()->
            createProjection(kwl, prefix);
         if (projection)
         {
            setProjection(projection);
         }
      }
   }
   else
   {
      //---
      // Old geometry file with no type keyword:
      //---
      rspfProjection* projection = rspfProjectionFactoryRegistry::instance()->
         createProjection(kwl, prefix);
      if (projection)
      {
         setProjection(projection);
      } 
   }
   
   return true;
}

//**************************************************************************************************
//! Saves the transform (if any) and projection (if any) states to the KWL.
//**************************************************************************************************
bool rspfImageGeometry::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   bool good_save = true;

   // m_transform:
   if (m_transform.valid())
   {
      rspfString transformPrefix = rspfString(prefix) + "transform.";
      good_save = m_transform->saveState(kwl, transformPrefix.c_str());
   }

   // m_projection:
   if (m_projection.valid())
   {
      rspfString projectionPrefix = rspfString(prefix) + "projection.";
      good_save &= m_projection->saveState(kwl, projectionPrefix.c_str());
   }

   // m_gsd:
   rspfDpt gsd;
   getMetersPerPixel(gsd);
   kwl.add(prefix, "gsd", gsd.toString(), true);

   // m_decimationFactors:
   if(m_decimationFactors.size())
   {
      rspfString resultPoints;
      rspf::toStringList(resultPoints, m_decimationFactors);
      kwl.add(prefix, "decimations", resultPoints, true);
   }
   else
   {
      kwl.add(prefix, "decimations", "", true);
   }

   // m_imageSize:
   kwl.add(prefix, "image_size", m_imageSize.toString(), true);

   // m_targetRrds;
   kwl.add(prefix, "target_rrds", m_targetRrds, true);

   // Base class:
   good_save &= rspfObject::saveState(kwl, prefix);
   
   return good_save;
}

void rspfImageGeometry::setTargetRrds(rspf_uint32 rrds)
{
   m_targetRrds = rrds;
}

rspf_uint32 rspfImageGeometry::getTargetRrds() const
{
   return m_targetRrds;
}

//**************************************************************************************************
//! Replaces any existing transform and projection with those in the copy_this object
//**************************************************************************************************
const rspfImageGeometry& rspfImageGeometry::operator=(const rspfImageGeometry& copy_this)
{
   if (this != &copy_this)
   {
      // Get deep copy of 2D transform if one exists:
      if (copy_this.m_transform.valid())
      {
         m_transform = (rspf2dTo2dTransform*) copy_this.m_transform->dup();
      }
      
      // Now establish a deep copy of the projection, if any:
      if (copy_this.m_projection.valid())
      {
         m_projection = (rspfProjection*) copy_this.m_projection->dup();
      }
      
      // the Gsd should already be solved from the source we are copying from
      m_imageSize         = copy_this.m_imageSize;
      m_decimationFactors = copy_this.m_decimationFactors;
      m_targetRrds        = copy_this.m_targetRrds;
   }
   return *this;
}

//**************************************************************************************************
//! Returns the rspfGpts for the four image corner points
//**************************************************************************************************
bool rspfImageGeometry::getCornerGpts(rspfGpt& gul, rspfGpt& gur, 
                                       rspfGpt& glr, rspfGpt& gll) const
{
   rspfDpt iul (0,0);
   rspfDpt iur (m_imageSize.x-1, 0);
   rspfDpt ilr (m_imageSize.x-1, m_imageSize.y-1);
   rspfDpt ill (0, m_imageSize.y-1);

   bool status = true;

   status &= localToWorld(iul, gul);
   status &= localToWorld(iur, gur);
   status &= localToWorld(ilr, glr);
   status &= localToWorld(ill, gll);

   return status;
}

void rspfImageGeometry::getTiePoint(rspfGpt& tie, bool edge) const
{
   if ( m_projection.valid() && (m_imageSize.hasNans() == false) )
   {
      // Use the easting/northing version of this method if underlying projection is meters:
      const rspfMapProjection* map_proj = 
         dynamic_cast<const rspfMapProjection*>(m_projection.get());
      if (map_proj && !map_proj->isGeographic())
      {
         rspfDpt enTie;
         getTiePoint(enTie, edge);
         if (!enTie.hasNans())
            tie = m_projection->inverse(enTie);
         else
            tie.makeNan();
         return; // return here only because it bugs Dave
      }

      // Use projection to ground to establish UL extreme of image:
      rspfDpt iul (0,0);
      rspfDpt iur (m_imageSize.x-1, 0);
      rspfDpt ilr (m_imageSize.x-1, m_imageSize.y-1);
      rspfDpt ill (0, m_imageSize.y-1);
      rspfDpt iRight(1, 0);
      rspfDpt iDown(0, 1);
      
      rspfGpt gul;
      rspfGpt gur; 
      rspfGpt glr;
      rspfGpt gll;
      rspfGpt gRight;
      rspfGpt gDown;
      
      localToWorld(iul, gul);
      localToWorld(iur, gur);
      localToWorld(ilr, glr);
      localToWorld(ill, gll);
      localToWorld(iRight, gRight);
      localToWorld(iDown, gDown);
      
      // Determine the direction of the image:
      if ( gul.lat > gDown.lat ) // oriented north up
      {
         if ( gul.lat >= gRight.lat ) // straight up or leaning right
         {
            tie.lat = gul.lat;
            tie.lon = gll.lon;
         }
         else // leaning left
         {
            tie.lat = gur.lat;
            tie.lon = gul.lon;
         }
      }
      else // south or on side
      {
         if ( gRight.lat >= gul.lat ) // straight down or leaning right
         {
            tie.lat = glr.lat;
            tie.lon = gur.lon;
         }
         else // leaning left
         {
            tie.lat = gll.lat;
            tie.lon = glr.lon;
         }   
      }

      if ( edge )
      {
         rspfDpt pt = m_projection->forward( tie );
         rspfDpt half_pixel_shift = m_projection->getMetersPerPixel() * 0.5;
         pt.y += half_pixel_shift.y;
         pt.x -= half_pixel_shift.x;
         tie = m_projection->inverse( pt );
      }
      
   } // if ( (m_imageSize.hasNans() == false) && m_projection.valid() )
   else
   { 
      tie.lat = rspf::nan();
      tie.lon = rspf::nan();
   }
}

//**************************************************************************************************
// Assigns tie to the UL easting northing of the image. If edge is true, the E/N will be for the 
// pixel-is-area representation
//**************************************************************************************************
void rspfImageGeometry::getTiePoint(rspfDpt& tie, bool edge) const
{
   if (!m_projection.valid() || m_imageSize.hasNans())
   {
      tie.makeNan();
      return; // return here only because it bugs Dave
   }

   // Use the geographic version of this method if underlying projection is NOT map in meters:
   const rspfMapProjection* map_proj = dynamic_cast<const rspfMapProjection*>(m_projection.get());
   if (!map_proj || map_proj->isGeographic())
   {
      // Use the geographic version of this method to establish UL:
      rspfGpt gTie;
      gTie.hgt = 0.0;
      getTiePoint(gTie, edge);
      if (!gTie.hasNans())
         tie = m_projection->forward( gTie );
      else
         tie.makeNan();
      return; // return here only because it bugs Dave
   }

   // The underlying projection is a proper map projection in meters. Use easting northing 
   // directly to avoid shifting the UL tiepoint because of skewed edge in geographic. Note:
   // assume the image is North up:
   rspfDpt iul (0,0);
   rspfDpt ful;
   rnToFull(iul, 0, ful);
   map_proj->lineSampleToEastingNorthing(ful, tie);
   if (edge && !tie.hasNans())
   {
      // Shift from pixel-is-point to pixel-is-area
      rspfDpt half_pixel_shift = map_proj->getMetersPerPixel() * 0.5;
      tie.y += half_pixel_shift.y;
      tie.x -= half_pixel_shift.x;
   }
}

//**************************************************************************************************
void rspfImageGeometry::undecimatePoint(const rspfDpt& rnPt,
                                         rspf_uint32 resolutionLevel,
                                         rspfDpt& outPt) const
{
   // Back out the decimation.
   rspfDpt decimation = decimationFactor(resolutionLevel);
   
   //---
   // If no nans and one or both of the factors is not 1.0 decimation should
   // be applied.
   //---
   if ( (decimation.x != 1.0) || (decimation.y != 1.0) ) 
   {
      if ( !decimation.hasNans() && !rnPt.hasNans() )
      {
         outPt.x = rnPt.x * (1.0/decimation.x);
         outPt.y = rnPt.y * (1.0/decimation.y);
      }
      else
      {
         outPt.makeNan();
      }
   }
   else
   {
      outPt = rnPt; // No decimation.
   }
}

//**************************************************************************************************
void rspfImageGeometry::decimatePoint(const rspfDpt& inPt,
                                       rspf_uint32 resolutionLevel,
                                       rspfDpt& rnPt) const
{
   
   // Apply the decimation.
   rspfDpt decimation = decimationFactor(resolutionLevel);

   //---
   // If no nans and one or both of the factors is not 1.0 decimation should
   // be applied.
   //---
   if ( (decimation.x != 1.0) || (decimation.y != 1.0) ) 
   {
      if ( !decimation.hasNans() && !inPt.hasNans() )
      {
         rnPt.x = inPt.x * decimation.x;
         rnPt.y = inPt.y * decimation.y;
      }
      else
      {
        rnPt.makeNan(); 
      }
   }
   else
   {
      rnPt = inPt; // No decimation.
   }
}

bool rspfImageGeometry::isEqualTo(const rspfObject& obj, rspfCompareType compareType)const
{
   bool result = rspfObject::isEqualTo(obj, compareType);
   const rspfImageGeometry* rhs = dynamic_cast<const rspfImageGeometry*> (&obj);
   if(rhs&&result) // we know the types are the same
   {
      result = ((m_decimationFactors.size() == rhs->m_decimationFactors.size())&&
                m_imageSize.isEqualTo(rhs->m_imageSize)&& 
                (m_targetRrds == rhs->m_targetRrds)); 
      
      if(result)
      {
         rspf_uint32 decimationIdx = 0;
         for(decimationIdx = 0; result&&(decimationIdx < m_decimationFactors.size());++decimationIdx)
         {
            result = m_decimationFactors[decimationIdx].isEqualTo(rhs->m_decimationFactors[decimationIdx]);
         }
      }
      
      if(result && compareType==RSPF_COMPARE_IMMEDIATE)
      {
         result = ((m_transform.get()  == rhs->m_transform.get())&& 
                   (m_projection.get() == rhs->m_projection.get()));  //!< Maintains full_image-to-world_space transformation
         
      }
      else
      {
         if(m_transform.valid()&&rhs->m_transform.valid())
         {
            result = m_transform->isEqualTo(*rhs->m_transform.get());
         }
         else if(reinterpret_cast<rspf_uint64>(m_transform.get()) | reinterpret_cast<rspf_uint64>(rhs->m_transform.get())) // one is null
         {
            result = false;
         }
         if(m_projection.valid()&&rhs->m_projection.valid())
         {
            result = m_projection->isEqualTo(*rhs->m_projection.get());
         }
         else if(reinterpret_cast<rspf_uint64>(m_projection.get()) | reinterpret_cast<rspf_uint64>(rhs->m_projection.get())) // one is null
         {
            result = false;
         }
         
      }
   }
   return result;
}

void rspfImageGeometry::getBoundingRect(rspfIrect& bounding_rect) const
{
   if (m_imageSize.hasNans())
   {
      bounding_rect.makeNan();
   }
   else
   {
      bounding_rect.set_ulx(0);
      bounding_rect.set_uly(0);
      bounding_rect.set_lrx(m_imageSize.x-1);
      bounding_rect.set_lry(m_imageSize.y-1);
   }
}

void rspfImageGeometry::getBoundingRect(rspfDrect& bounding_rect) const
{
   if (m_imageSize.hasNans())
   {
      bounding_rect.makeNan();
   }
   else
   {
      bounding_rect.set_ulx(0);
      bounding_rect.set_uly(0);
      bounding_rect.set_lrx(m_imageSize.x-1);
      bounding_rect.set_lry(m_imageSize.y-1);
   }
}

void rspfImageGeometry::applyScale(const rspfDpt& scale, bool recenterTiePoint)
{
   if ((scale.x != 0.0) && (scale.y != 0.0))
   {
      rspfMapProjection* map_Proj = dynamic_cast<rspfMapProjection*>(m_projection.get());
      if ( map_Proj )
      {
         m_imageSize.x = rspf::round<rspf_int32>(m_imageSize.x / scale.x);
         m_imageSize.y = rspf::round<rspf_int32>(m_imageSize.y / scale.y);
         map_Proj->applyScale(scale, recenterTiePoint);
      }
   }
}

rspfAdjustableParameterInterface* rspfImageGeometry::getAdjustableParameterInterface()
{
   return PTR_CAST(rspfAdjustableParameterInterface, getProjection());
}

const rspfAdjustableParameterInterface* rspfImageGeometry::getAdjustableParameterInterface()const
{
   return dynamic_cast<const rspfAdjustableParameterInterface*>(getProjection());

}

bool rspfImageGeometry::computeImageToGroundPartialsWRTAdjParam(rspfDpt& result,
                                                                           const rspfGpt& gpt,
                                                                           rspf_uint32 idx,
                                                                           rspf_float64 paramDelta)
{
   double den = 0.5/paramDelta; // this is the same as dividing by 2*delta
   
   result = rspfDpt(0.0,0.0);
   rspfAdjustableParameterInterface* adjustableParamInterface = getAdjustableParameterInterface();
   
   if(!adjustableParamInterface) return false;
   if(idx >= adjustableParamInterface->getNumberOfAdjustableParameters()) return false;
   
   rspfDpt p1, p2;
   // double middle = adjustableParamInterface->getAdjustableParameter(idx);
   double middle = adjustableParamInterface->getParameterCenter(idx);
   
   //set parm to high value
   // adjustableParamInterface->setAdjustableParameter(idx, middle + paramDelta, true);
   adjustableParamInterface->setParameterCenter(idx, middle + paramDelta, true);
   worldToLocal(gpt, p1);
   
   //set parm to low value and gte difference
   // adjustableParamInterface->setAdjustableParameter(idx, middle - paramDelta, true);
   adjustableParamInterface->setParameterCenter(idx, middle - paramDelta, true);
   worldToLocal(gpt, p2);
   
   //get partial derivative
   result = (p2-p1)*den;
   
   //reset param
   // adjustableParamInterface->setAdjustableParameter(idx, middle, true);
   adjustableParamInterface->setParameterCenter(idx, middle, true);
   
   return !result.hasNans();
}
                         
bool rspfImageGeometry::computeImageToGroundPartialsWRTAdjParams(NEWMAT::Matrix& result, 
                                                                            const rspfGpt& gpt,
                                                                            rspf_float64 paramDelta)
{
   rspfAdjustableParameterInterface* adjustableParamInterface = getAdjustableParameterInterface();
   
   rspf_uint32 nAdjustables = adjustableParamInterface->getNumberOfAdjustableParameters();
   
   rspf_uint32 idx = 0;
   
   result = NEWMAT::Matrix(nAdjustables, 2);
   for(;idx < nAdjustables; ++idx)
   {
      rspfDpt paramResResult;
      computeImageToGroundPartialsWRTAdjParam(paramResResult,
                                                        gpt,
                                                        idx,
                                                        paramDelta);
      result[idx][0] = paramResResult.x;
      result[idx][1] = paramResResult.y;
   }
   
   return true;
}

bool rspfImageGeometry::computeImageToGroundPartialsWRTAdjParams(NEWMAT::Matrix& result,
                                                                            const rspfGpt& gpt,
                                                                            const DeltaParamList& deltas)
{
   
   rspfAdjustableParameterInterface* adjustableParamInterface = getAdjustableParameterInterface();
   
   rspf_uint32 nAdjustables = adjustableParamInterface->getNumberOfAdjustableParameters();
   
   if(nAdjustables != deltas.size()) return false;
   rspf_uint32 idx = 0;
   
   result = NEWMAT::Matrix(nAdjustables, 2);
   for(;idx < nAdjustables; ++idx)
   {
      rspfDpt paramResResult;
      computeImageToGroundPartialsWRTAdjParam(paramResResult,
                                                        gpt,
                                                        idx,
                                                        deltas[idx]);
      // ROWxCOL
      result[idx][0] = paramResResult.x;
      result[idx][1] = paramResResult.y;
   }
   
   return true;
}

bool rspfImageGeometry::computeGroundToImagePartials(NEWMAT::Matrix& result,
                                                      const rspfGpt& gpt,
                                                      const rspfDpt3d& deltaLlh)
{
   if(!getProjection()) return false;
   rspfDpt p1;
   rspfDpt p2;
   
   rspfDpt deltaWithRespectToLon;
   rspfDpt deltaWithRespectToLat;
   rspfDpt deltaWithRespectToH;
   rspf_float64 h = rspf::isnan(gpt.height())?0.0:gpt.height();

   // do the change in lon first for the dx, dy
   //
   worldToLocal(rspfGpt(gpt.latd(), gpt.lond()+deltaLlh.x, h, gpt.datum()), p1);
   worldToLocal(rspfGpt(gpt.latd(), gpt.lond()-deltaLlh.x, h, gpt.datum()), p2);
   
   double den = 0.5/deltaLlh.x; // this is the same as dividing by 2*delta
   deltaWithRespectToLon = (p2-p1)*den;
   
    
   // do the change in lat for the dx, dy
   //
   worldToLocal(rspfGpt(gpt.latd()+deltaLlh.y, gpt.lond(), h, gpt.datum()), p1);
   worldToLocal(rspfGpt(gpt.latd()-deltaLlh.y, gpt.lond(), h, gpt.datum()), p2);
   
   den = 0.5/deltaLlh.y; // this is the same as dividing by 2*delta
   deltaWithRespectToLat = (p2-p1)*den;

   
   // do the change in height first for the dx, dy
   //
   worldToLocal(rspfGpt(gpt.latd(), gpt.lond(), h+deltaLlh.z, gpt.datum()), p1);
   worldToLocal(rspfGpt(gpt.latd(), gpt.lond(), h-deltaLlh.z, gpt.datum()), p2);
   
   den = 0.5/deltaLlh.z; // this is the same as dividing by 2*delta
   deltaWithRespectToH = (p2-p1)*den;
   
   
   result = NEWMAT::Matrix(3,2);
   
   // set the matrix
   //
   result[1][0] = deltaWithRespectToLon.x*DEG_PER_RAD; 
   result[1][1] = deltaWithRespectToLon.y*DEG_PER_RAD; 
   result[0][0] = deltaWithRespectToLat.x*DEG_PER_RAD; 
   result[0][1] = deltaWithRespectToLat.y*DEG_PER_RAD; 
   result[2][0] = deltaWithRespectToH.x; 
   result[2][1] = deltaWithRespectToH.y; 
   
   
   return true; 
}

bool rspfImageGeometry::computeGroundToImagePartials(NEWMAT::Matrix& result,
                                                      const rspfGpt& gpt)
{
   rspfDpt mpp = getMetersPerPixel();
   rspfGpt originPoint;
   rspf_float64 len = mpp.length();
   
   if(len > FLT_EPSILON)
   {
      rspf_float64 delta = originPoint.metersPerDegree().length();
   
      delta = len/delta;
      
      return computeGroundToImagePartials(result, 
                                          gpt, 
                                          rspfDpt3d(delta,
                                                     delta,
                                                     len));
   }
   
   return false;
}

rspf_float64 rspfImageGeometry::upIsUpAngle() const
{
   rspf_float64 result = rspf::nan();

   if ( m_projection.valid() )
   {
      if ( m_projection->isAffectedByElevation() )
      {
         const int NUMBER_OF_SAMPLES = 9;
         
         // In meters.  This is about a height of a 6 to 7 story building.
         const double ELEVATION_DISPLACEMENT = 20;
         
         rspfDrect bounds;
         getBoundingRect( bounds );
         
         if( !bounds.hasNans() )
         {
            rspf_float64 widthPercent  = bounds.width()*.1;
            rspf_float64 heightPercent = bounds.height()*.1;
            
            //---
            // Sanity check to make sure that taking 10 percent out on the image
            // gets us to at least 1 pixel away.
            //---
            if(widthPercent < 1.0) widthPercent = 1.0;
            if(heightPercent < 1.0) heightPercent = 1.0;
            
            // set up some work variables to help calculate the average partial
            //
            std::vector<rspfDpt> ipts(NUMBER_OF_SAMPLES);
            std::vector<rspfGpt> gpts(NUMBER_OF_SAMPLES);
            std::vector<rspfDpt> iptsDisplacement(NUMBER_OF_SAMPLES);
            std::vector<rspfDpt> partials(NUMBER_OF_SAMPLES);
            rspfDpt averageDelta(0.0,0.0);
            
            rspfDpt centerIpt = bounds.midPoint();
            
            //---
            // Lets take an average displacement about the center point (3x3 grid)
            // we will go 10 percent out of the width and height of the image and
            // look at the displacement at those points and average them we will
            // use the average displacement to compute the up is up rotation.
            //---
            
            // top row:

            // 45 degree left quadrant
            ipts[0] = centerIpt + rspfDpt(widthPercent, -heightPercent);
            // 45 degree middle top 
            ipts[1] = centerIpt + rspfDpt(0.0,heightPercent);
            // 45 degree right quadrant
            ipts[2] = centerIpt + rspfDpt( widthPercent, -heightPercent);
            
            // middle row
            ipts[3] = centerIpt + rspfDpt(-widthPercent, 0.0); // left middle
            ipts[4] = centerIpt;
            ipts[5] = centerIpt + rspfDpt(widthPercent, 0.0);
            
            // bottom row
            ipts[6] = centerIpt + rspfDpt(-widthPercent, heightPercent);
            ipts[7] = centerIpt + rspfDpt(0.0, heightPercent);
            ipts[8] = centerIpt + rspfDpt(widthPercent, heightPercent);
            
            rspf_uint32 idx = 0;
            for(idx = 0; idx < ipts.size(); ++idx)
            {
               double h = 0.0;
               localToWorld(ipts[idx], gpts[idx]);
               h = gpts[idx].height();
               if(rspf::isnan(h)) h = 0.0;
               gpts[idx].height(h + ELEVATION_DISPLACEMENT);
               worldToLocal(gpts[idx], iptsDisplacement[idx]);
               averageDelta = averageDelta + (iptsDisplacement[idx] - ipts[idx]);
            }
            
            rspf_float64 averageLength = averageDelta.length();
            if(averageLength < 1) return false;
            
            if(!rspf::almostEqual((double)0.0, (double)averageLength))
            {
               averageDelta = averageDelta/averageLength;
            }
            
            rspfDpt averageLocation =  (rspfDpt(averageDelta.x*bounds.width(), 
                                                  averageDelta.y*bounds.height())+centerIpt) ;
            rspfGpt averageLocationGpt;
            rspfGpt centerGpt;
            
            localToWorld(averageLocation, averageLocationGpt);
            localToWorld(ipts[0], centerGpt);
            
            rspfDpt deltaPt = averageLocation - centerIpt; 
            rspfDpt deltaUnitPt = deltaPt;
            double len = deltaPt.length();
            if(len > FLT_EPSILON)
            {
               deltaUnitPt  = deltaUnitPt/len;
            }
            
            // Image space model positive y is down.  Let's reflect to positve y up.
            deltaUnitPt.y *= -1.0; // Reflect y to be right handed.
            
            result = rspf::atan2d(deltaUnitPt.x, deltaUnitPt.y);
            
            //---
            // We are essentially simulating camera rotation so negate the rotation
            // value.
            //---
            result *= -1;
            
            if(result < 0) result += 360.0;
            
         }  // Matches: if( bounds.hasNans() == false )
      }
      else
      {
         result = 0;
      }
      
   } // Matches: if ( m_projection.valid() && m_projection->isAffectedByElevation() )

   return result;
   
} // End: rspfImageGeometry::upIsUpAngle()

rspf_float64 rspfImageGeometry::northUpAngle()const
{
   rspf_float64 result = rspf::nan();

   if ( m_projection.valid() )
   {
      rspfDrect bounds;
      getBoundingRect( bounds );
      
      if( !bounds.hasNans() )
      {
         rspfDpt centerIpt = bounds.midPoint();

         rspfDpt midBottomIpt( centerIpt.x, bounds.lr().y );
         rspfDpt midTopIpt( centerIpt.x, bounds.ul().y );

         rspfGpt midBottomGpt;
         rspfGpt midTopGpt;
         
         localToWorld( midBottomIpt, midBottomGpt );
         localToWorld( midTopIpt, midTopGpt );

         if ( !midBottomGpt.hasNans() && !midTopGpt.hasNans() )
         {
            result = midBottomGpt.azimuthTo( midTopGpt );

            if(result < 0) result += 360.0;
         }
      }
      
   } // Matches: if ( m_projection.valid() )

   return result;
   
} // End: rspfImageGeometry::northUpAngle()
