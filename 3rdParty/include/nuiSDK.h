/* 
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 */
/**
 * \file nuiSDK.h
 * \section NUISDK
 * \brief Basic header that includes all released headers
 * 
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * \warning
 * If you pass a NULL pointer to a function that explicitly expects a non-NULL
 * pointer in the documentation, that function in the nuiSDK will terminate
 * your application using an assertion statement. The case of a 
 * NULL pointer getting passed to this function denotes a catastrophic
 * bug and we recomment that it be fixed by the API user immediately.
 * In all applications, if a non-NULL pointer is expected by a
 * function,
 *
 *
 * \warning
 * All new additions to nuiSDK API must be approved by the Project Manager
 * for nuiSDK.
 *
 * \authors Rahul Thakkar, Saurabh Sood
 *
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuiSDK.h,v 2.2 2007/05/24 20:37:18 thakkarr Exp $
 * $Log: nuiSDK.h,v $
 * Revision 2.2  2007/05/24 20:37:18  thakkarr
 * Doxygen documentation file fixed.
 *
 * Revision 2.1  2007/05/24 20:03:30  thakkarr
 * - Added NUIF_VP api
 * - SDK error code added to handle nui_j2k2 errors
 *
 * Revision 2.0  2006/11/16 21:13:04  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:56:17  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.7  2005/08/03 19:50:28  thakkarr
 * Added NUI_J2K2 to nuiSDK non-static version
 *
 * Revision 1.6  2005/03/16 23:17:38  thakkarr
 * Created "_i" files
 *
 * Revision 1.5  2005/03/15 22:53:02  thakkarr
 * Added a suite of NUI_SUPPORT functions to match ALLOC_WIN library
 * functions and NUI_ library functions that we would like to
 * export to the world to use as well as for Pixia developers to use.
 *
 * Revision 1.4  2005/01/27 19:38:47  thakkarr
 * Intermediate commit
 * Rows and Column reporting has been made such that the entire
 * image is treated as a matrix of cells side-by-side. This means
 * we cannot take advantage of the "RDBMS" features of the
 * NUI 2.2 format for applications, however, most applications that
 * assume the break-down of a pyramid level into adjacent tiles
 * can easily use this method. Eventually, read and row,column processing
 * functions that take advantage of the "RDBMS" feature will be made
 * available.
 *
 * Revision 1.3  2005/01/19 14:58:23  thakkarr
 * nuif.c/h and nuif_get.c/h completed
 *
 * Revision 1.2  2005/01/17 19:05:46  thakkarr
 * Starting implementation of core code from prototype to main body.
 *
 * Revision 1.1  2005/01/13 16:54:33  soods
 * 	Skeletal Header File Only
 *
 *
 *
 */

#ifndef _NUISDK_H_HAS_BEEN_INCLUDED_
#define _NUISDK_H_HAS_BEEN_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif


#include <nui_constants.h> /*!< Pixia Corp global constants */

/*****************************************************************
 * Header files with NUI_ prefix deal with support functions that
 * may come in handy for a developer when using the NUIF interface
 * that specifically focuses on NUI 2.2 files.
 *****************************************************************/
#include "nui_support.h"    /*!< NUI_SUPPORT API */
#include "nui_cdf.h"        /*!< NUI_CDF API */
#include "nui_eta.h"        /*!< NUI_ETA API */
#include "nui_arg.h"        /*!< NUI_ARG API */
#include "nui_j2k2.h"       /*!< NUI_J2K2 API */

/*****************************************************************
 * Header files with NUIF prefix deal with the NUIF API. It is the
 * primary API to read, write and manage NUI 2.2 files.
 *****************************************************************/
#include "nuif_constants.h" /*!< Constants for NUIF */
#include "nuif_types.h"     /*!< Data types for NUIF */
#include "nuif.h"           /*!< Base functions for NUI file API */
#include "nuif_get.h"       /*!< Get information about NUI file */
#include "nuif_read.h"      /*!< Read raster data from NUI file */
#include "nuif_vp.h"        /*!< Read from NUI file by managed viewport */

/****************************************************************
 * Error codes, and error management functions for nuiSDK.
 ****************************************************************/
#define NUI_SDK_ERR_MEMORY      1 /*!< Error code: Out of memory */
#define NUI_SDK_ERR_NUI_ETA     2 /*!< Error code: NUI_ETA API error */
#define NUI_SDK_ERR_NUI_ARG     3 /*!< Error code: NUI_ARG API error */
#define NUI_SDK_ERR_NUI_CDF     4 /*!< Error code: NUI_CDF API error */
#define NUI_SDK_ERR_NUI_SUPPORT 5 /*!< Error code: NUI_SUPPORT API error */
#define NUI_SDK_ERR_NUIF        6 /*!< Error code: NUIF API error */
#define NUI_SDK_ERR_SYSTEM      7 /*!< Error code: strerror(errno) */
#define NUI_SDK_ERR_NUI         8 /*!< Error code: NUI API error */
#define NUI_SDK_ERR_NUI_J2K2    9 /*!< Error code: NUI_J2K2 API error */

extern NUI_INT32 nuiSDK_errno; 
/*!< Error code that holds information on the last error that occurred 
 *   in the nuiSDK. Retrieve a meaningful description of this error using
 *   nuiSDK_error_string() */

/**
 * \brief Returns a statically allocated error string that briefly
 *        describes the nature of error that occurred due to any of
 *        the functions in the nuiSDK interface.
 *
 * The function is not re-entrant. The same function will eventually
 * use a simple spin-locking IPC primitive to lock execution of the
 * calling thread to ensure that during the execution of this function,
 * the error string is correctly populated. In either case, please ensure
 * that the error string is immediately and atomically copied into
 * your own application's error handler string. This is only if you are 
 * using multi-threading.
 *
 * \returns A statically allocated error string that reflects the
 *          last error that occurred in the nuiSDK interface.
 * \note This error string, being statically allocated, will be 
 *       over-written if another error occurred and this function
 *       got called. Please copy the return value into a 
 *       separate character string local to your own application.
 */
NUI_SDK_API
NUI_CHAR *nuiSDK_error_string(void);

#ifdef __cplusplus
}
#endif

#endif /* _NUISDK_H_HAS_BEEN_INCLUDED_ */

/*
 * nuiSDK.h ends
 */
