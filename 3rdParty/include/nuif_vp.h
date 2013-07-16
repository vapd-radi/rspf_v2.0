/**
 * \file nuif_vp.h
 * \brief SDK to read viewport from a specified RRD in a NUI file
 * \warning Multiple threads may not share the same NUIF_VP pointer.
 *          Static variables are NOT used in the API but a very simple
 *          cache management method has been implemented making
 *          this API have the aforementioned requirement.
 * 
 * \author Rahul Thakkar
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuif_vp.h,v 2.8 2007/07/17 14:42:52 thakkarr Exp $
 * $Log: nuif_vp.h,v $
 * Revision 2.8  2007/07/17 14:42:52  thakkarr
 *  - Quite a few warnings removed for 64-bit compiles with appropriate type
 *    casting
 *  - Added code for better caching in NUIF_VP for future use.
 *
 * Revision 2.7  2007/06/01 14:19:38  jensenk
 * Changed #include<> to #include "" so the compiler pulls library headers locally,
 * and does not use the old copy in $PIXIA_INCLUDE.
 *
 * Revision 2.6  2007/05/30 15:54:58  thakkarr
 *  - Added a couple of functions for measuring data truly read from disk
 *    v/s data desired in viewport
 *  - Caching improved a little bit. Suggested use of caching in stand-alone
 *    viewers. Use strip based reads for most other cases.
 *
 * Revision 2.5  2007/05/25 19:42:23  thakkarr
 *  - MAFC read strip go to file location bug fixed
 *  - NUIF_VP debugging added
 *
 * Revision 2.4  2007/05/24 20:56:14  thakkarr
 * #include was circular - fixed
 *
 * Revision 2.3  2007/05/24 20:37:18  thakkarr
 * Doxygen documentation file fixed.
 *
 * Revision 2.2  2007/05/24 20:20:38  thakkarr
 *  - Minor updates in code.
 *  - Read by tile function checks if memory alloc reqd.
 *
 * Revision 2.1  2007/05/24 20:03:30  thakkarr
 * - Added NUIF_VP api
 * - SDK error code added to handle nui_j2k2 errors
 *
 */

#ifndef _NUIF_VP_H_HAS_BEEN_INCLUDED_
#define _NUIF_VP_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h>
#include "nuif_constants.h"
#include "nuif.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Helps manage memory for reading a viewport from NUI file RRD
 *        using best available method. Does not take advantage of tile
 *        overlapping at present. Uses strip-based reads for uncompressed
 *        non-overlapped NUI files, tile-based reads otherwise.
 */
typedef struct NUIF_VP NUIF_VP;

/**
 * \brief Creates NUIF_VP object to read viewports from a NUI file. 
 * \param nuif (in) Pointer to non-null valid open NUI file
 * \param is_cached (in) Not used
 * \param cached_tiles (in) Not used
 * \returns Properly initialized NUIF_VP object or NULL on error
 */
NUI_SDK_API
NUIF_VP *NUIF_VP_create(NUIF         *nuif, 
                        NUI_BOOLEAN   is_cached, 
                        NUI_UINT32    cached_tiles);

/**
 * \brief Destroys NUIF_VP object and frees resources
 * \param vp (in) Pointer to NUIF_VP object 
 */
NUI_SDK_API
void     NUIF_VP_destroy(NUIF_VP *vp);

/**
 * \brief Frees all memory allocated for various tile buffers
 *        Next time read viewport is called, required memory is
 *        automatically allocated.
 * \param vp (in) Pointer to NUIF_VP object 
 */
NUI_SDK_API
void     NUIF_VP_clear_memory(NUIF_VP *vp);

/**
 * \brief Reads a specified viewport for specified RRD into a buffer
 *        and returns that buffer. Only allocates memory if desired.
 * \param vp (in) Pointer to NUIF_VP object 
 * \param rrd (in) RRD to read from
 * \param top_lft_x (in) Top left X of viewport to read in terms of 
 *        RRD pixels.
 * \param top_lft_y (in) Top left Y of viewport to read in terms of 
 *        RRD pixels.
 * \param width (in) Width of viewport to read in terms of RRD pixels.
 * \param height (in) Height of viewport to read in terms of RRD pixels.
 * \returns Viewport pointer with valid image or NULL on error
 */
NUI_SDK_API
void    *NUIF_VP_read_from_level_packed(NUIF_VP   *vp,
                                        NUI_UINT32 rrd,
                                        NUI_UINT32 top_lft_x,
                                        NUI_UINT32 top_lft_y,
                                        NUI_UINT32 width,
                                        NUI_UINT32 height);

/**
 * \brief If the data was being read in terms of strips, i.e.
 *        cache was disabled and file was uncompressed and optimized,
 *        this function returns the amount of MB actually read from
 *        disk for all calls to NUIF_VP_read_from_level_packed() since 
 *        the creation of NUIF_VP
 * \param vp (in) Pointer to NUIF_VP object 
 * \param mb_read_from_disk (in) Amount if MB actually read from disk
 *        in strips. Will always be >= total MB of viewport requested.
 *        Ignored if NULL.
 * \returns The same value as received in \b mb_read_from_disk
 */
NUI_SDK_API
NUI_DOUBLE
NUIF_VP_get_strip_read_summary(NUIF_VP    *vp,
                               NUI_DOUBLE *mb_read_from_disk);

/**
 * \brief If the data was being read in terms of tiles, i.e.
 *        either caching is ON and cache size is 0 (read-tiles-no-cache)
 *        or caching is ON and cache size is valid (read-tiles-with-cache),
 *        this function returns the amount of MB actually read from
 *        disk for all calls to NUIF_VP_read_from_level_packed() since 
 *        the creation of NUIF_VP
 * \param vp (in) Pointer to NUIF_VP object 
 * \param total_tiles_accessed (in) Total tiles accessed for viewport read.
 *        Ignored if NULL.
 * \param tiles_found_in_cache (in) Of the \b total_tiles_accessed, this
 *        is the amount of tiles that were found in the cache.
 *        Ignored if NULL.
 * \param tiles_read_from_disk (in) Of the \b total_tiles_accessed, this
 *        is the amount of tiles that were not in cache and read from disk.
 *        Ignored if NULL.
 * \param mb_read_from_disk (in) Same is \b tiles_read_from_disk but in MB.
 *        Ignored if NULL.
 * \returns The same value as received in \b mb_read_from_disk
 */
NUI_SDK_API
void
NUIF_VP_get_cache_usage_summary(NUIF_VP *vp, 
                                NUI_UINT32 *total_tiles_accessed,
                                NUI_UINT32 *tiles_found_in_cache,
                                NUI_UINT32 *tiles_read_from_disk,
                                NUI_DOUBLE *mb_read_from_disk);

/**
 * \brief Internal use only. Enables internal debugging printout
 * \param state (in) If true, debugging is enabled. If false, debugging
 *        is off. Default is off.
 */
NUI_SDK_API
void     NUIF_VP_set_debug_state(NUI_BOOLEAN state);

#ifdef __cplusplus
}
#endif

#endif /* _NUIF_VP_H_HAS_BEEN_INCLUDED_ */

/**
 * nuif_vp.h ends
 */
