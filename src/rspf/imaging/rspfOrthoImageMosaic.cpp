//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfOrthoImageMosaic.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfOrthoImageMosaic.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>

static rspfTrace traceDebug ("rspfOrthoImageMosaic:debug");

RTTI_DEF1(rspfOrthoImageMosaic, "rspfOrthoImageMosaic", rspfImageMosaic);

//**************************************************************************************************
// 
//**************************************************************************************************
rspfOrthoImageMosaic::rspfOrthoImageMosaic()
   :rspfImageMosaic()
{
   m_Delta.makeNan();
   m_UpperLeftTie.makeNan();
}

//**************************************************************************************************
// 
//**************************************************************************************************
rspfOrthoImageMosaic::rspfOrthoImageMosaic(rspfConnectableObject::ConnectableObjectList& inputSources)
   :rspfImageMosaic(inputSources)
{
   m_Delta.makeNan();
   m_UpperLeftTie.makeNan();
}

//**************************************************************************************************
// 
//**************************************************************************************************
rspfOrthoImageMosaic::~rspfOrthoImageMosaic()
{
}

//**************************************************************************************************
// 
//**************************************************************************************************
rspf_uint32 rspfOrthoImageMosaic::getNumberOfOverlappingImages(const rspfIrect& rect,
                                                                 rspf_uint32 resLevel)const
{
   rspf_uint32 result = 0;
   for(rspf_uint32 i = 0; i < m_InputTiePoints.size(); ++i)
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      getInput(i));
      if(interface&&
         !m_InputTiePoints[i].hasNans())
      {
         rspfIrect tempRect = getRelativeRect(i, resLevel);

         if(!tempRect.hasNans())
         {
            if(tempRect.intersects(rect))
            {
               ++result;
            }
         }
      }
   }
   
   return 0;
}

//**************************************************************************************************
// 
//**************************************************************************************************
void rspfOrthoImageMosaic::getOverlappingImages(std::vector<rspf_uint32>& result,
                                                 const rspfIrect& rect,
                                                 rspf_uint32 resLevel)const
{
   result.clear();
   for(rspf_uint32 i = 0; i < m_InputTiePoints.size(); ++i)
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      getInput(i));
      if(interface&&
         !m_InputTiePoints[i].hasNans())
      {
         rspfIrect tempRect = getRelativeRect(i, resLevel);

         if(!tempRect.hasNans())
         {
            if(tempRect.intersects(rect))
            {
               result.push_back(i);
            }
         }
      }
   }
}

//**************************************************************************************************
// Returns the image geometry for the complete mosaic
//**************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfOrthoImageMosaic::getImageGeometry()
{
   if ( !m_Geometry.valid() )
   {
      // The geometry (projection) associated with this mosaic is necessarily the same for all
      // single-image objects feeding into this combiner, So we will copy the first image source's
      // geometry, and modify our copy to reflect the mosaic-specific items.
      rspfImageSource* interface = PTR_CAST(rspfImageSource, getInput(0));
      if(interface)
      {
         rspfRefPtr<rspfImageGeometry> inputGeom = interface->getImageGeometry();
         if ( inputGeom.valid() )
         {
            m_Geometry = new rspfImageGeometry(*inputGeom);
            updateGeometry();
         }
      }
   }
   return m_Geometry;
}

//**************************************************************************************************
//! If this object is maintaining an rspfImageGeometry, this method needs to be called after 
//! each time the contents of the mosaic changes.
//**************************************************************************************************
void rspfOrthoImageMosaic::updateGeometry()
{
   if (!m_Geometry.valid())
      return;

   rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, m_Geometry->getProjection());
   if (mapProj)
   {
      if(m_Units == "degrees")
      {
         mapProj->setDecimalDegreesPerPixel(m_Delta);
         mapProj->setUlGpt(rspfGpt(m_UpperLeftTie.y, m_UpperLeftTie.x, 0.0,
                                    mapProj->origin().datum()));
      }
      else if(m_Units == "meters")
      {
         mapProj->setMetersPerPixel(m_Delta);
         mapProj->setUlEastingNorthing(m_UpperLeftTie);
      }
   }
}

//**************************************************************************************************
// 
//**************************************************************************************************
rspfIrect rspfOrthoImageMosaic::getBoundingRect(rspf_uint32 resLevel) const
{
   rspfDpt decimation;

   decimation.makeNan();

   rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                   getInput(0));
   if(interface)
   {
      interface->getDecimationFactor(resLevel,
                                     decimation);
   }
   if(decimation.hasNans())
   {
      return m_BoundingRect;
   }
   return m_BoundingRect*decimation;
}

//**************************************************************************************************
// 
//**************************************************************************************************
void rspfOrthoImageMosaic::initialize()
{
   m_InputTiePoints.clear();
   m_Delta.makeNan();
   m_UpperLeftTie.makeNan();

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfOrthoImageMosaic::initialize() DEBUG: Entered..."
         << "Number of inputs = " << getNumberOfInputs()
         << std::endl;
   }
   if(getNumberOfInputs())
   {
      m_InputTiePoints.resize(getNumberOfInputs());
      for(rspf_uint32 i = 0; i < getNumberOfInputs(); ++i)
      {
         rspfImageSource *interface = PTR_CAST(rspfImageSource, getInput(i));
         m_InputTiePoints[i].makeNan();
         if(interface)
         {
            rspfRefPtr<rspfImageGeometry> geom = interface->getImageGeometry();
            if( geom.valid() )
            {
               const rspfMapProjection* mapPrj = PTR_CAST(rspfMapProjection, geom->getProjection());
               if(mapPrj)
               {
                  if(!mapPrj->isGeographic())
                  {
                     m_Units = "meters";
                     m_Delta = mapPrj->getMetersPerPixel();
                  }
                  else
                  {
                     m_Units = "degrees";
                     m_Delta = mapPrj->getDecimalDegreesPerPixel();
                  }
                  if(m_Units == "degrees")
                  {
                     m_InputTiePoints[i].x = mapPrj->getUlGpt().lond();
                     m_InputTiePoints[i].y = mapPrj->getUlGpt().latd();
                  }
                  else if(m_Units == "meters")
                  {
                     m_InputTiePoints[i].x = mapPrj->getUlEastingNorthing().x;;
                     m_InputTiePoints[i].y = mapPrj->getUlEastingNorthing().y;
                  }

                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_DEBUG)
                        << "tie points for input " << i << " = "
                        << m_InputTiePoints[i] << std::endl;
                  }

               }
            }
            if(!m_InputTiePoints[i].hasNans())
            {
               if(m_UpperLeftTie.hasNans())
               {
                  m_UpperLeftTie = m_InputTiePoints[i];
               }
               else if(!m_InputTiePoints[i].hasNans())
               {
                  if(m_Units == "meters")
                  {
                     m_UpperLeftTie.x = std::min(m_UpperLeftTie.x, m_InputTiePoints[i].x);
                     m_UpperLeftTie.y = std::max(m_UpperLeftTie.y, m_InputTiePoints[i].y);
                  }
                  else
                  {
                     m_UpperLeftTie.lon = std::min(m_UpperLeftTie.lon, m_InputTiePoints[i].lon);
                     m_UpperLeftTie.lat = std::max(m_UpperLeftTie.lat, m_InputTiePoints[i].lat);
                  }
               }
            }
         }
         else
         {
            m_InputTiePoints[i].makeNan();
//            CLOG << "Input " << i << " will not be used since no ortho information exists" << endl;
         }
      }
   }

   computeBoundingRect();
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "m_UpperLeftTie = " << m_UpperLeftTie << std::endl
         << "delta per pixel = " << m_Delta        << std::endl
         << "bounding rect   = " << m_BoundingRect << std::endl;
   }
   rspfImageMosaic::initialize();

   // Finally, update the geometry (if there was one already defined), to reflect the change in input
   updateGeometry();

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfOrthoImageMosaic::initialize() DEBUG: Leaving..." << std::endl;
   }
}

//**************************************************************************************************
// 
//**************************************************************************************************
rspfRefPtr<rspfImageData> rspfOrthoImageMosaic::getNextTile(rspf_uint32& returnedIdx,
                                                               const rspfIrect& origin,
                                                               rspf_uint32 resLevel)
{
   const char *MODULE="rspfOrthoImageMosaic::getNextTile";
   
   rspf_uint32 size = getNumberOfInputs();
   
   if(theCurrentIndex >= size)
   {
      return rspfRefPtr<rspfImageData>();
   }
   
   rspfImageSource* temp = NULL;
   rspfRefPtr<rspfImageData> result;
   rspfDataObjectStatus status = RSPF_NULL;
   
   do
   {
      temp = PTR_CAST(rspfImageSource, getInput(theCurrentIndex));
      if(temp)
      {
         rspfIrect relRect = getRelativeRect(theCurrentIndex, resLevel);

         if(traceDebug())
         {
            CLOG << "Rel rect for input " << theCurrentIndex
                 << " = " << relRect
                 << "\nRes level:  " << resLevel
                 << endl;
         }

         if(origin.intersects(relRect))
         {
            // get the rect relative to the input rect
            //
            rspfIrect shiftedRect = origin + (rspfIpt(-relRect.ul().x,
                                                        -relRect.ul().y));

            // request that tile from the input space.
            result = temp->getTile(shiftedRect, resLevel);

            // now change the origin to the output origin.
            if (result.valid())
            {
               result->setOrigin(origin.ul());
               
               status = result->getDataObjectStatus();

               if((status == RSPF_NULL)||(status == RSPF_EMPTY))
               {
                  result = NULL;
               }
            }
         }
         else
         {
            result = NULL;
         }
      }

      // Go to next source.
      ++theCurrentIndex;
      
   } while(!result.valid() && (theCurrentIndex<size));

   returnedIdx = theCurrentIndex;
   if(result.valid())
   {
      --returnedIdx;
   }
   return result;
}

//**************************************************************************************************
// 
//**************************************************************************************************
void rspfOrthoImageMosaic::computeBoundingRect(rspf_uint32 resLevel)
{
   const char* MODULE = "rspfOrthoImageMosaic::computeBoundingRect";
   
   m_BoundingRect.makeNan();

   for(rspf_uint32 i = 0; i < m_InputTiePoints.size(); ++ i)
   {
      rspfIrect shiftedRect = getRelativeRect(i, resLevel);

      if(traceDebug())
      {
         CLOG << "bounding rect for input " << (i+1)
              << ":" << shiftedRect << endl;
      }
      if(!shiftedRect.hasNans())
      {
         if(m_BoundingRect.hasNans())
         {
            m_BoundingRect = shiftedRect;
         }
         else
         {
            m_BoundingRect = m_BoundingRect.combine(shiftedRect);
         }
      }
   }
}

//**************************************************************************************************
// 
//**************************************************************************************************
rspfIrect rspfOrthoImageMosaic::getRelativeRect(rspf_uint32 index,
                                                  rspf_uint32 resLevel)const
{
   rspfIrect result;
   rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                   getInput(index));
   result.makeNan();
   if(interface&&
      !m_InputTiePoints[index].hasNans())
   {
      rspfIrect inputRect = interface->getBoundingRect();
      result = inputRect;
      
      if(!inputRect.hasNans())
      {
         rspfDpt shift = (m_InputTiePoints[index] - m_UpperLeftTie);
         
         shift.x/= m_Delta.x;
         shift.y/=-m_Delta.y;
         
	 result = result + shift;
         if(!resLevel)
         {
            return result;
         }
 	 rspfDpt decimation;
 	 interface->getDecimationFactor(resLevel, decimation);
	 if(!decimation.hasNans())
         {
            result = result * decimation;
         }
      }
   }
   
   return result;
}
