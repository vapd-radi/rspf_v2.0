//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRLevelFilter.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfRLevelFilter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfTrace.h>

static const char* CURRENT_RLEVEL_KW    = "current_rlevel";
static const char* OVERRIDE_GEOMETRY_KW = "override_geometry";

static rspfTrace traceDebug("rspfRLevelFilter:debug");

RTTI_DEF1(rspfRLevelFilter, "rspfRLevelFilter", rspfImageSourceFilter);

//**************************************************************************************************
rspfRLevelFilter::rspfRLevelFilter()
   : rspfImageSourceFilter(),
     theCurrentRLevel(0),
     theOverrideGeometryFlag(false)
{
}

//**************************************************************************************************
rspfRLevelFilter::~rspfRLevelFilter()
{
}

//**************************************************************************************************
void rspfRLevelFilter::getDecimationFactor(rspf_uint32 resLevel,
                                            rspfDpt& result) const
{
   if(!theInputConnection)
   {
      result.makeNan();
   }

   theInputConnection->getDecimationFactor(resLevel,
                                           result);
}

//**************************************************************************************************
bool rspfRLevelFilter::getOverrideGeometryFlag() const
{
   return theOverrideGeometryFlag;
}

//**************************************************************************************************
void rspfRLevelFilter::setOverrideGeometryFlag(bool override)
{
   theOverrideGeometryFlag = override;
}

//**************************************************************************************************
void rspfRLevelFilter::setCurrentRLevel(rspf_uint32 rlevel)
{
   theCurrentRLevel = rlevel;
   updateGeometry();
}

//**************************************************************************************************
rspf_uint32 rspfRLevelFilter::getCurrentRLevel()const
{
   return theCurrentRLevel;
}

//**************************************************************************************************
// Returns a pointer reference to the active image geometry at this filter. The input source
// geometry is modified, so we need to maintain our own geometry object as a data member.
//**************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfRLevelFilter::getImageGeometry()
{
   // Have we already defined our own geometry? Return it if so:
   if (m_ScaledGeometry.valid()) return m_ScaledGeometry;

   if (!theInputConnection) return rspfRefPtr<rspfImageGeometry>();

   rspf_uint32 rlevel = getCurrentRLevel();
   rspfRefPtr<rspfImageGeometry> inputGeom = theInputConnection->getImageGeometry();

   // If no scaling is happening, just return the input image geometry:
   if ((!inputGeom) || (rlevel == 0) || (getEnableFlag() == false))
      return inputGeom;

   // Need to create a copy of the input geom and modify it as our own, then pass that:
   m_ScaledGeometry = new rspfImageGeometry(*inputGeom);
   updateGeometry();

   // Return our version of the image geometry:
   return m_ScaledGeometry;
}

//**************************************************************************************************
//! If this object is maintaining an rspfImageGeometry, this method needs to be called after 
//! a scale change so that the geometry's projection is modified accordingly.
//**************************************************************************************************
void rspfRLevelFilter::updateGeometry()
{
   if (m_ScaledGeometry.valid())
   {
      // Modify the image geometry's projection with the scale factor before returning geom:
      rspfProjection* proj = m_ScaledGeometry->getProjection();
      rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, proj);;
      if(mapProj)
      {
         rspfDpt decimation;
         getDecimationFactor(theCurrentRLevel, decimation);
         decimation.x = 1.0/decimation.x;
         decimation.y = 1.0/decimation.y;

         // This will adjust both the scale and the tie point to account for decimation.
         mapProj->applyScale(decimation, true);  // recenter tie point flag
      }
   }
}

//**************************************************************************************************
void rspfRLevelFilter::getSummedDecimation(rspfDpt& result) const
{
   // Start with fresh values.
   result.x = 0.0;
   result.y = 0.0;

   rspf_uint32 rlevel = getCurrentRLevel();

   if (rlevel == 0)
   {
      return; // no decimation.
   }

   // Sum the decimations from rlevel one to the current rlevel.
   for (rspf_uint32 i = 1; i <= rlevel; ++i)
   {
      rspfDpt dpt;
      getDecimationFactor(i, dpt);
      if (dpt.hasNans())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfRLevelFilter::getSummedDecimation WARNING:"
            << "\nNans returned from getDecimationFactor!"
            << std::endl;
         result.makeNan();
         return;
      }
      result.x += dpt.x;
      result.y += dpt.y;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfRLevelFilter::getSummedDecimation result: " << result
         << std::endl;
   }
}

//**************************************************************************************************
rspfIrect rspfRLevelFilter::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfIrect rect;
   rect.makeNan();
   if(!theInputConnection) return rect;

   if (isSourceEnabled())
   {
      return theInputConnection->getBoundingRect(theCurrentRLevel);
   }
   return theInputConnection->getBoundingRect(resLevel);
}

//**************************************************************************************************
rspfRefPtr<rspfImageData> rspfRLevelFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return 0;
   }

   if(!isSourceEnabled())
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }

   // Ignoring resLevel as we're in charge of it here!
   return theInputConnection->getTile(tileRect, theCurrentRLevel);
}

//**************************************************************************************************
bool rspfRLevelFilter::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   const char* current  = kwl.find(prefix, CURRENT_RLEVEL_KW);
   const char* override = kwl.find(prefix, OVERRIDE_GEOMETRY_KW);

   if(current)
      theCurrentRLevel = rspfString(current).toULong();

   if(override)
      theOverrideGeometryFlag = rspfString(override).toBool();

   return rspfImageSourceFilter::loadState(kwl, prefix);

   // An existing image geometry object here will need to be updated:
   updateGeometry();
}

//**************************************************************************************************
bool rspfRLevelFilter::saveState(rspfKeywordlist& kwl,
                                  const char* prefix)const
{
   kwl.add(prefix,
           CURRENT_RLEVEL_KW,
           theCurrentRLevel,
           true);

   kwl.add(prefix,
           OVERRIDE_GEOMETRY_KW,
           (rspf_uint32)theOverrideGeometryFlag,
           true);

   return rspfImageSourceFilter::saveState(kwl, prefix);
}
