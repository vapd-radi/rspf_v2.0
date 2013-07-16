#include <rspf/projection/rspfAffineProjection.h>
RTTI_DEF1(rspfAffineProjection, "rspfAffineProjection", rspfProjection);
#include <rspf/base/rspfAffineTransform.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfAffineProjection:exec");
static rspfTrace traceDebug ("rspfAffineProjection:debug");
rspfAffineProjection::rspfAffineProjection()
   :
      rspfProjection(),
      theClientProjection (NULL),
      theAffineTransform (NULL)
{
}
rspfAffineProjection::rspfAffineProjection(rspfProjection* client)
   :
      rspfProjection(),
      theClientProjection (client),
      theAffineTransform (NULL)
{
}
rspfAffineProjection::rspfAffineProjection(const rspfKeywordlist& geom_kwl,
                                         const char* prefix)
   :
      rspfProjection(),
      theClientProjection (NULL),
      theAffineTransform (NULL)
{
   theClientProjection = rspfProjectionFactoryRegistry::instance()->
                         createProjection(geom_kwl, prefix);
   theAffineTransform = new rspfAffineTransform();
   bool rtn_stat = theAffineTransform->loadState(geom_kwl, prefix);
   if (!theClientProjection ||
       theClientProjection->getErrorStatus() || !rtn_stat)
      setErrorStatus(rspfErrorCodes::RSPF_ERROR);
}
   
rspfAffineProjection::~rspfAffineProjection()
{
}
void
rspfAffineProjection::worldToLineSample (const rspfGpt& worldPoint,
                                          rspfDpt&       lineSampPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::worldToLineSample: Entering..." << std::endl;
   if (theClientProjection.valid() && theAffineTransform.valid())
   {
      theClientProjection->worldToLineSample(worldPoint, lineSampPt);
      theAffineTransform->inverse(lineSampPt);
   }
   else
      lineSampPt.makeNan();
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::worldToLineSample: Returning..." << std::endl;
}
void
rspfAffineProjection::lineSampleToWorld(const rspfDpt& lineSampPt,
                                       rspfGpt&       worldPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::lineSampleToWorld: Entering..." << std::endl;
   if (theClientProjection.valid() && theAffineTransform.valid())
   {
      rspfDpt adjustedPt;
      theAffineTransform->forward(lineSampPt, adjustedPt);
      theClientProjection->lineSampleToWorld(adjustedPt, worldPt);
   }
   else
   {
      worldPt.makeNan();
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::lineSampleToWorld: Returning..." << std::endl;
}
   
void
rspfAffineProjection::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                               const double&   hgt,
                                               rspfGpt&       worldPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::lineSampleHeightToWorld: Entering..." << std::endl;
   if (theClientProjection.valid() && theAffineTransform.valid())
   {
      rspfDpt adjustedPt;
      theAffineTransform->forward(lineSampPt, adjustedPt);
      theClientProjection->lineSampleHeightToWorld(adjustedPt, hgt, worldPt);
   }
   else
   {
      worldPt.makeNan();
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "dEBUG rspfAffineProjection::lineSampleHeightToWorld: Returning..." << std::endl;
}
   
std::ostream& rspfAffineProjection::print(std::ostream& out) const
{
   
   if (theClientProjection.valid() && theAffineTransform.valid())
   {
      out <<
         "rspfAffineProjection:\n"
         "  Member theClientProjection: ";
      theClientProjection->print(out);
      out << "\n Member theAffineTransform: "
		  << *theAffineTransform << std::endl;
   }
   else
   {
	   out << "rspfAffineProjection -- Not initialized." << std::endl;
   }
   return out;
}
std::ostream& operator<<(std::ostream& os, const rspfAffineProjection& m)
{
   return m.print(os);
}
bool rspfAffineProjection::saveState(rspfKeywordlist& kwl,
                                      const char* prefix) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::saveState: entering..." << std::endl;
   if (theClientProjection.valid() && theAffineTransform.valid())
   {
      theClientProjection->saveState(kwl, prefix);
      theAffineTransform->saveState(kwl, prefix);
   }
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::saveState: returning..." << std::endl;
   return true;
}
bool rspfAffineProjection::loadState(const rspfKeywordlist& kwl,
                                            const char* prefix)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::loadState: entering..." << std::endl;
   bool good_load;
   int  err_stat;
   
   if (!theClientProjection)
   {
      theClientProjection = rspfProjectionFactoryRegistry::instance()->
                            createProjection(kwl, prefix);
      err_stat = theClientProjection->getErrorStatus();
      if (err_stat != rspfErrorCodes::RSPF_OK)
      {
         return false;
      }
   }
   else
   {
      good_load = theClientProjection->loadState(kwl, prefix);
      if (!good_load)
      {
         return false;
      }
   }
   if (!theAffineTransform)
   {
      theAffineTransform = new rspfAffineTransform();
   }
   good_load = theAffineTransform->loadState(kwl, prefix);
   if (!good_load)
   {
      return false;
   }
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfAffineProjection::loadState: returning..." << std::endl;
   return true;
}
rspfObject* rspfAffineProjection::dup() const
{
   rspfKeywordlist kwl;
   saveState(kwl);
   rspfProjection* duped = new rspfAffineProjection(kwl);
   return duped;
}
   
rspfGpt rspfAffineProjection::origin() const
{
   if (theClientProjection.valid())
      return theClientProjection->origin();
   return rspfGpt(0.0, 0.0, 0.0);
}
rspfDpt  rspfAffineProjection::getMetersPerPixel() const
{
   if (theClientProjection.valid())
      return theClientProjection->getMetersPerPixel();
   return rspfDpt(rspf::nan(), rspf::nan());
}
      
