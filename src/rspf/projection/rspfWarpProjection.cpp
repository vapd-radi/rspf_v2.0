#include <rspf/projection/rspfWarpProjection.h>
RTTI_DEF1(rspfWarpProjection, "rspfWarpProjection", rspfProjection);
#include <rspf/base/rspfQuadTreeWarp.h>
#include <rspf/base/rspfAffineTransform.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfNotifyContext.h>
static const char* AFFINE_PREFIX     = "affine.";
static const char* QUADWARP_PREFIX   = "quadwarp.";
static const char* PROJECTION_PREFIX = "projection.";
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfWarpProjection:exec");
static rspfTrace traceDebug ("rspfWarpProjection:debug");
rspfWarpProjection::rspfWarpProjection()
   :
      rspfProjection(),
      theClientProjection (0),
      theWarpTransform (0),
      theAffineTransform (0)
{
   theWarpTransform   = new rspfQuadTreeWarp;
   theAffineTransform = new rspfAffineTransform;   
}
rspfWarpProjection::rspfWarpProjection(rspfProjection* client)
   :
      rspfProjection(),
      theClientProjection (client),
      theWarpTransform (0),
      theAffineTransform (0)
{
   theWarpTransform   = new rspfQuadTreeWarp;
   theAffineTransform = new rspfAffineTransform;
}
rspfWarpProjection::rspfWarpProjection(const rspfKeywordlist& geom_kwl,
                                         const char* prefix)
   :
      rspfProjection(),
      theClientProjection (0),
      theWarpTransform (0),
      theAffineTransform (0)
{
   theClientProjection = rspfProjectionFactoryRegistry::instance()->
                         createProjection(geom_kwl, prefix);
   theWarpTransform = new rspfQuadTreeWarp();
   bool rtn_stat = theWarpTransform->loadState(geom_kwl, prefix);
   theAffineTransform = new rspfAffineTransform();
   rtn_stat &= theAffineTransform->loadState(geom_kwl, prefix);
   if ((!theClientProjection) ||
       (theClientProjection->getErrorStatus()) ||
       (!rtn_stat))
      setErrorStatus(rspfErrorCodes::RSPF_ERROR);
}
   
rspfWarpProjection::~rspfWarpProjection()
{
   theClientProjection = 0;
   theWarpTransform = 0;
   theAffineTransform = 0;
}
void rspfWarpProjection::worldToLineSample (const rspfGpt& worldPoint,
                                        rspfDpt&       lineSampPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::worldToLineSample: Entering..." << std::endl;
   if (theClientProjection.valid() && theWarpTransform.valid() && theAffineTransform.valid())
   {
      theClientProjection->worldToLineSample(worldPoint, lineSampPt);
      theAffineTransform->inverse(lineSampPt);
      theWarpTransform->inverse(lineSampPt);
   }
   else
   {
      lineSampPt.makeNan();
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::worldToLineSample: Returning..." << std::endl;
}
void rspfWarpProjection::lineSampleToWorld(const rspfDpt& lineSampPt,
                                            rspfGpt&       worldPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::lineSampleToWorld: Entering..." << std::endl;
   if (theClientProjection.valid() && theWarpTransform.valid() && theAffineTransform.valid())
   {
      rspfDpt adjustedPt;
      theWarpTransform->forward(adjustedPt);
      theAffineTransform->forward(lineSampPt, adjustedPt);
      theClientProjection->lineSampleToWorld(adjustedPt, worldPt);
   }
   else
   {
      worldPt.makeNan();
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::lineSampleToWorld: Returning..." << std::endl;
}
   
void rspfWarpProjection::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                             const double&   hgt,
                                             rspfGpt&       worldPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::lineSampleHeightToWorld: Entering..." << std::endl;
   if (theClientProjection.valid() && theWarpTransform.valid() && theAffineTransform.valid())
   {
      rspfDpt adjustedPt;
      theAffineTransform->forward(lineSampPt, adjustedPt);
      theWarpTransform->forward(adjustedPt);
      theClientProjection->lineSampleHeightToWorld(adjustedPt, hgt, worldPt);
   }
   else
   {
      worldPt.makeNan();
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::lineSampleHeightToWorld: Returning..." << std::endl;
}
   
std::ostream& rspfWarpProjection::print(std::ostream& out) const
{
   if (theClientProjection.valid() && theWarpTransform.valid() && theAffineTransform.valid())
   {
      out <<
         "rspfWarpProjection:\n"
	  << "  Member theClientProjection: ";
      theClientProjection->print(out);
      
      out << "  Member theAffineTransform: "  << *theAffineTransform <<  "\n"
		  << "  Member theWarpTransform: "    << *theWarpTransform << std::endl;
   }
   else
   {
	   out << "rspfWarpProjection -- Not inititialized." << std::endl;
   }
   return out;
}
std::ostream& operator<<(std::ostream& os, const rspfWarpProjection& m)
{
   return m.print(os);
}
bool rspfWarpProjection::saveState(rspfKeywordlist& kwl,
                                            const char* prefix) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::saveState: entering..." << std::endl;
   rspfString projPrefix     = rspfString(prefix) + PROJECTION_PREFIX;
   rspfString affinePrefix   = rspfString(prefix) + AFFINE_PREFIX;
   rspfString quadwarpPrefix = rspfString(prefix) + QUADWARP_PREFIX;
   if (theClientProjection.valid() && theWarpTransform.valid() && theAffineTransform.valid())
   {
      theClientProjection->saveState(kwl, projPrefix.c_str());
      theAffineTransform->saveState(kwl, affinePrefix.c_str());
      theWarpTransform->saveState(kwl, quadwarpPrefix.c_str());
   }
      
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::saveState: returning..." << std::endl;
   return rspfProjection::saveState(kwl, prefix);
}
bool rspfWarpProjection::loadState(const rspfKeywordlist& kwl,
                                            const char* prefix)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfWarpProjection::loadState: entering..." << std::endl;
   bool result = true;
   
   rspfString projPrefix = rspfString(prefix) + PROJECTION_PREFIX;
   rspfString affinePrefix = rspfString(prefix) + AFFINE_PREFIX;
   rspfString quadwarpPrefix = rspfString(prefix) + QUADWARP_PREFIX;
   theClientProjection = 0;
   if (!theWarpTransform)
      theWarpTransform = new rspfQuadTreeWarp();
   if (!theAffineTransform)
      theAffineTransform = new rspfAffineTransform();
   theClientProjection = rspfProjectionFactoryRegistry::instance()->createProjection(kwl, projPrefix.c_str());
   if(!theClientProjection)
   {
      result = false;
   }
   else
   {
      theWarpTransform->loadState(kwl,   quadwarpPrefix.c_str());
      theAffineTransform->loadState(kwl, affinePrefix.c_str());
   }
   
   return rspfProjection::loadState(kwl, prefix);
}
rspfObject* rspfWarpProjection::dup() const
{
   rspfKeywordlist kwl;
   saveState(kwl);
   return new rspfWarpProjection(kwl);
}
   
rspfGpt rspfWarpProjection::origin() const
{
   if (theClientProjection.valid())
      return theClientProjection->origin();
   return rspfGpt(0.0, 0.0, 0.0);
}
rspfDpt  rspfWarpProjection::getMetersPerPixel() const
{
   if (theClientProjection.valid())
      return theClientProjection->getMetersPerPixel();
   return rspfDpt(rspf::nan(), rspf::nan());
}
      
void rspfWarpProjection::setNewWarpTransform(rspf2dTo2dTransform* warp)
{
   if(warp)
   {
      theWarpTransform = warp;
   }
}
void rspfWarpProjection::setNewAffineTransform(rspf2dTo2dTransform* affine)
{
   if(affine)
   {
      theAffineTransform = affine;
   }
}
