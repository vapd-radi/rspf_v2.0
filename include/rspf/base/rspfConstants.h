/********************************************************************
 *
 * License:  See top level LICENSE.txt file.
 *
 * Author: Ken Melero
 * 
 * Description: Common file for global constants.
 *
 **************************************************************************
 * $Id: rspfConstants.h 22221 2013-04-11 15:30:08Z dburken $
 */
#ifndef rspfConstants_HEADER
#define rspfConstants_HEADER 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus   
#include <cfloat>
#else
#include <float.h>
#endif
#include <rspf/rspfConfig.h>

#ifdef NULL
#undef NULL
#define NULL 0
#endif
/**
 * WARNINGS SECTION:
 */
#ifdef _MSC_VER /* Quiet a bunch of MSVC warnings... */
#  pragma warning(disable:4786) /* visual c6.0 compiler */
#  pragma warning(disable:4251)/* for std:: member variable to have dll interface */
#  pragma warning(disable:4275) /* for std:: base class to have dll interface */
#  pragma warning(disable:4800) /* int forcing value to bool */
#  pragma warning(disable:4244) /* conversion, possible loss of data */
#endif
#if defined(__GNUC__)
#   define RSPF_DEPRECATE_METHOD(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#   define RSPF_DEPRECATE_METHOD(func)  __declspec(deprecated) func 
#else
#   define RSPF_DEPRECATE_METHOD(func)
#endif
   
/**
 * DLL IMPORT/EXORT SECTION
 */
#if defined(RSPF_STATIC)
#  define RSPFEXPORT
#  define RSPFIMPORT
#  define RSPFDLLEXPORT
#  define RSPF_DLL
#  define RSPFDLLEXPORT_DATA(type) type
#  define RSPF_DLL_DATA(type) type
#  define RSPFDLLEXPORT_CTORFN 
#elif defined(__MINGW32__) || defined(__CYGWIN__) || defined(_MSC_VER) || defined(__VISUALC__) || defined(__BORLANDC__) || defined(__WATCOMC__)
#  define RSPFEXPORT __declspec(dllexport)
#  define RSPFIMPORT __declspec(dllimport)
#  ifdef RSPFMAKINGDLL
#    define RSPFDLLEXPORT RSPFEXPORT
#    define RSPF_DLL       RSPFEXPORT
#    define RSPFDLLEXPORT_DATA(type) RSPFEXPORT type
#    define RSPF_DLL_DATA(type) RSPFEXPORT type
#    define RSPFDLLEXPORT_CTORFN
#  else
#    define RSPFDLLEXPORT RSPFIMPORT
#    define RSPF_DLL      RSPFIMPORT
#    define RSPFDLLEXPORT_DATA(type) RSPFIMPORT type
#    define RSPF_DLL_DATA(type) RSPFIMPORT type
#    define RSPFDLLEXPORT_CTORFN
#  endif
#else /* not #if defined(_MSC_VER) */
#  define RSPFEXPORT
#  define RSPFIMPORT
#  define RSPFDLLEXPORT
#  define RSPF_DLL
#  define RSPFDLLEXPORT_DATA(type) type
#  define RSPF_DLL_DATA(type) type
#  define RSPFDLLEXPORT_CTORFN
#endif /* #if defined(_MSC_VER) */
   
/**
 * Previous DLL import export section.  Commented out, but left just in case.
 */
#if 0 /* Comment out ALL this mess! */
#if defined(_MSC_VER) || defined(__VISUALC__) || defined(__BORLANDC__) || defined(__GNUC__) || defined(__WATCOMC__)
#  if (_MSC_VER >= 1300)	/* MSVC .NET 2003 version */
#    define RSPFEXPORT __declspec(dllexport)
#    define RSPFIMPORT __declspec(dllimport)
#  else
#    define RSPFEXPORT __declspec(dllexport)
#    define RSPFIMPORT __declspec(dllimport)
#  endif
#else /* compiler doesn't support __declspec() */
#   define RSPFEXPORT
#   define RSPFIMPORT
#endif

#if defined(__WXPM__)
#  if defined (__WATCOMC__)
#    define RSPFEXPORT __declspec(dllexport)
 /*
   __declspec(dllimport) prepends __imp to imported symbols. We do NOT
   want that!
 */
#    define RSPFIMPORT
#  elif (!(defined(__VISAGECPP__) && (__IBMCPP__ < 400 || __IBMC__ < 400 )))
#    define RSPFEXPORT _Export
#    define RSPFIMPORT _Export
#  endif
#elif defined(__RSPFMAC__)
#  ifdef __MWERKS__
#    define RSPFEXPORT __declspec(export)
#    define RSPFIMPORT __declspec(import)
#  endif
#endif

#if defined(_MSC_VER)
#  pragma warning(disable:4786) /* visual c6.0 compiler */
#  pragma warning(disable:4251) /* for std:: member variable to have dll interface */
#  pragma warning(disable:4275) /* for std:: base class to have dll interface */
#  pragma warning(disable:4800) /* int forcing value to bool */
#  pragma warning(disable:4244) /* conversion, possible loss of data */

#  ifdef RSPFSINGLEDLL /* one gigantic dll, all declared export */
#    ifdef EXPORT_OSMMATRIX
#      define EXPORT_OSMMATRIX  RSPFEXPORT
#    endif
#    ifndef RSPFDLLEXPORT
#      define RSPFDLLEXPORT    RSPFEXPORT
#    endif
#    ifdef EXPORT_OSMELEV
#      define EXPORT_OSMELEV    RSPFEXPORT
#    endif
#    ifdef EXPORT_OSMSPTDATA
#      define EXPORT_OSMSPTDATA RSPFEXPORT
#    endif
#    ifdef EXPORT_ISO8211
#      define EXPORT_ISO8211    RSPFEXPORT
#    endif
#    ifdef EXPORT_OSMPROJ
#      define EXPORT_OSMPROJ    RSPFEXPORT
#    endif
#    ifndef EXPORT_OSMIMAGING
#      define EXPORT_OSMIMAGING RSPFEXPORT
#    endif
#  else /* individual dlls, each with their own import/export symbols */
#    ifndef EXPORT_OSMMATRIX
#      ifdef OSMMATRIX_EXPORTS
#        define EXPORT_OSMMATRIX RSPFEXPORT
#      else
#        define EXPORT_OSMMATRIX RSPFIMPORT
#      endif
#    endif
#    ifndef RSPFDLLEXPORT
#      ifdef OSMBASE_EXPORTS
#        define RSPFDLLEXPORT RSPFEXPORT
#      else
#        define RSPFDLLEXPORT RSPFIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMELEV
#      ifdef OSMELEV_EXPORTS
#        define EXPORT_OSMELEV RSPFEXPORT
#      else
#        define EXPORT_OSMELEV RSPFIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMSPTDATA
#      ifdef OSMSPTDATA_EXPORTS
#        define EXPORT_OSMSPTDATA RSPFEXPORT
#      else
#        define EXPORT_OSMSPTDATA RSPFIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMPROJ
#      ifdef OSMPROJ_EXPORTS
#        define EXPORT_OSMPROJ RSPFEXPORT
#      else
#        define EXPORT_OSMPROJ RSPFIMPORT
#      endif
#    endif
#    ifndef EXPORT_ISO8211
#      ifdef ISO8211_EXPORTS
#        define EXPORT_ISO8211 RSPFEXPORT
#      else
#        define EXPORT_ISO8211 RSPFIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMIMAGING
#      ifdef OSMIMAGING_EXPORTS
#        define EXPORT_OSMIMAGING RSPFEXPORT
#      else
#        define EXPORT_OSMIMAGING RSPFIMPORT
#      endif
#    endif
#  endif
#else /* defined(_MSC_VER) */
#  ifdef RSPFMAKINGDLL
#    define RSPFDLLEXPORT RSPFEXPORT
#    define RSPF_DLL       RSPFEXPORT
#    define RSPFDLLEXPORT_DATA(type) RSPFEXPORT type
#    define RSPF_DLL_DATA(type) RSPFEXPORT type
#    define RSPFDLLEXPORT_CTORFN
#  elif defined(RSPFUSINGDLL)
#    define RSPFDLLEXPORT RSPFIMPORT
#    define RSPF_DLL      RSPFIMPORT
#    define RSPFDLLEXPORT_DATA(type) RSPFIMPORT type
#    define RSPF_DLL_DATA(type) RSPFIMPORT type
#    define RSPFDLLEXPORT_CTORFN
#  else /* not making nor using DLL */
#    define RSPFDLLEXPORT
#    define RSPF_DLL
#    define RSPF_DLL_DATA(type) type
#    define RSPFDLLEXPORT_DATA(type) type
#    define RSPFDLLEXPORT_CTORFN
#  endif
#  define EXPORT_OSMMATRIX  RSPFDLLEXPORT
#  define EXPORT_OSMELEV    RSPFDLLEXPORT
#  define EXPORT_OSMSPTDATA RSPFDLLEXPORT
#  define EXPORT_OSMPROJ    RSPFDLLEXPORT
#  define EXPORT_OSMIMAGING RSPFDLLEXPORT
#endif
#endif /* End of commented out "#if 0" old DLL import/export section. */

/*
  we know that if this is defined all other types are defined
  since its included from rspfConfig.h
*/
typedef char                   rspf_int8;
typedef unsigned char          rspf_uint8;
typedef signed char            rspf_sint8;

typedef short                  rspf_int16;
typedef unsigned short         rspf_uint16;
typedef signed short           rspf_sint16;

typedef int                    rspf_int32;
typedef unsigned int           rspf_uint32;
typedef signed int             rspf_sint32;

typedef float                  rspf_float32;
typedef double                 rspf_float64;


typedef long long              rspf_int64;
typedef unsigned long long     rspf_uint64;
typedef signed long long       rspf_sint64;
   
typedef rspf_int32            rspfErrorCode;

enum rspfVertexOrdering
{
   RSPF_VERTEX_ORDER_UNKNOWN    = 0,
   RSPF_CLOCKWISE_ORDER         = 1,
   RSPF_COUNTERCLOCKWISE_ORDER  = 2
};

//---
// For histogram builders.  Note that FAST computation mode will not sample all tiles.
//---   
enum rspfHistogramMode
{
   RSPF_HISTO_MODE_UNKNOWN = 0,
   RSPF_HISTO_MODE_NORMAL  = 1,
   RSPF_HISTO_MODE_FAST    = 2
};

/*
 Corner coordinates are relative to center (0) of pixel
 or relative to the upper left of pixel (1).
*/
enum rspfPixelType
{
   RSPF_PIXEL_IS_POINT = 0,
   RSPF_PIXEL_IS_AREA  = 1
};

/*
 Definitions for scalar type identification.
*/
enum rspfScalarType
{
   RSPF_SCALAR_UNKNOWN    =  0, 
   RSPF_UINT8             =  1, /**< 8 bit unsigned integer        */
   RSPF_SINT8             =  2, /**< 8 bit signed integer          */
   RSPF_UINT16            =  3, /**< 16 bit unsigned integer       */
   RSPF_SINT16            =  4, /**< 16 bit signed integer         */
   RSPF_UINT32            =  5, /**< 32 bit unsigned integer       */
   RSPF_SINT32            =  6, /**< 32 bit signed integer         */
   RSPF_UINT64            =  7, /**< 64 bit unsigned integer       */
   RSPF_SINT64            =  8, /**< 64 bit signed integer         */
   RSPF_FLOAT32           =  9, /**< 32 bit floating point         */
   RSPF_FLOAT64           = 10, /**< 64 bit floating point         */
   RSPF_CINT16            = 11, /**< 16 bit complex integer        */
   RSPF_CINT32            = 12, /**< 32 bit complex integer        */
   RSPF_CFLOAT32          = 13, /**< 32 bit complex floating point */
   RSPF_CFLOAT64          = 14, /**< 64 bit complex floating point */
   RSPF_NORMALIZED_FLOAT  = 15, /**< 32 bit normalized floating point */
   RSPF_NORMALIZED_DOUBLE = 16, /**< 64 bit normalized floating point */

   //---
   // Below for backward compatibility only.  Please use above enums in
   // conjunction with null, min, max settings to determine bit depth.
   //---
   RSPF_USHORT11          = 17, /**< 16 bit unsigned integer (11 bits used) */
   RSPF_UCHAR             = 1,  /**< 8 bit unsigned iteger  */
   RSPF_USHORT16          = 3,  /**< 16 bit unsigned iteger */
   RSPF_SSHORT16          = 4,  /**< 16 bit signed integer  */
   RSPF_FLOAT             = 9,  /**< 32 bit floating point  */
   RSPF_DOUBLE            = 10, /**< 64 bit floating point  */
};

/*
 Definitions for interleave type identification.
*/
enum rspfInterleaveType
{
   RSPF_INTERLEAVE_UNKNOWN = 0,
   RSPF_BIL                = 1,  /* band interleaved by line */
   RSPF_BIP                = 2,  /* band interleaved by pixel */
   RSPF_BSQ                = 3,  /* band sequential */
   RSPF_BSQ_MULTI_FILE     = 4   /* band sequential in separate files */
};

/*
 Definitions for origin location.
*/
enum rspfOriginLocationType
{
   RSPF_ORIGIN_LOCATION_UNKNOWN = 0,
   RSPF_CENTER                  = 1,
   RSPF_UPPER_LEFT              = 2,
   RSPF_UPPER_LEFT_CENTER       = 3,
   RSPF_LOWER_LEFT              = 4,
   RSPF_LOWER_LEFT_CENTER       = 5
};

/*
 Definitions for coordinate system type.
*/
enum rspfCoordinateSystemType
{
   RSPF_COORDINATE_SYSTEM_UNKNOWN = 0,
   RSPF_IMAGE_SPACE               = 1,
   RSPF_GEOGRAPHIC_SPACE          = 2,
   RSPF_PROJECTED_SPACE           = 3,
   RSPF_GEOCENTRIC_SPACE          = 4
};

/*
 Definitions for coordinate system orientation mode .
*/
enum rspfCoordSysOrientMode
{
   RSPF_LEFT_HANDED  = 0,
   RSPF_RIGHT_HANDED = 1
};

/*
 Definitions for unit type.
*/
enum rspfUnitType
{
   RSPF_UNIT_UNKNOWN    = 0,
   RSPF_METERS          = 1,
   RSPF_FEET            = 2,
   RSPF_US_SURVEY_FEET  = 3,
   RSPF_DEGREES         = 4,
   RSPF_RADIANS         = 5,
   RSPF_NAUTICAL_MILES  = 6,
   RSPF_SECONDS         = 7,
   RSPF_MINUTES         = 8,
   RSPF_PIXEL           = 9,
   RSPF_MILES           = 10,
   RSPF_MILLIMETERS     = 11,
   RSPF_MICRONS         = 12,
   RSPF_CENTIMETERS     = 13,
   RSPF_YARDS           = 14,
   RSPF_INCHES          = 15,
   RSPF_KILOMETERS      = 16
};

/** Definitions for data object status. */
enum rspfDataObjectStatus
{
   RSPF_STATUS_UNKNOWN = 0,
   RSPF_NULL           = 1, // not initialized
   RSPF_EMPTY          = 2, // initialized but blank or empty
   RSPF_PARTIAL        = 3, // contains some null/invalid values
   RSPF_FULL           = 4  // all valid data
};

/** Definitions for image type identification. */
enum rspfImageType
{
   RSPF_IMAGE_TYPE_UNKNOWN       = 0,
   RSPF_TIFF_STRIP               = 1,
   RSPF_TIFF_STRIP_BAND_SEPARATE = 2,
   RSPF_TIFF_TILED               = 3,
   RSPF_TIFF_TILED_BAND_SEPARATE = 4,
   RSPF_GENERAL_RASTER_BIP       = 5,
   RSPF_GENERAL_RASTER_BIL       = 6,
   RSPF_GENERAL_RASTER_BSQ       = 7,
   RSPF_JPEG                     = 8,
   RSPF_PDF                      = 9
};

/** Definitions for metadata type identification. */
enum rspfMetadataType
{
   RSPF_META_UNKNOWN   = 0,
   RSPF_META_TFW       = 1,
   RSPF_META_JGW       = 2,
   RSPF_META_GEOM      = 3,
   RSPF_META_README    = 4,
   RSPF_META_FGDC      = 5,
   RSPF_META_ENVI      = 6
};

enum rspfByteOrder
{
   RSPF_LITTLE_ENDIAN = 0,
   RSPF_BIG_ENDIAN    = 1
};

enum rspfCompareType
{
   RSPF_COMPARE_FULL=0,      // compares full traversal if supported.  Not just instance pointers
   RSPF_COMPARE_IMMEDIATE=1  // Only immediate attributes are compared
};

#ifndef FLT_EPSILON
#  define FLT_EPSILON __FLT_EPSILON__
#endif
   
#ifndef DBL_EPSILON
#  define DBL_EPSILON __DBL_EPSILON__
#endif
   
#ifndef M_PI
#  define M_PI             ((rspf_float64)3.141592653589793238462643)
#endif
#ifndef TWO_PI
#  define TWO_PI           ((rspf_float64)(2.0 * M_PI))
#endif
#ifndef DEG_PER_RAD
#  define DEG_PER_RAD      ((rspf_float64)(180.0/M_PI))
#endif
#ifndef SEC_PER_RAD
#  define SEC_PER_RAD      ((rspf_float64)206264.8062471)
#endif
#ifndef RAD_PER_DEG
#  define RAD_PER_DEG      ((rspf_float64)(M_PI/180.0))
#endif
#ifndef MTRS_PER_FT
#  define MTRS_PER_FT      ((rspf_float64)0.3048)
#endif
#ifndef FT_PER_MTRS
#  define FT_PER_MTRS      ((rspf_float64)3.280839895013122)
#endif
#ifndef FT_PER_MILE
#  define FT_PER_MILE      ((rspf_float64)5280.0)
#endif
#ifndef SQRT_TWO_PI
#  define SQRT_TWO_PI      ((rspf_float64)2.50662827463100050242)
#endif
#ifndef SQRT_TWO_PI_DIV2
#  define SQRT_TWO_PI_DIV2 ((rspf_float64)7.07106781186547524401E-1)
#endif

#define TIFFTAG_RSPF_METADATA 50955
#define RSPF_DEFAULT_TILE_HEIGHT  ((rspf_int32)64)
#define RSPF_DEFAULT_TILE_WIDTH   ((rspf_int32)64)

/*
 NOTE Reference for U.S. feet value:
 U.S.G.S. "Map Projections - A Working Manual"
 USGS Professional paper 1395 page 51
*/
#ifndef US_METERS_PER_FT
#  define  US_METERS_PER_FT ((rspf_float64)0.3048006096)
#endif

//---
// Integer nan kept for rspfIpt.
// This should be the most negative int: -2147483648
//---
#define RSPF_INT_NAN ((rspf_sint32)0x80000000)

#define RSPF_DEFAULT_NULL_PIX_UCHAR  ((rspf_uint8)0)
#define RSPF_DEFAULT_MIN_PIX_UCHAR   ((rspf_uint8)1)
#define RSPF_DEFAULT_MAX_PIX_UCHAR   ((rspf_uint8)255)

#define RSPF_DEFAULT_NULL_PIX_UINT8  ((rspf_uint8)0)
#define RSPF_DEFAULT_MIN_PIX_UINT8   ((rspf_uint8)1)
#define RSPF_DEFAULT_MAX_PIX_UINT8   ((rspf_uint8)255)

#define RSPF_DEFAULT_NULL_PIX_SINT8  ((rspf_sint8)0x80)
#define RSPF_DEFAULT_MIN_PIX_SINT8   ((rspf_sint8)0x81)
#define RSPF_DEFAULT_MAX_PIX_SINT8   ((rspf_sint8)0x7F)

#define RSPF_DEFAULT_NULL_PIX_SINT16 ((rspf_sint16)0x8000)
#define RSPF_DEFAULT_MIN_PIX_SINT16  ((rspf_sint16)0x8001)
#define RSPF_DEFAULT_MAX_PIX_SINT16  ((rspf_sint16)0x7FFF)

#define RSPF_DEFAULT_NULL_PIX_UINT16 ((rspf_uint16)0)
#define RSPF_DEFAULT_MIN_PIX_UINT16  ((rspf_uint16)1)
#define RSPF_DEFAULT_MAX_PIX_UINT16  ((rspf_uint16)0xFFFF)

#define RSPF_DEFAULT_NULL_PIX_SINT32 ((rspf_sint32)0x80000000)
#define RSPF_DEFAULT_MIN_PIX_SINT32  ((rspf_sint32)0x80000001)
#define RSPF_DEFAULT_MAX_PIX_SINT32  ((rspf_sint32)0x7FFFFFFF)

#define RSPF_DEFAULT_NULL_PIX_UINT32 ((rspf_uint32)0)
#define RSPF_DEFAULT_MIN_PIX_UINT32  ((rspf_uint32)1)
#define RSPF_DEFAULT_MAX_PIX_UINT32  ((rspf_uint32)0xFFFFFFFF)

#define RSPF_DEFAULT_NULL_PIX_SINT64 ((rspf_sint32)0x8000000000000000)
#define RSPF_DEFAULT_MIN_PIX_SINT64  ((rspf_sint32)0x8000000000000001)
#define RSPF_DEFAULT_MAX_PIX_SINT64  ((rspf_sint32)0x7FFFFFFFFFFFFFFF)

#define RSPF_DEFAULT_NULL_PIX_UINT64 ((rspf_uint32)0)
#define RSPF_DEFAULT_MIN_PIX_UINT64  ((rspf_uint32)1)
#define RSPF_DEFAULT_MAX_PIX_UINT64  ((rspf_uint32)0xFFFFFFFFFFFFFFFF)

#define RSPF_DEFAULT_NULL_PIX_UINT11 ((rspf_uint16)0)
#define RSPF_DEFAULT_MIN_PIX_UINT11  ((rspf_uint16)1)
#define RSPF_DEFAULT_MAX_PIX_UINT11  ((rspf_uint16)0x07FF)

#define RSPF_DEFAULT_NULL_PIX_FLOAT  ((rspf_float32)-1.0/FLT_EPSILON)
#define RSPF_DEFAULT_MIN_PIX_FLOAT   ((rspf_float32)((-1.0/FLT_EPSILON) + 1))
#define RSPF_DEFAULT_MAX_PIX_FLOAT   ((rspf_float32)((1.0/FLT_EPSILON)))

#define RSPF_DEFAULT_NULL_PIX_NORM_FLOAT  ((rspf_float32)0)
#define RSPF_DEFAULT_MIN_PIX_NORM_FLOAT   ((rspf_float32)((2*FLT_EPSILON)))
#define RSPF_DEFAULT_MAX_PIX_NORM_FLOAT   ((rspf_float32)1.0)

#define RSPF_DEFAULT_NULL_PIX_DOUBLE      ((rspf_float64)-1.0/DBL_EPSILON)
#define RSPF_DEFAULT_MIN_PIX_DOUBLE       ((rspf_float64)((-1.0/DBL_EPSILON) + 1))
#define RSPF_DEFAULT_MAX_PIX_DOUBLE       ((rspf_float64)((1.0/DBL_EPSILON)))

#define RSPF_DEFAULT_NULL_PIX_NORM_DOUBLE ((rspf_float64)0)
#define RSPF_DEFAULT_MIN_PIX_NORM_DOUBLE  ((rspf_float64)((2*DBL_EPSILON)))
#define RSPF_DEFAULT_MAX_PIX_NORM_DOUBLE  ((rspf_float64)(1.0))

#define RSPF_DEFAULT_MEAN_SEA_LEVEL ((rspf_float64)0.0)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef rspfConstants_HEADER */
