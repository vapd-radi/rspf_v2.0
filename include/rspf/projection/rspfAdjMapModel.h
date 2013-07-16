#ifndef rspfAdjMapModel_HEADER
#define rspfAdjMapModel_HEADER
#include <rspf/matrix/newmat.h>
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfRefPtr.h>
using namespace NEWMAT;
class rspfMapProjection;
class RSPFDLLEXPORT rspfAdjMapModel : public rspfSensorModel
{
public:
   rspfAdjMapModel();
   rspfAdjMapModel(rspfMapProjection* map_proj, const rspfIpt& image_size);
   rspfAdjMapModel(const rspfAdjMapModel& copy_this);
   rspfAdjMapModel(const rspfKeywordlist& kwl, const char* prefix);
   rspfAdjMapModel(const rspfFilename& geom_filename);
   bool initializeFromMap();
   virtual rspfObject* dup() const;
   
   virtual std::ostream& print(std::ostream& out) const;
   
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0) const;
   
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   
   static void writeGeomTemplate(ostream& os);
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   
   virtual void lineSampleToWorld(const rspfDpt& image_point,
                                  rspfGpt&       worldPoint) const;
   
   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;
      
   virtual void updateModel();
   virtual void initAdjustableParameters();
   virtual rspfDpt 	getMetersPerPixel () const;
   enum AdjParamIndex
   {
      OFFSET_X = 0,         // meters
      OFFSET_Y,             // meters
      ROTATION,             // degrees CCW
      SCALE_X,              // ratio
      SCALE_Y,              // ratio
      NUM_ADJ_PARAMS // not an index
   };
   /*!
    * rspfOptimizableProjection
    */
   inline virtual bool useForward()const {return true;} //!ground to image faster (you don't need DEM)
private:
   virtual ~rspfAdjMapModel();
   
   ColumnVector                    theAdjParams;
   rspfRefPtr<rspfMapProjection> theMapProjection;
   double                          theCosTheta;
   double                          theSinTheta;
TYPE_DATA
};
#endif /* End of "#ifndef rspfAdjMapModel_HEADER" */
