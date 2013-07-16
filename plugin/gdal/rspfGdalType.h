#ifndef rspfGdalType_HEADER
#define rspfGdalType_HEADER
#include <rspf/base/rspfConstants.h>
#include <gdal.h>
class rspfGdalType
{
public:
   rspfScalarType toOssim(GDALDataType gdalType)const;
   GDALDataType    toGdal(rspfScalarType)const;
};
#endif
