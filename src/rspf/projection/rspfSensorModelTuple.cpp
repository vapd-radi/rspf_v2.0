#include <rspf/projection/rspfSensorModelTuple.h>
#include <rspf/projection/rspfPositionQualityEvaluator.h>
#include <rspf/projection/rspfRpcModel.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfNotify.h>
static rspfTrace traceDebug(rspfString("rspfSensorModelTuple:debug"));
static rspfTrace traceExec(rspfString("rspfSensorModelTuple:exec"));
#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfSensorModelTuple.cpp 12582 2008-03-27 19:37:27Z dhicks $";
#endif
rspfRpcPqeInputs::rspfRpcPqeInputs()
   : theRpcElevationAngle(rspf::nan()),
     theRpcAzimuthAngle(rspf::nan()),
     theRpcBiasError(rspf::nan()),
     theRpcRandError(rspf::nan()),
     theSurfaceNormalVector(),
     theSurfaceCovMatrix(3,3)
{
}
rspfRpcPqeInputs::~rspfRpcPqeInputs()
{
}
rspfSensorModelTuple::rspfSensorModelTuple()
:
theNumImages(0),
theSurfCE90(0.0),
theSurfLE90(0.0),
theSurfAccSet(false),
theSurfAccRepresentsNoDEM(false),
theRpcPqeInputs()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfSensorModelTuple::rspfSensorModelTuple DEBUG:"
         << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << std::endl;
#endif 
   }
   
}
rspfSensorModelTuple::~rspfSensorModelTuple()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: ~rspfSensorModelTuple(): entering..." << std::endl;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG: ~rspfSensorModelTuple(): returning..." << std::endl;
}
void rspfSensorModelTuple::addImage(rspfSensorModel* image)
{
   theImages.push_back(image);
   theNumImages++;
}
bool rspfSensorModelTuple::
setIntersectionSurfaceAccuracy(const rspf_float64& surfCE90,
                               const rspf_float64& surfLE90)
{
   bool setOK = false;
   
   if (surfCE90>=0.0 && surfLE90>=0.0)
   {
      theSurfCE90 = surfCE90;
      theSurfLE90 = surfLE90;
      theSurfAccSet = true;
      theSurfAccRepresentsNoDEM = false;
      setOK = true;
   }
   else
   {
      theSurfCE90 = surfCE90;
      theSurfLE90 = surfLE90;
      theSurfAccSet = false;
      theSurfAccRepresentsNoDEM = true;
      setOK = true;
   }
   
   return setOK;
}
void rspfSensorModelTuple::getRpcPqeInputs(rspfRpcPqeInputs& obj) const
{
   obj = theRpcPqeInputs;
}
std::ostream& rspfSensorModelTuple::print(std::ostream& out) const
{
   out << "\n rspfSensorModelTuple::print:" << std::endl;
   for (int i=0; i<theNumImages; ++i)
      theImages[i]->print(out);
   return out;
}
rspfSensorModelTuple::IntersectStatus rspfSensorModelTuple::
intersect(const DptSet_t   obs,
          rspfEcefPoint&  pt,
          NEWMAT::Matrix&  covMat) const
{
   IntersectStatus opOK = OP_FAIL;
   bool covOK = true;
   bool epOK;
   rspf_int32 nImages = (rspf_int32)obs.size();
   
   NEWMAT::SymmetricMatrix N(3);
   NEWMAT::SymmetricMatrix BtWB(3);
   NEWMAT::Matrix Ni(3,3);
   NEWMAT::ColumnVector C(3);
   NEWMAT::ColumnVector BtWF(3);
   NEWMAT::ColumnVector F(2);
   NEWMAT::ColumnVector dR(3);
   NEWMAT::Matrix B(2,3);
   NEWMAT::SymmetricMatrix W(2);
   
   rspfGpt estG;
   theImages[0]->lineSampleHeightToWorld(obs[0], rspf::nan(), estG);
   
   for (int iter=0; iter<3; iter++)
   {   
      N = 0.0;
      C = 0.0;
      for (int i=0; i<nImages; i++)
      {
         rspfDpt resid;
         if (!getGroundObsEqComponents(i, iter, obs[i], estG, resid, B, W))
            covOK = false;
         
         F[0] = resid.x;
         F[1] = resid.y;
         BtWF << B.t() * W * F;
         BtWB << B.t() * W * B;
         C += BtWF;
         N += BtWB;
      }
      Ni = invert(N);
      dR = Ni * C;
      rspfEcefPoint estECF(estG);
      for (rspf_int32 i=0; i<3; i++)
         estECF[i] += dR[i];
      rspfGpt upd(estECF);
      estG = upd;
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG: intersect:\n"
            << "  iteration:\n" << iter
            << "  C:\n"  << C 
            << "  Ni:\n" << Ni 
            << "  dR:\n" << dR <<std::endl;
      }
   
   } // iterative loop
   
   rspfEcefPoint finalEst(estG);
   pt = finalEst;
   
   if (covOK)
   {
      covMat = Ni;
      epOK = true;
   }
   else
      epOK = false;
   
   if (epOK)
      opOK = OP_SUCCESS;
   else
      opOK = ERROR_PROP_FAIL;
   
   return opOK;
}
rspfSensorModelTuple::IntersectStatus rspfSensorModelTuple::
intersect(const rspf_int32&   img,
          const rspfDpt&      obs,
          const rspf_float64& atHeightAboveEllipsoid,
          rspfEcefPoint&      pt,
          NEWMAT::Matrix&      covMat)
{
   IntersectStatus opOK = OP_FAIL;
   rspfGpt ptG;
   
   theImages[img]->lineSampleHeightToWorld(obs, atHeightAboveEllipsoid, ptG);
   rspfEcefPoint ptECF(ptG);
   pt = ptECF;
   
   bool epOK = computeSingleInterCov(img, obs, ptG, AT_HGT, covMat);
      
   if (epOK)
      opOK = OP_SUCCESS;
   else
      opOK = ERROR_PROP_FAIL;
   
   return opOK;
}
rspfSensorModelTuple::IntersectStatus rspfSensorModelTuple::
intersect(const rspf_int32&     img,
          const rspfDpt&        obs,
                rspfEcefPoint&  pt,
                NEWMAT::Matrix&  covMat)
{
   IntersectStatus opOK = OP_FAIL;
   rspfGpt ptG;
   
   theImages[img]->lineSampleToWorld(obs, ptG);
   rspfEcefPoint ptECF(ptG);
   pt = ptECF;
   
   bool epOK = computeSingleInterCov(img, obs, ptG, AT_DEM, covMat);
   
   if (epOK)
      opOK = OP_SUCCESS;
   else
      opOK = ERROR_PROP_FAIL;
   
   return opOK;
}
bool rspfSensorModelTuple::getGroundObsEqComponents(
      const rspf_int32 img,
      const rspf_int32 iter,
      const rspfDpt& obs,
      const rspfGpt& ptEst,
      rspfDpt& resid,
      NEWMAT::Matrix& B,
      NEWMAT::SymmetricMatrix& W) const
{
   if (iter==0)
   {
      rspfGpt ptObs(obs.samp,obs.line);
      theImages[img]->getForwardDeriv(OBS_INIT, ptObs);
   }
   resid = theImages[img]->getForwardDeriv(EVALUATE, ptEst);
   rspfDpt pWRTx = theImages[img]->getForwardDeriv(P_WRT_X, ptEst);
   rspfDpt pWRTy = theImages[img]->getForwardDeriv(P_WRT_Y, ptEst);
   rspfDpt pWRTz = theImages[img]->getForwardDeriv(P_WRT_Z, ptEst);
   NEWMAT::SymmetricMatrix Cov(2);
   rspfSensorModel::CovMatStatus covStatus;
   covStatus = theImages[img]->getObsCovMat(obs,Cov);
   NEWMAT::Matrix Wfull = invert(Cov);
   W << Wfull;
   B[0][0] = pWRTx.u;
   B[1][0] = pWRTx.v;
   B[0][1] = pWRTy.u;
   B[1][1] = pWRTy.v;
   B[0][2] = pWRTz.u;
   B[1][2] = pWRTz.v;
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: getGroundObsEqComponents:"
         << "\n  pWRTx: "<<pWRTx
         << "\n  pWRTy: "<<pWRTy
         << "\n  pWRTz: "<<pWRTz <<std::endl;
   }
   
   bool covOK = false;
   if (covStatus == rspfSensorModel::COV_FULL)
      covOK = true;
   
   return covOK;
}
bool rspfSensorModelTuple::computeSingleInterCov(
      const rspf_int32& img,
      const rspfDpt&    obs,
      const rspfGpt&    ptG,
      HeightRefType_t    cRefType,
      NEWMAT::Matrix&    covMat)
{
   NEWMAT::SymmetricMatrix BtWB(3);
   NEWMAT::Matrix B(2,3);
   NEWMAT::SymmetricMatrix W(2);
   NEWMAT::Matrix surfCovENU(3,3);
   rspfDpt resid;
   
   bool tCovOK;
   bool covOK;
   
   rspfHgtRef hgtRef(cRefType);
   if (PTR_CAST(rspfRpcModel, theImages[img]))
   {
     rspfRpcModel* model = PTR_CAST(rspfRpcModel, theImages[img]);
      rspfGpt ptObs(obs.samp,obs.line);
      theImages[img]->getForwardDeriv(OBS_INIT, ptObs);
      resid = theImages[img]->getForwardDeriv(EVALUATE, ptG);
      rspfDpt pWRTx = theImages[img]->getForwardDeriv(P_WRT_X, ptG);
      rspfDpt pWRTy = theImages[img]->getForwardDeriv(P_WRT_Y, ptG);
      rspfDpt pWRTz = theImages[img]->getForwardDeriv(P_WRT_Z, ptG);
     
      rspfLsrSpace enu(ptG);
      NEWMAT::Matrix jECF(3,2);
      jECF(1,1) = pWRTx.u;
      jECF(1,2) = pWRTx.v;
      jECF(2,1) = pWRTy.u;
      jECF(2,2) = pWRTy.v;
      jECF(3,1) = pWRTz.u;
      jECF(3,2) = pWRTz.v;
      NEWMAT::Matrix jLSR(3,2);
      jLSR = enu.ecefToLsrRotMatrix()*jECF;
      rspf_float64  dU_dx = jLSR(1,1);
      rspf_float64  dU_dy = jLSR(2,1);
      rspf_float64  dU_dz = jLSR(3,1);
      rspf_float64  dV_dx = jLSR(1,2);
      rspf_float64  dV_dy = jLSR(2,2);
      rspf_float64  dV_dz = jLSR(3,2);
      
      rspf_float64 den = dU_dy*dV_dx - dV_dy*dU_dx;
      rspf_float64 dY  = dU_dx*dV_dz - dV_dx*dU_dz;
      rspf_float64 dX  = dV_dy*dU_dz - dU_dy*dV_dz;
      rspf_float64 dy_dH = dY / den;
      rspf_float64 dx_dH = dX / den;
      rspf_float64 tAz = atan2(dx_dH, dy_dH);
      rspf_float64 tEl = atan2(1.0, sqrt(dy_dH*dy_dH+dx_dH*dx_dH));
      rspfColumnVector3d surfN = hgtRef.getLocalTerrainNormal(ptG);
      rspf_float64 surfCE;
      rspf_float64 surfLE;
      
      if (theSurfAccRepresentsNoDEM)
      {
         
         surfN = surfN.zAligned();
         
         rspfRpcModel::rpcModelStruct rpcPar;
         model->getRpcParameters(rpcPar);
         rspf_float64 hgtRng  = rpcPar.hgtScale;
         surfCE = 0.0;
         rspf_float64 scaledHgtRng = abs(hgtRng/theSurfCE90);
         rspf_float64 scaled1SigmaHgtRng = abs(scaledHgtRng/theSurfLE90);
         surfLE = scaled1SigmaHgtRng*1.6449;
         
         rspfNotify(rspfNotifyLevel_INFO)
            << "\n computeSingleInterCov() RPC NoDEM state selected..."
            << "\n   RPC Height Scale = " << rpcPar.hgtScale <<" m"
            << "\n    Scale Divisor   = " <<abs(theSurfCE90)
            << "\n    1-Sigma Divisor = "<<abs(theSurfLE90)
            << std::endl;
      }
      else
      {
         surfCE = theSurfCE90;
         surfLE = theSurfLE90;
      }
      
      tCovOK = hgtRef.getSurfaceCovMatrix(surfCE, surfLE, surfCovENU);
      if (tCovOK)
      {
         theRpcPqeInputs.theRpcElevationAngle   = tEl*DEG_PER_RAD;
         theRpcPqeInputs.theRpcAzimuthAngle     = tAz*DEG_PER_RAD;
         theRpcPqeInputs.theRpcBiasError        = model->getBiasError();
         theRpcPqeInputs.theRpcRandError        = model->getRandError();
         theRpcPqeInputs.theSurfaceNormalVector = surfN;
         theRpcPqeInputs.theSurfaceCovMatrix    = surfCovENU;
         
         rspfNotify(rspfNotifyLevel_INFO)
            << "\n RPC error prop parameters..."
            << "\n   Elevation Angle  = " << tEl*DEG_PER_RAD<< " deg"
            << "\n   Azimuth Angle    = " << tAz*DEG_PER_RAD<<" deg"
            << "\n   RPC Bias Error   = " <<model->getBiasError() <<" m"
            << "\n   RPC Random Error = " <<model->getRandError()<<" m"
            << "\n            surfN   = " <<surfN
            << "\n       surfCovENU   = \n" <<surfCovENU
            << std::endl;
         
         rspfEcefPoint pt(ptG);
         
         rspfPositionQualityEvaluator qev
            (pt,model->getBiasError(),model->getRandError(),
             tEl,tAz,surfN,surfCovENU);
         NEWMAT::Matrix covENU(3,3);
         covOK = qev.getCovMatrix(covENU);
         if (covOK)
         {
            covMat = enu.lsrToEcefRotMatrix()*covENU*enu.ecefToLsrRotMatrix();
         }            
      }
      else
      {
         covOK = false;
      }
   }
   
   else
   {
      covOK = getGroundObsEqComponents(img, 0, obs, ptG, resid, B, W);
      BtWB << B.t() * W * B;
      NEWMAT::Matrix St(3,3);
      
      if (theSurfAccRepresentsNoDEM)
      {
         rspfNotify(rspfNotifyLevel_INFO)
            << "\n computeSingleInterCov() RPC NoDEM state selected..."
            << " Not valid for this sensor model" << std::endl;
      }
      
      if (hgtRef.getSurfaceCovMatrix(theSurfCE90, theSurfLE90, surfCovENU))
      {
         tCovOK = hgtRef.getSurfaceNormalCovMatrix(ptG, surfCovENU, St);
      }
      else
      {
         tCovOK = false;
      }
         
      if (tCovOK)
      {
         NEWMAT::Matrix Sti = invert(St);
         covMat = invert(BtWB + Sti);
      }
      else
      {
         covMat = invert(BtWB);
      }
   }
   
   return covOK;
}
NEWMAT::Matrix rspfSensorModelTuple::invert(const NEWMAT::Matrix& m) const
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
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "DEBUG: rspfSensorModelTuple::invert(): "
               << "\nsingular matrix in SVD..."
               << std::endl;
         }
      }
   }
   return v*d*u.t();
}
