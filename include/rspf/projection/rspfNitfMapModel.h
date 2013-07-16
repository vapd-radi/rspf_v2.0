#ifndef rspfNitfMapModel_HEADER
#define rspfNitfMapModel_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <iostream>
class rspfString;
class rspfMapProjection;
class RSPFDLLEXPORT rspfNitfMapModel : public rspfSensorModel
{
public:
   /*!
    * CONSTRUCTORS:
    */
   rspfNitfMapModel();
   rspfNitfMapModel(const rspfFilename& init_file);
   rspfNitfMapModel(const rspfKeywordlist& geom_kwl);
   
   virtual ~rspfNitfMapModel();
   /*!
    * Returns pointer to a new instance, copy of this.
    */
   virtual rspfObject* dup() const { return 0; } // TBR
   
   /*!
    * Extends base-class implementation. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   
   /*!
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   /*!
    * Writes a template of geom keywords processed by loadState and saveState
    * to output stream.
    */
   static void writeGeomTemplate(ostream& os);
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   /*!
    * rspfOptimizableProjection
    */
   inline virtual bool useForward()const {return false;} //!image to ground faster
protected:
   
TYPE_DATA
};
#endif
