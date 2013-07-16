#include <rspf/projection/rspfCoarseGridModel.h>
RTTI_DEF1(rspfCoarseGridModel, "rspfCoarseGridModel", rspfSensorModel);
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/support_data/rspfSupportFilesList.h>
#include <cstdio>
#include <fstream>
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfCoarseGridModel:exec");
static rspfTrace traceDebug ("rspfCoarseGridModel:debug");
static const char* MODEL_TYPE = "rspfCoarseGridModel";
static const char* GRID_FILE_NAME_KW = "grid_file_name";
const rspfFilename DEFAULT_GEOM_FILE_EXT ("geom");
const rspfFilename DEFAULT_GRID_FILE_EXT ("ocg");
double rspfCoarseGridModel::theInterpolationError = .1;
rspf_int32 rspfCoarseGridModel::theMinGridSpacing     = 100;
rspfCoarseGridModel::rspfCoarseGridModel()
   :
      rspfSensorModel(),
      theDlatDparamGrid (0),
      theDlonDparamGrid (0),
      theHeightEnabledFlag(true)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel: entering..."
      << std::endl;
   theLatGrid.setDomainType(rspfDblGrid::SAWTOOTH_90);
   theLonGrid.setDomainType(rspfDblGrid::WRAP_180);
   theLatGrid.enableExtrapolation();
   theLonGrid.enableExtrapolation();
   setErrorStatus();
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel: returning..."
      << std::endl;
}
rspfCoarseGridModel::rspfCoarseGridModel(const rspfCoarseGridModel& model)
   :
      rspfSensorModel  (model),
      theGridFilename   (model.theGridFilename),
      theLatGrid        (model.theLatGrid),
      theLonGrid        (model.theLonGrid),
      theDlatDhGrid     (model.theDlatDhGrid),
      theDlonDhGrid     (model.theDlonDhGrid),
      theDlatDparamGrid (0),
      theDlonDparamGrid (0),
      theHeightEnabledFlag(true)
{
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel(model): entering..."
      << std::endl;
   int numberOfParams = getNumberOfAdjustableParameters();
   if(numberOfParams)
   {
      theDlatDparamGrid = new rspfDblGrid [numberOfParams];
      theDlonDparamGrid = new rspfDblGrid [numberOfParams];
      
      for (int i=0; i<numberOfParams; i++)
      {
         theDlatDparamGrid[i] = model.theDlatDparamGrid[i];
         theDlonDparamGrid[i] = model.theDlonDparamGrid[i];
      }
   }
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel: returning..."
      << std::endl;
}
rspfCoarseGridModel::rspfCoarseGridModel(const rspfFilename& geom_file)
   :
      rspfSensorModel(),
      theDlatDparamGrid (0),
      theDlonDparamGrid (0),
      theHeightEnabledFlag(true)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel(geom_file): entering..." << std::endl;
   
   theLatGrid.setDomainType(rspfDblGrid::SAWTOOTH_90);
   theLonGrid.setDomainType(rspfDblGrid::WRAP_180);
   theLatGrid.enableExtrapolation();
   theLonGrid.enableExtrapolation();
   rspfKeywordlist kwl (geom_file);
   loadState(kwl);
   if (traceExec())   rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel(geom_file): returning..." << std::endl;
   return;
}
rspfCoarseGridModel::rspfCoarseGridModel(const rspfKeywordlist& geom_kwl)
   :
      rspfSensorModel(),
      theDlatDparamGrid (0),
      theDlonDparamGrid (0),
      theHeightEnabledFlag(true)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::rspfCoarseGridModel(geom_kwl): entering..." << std::endl;
   theLatGrid.setDomainType(rspfDblGrid::SAWTOOTH_90);
   theLonGrid.setDomainType(rspfDblGrid::WRAP_180);
   theLatGrid.enableExtrapolation();
   theLonGrid.enableExtrapolation();
   loadState(geom_kwl);
}
void rspfCoarseGridModel::buildGrid(const rspfDrect& imageBounds,
                                     rspfProjection* proj,
                                     double heightDelta,
                                     bool enableHeightFlag,
                                     bool makeAdjustableFlag)
{
   rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();
   geom->setProjection(proj);
   buildGrid(imageBounds, geom.get(), heightDelta, enableHeightFlag, makeAdjustableFlag);
}  
void rspfCoarseGridModel::buildGrid(const rspfDrect& imageBounds,
                                     rspfImageGeometry* geom,
                                     double heightDelta,
                                     bool enableHeightFlag,
                                     bool makeAdjustableFlag)
{
   theHeightEnabledFlag =  enableHeightFlag;
   
   if (!geom->getProjection() || imageBounds.hasNans())
      return;
   const rspfDatum* targetDatum = rspfDatumFactory::instance()->wgs84();
   rspfIpt gridSize(2,2);
   rspfDpt gridOrigin(0,0);
   rspfGpt gpt;
   rspfGpt gpt2;
   rspfGpt bilinearGpt;
   resizeAdjustableParameterArray(0);
   double normSplit = 1.0;
   rspfIpt imageSize = rspfIpt(imageBounds.width(), imageBounds.height());
   double error = 0.0;
   rspfIpt imageOrigin = imageBounds.ul();
   rspfDpt spacing ((double)(imageBounds.width()-1)/(gridSize.x-1),
      (double)(imageBounds.height()-1)/(gridSize.y-1));
   if(theDlatDparamGrid)
   {
      delete [] theDlatDparamGrid;
      theDlatDparamGrid = NULL;
   }
   if(theDlonDparamGrid)
   {
      delete [] theDlonDparamGrid;
      theDlonDparamGrid = NULL;
   }
   geom->localToWorld(imageBounds.midPoint(), gpt);
   do
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "Checking grid size " << gridSize << std::endl;
      }
      spacing = rspfDpt((double)(imageBounds.width()-1)/(gridSize.x-1),
         (double)(imageBounds.height()-1)/(gridSize.y-1));
      theLatGrid.setNullValue(rspf::nan());
      theLonGrid.setNullValue(rspf::nan());
      theDlatDhGrid.setNullValue(0.0);
      theDlonDhGrid.setNullValue(0.0);
      theLatGrid.setDomainType(rspfDblGrid::SAWTOOTH_90);
      theLonGrid.setDomainType(rspfDblGrid::WRAP_180);
      theLatGrid.initialize(gridSize, gridOrigin, spacing);
      theLonGrid.initialize(gridSize, gridOrigin, spacing);
      theDlatDhGrid.initialize(gridSize, gridOrigin, spacing);
      theDlonDhGrid.initialize(gridSize, gridOrigin, spacing);
      rspf_int32 x, y;
      for(y = 0; y < gridSize.y; ++y)
      {
         for(x = 0; x < gridSize.x; ++x)
         {
            rspfDpt norm((double)x/(double)(gridSize.x-1),
               (double)y/(double)(gridSize.y-1));
            rspfDpt pt(imageOrigin.x + norm.x*(imageSize.x-1),
               imageOrigin.y + norm.y*(imageSize.y-1));
            geom->localToWorld(pt, gpt);
            double h = gpt.height();
            if(rspf::isnan(h))
            {
               h += heightDelta;
            }
            rspfDpt fullPt;
            geom->rnToFull(pt, 0, fullPt);
            geom->getProjection()->lineSampleHeightToWorld(fullPt, h, gpt2);
            gpt.changeDatum(targetDatum);
            gpt2.changeDatum(targetDatum);
            theLatGrid.setNode(x, y, gpt.latd());
            theLonGrid.setNode(x, y, gpt.lond());
            theDlatDhGrid.setNode(x, y, (gpt2.latd() - gpt.latd())/heightDelta);
            theDlonDhGrid.setNode(x, y, (gpt2.lond() - gpt.lond())/heightDelta);
         }
      }
      rspf_int32 upperY = 2*gridSize.y;
      rspf_int32 upperX = 2*gridSize.x;
      error = 0.0;
      initializeModelParams(imageBounds);
      for(y = 0; ((y < upperY)&&(error < theInterpolationError)); ++y)
      {
         for(x = 0; ((x < upperX)&&(error<theInterpolationError)); ++x)
         {
            rspfDpt norm((double)x/(double)(upperX-1),
               (double)y/(double)(upperY-1));
            rspfDpt imagePoint(imageOrigin.x + norm.x*(imageSize.x-1),
               imageOrigin.y + norm.y*(imageSize.y-1));
            rspfDpt testIpt;
            geom->localToWorld(imagePoint, gpt);
            worldToLineSample(gpt, testIpt);
            error = (testIpt-imagePoint).length();
         }
      }
      gridSize.x *= 2;
      gridSize.y *= 2;
      normSplit *= .5;
   } while((error > theInterpolationError) &&
           ((imageSize.x*normSplit) > theMinGridSpacing) &&
           ((imageSize.y*normSplit) > theMinGridSpacing));
   gridSize = theLatGrid.size();
   rspfAdjustableParameterInterface* adjustableParameters = 
      PTR_CAST(rspfAdjustableParameterInterface, geom->getProjection());
   removeAllAdjustments();
   if(adjustableParameters&&makeAdjustableFlag)
   {
      if(adjustableParameters->getNumberOfAdjustableParameters() > 0)
      {
         newAdjustment(adjustableParameters->getNumberOfAdjustableParameters());
         int numberOfParams = getNumberOfAdjustableParameters();
         if(numberOfParams)
         {
            theDlatDparamGrid = new rspfDblGrid [numberOfParams];
            theDlonDparamGrid = new rspfDblGrid [numberOfParams];
            for(int paramIdx = 0; paramIdx < numberOfParams; ++ paramIdx)
            {
               theDlonDparamGrid[paramIdx].setNullValue(0.0);
               theDlatDparamGrid[paramIdx].setNullValue(0.0);
               theDlatDparamGrid[paramIdx].initialize(gridSize, gridOrigin, spacing);
               theDlonDparamGrid[paramIdx].initialize(gridSize, gridOrigin, spacing);
               setAdjustableParameter(paramIdx, 0.0);
               setParameterSigma(paramIdx, adjustableParameters->getParameterSigma(paramIdx));
               setParameterUnit(paramIdx, adjustableParameters->getParameterUnit(paramIdx));
               setParameterCenter(paramIdx, 0.0);
               setParameterDescription(paramIdx,
                  adjustableParameters->getParameterDescription(paramIdx));
               double oldParameter = adjustableParameters->getAdjustableParameter(paramIdx);
               adjustableParameters->setAdjustableParameter(paramIdx, 1.0, true);
               double adjust = adjustableParameters->computeParameterOffset(paramIdx);
               double deltaLat = 0;
               double deltaLon = 0;
               if(adjust != 0.0)
               {
                  for(int y = 0; y < gridSize.y; ++y)
                  {
                     for(int x = 0; x < gridSize.x; ++x)
                     {
                        rspfDpt norm((double)x/(double)(gridSize.x-1),
                           (double)y/(double)(gridSize.y-1));
                        rspfDpt pt(imageOrigin.x + norm.x*(imageSize.x-1),
                           imageOrigin.y + norm.y*(imageSize.y-1));
                        geom->localToWorld(pt, gpt);
                        gpt.changeDatum(targetDatum);
                        gpt2.latd(theLatGrid(pt));
                        gpt2.lond(theLonGrid(pt));
                        deltaLat = gpt.latd()-gpt2.latd();
                        deltaLon = gpt.lond()-gpt2.lond();
                        theDlatDparamGrid[paramIdx].setNode(x, y, deltaLat/adjust);
                        theDlonDparamGrid[paramIdx].setNode(x, y, deltaLon/adjust);
                     }
                  }
                  theDlatDparamGrid[paramIdx].enableExtrapolation();
                  theDlonDparamGrid[paramIdx].enableExtrapolation();
               }
               adjustableParameters->setAdjustableParameter(paramIdx, oldParameter, true);
            }
         }
      }
   }
   getAdjustment(theInitialAdjustment);
}
void rspfCoarseGridModel::setInterpolationError(double error)
{
   theInterpolationError = error;
}
void rspfCoarseGridModel::setMinGridSpacing(rspf_int32 minSpacing)
{
   theMinGridSpacing = minSpacing;
}
void rspfCoarseGridModel::initializeModelParams(rspfIrect imageBounds)
{
   rspfIpt gridSize (theLatGrid.size());
   rspfDpt spacing  (theLatGrid.spacing());
   rspfDpt v[4];
   v[0].lat = theLatGrid.getNode(0,0);
   v[0].lon = theLonGrid.getNode(0,0);
   v[1].lat = theLatGrid.getNode(gridSize.x-1, 0);
   v[1].lon = theLonGrid.getNode(gridSize.x-1, 0);
   v[2].lat = theLatGrid.getNode(gridSize.x-1, gridSize.y-1);
   v[2].lon = theLonGrid.getNode(gridSize.x-1, gridSize.y-1);
   v[3].lat = theLatGrid.getNode(0, gridSize.y-1);
   v[3].lon = theLonGrid.getNode(0, gridSize.y-1);
   for (int i=0; i<4; i++)
   {
      if (v[i].lon > 180.0)
         v[i].lon -= 360.0;
   }
   theBoundGndPolygon = rspfPolygon(4, v);
   
   theImageSize  = rspfDpt(imageBounds.width(), imageBounds.height());
   theRefImgPt   = imageBounds.midPoint();
   theRefGndPt.lat = theLatGrid(theRefImgPt);
   theRefGndPt.lon = theLonGrid(theRefImgPt);
   
   rspfDpt ref_ip_dx (theRefImgPt.x+1.0, theRefImgPt.y    );
   rspfDpt ref_ip_dy (theRefImgPt.x    , theRefImgPt.y+1.0);
   rspfGpt ref_gp_dx (theLatGrid(ref_ip_dx), theLonGrid(ref_ip_dx));
   rspfGpt ref_gp_dy (theLatGrid(ref_ip_dy), theLonGrid(ref_ip_dy));
   theGSD.x   = theRefGndPt.distanceTo(ref_gp_dx);
   theGSD.y   = theRefGndPt.distanceTo(ref_gp_dy);
   theMeanGSD = (theGSD.line + theGSD.samp)/2.0;
   theImageClipRect  = imageBounds;
   theSubImageOffset = imageBounds.ul();
}
rspfCoarseGridModel::~rspfCoarseGridModel()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfCoarseGridModel::~rspfCoarseGridModel: entering..."
      << std::endl;
   if(theDlatDparamGrid&&theDlonDparamGrid)
   {
      delete [] theDlatDparamGrid;
      delete [] theDlonDparamGrid;
      theDlatDparamGrid = NULL;
      theDlonDparamGrid = NULL;
   }
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfCoarseGridModel::~rspfCoarseGridModel: returning..."
      << std::endl;
}
void rspfCoarseGridModel::lineSampleToWorld(const rspfDpt& image_point,
                                             rspfGpt&       gpt) const
{
   if(!theHeightEnabledFlag)
   {
      if (!insideImage(image_point))
      {
         gpt = extrapolate(image_point);
         return;
      }
      
      lineSampleHeightToWorld(image_point, 0.0, gpt);
   }
   else
   {
      rspfSensorModel::lineSampleToWorld(image_point, gpt);
   }
}
void
rspfCoarseGridModel::lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                              const double&   arg_hgt_above_ellipsoid,
                                              rspfGpt&       worldPt) const
{
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::lineSampleHeightToWorld: entering..." << std::endl;
   
   if(theLatGrid.size().x < 1 ||
      theLatGrid.size().y < 1)
   {
      worldPt.makeNan();
      return;
   }
   double height = (rspf::isnan(arg_hgt_above_ellipsoid)) ? 0.0 : arg_hgt_above_ellipsoid;
   rspfDpt ip = lineSampPt + theSubImageOffset;
   
   worldPt.lat = theLatGrid(ip);
   worldPt.lon = theLonGrid(ip);
   worldPt.hgt = height;
   if(theHeightEnabledFlag)
   {
      worldPt.lat += theDlatDhGrid(ip)*height;
      worldPt.lon += theDlonDhGrid(ip)*height;
   }
   int numberOfParams = getNumberOfAdjustableParameters();
 
   for (int p=0; p<numberOfParams; p++)
   {
       worldPt.lat += (theDlatDparamGrid[p](ip) * computeParameterOffset(p));
       worldPt.lon += (theDlonDparamGrid[p](ip) * computeParameterOffset(p));
   }
   worldPt.limitLonTo180();
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::lineSampleHeightToWorld: returning..." << std::endl;
}
void rspfCoarseGridModel::initAdjustableParameters()
{
   if(getNumberOfAdjustableParameters() < 1)
   {
      addAdjustment(theInitialAdjustment, true);
   }
   else
   {
      setAdjustment(theInitialAdjustment, true);
   }
}
void rspfCoarseGridModel::imagingRay(const rspfDpt& image_point,
                                      rspfEcefRay&   image_ray) const
{
   rspfSensorModel::imagingRay(image_point, image_ray);
}
std::ostream& rspfCoarseGridModel::print(std::ostream& out) const 
{
   out << "\nDump of rspfCoarseGridModel object at: " << this << "\n"
       << "\n           Grid File Name: " << theGridFilename 
       << "\n                 Image ID: " << theImageID 
       << "\n                   Sensor: " << theSensorID
       << "\n  Image Size (rows, cols): " << theImageSize
       << "\n      Ref Pt (samp, line): " << theRefImgPt 
       << "\n   Ref Pt (lat, lon, hgt): " << theRefGndPt 
       << "\n           GSD (row, col): " << theGSD 
       << "\n  Bounding Ground Polygon: " << theBoundGndPolygon << endl;
   
   char buf[256];
   rspfIpt size (theLatGrid.size());
   rspfDpt spacing (theLatGrid.spacing());
   int line, samp;
   rspfIpt node;
   
   out << "[ line,  samp]        lat        lon         dLat/dH      dLon/dH\n"
       << "-------------------------------------------------------------------"
       <<endl;
   for (node.y=0; node.y<size.y; node.y++)
   {
      line = (int) (node.y*spacing.y);
      
      for (node.x=0; node.x<size.x; node.x++)
      {
         samp = (int) (node.x*spacing.x);
         
         sprintf(buf, "[%5d, %5d]    %+9.5f  %+10.5f    %+11.4e  %+11.4e",
                  line, samp,
                  theLatGrid.getNode(node),
                  theLonGrid.getNode(node),
                  theDlatDhGrid.getNode(node),
                  theDlonDhGrid.getNode(node));
         out << buf << endl;
      }
      out <<"-----------------------------------------------------------------"
          <<endl;
   }
   out << "\n\nDump of lat/lon Partials w.r.t. Adjustable Parameters:"<<endl;
   out << "\nEnd Dump of rspfCoarseGridModel.\n" <<  endl;
   return out;
}
bool rspfCoarseGridModel::saveState(rspfKeywordlist& kwl,
                                     const char* prefix) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::saveState: entering..." << std::endl;
   kwl.add(prefix, rspfKeywordNames::TYPE_KW, "rspfCoarseGridModel");
   kwl.add(prefix, GRID_FILE_NAME_KW, theGridFilename);
   kwl.add(prefix, "height_enabled_flag", theHeightEnabledFlag, true);
   rspfSensorModel::saveState(kwl, prefix);
   rspfString initAdjPrefix = rspfString(prefix) + "init_adjustment.";
   theInitialAdjustment.saveState(kwl, initAdjPrefix);
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::saveState: returning..." << std::endl;
   return true;
}
bool rspfCoarseGridModel::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix) 
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadState: entering..." << std::endl;
   clearErrorStatus();
   const char* value;
   bool success;
   
   value = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if (!value || (strcmp(value, "rspfCoarseGridModel")))
   {
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadState:  returning..." << std::endl;
      theErrorStatus++;
      return false;
   }
   value = kwl.find(prefix, "height_enabled_flag");
   if(value)
   {
      theHeightEnabledFlag = rspfString(value).toBool();
   }
   success = rspfSensorModel::loadState(kwl, prefix);
   if (!success)
   {
      theErrorStatus++;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadState: returning with error..." << std::endl;
      return false;
   }
   theGridFilename = kwl.find(prefix, GRID_FILE_NAME_KW);
   if (!theGridFilename.isReadable())
   {
      rspfFilename alt_path_to_grid = kwl.find(prefix, rspfKeywordNames::GEOM_FILE_KW);
      theGridFilename = alt_path_to_grid.setExtension(DEFAULT_GRID_FILE_EXT);
   }
   if (!theGridFilename.isReadable())
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "rspfCoarseGridModel::loadState() -- Error "
         "encountered opening coarse grid file at "<< "<" <<theGridFilename << ">." << std::endl;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadState: returning with error..." << std::endl;
      theErrorStatus++;
      return false;
   }
   if (!loadCoarseGrid(theGridFilename))
   {
      theErrorStatus++;
      return false;
   }
   rspfSupportFilesList::instance()->add(theGridFilename.expand());
   rspfString initAdjPrefix = rspfString(prefix) + "init_adjustment.";
   theInitialAdjustment.loadState(kwl, initAdjPrefix.c_str());
   if((rspf::isnan(theRefGndPt.hgt)) ||
      (theRefGndPt.hgt == 0))
   {
      theRefGndPt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(theRefGndPt);
      if(theRefGndPt.hgt < 0)
      {
         theRefGndPt.hgt = fabs(theRefGndPt.hgt);
      }
   }
      
   if(theInitialAdjustment.getNumberOfAdjustableParameters() < 1)
   {
      getAdjustment(theInitialAdjustment);
   }
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadState: returning..." << std::endl;
   if (theErrorStatus)
      return false;
   
   return true;
}
bool rspfCoarseGridModel::saveCoarseGrid(const rspfFilename& fileName)const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::saveCoarseGrid: entering..." << std::endl;
   theGridFilename = fileName.expand();
   theGridFilename.setExtension(DEFAULT_GRID_FILE_EXT);
   ofstream outstream (theGridFilename.chars());
   if (!outstream.is_open())
   {
      theErrorStatus++;
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::saveCoarseGrid: Error "
         "encountered creating coarse grid file <" << theGridFilename<< ">. Check that directory "
         "exists and is writable." << std::endl;
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::saveCoarseGrid: returning with error..." << std::endl;
      return false;
   }
   theLatGrid.save(outstream, "Latitude Grid");
   theLonGrid.save(outstream, "Longitude Grid");
   theDlatDhGrid.save(outstream, "dLat/dH Grid");
   theDlonDhGrid.save(outstream, "dLon_dH Grid");
   rspfString descr;
   int numberOfParams = getNumberOfAdjustableParameters();
   for (int p=0; p<numberOfParams; p++)
   {
      descr = getParameterDescription(p) + " dLat_dParam Grid";
      theDlatDparamGrid[p].save(outstream, descr.chars());
      descr = getParameterDescription(p) + " dLon_dParam Grid";
      theDlonDparamGrid[p].save(outstream, descr.chars());
   }
   
   rspfFilename geom_file (theGridFilename);
   geom_file.setExtension(DEFAULT_GEOM_FILE_EXT);
   rspfKeywordlist kwl;
   saveState(kwl);
   kwl.write(geom_file);
   rspfSupportFilesList::instance()->add(geom_file);
   rspfSupportFilesList::instance()->add(theGridFilename);
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::saveCoarseGrid: returning..." << std::endl;
   return true;
}
bool rspfCoarseGridModel::loadCoarseGrid(const rspfFilename& cgFileName)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadCoarseGrid: entering..." << std::endl;
   rspfDpt v[4];
   rspfIpt grid_size;
   ifstream instream (cgFileName.chars());
   if (!instream.is_open())
   {
      theErrorStatus++;
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfCoarseGridModel::loadCoarseGrid: Error encountered opening coarse grid file <" << cgFileName
         << ">. Check that the file exists and is readable." << std::endl;
      }
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "CEBUG rspfCoarseGridModel::loadCoarseGrid: returning with error..." << std::endl;
      return false;
   }
   theGridFilename = cgFileName;
   if(theDlatDparamGrid)
     {
       delete [] theDlatDparamGrid;
       theDlatDparamGrid = NULL;
     }
   if(theDlonDparamGrid)
     {
       delete [] theDlonDparamGrid;
       theDlonDparamGrid = NULL;
     }
   int numberOfParams = getNumberOfAdjustableParameters();
   if(numberOfParams)
   {
      theDlatDparamGrid = new rspfDblGrid [numberOfParams];
      theDlonDparamGrid = new rspfDblGrid [numberOfParams];
   }
   if (!theLatGrid.load(instream))
   {
      ++theErrorStatus;
      return false;
   }
   if (!theLonGrid.load(instream))
   {
      ++theErrorStatus;
      return false;
   }
   if (!theDlatDhGrid.load(instream))
   {
      ++theErrorStatus;
      return false;
   }
   if (!theDlonDhGrid.load(instream))
   {
      ++theErrorStatus;
      return false;
   }
   for (int p=0; p<numberOfParams; p++)
   {
      if (!theDlatDparamGrid[p].load(instream))
      {
         ++theErrorStatus;
         return false;
      }
      if (!theDlonDparamGrid[p].load(instream))
      {
         ++theErrorStatus;
         return false;
      }
   }
   grid_size = theLatGrid.size();
   v[0].lat = theLatGrid(0,0);
   v[0].lon = theLonGrid(0,0);
   v[1].lat = theLatGrid(theImageSize.x-1, 0);
   v[1].lon = theLonGrid(theImageSize.x-1, 0);
   v[2].lat = theLatGrid(theImageSize.x-1, theImageSize.y-1);
   v[2].lon = theLonGrid(theImageSize.x-1, theImageSize.y-1);
   v[3].lat = theLatGrid(0, theImageSize.y-1);
   v[3].lon = theLonGrid(0, theImageSize.y-1);
   theBoundGndPolygon = rspfPolygon(4, v);
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::loadCoarseGrid: returning..." << std::endl;
   return true;
}
void rspfCoarseGridModel::reallocateGrid(const rspfIpt& grid_size)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::reallocateGrid:entering..." << endl;
   if(theDlatDparamGrid)
     {
       delete [] theDlatDparamGrid;
       theDlatDparamGrid = NULL;
     }
   if(theDlonDparamGrid)
     {
       delete [] theDlonDparamGrid;
       theDlonDparamGrid = NULL;
     }
   rspfDpt spacing ((double)(theImageSize.x-1)/(double)(grid_size.x-1),
                     (double)(theImageSize.y-1)/(double)(grid_size.y-1));
   rspfDpt grid_origin(0.0, 0.0);
   theLatGrid.setNullValue(rspf::nan());
   theLonGrid.setNullValue(rspf::nan());
   theDlatDhGrid.setNullValue(0.0);
   theDlonDhGrid.setNullValue(0.0);
   theLatGrid.initialize(grid_size, grid_origin, spacing);
   theLonGrid.initialize(grid_size, grid_origin, spacing);
   theDlatDhGrid.initialize(grid_size, grid_origin, spacing);
   theDlonDhGrid.initialize(grid_size, grid_origin, spacing);
   
   int numberOfParams = getNumberOfAdjustableParameters();
   if(numberOfParams)
   {
      
      theDlatDparamGrid = new rspfDblGrid [numberOfParams];
      theDlonDparamGrid = new rspfDblGrid [numberOfParams];
   }
   for (int p=0; p<numberOfParams; p++)
   {
      theDlonDparamGrid[p].setNullValue(0.0);
      theDlatDparamGrid[p].setNullValue(0.0);
      theDlatDparamGrid[p].initialize(grid_size, grid_origin, spacing);
      theDlonDparamGrid[p].initialize(grid_size, grid_origin, spacing);
   }
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::reallocateGrid: returning..." << std::endl;
   return;
}
void rspfCoarseGridModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::writeGeomTemplate: entering..." << std::endl;
   os <<
      "//**************************************************************\n"
      "// Template for OCG model kewordlist\n"
      "//**************************************************************\n"
      << rspfKeywordNames::TYPE_KW << ": " << MODEL_TYPE << endl;
   rspfSensorModel::writeGeomTemplate(os);
   
   os << "//\n"
      << "// Derived-class rspfCoarseGridModel Keywords:\n"
      << "//\n"
      << GRID_FILE_NAME_KW << ": <string>\n" << endl;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfCoarseGridModel::writeGeomTemplate: returning..." << std::endl;
   return;
}
   
rspfGpt rspfCoarseGridModel::extrapolate(const rspfDpt& local_ip, const double& height) const
{
   rspfGpt gpt;
   lineSampleHeightToWorld(local_ip, height, gpt);
   return gpt;
}
