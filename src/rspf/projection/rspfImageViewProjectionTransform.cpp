//*****************************************************************************
// FILE: rspfImageViewProjectionTransform.cc
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Garrett Potts
//         Oscar Kramer (oscar@krameranalytic.com)
//
// DESCRIPTION: Contains declaration of rspfImageViewProjectionTransform.
//    This class provides an image to view transform that utilizes two
//    independent 2D-to-3D projections. Intended for transforming view to
//    geographic "world" space to input image space.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfImageViewProjectionTransform.cpp 20489 2012-01-23 20:07:56Z dburken $
//
#include <rspf/projection/rspfImageViewProjectionTransform.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <cmath>

RTTI_DEF1(rspfImageViewProjectionTransform,
          "rspfImageViewProjectionTransform",
          rspfImageViewTransform);

//*****************************************************************************
//  CONSTRUCTOR: rspfImageViewProjectionTransform
//*****************************************************************************
rspfImageViewProjectionTransform::rspfImageViewProjectionTransform
(  rspfImageGeometry* imageGeometry, rspfImageGeometry* viewGeometry)
:  m_imageGeometry(imageGeometry),
   m_viewGeometry(viewGeometry)
{
}

//*****************************************************************************
//  CONSTRUCTOR: rspfImageViewProjectionTransform
//*****************************************************************************
rspfImageViewProjectionTransform::
rspfImageViewProjectionTransform(const rspfImageViewProjectionTransform& src)
: rspfImageViewTransform(src),
  m_imageGeometry(src.m_imageGeometry),
  m_viewGeometry(src.m_viewGeometry)
{
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfImageViewProjectionTransform
//*****************************************************************************
rspfImageViewProjectionTransform::~rspfImageViewProjectionTransform()
{
}

//*****************************************************************************
//  Workhorse of the object. Converts image-space to view-space.
//*****************************************************************************
void rspfImageViewProjectionTransform::imageToView(const rspfDpt& ip, rspfDpt& vp) const
{
   // Check for same geometries on input and output (this includes NULL geoms):
   if (m_imageGeometry == m_viewGeometry)
   {
      vp = ip;
      return;
   }
   
   // Otherwise we need access to good geoms. Check for a bad geometry object:
   if (!m_imageGeometry || !m_viewGeometry)
   {
      vp.makeNan();
      return;
   }
   
   // Check for same projection on input and output sides to save projection to ground:
   if (m_imageGeometry->getProjection() == m_viewGeometry->getProjection())
   {
      // Check for possible same 2D transforms as well:
      if ( (m_imageGeometry->getTransform() == m_viewGeometry->getTransform()) &&
           (m_imageGeometry->decimationFactor(0) == m_viewGeometry->decimationFactor(0)) )
      {
         vp = ip;
         return;
      }
      
      // Not the same 2D transform, so just perform local-image -> full-image -> local-view:
      rspfDpt fp;
      m_imageGeometry->rnToFull(ip, 0, fp);
      m_viewGeometry->fullToRn(fp, 0, vp);
      return;
   }
   
   //---
   // Completely different left and right side geoms (typical situation).
   // Need to project to ground.
   //---
   rspfGpt gp;
   m_imageGeometry->localToWorld(ip, gp);
   m_viewGeometry->worldToLocal(gp, vp);

#if 0 /* Please leave for debug. */
   cout <<"DEBUG rspfImageViewProjectionTransform::imageToView:"
        <<"\n    ip: "<<ip
        <<"\n    gp: "<<gp
        <<"\n    vp: "<<vp<<std::endl;
#endif
}

//*****************************************************************************
//  Other workhorse of the object. Converts view-space to image-space.
//*****************************************************************************
void rspfImageViewProjectionTransform::viewToImage(const rspfDpt& vp, rspfDpt& ip) const
{
   // Check for same geometries on input and output (this includes NULL geoms):
   if (m_imageGeometry == m_viewGeometry)
   {
      ip = vp;
      return;
   }
   
   // Otherwise we need access to good geoms. Check for a bad geometry object:
   if (!m_imageGeometry || !m_viewGeometry)
   {
      ip.makeNan();
      return;
   }
   
   // Check for same projection on input and output sides to save projection to ground:
   const rspfProjection* iproj = m_imageGeometry->getProjection();
   const rspfProjection* vproj = m_viewGeometry->getProjection();
   if ((iproj && vproj && iproj->isEqualTo(*vproj)) || (iproj == vproj))
   {
      // Check for possible same 2D transforms as well:
      const rspf2dTo2dTransform* ixform = m_imageGeometry->getTransform();
      const rspf2dTo2dTransform* vxform = m_viewGeometry->getTransform();
      if (((ixform && vxform && ixform->isEqualTo(*vxform)) || (ixform == vxform)) &&
          (m_imageGeometry->decimationFactor(0) == m_viewGeometry->decimationFactor(0)))
      {
         ip = vp;
         return;
      }
      
      // Not the same 2D transform, so just perform local-image -> full-image -> local-view:
      rspfDpt fp;
      m_viewGeometry->rnToFull(vp, 0, fp);
      m_imageGeometry->fullToRn(fp, 0, ip);
      return;
   }
   
   //---
   // Completely different left and right side geoms (typical situation).
   // Need to project to ground.
   //---
   rspfGpt gp;
   m_viewGeometry->localToWorld(vp, gp);
   m_imageGeometry->worldToLocal(gp, ip);

#if 0 /* Please leave for debug. */
   cout <<"DEBUG rspfImageViewProjectionTransform::viewToImage:"
        <<"\n    vp: "<<vp
        <<"\n    gp: "<<gp
        <<"\n    ip: "<<ip
        <<std::endl;
#endif
}

//*****************************************************************************
//! OLK: Not sure where this is used, but needed to satisfy rspfViewInterface base class.
//*****************************************************************************
bool rspfImageViewProjectionTransform::setView(rspfObject* baseObject)
{
   rspfImageGeometry* g = dynamic_cast<rspfImageGeometry*>(baseObject);
   bool new_view_set = false;
   if (g)
   {
      m_viewGeometry = g;
      new_view_set = true;
   }
   else
   {
      rspfProjection* proj = dynamic_cast<rspfProjection*>(baseObject);
      if(proj)
      {
         if(m_viewGeometry.valid())
         {
            m_viewGeometry->setProjection(proj);
         }
         else
         {
            m_viewGeometry = new rspfImageGeometry(0, proj);
         }

         new_view_set = true;
      }
   }

   if ( m_viewGeometry.valid() && m_viewGeometry->getImageSize().hasNans() )
   {
      // Sets the view image size from the image geometry if present.
      initializeViewSize();
   } 

   return new_view_set;
}

//*****************************************************************************
//  Dumps contents to stream
//*****************************************************************************
std::ostream& rspfImageViewProjectionTransform::print(std::ostream& out)const
{
    out << "rspfImageViewProjectionTransform::print: ..... entered " <<endl;

    if(m_imageGeometry.valid())
    {
        out << "  Input Image (LEFT) Geometry: " << endl;
        m_imageGeometry->print(out);
    }
    else
    {
        out << "  None defined." << endl;
    }
    if(m_viewGeometry.valid())
    {
        out << "Output View (RIGHT) Geometry: " << endl;
        m_viewGeometry->print(out);
    }
    else
    {
        out << "  None defined." << endl;
    }
    return out;
}

//**************************************************************************************************
// Converts the local image space rect into bounding view-space rect
//**************************************************************************************************
rspfDrect rspfImageViewProjectionTransform::getImageToViewBounds(const rspfDrect& imageRect)const
{
    // Let base class try:
   rspfDrect result = rspfImageViewTransform::getImageToViewBounds(imageRect);

   // If not successful, compute using input and output geometries:
   if (result.hasNans() && m_imageGeometry.valid() && m_viewGeometry.valid() &&
       m_imageGeometry->hasProjection() && m_viewGeometry->hasProjection())
   {
      rspfGeoPolygon viewClip;
      m_viewGeometry->getProjection()->getGroundClipPoints(viewClip);
      if(viewClip.size())
      {
         std::vector<rspfGpt> imageGpts(4);
         m_imageGeometry->localToWorld(imageRect.ul(), imageGpts[0]);
         m_imageGeometry->localToWorld(imageRect.ur(), imageGpts[1]);
         m_imageGeometry->localToWorld(imageRect.lr(), imageGpts[2]);
         m_imageGeometry->localToWorld(imageRect.ll(), imageGpts[3]);

         const rspfDatum* viewDatum = m_viewGeometry->getProjection()->origin().datum();
         imageGpts[0].changeDatum(viewDatum);
         imageGpts[1].changeDatum(viewDatum);
         imageGpts[2].changeDatum(viewDatum);
         imageGpts[3].changeDatum(viewDatum);
         
         rspfPolyArea2d viewPolyArea(viewClip.getVertexList());
         rspfPolyArea2d imagePolyArea(imageGpts);
         viewPolyArea &= imagePolyArea;
         std::vector<rspfPolygon> visiblePolygons;
         viewPolyArea.getVisiblePolygons(visiblePolygons);
         if(visiblePolygons.size())
         {
            std::vector<rspfDpt> vpts;
            rspf_uint32 idx = 0;
            for(idx=0; idx<visiblePolygons[0].getNumberOfVertices();++idx)
            {
               rspfDpt tempPt;
               rspfGpt gpt(visiblePolygons[0][idx].lat, visiblePolygons[0][idx].lon, 0.0,  viewDatum);
               m_viewGeometry->worldToLocal(gpt, tempPt);
               vpts.push_back(tempPt);
            }
            result = rspfDrect(vpts);
         }
      }
   }
   return result;
}

//*****************************************************************************
//  METHOD: rspfImageViewProjectionTransform::loadState
//*****************************************************************************
bool rspfImageViewProjectionTransform::loadState(const rspfKeywordlist& kwl,
                                                  const char* prefix)
{
   bool result = rspfImageViewTransform::loadState(kwl, prefix);
   if(result)
   {
      rspfString imagePrefix = rspfString(prefix)+"image_geometry.";
      rspfString viewPrefix  = rspfString(prefix)+"view_geometry.";
      if(kwl.numberOf(imagePrefix.c_str())>0)
      {
         m_imageGeometry = new rspfImageGeometry();
         m_imageGeometry->loadState(kwl, imagePrefix.c_str());
      }
      if(kwl.numberOf(viewPrefix.c_str())>0)
      {
         m_viewGeometry = new rspfImageGeometry();
         m_viewGeometry->loadState(kwl, viewPrefix.c_str());
      }
      
   }
   
   return result;
}

//**************************************************************************************************
// 
//**************************************************************************************************
bool rspfImageViewProjectionTransform::saveState(rspfKeywordlist& kwl,
                                                  const char* prefix)const
{
   rspfString imagePrefix = rspfString(prefix)+"image_geometry.";
   rspfString viewPrefix  = rspfString(prefix)+"view_geometry.";
   
   if(m_imageGeometry.valid())
   {
      m_imageGeometry->saveState(kwl, imagePrefix.c_str());
   }
   if(m_viewGeometry.valid())
   {
      m_viewGeometry->saveState(kwl, viewPrefix.c_str());
   }
   return rspfImageViewTransform::saveState(kwl, prefix);
}

//**************************************************************************************************
// Returns the GSD of input image.
//**************************************************************************************************
rspfDpt rspfImageViewProjectionTransform::getInputMetersPerPixel() const
{
    rspfDpt result;

    if (m_imageGeometry.valid() && m_imageGeometry->hasProjection())
        result = m_imageGeometry->getProjection()->getMetersPerPixel();
    else
        result.makeNan();

    return result;
}

//**************************************************************************************************
// Returns the GSD of the output view.
//**************************************************************************************************
rspfDpt rspfImageViewProjectionTransform::getOutputMetersPerPixel() const
{
    rspfDpt result;

    if(m_viewGeometry.valid() && m_viewGeometry->hasProjection())
        result = m_viewGeometry->getProjection()->getMetersPerPixel();
    else
        result.makeNan();

    return result;
}

// Initialize view geometry image size from image geometry.
bool rspfImageViewProjectionTransform::initializeViewSize()
{
   bool result = false;

   if ( m_imageGeometry.valid() )
   {
      rspfIrect imageRect( 0, 0,
                            m_imageGeometry->getImageSize().x - 1,
                            m_imageGeometry->getImageSize().y - 1);
      
      if ( (imageRect.hasNans() == false) && m_viewGeometry.valid() )
      {
         rspfRefPtr<rspfProjection> viewProj = m_viewGeometry->getProjection();
         if ( viewProj.valid() )
         {
            rspfIpt size;
            size.makeNan();
            
            const rspfEquDistCylProjection* eqProj =
               dynamic_cast<const rspfEquDistCylProjection*>( viewProj.get() );
            if ( eqProj )
            {
               // Specialized to take into consideration a date line crossing.
               
               // Get the ground points we need:
               rspfDpt iRight(imageRect.ul().x+1, imageRect.ul().y);
               rspfDpt iDown(imageRect.ul().x, imageRect.ul().y+1);
               rspfGpt gul;
               rspfGpt gur; 
               rspfGpt glr;
               rspfGpt gll;
               rspfGpt gRight;
               rspfGpt gDown;
               rspf_float64 hdd; // height decimal degrees
               rspf_float64 wdd; // width decimal degrees
               rspf_float64 leftLon;
               rspf_float64 rightLon;
               
               m_imageGeometry->localToWorld( imageRect.ul(), gul );
               m_imageGeometry->localToWorld( imageRect.ur(), gur );
               m_imageGeometry->localToWorld( imageRect.lr(), glr );
               m_imageGeometry->localToWorld( imageRect.ll(), gll );
               m_imageGeometry->localToWorld( iRight, gRight );
               m_imageGeometry->localToWorld( iDown, gDown );
               
               //---
               // Determine the direction of the image and get the geographic bounding box.
               // For the longitude consider a date line crossing.
               //---
               if ( gul.lat > gDown.lat ) // oriented north up
               {
                  if ( gul.lat >= gRight.lat ) // straight up or leaning right
                  {
                     hdd      = gul.lat - glr.lat;
                     leftLon  = gll.lon;
                     rightLon = gur.lon;
                  }
                  else // leaning left
                  {
                     hdd      = gur.lat - gll.lat;
                     leftLon  = gul.lon;
                     rightLon = glr.lon;
                  }
               }
               else // south or on side
               {
                  if ( gRight.lat >= gul.lat ) // straight down or leaning right
                  {
                     hdd      = glr.lat - gul.lat;
                     leftLon  = gur.lon;
                     rightLon = gll.lon;
                  }
                  else // leaning left
                  {
                     hdd      = gll.lat - gur.lat;
                     leftLon  = glr.lon;
                     rightLon = gul.lon;
                  }
               }
               
               if ( rightLon > leftLon )
               {
                  wdd = rightLon - leftLon;
               }
               else // Date line cross:
               {
                  wdd = rightLon + 360.0 - leftLon;
               }

               //---
               // Add one pixel width/height to the point to point size to make it edge to edge
               // before dividing by gsd to get the size.  The view projection gsd is used here
               // since the view could be a different resolution than the input projection.
               //---
               size.x = rspf::round<rspf_int32>(
                  ( wdd + eqProj->getDecimalDegreesPerPixel().x ) /
                  eqProj->getDecimalDegreesPerPixel().x );
               size.y = rspf::round<rspf_int32>(
                  ( hdd + eqProj->getDecimalDegreesPerPixel().y ) /
                  eqProj->getDecimalDegreesPerPixel().y );
               
#if 0 /* Please leave for debug: */
               cout << "\nwdd: " << wdd
                    << "\nhdd: " << hdd
                    << "\ngul: " << gul
                    << "\ngur: " << gur
                    << "\nglr: " << glr
                    << "\ngll: " << gll
                    << endl;
#endif
            }
            else // Not an rspfEquDistCylProjection:
            {
               rspfDrect rect = getImageToViewBounds( imageRect );
               size.x = rspf::round<rspf_int32>( rect.width()  );
               size.y = rspf::round<rspf_int32>( rect.height() );
            }

#if 0 /* Please leave for debug: */
            cout << "m_imageGeometry:\n" << *(m_imageGeometry.get())
                 << "\n\nm_viewGeometry:\n" << *(m_imageGeometry.get())
                 << "\n\ncomputed view size: " << size << endl;
#endif
            
            if ( size.hasNans() == false )
            {
               m_viewGeometry->setImageSize( size );
               result = true;
            }
            
         } // Matches: if ( viewProj.valid() )

      } // Matches: if ( (imageRect.hasNans() == false) ... 
      
   } // Matches: if ( m_imageGeometry.valid() && ...

   return result;
   
} // End:  bool rspfImageViewProjectionTransform::initializeViewSize()
