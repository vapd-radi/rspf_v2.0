#ifndef rspfCoarseGridModel_HEADER
#define rspfCoarseGridModel_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/base/rspfFilename.h>
class rspfImageGeometry;
/******************************************************************************
 *
 * CLASS:  rspfCoarseGridModel
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfCoarseGridModel : public rspfSensorModel
{
public:
   /**
    * CONSTRUCTORS:
    */
   rspfCoarseGridModel();
   rspfCoarseGridModel(const rspfCoarseGridModel& copy_this);
   /**
    * CONSTRUCTOR (filename)
    * Accepts name of geometry file. This can be either MET ECG geom file, or
    * RSPF keywordlist geometry file.
    */
   rspfCoarseGridModel(const rspfFilename& geom_file);
   /**
    * CONSTRUCTOR (keywordlist)
    * Accepts RSPF keywordlist geometry file.
    */
   rspfCoarseGridModel(const rspfKeywordlist& geom_kwl);
   
   ~rspfCoarseGridModel();
   /**
    * This method will build a grid from any projector.
    * The first argument must be the image space bounds for
    * the projection.  The second argument is the projector
    * that will be used to approximate a bilinear grid over.
    *
    * The accuracy of the grid can be controlled by the static method
    * setInterpolationError.
    */
   virtual void buildGrid(const rspfDrect& imageBounds,
                          rspfProjection* proj,
                          double heightDelta=500.0,
                          bool enableHeightFlag=false,
                          bool makeAdjustableFlag=true);
   virtual void buildGrid(const rspfDrect& imageBounds,
                          rspfImageGeometry* geom,
                          double heightDelta=500.0,
                          bool enableHeightFlag=false,
                          bool makeAdjustableFlag=true);
   
   /**
    *  This is used when building a grid from a projector.
    *  You can set the interpolation error.  The default is
    *  subpixel accuracy (within .1 of a pixel).
    */
   static void setInterpolationError(double error=.1);
   static void setMinGridSpacing(rspf_int32 minSpacing = 100);
   /**
    * METHOD: print()
    * Extends base-class implementation. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   
   /**
    * METHODS:  saveState, loadState
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0) const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   /**
    * STATIC METHOD: writeGeomTemplate(ostream)
    * Writes a template of geometry keywords processed by loadState and
    * saveState to output stream.
    */
   static void writeGeomTemplate(ostream& os);
   /**
    * METHOD: dup()
    * Returns pointer to a new instance, copy of this.
    */
   virtual rspfObject* dup() const
      { return new rspfCoarseGridModel(*this); }
   
   /**
    * METHOD: saveCoarseGrid(), loadCoarseGrid()
    * Saves/loads the coarse grid to/from the specified file. Returns true if
    * successful.
    */
   bool saveCoarseGrid(const rspfFilename& cgFileName) const;
   bool loadCoarseGrid(const rspfFilename& cgFileName);
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   
   virtual void lineSampleToWorld(const rspfDpt& image_point,
                                  rspfGpt&       gpt) const;
   /**
    * METHOD: lineSampleHeightToWorld(image_point, height, &ground_point)
    * This is the virtual that performs the actual work of projecting
    * the image point to the earth at some specified elevation.
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       world_pt) const;
   virtual void initAdjustableParameters();
   /*!
    * rspfOptimizableProjection
    */
   inline virtual bool useForward()const {return false;} //!image to ground faster
   
protected:
   /**
    * METHOD: reallocateGrid()
    * Deletes existing allocated memory and reallocates
    * new space. This may happen if a new grid is loaded over an existing one.
    */
   void reallocateGrid(const rspfIpt& size);
   
   void initializeModelParams(rspfIrect irect);
   virtual rspfGpt extrapolate (const rspfDpt& imgPt, const double& height=rspf::nan()) const;
   /**
    * Data Members:
    */
   mutable rspfFilename theGridFilename;
   rspfDblGrid  theLatGrid;         // degrees
   rspfDblGrid  theLonGrid;         // degrees
   rspfDblGrid  theDlatDhGrid;      // degrees/meter
   rspfDblGrid  theDlonDhGrid;      // degrees/meter
   rspfDblGrid* theDlatDparamGrid;  // degrees/(units-of-param)
   rspfDblGrid* theDlonDparamGrid;  // degrees/(units-of-param)
   static double       theInterpolationError;
   static rspf_int32  theMinGridSpacing;
   rspfAdjustmentInfo theInitialAdjustment;
   bool                theHeightEnabledFlag;
   
   TYPE_DATA
};
#endif
