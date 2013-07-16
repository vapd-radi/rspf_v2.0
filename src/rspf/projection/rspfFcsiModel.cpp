#include <cstdlib>
#include <fstream>
#include <sstream>
#include <rspf/projection/rspfFcsiModel.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfEcefRay.h>
RTTI_DEF1(rspfFcsiModel, "rspfFcsiModel", rspfSensorModel);
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfFcsiModel:exec");
static rspfTrace traceDebug ("rspfFcsiModel:debug");
static const int MODEL_VERSION_NUMBER  = 1;
static const rspfString PARAM_NAMES[] ={"x_pos_offset",
                                         "y_pos_offset",
                                         "z_pos_offset",
                                         "x_rot_corr",
                                         "y_rot_corr",
                                         "z_rot_corr",
                                         "foc_len_offset",
                                         "scan_skew_corr"};
static const rspfString PARAM_UNITS[] ={"meters",
                                         "meters",
                                         "meters",
                                         "degrees",
                                         "degress",
                                         "degress",
                                         "millimeters",
                                         "degrees"};
const char* rspfFcsiModel::PRINCIPAL_POINT_X_KW   = "principal_point_x";
const char* rspfFcsiModel::PRINCIPAL_POINT_Y_KW   = "principal_point_y";
const char* rspfFcsiModel::SCAN_SCALE_MATRIX_00_KW = "scan_scale_matrix_00";
const char* rspfFcsiModel::SCAN_SCALE_MATRIX_01_KW = "scan_scale_matrix_01";
const char* rspfFcsiModel::SCAN_SCALE_MATRIX_10_KW = "scan_scale_matrix_10";
const char* rspfFcsiModel::SCAN_SCALE_MATRIX_11_KW = "scan_scale_matrix_11";
const char* rspfFcsiModel::SCAN_ROTATION_ANGLE_KW = "scan_rotation_angle";
const char* rspfFcsiModel::SCAN_SKEW_ANGLE_KW     = "scan_skew_angle";
const char* rspfFcsiModel::FOCAL_LENGTH_KW        = "focal_length";
const char* rspfFcsiModel::PLATFORM_POSITION_X_KW = "platform_position_x";
const char* rspfFcsiModel::PLATFORM_POSITION_Y_KW = "platform_position_y";
const char* rspfFcsiModel::PLATFORM_POSITION_Z_KW = "platform_position_z";
const char* rspfFcsiModel::CAMERA_ORIENTATION_MATRIX_ELEM_KW
                               = "camera_orientation_matrix_elem_";
rspfFcsiModel::rspfFcsiModel()
   :
   rspfSensorModel       (),
   thePrincipalPoint      (0, 0),
   theScanSkew            (0.0),  
   theScanRotation        (0.0),
   theOpticalDistortion   (0),
   theLsrToEcfRot         (3, 3),
   theEcfOffset           (0, 0, 0),
   theXrotCorr            (0.0),
   theYrotCorr            (0.0),
   theZrotCorr            (0.0),
   theFocalOffset         (0.0),
   theScanSkewCorr        (0.0),
   theAdjLsrToEcfRot      (3, 3),
   theAdjEcfToLsrRot      (3, 3)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: entering..." << std::endl;
   theScanScaleMatrix[0] = 1.0;
   theScanScaleMatrix[1] = 0.0;
   theScanScaleMatrix[2] = 0.0;
   theScanScaleMatrix[3] = 1.0;
   theScanXformMatrix[0] = 1.0;
   theScanXformMatrix[1] = 0.0;
   theScanXformMatrix[2] = 0.0;
   theScanXformMatrix[3] = 1.0;
   initAdjustableParameters();
   setErrorStatus(); // indicate uninitialized
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: returning..." << std::endl;
}
rspfFcsiModel::rspfFcsiModel(const rspfFilename& init_file)
   :
   rspfSensorModel       (),
   thePrincipalPoint      (0, 0),
   theScanSkew            (0.0),  
   theScanRotation        (0.0),
   theOpticalDistortion   (0),
   theLsrToEcfRot         (3, 3),
   theEcfOffset           (0, 0, 0),
   theXrotCorr            (0.0),
   theYrotCorr            (0.0),
   theZrotCorr            (0.0),
   theFocalOffset         (0.0),
   theScanSkewCorr        (0.0),
   theAdjLsrToEcfRot      (3, 3),
   theAdjEcfToLsrRot      (3, 3)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: entering..." << std::endl;
   initAdjustableParameters();
   rspfKeywordlist kwl (init_file);
   loadState(kwl);
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: Exited..." << std::endl;
}
rspfFcsiModel::rspfFcsiModel(const rspfKeywordlist& geom_kwl)
   :
   rspfSensorModel       (),
   thePrincipalPoint      (0, 0),
   theScanSkew            (0.0),  
   theScanRotation        (0.0),
   theOpticalDistortion   (0),
   theLsrToEcfRot         (3, 3),
   theEcfOffset           (0, 0, 0),
   theXrotCorr            (0.0),
   theYrotCorr            (0.0),
   theZrotCorr            (0.0),
   theFocalOffset         (0.0),
   theScanSkewCorr        (0.0),
   theAdjLsrToEcfRot      (3, 3),
   theAdjEcfToLsrRot      (3, 3)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: entering..." << std::endl;
   initAdjustableParameters();
   loadState(geom_kwl);
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: Exited..." << std::endl;
}
rspfFcsiModel::rspfFcsiModel(const rspfFcsiModel& foxy)
   :
   rspfSensorModel     (foxy),
   thePrincipalPoint    (foxy.thePrincipalPoint),
   theScanSkew          (foxy.theScanSkew),  
   theScanRotation      (foxy.theScanRotation),
   theFocalLen          (foxy.theFocalLen),
   thePlatformPos       (foxy.thePlatformPos),
   theLsrToEcfRot       (foxy.theLsrToEcfRot),
   theEcfOffset         (foxy.theEcfOffset),
   theXrotCorr          (foxy.theXrotCorr),
   theYrotCorr          (foxy.theYrotCorr),
   theZrotCorr          (foxy.theZrotCorr),
   theFocalOffset       (foxy.theFocalOffset),
   theScanSkewCorr      (foxy.theScanSkewCorr)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: entering..." << std::endl;
   if(foxy.theOpticalDistortion)
   {
      theOpticalDistortion =
         new rspfRadialDecentLensDistortion (*(foxy.theOpticalDistortion));
   }
   for (int i=0; i<4; i++)
      theScanScaleMatrix[i] = foxy.theScanScaleMatrix[i];
   updateModel();
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: returning..." << std::endl;
}
void rspfFcsiModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                             const double&   height,
                                             rspfGpt&       gpt) const
{
   bool debug = false;  // setable via interactive debugger
   if (traceExec() || debug)  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: entering..." << std::endl;
   
   if (!insideImage(image_point))
   {
      gpt = extrapolate(image_point, height);
   }
   else
   {
      rspfEcefRay imaging_ray;
      imagingRay(image_point, imaging_ray);
      rspfEcefPoint Pecf (imaging_ray.intersectAboveEarthEllipsoid(height));
      gpt = rspfGpt(Pecf);
      if (traceDebug() || debug)
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "Pecf = " << Pecf << std::endl;
         rspfNotify(rspfNotifyLevel_DEBUG) << "gpt = " << gpt << std::endl;
      }
   }
   if (traceExec() || debug)  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::rspfFcsiModel: returning..." << std::endl;
}
   
   
void rspfFcsiModel::imagingRay(const rspfDpt& image_point,
                                rspfEcefRay&   image_ray) const
{
   bool debug = false;  // setable via interactive debugger: "set debug = true"
   if (traceExec() || debug)  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::imagingRay: entering..." << std::endl;
   
   rspfDpt p0 (image_point + theSubImageOffset);
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "image_point = " << image_point << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "theSubImageOffset = " << theSubImageOffset << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "p0 = " << p0 << std::endl;
   }
   
   rspfDpt p1 (p0 - theRefImgPt);
   rspfDpt f1 (theScanXformMatrix[0]*p1.x + theScanXformMatrix[1]*p1.y,
                theScanXformMatrix[2]*p1.x + theScanXformMatrix[3]*p1.y);
   rspfDpt film (f1 - thePrincipalPoint);
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theRefImgPt = " << theRefImgPt << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "p1 = " << p1 << std::endl;
      for (int i=0; i<4; i++)
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "theScanXformMatrix["<<i<<"] = "<<theScanXformMatrix[i] << std::endl;
      }
      rspfNotify(rspfNotifyLevel_DEBUG) << "f1 = " << f1 << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "thePrincipalPoint = " << thePrincipalPoint << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "film (distorted) = " << film << std::endl;
   }
   if (theOpticalDistortion)
      theOpticalDistortion->inverse(film);
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "film (undistorted) = " << film << std::endl;
      if(theOpticalDistortion)
      {
         theOpticalDistortion->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
   }
   
   rspfColumnVector3d cam_ray_dir (film.x, film.y, -theAdjFocalLen);
   rspfEcefVector ecf_ray_dir (theAdjLsrToEcfRot*cam_ray_dir);
   ecf_ray_dir = ecf_ray_dir*(1.0/ecf_ray_dir.magnitude());
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theAdjFocalLen = " << theAdjFocalLen << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "cam_ray_dir = " << cam_ray_dir << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "theAdjLsrToEcfRot = \n" << theAdjLsrToEcfRot << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "ecf_ray_dir = " << ecf_ray_dir << std::endl;
   }
   
   image_ray.setOrigin(theAdjPlatformPos);
   image_ray.setDirection(ecf_ray_dir);
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theAdjPlatformPos = " << theAdjPlatformPos << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "theAdjPlatformPos (rspfGpt): " << rspfGpt(theAdjPlatformPos)
                                          << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "image_ray = " << image_ray << std::endl;
   }
   if (traceExec() || debug)  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::imagingRay: returning..." << std::endl;
}
void rspfFcsiModel::worldToLineSample(const rspfGpt& world_point,
                                       rspfDpt&       image_point) const
{
   bool debug = false;  // setable via interactive debugger: "set debug = true"
   if (traceExec() || debug)  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::imagingRay: entering..." << std::endl;
   rspfEcefPoint g_ecf (world_point);
   rspfEcefVector ecf_ray_dir (g_ecf - theAdjPlatformPos);
   rspfColumnVector3d cam_ray_dir (theAdjEcfToLsrRot*ecf_ray_dir.data());
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"world_point = " << world_point << endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"g_ecf = " << g_ecf << endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"theAdjPlatformPos = " << theAdjPlatformPos << endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"theAdjPlatformPos (rspfGpt): "<<rspfGpt(theAdjPlatformPos)<<endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"ecf_ray_dir = " << ecf_ray_dir.unitVector() << endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"theAdjEcfToLsrRot = \n" << theAdjEcfToLsrRot << endl;
      rspfNotify(rspfNotifyLevel_DEBUG)<<"cam_ray_dir = " << cam_ray_dir << endl;
   }
      
   double scale = -theAdjFocalLen/cam_ray_dir[2];
   rspfDpt film (scale*cam_ray_dir[0], scale*cam_ray_dir[1]);
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theAdjFocalLen = " << theAdjFocalLen << endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "scale = " << scale << endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "film (undistorted) = " << film << endl;
   }
      
   if (theOpticalDistortion)
      theOpticalDistortion->forward(film);
   
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "film (distorted) = " << film << endl;
   }
      
   rspfDpt f1(film + thePrincipalPoint);
   rspfDpt p1(theInvScanXformMatrix[0]*f1.x+theInvScanXformMatrix[1]*f1.y,
               theInvScanXformMatrix[2]*f1.x+theInvScanXformMatrix[3]*f1.y);
   rspfDpt p0 (p1 + theRefImgPt);
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "thePrincipalPoint = " << thePrincipalPoint << endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "f1 = " << f1 << endl;
      for (int i=0; i<4; i++)
         rspfNotify(rspfNotifyLevel_DEBUG)<<"theInvScanXformMatrix["<<i<<"] = "<<theInvScanXformMatrix[i]
             <<endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "p1 = " << p1 << endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "theRefImgPt = " << theRefImgPt << endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "p0 = " << p0 << endl;
   }
      
   image_point = p0 - theSubImageOffset;
   
   if (traceDebug() || debug)
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theImageOffset = " << theSubImageOffset << endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "image_point = " << image_point << endl;
   }
   
   if (traceExec() || debug)  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::imagingRay: returning..." << std::endl;
}
std::ostream& rspfFcsiModel::print(std::ostream& out) const
{
   out << "\nDump of rspfFcsiModel object at " << hex << this << ":\n"
       << "\nrspfFcsiModel data members: "
       << "\n        thePrincipalPoint: " << thePrincipalPoint
       << "\n        theScanScaleMatrix: " << theScanScaleMatrix[0] << "  "
       << theScanScaleMatrix[1]
       << "\n                            " << theScanScaleMatrix[2] << "  "
       << theScanScaleMatrix[3]
       << "\n          theScanSkew: " << theScanSkew
       << "\n      theScanRotation: " << theScanRotation;
   for (int i=0; i<4; ++i)
   {
      out << "\ntheScanXformMatrix["<<i<<"]: " << theScanXformMatrix[i];
   }
   out << "\n          theFocalLen: " << theFocalLen
       << "\n       thePlatformPos: " << thePlatformPos
       << "\n       theLsrToEcfRot: \n"  << theLsrToEcfRot
       << "\n    theAdjLsrToEcfRot: \n" << theAdjLsrToEcfRot
       << "\n         theEcfOffset: " << theEcfOffset
       << "\n          theXrotCorr: " << theXrotCorr
       << "\n          theYrotCorr: " << theYrotCorr
       << "\n          theZrotCorr: " << theZrotCorr
       << "\n       theFocalOffset: " << theFocalOffset
       << "\n      theScanSkewCorr: " << theScanSkewCorr
       << endl;
   
   if (theOpticalDistortion)
   {
      out << *theOpticalDistortion << endl;
   }
   return rspfSensorModel::print(out);
}
bool rspfFcsiModel::saveState(rspfKeywordlist& kwl,
                              const char* prefix) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::saveState: entering..." << std::endl;
   kwl.add(prefix, rspfKeywordNames::TYPE_KW, "rspfFcsiModel");
   rspfSensorModel::saveState(kwl, prefix);
   kwl.add(prefix, PRINCIPAL_POINT_X_KW,    thePrincipalPoint.x);
   kwl.add(prefix, PRINCIPAL_POINT_Y_KW,    thePrincipalPoint.y);
   kwl.add(prefix, SCAN_SCALE_MATRIX_00_KW, theScanScaleMatrix[0]);
   kwl.add(prefix, SCAN_SCALE_MATRIX_01_KW, theScanScaleMatrix[1]);
   kwl.add(prefix, SCAN_SCALE_MATRIX_10_KW, theScanScaleMatrix[2]);
   kwl.add(prefix, SCAN_SCALE_MATRIX_11_KW, theScanScaleMatrix[3]);
   kwl.add(prefix, SCAN_SKEW_ANGLE_KW,      theScanSkew);
   kwl.add(prefix, SCAN_ROTATION_ANGLE_KW,  theScanRotation);
   kwl.add(prefix, FOCAL_LENGTH_KW,         theFocalLen);
   kwl.add(prefix, PLATFORM_POSITION_X_KW,
           thePlatformPos.x());
   kwl.add(prefix, PLATFORM_POSITION_Y_KW,
           thePlatformPos.y());
   kwl.add(prefix, PLATFORM_POSITION_Z_KW,
           thePlatformPos.z());
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         ostringstream ostr;
         ostr << CAMERA_ORIENTATION_MATRIX_ELEM_KW
              << i << "_" << j << ends;
         kwl.add(prefix, ostr.str().c_str(), theLsrToEcfRot[i][j]);
      }
   }
   
   if (theOpticalDistortion)
   {
      ostringstream ostr;
      ostr << prefix << "distortion." << ends;
      theOpticalDistortion->saveState(kwl, ostr.str().c_str());
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::saveState: returning..." << std::endl;
   return true;
}
bool rspfFcsiModel::loadState(const rspfKeywordlist& kwl,
                               const char* prefix) 
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::loadState: entering..." << std::endl;
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG:"
                                          << "\nInput kwl:  " << kwl
                                          << std::endl;
   }
   const char* value;
   const char* keyword;
   bool success;
   keyword = rspfKeywordNames::TYPE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "Type name not found");
      return false;
      
   }
   if (strcmp(value, TYPE_NAME(this)))
   {
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::saveState: returning..." << std::endl;
      return false;
   }
      
   
   success = rspfSensorModel::loadState(kwl, prefix);
   if (!success)
   {
      return false;
   }
   if(!getNumberOfAdjustableParameters())
   {
      initAdjustableParameters();
   }
   keyword = PRINCIPAL_POINT_X_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "principle point x not given");
      return false;
   }
   thePrincipalPoint.x = atof(value);
 
   keyword = PRINCIPAL_POINT_Y_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "principle point y not given");
      return false;
   }
      
   thePrincipalPoint.y = atof(value);
 
   keyword = SCAN_SCALE_MATRIX_00_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "scale matrix 0,0 not given");
      return false;
   }
   theScanScaleMatrix[0] = atof(value);
 
   keyword = SCAN_SCALE_MATRIX_01_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "scale matrix 0,1 not given");
      return false;
   }
   theScanScaleMatrix[1] = atof(value);
 
   keyword = SCAN_SCALE_MATRIX_10_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "scale matrix 1,0 not given");
      return false;
   }
   theScanScaleMatrix[2] = atof(value);
 
   keyword = SCAN_SCALE_MATRIX_11_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "scale matrix 1,1 not given");
      return false;
   }
   theScanScaleMatrix[3] = atof(value);
 
   keyword = SCAN_SKEW_ANGLE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
      theScanSkew = 0.0;
   else
      theScanSkew = atof(value);
   keyword = SCAN_ROTATION_ANGLE_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
      theScanRotation = 0.0;
   else
      theScanRotation = atof(value);
 
   keyword = FOCAL_LENGTH_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "Focal length not given");
      return false;
   }
   theFocalLen = atof(value);
 
   keyword = PLATFORM_POSITION_X_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "Platform position X not given");
      return false;
   }
   thePlatformPos.x() = atof(value);
 
   keyword = PLATFORM_POSITION_Y_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "Platform position Y not given");
   }
   thePlatformPos.y() = atof(value);
 
   keyword = PLATFORM_POSITION_Z_KW;
   value = kwl.find(prefix, keyword);
   if (!value)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "Platform position Z not given");
   }
   thePlatformPos.z() = atof(value);
   
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         ostringstream ostr;
         ostr << CAMERA_ORIENTATION_MATRIX_ELEM_KW
              << i << "_" << j << ends;
         rspfString s = ostr.str();
         keyword = s.c_str();
         value = kwl.find(prefix, keyword);
         if (!value)
         {
            rspfSetError(getClassName(),
                          rspfErrorCodes::RSPF_ERROR,
                          "Camera orientation matrix not given");
         }
         theLsrToEcfRot[i][j] = rspfString(value).toDouble();
      }
   }
   
   if (theOpticalDistortion)
      delete theOpticalDistortion;
   theOpticalDistortion = new rspfRadialDecentLensDistortion(kwl, prefix);
   clearErrorStatus();
   updateModel();
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::loadState: returning..." << std::endl;
   return true;
}
void  rspfFcsiModel::updateModel()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::updateModel: entering..." << std::endl;
   
   theEcfOffset.x() = getAdjustableParameter(X_POS) *
                      getParameterSigma(X_POS);
   theEcfOffset.y() = getAdjustableParameter(Y_POS) *
                      getParameterSigma(Y_POS);
   theEcfOffset.z() = getAdjustableParameter(Z_POS) *
                      getParameterSigma(Z_POS);
   theXrotCorr      = getAdjustableParameter(X_ROT) *
                      getParameterSigma(X_ROT);
   theYrotCorr      = getAdjustableParameter(Y_ROT) *
                      getParameterSigma(Y_ROT);
   theZrotCorr      = getAdjustableParameter(Z_ROT) *
                      getParameterSigma(Z_ROT);
   theFocalOffset   = getAdjustableParameter(FOCAL_LEN) *
                      getParameterSigma(FOCAL_LEN);
   theScanSkewCorr  = getAdjustableParameter(SCAN_SKEW) *
                      getParameterSigma(SCAN_SKEW);
   theAdjFocalLen    = theFocalLen    + theFocalOffset;
   theAdjPlatformPos = thePlatformPos + theEcfOffset;
   
   double cos_rot  = rspf::cosd(theScanRotation);
   double sin_rot  = rspf::sind(theScanRotation);
   double tan_skew = rspf::tand(theScanSkew + theScanSkewCorr);
   double a =  theScanScaleMatrix[0]*(sin_rot*tan_skew + cos_rot) +
               theScanScaleMatrix[1]*(cos_rot*tan_skew - sin_rot);
   double b =  theScanScaleMatrix[0]*sin_rot + theScanScaleMatrix[1]*cos_rot;
   double c =  theScanScaleMatrix[2]*(sin_rot*tan_skew + cos_rot) +
               theScanScaleMatrix[3]*(cos_rot*tan_skew - sin_rot);
   double d =  theScanScaleMatrix[2]*sin_rot + theScanScaleMatrix[3]*cos_rot;
   theScanXformMatrix[0] = a;
   theScanXformMatrix[1] = b;
   theScanXformMatrix[2] = c;
   theScanXformMatrix[3] = d;
   theInvScanXformMatrix[0] =  d/(d*a - b*c);
   theInvScanXformMatrix[1] = -b/(d*a - b*c);
   theInvScanXformMatrix[2] =  c/(b*c - a*d);
   theInvScanXformMatrix[3] = -a/(b*c - a*d);
   double cw = rspf::cosd(theXrotCorr);
   double sw = rspf::sind(theXrotCorr);
   double cp = rspf::cosd(theYrotCorr);
   double sp = rspf::sind(theYrotCorr);
   double ck = rspf::cosd(theZrotCorr);
   double sk = rspf::sind(theZrotCorr);
   NEWMAT::Matrix attitudeCorrection(3, 3);
   attitudeCorrection(1,1) =  ck*cw - sk*sp*sw;
   attitudeCorrection(1,2) =  sk*cp;
   attitudeCorrection(1,3) =  ck*sw + sk*sp*cw;
   attitudeCorrection(2,1) = -sk*cw - ck*sp*sw;
   attitudeCorrection(2,2) =  cp*ck;
   attitudeCorrection(2,3) = -sk*sw + ck*sp*cw;
   attitudeCorrection(3,1) = -cp*sw;
   attitudeCorrection(3,2) = -sp;
   attitudeCorrection(3,3) =  cp*cw;
   theAdjLsrToEcfRot = attitudeCorrection * theLsrToEcfRot;
   theAdjEcfToLsrRot = theAdjLsrToEcfRot.t();
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::updateModel: returning..." << std::endl;
}
void rspfFcsiModel::initAdjustableParameters()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::initAdjustableParameters: entering..." << std::endl;
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
   setParameterSigma(X_ROT, 0.01);
   setParameterSigma(Y_ROT, 0.01);
   setParameterSigma(Z_ROT, 0.01);
   setParameterSigma(FOCAL_LEN,0.005);  
   setParameterSigma(SCAN_SKEW, 0.001);
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::initAdjustableParameters: returning..." << std::endl;
}
void rspfFcsiModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::writeGeomTemplate: entering..." << std::endl;
   os <<
      "//*****************************************************************\n"
      "// Template for Frame Camera Scanned Image (FCSI) model keywordlist\n"
      "//*****************************************************************\n"
      << rspfKeywordNames::TYPE_KW << ": " << "rspfFcsiModel" << endl;
   rspfSensorModel::writeGeomTemplate(os);
   
   os << "//***\n"
      << "// Derived-class FCSI Keywords:\n"
      << "//***\n"
      << PRINCIPAL_POINT_X_KW << ": <pixels>\n"
      << PRINCIPAL_POINT_Y_KW << ": <pixels>\n"
      << SCAN_SCALE_MATRIX_00_KW << ": <mm/pixel>\n"
      << SCAN_SCALE_MATRIX_01_KW << ": <mm/pixel>\n"
      << SCAN_SCALE_MATRIX_10_KW << ": <mm/pixel>\n"
      << SCAN_SCALE_MATRIX_11_KW << ": <mm/pixel>\n"
      << SCAN_SKEW_ANGLE_KW << ": <degrees> [OPTIONAL]\n"
      << SCAN_ROTATION_ANGLE_KW << ": <degrees> [OPTIONAL]\n"
      << FOCAL_LENGTH_KW << ": <millimeters>\n"
      << PLATFORM_POSITION_X_KW << ": <meters (ECF)>\n"
      << PLATFORM_POSITION_Y_KW << ": <meters (ECF)>\n"
      << PLATFORM_POSITION_Z_KW << ": <meters (ECF)>\n"
      << "\n"
      << "//***\n"
      << "// Camera (LSR) to ECF orientation matrix:\n"
      << "//***\n";
   
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         ostringstream ostr;
         ostr << CAMERA_ORIENTATION_MATRIX_ELEM_KW
              << i << "_" << j << ends;
         os << ostr.str() << ": <Euler rot matrix element>\n";
      }
   }
   os << "\n"
      << "//***\n"
      << "// Optical distortion coefficients [OPTIONAL]:\n"
      << "//***\n"
      << "distortion."
      << rspfRadialDecentLensDistortion::RADIAL_DISTORTION_COEFF_KW
      << "N: <coeff N> (for N <= 5)\n"
      << "distortion."
      << rspfRadialDecentLensDistortion::DECENT_DISTORTION_COEFF_KW
      << "N: <coeff N> (for N <= 4)\n"
      << endl;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfFcsiModel::writeGeomTemplate: returning..." << std::endl;
   
   return;
}
