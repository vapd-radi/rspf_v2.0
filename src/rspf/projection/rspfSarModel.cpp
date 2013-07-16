#include <rspf/elevation/rspfHgtRef.h>
#include <rspf/projection/rspfSarModel.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotify.h>
RTTI_DEF1(rspfSarModel, "rspfSarModel", rspfSensorModel);
static rspfTrace traceExec (rspfString("rspfSarModel:exec"));
static rspfTrace traceDebug(rspfString("rspfSarModel:debug"));
#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfSarModel.cpp 17206 2010-04-25 23:20:40Z dburken $";
#endif
static const char ACQ_MODE_KW[]         = "acq_mode";
static const char ORP_POS_KW[]          = "orp_pos";
static const char ORP_CENTER_KW[]       = "orp_ctr_xy";
static const char OPNORM_KW[]           = "opnorm";
static const char OP_X_AXIS_KW[]        = "opx";
static const char OIPR_KW[]             = "oipr";
static const char PIX_SIZE_KW[]         = "pixel_size";
static const char ARP_TIME_KW[]         = "arptm";
static const char ARP_POLY_COEFF_X_KW[] = "arppol_coeff_x";
static const char ARP_POLY_COEFF_Y_KW[] = "arppol_coeff_y";
static const char ARP_POLY_COEFF_Z_KW[] = "arppol_coeff_z";
static const char TIME_COEFF[]          = "time_coeff";
static const rspfString PARAM_NAMES[] ={"x_pos_offset",
                                         "y_pos_offset",
                                         "z_pos_offset"};
static const rspfString PARAM_UNITS[] ={"meters",
                                         "meters",
                                         "meters"};
 
rspfSarModel::rspfSarModel()
   :
   theAcquisitionMode(),
   theOrpPosition(),
   theOrpCenter(),
   theOutputPlaneNormal(),
   theOipr(0.0),
   theLsrOffset(0, 0, 0)
{
   initAdjustableParameters();
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfSarModel::rspfSarModel DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   
}
rspfSarModel::~rspfSarModel()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG DESTRUCTOR: ~rspfSarModel(): entering..." << std::endl;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG DESTRUCTOR: ~rspfSarModel(): returning..." << std::endl;
}
void rspfSarModel::initAdjustableParameters()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSarModel::initAdjustableParameters: entering..." << std::endl;
   int numParams = NUM_ADJUSTABLE_PARAMS;
   resizeAdjustableParameterArray(NUM_ADJUSTABLE_PARAMS);
   
   for (int i=0; i<numParams; i++)
   {
      setAdjustableParameter(i, 0.0);
      setParameterDescription(i, PARAM_NAMES[i]);
      setParameterUnit(i, PARAM_UNITS[i]);
   }
   
   setParameterSigma(X_POS, 50.0);
   setParameterSigma(Y_POS, 50.0);
   setParameterSigma(Z_POS, 50.0);  
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSarModel::initAdjustableParameters: returning..." << std::endl;
}
bool rspfSarModel::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if (rspfSensorModel::loadState(kwl, prefix) == false)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfSarModel::loadState(): returning with error..."
            << std::endl;
      }
      
      return false;
   }
   const char* lookup;
   
   lookup = kwl.find(prefix, ACQ_MODE_KW);
   theAcquisitionMode = rspfSarModel::UNKNOWN;
   if (lookup)
   {
      setAcquisitionMode(rspfString(lookup));
   }
   if (theAcquisitionMode == rspfSarModel::UNKNOWN)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << ACQ_MODE_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theOrpPosition.makeNan();
   lookup = kwl.find(prefix, ORP_POS_KW);
   if (lookup)
   {
      theOrpPosition.toPoint(std::string(lookup));
   }
   if (theOrpPosition.hasNans())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << ORP_POS_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   
   theOrpCenter.makeNan();
   lookup = kwl.find(prefix, ORP_CENTER_KW);
   if (lookup)
   {
      theOrpCenter.toPoint(std::string(lookup));
   }
   if (theOrpCenter.hasNans())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << ORP_CENTER_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theOutputPlaneNormal.makeNan();
   lookup = kwl.find(prefix, OPNORM_KW);
   if (lookup)
   {
      theOutputPlaneNormal.toPoint(std::string(lookup));
   }
   if (theOutputPlaneNormal.hasNans())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << OPNORM_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theOutputPlaneXaxis.makeNan();
   lookup = kwl.find(prefix, OP_X_AXIS_KW);
   if (lookup)
   {
      theOutputPlaneXaxis.toPoint(std::string(lookup));
   }
   if (theOutputPlaneXaxis.hasNans())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << OP_X_AXIS_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theOipr = rspf::nan();
   lookup = kwl.find(prefix, OIPR_KW);
   if (lookup)
   {
      theOipr = rspfString::toFloat64(lookup);
   }
   
   if (rspf::isnan(theOipr))
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << OIPR_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   thePixelSize = rspf::nan();
   lookup = kwl.find(prefix, PIX_SIZE_KW);
   if (lookup)
   {
      thePixelSize = rspfString::toFloat64(lookup);
   }
   
   if (rspf::isnan(thePixelSize))
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << PIX_SIZE_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theArpTime = rspf::nan();
   lookup = kwl.find(prefix, ARP_TIME_KW);
   if (lookup)
   {
      theArpTime = rspfString::toFloat64(lookup);
   }
   if (rspf::isnan(theArpTime))
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << ARP_TIME_KW << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theArpXPolCoeff.clear();
   theArpYPolCoeff.clear();
   theArpZPolCoeff.clear();
   rspf_uint32 x_count = kwl.numberOf(ARP_POLY_COEFF_X_KW);
   rspf_uint32 y_count = kwl.numberOf(ARP_POLY_COEFF_Y_KW);
   rspf_uint32 z_count = kwl.numberOf(ARP_POLY_COEFF_Z_KW);
   if ( (x_count == y_count) && (x_count == z_count) )
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
   
      while ( (found < x_count) &&
              (count < 100) ) // limit lookups to 100...
      {
         rspfString s = rspfString::toString(count);
         rspfString xkw = ARP_POLY_COEFF_X_KW;
         rspfString ykw = ARP_POLY_COEFF_Y_KW;
         rspfString zkw = ARP_POLY_COEFF_Z_KW;
         xkw += s;
         ykw += s;
         zkw += s;
         const char* xLookup = kwl.find(prefix, xkw.c_str());
         const char* yLookup = kwl.find(prefix, ykw.c_str());
         const char* zLookup = kwl.find(prefix, zkw.c_str());
         if (xLookup && yLookup && zLookup)
         {
            ++found;
            theArpXPolCoeff.push_back(rspfString::toFloat64(xLookup));
            theArpYPolCoeff.push_back(rspfString::toFloat64(yLookup));
            theArpZPolCoeff.push_back(rspfString::toFloat64(zLookup));
         }
         ++count;
      }
   }     
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << "arppol_coeff" << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   theTimeCoeff.clear();
   rspf_uint32 timeCount = kwl.numberOf(TIME_COEFF);
   if (timeCount)
   {
      rspf_uint32 found = 0;
      rspf_uint32 count = 0;
   
      while ( (found < timeCount) &&
              (count < 100) ) // limit lookups to 100...
      {
         rspfString kw = TIME_COEFF;
         kw += rspfString::toString(count);;
         lookup = kwl.find(prefix, kw.c_str());
         if (lookup)
         {
            ++found;
            theTimeCoeff.push_back(rspfString::toFloat64(lookup));
         }
         ++count;
      }
   }     
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG rspfSarModel::loadState() lookup failure: "
            << TIME_COEFF << "\nreturning with error..."
            << std::endl;
      }
      return false;
   }
   
   
   updateModel();
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfSarModel::loadState() DEBUG: " << std::endl;
   }
   
   return true;
}
bool rspfSarModel::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   kwl.add(prefix, ACQ_MODE_KW, getAcquistionModeString());
   kwl.add(prefix, ORP_POS_KW, theOrpPosition.toString(15).c_str());
   kwl.add(prefix, ORP_CENTER_KW, theOrpCenter.toString(15).c_str());
   kwl.add(prefix, OPNORM_KW, theOutputPlaneNormal.toString(15).c_str());
   kwl.add(prefix, OP_X_AXIS_KW, theOutputPlaneXaxis.toString(15).c_str());
   kwl.add(prefix, OIPR_KW, theOipr);
   kwl.add(prefix, PIX_SIZE_KW, thePixelSize);
   kwl.add(prefix, ARP_TIME_KW, theArpTime);
   rspf_uint32 i;
   for (i = 0; i < theArpXPolCoeff.size(); ++i)
   {
      rspfString kw = ARP_POLY_COEFF_X_KW;
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, theArpXPolCoeff[i]);
   }
   for (i = 0; i < theArpYPolCoeff.size(); ++i)
   {
      rspfString kw = ARP_POLY_COEFF_Y_KW;
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, theArpYPolCoeff[i]);
   }
   for (i = 0; i < theArpZPolCoeff.size(); ++i)
   {
      rspfString kw = ARP_POLY_COEFF_Z_KW;
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, theArpZPolCoeff[i]);
   }
   for (i = 0; i < theTimeCoeff.size(); ++i)
   {
      rspfString kw = TIME_COEFF;
      kw += rspfString::toString(i);
      kwl.add(prefix, kw, theTimeCoeff[i]);
   }
   
   return rspfSensorModel::saveState(kwl, prefix);
}
void rspfSarModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfSarModel::writeGeomTemplate: entering..." << std::endl;
   os <<
      "//************************************\n"
      "// Template for SAR model keywordlist\n"
      "//************************************\n";
 
   rspfSensorModel::writeGeomTemplate(os);
   os << "//***\n"
      << "// Base-class SAR Keywords:\n"
      << "//***\n"
      << ACQ_MODE_KW         << ": <SCAN, SPOT>\n"
      << ORP_POS_KW          << ": <meters (ECF)>\n"
      << ORP_CENTER_KW       << ": <pixels>\n"
      << OPNORM_KW           << ": <unit vector>\n"
      << OP_X_AXIS_KW        << ": <unit vector>\n"
      << OIPR_KW             << ": <meters>\n"
      << PIX_SIZE_KW         << ": <meters>\n"
      << ARP_TIME_KW         << ": <sec>\n"
      << ARP_POLY_COEFF_X_KW << ": <meters (ECF)>\n"
      << ARP_POLY_COEFF_Y_KW << ": <meters (ECF)>\n"
      << ARP_POLY_COEFF_Z_KW << ": <meters (ECF)>\n"
      << TIME_COEFF          << ": <sec>\n";
   }
std::ostream& rspfSarModel::print(std::ostream& out) const
{
   out << "// rspfSarModel::print:" << std::endl;
   rspfKeywordlist kwl;
   if (saveState(kwl))
   {
      kwl.print(out);
   }
   else
   {
      out << "// rspfSarModel::saveState failed!" << std::endl;
   }
   return rspfSensorModel::print(out);
}
rspfString rspfSarModel::getAcquistionModeString() const
{
   rspfString result;
   
   switch (theAcquisitionMode)
   {
      case rspfSarModel::SCAN:
         result = "scan";
         break;
      case rspfSarModel::SPOT:
         result = "spot";
         break;
      case rspfSarModel::UNKNOWN:
      default:
         result = "unknown";
         break;
   }
   return result;
}
void rspfSarModel::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                            const double&   heightAboveEllipsoid,
                                            rspfGpt&       worldPt) const
{
   rspfEcefPoint opPt;
   computeOPfromImage(lineSampPt, opPt);
   
   rspf_float64 iTime = getArpTime(lineSampPt);
   
   rspfEcefPoint  arpPos = getArpPos(iTime);
   rspfEcefVector arpVel = getArpVel(iTime);
   
   rspf_float64 range;
   rspf_float64 doppler;
   computeRangeDoppler(opPt, arpPos, arpVel, range, doppler);
   
   rspf_float64 hgtSet;
   if ( rspf::isnan(heightAboveEllipsoid) )
   {
      rspfGpt orpG(theOrpPosition);
      hgtSet = orpG.height();
   }
   else
   {
      hgtSet = heightAboveEllipsoid;
   }
      
   rspfHgtRef hgtRef(AT_HGT, hgtSet);
   
   rspfEcefPoint ellPt;
   projOPtoSurface(opPt, range, doppler, arpPos, arpVel, &hgtRef, ellPt);
   rspfGpt iPt(ellPt);
   
   worldPt = iPt;
}
void rspfSarModel::worldToLineSample(const rspfGpt& world_point,
                                      rspfDpt&       image_point) const
{
   rspfEcefPoint pt(world_point);
   
   rspfEcefPoint opPt;
   projEllipsoidToOP(pt, opPt);
   
   computeImageFromOP(opPt,image_point);
}
void rspfSarModel::lineSampleToWorld(const rspfDpt& lineSampPt,
                                      rspfGpt&       worldPt) const
{
   rspfEcefPoint opPt;
   computeOPfromImage(lineSampPt, opPt);
   
   rspf_float64 iTime = getArpTime(lineSampPt);
   
   rspfEcefPoint  arpPos = getArpPos(iTime);
   rspfEcefVector arpVel = getArpVel(iTime);
   
   rspf_float64 range;
   rspf_float64 doppler;
   computeRangeDoppler(opPt, arpPos, arpVel, range, doppler);
   
   rspfHgtRef hgtRef(AT_DEM);
   
   rspfEcefPoint ellPt;
   projOPtoSurface(opPt, range, doppler, arpPos, arpVel, &hgtRef, ellPt);
   rspfGpt iPt(ellPt);
   
   worldPt = iPt;
}
void rspfSarModel::imagingRay(const rspfDpt& image_point,
                               rspfEcefRay&   image_ray) const
{
   rspfEcefPoint opPt;
   computeOPfromImage(image_point, opPt);
   
   rspf_float64 iTime = getArpTime(image_point);
   
   rspfEcefPoint arpPos = getArpPos(iTime);
   
   rspfGpt start(arpPos);
   rspfGpt end(opPt);
   image_ray = rspfEcefRay(start, end);
   return;   
}
rspfSensorModel::CovMatStatus rspfSarModel::getObsCovMat(
   const rspfDpt& /* ipos */, NEWMAT::SymmetricMatrix& Cov)
{
   NEWMAT::SymmetricMatrix B(2);
   B(1,1) = thePixelSpacing;
   B(2,2) = B(1,1);
   B(1,2) = 0.0;
   rspf_float64 defPointingSigma = 0.5;
   NEWMAT::SymmetricMatrix P(2);
   P(1,1) = defPointingSigma*defPointingSigma;
   P(2,2) = P(1,1);
   P(1,2) = 0.0;
   NEWMAT::SymmetricMatrix Cm;
   Cm << B * P * B.t();
   
   NEWMAT::SymmetricMatrix Ctot = Cm; //+ other contributors as identified
   
   NEWMAT::SymmetricMatrix Bad(2);
   Bad(1,1) = 1.0;
   Bad(2,2) = theParDopWRTaz;
   Bad(1,2) = 0.0;
   
   Cov << Bad * Ctot * Bad.t();
   return rspfSensorModel::COV_PARTIAL;
}
void rspfSarModel::updateModel()
{
   thePixelSpacing = thePixelSize;
   
   theOPZ = theOutputPlaneNormal.unitVector();
   theOPX = theOutputPlaneXaxis.unitVector();
   
   theOPY = theOPZ.cross(theOPX);
   theOPY.normalize();
   
   theOPX = theOPY.cross(theOPZ);
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG updateModel:\n OP vectors...";
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "\n  OPX: "<<theOPX
      << "\n  OPY: "<<theOPY
      << "\n  OPZ: "<<theOPZ<<endl;
      rspfGpt orpg(theOrpPosition);
      rspfNotify(rspfNotifyLevel_DEBUG)
      <<"  ORPgeo: "<<orpg<<endl;
   }
   theLsrOffset.x() = getAdjustableParameter(X_POS) *
                      getParameterSigma(X_POS);
   theLsrOffset.y() = getAdjustableParameter(Y_POS) *
                      getParameterSigma(Y_POS);
   theLsrOffset.z() = getAdjustableParameter(Z_POS) *
                      getParameterSigma(Z_POS);
   theRefGndPt = rspfGpt(theOrpPosition);
   theRefImgPt = theOrpCenter;
   theGSD.samp = thePixelSpacing;
   theGSD.line = thePixelSpacing;
   theMeanGSD  = thePixelSpacing;
   
}
rspfEcefPoint rspfSarModel::getArpPos() const
{
   rspfEcefPoint pos(theArpXPolCoeff[0],
                      theArpYPolCoeff[0],
                      theArpZPolCoeff[0]);
                       
   rspfGpt posG(pos);
   rspfLsrSpace enu(posG);
   rspfColumnVector3d tpnn(theLsrOffset[0],theLsrOffset[1], theLsrOffset[2]);
   rspfEcefVector ecfOffset = enu.lsrToEcefRotMatrix()*tpnn;
   pos = pos + ecfOffset;
   
   return pos;
}
rspfEcefVector rspfSarModel::getArpVel() const
{
   rspfEcefVector vec(theArpXPolCoeff[1],
                       theArpYPolCoeff[1],
                       theArpZPolCoeff[1]);
   return vec;
}
rspfEcefPoint rspfSarModel::getArpPos(const rspf_float64& time) const
{
   rspf_int32 i;
   rspf_uint32 nTermsX = (rspf_uint32)theArpXPolCoeff.size();
   rspf_uint32 nTermsY = (rspf_uint32)theArpYPolCoeff.size();
   rspf_uint32 nTermsZ = (rspf_uint32)theArpZPolCoeff.size();
   rspf_float64 x = theArpXPolCoeff[nTermsX-1];
   for (i=nTermsX-2; i>=0; i--)
      x = x*time + theArpXPolCoeff[i];
      
   rspf_float64 y = theArpYPolCoeff[nTermsY-1];
   for (i=nTermsY-2; i>=0; i--)
      y = y*time + theArpYPolCoeff[i];
      
   rspf_float64 z = theArpZPolCoeff[nTermsZ-1];
   for (i=nTermsZ-2; i>=0; i--)
      z = z*time + theArpZPolCoeff[i];
   
   rspfEcefPoint pos(x, y, z);
                       
   rspfGpt posG(pos);
   rspfLsrSpace enu(posG);
   rspfColumnVector3d tpnn(theLsrOffset[0],theLsrOffset[1], theLsrOffset[2]);
   rspfEcefVector ecfOffset = enu.lsrToEcefRotMatrix()*tpnn;
   pos = pos + ecfOffset;
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG getArpPos:"<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " time   = "<<time<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " pos    = "<<pos<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " offset = "<<theLsrOffset<<endl;
   }
   
   return pos;
}
rspfEcefVector rspfSarModel::getArpVel(const rspf_float64& time) const
{
   rspf_int32 i;
   rspf_uint32 nTermsX = (rspf_uint32)theArpXPolCoeff.size();
   rspf_uint32 nTermsY = (rspf_uint32)theArpYPolCoeff.size();
   rspf_uint32 nTermsZ = (rspf_uint32)theArpZPolCoeff.size();
   
   rspf_float64 x = nTermsX * theArpXPolCoeff[nTermsX-1];
   for (i=nTermsX-2; i>=1; i--)
      x = x*time + i*theArpXPolCoeff[i];
      
   rspf_float64 y = nTermsY * theArpYPolCoeff[nTermsY-1];
   for (i=nTermsY-2; i>=1; i--)
      y = y*time + i*theArpYPolCoeff[i];
      
   rspf_float64 z = nTermsZ * theArpZPolCoeff[nTermsZ-1];
   for (i=nTermsZ-2; i>=1; i--)
      z = z*time + i*theArpZPolCoeff[i];
      
   rspfEcefVector vel(x, y, z);
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG getArpVel:"<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " time = "<<time<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " vel  = "<<vel<<endl;
   }
   
   return vel;
}
rspf_float64 rspfSarModel::getArpTime() const
{
   return theTimeCoeff[0];
}
rspf_float64 rspfSarModel::getArpTime(const rspfDpt& imgPt) const
{
   rspf_float64 s = imgPt.samp;
   rspf_float64 l = imgPt.line;
  
   rspf_float64 s2 = s*s;
   rspf_float64 s3 = s2*s;
   rspf_float64 l2 = l*l;
   rspf_float64 l3 = l2*l;
      
   rspf_float64 imgTime = theTimeCoeff[0];
   imgTime += theTimeCoeff[1]  * l;
   imgTime += theTimeCoeff[2]  * l2;
   imgTime += theTimeCoeff[3]  * l3;
   imgTime += theTimeCoeff[4]  * s;
   imgTime += theTimeCoeff[5]  * s * l;
   imgTime += theTimeCoeff[6]  * s * l2;
   imgTime += theTimeCoeff[7]  * s * l3;
   imgTime += theTimeCoeff[8]  * s2;
   imgTime += theTimeCoeff[9]  * s2 * l;
   imgTime += theTimeCoeff[10] * s2 * l2;
   imgTime += theTimeCoeff[11] * s2 * l3;
   imgTime += theTimeCoeff[12] * s3;
   imgTime += theTimeCoeff[13] * s3 * l;
   imgTime += theTimeCoeff[14] * s3 * l2;
   imgTime += theTimeCoeff[15] * s3 * l3;
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG getArpTime:"<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " imgPt   = "<<imgPt<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << " imgTime = "<<imgTime<<endl;
   }
      
   return imgTime;
}
bool rspfSarModel::computeRangeDoppler(const rspfEcefPoint& pt,
                                        const rspfEcefPoint& arpPos,
                                        const rspfEcefVector& arpVel,
                                        rspf_float64& range,
                                        rspf_float64& doppler) const
{
   rspfEcefVector delta = pt - arpPos;
   range = delta.magnitude();
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG computeRangeDoppler: range vector..." << endl;
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "delta components: \n"<<delta<<endl;
   }
   
   if (range != 0.0)
   {
      doppler = arpVel.dot(delta) / range;
      return true;
   }
   
   return false;
}
bool rspfSarModel::computeOPfromImage(const rspfDpt& imgPt,
                                       rspfEcefPoint& opPt) const
{
   rspf_float64 dL = imgPt.line - theOrpCenter.line;
   rspf_float64 dS = imgPt.samp - theOrpCenter.samp;
      
   dL *= thePixelSpacing;
   dS *= thePixelSpacing;
   
   opPt = theOPX*dL + theOPY*dS + theOrpPosition;
   
   return true;
}
bool rspfSarModel::computeImageFromOP(const rspfEcefPoint& opPt, rspfDpt& imgPt) const
{
   rspfEcefVector delta(opPt - theOrpPosition);
   
   rspf_float64 dL = delta.dot(theOPX) / thePixelSpacing;
   rspf_float64 dS = delta.dot(theOPY) / thePixelSpacing;
      
   imgPt.line = theOrpCenter.line + dL;
   imgPt.samp = theOrpCenter.samp + dS;
   
   return true;
}
bool rspfSarModel::projOPtoSurface(const rspfEcefPoint& opPt,
                                    const rspf_float64& range,
                                    const rspf_float64& doppler,
                                    const rspfEcefPoint& arpPos,
                                    const rspfEcefVector& arpVel,
                                    const rspfHgtRef* hgtRef,
                                    rspfEcefPoint& ellPt) const
{
   rspf_float64 sx  = 0.0;
   rspf_float64 sy  = 0.0;
   
   rspfColumnVector3d tpn(sx, sy, 1.0);
   rspfColumnVector3d tpnn(-sx, -sy, 1.0);
   rspfEcefPoint rg(opPt);
   
   NEWMAT::SymmetricMatrix BtB(3);
   NEWMAT::ColumnVector BtF(3);
   NEWMAT::ColumnVector F(3);
   NEWMAT::ColumnVector dR(3);
   
   F(1)=theOipr;
   
   rspf_int32 iter = 0;
   
   while ((F(1)>=theOipr || F(2)>=0.0003048 || F(3)>=0.5) && iter<5)
   {
      rspfGpt pg(rg);
      rspf_float64 atHgt = hgtRef->getRefHeight(pg);
      pg.height(atHgt);
      rspfEcefPoint rt(pg);
      rspfLsrSpace enu(pg);
      rspfEcefVector st = enu.lsrToEcefRotMatrix()*tpn;
      rspf_float64 rngComp;
      rspf_float64 dopComp;
      computeRangeDoppler(rg, arpPos, arpVel, rngComp, dopComp);
      
      rspf_float64 diffHgt = st.dot(rg-rt);
      F(1) = rngComp - range;
      F(2) = dopComp - doppler;
      F(3) = diffHgt;
   
      rspfEcefVector delta = rg - arpPos;
      rspfEcefVector deltaUv = delta.unitVector();
      rspfEcefVector p_fr = -deltaUv;
      rspf_float64 vDotr = arpVel.dot(deltaUv);
      rspfEcefVector p_fd = (arpVel - deltaUv*vDotr)/rngComp;
      rspfColumnVector3d p_ft = enu.lsrToEcefRotMatrix()*tpnn;
      NEWMAT::Matrix B = rspfMatrix3x3::create(p_fr[0], p_fr[1], p_fr[2],
                                                p_fd[0], p_fd[1], p_fd[2],
                                                p_ft[0], p_ft[1], p_ft[2]);
      BtF << B.t()*F;
      BtB << B.t()*B;
      dR = solveLeastSquares(BtB, BtF);
      for (rspf_int32 k=0; k<3; k++)
         rg[k] -= dR(k+1);
      
      iter++;
   }
   
   ellPt = rg;
   return true;
}
bool rspfSarModel::projEllipsoidToOP(const rspfEcefPoint& ellPt,
                                      rspfEcefPoint& opPt) const
{
   rspfDpt currentImagePt;
   rspfEcefPoint selPt;
   
   currentImagePt = theOrpCenter;
   
   rspf_float64 checkTime = 1.0;
   rspf_float64 lTime=10000.0;
   rspf_int32 iter = 0;
   
   
   while (checkTime > 10.0e-6 && iter<5)
   {
   
      rspf_float64 cTime = getArpTime(currentImagePt);
      checkTime = fabs(cTime-lTime)/cTime;
      rspfEcefPoint  arpPos = getArpPos(cTime);
      rspfEcefVector arpVel = getArpVel(cTime);
      rspfEcefVector arpVelunit = arpVel.unitVector();
      rspf_float64 constOP = theOPZ.dot(
         rspfEcefVector(theOrpPosition[0],theOrpPosition[1],theOrpPosition[2]));
      rspf_float64 constRD = arpVelunit.dot(
         rspfEcefVector(ellPt[0],ellPt[1],ellPt[2]));
      rspf_float64 ndot = theOPZ.dot(arpVelunit);
      rspf_float64 det = 1.0 - ndot*ndot;
      rspf_float64 c1 = (constOP - ndot*constRD) / det;
      rspf_float64 c2 = (constRD - ndot*constOP) / det;
      rspfEcefPoint ori = (theOPZ*c1+arpVelunit*c2)+rspfEcefPoint(0.0,0.0,0.0);
      rspfEcefVector dir = theOPZ.cross(arpVelunit);
   
      rspfEcefVector delta = ori - arpPos;
      rspf_float64 a2 = dir.length()*dir.length();
      rspf_float64 a1 = delta.dot(dir);
      rspfEcefVector rng(ellPt-arpPos);
      rspf_float64 a0 = delta.length()*delta.length()-rng.length()*rng.length();
      rspf_float64 rootSqr = a1*a1-a0*a2;
      rspf_float64 root = sqrt(rootSqr);
      rspfEcefPoint p1 = ori - dir*((a1+root)/a2);
      rspfEcefPoint p2 = ori - dir*((a1-root)/a2);
      rspfEcefVector p1Delta = p1 - ellPt;
      rspfEcefVector p2Delta = p2 - ellPt;
      if (p1Delta.magnitude()<p2Delta.magnitude())
         selPt = p1;
      else
         selPt = p2;
      computeImageFromOP(selPt,currentImagePt);
      lTime = cTime;
      iter++;
   }
      
   opPt = selPt;
   
   return true;
}
rspfDpt rspfSarModel::getForwardDeriv(int derivMode,
                                        const rspfGpt& pos,
                                        double h)
{
   if (derivMode >= 0)
   {
      return rspfSensorModel::getForwardDeriv(derivMode, pos, h);
   }
   
   else
   {
      rspfDpt returnData;
      if (derivMode==OBS_INIT)
      {
         rspfDpt obs;
         obs.samp = pos.latd();
         obs.line = pos.lond();
         theObsTime = getArpTime(obs);
         theObsArpPos = getArpPos(theObsTime);
         theObsArpVel = getArpVel(theObsTime);
         theObsArpVel_U = theObsArpVel.unitVector();
         theObsArpVel_Mag = theObsArpVel.magnitude();
         computeOPfromImage(obs, theObsOP);
         computeRangeDoppler
            (theObsOP, theObsArpPos, theObsArpVel, theObsRng, theObsDop);
         rspfDpt obsRD(theObsRng, theObsDop);
         theObs = obsRD;
      }
      else if (derivMode==EVALUATE)
      {
         rspfEcefPoint gpos(pos);
         rspfEcefPoint ellObs = gpos;
         theObsPosition = gpos;
         rspfEcefVector rng = ellObs - theObsArpPos;
         rspfEcefVector rngU = rng.unitVector();
         rspfEcefVector slantPerp = rngU.cross(theObsArpVel);
         rspfEcefVector rs(theObsArpPos[0],theObsArpPos[1],theObsArpPos[2]);
         rspf_float64 signPar = (slantPerp.dot(rs)<0.0) ? -1:1;
         rspfEcefVector slantNormal = slantPerp.unitVector() * signPar;
         rspfEcefVector azU = slantNormal.unitVector().cross(rngU);
         rspf_float64 dca = acos(theObsArpVel_U.dot(rngU));
         signPar = ((theObsArpVel.cross(rng)).dot(slantNormal)<0.0) ? 1:-1;
         theParDopWRTaz = signPar*sin(dca)*theObsArpVel_Mag/rng.magnitude();
         theParWRTx.u = rngU.x();
         theParWRTy.u = rngU.y();
         theParWRTz.u = rngU.z();
         theParWRTx.v = azU.x()*theParDopWRTaz;
         theParWRTy.v = azU.y()*theParDopWRTaz;
         theParWRTz.v = azU.z()*theParDopWRTaz;
         rspf_float64 cRng;
         rspf_float64 cDop;
         rspfEcefPoint opPt;
         projEllipsoidToOP(ellObs, opPt);
         computeRangeDoppler(opPt, theObsArpPos, theObsArpVel, cRng, cDop);
         rspfDpt resid(theObsRng-cRng, theObsDop-cDop);
         returnData = resid;
      }
      else if (derivMode==P_WRT_X)
      {
         returnData = theParWRTx;
      }
      else if (derivMode==P_WRT_Y)
      {
         returnData = theParWRTy;
      }
      else
      {
         returnData = theParWRTz;
      }
      return returnData;
   }
}
void rspfSarModel::setAcquisitionMode(const rspfString& mode)
{
   rspfString os = mode;
   os.downcase();
   if (os == "scan")
   {
      theAcquisitionMode = rspfSarModel::SCAN;
   }
   else if (os == "spot")
   {
      theAcquisitionMode = rspfSarModel::SPOT;
   }
   else
   {
      theAcquisitionMode = rspfSarModel::UNKNOWN;
   }
}
