#include <cstdlib> /* for atof */
#include <rspf/projection/rspfAdjMapModel.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/projection/rspfMapProjectionFactory.h>
#include <rspf/elevation/rspfElevManager.h>
static const char* PARAMETER_KEYWORDS[] = {"map_offset_x",
                                           "map_offset_y",
                                           "map_rotation",
                                           "map_scale_x",
                                           "map_scale_y"};
static const char* MAP_PROJ_FILE_KW    = "map_proj_filename";
RTTI_DEF1(rspfAdjMapModel, "rspfAdjMapModel", rspfSensorModel);
rspfAdjMapModel::rspfAdjMapModel()
   :
   rspfSensorModel(),
   theAdjParams(NUM_ADJ_PARAMS),
   theMapProjection(),
   theCosTheta(0.0),
   theSinTheta(0.0)
{
   theImageSize = rspfIpt(0,0);
   initAdjustableParameters();
   theErrorStatus++;
}
rspfAdjMapModel::rspfAdjMapModel(rspfMapProjection* map_proj,
                                   const rspfIpt& image_size)
   :
   rspfSensorModel(),
   theAdjParams(NUM_ADJ_PARAMS),
   theMapProjection(map_proj),
   theCosTheta(0.0),
   theSinTheta(0.0)
{
   initializeFromMap();
   theImageSize = image_size;
   initAdjustableParameters();
}
rspfAdjMapModel::rspfAdjMapModel(const rspfAdjMapModel& copy_this)
   :
   rspfSensorModel(copy_this),
   theAdjParams(NUM_ADJ_PARAMS),
   theMapProjection(copy_this.theMapProjection),
   theCosTheta(0.0),
   theSinTheta(0.0)
{
   theAdjParams = copy_this.theAdjParams;
}
rspfAdjMapModel::rspfAdjMapModel(const rspfKeywordlist& kwl,
                                   const char* prefix)
   :
   rspfSensorModel(kwl),
   theAdjParams(NUM_ADJ_PARAMS),
   theMapProjection(),
   theCosTheta(0.0),
   theSinTheta(0.0)      
                
{
   initAdjustableParameters();
   loadState(kwl, prefix);
}
rspfAdjMapModel::rspfAdjMapModel(const rspfFilename& kwl_filename)
   :
   rspfSensorModel(),
   theAdjParams(NUM_ADJ_PARAMS),
   theMapProjection(),
   theCosTheta(0.0),
   theSinTheta(0.0)      
{
   initAdjustableParameters();
   rspfKeywordlist kwl (kwl_filename);
   loadState(kwl);
}
bool rspfAdjMapModel::initializeFromMap()
{
   if (!theMapProjection)
   {
      theErrorStatus++;
      return false;
   }
   theSensorID = "AdjMapModel";
   theGSD = theMapProjection->getMetersPerPixel();
   theMeanGSD = 0.5*(theGSD.x + theGSD.y);
   theRefGndPt = theMapProjection->origin();
   return true;
}
rspfAdjMapModel::~rspfAdjMapModel()
{
   theAdjParams.CleanUp();
}
void rspfAdjMapModel::initAdjustableParameters()
{
   removeAllAdjustments();
   newAdjustment(NUM_ADJ_PARAMS);
   setAdjustmentDescription("Initial Geometry");
   setAdjustableParameter (OFFSET_X, 0.0, 1.0);
   setParameterDescription(OFFSET_X, "map_offset_x");
   setParameterCenter     (OFFSET_X, 0.0);
   setAdjustableParameter (OFFSET_Y, 0.0, 1.0);
   setParameterDescription(OFFSET_Y, "map_offset_y");
   setParameterCenter     (OFFSET_Y, 0.0);
   setAdjustableParameter (ROTATION, 0.0, 1.0);
   setParameterDescription(ROTATION, "map_rotation");
   setParameterCenter     (ROTATION, 0.0);
   setAdjustableParameter (SCALE_X, 0.0, 1.0);
   setParameterDescription(SCALE_X, "map_scale_x");
   setParameterCenter     (SCALE_X, 1.0);
   setAdjustableParameter (SCALE_Y, 0.0, 1.0);
   setParameterDescription(SCALE_Y, "map_scale_y");
   setParameterCenter     (SCALE_Y, 1.0);
   updateModel();
}
void rspfAdjMapModel::updateModel()
{
   for (int i=0; i<NUM_ADJ_PARAMS; i++)
      theAdjParams[i] = computeParameterOffset(i);
   theCosTheta = rspf::cosd(theAdjParams[ROTATION]);
   theSinTheta = rspf::sind(theAdjParams[ROTATION]);
}
void rspfAdjMapModel::lineSampleHeightToWorld(const rspfDpt& image_point,
                                     const double&   heightEllipsoid,
                                     rspfGpt&       worldPoint) const
{
   if (!theMapProjection)
   {
      worldPoint = rspfGpt(rspf::nan(), rspf::nan(), rspf::nan());
      return;
   }
   double x = theAdjParams[SCALE_X]*(image_point.x - theAdjParams[OFFSET_X]);
   double y = theAdjParams[SCALE_Y]*(image_point.y - theAdjParams[OFFSET_Y]);
   rspfDpt adjusted_point(x*theCosTheta + y*theSinTheta, 
                           y*theCosTheta - x*theSinTheta);
   theMapProjection->lineSampleHeightToWorld(adjusted_point,
                                             heightEllipsoid,
                                             worldPoint);
   worldPoint.height(heightEllipsoid);
}
void rspfAdjMapModel::lineSampleToWorld(const rspfDpt& image_point,
                                         rspfGpt&       worldPoint) const
{
   lineSampleHeightToWorld(image_point, 0.0, worldPoint);
   if (!worldPoint.hasNans())
   {
   }
}
void rspfAdjMapModel::worldToLineSample(const rspfGpt& world_point,
                                         rspfDpt&       image_point) const
{
   if (!theMapProjection)
   {
      image_point = rspfDpt(rspf::nan(), rspf::nan());
      return;
   }
   rspfDpt p1;
   theMapProjection->worldToLineSample(world_point, p1);
   rspfDpt p2 (p1.x*theCosTheta - p1.y*theSinTheta, 
                p1.y*theCosTheta + p1.x*theSinTheta);
   image_point.x = p2.x/theAdjParams[SCALE_X] + theAdjParams[OFFSET_X];
   image_point.y = p2.y/theAdjParams[SCALE_Y] + theAdjParams[OFFSET_Y];
}
bool rspfAdjMapModel::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   kwl.add(prefix, rspfKeywordNames::TYPE_KW, "rspfAdjMapModel");
   rspfSensorModel::saveState(kwl, prefix);
   for (int i=0; i<NUM_ADJ_PARAMS; i++)
      kwl.add(prefix, PARAMETER_KEYWORDS[i], theAdjParams[i]);
   if (theMapProjection.valid())
   {
      theMapProjection->saveState(kwl, prefix);
   }
   return true;
}
bool rspfAdjMapModel::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   static const rspfString MODULE ("rspfAdjMapModel::loadState() -- ");
   bool success = true;
   const char* value_str;
   double value;
   rspfString error_msg (MODULE+"Error encountered reading keyword: ");
   try
   {
      value_str = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
      if ((!value_str) || (strcmp(value_str, TYPE_NAME(this))))
      {
         throw (error_msg + rspfKeywordNames::TYPE_KW);
      }
      if (!theMapProjection)
      {
         value_str = kwl.find(prefix, MAP_PROJ_FILE_KW);
         if (!value_str)
         {
            throw (error_msg + MAP_PROJ_FILE_KW);
         }
         theMapProjection = PTR_CAST(rspfMapProjection,
            rspfMapProjectionFactory::instance()->createProjection(rspfKeywordlist(value_str)));
         if (!theMapProjection)
         {
            throw "Error encountered instantiating map ";
         }
      }
      for (int i=0; i<NUM_ADJ_PARAMS; i++)
      {
         value_str = kwl.find(prefix, PARAMETER_KEYWORDS[i]);
         if (!value_str)
         {
            throw (error_msg + PARAMETER_KEYWORDS[i]).chars();
         }
         value = atof(value_str);
         setParameterCenter(i, value);
      }
   }
   catch (const char* /*message*/)
   {
      theErrorStatus++;
      success = false;
   }
   if (success)
   {
      initializeFromMap();
      updateModel();
   }
   return success;
}  
void rspfAdjMapModel::writeGeomTemplate(ostream& os)
{
   os <<
      "//*****************************************************************\n"
      "// Template for Adjustable Map Model keywordlist\n"
      "//*****************************************************************\n"
      << rspfKeywordNames::TYPE_KW << ": " << "rspfFcsiModel" << endl;
   rspfSensorModel::writeGeomTemplate(os);
   os << "//***\n"
      << "// Derived-class AdjMapModel Keywords:\n"
      << "//***\n"
      << PARAMETER_KEYWORDS[OFFSET_X] << ": <east offset meters>\n"
      << PARAMETER_KEYWORDS[OFFSET_Y] << ": <north offset meters>\n"
      << PARAMETER_KEYWORDS[ROTATION] << ": <degrees CW>\n"
      << PARAMETER_KEYWORDS[SCALE_X]  << ": <ratio>\n"
      << PARAMETER_KEYWORDS[SCALE_Y]  << ": <ratio>\n"
      << endl;
   return;
}
std::ostream& rspfAdjMapModel::print(std::ostream& out) const
{      
   rspfSensorModel::print(out);
   for (int i=0; i<NUM_ADJ_PARAMS; i++)
      out << PARAMETER_KEYWORDS[i]  << ": " << theAdjParams[i] << endl;
   out << endl;
   return out;
}
rspfDpt rspfAdjMapModel::getMetersPerPixel () const
{
   if (theMapProjection.valid())
   {
      return theMapProjection->getMetersPerPixel();
   }
   
   return rspfDpt(0,0);
}
rspfObject* rspfAdjMapModel::dup() const
{
   return new rspfAdjMapModel(*this);
}
