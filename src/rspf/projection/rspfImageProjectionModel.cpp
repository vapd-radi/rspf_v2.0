#include <string>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/projection/rspfProjection.h>
RTTI_DEF1(rspfImageProjectionModel,
          "rspfImageProjectionModel",
          rspfImageModel)
rspfImageProjectionModel::rspfImageProjectionModel()
   : rspfImageModel(),
     theProjection(0)
{
}
rspfImageProjectionModel::~rspfImageProjectionModel()
{
   theProjection = 0;
}
void rspfImageProjectionModel::initialize(const rspfImageHandler& ih)
{
   rspfImageModel::initialize(ih);
   
   theProjection = 0;
   rspfImageHandler* iih = const_cast<rspfImageHandler*>(&ih); // cast away constness
   rspfRefPtr<rspfImageGeometry> image_geom = iih->getImageGeometry();
   if ( image_geom.valid() )
   {
      theProjection = image_geom->getProjection();
   }
}
const rspfProjection* rspfImageProjectionModel::getProjection() const
{
   return theProjection;
}
