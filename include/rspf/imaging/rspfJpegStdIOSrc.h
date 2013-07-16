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
#ifndef rspfJpegStdIOSrc_HEADER
#define rspfJpegStdIOSrc_HEADER

#include <rspf/base/rspfConstants.h> /** for RSPF_DLL export macro */
extern "C"
{
#include <cstdio>                      /** for size_t  */
#include <csetjmp>                     /** for jmp_buf */
#include <jpeglib.h>                   /** for jpeg stuff */
   //#include <jinclude.h>
   //#include <jerror.h>
   /**
    * @brief Method which uses memory instead of a FILE* to read from.
    * @note Used in place of "jpeg_stdio_src(&cinfo, infile)".
    */
   RSPF_DLL void rspfJpegStdIOSrc ( j_decompress_ptr cinfo,
                                     FILE* infile);
}
#endif /* #ifndef rspfJpegMemSrc_HEADER */
