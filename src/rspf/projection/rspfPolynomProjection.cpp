#include <rspf/projection/rspfPolynomProjection.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
RTTI_DEF2(rspfPolynomProjection, "rspfPolynomProjection", rspfProjection, rspfOptimizableProjection);
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTieGptSet.h>
#include <rspf/base/rspfString.h>
#include <iostream>
#include <sstream>
#include <float.h>
static const char* MODEL_TYPE        = "rspfPolynomProjection";
static const char* LAT_SCALE_KW      = "lat_scale";
static const char* LON_SCALE_KW      = "long_scale";
static const char* HGT_SCALE_KW      = "height_scale";
static const char* LAT_OFFSET_KW     = "lat_off";
static const char* LON_OFFSET_KW     = "long_off";
static const char* HGT_OFFSET_KW     = "height_off";
static const char* POLY_LINE_KW      = "poly_line";
static const char* POLY_SAMP_KW      = "poly_samp";
rspfPolynomProjection::rspfPolynomProjection()
   :
   rspfOptimizableProjection(),
   theLonOffset(0.0),
   theLonScale(1.0),
   theLatOffset(0.0),
   theLatScale(1.0),
   theHgtOffset(0.0),
   theHgtScale(1e-5)
 {}
rspfPolynomProjection::rspfPolynomProjection(const rspfPolynomProjection& model)
   :
   rspfOptimizableProjection(model),
   thePolyLine(model.thePolySamp),
   thePolySamp(model.thePolyLine),
   thePolyLine_DLon(model.thePolyLine_DLon),
   thePolyLine_DLat(model.thePolyLine_DLat),
   thePolySamp_DLon(model.thePolySamp_DLon),
   thePolySamp_DLat(model.thePolySamp_DLat),
   theLonOffset(model.theLonOffset),
   theLonScale(model.theLonScale),
   theLatOffset(model.theLatOffset),
   theLatScale(model.theLatScale),
   theHgtOffset(model.theHgtOffset),
   theHgtScale(model.theHgtScale)
{
}
rspfPolynomProjection::~rspfPolynomProjection()
{}
void 
rspfPolynomProjection::worldToLineSample(const rspfGpt& ground_point,
                                          rspfDpt&       imgPt)const
{
   if(ground_point.isLatNan() || ground_point.isLonNan() )
     {
       imgPt.makeNan();
       return;
     }
   vector<double> gpt(3);
   gpt[0] = (ground_point.lon - theLonOffset) * theLonScale;
   gpt[1] = (ground_point.lat - theLatOffset) * theLatScale;
   
   if(rspf::isnan(ground_point.hgt)||rspf::almostEqual(theHgtOffset, 0.0))
   {
      gpt[2] = 0.0;
   }
   else
   {
      gpt[2] = (ground_point.hgt - theHgtOffset) * theHgtScale;
   }
   
   imgPt.x = thePolySamp.eval(gpt);
   imgPt.y = thePolyLine.eval(gpt);
   return;
}
void  
rspfPolynomProjection::lineSampleToWorld(const rspfDpt& imagePoint,
                                          rspfGpt&       worldPoint)const
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
void rspfPolynomProjection::lineSampleHeightToWorld(const rspfDpt& image_point,
                                            const double&   ellHeight,
                                            rspfGpt&       gpt) const
{
   static const int    MAX_NUM_ITERATIONS  = 10;
   static const double CONVERGENCE_EPSILON = 0.1;  // pixels
   
   double U    = image_point.y;
   double V    = image_point.x;
   vector<double> ngpt(3);
   ngpt[0] = 0.0; //normalized longitude (center)
   ngpt[1] = 0.0; //normalized latitude
   if(rspf::isnan(ellHeight)||rspf::almostEqual(theHgtOffset, 0.0))
   {
      ngpt[2] = 0;  // norm height
   }
   else
   {
      ngpt[2] = (ellHeight - theHgtOffset) * theHgtScale;  // norm height
   }
   
   double epsilonU = CONVERGENCE_EPSILON;
   double epsilonV = CONVERGENCE_EPSILON;
   int    iteration = 0;
   double Uc, Vc;
   double deltaU, deltaV;
   double dU_dLat, dU_dLon, dV_dLat, dV_dLon, W;
   double deltaLat, deltaLon;
   
   do
   {
      Uc = thePolyLine.eval(ngpt); //TBC
      Vc = thePolySamp.eval(ngpt); //TBC
      
      deltaU = U - Uc;
      deltaV = V - Vc;
      
      if ((fabs(deltaU) > epsilonU) || (fabs(deltaV) > epsilonV))
      {
         dU_dLat = thePolyLine_DLat.eval(ngpt);
         dU_dLon = thePolyLine_DLon.eval(ngpt);
         dV_dLat = thePolySamp_DLat.eval(ngpt);
         dV_dLon = thePolySamp_DLon.eval(ngpt);
         
         W = dU_dLon*dV_dLat - dU_dLat*dV_dLon;
         
         deltaLat = (dU_dLon*deltaV - dV_dLon*deltaU) / W;
         deltaLon = (dV_dLat*deltaU - dU_dLat*deltaV) / W;
         ngpt[0] += deltaLon;
         ngpt[1] += deltaLat;
      }
      
      iteration++;
      
   } while (((fabs(deltaU)>epsilonU) || (fabs(deltaV)>epsilonV))
            && (iteration < MAX_NUM_ITERATIONS));
      
   if (iteration == MAX_NUM_ITERATIONS)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfPolynomProjection::lineSampleHeightToWorld: \nMax number of iterations reached in ground point "
                                         << "solution. Results are inaccurate." << endl;
   }
   gpt.lat = ngpt[1] / theLatScale + theLatOffset;
   gpt.lon = ngpt[0] / theLonScale + theLonOffset;
   gpt.hgt = ellHeight;   
}
rspfGpt rspfPolynomProjection::origin()const
{
   return rspfGpt(theLatOffset,
                   theLonOffset,
                   theHgtOffset);
}
rspfDpt rspfPolynomProjection::getMetersPerPixel() const
{
   rspfGpt centerG(theLatOffset, theLonOffset, theHgtOffset);
   rspfDpt centerI;
   worldToLineSample(centerG,centerI);
   rspfDpt left  = centerI + rspfDpt(-1,0);
   rspfDpt right = centerI + rspfDpt(1,0);
   rspfDpt top  = centerI + rspfDpt(0,-1);
   rspfDpt bottom = centerI + rspfDpt(0,1);
   rspfGpt leftG;
   rspfGpt rightG;
   rspfGpt topG;
   rspfGpt bottomG;
   
   lineSampleToWorld(left, leftG);
   lineSampleToWorld(right, rightG);
   lineSampleToWorld(top, topG);
   lineSampleToWorld(bottom, bottomG);
   rspfDpt result;
   
   result.x = (rspfEcefPoint(leftG) - rspfEcefPoint(rightG)).magnitude()/2.0;
   result.y = (rspfEcefPoint(topG) - rspfEcefPoint(bottomG)).magnitude()/2.0;
   return result; 
}
bool rspfPolynomProjection::operator==(const rspfProjection& projection) const
{
   if(&projection == this) return true;
   return false;
}
std::ostream& rspfPolynomProjection::print(std::ostream& out) const
{
   out << "\nDump of rspfPolynomProjection object at " << hex << this << ":\n"
       << LAT_SCALE_KW   << ": " << theLatScale   << "\n"
       << LON_SCALE_KW   << ": " << theLonScale   << "\n"
       << HGT_SCALE_KW   << ": " << theHgtScale   << "\n"
       << LAT_OFFSET_KW  << ": " << theLatOffset  << "\n"
       << LON_OFFSET_KW  << ": " << theLonOffset  << "\n"
       << HGT_OFFSET_KW  << ": " << theHgtOffset  << "\n"
       << "PolySamp: "   <<thePolySamp << "\n"
       << "PolyLine: "   <<thePolyLine << "\n";
   out << endl;
   return rspfProjection::print(out);  
}
bool rspfPolynomProjection::saveState(rspfKeywordlist& kwl,
                              const char* prefix) const
{ 
   kwl.add(prefix, rspfKeywordNames::TYPE_KW, MODEL_TYPE);
   rspfProjection::saveState(kwl, prefix);
   
   kwl.add(prefix, LAT_SCALE_KW, theLatScale);
   kwl.add(prefix, LON_SCALE_KW, theLonScale);
   kwl.add(prefix, HGT_SCALE_KW, theHgtScale);
   kwl.add(prefix, LAT_OFFSET_KW, theLatOffset);
   kwl.add(prefix, LON_OFFSET_KW, theLonOffset);   
   kwl.add(prefix, HGT_OFFSET_KW, theHgtOffset);
   ostringstream polyLineS;
   thePolyLine.print(polyLineS);
   ostringstream polySampS;
   thePolySamp.print(polySampS);
   kwl.add(prefix, POLY_LINE_KW, polyLineS.str().c_str());
   kwl.add(prefix, POLY_SAMP_KW, polySampS.str().c_str());
   return true;
}
bool rspfPolynomProjection::loadState(const rspfKeywordlist& kwl,
                              const char* prefix) 
{
   const char* value;
   const char* keyword;
   bool success = rspfProjection::loadState(kwl, prefix);
   if (!success)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfPolynomProjection::loadState(): rspfProjection::loadState() returning with error..." << endl;
      return false;
   }
      
   
   keyword = LAT_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLatScale = rspfString(value).toDouble();
   
   keyword = LON_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLonScale = rspfString(value).toDouble();
   
   keyword = HGT_SCALE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theHgtScale = rspfString(value).toDouble();
         
   keyword = LAT_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLatOffset = rspfString(value).toDouble();
   
   keyword = LON_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theLonOffset = rspfString(value).toDouble();
   
   keyword = HGT_OFFSET_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   theHgtOffset = rspfString(value).toDouble();
   keyword = POLY_LINE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   std::istringstream polyLineIS;
   polyLineIS.str(std::string(value));
   thePolyLine.import(polyLineIS);
   keyword = POLY_SAMP_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::loadState(): Error encountered parsing the following required keyword: "
                                          << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                          << std::endl;
      return false;
   }
   std::istringstream polySampIS;
   polySampIS.str(std::string(value));
   thePolySamp.import(polySampIS);
   buildDerivatives();
   
   return true;
}
void 
rspfPolynomProjection::setPolyLine(const rspfPolynom< rspf_float64 , 3 >& poly)
{
   thePolyLine = poly;
   buildLineDerivatives();
}
void 
rspfPolynomProjection::setPolySamp(const rspfPolynom< rspf_float64 , 3 >& poly)
{
   thePolySamp = poly;
   buildSampDerivatives();
}
void
rspfPolynomProjection::buildDerivatives()
{
   buildLineDerivatives();
   buildSampDerivatives();
}
void
rspfPolynomProjection::buildLineDerivatives()
{
   thePolyLine.pdiff(0, thePolyLine_DLon);
   thePolyLine.pdiff(1, thePolyLine_DLat);
}
void
rspfPolynomProjection::buildSampDerivatives()
{
   thePolySamp.pdiff(0, thePolySamp_DLon);
   thePolySamp.pdiff(1, thePolySamp_DLat);
}
bool
rspfPolynomProjection::setupOptimizer(const rspfString& setup)
{
   return setupDesiredExponents(setup);
}
rspf_uint32
rspfPolynomProjection::degreesOfFreedom()const
{
   return (rspf_uint32)theExpSet.size() * 2;
}
bool
rspfPolynomProjection::setupDesiredExponents(const rspfString& monoms)
{
  bool res=false;
  theExpSet.clear();
  std::vector< rspfString > spm =   monoms.explode(" \t,;");
    for (std::vector< rspfString >::const_iterator it = spm.begin(); it!=spm.end() ;++it)
  {
     rspfPolynom< rspf_float64 , 3 >::EXP_TUPLE et;
     res = stringToExp(*it, et);
     if (!res)
     {
        rspfNotify(rspfNotifyLevel_FATAL) << 
           "FATAL rspfPolynomProjection::setupDesiredExponents(): bad exponent tuple string: "<<*it<<std::endl;
        return false;
     }
     theExpSet.insert(et);
  }
  return true;
}
void
rspfPolynomProjection::setupDesiredExponents(int horizonal_ord, int vertical_ord)
{
   theExpSet.clear();
   thePolySamp.addExpTupleRight(2, horizonal_ord, theExpSet );
   thePolySamp.addExpTupleRight(1, vertical_ord, theExpSet );
}
double
rspfPolynomProjection::optimizeFit(const rspfTieGptSet& tieSet, double* /* targetVariance */)
{
   if (tieSet.size() <=0 )
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::fitToTiePoints(): empty tie point set"<< std::endl;
      return -1.0;
   }
   rspfGpt gmin,gmax;
   tieSet.getGroundBoundaries(gmin,gmax);
   if(gmax.isLatNan() || gmax.isLonNan()|| gmin.isLatNan()||gmin.isLonNan())
   {
      return -1.0;
   }
   theLonOffset = (gmax.lon+gmin.lon)/2.0;
   theLatOffset = (gmax.lat+gmin.lat)/2.0;
   if(rspf::isnan(gmax.hgt)||rspf::isnan(gmin.hgt))
   {
      theHgtOffset = 0.0;
   }
   else if (gmax.hgt <= RSPF_DEFAULT_MIN_PIX_DOUBLE)
   {
      theHgtOffset = 0.0;
   }
   else
   {
      theHgtOffset = (gmax.hgt+gmin.hgt)/2.0;
   }
   rspf_float64 lonScale  = (gmax.lon-gmin.lon)/2.0;
   rspf_float64 latScale  = (gmax.lat-gmin.lat)/2.0;
   rspf_float64 hgtScale  = 1.0;
   if(!rspf::isnan(gmax.hgt)&&!rspf::isnan(gmin.hgt))
   {
      hgtScale = (gmax.hgt-gmin.hgt)/2.0;
   }
   if ((lonScale < DBL_EPSILON) && (latScale < DBL_EPSILON))
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::fitToTiePoints(): ground points are all on the same spot"<< std::endl;
      return -1.0;
   }
   if (lonScale < DBL_EPSILON) lonScale = theLatScale;  
   if (latScale < DBL_EPSILON) latScale = theLonScale;
   
   if (hgtScale < DBL_EPSILON) hgtScale = 1.0; //TBC : arbitrary value in meters
   theLonScale = 1.0/lonScale;
   theLatScale = 1.0/latScale;
   theHgtScale = 1.0/hgtScale;
   vector< rspfPolynom< rspf_float64 , 3 >::VAR_TUPLE > inputs(tieSet.size());
   vector< rspf_float64 > lines(tieSet.size());
   vector< rspf_float64 > samps(tieSet.size());
   const vector<rspfRefPtr<rspfTieGpt> >& theTPV = tieSet.getTiePoints();
   vector< rspfPolynom< rspf_float64 , 3 >::VAR_TUPLE >::iterator  pit;
   vector< rspf_float64 >::iterator sit;
   vector< rspf_float64 >::iterator lit;
   vector<rspfRefPtr<rspfTieGpt> >::const_iterator tit;
   for (tit = theTPV.begin(), pit=inputs.begin(), lit=lines.begin(), sit=samps.begin(); tit!=theTPV.end() ; ++tit,++lit, ++sit,++pit)
   {
      pit->push_back(((*tit)->lon - theLonOffset)*theLonScale);
      pit->push_back(((*tit)->lat - theLatOffset)*theLatScale);
      if (rspf::isnan((*tit)->hgt))
      {
         pit->push_back(theHgtOffset);
      }
      else
      {
         pit->push_back(((*tit)->hgt - theHgtOffset)*theHgtScale);
      }
      *sit = (*tit)->tie.x;
      *lit = (*tit)->tie.y;
   }  
   double line_rms=0.0;
   bool resfit = thePolyLine.LMSfit(theExpSet, inputs, lines, &line_rms);
   if (!resfit)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::fitToTiePoints(): lines polynom LMS fit failed "<< std::endl;
      return -1.0;
   }
   double samp_rms=0.0;
   resfit = thePolySamp.LMSfit(theExpSet, inputs, samps, &samp_rms);
   if (!resfit)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::fitToTiePoints(): samples polynom LMS fit failed "<< std::endl;
      return -1.0;
   }
   buildDerivatives();
   return samp_rms*samp_rms + line_rms*line_rms; //variance
}
bool 
rspfPolynomProjection::stringToExp(const rspfString& s, rspfPolynom< rspf_float64 , 3 >::EXP_TUPLE& et)const
{
   et.clear();
   rspfString ts = s.trim().upcase();
   rspfString tkeys("XYZ");
   if (ts.size() == 0)
   {
      return false;
   }
   if (ts[static_cast<std::string::size_type>(0)] == '1')
   {
      for(int i=0;i<3;i++) et.push_back(0);
      return true;
   }
   int ex[3]={0,0,0};
   while (ts.size()>0)
   {
      int symb = getSymbol(ts, tkeys);
      if (symb<0)
      {
            rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfPolynomProjection::stringToExp(): cant find any symbol"<< std::endl;
            return false;
      }
      int expo = getExponent(ts);
      if (ex[symb]>0)
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "FATAL rspfPolynomProjection::stringToExp(): symbol appears twice: "
            <<tkeys[static_cast<std::string::size_type>(symb)]
            << std::endl;
         return false;
      }
      ex[symb] = expo;
   }
   for(int i=0;i<3;i++) et.push_back(ex[i]);
   return true;
}
int
rspfPolynomProjection::getSymbol(rspfString& ts,const rspfString& symbols) const
{ //remove symbol from string ts and return symbol index, -1 = error
  for(unsigned int i=0;i<symbols.size();++i)
   {
     if (ts.operator[](0) == symbols.operator[](i))
     {
        ts = ts.afterPos(0);
        return i;
     }
   }
   return -1;
}
int
rspfPolynomProjection::getExponent(rspfString& ts) const
{ //remove exponent from string, no exponent means 1
   unsigned int pos=0;
   int expo=0;
   const rspfString& cts(ts);
   while ( (pos<ts.size()) && (cts.operator[](pos)<='9') && (cts.operator[](pos)>='0'))
   {
      expo = 10*expo + (cts.operator[](pos)-'0');
      ++pos;
   }
   if (pos>0) ts=ts.afterPos(pos-1);
   if (expo==0) expo=1;
   return expo;
}
