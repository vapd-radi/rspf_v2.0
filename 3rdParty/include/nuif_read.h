/**
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 */
/**
 * \file nuif_read.h
 * \section NUIF_READ
 * \brief API to read raster data from a nui file
 * 
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * 
 * \authors Rahul Thakkar, Saurabh Sood
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuif_read.h,v 2.0 2006/11/16 21:13:04 jensenk Exp $
 * $Log: nuif_read.h,v $
 * Revision 2.0  2006/11/16 21:13:04  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:56:17  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.9  2005/08/04 13:15:06  thakkarr
 * Bug fix: Steamed read functions had not been exported. Now they are.
 *
 * Revision 1.8  2005/06/10 22:02:16  thakkarr
 * Streamed tile read implemented and tested
 *
 * Revision 1.7  2005/04/01 20:12:44  thakkarr
 * Added better comments
 *
 * Revision 1.6  2005/03/16 23:17:38  thakkarr
 * Created "_i" files
 *
 * Revision 1.5  2005/03/15 22:53:02  thakkarr
 * Added a suite of NUI_SUPPORT functions to match ALLOC_WIN library
 * functions and NUI_ library functions that we would like to
 * export to the world to use as well as for Pixia developers to use.
 *
 * Revision 1.4  2005/01/28 17:16:45  thakkarr
 * Finished SDK
 * Testing and cleanup begin
 *
 * Revision 1.3  2005/01/25 17:42:59  thakkarr
 * Added comments
 *
 * Revision 1.2  2005/01/19 14:58:23  thakkarr
 * nuif.c/h and nuif_get.c/h completed
 *
 * Revision 1.1  2005/01/13 18:33:50  soods
 *  Skeletal File modified
 *
 * Revision 1.1  2005/01/13 17:00:14  soods
 * 	Template Only
 *
 *
 *
 */

#ifndef _NUIF_READ_H_HAS_BEEN_INCLUDED_
#define _NUIF_READ_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h>  /*!< Pixia Corp global constants */
#include "nuif_constants.h" /*!< Constants for NUIF */
#include "nuif_types.h"     /*!< Data types for NUIF */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This function allocates memory for 1 tile for the specified NUIF
 *        object. Amount of memory allocated depends on the tile width and
 *        height, and number of bytes per pixel. The returned memory
 *        buffer is initialized to zero value.
 *
 * \warning This function is \b obsolete. Use NUI_SUPPORT_malloc() and 
 *          NUIF_LEVEL_get_tile_data_size_in_bytes(). If you allocate
 *          memory using NUI_SUPPORT_malloc(), free it using 
 *          NUI_SUPPORT_free().
 *        
 * \param nuif (in) Pointer to a valid non-NULL NUIF object that points to
 *        an open NUI 2.2 file that has to be read.
 * \param level_number (in) Pyramid level number (starting from 0) of the
 *        level you plan to read the tile from.
 * \param bytes_allocated (out) Pointer to location that will hold the number
 *        of bytes allocated in the memory block returned by this function.
 * \returns A pointer to a byte array that points to memory allocated for
 *        for holding one tile with all bands in it. The amount of bytes 
 *        allocated are placed in \b bytes_allocated. Function returns NULL
 *        if the file was originally created to hold raster data in planar
 *        format or if the system runs out of memory. The returned memory
 *        buffer is initialized to zero value.
 *
 * \sa NUI_SUPPORT_free(), NUIF_read_packed_tile_from_level()
 *
 * \warning If the NUI 2.2 file stores data in a planar fashion, the function
 *        returns NULL and sets error code. 
 *
 * \note  A NUIF tile has image data that may be stored in a packed or planar
 *        format. Packed format means if you have 4 bands (channels) in an
 *        image, the pixels for the 4 bands are stored as B0B1B2B3B0B1B2B3...
 *        Planar format means if you have 4 bands in an image, the pixels
 *        for the 4 bands are stored as B0B0B0B0... B1B1B1B1... B2B2B2B2... 
 *        B3B3B3B3...
 *
 */
NUI_SDK_API
NUI_UCHAR *NUIF_allocate_memory_for_packed_tile(NUIF       *nuif, 
                                                NUI_UINT32  level_number,
                                                NUI_UINT32 *bytes_allocated);

/**
 * \brief This function frees memory that was allocated using 
 *        NUIF_allocate_memory_for_packed_tile()
 *
 * \warning This function is \b obsolete. Use NUI_SUPPORT_malloc() and 
 *          NUIF_LEVEL_get_tile_data_size_in_bytes() to allocate memory and
 *          NUI_SUPPORT_free() to free the memory that was allocated using
 *          NUI_SUPPORT_malloc().
 *        
 * \param pointer (in) Pointer to a non-NULL memory block that was allocated
 *        using NUIF_allocate_memory_for_packed_tile().
 *
 * \sa NUI_SUPPORT_malloc(), NUIF_read_packed_tile_from_level()
 *
 */
NUI_SDK_API
void       NUIF_free_memory_for_packed_tile(NUI_UCHAR *pointer);

/**
 * \brief Reads a tile with packed color data, from a NUI file pyramid level.
 *
 * \param nuif (in) Non-NULL pointer to an open NUI 2.2 file object
 * \param level_number (in) Pyramid level number (starting from 0) of the
 *        level to read the tile from.
 * \param row (in) Row number of the tile within this pyramid level.
 * \param column (in) Column number of the tile within this pyramid level.
 * \param return_buffer (in/out) Pointer to a memory buffer that reserves
 *        memory for holding one tile. The function reads the
 *        tile from the file on your storage device and places it in this
 *        memory buffer. This memory buffer must begin on a memory page
 *        boundary.
 * \returns 0 upon successfully reading the tile into memory or -1 upon
 *        failure. If the file was not created to store raster data in a 
 *        packed format, this function returns error.
 * \warning If the NUI 2.2 file stores data in a planar fashion, the function
 *        returns -1 and sets error code. 
 *
 * \sa NUIF_LEVEL_get_by_id(), NUIF_LEVEL_get_first(), 
 *     NUIF_LEVEL_get_last(), NUIF_LEVEL_get_previous(), 
 *     NUIF_LEVEL_get_next(), NUIF_LEVEL_get_by_id(),
 *     NUI_SUPPORT_free(), NUI_SUPPORT_malloc(),
 *     NUIF_LEVEL_get_tile_data_size_in_bytes();
 *
 * \note  A NUIF tile has image data that may be stored in a packed or planar
 *        format. Packed format means if you have 4 bands (channels) in an
 *        image, the pixels for the 4 bands are stored as B0B1B2B3B0B1B2B3...
 *        Planar format means if you have 4 bands in an image, the pixels
 *        for the 4 bands are stored as B0B0B0B0... B1B1B1B1... B2B2B2B2... 
 *        B3B3B3B3...
 *
 * \note The function NUIF_free_memory_for_packed_tile() and 
 *       NUIF_allocate_memory_for_packed_tile() are now depricated. To 
 *       manage memory for \b return_buffer, use NUI_SUPPORT_malloc()
 *       and NUI_SUPPORT_free(). The key is that memory must be allocated
 *       at memory page boundaries by the system you are working on. These
 *       functions ensure that. To get number of bytes required to be
 *       allocated, use NUIF_LEVEL_get_tile_data_size_in_bytes().
 *
 * \note If the NUI 2.2 file is compressed, the data in memory is the
 *       compressed image. You have to use the NUI_CODEC interface to
 *       decode the image (Available upon request).
 */
NUI_SDK_API
NUI_INT32  NUIF_read_packed_tile_from_level(NUIF  *nuif,
                                NUI_UINT32  level_number,
                                NUI_UINT32  row, 
                                NUI_UINT32  column, 
                                NUI_UCHAR  *return_buffer);

/*
 * These functions, although get/set functions directly deal with
 * what happens inside NUIF_read_packed_tile_from_level(). Hence they
 * are kept here. The following functions deal with streaming data
 * within a tile read. Outside of that a tile is an atomic unit in the
 * streamed read of a NUI file. Now we have more read-control inside a 
 * tile.
 */
/**
 * \brief If the state is set to NUI_BOOLEAN_TRUE, streamed tile read is 
 *        activated. 
 *
 * There are times when due to the tile size being too large for the
 * read band-width to accomodate, the reading of a tile becomes a bottle-neck
 * to the calling application. This means a tile must not be read in a.
 * single gulp. Instead the tile read operation needs to be broken down
 * into several smaller read operations (each read operation being an
 * integer multiple of disk block size). This function enables/disables 
 * this operation in NUIF_read_packed_tile_from_level() function. If set to
 * NUI_BOOLEAN_TRUE and if a proper callback function is set using
 * NUIF_set_streamed_tile_read_callback(), a tile read operation is streamed.
 *
 * If the callback function returns a 0, all is well. If it returns -1,
 * the reading of a tile is interrupted and NUIF_read_packed_tile_from_level()
 * function returns a -1, while setting the error code to indicate that
 * the streamed tile read operation was interrupted by the API user.
 *       
 * \param nuif - (in) Pointer to non-NULL NUIF object
 * \param state - (in) Indicates whether streamed tile read is enabled 
 *        (NUI_BOOLEAN_TRUE) or disabled (NUI_BOOLEAN_FALSE).
 * \returns Previous value of \b state.
 * \note The default value of streamed tile reading is that it is disabled
 *       i.e. \b state is set to NUI_BOOLEAN_FALSE.
 */
NUI_SDK_API
NUI_BOOLEAN NUIF_set_streamed_tile_read_state(NUIF       *nuif,
                                              NUI_BOOLEAN state);

/**
 * \brief If the state is set to NUI_BOOLEAN_TRUE, streamed tile read is 
 *        activated. 
 *
 * \param nuif - (in) Pointer to non-NULL NUIF object
 * \returns Value of \b state from NUIF_set_streamed_tile_read_state() 
 * \note The default value of streamed tile reading is that it is disabled
 *       i.e. \b state is set to NUI_BOOLEAN_FALSE.
 */
NUI_SDK_API
NUI_BOOLEAN NUIF_get_streamed_tile_read_state(NUIF *nuif);

/**
 * \brief If streamed tile read is enabled and callback is set, the
 *        NUIF_read_packed_tile_from_level() function will read the tile in
 *        small chunks. The size of each chunk is determined by \b blocks
 *        as specified in this function.
 *
 * \param nuif - (in) Pointer to non-NULL NUIF object
 * \param blocks - (in) Specifies the number of blocks (1 or more) to read
 *        in the single read operation when reading the tile. Multiple such
 *        reads will read in the entire tile. At the end of each read, the
 *        callback function will get called.
 * \returns Previous value of \b blocks.
 * \note The default value of \b blocks is 1.
 */
NUI_SDK_API
NUI_UINT32 NUIF_set_streamed_tile_read_block_count(NUIF      *nuif,
                                                   NUI_UINT32 blocks);

/**
 * \brief Returns the value set by NUIF_set_streamed_tile_read_block_count().
 * \param nuif - (in) Pointer to non-NULL NUIF object
 * \returns the value set by NUIF_set_streamed_tile_read_block_count().
 */
NUI_SDK_API
NUI_UINT32 NUIF_get_streamed_tile_read_block_count(NUIF *nuif);

/**
 * \brief This structure is passed to the streamed tile read callback function
 *        every single time it is called
 */
typedef struct _nuif_streamed_tile_info_struct_ {
    NUIF       *nuif;
    /*!< Pointer to the NUIF object from which the tile was read */
    NUIF_LEVEL *level;
    /*!< Pointer to the NUIF_LEVEL object from which the tile was read */
    NUI_UCHAR  *buffer_that_contains_the_tile;
    /*!< Pointer to the memory buffer that was passed to the
         NUIF_read_packed_tile_from_level() function. */
    NUI_INT64   buffer_size;
    /*!< Size of the memory buffer that was passed to the
         NUIF_read_packed_tile_from_level() function (in bytes). */
    NUI_UINT32  buffer_start_offset_of_blocks_just_read;
    /*!< Offset from start of \b buffer_that_contains_the_tile
         from where the last read operation started placing data from
         the NUI tile. */
    NUI_UINT32  buffer_end_offset_of_blocks_just_read;
    /*!< Offset from start of \b buffer_that_contains_the_tile
         from where the last read operation ended placing data from
         the NUI tile. */
    NUI_UINT32  block_size_in_bytes;
    /*!< Size of one disk storage device block that contains the NUI file
         represented by NUIF */
    NUI_UINT32 bytes_just_read;
    /*!< Number of bytes just read in the last read operation */
    NUI_UINT32  blocks_just_read;
    /*!< Number of blocks just read in the last read operation */
    NUI_UINT32  total_bytes_read;
    /*!< Number of bytes read in all read operations for this tile so far */
    NUI_UINT32  total_blocks_read;
    /*!< Number of blocks read in all read operations for this tile so far */
    void       *user_data;
    /*!< This was passed by user in the parameter named \b callback_user_data
         using NUIF_set_streamed_tile_read_callback() */
} NUIF_STREAMED_TILE_INFO;

/**
 * \brief Sets a callback function to be called everytime a set of blocks
 *        is read from a tile. If NULL, streamed tile read is disabled.
 *
 * There are times when due to the tile size being too large for the
 * read band-width to accomodate, the reading of a tile becomes a bottle-neck
 * to the calling application. This means a tile must not be read in a.
 * single gulp. Instead the tile read operation needs to be broken down
 * into several smaller read operations (each read operation being an
 * integer multiple of disk block size). This function enables/disables 
 * this operation in the NUIF_read_packed_tile_from_level() function. If set to
 * NUI_BOOLEAN_TRUE and if a proper callback function is set using
 * NUIF_set_streamed_tile_read_callback(), a tile read operation is streamed.
 *
 * If the callback function is set to NULL, streamed tile read is considered
 * to be disabled.
 *
 * If the callback function returns a 0, all is well. If it returns -1,
 * the reading of a tile is interrupted and the NUIF_read_packed_tile_from_level()
 * function returns a -1, while setting the error code to indicate that
 * the streamed tile read operation was interrupted by the API user.
 *       
 * \param nuif - (in) Pointer to non-NULL NUIF object
 * \param callback - (in) Specifies the callback function to be called
 *        every time the streamed read operation reads in a chunk of
 *        data. The user can use NUIF_STREAMED_TILE_INFO to determine
 *        details on the read operation.
 * \param callback_user_data - (in) Specifies a pointer to a user defined
 *        data structure or data block that the user wants back when the
 *        callback is called. Default value is NULL.
 * \returns Previous value of \b state.
 * \note The default value of callback function is NULL.
 */
NUI_SDK_API
void        NUIF_set_streamed_tile_read_callback(NUIF *nuif,
                NUI_INT32 (*callback)(NUIF_STREAMED_TILE_INFO *info),
                void       *callback_user_data);


/* Coming soon: More read functions */
  
#ifdef __cplusplus
}
#endif

#endif /* _NUIF_READ_H_HAS_BEEN_INCLUDED_ */

/*
 * nuif_read.h ends
 */
