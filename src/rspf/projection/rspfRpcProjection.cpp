#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfRpcProjection.h>
#include <rspf/projection/rspfRpcSolver.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfTieGptSet.h>
#include <rspf/imaging/rspfImageGeometry.h>
RTTI_DEF3(rspfRpcProjection, "rspfRpcProjection", rspfProjection, rspfOptimizableProjection,
          rspfAdjustableParameterInterface);
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <rspf/matrix/newmatrc.h>
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfRpcProjection:exec");
static rspfTrace traceDebug ("rspfRpcProjection:debug");
static const int    MODEL_VERSION_NUMBER  = 1;
static const int    NUM_COEFFS        = 20;
static const char*  MODEL_TYPE        = "rspfRpcModel";
static const char*  POLY_TYPE_KW      = "polynomial_format";
static const char*  LINE_SCALE_KW     = "line_scale";
static const char*  SAMP_SCALE_KW     = "samp_scale";
static const char*  LAT_SCALE_KW      = "lat_scale";
static const char*  LON_SCALE_KW      = "long_scale";
static const char*  HGT_SCALE_KW      = "height_scale";
static const char*  LINE_OFFSET_KW    = "line_off";
static const char*  SAMP_OFFSET_KW    = "samp_off";
static const char*  LAT_OFFSET_KW     = "lat_off";
static const char*  LON_OFFSET_KW     = "long_off";
static const char*  HGT_OFFSET_KW     = "height_off";
static const char*  LINE_NUM_COEF_KW  = "line_num_coeff_";
static const char*  LINE_DEN_COEF_KW  = "line_den_coeff_";
static const char*  SAMP_NUM_COEF_KW  = "samp_num_coeff_";
static const char*  SAMP_DEN_COEF_KW  = "samp_den_coeff_";
static const rspf_int32 INTRACK_OFFSET = 0;
static const rspf_int32 CRTRACK_OFFSET = 1;
static const rspf_int32 INTRACK_SCALE  = 2;
static const rspf_int32 CRTRACK_SCALE  = 3;
static const rspf_int32 MAP_ROTATION   = 4;
static const rspf_int32 NUM_ADJUSTABLE_PARAMS = 5;
static const rspfString PARAM_NAMES[] ={"intrack_offset",
                                        "crtrack_offset",
                                        "intrack_scale",
                                        "crtrack_scale",
                                        "map_rotation",
                                        "yaw_offset"};
static const rspfString PARAM_UNITS[] ={"pixel",
                                        "pixel",
                                        "scale",
                                        "scale",
                                        "degrees",
                                        "degrees"};
rspfRpcProjection::rspfRpcProjection()
   : rspfOptimizableProjection(),
     theIntrackOffset(0),
     theCrtrackOffset(0),
     theIntrackScale(0.0),
     theCrtrackScale(0.0),
     theYawSkew   (0.0),
     theCosMapRot (1.0),
     theSinMapRot (0.0)
 {
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection Default Constructor: entering..." << std::endl;
   initAdjustableParameters();
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection Default Constructor: returning..." << std::endl;
}
rspfRpcProjection::rspfRpcProjection(const rspfRpcProjection& model)
   :
      rspfOptimizableProjection(model),
      rspfAdjustableParameterInterface(model),
      thePolyType     (model.thePolyType),
      theLineScale    (model.theLineScale),
      theSampScale    (model.theSampScale),
      theLatScale     (model.theLatScale),
      theLonScale     (model.theLonScale),
      theHgtScale     (model.theHgtScale),
      theLineOffset   (model.theLineOffset),
      theSampOffset   (model.theSampOffset),
      theLatOffset    (model.theLatOffset),
      theLonOffset    (model.theLonOffset),
      theHgtOffset    (model.theHgtOffset),
      theIntrackOffset(model.theIntrackOffset),
      theCrtrackOffset(model.theCrtrackOffset),
      theIntrackScale(model.theIntrackScale),
      theCrtrackScale(model.theCrtrackScale),
      theYawSkew      (model.theYawSkew),
      theCosMapRot    (model.theCosMapRot),
      theSinMapRot    (model.theSinMapRot)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection Copy Constructor: entering..." << std::endl;
   for (int i=0; i<20; i++)
   {
      theLineNumCoef[i] = model.theLineNumCoef[i];
      theLineDenCoef[i] = model.theLineDenCoef[i];
      theSampNumCoef[i] = model.theSampNumCoef[i];
      theSampDenCoef[i] = model.theSampDenCoef[i];
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection Copy Constructor: returning..." << std::endl;
}
rspfRpcProjection::~rspfRpcProjection()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG ~rspfRpcProjection() Destructor: entering..." << std::endl;
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNotify(rspfNotifyLevel_DEBUG): returning..." << std::endl;
}
rspfObject* rspfRpcProjection::getBaseObject()
{
   return this;
}
const rspfObject* rspfRpcProjection::getBaseObject()const
{
   return this;
}
rspfRpcProjection& 
rspfRpcProjection::operator=(const rspfRpcProjection& source)
{
   if (this != &source)
   {
      rspfOptimizableProjection::operator=(source);
      thePolyType    = source.thePolyType;
      theLineScale   = source.theLineScale;
      theSampScale   = source.theSampScale;
      theLatScale    = source.theLatScale;
      theLonScale    = source.theLonScale;
      theHgtScale    = source.theHgtScale;
      theLineOffset  = source.theLineOffset;
      theSampOffset  = source.theSampOffset;
      theLatOffset   = source.theLatOffset;
      theLonOffset   = source.theLonOffset;
      theHgtOffset   = source.theHgtOffset;
      for (int i=0; i<20; i++)
      {
         theLineNumCoef[i] = source.theLineNumCoef[i];
         theLineDenCoef[i] = source.theLineDenCoef[i];
         theSampNumCoef[i] = source.theSampNumCoef[i];
         theSampDenCoef[i] = source.theSampDenCoef[i];
      }
   }
   return *this;
}
void rspfRpcProjection::setAttributes(rspf_float64 sampleOffset,
                                       rspf_float64 lineOffset,
                                       rspf_float64 sampleScale,
                                       rspf_float64 lineScale,
                                       rspf_float64 latOffset,
                                       rspf_float64 lonOffset,
                                       rspf_float64 heightOffset,
                                       rspf_float64 latScale,
                                       rspf_float64 lonScale,
                                       rspf_float64 heightScale,
                                       const std::vector<double>& xNumeratorCoeffs,
                                       const std::vector<double>& xDenominatorCoeffs,
                                       const std::vector<double>& yNumeratorCoeffs,
                                       const std::vector<double>& yDenominatorCoeffs,
                                       PolynomialType polyType)
{
   thePolyType = polyType;
   
   theLineScale  = lineScale;
   theSampScale  = sampleScale;
   theLatScale   = latScale;
   theLonScale   = lonScale;
   theHgtScale   = heightScale;
   theLineOffset = lineOffset;
   theSampOffset = sampleOffset;
   theLatOffset  = latOffset;
   theLonOffset  = lonOffset;
   theHgtOffset  = heightOffset;
   if(xNumeratorCoeffs.size() == 20)
   {
      std::copy(xNumeratorCoeffs.begin(),
                xNumeratorCoeffs.end(),
                theSampNumCoef);
   }
   if(xDenominatorCoeffs.size() == 20)
   {
      std::copy(xDenominatorCoeffs.begin(),
                xDenominatorCoeffs.end(),
                theSampDenCoef);
   }
   if(yNumeratorCoeffs.size() == 20)
   {
      std::copy(yNumeratorCoeffs.begin(),
                yNumeratorCoeffs.end(),
                theLineNumCoef);
   }
   if(yDenominatorCoeffs.size() == 20)
   {
      std::copy(yDenominatorCoeffs.begin(),
                yDenominatorCoeffs.end(),
                theLineDenCoef);
   }
}
void rspfRpcProjection::worldToLineSample(const rspfGpt& ground_point,
                                      rspfDpt&       imgPt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::worldToLineSample(): entering..." << std::endl;
   if(ground_point.isLatNan() ||
      ground_point.isLonNan() )
     {
       imgPt.makeNan();
       return;
     }
         
   double nlat = (ground_point.lat - theLatOffset) / theLatScale;
   double nlon = (ground_point.lon - theLonOffset) / theLonScale;
   double nhgt;
   if(rspf::isnan(ground_point.hgt))
   {
      nhgt = (theHgtScale - theHgtOffset) / theHgtScale;
   }
   else
   {
      nhgt = (ground_point.hgt - theHgtOffset) / theHgtScale;
   }
   
   double Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
   double Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
   double Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
   double Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
   double U_rot  = Pu / Qu;
   double V_rot  = Pv / Qv;
   double U = U_rot*theCosMapRot + V_rot*theSinMapRot;
   double V = V_rot*theCosMapRot - U_rot*theSinMapRot;
   imgPt.line = U*(theLineScale+theIntrackScale) + theLineOffset + theIntrackOffset;
   imgPt.samp = V*(theSampScale+theCrtrackScale) + theSampOffset + theCrtrackOffset;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::worldToLineSample(): returning..." << std::endl;
   return;
}
void  rspfRpcProjection::lineSampleToWorld(const rspfDpt& imagePoint,
                                            rspfGpt&       worldPoint) const
{
   if(!imagePoint.hasNans())
   {
      
      lineSampleHeightToWorld(imagePoint,
                              worldPoint.height(),
                              worldPoint);
   }
   else
   {
      worldPoint.makeNan();
   }
}
void rspfRpcProjection::lineSampleHeightToWorld(const rspfDpt& image_point,
                                            const double&   ellHeight,
                                            rspfGpt&       gpt) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::lineSampleHeightToWorld: entering..." << std::endl;
   static const int    MAX_NUM_ITERATIONS  = 10;
   static const double CONVERGENCE_EPSILON = 0.1;  // pixels
   
   double skew = (image_point.x-theSampOffset - theCrtrackOffset)*theYawSkew;
   double U    = (image_point.y-theLineOffset - theIntrackOffset+skew) / (theLineScale+theIntrackScale);
   double V    = (image_point.x-theSampOffset - theCrtrackOffset) / (theSampScale+theCrtrackScale);
   double U_rot = theCosMapRot*U - theSinMapRot*V;
   double V_rot = theSinMapRot*U + theCosMapRot*V;
   U = U_rot; V = V_rot;
   double nlat      = 0.0;  // normalized latitude
   double nlon      = 0.0;  // normalized longitude
   
   double nhgt;
   if(rspf::isnan(ellHeight))
   {
     nhgt = (theHgtScale - theHgtOffset) / theHgtScale;  // norm height
   }
   else
   {
      nhgt = (ellHeight - theHgtOffset) / theHgtScale;  // norm height
   }
   
   double epsilonU = CONVERGENCE_EPSILON/(theLineScale+theIntrackScale);
   double epsilonV = CONVERGENCE_EPSILON/(theSampScale+theCrtrackScale);
   int    iteration = 0;
   double Pu, Qu, Pv, Qv;
   double dPu_dLat, dQu_dLat, dPv_dLat, dQv_dLat;
   double dPu_dLon, dQu_dLon, dPv_dLon, dQv_dLon;
   double Uc, Vc;
   double deltaU, deltaV;
   double dU_dLat, dU_dLon, dV_dLat, dV_dLon, W;
   double deltaLat, deltaLon;
   
   do
   {
      Pu = polynomial(nlat, nlon, nhgt, theLineNumCoef);
      Qu = polynomial(nlat, nlon, nhgt, theLineDenCoef);
      Pv = polynomial(nlat, nlon, nhgt, theSampNumCoef);
      Qv = polynomial(nlat, nlon, nhgt, theSampDenCoef);
      Uc = Pu/Qu;
      Vc = Pv/Qv;
      
      deltaU = U - Uc;
      deltaV = V - Vc;
      
      if ((fabs(deltaU) > epsilonU) || (fabs(deltaV) > epsilonV))
      {
         dPu_dLat = dPoly_dLat(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dLat = dPoly_dLat(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dLat = dPoly_dLat(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dLat = dPoly_dLat(nlat, nlon, nhgt, theSampDenCoef);
         dPu_dLon = dPoly_dLon(nlat, nlon, nhgt, theLineNumCoef);
         dQu_dLon = dPoly_dLon(nlat, nlon, nhgt, theLineDenCoef);
         dPv_dLon = dPoly_dLon(nlat, nlon, nhgt, theSampNumCoef);
         dQv_dLon = dPoly_dLon(nlat, nlon, nhgt, theSampDenCoef);
         
         dU_dLat = (Qu*dPu_dLat - Pu*dQu_dLat)/(Qu*Qu);
         dU_dLon = (Qu*dPu_dLon - Pu*dQu_dLon)/(Qu*Qu);
         dV_dLat = (Qv*dPv_dLat - Pv*dQv_dLat)/(Qv*Qv);
         dV_dLon = (Qv*dPv_dLon - Pv*dQv_dLon)/(Qv*Qv);
         
         W = dU_dLon*dV_dLat - dU_dLat*dV_dLon;
         
         deltaLat = (dU_dLon*deltaV - dV_dLon*deltaU) / W;
         deltaLon = (dV_dLat*deltaU - dU_dLat*deltaV) / W;
         nlat += deltaLat;
         nlon += deltaLon;
      }
      
      iteration++;
      
   } while (((fabs(deltaU)>epsilonU) || (fabs(deltaV)>epsilonV))
            && (iteration < MAX_NUM_ITERATIONS));
      
   if (iteration == MAX_NUM_ITERATIONS)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfRpcProjection::lineSampleHeightToWorld: \nMax number of iterations reached in ground point "
                                         << "solution. Results are inaccurate." << endl;
   }
   gpt.lat = nlat*theLatScale + theLatOffset;
   gpt.lon = nlon*theLonScale + theLonOffset;
   gpt.hgt = ellHeight;
   
}
rspfGpt rspfRpcProjection::origin()const
{
   return rspfGpt(theLatOffset,
                   theLonOffset,
                   theHgtOffset);
}
rspfDpt rspfRpcProjection::getMetersPerPixel() const
{
   rspfDpt result;
   
   rspfDpt top  = rspfDpt(theSampOffset,
                            theLineOffset-1);
   rspfDpt bottom = rspfDpt(theSampOffset,
                              theLineOffset+1);
   rspfGpt topG;
   rspfGpt bottomG;
   
   lineSampleToWorld(top, topG);
   lineSampleToWorld(bottom, bottomG);
   
   result.y = (rspfEcefPoint(topG) - rspfEcefPoint(bottomG)).magnitude()/2.0;
   result.x = result.y;
   return result;
}
bool rspfRpcProjection::operator==(const rspfProjection& projection) const
{
   if(&projection == this) return true;
   return false;
}
double rspfRpcProjection::polynomial(const double& P, const double& L,
                                      const double& H, const double* c) const
{
   double r;
   if (thePolyType == A)
   {
      r = c[ 0]       + c[ 1]*L     + c[ 2]*P     + c[ 3]*H     +
          c[ 4]*L*P   + c[ 5]*L*H   + c[ 6]*P*H   + c[ 7]*L*P*H +
          c[ 8]*L*L   + c[ 9]*P*P   + c[10]*H*H   + c[11]*L*L*L +
          c[12]*L*L*P + c[13]*L*L*H + c[14]*L*P*P + c[15]*P*P*P +
          c[16]*P*P*H + c[17]*L*H*H + c[18]*P*H*H + c[19]*H*H*H;
   }
   else
   {
      r = c[ 0]       + c[ 1]*L     + c[ 2]*P     + c[ 3]*H     +
          c[ 4]*L*P   + c[ 5]*L*H   + c[ 6]*P*H   + c[ 7]*L*L   +
          c[ 8]*P*P   + c[ 9]*H*H   + c[10]*L*P*H + c[11]*L*L*L +
          c[12]*L*P*P + c[13]*L*H*H + c[14]*L*L*P + c[15]*P*P*P +
          c[16]*P*H*H + c[17]*L*L*H + c[18]*P*P*H + c[19]*H*H*H;
   }
   
   return r;
}
double rspfRpcProjection::dPoly_dLat(const double& P, const double& L,
                                 const double& H, const double* c) const
{
   double dr;
   if (thePolyType == A)
   {
      dr = c[2] + c[4]*L + c[6]*H + c[7]*L*H + 2*c[9]*P + c[12]*L*L +
           2*c[14]*L*P + 3*c[15]*P*P +2*c[16]*P*H + c[18]*H*H;
   }
   else
   {
      dr = c[2] + c[4]*L + c[6]*H + 2*c[8]*P + c[10]*L*H + 2*c[12]*L*P +
           c[14]*L*L + 3*c[15]*P*P + c[16]*H*H + 2*c[18]*P*H;
   }
   
   return dr;
}
double rspfRpcProjection::dPoly_dLon(const double& P, const double& L,
                                 const double& H, const double* c) const
{
   double dr;
   if (thePolyType == A)
   {
      dr = c[1] + c[4]*P + c[5]*H + c[7]*P*H + 2*c[8]*L + 3*c[11]*L*L +
           2*c[12]*L*P + 2*c[13]*L*H + c[14]*P*P + c[17]*H*H;
   }
   else
   {
      dr = c[1] + c[4]*P + c[5]*H + 2*c[7]*L + c[10]*P*H + 3*c[11]*L*L +
           c[12]*P*P + c[13]*H*H + 2*c[14]*P*L + 2*c[17]*L*H;
   }
   return dr;
}
std::ostream& rspfRpcProjection::print(std::ostream& out) const
{
   out << "\nDump of rspfRpcProjection object at " << hex << this << ":\n"
       << POLY_TYPE_KW   << ": " << thePolyType   << "\n"
       << LINE_SCALE_KW  << ": " << theLineScale  << "\n"
       << SAMP_SCALE_KW  << ": " << theSampScale  << "\n"
       << LAT_SCALE_KW   << ": " << theLatScale   << "\n"
       << LON_SCALE_KW   << ": " << theLonScale   << "\n"
       << HGT_SCALE_KW   << ": " << theHgtScale   << "\n"
       << LINE_OFFSET_KW << ": " << theLineOffset << "\n"
       << SAMP_OFFSET_KW << ": " << theSampOffset << "\n"
       << LAT_OFFSET_KW  << ": " << theLatOffset  << "\n"
       << LON_OFFSET_KW  << ": " << theLonOffset  << "\n"
       << HGT_OFFSET_KW  << ": " << theHgtOffset  << endl;
   for (int i=0; i<NUM_COEFFS; i++)
      out<<"  "<<LINE_NUM_COEF_KW<<"["<<i<<"]: "<<theLineNumCoef[i]<<endl;
   out << endl;
   for (int i=0; i<NUM_COEFFS; i++)
      out<<"  "<<LINE_DEN_COEF_KW<<"["<<i<<"]: "<<theLineDenCoef[i]<<endl;
   out << endl;
   for (int i=0; i<NUM_COEFFS; i++)
      out<<"  "<<SAMP_NUM_COEF_KW<<"["<<i<<"]: "<<theSampNumCoef[i]<<endl;
   out << endl;
   for (int i=0; i<NUM_COEFFS; i++)
      out<<"  "<<SAMP_DEN_COEF_KW<<"["<<i<<"]: "<<theSampDenCoef[i]<<endl;
      
   out << endl;
   return rspfProjection::print(out);
}
bool rspfRpcProjection::saveState(rspfKeywordlist& kwl,
                              const char* prefix) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::saveState(): entering..." << std::endl;
   kwl.add(prefix, rspfKeywordNames::TYPE_KW, MODEL_TYPE);
   rspfProjection::saveState(kwl, prefix);
   kwl.add(prefix,
           POLY_TYPE_KW,
           (char)thePolyType,
           true);
   
   kwl.add(prefix, LINE_SCALE_KW, theLineScale);
   kwl.add(prefix, SAMP_SCALE_KW, theSampScale);
   kwl.add(prefix, LAT_SCALE_KW, theLatScale);
   kwl.add(prefix, LON_SCALE_KW, theLonScale);
   kwl.add(prefix, HGT_SCALE_KW, theHgtScale);
   kwl.add(prefix, LINE_OFFSET_KW, theLineOffset);
   kwl.add(prefix, SAMP_OFFSET_KW, theSampOffset);
   kwl.add(prefix, LAT_OFFSET_KW, theLatOffset);
   kwl.add(prefix, LON_OFFSET_KW, theLonOffset);
   
   kwl.add(prefix, HGT_OFFSET_KW, theHgtOffset);
   for (int i=0; i<NUM_COEFFS; i++)
   {
      kwl.add(prefix, (LINE_NUM_COEF_KW + rspfString::toString(i)).c_str(), theLineNumCoef[i]);
      kwl.add(prefix, (LINE_DEN_COEF_KW + rspfString::toString(i)).c_str(), theLineDenCoef[i]);
      kwl.add(prefix, (SAMP_NUM_COEF_KW + rspfString::toString(i)).c_str(), theSampNumCoef[i]);
      kwl.add(prefix, (SAMP_DEN_COEF_KW + rspfString::toString(i)).c_str(), theSampDenCoef[i]);
   }
      
   saveAdjustments(kwl, prefix);
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::saveState(): returning..." << std::endl;
   return true;
}
bool rspfRpcProjection::loadState(const rspfKeywordlist& kwl,
                              const char* prefix) 
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::loadState(): entering..." << std::endl;
   const char* value;
   const char* keyword;
   bool success = rspfProjection::loadState(kwl, prefix);
   if (!success)
   {
      theErrorStatus++;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::loadState(): returning with error..." << std::endl;
      return false;
   }
      
   keyword = POLY_TYPE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   thePolyType = (PolynomialType) value[0];
      
   keyword = LINE_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLineScale = rspfString(value).toDouble();
   
   keyword = SAMP_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theSampScale = rspfString(value).toDouble();
   
   keyword = LAT_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLatScale = rspfString(value).toDouble();
   
   keyword = LON_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLonScale = rspfString(value).toDouble();
   
   keyword = HGT_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theHgtScale = rspfString(value).toDouble();
   
   keyword = LINE_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLineOffset = rspfString(value).toDouble();
   
   keyword = SAMP_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theSampOffset = rspfString(value).toDouble();
   
   keyword = LAT_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLatOffset = rspfString(value).toDouble();
   
   keyword = LON_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLonOffset = rspfString(value).toDouble();
   
   keyword = HGT_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theHgtOffset = rspfString(value).toDouble();
   for (int i=0; i<NUM_COEFFS; i++)
   {
      value = kwl.find(prefix, (LINE_NUM_COEF_KW+rspfString::toString(i)).c_str());
      if (!value)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                             << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                             << std::endl;
         return false;
      }
      theLineNumCoef[i] = rspfString(value).toDouble();
   
      value = kwl.find(prefix, (LINE_DEN_COEF_KW+rspfString::toString(i)).c_str());
      if (!value)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                             << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                             << std::endl;
         return false;
      }
      theLineDenCoef[i] = rspfString(value).toDouble();
   
      value = kwl.find(prefix, (SAMP_NUM_COEF_KW+rspfString::toString(i)).c_str());
      if (!value)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                             << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                             << std::endl;
         return false;
      }
      theSampNumCoef[i] = rspfString(value).toDouble();
      
      value = kwl.find(prefix, (SAMP_DEN_COEF_KW+rspfString::toString(i)).c_str());
      if (!value)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::loadState(): Error encountered parsing the following required keyword: "
                                             << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                             << std::endl;
         return false;
      }
      theSampDenCoef[i] = rspfString(value).toDouble();
   }
   loadAdjustments(kwl, prefix);
   if(getNumberOfAdjustableParameters() < 1)
   {
      initAdjustableParameters();
   }
      
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfRpcProjection::loadState(): returning..." << std::endl;
   return true;
}
void rspfRpcProjection::initAdjustableParameters()
{
   resizeAdjustableParameterArray(NUM_ADJUSTABLE_PARAMS);
   int numParams = getNumberOfAdjustableParameters();
   for (int i=0; i<numParams; i++)
   {
      setAdjustableParameter(i, 0.0);
      setParameterDescription(i, PARAM_NAMES[i]);
      setParameterUnit(i,PARAM_UNITS[i]);
   }
   setParameterSigma(INTRACK_OFFSET, 50.0);
   setParameterSigma(CRTRACK_OFFSET, 50.0);
   setParameterSigma(INTRACK_SCALE, 50.0);  
   setParameterSigma(CRTRACK_SCALE, 50.0);  
   setParameterSigma(MAP_ROTATION, 0.1);
}
void rspfRpcProjection::adjustableParametersChanged()
{
   theIntrackOffset    = computeParameterOffset(INTRACK_OFFSET);
   theCrtrackOffset    = computeParameterOffset(CRTRACK_OFFSET);
   theIntrackScale     = computeParameterOffset(INTRACK_SCALE);
   theCrtrackScale     = computeParameterOffset(CRTRACK_SCALE);
   double mapRotation  = computeParameterOffset(MAP_ROTATION);
   theCosMapRot        = rspf::cosd(mapRotation);
   theSinMapRot        = rspf::sind(mapRotation);
}
bool
rspfRpcProjection::setupOptimizer(const rspfString& setup)
{
   rspfKeywordlist kwl;
   if(kwl.addFile(rspfFilename(setup)))
   {
      return loadState(kwl);
   }
   else
   {
      rspfRefPtr<rspfProjection> proj = rspfProjectionFactoryRegistry::instance()->createProjection(setup);
      if(proj.valid())
      {
         kwl.clear();
         proj->saveState(kwl);
         
         return loadState(kwl);
      }
   }
   
   return false;
}
rspf_uint32
rspfRpcProjection::degreesOfFreedom()const
{
   rspf_uint32 dof = 0;
   rspf_uint32 idx = 0;
   rspf_uint32 numAdj = getNumberOfAdjustableParameters();
   for(idx = 0; idx < numAdj; ++idx)
   {
      if(!isParameterLocked(idx))
      {
         ++dof;
      }
   }
   
   return dof;
}
rspfGpt
rspfRpcProjection::getInverseDeriv(int parmIdx, const rspfDpt& ipos, double hdelta)
{   
   double den = 0.5/hdelta;
   rspfGpt res,gd;
   double middle = getAdjustableParameter(parmIdx);
   setAdjustableParameter(parmIdx, middle + hdelta, true);
   res = inverse(ipos);
   setAdjustableParameter(parmIdx, middle - hdelta, true);
   gd = inverse(ipos);
   setAdjustableParameter(parmIdx, middle, true);
   res.lon = den*(res.lon - gd.lon) * 100000.0; //TBC : approx meters
   res.lat = den*(res.lat - gd.lat) * 100000.0 * cos(gd.lat / 180.0 * M_PI);
   res.hgt = den*(res.hgt - gd.hgt);
   return res;
}
rspfDpt
rspfRpcProjection::getForwardDeriv(int parmIdx, const rspfGpt& gpos, double hdelta)
{   
   static double den = 0.5/hdelta;
   rspfDpt res;
   double middle = getAdjustableParameter(parmIdx);
   setAdjustableParameter(parmIdx, middle + hdelta, true);
   res = inverse(gpos);
   setAdjustableParameter(parmIdx, middle - hdelta, true);
   res -= inverse(gpos);
   res = res*den;
   setAdjustableParameter(parmIdx, middle, true);
   return res;
}
double
rspfRpcProjection::optimizeFit(const rspfTieGptSet& tieSet, double* /* targetVariance */)
{
#if 1
   rspfRefPtr<rspfRpcSolver> solver = new rspfRpcSolver(false, false); //TBD : choices should be part of setupFromString
   std::vector<rspfDpt> imagePoints;
   std::vector<rspfGpt> groundPoints;
   tieSet.getSlaveMasterPoints(imagePoints, groundPoints);
   solver->solveCoefficients(imagePoints, groundPoints);
   rspfRefPtr< rspfImageGeometry > optProj = solver->createRpcProjection();
   if (!optProj)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfRpcProjection::optimizeFit(): error when optimizing the RPC with given tie points"
                                             << std::endl;
      return -1.0;
   }
   if(optProj->hasProjection())
   {
      rspfKeywordlist kwl;
      optProj->getProjection()->saveState(kwl);
      this->loadState(kwl);
   }
   return std::pow(solver->getRmsError(), 2); //variance in pixel^2
#else
 
   int np = getNumberOfAdjustableParameters();
   int nobs = tieSet.size();
   int iter=0;
   int iter_max = 200;
   double minResidue = 1e-10; //TBC
   double minDelta = 1e-10; //TBC
   NEWMAT::SymmetricMatrix A;
   NEWMAT::ColumnVector residue;
   NEWMAT::ColumnVector projResidue;
   double deltap_scale = 1e-4; //step_Scale is 1.0 because we expect parameters to be between -1 and 1
   buildNormalEquation(tieSet, A, residue, projResidue, deltap_scale);
   double ki2=residue.SumSquare();
   rspfAdjustmentInfo cadj;
   getAdjustment(cadj);
   std::vector< rspfAdjustableParameterInfo >& parmlist = cadj.getParameterList();
   NEWMAT::ColumnVector cparm(np), nparm(np);
   for(int n=0;n<np;++n)
   {
      cparm(n+1) = parmlist[n].getParameter();
   }
   double damping_speed = 2.0;
   double maxdiag=0.0;
   for(int d=1;d<=np;++d) {
      if (maxdiag < A(d,d)) maxdiag=A(d,d);
   }
   double damping = 1e-3 * maxdiag;
   double olddamping = 0.0;
   bool found = false;
cout<<"rms="<<sqrt(ki2/nobs)<<" ";
cout.flush();
   while ( (!found) && (iter < iter_max) ) //non linear optimization loop
   {
      bool decrease = false;
      do
      {
         for(int d=1;d<=np;++d) A(d,d) += damping - olddamping;
         olddamping = damping;
         NEWMAT::ColumnVector deltap = solveLeastSquares(A, projResidue);
         if (deltap.NormFrobenius() <= minDelta) 
         {
            found = true;
         } else {
            nparm = cparm + deltap;
            for(int n=0;n<np;++n)
            {
               setAdjustableParameter(n, nparm(n+1), false); //do not update now, wait
            }
            adjustableParametersChanged();
            NEWMAT::ColumnVector newresidue = getResidue(tieSet);
            double newki2=newresidue.SumSquare();
            double res_reduction = (ki2 - newki2) / (deltap.t()*(deltap*damping + projResidue)).AsScalar();
       cout<<sqrt(newki2/nobs)<<" ";
       cout.flush();
            if (res_reduction > 0)
            {
               cparm = nparm;
               ki2=newki2;
               deltap_scale = max(1e-15, deltap.NormInfinity()*1e-4);
               buildNormalEquation(tieSet, A, residue, projResidue, deltap_scale);
               olddamping = 0.0;
               found = ( projResidue.NormInfinity() <= minResidue );
               damping *= std::max( 1.0/3.0, 1.0-std::pow((2.0*res_reduction-1.0),3));
               damping_speed = 2.0;
               decrease = true;
            } else {
               for(int n=0;n<np;++n)
               {
                  setAdjustableParameter(n, nparm(n+1), false); //do not update right now
               }
               adjustableParametersChanged();
               damping *= damping_speed;
               damping_speed *= 2.0;
            }
         }
      } while (!decrease && !found);
      ++iter;
   }
cout<<endl;
   return ki2/nobs;
#endif
}
void
rspfRpcProjection::buildNormalEquation(const rspfTieGptSet& tieSet,
                                      NEWMAT::SymmetricMatrix& A,
                                      NEWMAT::ColumnVector& residue,
                                      NEWMAT::ColumnVector& projResidue,
                                      double pstep_scale)
{
   int np = getNumberOfAdjustableParameters();
   int dimObs;
   bool useImageObs = useForward(); //caching
   if (useImageObs)
   {
      dimObs = 2; //image observation
   } else {
      dimObs = 3; //ground observations
   }
   int no = dimObs * tieSet.size(); //number of observations
   A.ReSize(np);
   residue.ReSize(no);
   projResidue.ReSize(np);
   A           = 0.0;
   projResidue = 0.0;
   const vector<rspfRefPtr<rspfTieGpt> >& theTPV = tieSet.getTiePoints();
   vector<rspfRefPtr<rspfTieGpt> >::const_iterator tit;
   unsigned long c=1;
   if (useImageObs)
   { 
     rspfDpt* imDerp = new rspfDpt[np];
     rspfDpt resIm;
      for (tit = theTPV.begin() ; tit != theTPV.end() ; ++tit)
      {
         resIm = (*tit)->tie - forward(*(*tit));
         residue(c++) = resIm.x;
         residue(c++) = resIm.y;
         for(int p=0;p<np;++p)
         {
            imDerp[p] = getForwardDeriv( p , *(*tit) , pstep_scale);
         }
         for(int p1=0;p1<np;++p1)
         {        
            projResidue.element(p1) += imDerp[p1].x * resIm.x + imDerp[p1].y * resIm.y;
            for(int p2=p1;p2<np;++p2)
            {
               A.element(p1,p2) += imDerp[p1].x * imDerp[p2].x + imDerp[p1].y * imDerp[p2].y;
            }
         }
      }
      delete []imDerp;
   }
   else
   {
      std::vector<rspfGpt> gdDerp(np);
      rspfGpt gd, resGd;
      for (tit = theTPV.begin() ; tit != theTPV.end() ; ++tit)
      {
         gd = inverse((*tit)->tie);
         residue(c++) = resGd.lon = ((*tit)->lon - gd.lon) * 100000.0;
         residue(c++) = resGd.lat = ((*tit)->lat - gd.lat) * 100000.0 * cos(gd.lat / 180.0 * M_PI);
         residue(c++) = resGd.hgt = (*tit)->hgt - gd.hgt; //TBD : normalize to meters?
         for(int p=0;p<np;++p)
         {
            gdDerp[p] = getInverseDeriv( p , (*tit)->tie, pstep_scale);
         }
         for(int p1=0;p1<np;++p1)
         {        
            projResidue.element(p1) += gdDerp[p1].lon * resGd.lon + gdDerp[p1].lat * resGd.lat + gdDerp[p1].hgt * resGd.hgt; //TBC
            for(int p2=p1;p2<np;++p2)
            {
               A.element(p1,p2) += gdDerp[p1].lon * gdDerp[p2].lon + gdDerp[p1].lat * gdDerp[p2].lat + gdDerp[p1].hgt * gdDerp[p2].hgt;
            }
         }
      }
   } //end of if (useImageObs)
}
NEWMAT::ColumnVector
rspfRpcProjection::getResidue(const rspfTieGptSet& tieSet)
{
   NEWMAT::ColumnVector residue;
   int dimObs;
   bool useImageObs = useForward(); //caching
   if (useImageObs)
   {
      dimObs = 2; //image observation
   } else {
      dimObs = 3; //ground observations
   }
   int no = dimObs * tieSet.size(); //number of observations
   residue.ReSize(no);
   const vector<rspfRefPtr<rspfTieGpt> >& theTPV = tieSet.getTiePoints();
   vector<rspfRefPtr<rspfTieGpt> >::const_iterator tit;
   unsigned long c=1;
   if (useImageObs)
   { 
     rspfDpt resIm;
      for (tit = theTPV.begin() ; tit != theTPV.end() ; ++tit)
      {
         resIm = (*tit)->tie - forward(**tit);
         residue(c++) = resIm.x;
         residue(c++) = resIm.y;
      }
   } else {
      rspfGpt gd;
      for (tit = theTPV.begin() ; tit != theTPV.end() ; ++tit)
      {
         gd = inverse((*tit)->tie);
         residue(c++) = ((*tit)->lon - gd.lon) * 100000.0; //approx meters //TBC TBD
         residue(c++) = ((*tit)->lat - gd.lat) * 100000.0 * cos(gd.lat / 180.0 * M_PI);
         residue(c++) = (*tit)->hgt - gd.hgt; //meters
      }
   } //end of if (useImageObs)
   return residue;
}
/*!
 * solves Ax = r , with A symmetric positive definite
 * A can be rank deficient
 * size of A is typically between 10 and 100 rows
 */
NEWMAT::ColumnVector 
rspfRpcProjection::solveLeastSquares(NEWMAT::SymmetricMatrix& A,  NEWMAT::ColumnVector& r)const
{
   NEWMAT::ColumnVector x = invert(A)*r;
   return x;
}
/** 
 * stable invert stolen from rspfRpcSolver
 */
NEWMAT::Matrix 
rspfRpcProjection::invert(const NEWMAT::Matrix& m)const
{
   rspf_uint32 idx = 0;
   NEWMAT::DiagonalMatrix d;
   NEWMAT::Matrix u;
   NEWMAT::Matrix v;
   NEWMAT::SVD(m, d, u, v, true, true);
   
   for(idx=0; idx < (rspf_uint32)d.Ncols(); ++idx)
   {
      if(d[idx] > 1e-14) //TBC : use DBL_EPSILON ?
      {
         d[idx] = 1.0/d[idx];
      }
      else
      {
         d[idx] = 0.0;
cout<<"warning: singular matrix in SVD"<<endl;
      }
   }
   return v*d*u.t();
}
