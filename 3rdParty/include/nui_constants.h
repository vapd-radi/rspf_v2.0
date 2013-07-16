/*
 * Copyright (C) 2003 Pixia Corp. All rights reserved.
 * All tags in header are for Automatic document generation
 */
/**
 * \file nui_constants.h
 * \section NUI_CONSTANTS
 * \brief Defines constants that are required by all applications
 *        that use Pixia Corp .nui technology.
 *
 * To add constants to this file, you must consult with
 * PIXIA R&D.
 *
 * \author Rahul Thakkar, Saurabh Sood.
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui/nui_constants.h,v 2.5 2007/05/25 12:43:56 jensenk Exp $
 * $Log: nui_constants.h,v $
 * Revision 2.5  2007/05/25 12:43:56  jensenk
 * Added min/max for NUI types based on constants in limits.h.
 * Added float and double macros for real-zero and real-equal comparisons.
 *
 * Revision 2.4  2007/04/24 14:53:27  soods
 * Fixed the sizeof for float and double (NUI_DATA_TYPE)
 *
 * Revision 2.3  2007/04/24 13:18:26  soods
 * Corrected min values of float and double in NUI_DATA_TYPE
 *
 * Revision 2.2  2007/04/19 17:29:54  soods
 * Added NUI_DATA_TYPE enum with supporting macros
 *
 * Revision 2.1  2007/03/07 17:41:22  jensenk
 * Added missing VS2005 solutions and new sys_make_all_vc8 scripts.
 *
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.11  2006/02/17 20:12:46  thakkarr
 * Better formatting
 *
 * Revision 1.10  2005/06/28 14:59:31  thakkarr
 * Added ability in UNIX to replace WINDOWS backslash with UNIX fwd slash
 *
 * Revision 1.9  2005/02/18 18:36:07  soods
 *  Ported to Mac OS (Darwin)
 *
 * Revision 1.8  2005/02/09 18:56:30  thakkarr
 * Improved nui_constants with better comments and defines
 *
 * Revision 1.7  2005/02/07 12:25:56  thakkarr
 * Added NUI_BOOLEAN
 *
 * Revision 1.6  2005/01/28 11:20:09  soods
 *  Updated the includes for various platform
 *
 * Revision 1.5  2005/01/17 11:22:27  soods
 *  Added the comments for Struct Alignement Tag and NUI_SDK_API
 *  Added a renamed version of Struct alignement tag for GCC
 *
 * Revision 1.4  2005/01/14 19:01:28  soods
 *  Added the NUI_SDK_API for DLL export functions on Windows
 *
 * Revision 1.3  2005/01/10 16:29:00  soods
 *  Included GCC compiler specific definations for enforcing struct padding
 *
 * Revision 1.2  2004/11/24 15:33:27  soods
 * Corrected format argument in NUI_UINT64_TO_ASCII to "%lld"
 *
 * Revision 1.1  2004/10/27 20:40:09  Administrator
 * Added to cvs
 *
 */

#ifndef _NUI_CONSTANTS_H_HAS_BEEN_INCLUDED_
#define _NUI_CONSTANTS_H_HAS_BEEN_INCLUDED_

#include <limits.h>
#include <float.h>
#include <math.h>

/* Detect which platform is being used. */
#if   defined(_WIN32) || defined(_WIN64) || defined (_WINDOWS)
    #define NUI_PLATFORM_WINDOWS
    #include <windows.h>
    #include <io.h>
#elif defined(__linux__)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_LINUX
    #include <unistd.h>
#elif defined(__sgi) || defined(sgi)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_IRIX
    #include <unistd.h>
#elif defined(__APPLE__)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_MACOS
    #include <unistd.h>
#elif defined(_AIX)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_AIX
    #include <unistd.h>
#elif defined(__sparc__)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_SPARC
    #include <unistd.h>
#elif defined(__FreeBSD__) || defined (_FREEBSD4) || defined (_FREEBSD5)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_FREEBSD
    #include <unistd.h>
#elif defined(__OpenBSD__) || defined (_OPENBSD)
    #define NUI_PLATFORM_UNIX
    #define NUI_PLATFORM_UNIX_OPENBSD
    #include <unistd.h>
#endif


/*******************************************************
 * The structure padding for different compilers and/or
 * different platform varies. The header structure should
 * be exactly same for NUI portability. The following
 * tags override the default struct padding by gcc compiler
 * by aligning it to nearest multiple of offset from start.
 * Windows Struct configuration is chosen as a base.
 * Example:
 * _NUI_STRUCT_UNIX_ALIGN_8_ alignes the start offset of
 * struct member to nearest multiple of 8
 * Note: This tag can only increase padding to decrease
 * padding programmer should use #pragma pack(n) directive
 *******************************************************/

#ifdef NUI_PLATFORM_UNIX_LINUX		/* obsolete, will be discontinued */
    #define _NUI_STRUCT_UNIX_ALIGN_1_   __attribute__ ((aligned (1 )))
    #define _NUI_STRUCT_UNIX_ALIGN_2_   __attribute__ ((aligned (2 )))
    #define _NUI_STRUCT_UNIX_ALIGN_4_   __attribute__ ((aligned (4 )))
    #define _NUI_STRUCT_UNIX_ALIGN_8_   __attribute__ ((aligned (8 )))
    #define _NUI_STRUCT_UNIX_ALIGN_16_  __attribute__ ((aligned (16)))
    #define _NUI_STRUCT_UNIX_ALIGN_32_  __attribute__ ((aligned (32)))
#elif defined(NUI_PLATFORM_UNIX_MACOS) /* obsolete, will be discontinued */
    #define _NUI_STRUCT_UNIX_ALIGN_1_   __attribute__ ((aligned (1 )))
    #define _NUI_STRUCT_UNIX_ALIGN_2_   __attribute__ ((aligned (2 )))
    #define _NUI_STRUCT_UNIX_ALIGN_4_   __attribute__ ((aligned (4 )))
    #define _NUI_STRUCT_UNIX_ALIGN_8_   __attribute__ ((aligned (8 )))
    #define _NUI_STRUCT_UNIX_ALIGN_16_  __attribute__ ((aligned (16)))
    #define _NUI_STRUCT_UNIX_ALIGN_32_  __attribute__ ((aligned (32)))
#else
    #define _NUI_STRUCT_UNIX_ALIGN_1_
    #define _NUI_STRUCT_UNIX_ALIGN_2_
    #define _NUI_STRUCT_UNIX_ALIGN_4_
    #define _NUI_STRUCT_UNIX_ALIGN_8_
    #define _NUI_STRUCT_UNIX_ALIGN_16_
    #define _NUI_STRUCT_UNIX_ALIGN_32_
#endif

#ifdef NUI_PLATFORM_UNIX_LINUX		/* Use this henceforth */
    #define _NUI_STRUCT_GCC_ALIGN_1_   __attribute__ ((aligned (1 )))
    #define _NUI_STRUCT_GCC_ALIGN_2_   __attribute__ ((aligned (2 )))
    #define _NUI_STRUCT_GCC_ALIGN_4_   __attribute__ ((aligned (4 )))
    #define _NUI_STRUCT_GCC_ALIGN_8_   __attribute__ ((aligned (8 )))
    #define _NUI_STRUCT_GCC_ALIGN_16_  __attribute__ ((aligned (16)))
    #define _NUI_STRUCT_GCC_ALIGN_32_  __attribute__ ((aligned (32)))
#else
    #define _NUI_STRUCT_GCC_ALIGN_1_
    #define _NUI_STRUCT_GCC_ALIGN_2_
    #define _NUI_STRUCT_GCC_ALIGN_4_
    #define _NUI_STRUCT_GCC_ALIGN_8_
    #define _NUI_STRUCT_GCC_ALIGN_16_
    #define _NUI_STRUCT_GCC_ALIGN_32_
#endif

/*!< _NUI_STRUCT_GCC_ALIGN_... constants are GCC compiler's way of
     enforcing structure padding. Pixia Corp's has a single code base
     for all platforms. As a result, a structure's byte alignment
     becomes a problem from compiler to compiler. GCC has a way to
     force this alignment to be one way. This is done using the
     constants _NUI_STRUCT_GCC_ALIGN_...
*/

/*******************************************************
 * NUI_SDK_API on Windows platform defines the tag to export
 * a library function to a DLL file (dynamic linked lib).
 *******************************************************/
#define NUI_SDK_API
#if defined(NUI_PLATFORM_WINDOWS)
    #if defined(_NUI_SDK_EXPORT_HAS_BEEN_DEFINED_)
        #undef NUI_SDK_API
        #define NUI_SDK_API __declspec(dllexport)
    #endif
#endif

/*******************************************************
 * Data types re-defined for convenience and consistency
 *******************************************************/
#ifndef NUI_INT16
#define NUI_INT16  short
/*!< NUI 16-bit integer */
#endif

#ifndef NUI_UINT16
#define NUI_UINT16 unsigned short
/*!< NUI unsigned 16-bit integer */
#endif

#ifndef NUI_INT32
#define NUI_INT32  int
/*!< NUI 32-bit integer */
#endif

#ifndef NUI_UINT32
#define NUI_UINT32 unsigned int
/*!< NUI unsigned 32-bit integer */
#endif

#ifndef NUI_LONG
#define NUI_LONG   long
/*!< NUI long integer */
#endif

#ifndef NUI_ULONG
#define NUI_ULONG  unsigned long
/*!< NUI unsigned long integer */
#endif


#if defined (NUI_PLATFORM_WINDOWS) /* For Windows platforms */
    #ifndef NUI_UINT64
    #define NUI_UINT64 unsigned __int64
    /*!< NUI unsigned 64-bit integer */
    #endif
    #ifndef NUI_INT64
    #define NUI_INT64  __int64
    /*!< NUI signed 64-bit integer */
    #endif
#else /* For UNIX platforms */
    #ifndef NUI_UINT64
    #define NUI_UINT64 unsigned long long
    /*!< NUI unsigned 64-bit integer */
    #endif
    #ifndef NUI_INT64
    #define NUI_INT64  long long
    /*!< NUI signed 64-bit integer */
    #endif
#endif

#ifndef NUI_FLOAT
#define NUI_FLOAT  float
/*!< NUI single precision number */
#endif

#ifndef NUI_DOUBLE
#define NUI_DOUBLE double
/*!< NUI double precision number */
#endif

#ifndef NUI_CHAR
#define NUI_CHAR   char
/*!< NUI character */
#endif

#ifndef NUI_UCHAR
#define NUI_UCHAR  unsigned char
/*!< NUI unsigned character */
#endif

/**
 * \brief This is a special data type for presenting boolean states
 */
typedef enum {
    NUI_BOOLEAN_TRUE = 1, /*!< Same as NUI_TRUE */
    NUI_BOOLEAN_FALSE = 0 /*!< Same as NUI_FALSE */
} NUI_BOOLEAN;

/*
 * Integral and non-integral data type size limits
 */
#define NUI_UCHAR_MAX   UCHAR_MAX      /*!< Max NUI_UCHAR value */
#define NUI_CHAR_MIN	SCHAR_MIN      /*!< Min NUI_CHAR value */
#define NUI_CHAR_MAX	SCHAR_MAX      /*!< Max NUI_CHAR value */
#define NUI_INT16_MIN	SHRT_MIN       /*!< Min NUI_INT16 value */
#define NUI_INT16_MAX	SHRT_MAX       /*!< Max NUI_INT16 value */
#define NUI_UINT16_MAX	USHRT_MAX      /*!< Max NUI_UINT16 value */
#define NUI_INT32_MIN	INT_MIN        /*!< Min NUI_INT32 value */
#define NUI_INT32_MAX	INT_MAX        /*!< Max NUI_INT32 value */
#define NUI_UINT32_MAX	UINT_MAX       /*!< Max NUI_UINT32 value */
#define NUI_INT64_MIN	LLONG_MIN      /*!< Min NUI_INT64 value */
#define NUI_INT64_MAX	LLONG_MAX      /*!< Max NUI_INT64 value */
#define NUI_UINT64_MAX	ULLONG_MAX     /*!< Max NUI_UINT64 value */
#define NUI_FLOAT_MIN   (-FLT_MAX)     /*!< Min NUI_FLOAT value */
#define NUI_FLOAT_MAX	FLT_MAX        /*!< Max NUI_FLOAT value */
#define NUI_FLOAT_ZERO  (1.0e-6f)      /*!< Zero/roundoff tolerance for
                                          comparison of NUI_FLOAT values */
#define NUI_DOUBLE_MIN  (-DBL_MAX)     /*!< Min NUI_DOUBLE value */
#define NUI_DOUBLE_MAX	DBL_MAX        /*!< Max NUI_DOUBLE value */
#define NUI_DOUBLE_ZERO (1.0e-12)      /*!< Zero/roundoff tolerance for
                                            comparison of NUI_DOUBLE values */

#define NUI_FLOAT_EQUAL(f1,f2)      (fabs((f1)-(f2)) <= NUI_FLOAT_ZERO)
/*!< Macro to test whether two NUI_FLOAT values are equal with consideration
     for roundoff errors */
#define NUI_FLOAT_NOT_EQUAL(f1,f2)  (fabs((f1)-(f2)) >  NUI_FLOAT_ZERO)
/*!< Macro to test whether two NUI_FLOAT values are not equal with consideration
     for roundoff errors */
#define NUI_DOUBLE_EQUAL(f1,f2)     (fabs((f1)-(f2)) <= NUI_DOUBLE_ZERO)
/*!< Macro to test whether two NUI_DOUBLE values are equal with consideration
     for roundoff errors */
#define NUI_DOUBLE_NOT_EQUAL(f1,f2) (fabs((f1)-(f2)) >  NUI_DOUBLE_ZERO)
/*!< Macro to test whether two NUI_DOUBLE values are not equal with consideration
     for roundoff errors */

/**
 * \brief Specifies the basic data types
 *        Values are matched with that of NUIF_BITS_PER_CHANNEL
 *        for legacy reasons.
 */
typedef enum {
    NUI_DATA_TYPE_CHAR = 508,
                /*!< Signed char value(s) */
    NUI_DATA_TYPE_UCHAR = 8,
                /*!< Unsigned char value(s) */
    NUI_DATA_TYPE_UINT16 = 516,
                /*!< Unsigned 16 bit value(s) */
    NUI_DATA_TYPE_UINT32 = 532,
                /*!< Unsigned 32 bit value(s) */
    NUI_DATA_TYPE_UINT64 = 564,
                /*!< Unsigned 64 bit value(s) */
    NUI_DATA_TYPE_INT16 = 16,
                /*!< Signed 16 bit value(s) */
    NUI_DATA_TYPE_INT32 = 32,
                /*!< Signed 32 bit value(s) */
    NUI_DATA_TYPE_INT64 = 64,
                /*!< Signed 64 bit value(s) */
    NUI_DATA_TYPE_FLOAT = 132,
                /*!< Single precision float value(s) */
    NUI_DATA_TYPE_DOUBLE = 164
                /*!< Double precision float value(s) */
} NUI_DATA_TYPE;

/**
 * \brief Macro to get bits stored in the data type
 */
#define NUI_DATA_TYPE_GET_BITS(type) (\
(type == NUI_DATA_TYPE_CHAR) ? 8 : \
(type == NUI_DATA_TYPE_UCHAR) ? 8 : \
(type == NUI_DATA_TYPE_INT16) ? 16 : \
(type == NUI_DATA_TYPE_UINT16) ? 16 : \
(type == NUI_DATA_TYPE_INT32) ? 32 : \
(type == NUI_DATA_TYPE_UINT32) ? 32 : \
(type == NUI_DATA_TYPE_INT64) ? 64 : \
(type == NUI_DATA_TYPE_UINT64) ? 64 : \
(type == NUI_DATA_TYPE_FLOAT) ? sizeof(float) * 8: \
(type == NUI_DATA_TYPE_DOUBLE) ? sizeof(NUI_DOUBLE) * 8: \
0)

/**
 * \brief Macro to get bytes stored in the data type
 */
#define NUI_DATA_TYPE_GET_BYTES(type) (\
(type == NUI_DATA_TYPE_CHAR) ? 1 : \
(type == NUI_DATA_TYPE_UCHAR) ? 1 : \
(type == NUI_DATA_TYPE_INT16) ? 2 : \
(type == NUI_DATA_TYPE_UINT16) ? 2 : \
(type == NUI_DATA_TYPE_INT32) ? 4 : \
(type == NUI_DATA_TYPE_UINT32) ? 4 : \
(type == NUI_DATA_TYPE_INT64) ? 8 : \
(type == NUI_DATA_TYPE_UINT64) ? 8 : \
(type == NUI_DATA_TYPE_FLOAT) ? sizeof(float) : \
(type == NUI_DATA_TYPE_DOUBLE) ? sizeof(NUI_DOUBLE) : \
0)

/**
 * \brief Macro to get maximum value allowed for data type
 */
#define NUI_DATA_TYPE_GET_MAX(type) (\
(type == NUI_DATA_TYPE_CHAR)   ? NUI_CHAR_MAX : \
(type == NUI_DATA_TYPE_UCHAR)  ? NUI_UCHAR_MAX : \
(type == NUI_DATA_TYPE_INT16)  ? NUI_INT16_MAX : \
(type == NUI_DATA_TYPE_UINT16) ? NUI_UINT16_MAX : \
(type == NUI_DATA_TYPE_INT32)  ? NUI_INT32_MAX : \
(type == NUI_DATA_TYPE_UINT32) ? NUI_UINT32_MAX : \
(type == NUI_DATA_TYPE_INT64)  ? NUI_INT64_MAX : \
(type == NUI_DATA_TYPE_UINT64) ? NUI_UINT64_MAX : \
(type == NUI_DATA_TYPE_FLOAT)  ? FLT_MAX : \
(type == NUI_DATA_TYPE_DOUBLE) ? DBL_MAX : \
0)

/**
 * \brief Macro to get minimum value allowed for data type
 */
#define NUI_DATA_TYPE_GET_MIN(type) (\
(type == NUI_DATA_TYPE_CHAR)   ? NUI_CHAR_MIN : \
(type == NUI_DATA_TYPE_UCHAR)  ? 0 : \
(type == NUI_DATA_TYPE_INT16)  ? NUI_INT16_MIN : \
(type == NUI_DATA_TYPE_UINT16) ? 0 : \
(type == NUI_DATA_TYPE_INT32)  ? NUI_INT32_MIN : \
(type == NUI_DATA_TYPE_UINT32) ? 0 : \
(type == NUI_DATA_TYPE_INT64)  ? NUI_INT64_MIN : \
(type == NUI_DATA_TYPE_UINT64) ? 0 : \
(type == NUI_DATA_TYPE_FLOAT)  ? -FLT_MAX : \
(type == NUI_DATA_TYPE_DOUBLE) ? -DBL_MAX : \
0)

/*
 * Useful constants
 */
#define NUI_MB              (1048576) /*!< 1024 bytes x 1024 bytes */
#define NUI_GB              (1073741824) /*!< NUI_MB * 1024 */
#define NUI_PI              (3.1415926535897932384626433) /*!< PI */
#define NUI_MAX_FILENAMELEN (32768)   /*!< Large string - Unicode support */

#ifndef NUI_TRUE
#define NUI_TRUE			1 /*!< For Boolean returns in all Pixia API */
#define NUI_FALSE			0 /*!< For Boolean returns in all Pixia API */
#endif

#ifndef NUI_ERROR
#define NUI_ERROR			-1 /*!< Standard return error code */
#define NUI_SUCCESS			0  /*!< Standard return error code */
#endif

/*
 * Standard Extension and Path delimiters for Windows and Unix
 */
#define NUI_EXT_DELIMITER	'.' /*!< Extension delimiter */
#if defined(NUI_PLATFORM_WINDOWS)
#define NUI_PATH_DELIMITER      0x5C /*!< '\' Path delimiter for Windows */
#define NUI_PATH_DELIMITER_ALT  0x2F /*!< '/' Path delimiter Windows & UNIX */
#elif defined (NUI_PLATFORM_UNIX)
#define NUI_PATH_DELIMITER      0x2F /*!< '/' Path delimiter for UNIX */
#define NUI_PATH_DELIMITER_ALT  0x2F /*!< '/' Path delimiter Windows & UNIX */
#define NUI_PATH_DELIMITER_WIN  0x5C /*!< '\' Path delimiter for Windows */
#endif

#if defined(NUI_PLATFORM_WINDOWS) /* This applies only to Windows */
#define NUI_INT64_TO_ASCII(NUMBER, STRING) _i64toa((NUMBER),(STRING),10)
/*!< Converts a 64 bit signed integer to ascii on Microsoft Windows */
#define NUI_UINT64_TO_ASCII(NUMBER, STRING) _ui64toa((NUMBER),(STRING),10)
/*!< Converts a 64 bit unsigned integer to ascii on Microsoft Windows */
#elif defined(NUI_PLATFORM_UNIX) /* This else applies only to UNIX */
#define NUI_INT64_TO_ASCII(NUMBER, STRING) sprintf((STRING),"%lld",(NUMBER))
/*!< Converts a 64 bit signed integer to ascii (not working) */
#define NUI_UINT64_TO_ASCII(NUMBER, STRING) sprintf((STRING),"%lld",(NUMBER))
/*!< Converts a 64 bit unsigned integer to ascii (not working) */
#endif

/*
 * Turn off some really popular warnings from VS2005 which are true for
 * Windows but not true for all other platforms.  Specifically, tell it
 * that we do not want warnings for CRT security flaws, nor to suggest
 * replacing all POSIX-compliant API names with ISO C++ names.
 */
#if defined(NUI_PLATFORM_WINDOWS) && (_MSC_VER >= 1400)
	/* No messages about using "_unlink" ISO C++ instead of "unlink" POSIX */
#	pragma warning (disable:4996)
	/* No warnings about the unsafe (Microsoft-supplied) CRT. */
#	ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

#endif /* _NUI_CONSTANTS_H_HAS_BEEN_INCLUDED_ */

/*
 * nui_constants.h ends
 */

