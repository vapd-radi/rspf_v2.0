//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// $Id: rspfEastingNorthingCutter.cpp 15766 2009-10-20 12:37:09Z gpotts $
//----------------------------------------------------------------------------

#include <sstream>
#include <rspf/imaging/rspfEastingNorthingCutter.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>

RTTI_DEF2(rspfEastingNorthingCutter, "rspfEastingNorthingCutter", rspfRectangleCutFilter, rspfViewInterface);

static rspfTrace traceDebug(rspfString("rspfEastingNorthingCutter:debug"));


rspfEastingNorthingCutter::rspfEastingNorthingCutter(rspfObject* owner,
                                                       rspfImageSource* inputSource)
   :rspfRectangleCutFilter(owner,
                            inputSource),
    rspfViewInterface(0),
    theViewProjection(0)
{
   theUlEastingNorthing.makeNan();
   theLrEastingNorthing.makeNan();
   theCutType = RSPF_RECTANGLE_NULL_OUTSIDE;
}

rspfEastingNorthingCutter::rspfEastingNorthingCutter(rspfImageSource* inputSource)
   :rspfRectangleCutFilter(inputSource),
    rspfViewInterface(0),
    theViewProjection(0)
{
   theUlEastingNorthing.makeNan();
   theLrEastingNorthing.makeNan();
   theCutType = RSPF_RECTANGLE_NULL_OUTSIDE;
}

rspfEastingNorthingCutter::~rspfEastingNorthingCutter()
{
}

void rspfEastingNorthingCutter::setEastingNorthingRectangle(const rspfDpt& ul,
                                                             const rspfDpt& lr)
{
   theUlEastingNorthing = ul;
   theLrEastingNorthing = lr;

   transformVertices();
}

rspfDpt rspfEastingNorthingCutter::getUlEastingNorthing()const
{
   return theUlEastingNorthing;
}

rspfDpt rspfEastingNorthingCutter::getLrEastingNorthing()const
{
   return theLrEastingNorthing;
}


void rspfEastingNorthingCutter::initialize()
{
   rspfRectangleCutFilter::initialize();
   transformVertices();
}

bool rspfEastingNorthingCutter::saveState(rspfKeywordlist& kwl,
                                           const char* prefix)const
{
   kwl.add(prefix,
           "ul_en",
           rspfString::toString(theUlEastingNorthing.x) + " " +
           rspfString::toString(theUlEastingNorthing.y),
           true);
   kwl.add(prefix,
           "lr_en",
           rspfString::toString(theLrEastingNorthing.x) + " " +
           rspfString::toString(theLrEastingNorthing.y),
           true);

   return rspfRectangleCutFilter::saveState(kwl, prefix);
}

bool rspfEastingNorthingCutter::loadState(const rspfKeywordlist& kwl,
                                           const char* prefix)
{
   const char *ul = kwl.find(prefix,
                             "ul_en");
   const char *lr = kwl.find(prefix,
                             "lr_en");

   theUlEastingNorthing.makeNan();
   theLrEastingNorthing.makeNan();
   
   if(ul)
   {
      istringstream in(ul);
      rspfString x;
      rspfString y;
      
      in >> x >> y;
      
      theUlEastingNorthing.x = x.toDouble();
      theUlEastingNorthing.y = y.toDouble();
      
   }
   
   if(lr)
   {
      istringstream in(lr);
      rspfString x;
      rspfString y;
      
      in >> x >> y;
      
      theLrEastingNorthing.x = x.toDouble();
      theLrEastingNorthing.y = y.toDouble();
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfEastingNorthingCutter::loadState DEBUG:"
         << "\ntheUlEastingNorthing:  " << theUlEastingNorthing
         << "\ntheLrEastingNorthing:  " << theLrEastingNorthing
         << endl;
   }

   return rspfRectangleCutFilter::loadState(kwl, prefix);
}

bool rspfEastingNorthingCutter::setView(rspfObject* baseObject)
{
   rspfProjection* tempProj = PTR_CAST(rspfProjection,
                                        baseObject);
   if(!tempProj)
   {
      return false;
   }
   

   theViewProjection = tempProj;

   transformVertices();

   return true;
}

rspfObject* rspfEastingNorthingCutter::getView()
{
   return theViewProjection.get();
}

const rspfObject* rspfEastingNorthingCutter::getView()const
{
   return theViewProjection.get();
}

void rspfEastingNorthingCutter::transformVertices()
{
   theRectangle.makeNan();

   if(theUlEastingNorthing.hasNans()||
      theLrEastingNorthing.hasNans()||
      (!theViewProjection))
   {
      return;
   }

   rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection,
                                          theViewProjection.get());
   rspfDpt ul;
   rspfDpt lr;
   rspfDpt easting;
   rspfDpt northing;
   if(mapProj)
   {
      mapProj->eastingNorthingToLineSample(theUlEastingNorthing, ul);
      mapProj->eastingNorthingToLineSample(theLrEastingNorthing, lr);

      rspfDrect drect(ul.x, ul.y, lr.x, lr.y);

      theRectangle = drect;

      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfEastingNorthingCutter::transformVertices DEBUG:"
            << "original drect:  " << drect
            << "\ntheRectangle:  " << theRectangle
            << endl;
      }
   }
}
