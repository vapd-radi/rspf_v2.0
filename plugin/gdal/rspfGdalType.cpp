#include "rspfGdalType.h"
#include <rspf/base/rspfCommon.h>
rspfScalarType rspfGdalType::toOssim(GDALDataType gdalType)const
{
   switch(gdalType)
   {
      case GDT_Byte:
      {
         return RSPF_UCHAR;
      }
      case GDT_UInt16:
      {
         return RSPF_USHORT16;
      }
      case GDT_Int16:
      {
         return RSPF_SSHORT16;
      }
      case GDT_Int32:
      {
         rspf_int32 sizeType = GDALGetDataTypeSize(gdalType)/8;
         if(sizeType == 2)
         {
            return RSPF_SSHORT16;
         }
         break;
      }
      case GDT_Float32:
      {
         return RSPF_FLOAT;
         break;
      }
      case GDT_Float64:
      {
         return RSPF_DOUBLE;
         break;
      }
      default:
         break;
   }
   
   return RSPF_SCALAR_UNKNOWN;
}
GDALDataType rspfGdalType::toGdal(rspfScalarType rspfType)const
{
   switch(rspfType)
   {
      case RSPF_UCHAR:
      {
         return  GDT_Byte;
      }
      case RSPF_USHORT11:
      case RSPF_USHORT16:
      {
         return  GDT_UInt16;
      }
      case RSPF_SSHORT16:
      {
         return GDT_Int16;
         break;
      }
      case RSPF_FLOAT:
      case RSPF_NORMALIZED_FLOAT:
      {
         return GDT_Float32;
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         return GDT_Float64;
      }
      case RSPF_SCALAR_UNKNOWN:
      {
         break;
      }
      default:
         break;
   }
   return GDT_Unknown;
}
