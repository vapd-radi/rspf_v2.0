//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken, original code from Thomas G. Lane
//
// Description:
// Code interfaces to use with jpeg-6b library to read a jpeg image from
// memory.
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfJpegMemSrc_HEADER
#define rspfJpegMemSrc_HEADER

#include <rspf/base/rspfConstants.h> /** for RSPF_DLL export macro */
extern "C"
{
#include <cstdio>                      /** for size_t  */
#include <csetjmp>                     /** for jmp_buf */
#include <jpeglib.h>                   /** for jpeg stuff */


/** @brief Extended error handler struct. */
struct RSPF_DLL rspfJpegErrorMgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct rspfJpegErrorMgr* rspfJpegErrorPtr;

/**
 * @brief Error routine that will replace jpeg's standard error_exit method.
 */
RSPF_DLL void rspfJpegErrorExit (j_common_ptr cinfo);

/**
 * @brief Method which uses memory instead of a FILE* to read from.
 * @note Used in place of "jpeg_stdio_src(&cinfo, infile)".
 */
RSPF_DLL void rspfJpegMemorySrc (j_decompress_ptr cinfo,
                                   const JOCTET * buffer,
                                   std::size_t bufsize);
}
#endif /* #ifndef rspfJpegMemSrc_HEADER */
