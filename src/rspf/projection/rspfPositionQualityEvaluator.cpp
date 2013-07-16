#include <rspf/projection/rspfPositionQualityEvaluator.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfLsrSpace.h>
static rspfTrace traceDebug(rspfString("rspfPositionQualityEvaluator:debug"));
static rspfTrace traceExec(rspfString("rspfPositionQualityEvaluator:exec"));
#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfPositionQualityEvaluator.cpp";
#endif
const rspf_uint32 nTableEntries = 21;
const rspf_uint32 nMultiplier = nTableEntries-1;
const rspf_float64 table90[nTableEntries]=
  {1.644854,1.645623,1.647912,1.651786,1.657313,
   1.664580,1.673829,1.685227,1.699183,1.716257,
   1.737080,1.762122,1.791522,1.825112,1.862530,
   1.903349,1.947158,1.993595,2.042360,2.093214,2.145966};
   
const rspf_float64 Fac1D[NUM_PROB_LEVELS] =
   {1.0, 0.6745, 1.6449, 1.96};
   
const rspf_float64 Fac2D[NUM_PROB_LEVELS] =
   {1.0, 1.1774, 2.1460, 2.4477};
   
const rspf_float64 Fac2D90[NUM_PROB_LEVELS] =
   {0.46598, 0.54865, 1.0, 1.14059};
rspfPositionQualityEvaluator::rspfPositionQualityEvaluator()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfPositionQualityEvaluator::rspfPositionQualityEvaluator DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   
   theEvaluatorValid = false;
   theRpcModel.theType = 'N';
}
rspfPositionQualityEvaluator::
rspfPositionQualityEvaluator(const rspfEcefPoint& pt,const NEWMAT::Matrix& covMat)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfPositionQualityEvaluator::rspfPositionQualityEvaluator DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   theRpcModel.theType = 'N';
   rspfGpt ptG(pt);
   thePtG = ptG;
   
   rspfLsrSpace enu(ptG);
   theLocalFrame = enu;
   
   theCovMat = theLocalFrame.ecefToLsrRotMatrix()*covMat*
               theLocalFrame.lsrToEcefRotMatrix();
   theEvaluatorValid = decomposeMatrix();
}
rspfPositionQualityEvaluator::
rspfPositionQualityEvaluator(const rspfEcefPoint&      pt,
                              const rspf_float64&       errBiasLOS,
                              const rspf_float64&       errRandLOS,
                              const rspf_float64&       elevAngleLOS,
                              const rspf_float64&       azimAngleLOS,
                              const rspfColumnVector3d& surfN,
                              const NEWMAT::Matrix&      surfCovMat)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfPositionQualityEvaluator::rspfPositionQualityEvaluator DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   theRpcModel.theType = 'N';
   rspfGpt ptG(pt);
   thePtG = ptG;
    
   rspfLsrSpace enu(ptG);
   theLocalFrame = enu;
   if (constructMatrix
      (errBiasLOS, errRandLOS, elevAngleLOS, azimAngleLOS, surfN, surfCovMat))
   {
      theEvaluatorValid = decomposeMatrix();
   }
   else
   {
      theEvaluatorValid = false;
   }
}
rspfPositionQualityEvaluator::
rspfPositionQualityEvaluator(const rspfEcefPoint&      pt,
                              const rspf_float64&       errBiasLOS,
                              const rspf_float64&       errRandLOS,
                              const pqeRPCModel&         rpc,
                              const rspfColumnVector3d& surfN,
                              const NEWMAT::Matrix&      surfCovMat)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfPositionQualityEvaluator::rspfPositionQualityEvaluator DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   rspfGpt ptG(pt);
   thePtG = ptG;
    
   rspfLsrSpace enu(ptG);
   theLocalFrame = enu;
   
   theRpcModel = rpc;
   
   double elevAngleLOS;
   double azimAngleLOS;
   computeElevAzim(rpc, elevAngleLOS, azimAngleLOS);
   
   if (constructMatrix
      (errBiasLOS, errRandLOS, elevAngleLOS, azimAngleLOS, surfN, surfCovMat))
   {
      theEvaluatorValid = decomposeMatrix();
   }
   else
   {
      theEvaluatorValid = false;
   }
}
rspfPositionQualityEvaluator::~rspfPositionQualityEvaluator()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: ~rspfPositionQualityEvaluator(): returning..." << std::endl;
}
std::ostream& rspfPositionQualityEvaluator::
print(std::ostream& out) const
{
   out << "\nPositionQualityEvaluator Summary..."<<std::endl;
   out << " theEvaluatorValid: ";
   if (theEvaluatorValid)
      out<<"True"<<std::endl;
   else
      out<<"False"<<std::endl;
   out << " thePtG: "<<thePtG<<std::endl;
   out << " theCovMat [m]:\n"<<theCovMat;
   out << fixed << setprecision(1);
   out << " theEllipse: "<<theEllipse.theSemiMajorAxis<<"  "
                         <<theEllipse.theSemiMinorAxis
                         <<" [m, 1 sigma] at ";
   out << theEllipse.theAzimAngle*DEG_PER_RAD<<" [deg] azimuth"<<endl;
   return out;
}
bool rspfPositionQualityEvaluator::getCovMatrix(NEWMAT::Matrix& covMat) const
{
   if (theEvaluatorValid)
   {
      covMat = theCovMat;
   }
   return theEvaluatorValid;
}
bool rspfPositionQualityEvaluator::
addContributingCovariance(NEWMAT::Matrix& covMat)
{
   bool matrixOK = (covMat.Nrows()==3) && (covMat.Nrows()==3);
   if (theEvaluatorValid && matrixOK)
   {
      theCovMat += covMat;
   
      theEvaluatorValid = decomposeMatrix();
   }
   return (theEvaluatorValid && matrixOK);
}
bool rspfPositionQualityEvaluator::
addContributingCE_LE(const rspf_float64& cCE, const rspf_float64& cLE)
{
   NEWMAT::Matrix covMat(3,3);
   
   formCovMatrixFromCE_LE(cCE, cLE, covMat);
   return addContributingCovariance(covMat);
}
bool rspfPositionQualityEvaluator::
subtractContributingCovariance(NEWMAT::Matrix& covMat)
{
   bool matrixOK = (covMat.Nrows()==3) && (covMat.Nrows()==3);
   if (theEvaluatorValid && matrixOK)
   {
      theCovMat -= covMat;
   
      theEvaluatorValid = decomposeMatrix();
   }
   return (theEvaluatorValid && matrixOK);
}
bool rspfPositionQualityEvaluator::
subtractContributingCE_LE(const rspf_float64& cCE, const rspf_float64& cLE)
{
   NEWMAT::Matrix covMat(3,3);
   
   formCovMatrixFromCE_LE(cCE, cLE, covMat);
   return subtractContributingCovariance(covMat);
}
bool rspfPositionQualityEvaluator::
computeCE_LE(const pqeProbLev_t pLev, rspf_float64& CE, rspf_float64& LE) const
{
   if (theEvaluatorValid)
   {
      LE = sqrt(theCovMat(3,3)) * (rspf_float64)Fac1D[pLev];
      CE = (rspf_float64)Fac2D90[pLev] * compute90PCE();
   }
   
   return theEvaluatorValid;
}
bool rspfPositionQualityEvaluator::
extractErrorEllipse(const pqeProbLev_t pLev, pqeErrorEllipse& ellipse)
{
   if (theEvaluatorValid)
   {
      ellipse.theSemiMinorAxis =
         (rspf_float64)Fac2D[pLev] * theEllipse.theSemiMinorAxis;
      ellipse.theSemiMajorAxis =
         (rspf_float64)Fac2D[pLev] * theEllipse.theSemiMajorAxis;
      
      ellipse.theAzimAngle = theEllipse.theAzimAngle;
      
      ellipse.theCenter = thePtG;
   }
   return theEvaluatorValid;
}
bool rspfPositionQualityEvaluator::
extractErrorEllipse(const pqeProbLev_t     pLev,
                    const rspf_float64&   angularIncrement,
                          pqeErrorEllipse& ellipse,
                          pqeImageErrorEllipse_t& ellImage)
{
   bool computeImageEllipse = true;
   
   if (theRpcModel.theType == 'N')
      computeImageEllipse = false;
      
   if (theEvaluatorValid && computeImageEllipse)
   {
      extractErrorEllipse(pLev, ellipse);
      
      int numSteps = 360/(int)angularIncrement;
      double dxMaj = ellipse.theSemiMajorAxis*sin(ellipse.theAzimAngle);
      double dyMaj = ellipse.theSemiMajorAxis*cos(ellipse.theAzimAngle);
      double dxMin = ellipse.theSemiMinorAxis*sin(ellipse.theAzimAngle+M_PI/2.0);
      double dyMin = ellipse.theSemiMinorAxis*cos(ellipse.theAzimAngle+M_PI/2.0);
      
      for (int j = 0; j<=numSteps; ++j)
      {
      
         double ang = TWO_PI*j/numSteps;
         double sang = sin(ang);
         double cang = cos(ang);
         double x = dxMaj*cang + dxMin*sang;
         double y = dyMaj*cang + dyMin*sang;
         double z = 0.0;
         
         rspfLsrPoint cpLSR(x, y, z, theLocalFrame);
         rspfEcefPoint cp = rspfEcefPoint(cpLSR);
         rspfGpt cpG(cp);
         double lat = cpG.latd();
         double lon = cpG.lond();
         double hgt = cpG.height();
      
         double nlat = (lat - theRpcModel.theLatOffset) /
                        theRpcModel.theLatScale;
         double nlon = (lon - theRpcModel.theLonOffset) /
                        theRpcModel.theLonScale;
         double nhgt;
         if( rspf::isnan(hgt) )
         {
            nhgt = (theRpcModel.theHgtScale - theRpcModel.theHgtOffset) /
                    theRpcModel.theHgtScale;
         }
         else
         {
            nhgt = (hgt - theRpcModel.theHgtOffset) / theRpcModel.theHgtScale;
         }
         double Pu = polynomial(nlat, nlon, nhgt, theRpcModel.theLineNumCoef);
         double Qu = polynomial(nlat, nlon, nhgt, theRpcModel.theLineDenCoef);
         double Pv = polynomial(nlat, nlon, nhgt, theRpcModel.theSampNumCoef);
         double Qv = polynomial(nlat, nlon, nhgt, theRpcModel.theSampDenCoef);
         double Un  = Pu / Qu;
         double Vn  = Pv / Qv;
         double U  = Un*theRpcModel.theLineScale + theRpcModel.theLineOffset;
         double V  = Vn*theRpcModel.theSampScale + theRpcModel.theSampOffset;
         rspfDpt img(V,U);
         ellImage.push_back(img);
      
      }
   }
   return (theEvaluatorValid && computeImageEllipse);
}
bool rspfPositionQualityEvaluator::decomposeMatrix()
{
      NEWMAT::SymmetricMatrix S(2);
      S<<theCovMat(1,1)<<theCovMat(1,2)<<theCovMat(2,2);
      NEWMAT::DiagonalMatrix D;
      NEWMAT::Matrix V;
      NEWMAT::Jacobi(S,D,V);
      theEllipse.theSemiMinorAxis = sqrt(D(1,1));
      theEllipse.theSemiMajorAxis = sqrt(D(2,2));
      theEigenvectors = V;
      rspf_float64 sin2theta = 2.0*theCovMat(1,2);
      rspf_float64 cos2theta = theCovMat(1,1)-theCovMat(2,2);
      if (cos2theta == 0.0)
      {
         return false;
      }
      else
      {
         double rotAngle = atan3(sin2theta, cos2theta)/2.0;
         theEllipse.theAzimAngle = M_PI/2.0 - rotAngle;
         if (theEllipse.theAzimAngle < 0.0)
            theEllipse.theAzimAngle += TWO_PI;
      }
      return true;
}
bool rspfPositionQualityEvaluator::
constructMatrix(const rspf_float64&       errBiasLOS,
                const rspf_float64&       errRandLOS,
                const rspf_float64&       elevAngleLOS,
                const rspf_float64&       azimAngleLOS,
                const rspfColumnVector3d& surfN,
                const NEWMAT::Matrix&      surfCovMat)
{
   bool constructOK = true;
   rspfColumnVector3d lsrNorm(0.0,0.0,1.0);
   
   rspf_float64 eTot = sqrt(errBiasLOS*errBiasLOS + errRandLOS*errRandLOS);
   if (eTot == 0.0)
      eTot = 0.001;
   
   double elC = elevAngleLOS;
   double azC = azimAngleLOS;   
   rspfColumnVector3d  LOS(sin(azC)*cos(elC), cos(azC)*cos(elC), sin(elC));
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"DEBUG: constructMatrix..."<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"  tEl,tAz: "<<elC*DEG_PER_RAD<<"  "<<azC*DEG_PER_RAD<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"  LOS:     "<<LOS<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG)
         <<"  eTot:    "<<eTot<<endl;
   }
   
   rspfColumnVector3d tSlopeN = surfN.unit();
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) <<"  tSlopeN: "<<tSlopeN<<endl;
   }
   
   rspfColumnVector3d pMinU(0,1,0);
   rspfColumnVector3d pMinAxis = LOS.cross(tSlopeN);
   if (pMinAxis.magnitude() > DBL_EPSILON)
   {
      pMinU = pMinAxis.unit();
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) <<"  pMinU: "<<pMinU<<endl;
   }
   
   rspfColumnVector3d pMaxU = (tSlopeN.cross(pMinU)).unit();
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) <<"  pMaxU: "<<pMaxU<<endl;
   }
   
   double elevAngTerr = acos(pMaxU.dot(LOS));
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"  elev angle rel to surface: "
      <<elevAngTerr*DEG_PER_RAD<<endl;
   }
   
   if (elevAngTerr > 0.0)
   {
      double planeErrorL = eTot/sin(elevAngTerr);
      
      rspfColumnVector3d pL = vperp(pMaxU, lsrNorm);
      rspfColumnVector3d pN = vperp(pMinU, lsrNorm);
      
      rspfColumnVector3d eL = pL * planeErrorL;
      rspfColumnVector3d eN = pN * eTot;
      double magL = eL.magnitude();
      double magN = eN.magnitude();
      rspfColumnVector3d pV = pMaxU - pL;
      rspfColumnVector3d eV = pV * planeErrorL;
      double magV = eV.magnitude();
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         <<"  Projected horizontal/vertical components...."<<endl
         <<"   pL: "<<pL<<"  magL: "<<magL<<endl
         <<"   pN: "<<pN<<"  magN: "<<magN<<endl
         <<"   pV: "<<pV<<"  magV: "<<magV<<endl;
      }
      double tSlope = acos(tSlopeN.dot(lsrNorm));
      double surfSigV = sqrt(surfCovMat(3,3));
      double surfSigH = sqrt((surfCovMat(1,1)+surfCovMat(2,2))/2.0);
      double surfSigV_h = surfSigH * tan(tSlope);
      double vSigTot = sqrt(surfSigV*surfSigV + surfSigV_h*surfSigV_h);
      
      rspfColumnVector3d s_surfN = (lsrNorm.dot(tSlopeN))*tSlopeN;
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) <<" Surface uncertainty...."<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG) <<"  tSlope angle: "<<tSlope*DEG_PER_RAD<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG) <<"  s_surfN(unit): "<<s_surfN<<endl;
      }
      s_surfN = s_surfN * vSigTot;
      double sigTn = s_surfN.magnitude();
      
      double sigVl = sigTn/sin(elevAngTerr);
      rspfColumnVector3d vSigHvec = sigVl * vperp(LOS, lsrNorm);
      rspfColumnVector3d vSigVvec = sigVl * (LOS.dot(lsrNorm))*lsrNorm;
      double vSigH = vSigHvec.magnitude();
      double vSigV = vSigVvec.magnitude();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         <<" s_surfN: "<<s_surfN
         <<"\n vSigHvec: "<<vSigHvec
         <<"\n vSigVvec: "<<vSigVvec<<endl;
      }
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)<<"----------------------------"<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG)<<" surfSigH: "<<surfSigH<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG)<<" surfSigV: "<<surfSigV<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG)<<"  vSigTot: "<<vSigTot<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG)<<"    vSigH: "<<vSigH<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG)<<"    vSigV: "<<vSigV<<endl;
         rspfNotify(rspfNotifyLevel_DEBUG)<<"----------------------------"<<endl;
      }
      magL = sqrt(magL*magL + vSigH*vSigH);
      double theta;
      double pMax, pMin;
      if (magL > magN)
      {
         pMax = magL;
         pMin = magN;
         theta = atan3(pL[1],pL[0]);
      }
      else
      {
         pMax = magN;
         pMin = magL;
         theta = atan3(pN[1],pN[0]);
      }
      NEWMAT::Matrix Cov(2,2);
      NEWMAT::Matrix Vcomp(2,2);
      NEWMAT::DiagonalMatrix Dcomp(2);
      
      Dcomp(1,1) = pMax*pMax;
      Dcomp(2,2) = pMin*pMin;
      Vcomp(1,1) = cos(theta);
      Vcomp(2,1) = sin(theta);
      Vcomp(1,2) =-Vcomp(2,1);
      Vcomp(2,2) = Vcomp(1,1);
      Cov = Vcomp*Dcomp*Vcomp.t();
      NEWMAT::Matrix covMat(3,3);
      covMat(1,1) = Cov(1,1);   
      covMat(1,2) = Cov(1,2);   
      covMat(1,3) = 0.0;   
      covMat(2,1) = Cov(2,1);
      covMat(2,2) = Cov(2,2);   
      covMat(2,3) = 0.0;
      covMat(3,1) = covMat(1,3);
      covMat(3,2) = covMat(2,3);
      covMat(3,3) = magV*magV + vSigV*vSigV;
      theCovMat = covMat;
      
   }  //end if (elevAngTerr > 0.0)
   else
   {
      constructOK = false;
      rspfNotify(rspfNotifyLevel_WARN)
         << "WARNING: rspfPositionQualityEvaluator::constructMatrix(): "
         << "\n   terrain-referenced elev ang: "<<elevAngTerr*DEG_PER_RAD
         << std::endl;
   }
      
   return constructOK;
}
bool rspfPositionQualityEvaluator::
formCovMatrixFromCE_LE(const rspf_float64&  CE,
                       const rspf_float64&  LE,
                             NEWMAT::Matrix& covMat) const
{         
   covMat = 0.0;
   covMat(1,1) = CE/2.146;
   covMat(2,2) = CE/2.146;
   covMat(3,3) = LE/1.6449;
   covMat(1,1) *= covMat(1,1);
   covMat(2,2) *= covMat(2,2);
   covMat(3,3) *= covMat(3,3);
   return true;
}
double rspfPositionQualityEvaluator::compute90PCE() const
{         
   rspf_float64 pRatio = theEllipse.theSemiMinorAxis/
                          theEllipse.theSemiMajorAxis;
   rspf_uint32 ndx = int(floor(pRatio*nMultiplier));
   rspf_float64 alpha;
   if (ndx == nMultiplier)
   {
      alpha = table90[ndx];
   }
   else
   {
      rspf_float64 fac = (pRatio-(rspf_float64)ndx/(rspf_float64)nMultiplier) / 0.05;
      alpha = fac * (table90[ndx+1]-table90[ndx]) + table90[ndx];
   }
              
   rspf_float64 CE90 = alpha * theEllipse.theSemiMajorAxis;
   
   return CE90;
}
bool rspfPositionQualityEvaluator::
computeElevAzim(const pqeRPCModel     rpc,
                      rspf_float64&  elevAngleLOS,
                      rspf_float64&  azimAngleLOS) const
{
   double nlat = (thePtG.lat - rpc.theLatOffset) / rpc.theLatScale;
   double nlon = (thePtG.lon - rpc.theLonOffset) / rpc.theLonScale;
   double nhgt;
   if( rspf::isnan(thePtG.hgt) )
   {
      nhgt = (rpc.theHgtScale - rpc.theHgtOffset) / rpc.theHgtScale;
   }
   else
   {
      nhgt = (thePtG.hgt - rpc.theHgtOffset) / rpc.theHgtScale;
   }
   
   double Pu = polynomial(nlat, nlon, nhgt, rpc.theLineNumCoef);
   double Qu = polynomial(nlat, nlon, nhgt, rpc.theLineDenCoef);
   double Pv = polynomial(nlat, nlon, nhgt, rpc.theSampNumCoef);
   double Qv = polynomial(nlat, nlon, nhgt, rpc.theSampDenCoef);
   double dPu_dLat = dPoly_dLat(nlat, nlon, nhgt, rpc.theLineNumCoef);
   double dQu_dLat = dPoly_dLat(nlat, nlon, nhgt, rpc.theLineDenCoef);
   double dPv_dLat = dPoly_dLat(nlat, nlon, nhgt, rpc.theSampNumCoef);
   double dQv_dLat = dPoly_dLat(nlat, nlon, nhgt, rpc.theSampDenCoef);
   double dPu_dLon = dPoly_dLon(nlat, nlon, nhgt, rpc.theLineNumCoef);
   double dQu_dLon = dPoly_dLon(nlat, nlon, nhgt, rpc.theLineDenCoef);
   double dPv_dLon = dPoly_dLon(nlat, nlon, nhgt, rpc.theSampNumCoef);
   double dQv_dLon = dPoly_dLon(nlat, nlon, nhgt, rpc.theSampDenCoef);
   double dPu_dHgt = dPoly_dHgt(nlat, nlon, nhgt, rpc.theLineNumCoef);
   double dQu_dHgt = dPoly_dHgt(nlat, nlon, nhgt, rpc.theLineDenCoef);
   double dPv_dHgt = dPoly_dHgt(nlat, nlon, nhgt, rpc.theSampNumCoef);
   double dQv_dHgt = dPoly_dHgt(nlat, nlon, nhgt, rpc.theSampDenCoef);
   double dU_dLat = (Qu*dPu_dLat - Pu*dQu_dLat)/(Qu*Qu);
   double dU_dLon = (Qu*dPu_dLon - Pu*dQu_dLon)/(Qu*Qu);
   double dU_dHgt = (Qu*dPu_dHgt - Pu*dQu_dHgt)/(Qu*Qu);
   double dV_dLat = (Qv*dPv_dLat - Pv*dQv_dLat)/(Qv*Qv);
   double dV_dLon = (Qv*dPv_dLon - Pv*dQv_dLon)/(Qv*Qv);
   double dV_dHgt = (Qv*dPv_dHgt - Pv*dQv_dHgt)/(Qv*Qv);
   
   dU_dLat *= rpc.theLineScale/rpc.theLatScale;
   dU_dLon *= rpc.theLineScale/rpc.theLonScale;
   dU_dHgt *= rpc.theLineScale/rpc.theHgtScale;
   dV_dLat *= rpc.theSampScale/rpc.theLatScale;
   dV_dLon *= rpc.theSampScale/rpc.theLonScale;
   dV_dHgt *= rpc.theSampScale/rpc.theHgtScale;
   dU_dLat *= DEG_PER_RAD;
   dU_dLon *= DEG_PER_RAD;
   dV_dLat *= DEG_PER_RAD;
   dV_dLon *= DEG_PER_RAD;
   
   rspfEcefPoint location(thePtG);
   NEWMAT::Matrix jMat(3,3);
   thePtG.datum()->ellipsoid()->jacobianWrtEcef(location, jMat);
   rspfDpt pWRTx;
   rspfDpt pWRTy;
   rspfDpt pWRTz;
   pWRTx.u = dU_dLat*jMat(1,1)+dU_dLon*jMat(2,1)+dU_dHgt*jMat(3,1);
   pWRTy.u = dU_dLat*jMat(1,2)+dU_dLon*jMat(2,2)+dU_dHgt*jMat(3,2);
   pWRTz.u = dU_dLat*jMat(1,3)+dU_dLon*jMat(2,3)+dU_dHgt*jMat(3,3);
   pWRTx.v = dV_dLat*jMat(1,1)+dV_dLon*jMat(2,1)+dV_dHgt*jMat(3,1);
   pWRTy.v = dV_dLat*jMat(1,2)+dV_dLon*jMat(2,2)+dV_dHgt*jMat(3,2);
   pWRTz.v = dV_dLat*jMat(1,3)+dV_dLon*jMat(2,3)+dV_dHgt*jMat(3,3);
   NEWMAT::Matrix jECF(3,2);
   jECF(1,1) = pWRTx.u;
   jECF(1,2) = pWRTx.v;
   jECF(2,1) = pWRTy.u;
   jECF(2,2) = pWRTy.v;
   jECF(3,1) = pWRTz.u;
   jECF(3,2) = pWRTz.v;
   NEWMAT::Matrix jLSR(3,2);
   jLSR = theLocalFrame.ecefToLsrRotMatrix()*jECF;
   double dU_dx = jLSR(1,1);
   double dU_dy = jLSR(2,1);
   double dU_dz = jLSR(3,1);
   double dV_dx = jLSR(1,2);
   double dV_dy = jLSR(2,2);
   double dV_dz = jLSR(3,2);
   
   double den = dU_dy*dV_dx - dV_dy*dU_dx;
   double dY  = dU_dx*dV_dz - dV_dx*dU_dz;
   double dX  = dV_dy*dU_dz - dU_dy*dV_dz;
   double dy_dH = dY / den;
   double dx_dH = dX / den;
   
   azimAngleLOS = atan2(dx_dH, dy_dH);
   elevAngleLOS = atan2(1.0, sqrt(dy_dH*dy_dH+dx_dH*dx_dH));
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"DEBUG: computeElevAzim..."<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<
         " el,az = "<<elevAngleLOS*DEG_PER_RAD<<" "<<azimAngleLOS*DEG_PER_RAD<<endl;
   }
   return true;
}
rspfColumnVector3d rspfPositionQualityEvaluator::
vperp(const rspfColumnVector3d& v1, const rspfColumnVector3d& v2) const
{
   
   double scale = v1.dot(v2)/v2.dot(v2);
   rspfColumnVector3d v = v2*scale;
   
   rspfColumnVector3d p = v1 - v;
   
   return p;
}
double rspfPositionQualityEvaluator::atan3(const double  y,
                                            const double  x) const
{
   double u,v,pih=0.5*M_PI,result;
   if (x == 0.0)
      result = M_PI - pih * rspf::sgn(y);
   else
   {
      if (y == 0.0)
      {
         if (x > 0.0)
            result = 0.0;
         else
            result = M_PI;
      }
      else
      {
         u = y/x;
         v = fabs(u);
         result = atan(v);
         result *= v/u;
         if (x < 0.0)
            result += M_PI;
         else
            if (result < 0.0)
               result += TWO_PI;
      }
   }
   
   return result;
}
double rspfPositionQualityEvaluator::polynomial(
   const double& P,
   const double& L,
   const double& H,
   const double* c) const
{
   double r;
   if (theRpcModel.theType == 'A')
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
double rspfPositionQualityEvaluator::dPoly_dLat(
   const double& P,
   const double& L,
   const double& H,
   const double* c) const
{
   double dr;
   if (theRpcModel.theType == 'A')
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
double rspfPositionQualityEvaluator::dPoly_dLon(
   const double& P,
   const double& L,
   const double& H,
   const double* c) const
{
   double dr;
   if (theRpcModel.theType == 'A')
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
double rspfPositionQualityEvaluator::dPoly_dHgt(
   const double& P,
   const double& L,
   const double& H,
   const double* c) const
{
   double dr;
   if (theRpcModel.theType == 'A')
   {
      dr = c[3] + c[5]*L + c[6]*P + c[7]*L*P + 2*c[10]*H + c[13]*L*L +
           c[16]*P*P + 2*c[17]*L*H + 2*c[18]*P*H + 3*c[19]*H*H;
   }
   else
   {
      dr = c[3] + c[5]*L + c[6]*P + 2*c[9]*H + c[10]*L*P + 2*c[13]*L*H +
           2*c[16]*P*H + c[17]*L*L + c[18]*P*P + 3*c[19]*H*H;
   }
   return dr;
}
