/*
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 */
/**
 * \file nuif_get.h
 * \section NUIF_GET
 * \brief API to get information from a nui file. These include functions to
 * manage properties, pyramid levels and bands.
 * 
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * 
 * \authors Rahul Thakkar, Saurabh Sood
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuif_get.h,v 2.3 2007/06/11 13:17:51 jensenk Exp $
 * $Log: nuif_get.h,v $
 * Revision 2.3  2007/06/11 13:17:51  jensenk
 * Fixed bug so we now return a properly formatted WKT string from the NUI
 * property using OGR instead of NUI_GEO. Added ability to get coordinate system
 * name and units string from WKT (for the ESRI plugin).
 *
 * Revision 2.2  2007/06/08 11:03:19  thakkarr
 * - Added bits/band to nui data type function
 * - Added ability to get name of nui data type
 *
 * Revision 2.1  2006/12/04 22:46:40  thakkarr
 *
 * - nge.cpp nex_create.c   Added code to read and write unsigned and signed data types
 * - nuif_get.c nuif_get.h  Can read data types
 * - nuif_type.h            Added signed and unsigned band data types
 * - nui_black.c            Can create signed and unsigned nui files
 * - nui_black_main.c       Can create signed and unsigned nui files
 * - mafc_ files            Can handle signed and unsigned nui files.
 *                          Added function to check if data type is signed or not
 * - nuix_output files      Can read signed or unsigned. Explicit s or u with bits/band
 * - nuix_source files      Can read signed or unsigned. Explicit s or u with bits/band
 *
 * Revision 2.0  2006/11/16 21:13:04  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:56:17  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.9  2006/05/22 15:05:07  soods
 * Added three functions for pixel, lat-long and projected space conversions
 *
 * Revision 1.8  2005/06/13 23:02:32  thakkarr
 * Added functions and support constants for getting geo-spatial information
 * like coordinate system WKT, name, units name, and bounding box.
 *
 * Revision 1.7  2005/03/23 15:55:42  thakkarr
 * NUIF_LEVEL_convert_to_image_coordinates() and
 * NUIF_LEVEL_convert_from_image_coordinates() now use NUI_DOUBLE instead
 * of NUI_UINT32
 *
 * Revision 1.6  2005/03/23 00:48:23  thakkarr
 * Added new functions for get_i
 *
 * Revision 1.5  2005/01/31 19:31:47  thakkarr
 * Added ability to get property information and fixed header
 * formatted information. Finished nuiSDK_print
 *
 * Revision 1.4  2005/01/28 17:16:45  thakkarr
 * Finished SDK
 * Testing and cleanup begin
 *
 * Revision 1.3  2005/01/27 19:38:47  thakkarr
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

#ifndef _NUIF_GET_H_HAS_BEEN_INCLUDED_
#define _NUIF_GET_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h>  /*!< Pixia Corp global constants */
#include "nuif_constants.h" /*!< Constants for NUIF */
#include "nuif_types.h"     /*!< Data types for NUIF */

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************
 * The first set of functions deal with getting information from the 
 * NUIF object. The functions have a NUIF prefix.
 **********************************************************************/

/**
 * \brief Returns name of the NUI file assigned to it.
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns Returns name of the NUI file assigned to it or NULL if not
 *          setting NUIF error codes.
 */
NUI_SDK_API
NUI_CHAR *NUIF_get_nui_filename(NUIF *nuif);

/**
 * \brief Get basic image information.
 *
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \param image_width (out) Address to place width of image
 * \param image_height (out) Address to place height of image
 * \param channels (out) Address to place bands per pixel
 * \param bits (out) Address to place bits/band (NUIF_BITS_PER_CHANNEL)
 * \param type (out) Address to place pixel data type (NUIF_DATA_TYPE)
 * \param layout (out) Address to place pixel layout (NUIF_DATA_LAYOUT)
 * \param codec (out) Address to place compression info (NUIF_CODEC)
 * \param compression_ratio (out) If the image is compressed (as stated in
 *        \b codec), \b compression_ratio represents a ratio by which image
 *        is to be compressed. Value N represents N:1 compression of a
 *        tile in the NUI 2.2 file. Parameter specifies address to place
 *        the compression ratio.
 *
 * Any one of the (out) parameters can be NULL, and if NULL that parameter
 * is ignored. If the NUIF object is not initialized, default values are
 * returned.
 */
NUI_SDK_API
void  NUIF_get_image_parameters(NUIF                  *nuif, 
                                NUI_UINT32            *image_width, 
                                NUI_UINT32            *image_height,
                                NUI_UINT32            *channels,
                                NUIF_BITS_PER_CHANNEL *bits,
                                NUIF_DATA_TYPE        *type,
                                NUIF_DATA_LAYOUT      *layout,
                                NUIF_CODEC            *codec,
                                NUI_DOUBLE            *compression_ratio);

/**
 * \brief NUIF_BITS_PER_CHANNEL not only tells you the number of bits in
 *        the band, but also its data type - i.e. if it is 8-bit signed/
 *        unsigned, 16-bits signed/unsigned, 32 or 64-bit float, etc.
 *        This function returns what that data type is.
 * \param bits (in) One of NUIF_BITS_PER_CHANNEL or value as returned by 
 *        NUIF_get_image_parameters().
 * \returns One of NUI_DATA_TYPE as defined in nui_constants.h
 */
NUI_SDK_API
NUI_DATA_TYPE NUIF_get_bits_per_channel_data_type(NUIF_BITS_PER_CHANNEL bits);

/**
 * \brief Returns the name of NUI_DATA_TYPE. Name is in a long format
 *        such as "8-bit unsigned" or "32-bit floating point".
 * \param type (in) One of NUI_DATA_TYPE
 * \returns The name of NUI_DATA_TYPE. Name is in a long format
 *        such as "8-bit unsigned" or "32-bit floating point".
 * \warning This is different from NUIF_DATA_TYPE and same as 
 *        NUIF_BITS_PER_CHANNEL by way of interpretation.
 */
NUI_SDK_API
const NUI_CHAR *NUIF_get_nui_data_type_long_name(NUI_DATA_TYPE type);


/**
 * \brief Returns the name of NUI_DATA_TYPE. Name is in a short format
 *        such as "u8", "u32", "f32" or "f64".
 * \param type (in) One of NUI_DATA_TYPE
 * \returns The name of NUI_DATA_TYPE. Name is in a short format
 *        such as "u8", "u32", "f32" or "f64".
 * \warning This is different from NUIF_DATA_TYPE and same as 
 *        NUIF_BITS_PER_CHANNEL by way of interpretation.
 */
NUI_SDK_API
const NUI_CHAR *NUIF_get_nui_data_type_short_name(NUI_DATA_TYPE type);

/**
 * \brief Returns the texture type of the NUI 2.2 image file
 *
 * If this function returns anything other than NUIF_TEXTURE_TYPE_NONE, it
 * means that the image is expected to be used as an OpenGL texture map. 
 *
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \note Recommended tile size for such files is 512x512 or 256x256.
 * \returns The texture type of the NUI 2.2 image file
 */
NUI_SDK_API
NUIF_TEXTURE_TYPE NUIF_get_texture_type(NUIF *nuif);

/**
 * \brief User can query if the NUI 2.2 file contains or does not contain
 *        image data using this function.
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns NUI_TRUE if image file has image data or plans to have image data
 *          (if a newly created instance of the file), else returns NUI_FALSE
 *          if the file does not contain or plan to contain image data and is
 *          only header information.
 * \note  Any changes to functions affecting image data placement in the NUI
 *        2.2 file will force a recomputation of level offsets.
 */
NUI_SDK_API
NUI_INT16  NUIF_get_image_data_present_in_file_state(NUIF *nuif);

/**
 * \brief Returns true if the bits per channel hold signed information.
 *        Returns false if the bits per channel hold unsigned information.
 *
 * NUIF_BITS_PER_CHANNEL hold the bits per band as specified in the NUI
 * file and whether this data is signed or unsigned.
 * 
 * You can use NUIF_get_total_bits_per_channel() with
 * NUIF_do_bits_per_channel_hold_signed_data() to determine band data type.
 * To find out if bits per channel hold single or double precision
 * floating point data, use NUIF_BITS_PER_CHANNEL from the function
 * NUIF_get_image_parameters().
 * 
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns NUI_BOOLEAN_TRUE if bits per channel hold signed information
 *        or NUI_BOOLEAN_TRUEif bits per channel hold unsigned information.
 * \sa NUIF_get_total_bits_per_channel(), NUIF_get_image_parameters(),
 *     NUIF_BITS_PER_CHANNEL
 */
NUI_BOOLEAN NUIF_do_bits_per_channel_hold_signed_data(NUIF *nuif);

/**
 * \brief Returns actual bits per channel
 *
 * Actual bits/channel may be >= NUIF_BITS_PER_CHANNEL in the image. 
 * Example an 11-bits/channel image utilizes the lower 11-bits of a 
 * 16-bit pixel channel value. So actual bits/channel is 16 with maximum
 * pixel value - an 11-bit number.
 * 
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns Actual bits per channel.
 * \note All channels must have the same number of bits/channel.
 */
NUI_SDK_API
NUI_UINT32 NUIF_get_total_bits_per_channel(NUIF *nuif);

/**
 * \brief Returns actual bytes per channel
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns Actual bytes/channel (actual bits/channel divided by 8)
 */
NUI_SDK_API
NUI_UINT32 NUIF_get_total_bytes_per_channel(NUIF *nuif);

/**
 * \brief Returns actual bytes per channel
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns Actual bytes/pixel (actual bytes/channel times number of channels)
 */
NUI_SDK_API
NUI_UINT32 NUIF_get_total_bytes_per_pixel(NUIF *nuif);

/**
 * \brief Provides information on the size of a tile in the NUI file
 *
 * The size of a tile in a NUI file is twice the maximum estimated
 * display size for this NUI file. This means, if the NUI file is to
 * be displayed on a device whose pixel resolution is 1280x1024, then
 * the tile size is 2560x2048.
 *
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \param tile_width (out) Address to place width of a tile.
 * \param tile_height (out) Address to place height of a tile.
 * \note Any of the (out) parameters, if NULL, are ignored.
 */
NUI_SDK_API
void       NUIF_get_tile_size(NUIF       *nuif, 
                              NUI_UINT32 *tile_width,
                              NUI_UINT32 *tile_height);

/**
 * \brief Gets total pyramid levels stored in the NUI file
 *
 * A NUI file may have 1 or more pyramid levels. A pyramid level is a reduced
 * resolution dataset (RRD) of the original image. Simply put, it is a scaled-
 * down version of the same image.
 *
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns Total pyramid levels stored in the NUI file.
 */
NUI_SDK_API
NUI_UINT32 NUIF_get_total_pyramid_levels(NUIF *nuif);

/** 
 * \brief Returns total number of properties in a NUI file.
 *
 * A property holds a single unit of metadata in a NUI 2.2 file.
 *
 * \param nuif (in) A valid non-NULL NUIF object pointer.
 * \returns Total number of properties in a NUI file.
 * \sa nuif_property.h
 */
NUI_SDK_API
NUI_UINT32 NUIF_get_total_properties(NUIF *nuif); 

/**
 * \brief Returns information about the fixed portion of the NUI 2.2 
 *        file header.
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \param as_html (in) If NUI_TRUE, information is formatted in HTML, if
 *        NUI_FALSE it is formatted for printing on a console.
 * \returns A character string with formatted fixed NUI file header 
 *        information.
 * \warning Do not free the returned value. Immediately copy it to another
 *        character string.
 */
NUI_SDK_API
NUI_CHAR   *NUIF_get_formatted_fixed_information(NUIF       *nuif,
                                                 NUI_INT32   as_html);

/**********************************************************************
 * NUIF_CHANNEL interface gets information about individual
 * bands. Since NUIF_CHANNEL list is a linked list, there has to be
 * a way to traverse the list of NUIF_CHANNEL objects in a NUIF object.
 * Once you have a NUIF_CHANNEL object, there have to be functions to
 * get information about the NUIF_CHANNEL object.
 **********************************************************************/

/**
 * \brief Returns the first channel
 * \param nuif (in) non-NULL, properly initialized NUIF object
 * \returns Returns a pointer to first channel or NULL upon error.
 */
NUI_SDK_API
NUIF_CHANNEL *NUIF_CHANNEL_get_first(NUIF *nuif);

/**
 * \brief Returns the next channel (after specified one)
 * \param channel (in) non-NULL, properly initialized NUIF_CHANNEL object
 * \returns Returns a pointer to next channel object after \b channel
 *          or NULL upon reaching end of list (NULL is not an error).
 */
NUI_SDK_API
NUIF_CHANNEL *NUIF_CHANNEL_get_next(NUIF_CHANNEL *channel);

/**
 * \brief Returns the last channel
 * \param nuif (in) non-NULL, properly initialized NUIF object
 * \returns Returns a pointer to last channel or NULL upon error.
 */
NUI_SDK_API
NUIF_CHANNEL *NUIF_CHANNEL_get_last(NUIF *nuif);

/**
 * \brief Returns the previous channel (before specified one)
 * \param channel (in) non-NULL, properly initialized NUIF_CHANNEL object
 * \returns Returns a pointer to previous channel object before \b channel
 *          or NULL upon reaching end of list (NULL is not an error).
 */
NUI_SDK_API
NUIF_CHANNEL *NUIF_CHANNEL_get_previous(NUIF_CHANNEL *channel);

/**
 * \brief Returns channel object by its identifier.
 * \param channel (in) non-NULL, properly initialized NUIF_CHANNEL object
 * \param channel_id (in) ID of the channel to find
 * \returns Returns whose ID is \b channel_id or NULL if not found
 */
NUI_SDK_API
NUIF_CHANNEL *NUIF_CHANNEL_get_by_id(NUIF        *nuif, 
                                     NUI_UINT32   channel_id);

/**
 * \brief Returns unique channel id assigned to this channel
 * \param channel (in) Pointer to non-NULL channel property
 * \returns Unique channel id assigned to this channel
 */
NUI_SDK_API
NUI_UINT32 NUIF_CHANNEL_get_id(NUIF_CHANNEL *channel);

/**
 * \brief Returns name assigned to a channel
 * \param channel (in) Pointer to non-NULL channel property
 * \returns Name assigned to a channel
 */
NUI_SDK_API
NUI_CHAR  *NUIF_CHANNEL_get_name(NUIF_CHANNEL *channel);

/**
 * \brief Provides minimum and maximum possible color value a pixel
 *        can have.
 * \param channel (in) Pointer to non-NULL channel property
 * \param minimum_value (out) Address to place minimum possible pixel value
 * \param maximum_value (out) Address to place maximum possible pixel value
 */
NUI_SDK_API
void       NUIF_CHANNEL_get_range(NUIF_CHANNEL *channel,
                                  NUI_DOUBLE  *minimum_value,
                                  NUI_DOUBLE  *maximum_value);

/**
 * \brief Provides statistics about a channel for an image.
 * \param channel (in) Pointer to non-NULL channel property
 * \param minimum (out) Address for holding minimum pixel value in image
 * \param maximum (out) Address for holding maximum pixel value in image
 * \param standard_deviation (out) Address for holding standard deviation
 * \param mean (out) Address for holding mean of pixel distribution
 * \param median (out) Address for holding median of pixel distribution
 * \param mode (out) Address for holding mode of pixel distribution
 */
NUI_SDK_API
void       NUIF_CHANNEL_get_statistics(NUIF_CHANNEL *channel,
                                       NUI_DOUBLE  *minimum,
                                       NUI_DOUBLE  *maximum,
                                       NUI_DOUBLE  *standard_deviation,
                                       NUI_DOUBLE  *mean,
                                       NUI_DOUBLE  *median,
                                       NUI_DOUBLE  *mode);

/**
 * \brief Provides statistics about a channel
 * \param channel (in) Pointer to non-NULL channel property
 * \param histogram_elements (out) Number of elements in \b histogram array
 * \returns An array of 64-bit integers that holds the histogram
 */
NUI_SDK_API
NUI_INT64  *NUIF_CHANNEL_get_histogram(NUIF_CHANNEL *channel,
                                       NUI_UINT32  *histogram_elements);

/**
 * \brief Prints information about a channel to a file
 * \param filename (in) Name of file to write to. If NULL, output goes to 
 *        standard output
 * \param channel (in) non-NULL, properly initialized NUIF_CHANNEL object.
 */
NUI_SDK_API
void        NUIF_CHANNEL_print(NUI_CHAR     *filename, 
                               NUIF_CHANNEL *channel);

/**
 * \brief Returns formatted channel information
 * \param channel (in) non-NULL, properly initialized NUIF_CHANNEL object.
 * \param as_html (in) If NUI_TRUE, information is formatted in HTML, if
 *        NUI_FALSE it is formatted for printing on a console.
 * \returns A character string with formatted channel information
 * \warning Do not free the returned value. Immediately copy it to another
 *          character string.
 */
NUI_SDK_API
NUI_CHAR   *NUIF_CHANNEL_get_formatted_information(NUIF_CHANNEL *channel,
                                                   NUI_INT32           as_html);


/*****************************************************************
 * This group of functions deal with getting information on each
 * pyramid level and traversing the list of pyramid levels, thus
 * getting a pointer to each pyramid level object. The functions
 * have a NUIF_LEVEL prefix.
 *****************************************************************/

/**
 * \brief Returns the first pyramid level (original image)
 * \param nuif (in) non-NULL, properly initialized NUIF object
 * \returns Returns a pointer to first pyramid level (original image)
 *          or NULL upon error.
 * \sa NUIF_LEVEL_get_first(), NUIF_LEVEL_get_last(),
 *     NUIF_LEVEL_get_previous(), NUIF_LEVEL_get_next(), 
 *     NUIF_LEVEL_get_by_id()
 */
NUI_SDK_API
NUIF_LEVEL *NUIF_LEVEL_get_first(NUIF *nuif);

/**
 * \brief Returns the last pyramid level (smallest image)
 * \param nuif (in) non-NULL, properly initialized NUIF object
 * \returns Returns a pointer to last pyramid level (smallest image)
 *          or NULL upon error.
 * \sa NUIF_LEVEL_get_first(), NUIF_LEVEL_get_last(),
 *     NUIF_LEVEL_get_previous(), NUIF_LEVEL_get_next(), 
 *     NUIF_LEVEL_get_by_id()
 */
NUI_SDK_API
NUIF_LEVEL *NUIF_LEVEL_get_last(NUIF *nuif);

/**
 * \brief Returns the next pyramid level (after specified one)
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \returns Returns a pointer to next pyramid level after \b level
 *          or NULL upon reaching end of list (NULL is not an error).
 * \sa NUIF_LEVEL_get_first(), NUIF_LEVEL_get_last(),
 *     NUIF_LEVEL_get_previous(), NUIF_LEVEL_get_next(), 
 *     NUIF_LEVEL_get_by_id()
 */
NUI_SDK_API
NUIF_LEVEL *NUIF_LEVEL_get_next(NUIF_LEVEL *level);

/**
 * \brief Returns the previous pyramid level (before specified one)
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \returns Returns a pointer to previous pyramid level before \b level
 *          or NULL upon reaching end of list (NULL is not an error).
 * \sa NUIF_LEVEL_get_first(), NUIF_LEVEL_get_last(),
 *     NUIF_LEVEL_get_previous(), NUIF_LEVEL_get_next(), 
 *     NUIF_LEVEL_get_by_id()
 */
NUI_SDK_API
NUIF_LEVEL *NUIF_LEVEL_get_previous(NUIF_LEVEL *level);

/**
 * \brief Returns pyramid level by its identifier.
 * \param nuif (in) non-NULL, properly initialized NUIF object
 * \param level_id (in) ID of the level to find
 * \returns Returns whose ID is \b level_id or NULL if not found
 * \sa NUIF_LEVEL_get_first(), NUIF_LEVEL_get_last(),
 *     NUIF_LEVEL_get_previous(), NUIF_LEVEL_get_next(), 
 *     NUIF_LEVEL_get_by_id()
 */
NUI_SDK_API
NUIF_LEVEL *NUIF_LEVEL_get_by_id(NUIF       *nuif, 
                                 NUI_UINT32 level_id);

/**
 * \brief Gives information about the size of the pyramid level.
 * \param level (in) Pointer to non-NULL NUIF_LEVEL object.
 * \param image_width (out) Address for width of image within the level
 * \param image_height (out) Address for height of image within the level
 * \param aligned_width (out) Address for width of level aligned to a tile
 * \param aligned_height (out) Address for height of image aligned to a tile
 * \param total_rows (out) Total rows of tiles in the level
 * \param total_columns (out) Total columns of tiles in the level
 * \note Image width and height are in pixels and is the actual width and
 *       height of the pyramid level. Aligned width and height are always
 *       larger than image width and height and specify the width and height
 *       of a level as it is stored on disk (which is aligned to tile width
 *       and height). In a level, the are \b R rows and \b C columns, each
 *       cell (R<sub>i</sub>, C<sub>j</sub>) being 1 tile. Tile width * C =
 *       Aligned level width (and same for height).
 * \note Any of the (out) parameters, if NULL, are ignored.
 */
NUI_SDK_API
void       NUIF_LEVEL_get_size(NUIF_LEVEL *level,
                               NUI_UINT32 *image_width,
                               NUI_UINT32 *image_height,
                               NUI_UINT32 *aligned_width,
                               NUI_UINT32 *aligned_height,
                               NUI_UINT32 *total_rows,
                               NUI_UINT32 *total_columns);

/**
 * \brief Returns unique pyramid level identifier (not stored in file)
 * \param level (in) Pointer to non-NULL NUIF_LEVEL object.
 * \returns Unique pyramid level identifier (not stored in file)
 * \note Level identifier is set by the API automatically when it
 *       is appended to NUIF object.
 */
NUI_SDK_API
NUI_UINT32 NUIF_LEVEL_get_id(NUIF_LEVEL *level);

/**
 * \brief Provides pyramid level scale factors for a level
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \param x_scale (out) Ratio of level image width to original image width
 * \param y_scale (out) Ratio of level image height to original image height
 * \note NUIF_compute_level_scale_factors() must be called prior to 
 *       calling this function.
 * \note Any of the (out) parameters, if NULL, are ignored.
 */
NUI_SDK_API
void       NUIF_LEVEL_get_scale_factors(NUIF_LEVEL *level,
                                        NUI_DOUBLE *x_scale,
                                        NUI_DOUBLE *y_scale);

/**
 * \brief Returns starting point of image data for a level in file.
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \returns Returns starting point of image data for a level in file 
 *          relative to start of file.
 */
NUI_SDK_API
NUI_INT64  NUIF_LEVEL_get_image_data_start_offset(NUIF_LEVEL *level);

/**
 * \brief Returns size of image data for the level, in bytes
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \returns Size of image data for the level, in bytes
 */
NUI_SDK_API
NUI_INT64  NUIF_LEVEL_get_image_data_size_in_bytes(NUIF_LEVEL *level);

/**
 * \brief Returns size (in bytes) of a tile in the level
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \returns Size (in bytes) of a tile in the level. If the tile is compressed
 *          the function effectively returns the size of the compressed
 *          tile.
 */
NUI_SDK_API
NUI_INT64  NUIF_LEVEL_get_tile_data_size_in_bytes(NUIF_LEVEL *level);


/**
 * \brief Converts from X,Y position relative to specified pyramid
 *        level to X,Y position relative to original image. Input values
 *        can be outside the image. Correct output values are interpolated.
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \param level_x (in) X coordinate of a point in the image for the level
 * \param level_y (in) Y coordinate of a point in the image for the level
 * \param image_x (out) Receives corresponding X coordinate in original image
 * \param image_y (out) Receives corresponding Y coordinate in original image
 * \returns 0 on success, -1 if level was not properly initialized
 * \note Any of the (out) parameters, if NULL, are ignored.
 */
NUI_SDK_API
NUI_INT32 NUIF_LEVEL_convert_to_image_coordinates(NUIF_LEVEL *level,
                                                  NUI_DOUBLE  level_x,
                                                  NUI_DOUBLE  level_y,
                                                  NUI_DOUBLE *image_x,
                                                  NUI_DOUBLE *image_y);

/**
 * \brief Converts from X,Y position relative to original image
 *        to X,Y position relative to specified pyramid level. Input values
 *        can be outside the image. Correct output values are interpolated.
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \param image_x (in) X coordinate in original image
 * \param image_y (in) Y coordinate in original image
 * \param level_x (out) Receives corresponding X coordinate for the level
 * \param level_y (out) Receives corresponding Y coordinate for the level
 * \returns 0 on success, -1 if level was not properly initialized
 * \note Any of the (out) parameters, if NULL, are ignored.
 */
NUI_SDK_API
NUI_INT32 NUIF_LEVEL_convert_from_image_coordinates(NUIF_LEVEL *level,
                                                    NUI_DOUBLE  image_x,
                                                    NUI_DOUBLE  image_y,
                                                    NUI_DOUBLE *level_x,
                                                    NUI_DOUBLE *level_y);

/**
 * \brief Given X,Y position relative to the level, this function
 *        computes the row and column of the tile this point is in.
 *
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \param level_x (in) X coordinate of a point in the image wrt the level
 * \param level_y (in) Y coordinate of a point in the image wrt the level
 * \param row (out) Receives row number of the tile containing the point
 * \param column (out) Receives column number of the tile containing the point
 *
 * \returns 0 on success or -1 if level was not properly initialized.
 *
 * \note Any of the (out) parameters, if NULL, are ignored.
 */
NUI_SDK_API
NUI_INT32 NUIF_LEVEL_compute_row_and_column_for_level(NUIF_LEVEL *level,
                                                      NUI_UINT32  level_x,
                                                      NUI_UINT32  level_y,
                                                      NUI_UINT32 *row,
                                                      NUI_UINT32 *column);

/**
 * \brief Given a row and column, function computes top-left and bottom-right
 *        pixel coordinates of the tile represented by (column, row).
 *
 * Based on whether the user has chosen to include the image in the file
 * and whether compression is enabled or disabled, the viewport that the
 * tile contains is computed.
 * 
 * The specified output viewport is inclusive for all four coordinates.
 *
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object
 * \param row (in) Row number of the tile
 * \param column (in) Column number of the tile
 * \param level_top_left_x (out) Top left X corner of the tile in pixels 
 *        in pyramid level coordinates.
 * \param level_top_left_y (out) Top left Y corner of the tile in pixels 
 *        in pyramid level coordinates.
 * \param level_bottom_right_x (out) Bottom right X corner of the tile in
 *        pixels in pyramid level coordinates.
 * \param level_bottom_right_y (out) Bottom right Y corner of the tile in
 *        pixels in pyramid level coordinates.
 */
NUI_SDK_API
NUI_INT32 NUIF_LEVEL_compute_row_and_column_viewport(
                                     NUIF_LEVEL *level,
                                     NUI_UINT32  row,
                                     NUI_UINT32  column,
                                     NUI_UINT32 *level_top_left_x,
                                     NUI_UINT32 *level_top_left_y,
                                     NUI_UINT32 *level_bottom_right_x,
                                     NUI_UINT32 *level_bottom_right_y);

/**
 * \brief Prints information about a tile to a file
 * \param filename (in) Name of file to write to. If NULL output goes to 
 *        standard output
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object.
 * \returns 0 on success, -1 on error
 */
NUI_SDK_API
void       NUIF_LEVEL_print(NUI_CHAR  *filename, 
                            NUIF_LEVEL *level);

/**
 * \brief Returns formatted level information
 * \param level (in) non-NULL, properly initialized NUIF_LEVEL object.
 * \param as_html (in) If NUI_TRUE, information is formatted in HTML, if
 *        NUI_FALSE it is formatted for printing on a console.
 * \returns A character string with formatted level information
 * \warning Do not free the returned value. Immediately copy it to another
 *          character string.
 */
NUI_SDK_API
NUI_CHAR   *NUIF_LEVEL_get_formatted_information(NUIF_LEVEL *level, 
                                                 NUI_INT32 as_html);

/*****************************************************************
 * The NUIF_GEO interface retrieves geo-spatial information about
 * a NUI 2.2 file. This interface has been developed using GDAL
 * (Geo-spatial Data Abstraction Layer) developed by Frank
 * Warmerdam. See http://www.remotesensing.org/gdal/
 *****************************************************************/
/**
 * \brief Returns geo-spatial information in a NUI 2.2 file.
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \returns NUIF_GEO object that holds all geo-spatial information 
 *          in a valid NUIF object.
 */
NUI_SDK_API
NUIF_GEO *NUIF_GEO_get_object(NUIF *nuif);

/**
 * \brief Provides basic information about the geo-spatial data in file.
 * \param geo (in) A valid non-NULL NUIF_GEO object
 * \param width (out) Address to place width of the image in pixels.
 * \param height (out) Address to place height of the image in pixels.
 * \param x_pixel_size (out) Address to place size of pixel in X.
 * \param y_pixel_size (out) Address to place size of pixel in Y.
 * \returns 0 on success, -1 on failure
 */
NUI_SDK_API
NUI_INT32  NUIF_GEO_get_basic_parameters(NUIF_GEO    *geo,
                                         NUI_UINT32 *width,
                                         NUI_UINT32 *height,
                                         NUI_DOUBLE *x_pixel_size,
                                         NUI_DOUBLE *y_pixel_size);


/**
 * \brief Returns total number geo-spatial keys in NUIF's NUIF_GEO object.
 * \param geo (in) A valid non-NULL NUIF_GEO object
 * \returns Total number of geo-spatial keys in NUIF's NUIF_GEO object. If
 *          there are no keys or no NUIF_GEO object, the function returns
 *          0 and sets error code.
 */
NUI_SDK_API
NUI_UINT32  NUIF_GEO_get_total_geo_keys(NUIF_GEO    *geo);

/**
 * \brief Returns the first valid geo-spatial key in NUIF's NUIF_GEO object.
 * \param geo (in) A valid non-NULL NUIF_GEO object
 * \returns Pointer to first NUIF_GEO_KEY object in the list of geo-spatial
 *          keys or NULL if there is no NUIF_GEO object in the NUIF object or
 *          there are no geo-keys in the object (setting error code).
 */
NUI_SDK_API
NUIF_GEO_KEY *NUIF_GEO_get_first_geo_key(NUIF_GEO    *geo);

/**
 * \brief Returns the last valid geo-spatial key in NUIF's NUIF_GEO object.
 * \param geo (in) A valid non-NULL NUIF_GEO object
 * \returns Pointer to last NUIF_GEO_KEY object in the list of geo-spatial
 *          keys or NULL if there is no NUIF_GEO object in the NUIF object or
 *          there are no geo-keys in the object (setting error code).
 */
NUI_SDK_API
NUIF_GEO_KEY *NUIF_GEO_get_last_geo_key(NUIF_GEO    *geo);

/**
 * \brief Returns the next geo-spatial key after specified key
 * \param key (in) A non-NULL NUIF_GEO_KEY object that is a part of a
 *        valid NUIF_GEO object that is a part of a valid NUIF object.
 * \returns Pointer to next geo-spatial key after specified key or NULL
 *          if no keys after this one (NULL is not an error).
 */
NUI_SDK_API
NUIF_GEO_KEY *NUIF_GEO_get_next_geo_key(NUIF_GEO_KEY *key);

/**
 * \brief Returns the previous geo-spatial key before specified key
 * \param key (in) A non-NULL NUIF_GEO_KEY object that is a part of a
 *        valid NUIF_GEO object that is a part of a valid NUIF object.
 * \returns Pointer to previous geo-spatial key before specified key or
 *          NULL if no keys after this one (NULL is not an error).
 */
NUI_SDK_API
NUIF_GEO_KEY *NUIF_GEO_get_previous_geo_key(NUIF_GEO_KEY *key);

/**
 * \brief Provides information about a geo-spatial key
 * \note A geo-spatial key in a NUI 2.2 file mimics a GeoTIFF geo-key.
 * \param key (in) A non-NULL NUIF_GEO_KEY object that is a part of a
 *        valid NUIF_GEO object that is a part of a valid NUIF object.
 * \param type (out) Address for one of NUIF_GEO_KEY_TYPE specifying 
 *        whether type of data values in the key are doubles, short
 *        integers or ASCII characters.
 * \param id (out) Address for a GeoTIFF identifier for the key.
 * \param total_values (out) Address for total number of keys in NUIF_GEO
 *        object.
 * \returns A list of values totalling \b total_values of specified 
 *          data \b type or NULL if no values and sets error code
 * \warning Do not free returned pointer.
 *
 */
NUI_SDK_API
void *NUIF_GEO_get_geo_key_parameters(NUIF_GEO_KEY      *key,
                                      NUIF_GEO_KEY_TYPE *type,
                                      NUI_UINT32        *id,
                                      NUI_UINT32        *total_values);

/**
 * \brief Prints information about a Geo-header from a NUI 2.2 file
 * \param filename (in) output text file name to append to. If NULL,
 *        output is sent to \b stdout.
 * \param geo (in) non-NULL geo-spatial header from NUI 2.2 file
 */
NUI_SDK_API
void NUIF_GEO_print(NUI_CHAR *filename, NUIF_GEO *geo);

/**
 * \brief Returns information about a Geo-header from a NUI 2.2 file
 * \param geo (in) non-NULL geo-spatial header from NUI 2.2 file
 * \param as_html (in) If NUI_TRUE, string is formatted as HTML, else not.
 * \returns Formatted information about a Geo-header from a NUI 2.2 file.
 */
NUI_SDK_API
NUI_CHAR *NUIF_GEO_get_formatted_information(NUIF_GEO *geo, 
                                             NUI_INT32 as_html);

/****************************************************************
 * The following NUIF_GEO functions are temporary and may 
 * evolve over time. These functions are not as fast as we would
 * like them to be. However, over time, we indend to optimize them
 * further. Their names and purpose will not change, their internal
 * run-time will be optimized.
 * Start of functions.
 ****************************************************************/
/**
 * \brief Returns geo-spatial coordinate system information in standard
 *        OpenGIS well known text format.
 *
 * OpenGIS Well Known Text is a standard character string in BNR form.
 * This string is understood by most GIS applications including ESRI
 * tools. For interoperability between NUI file format and 3rd party
 * software. It is essential to have the ability in the nuiSDK to 
 * query geo-spatial coordinate system information in this format. 
 * Several string forms have been provided to work with various 
 * 3rd party systems.
 *
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \param format - (in) One of NUIF_OPENGIS_WKT_FORMAT. Specifies the
 *        layout of the character string that is returned. For instance
 *        NUI_OPENGIS_WKT_FOR_ESRI returns character string which can
 *        be passed directly to ESRI ArcGIS and ERDAS Imagine tools.
 * \param wkt - (out) Character string to hold the OpenGIS well known text.
 *        Keep it atleast 2048 bytes in size.
 * \param wkt_length - (in) Length of character string \b wkt in bytes.
 * \returns 0 on success -1 on failure and sets error code.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_get_coordinate_system_as_OpenGIS_WKT(NUIF     *nuif,
                                          NUIF_OPENGIS_WKT_FORMAT format,
                                          NUI_CHAR               *wkt,
                                          NUI_INT32               wkt_length);

/**
 * \brief Returns the name of the coordinate system as specified in the
 *        NUI 2.2 file.
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \param cs_name - (out) location of name of coordinate system.
 *        Ignored if NULL.
 * \param cs_name_length - (in) Non-zero length of \b cs_name string.
 * \param units_name - (out) location of name of units in which the
 *         pixel-size information is provided. Ignored if NULL.
 * \param units_name_length - (in) Non-zero length of \b units_name string.
 *
 * \returns 0 on success or -1 on failure and sets error code.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_get_coordinate_system_name_and_units(NUIF *nuif,
                                            NUI_CHAR  *cs_name,
                                            NUI_UINT32 cs_name_length,
                                            NUI_CHAR  *units_name,
                                            NUI_UINT32 units_name_length);

/**
 * \brief Returns the name of the coordinate system as specified in the
 *        OpenGIS well-known text.
 * \param wkt (in) A non-NULL OpenGIS well-known text string.
 * \param cs_name - (out) location of name of coordinate system.
 *        Ignored if NULL.
 * \param cs_name_length - (in) Non-zero length of \b cs_name string.
 * \param units_name - (out) location of name of units in which the
 *         pixel-size information is provided. Ignored if NULL.
 * \param units_name_length - (in) Non-zero length of \b units_name string.
 *
 * \returns 0 on success or -1 on failure and sets error code.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_get_coordinate_system_name_and_units_from_wkt(
                                            const NUI_CHAR *wkt,
                                            NUI_CHAR       *cs_name,
                                            NUI_UINT32      cs_name_length,
                                            NUI_CHAR       *units_name,
                                            NUI_UINT32      units_name_length);

/**
 * \brief This function returns the projected coordinates of the bounding box
 *        of the NUI 2.2 file.
 * 
 * This function projects the top left and bottom right corners of
 * the image into the coordinate system that is in the NUI 2.2 file
 * and returns the bounding box. The bounding box of the image is
 * from 0,0 to w-1, h-1. Where w is the width of the image in pixels
 * and h is the height of the image in pixels. 
 *
 * \note Using this information, and assuming the pixels are all of the
 *       same size and rectangular, you can computer the projected
 *       coordinates of any pixel in the raster dataset.
 *
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \param top_left_x - (out) Projected location of X-coordinate of the 
 *        top left corner of the image (for pixel 0, 0). Ignored of NULL.
 * \param top_left_y - (out) Projected location of Y-coordinate of the 
 *        top left corner of the image (for pixel 0, 0). Ignored of NULL.
 * \param bottom_right_x - (out) Projected location of X-coordinate of the 
 *        bottom right corner of the image (for pixel w-1, h-1). 
 *        Ignored of NULL.
 * \param bottom_right_y - (out) Projected location of Y-coordinate of the 
 *        bottom right corner of the image (for pixel w-1, h-1). 
 *        Ignored of NULL.
 * \returns 0 on success or -1 on failure and sets error code.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_get_projected_bounding_box(NUIF       *nuif,
                                              NUI_DOUBLE *top_left_x,
                                              NUI_DOUBLE *top_left_y,
                                              NUI_DOUBLE *bottom_right_x,
                                              NUI_DOUBLE *bottom_right_y);

/**
 * \brief Converts from projected coordinates to lat/long (if possible) 
 *        and pixel coordinates
 * 
 * This function takes projected coordinates of a point in the NUI file.
 * If the NUI file has a valid coordinate reference system and units, it
 * computes the latitude and longitude information and returns it. The
 * function also returns corresponding pixel coordinates for that position.
 *
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \param projected_x (in) Projected X coordinate of a point in the NUI file.
 * \param projected_y (in) Projected Y coordinate of a point in the NUI file.
 * \param pixel_x (out) Receives pixel X of that point in the NUI file.
 *        Ignored if NULL.
 * \param pixel_y (out) Receives pixel Y of that point in the NUI file.
 *        Ignored if NULL.
 * \param latitude (out) Receives latitude of that point in the NUI file.
 *        Ignored if NULL. A coordinate reference system and GSD units are
 *        required to be present in the NUI file to compute this. If not
 *        present and latitude parameter was specified, it is filled with
 *        a very large negative number which clearly indicates it is invalid.
 * \param longitude (out) Receives longitude of that point in the NUI file.
 *        Ignored if NULL. A coordinate reference system and GSD units are
 *        required to be present in the NUI file to compute this. If not
 *        present and longitude parameter was specified, it is filled with
 *        a very large negative number which clearly indicates it is invalid.
 * \return 0 on success, -1 on failure.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_convert_from_projected_coordinates(NUIF       *nuif,
                                                      NUI_DOUBLE  projected_x,
                                                      NUI_DOUBLE  projected_y,
                                                      NUI_DOUBLE *pixel_x,
                                                      NUI_DOUBLE *pixel_y,
                                                      NUI_DOUBLE *latitude,
                                                      NUI_DOUBLE *longitude);
/**
 * \brief Converts from pixel coordinates to lat/long (if possible) 
 *        and projected coordinates
 * 
 * This function takes pixel coordinates of a point in the NUI file.
 * If the NUI file has a valid coordinate reference system and units, it
 * computes the latitude and longitude information. The function
 * also returns corresponding projected coordinates for that position.
 *
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \param pixel_x (in) Pixel X coordinate of a point in the NUI file.
 * \param pixel_y (in) Pixel Y coordinate of a point in the NUI file.
 * \param projected_x (out) Receives projected X of that point
 *        in the NUI file. Ignored if NULL.
 * \param projected_y (out) Receives projected Y of that point
 *        in the NUI file. Ignored if NULL.
 * \param latitude (out) Receives latitude of that point in the NUI file.
 *        Ignored if NULL. A coordinate reference system and GSD units are
 *        required to be present in the NUI file to compute this. If not
 *        present and latitude parameter was specified, it is filled with
 *        a very large negative number which clearly indicates it is invalid.
 * \param longitude (out) Receives longitude of that point in the NUI file.
 *        Ignored if NULL. A coordinate reference system and GSD units are
 *        required to be present in the NUI file to compute this. If not
 *        present and longitude parameter was specified, it is filled with
 *        a very large negative number which clearly indicates it is invalid.
 * \return 0 on success, -1 on failure.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_convert_from_pixel_coordinates(NUIF       *nuif,
                                                  NUI_DOUBLE  pixel_x,
                                                  NUI_DOUBLE  pixel_y,
                                                  NUI_DOUBLE *projected_x,
                                                  NUI_DOUBLE *projected_y,
                                                  NUI_DOUBLE *latitude,
                                                  NUI_DOUBLE *longitude);

/**
 * \brief Given proper geo-spatial reference information in the NUI file,
 *        this function onverts from latitude/longitude to pixel and
 *        and projected coordinates for that point in the NUI file.
 * 
 * This function takes latitude/longitude of a point in the NUI file.
 * If the NUI file has a valid coordinate reference system and units, it
 * computes the pixel and projected coordinates for that point.
 *
 * \param nuif (in) A non-NULL NUIF object with a valid NUIF_GEO in it.
 * \param latitude (in) Latitude of a point in the NUI file.
 * \param longitude (in) Longitude of a point in the NUI file.
 * \param pixel_x (out) Receives pixel X of that point in the NUI file.
 *        Ignored if NULL.
 * \param pixel_y (out) Receives pixel Y of that point in the NUI file.
 *        Ignored if NULL.
 * \param projected_x (out) Receives projected X of that point
 *        in the NUI file. Ignored if NULL.
 * \param projected_y (out) Receives projected Y of that point
 *        in the NUI file. Ignored if NULL.
 * \return 0 on success, -1 on failure.
 */
NUI_SDK_API
NUI_INT32 NUIF_GEO_convert_from_latlong(NUIF       *nuif,
                                        NUI_DOUBLE  latitude,
                                        NUI_DOUBLE  longitude,
                                        NUI_DOUBLE *pixel_x,
                                        NUI_DOUBLE *pixel_y,
                                        NUI_DOUBLE *projected_x,
                                        NUI_DOUBLE *projected_y);

/****************************************************************
 * The above mentioned NUIF_GEO functions are temporary and may 
 * evolve over time.
 * End of functions.
 ****************************************************************/

/*****************************************************************
 * The NUIF_PROPERTY interface retrieves information about 
 * metadata stored in the file. Property constants are listed
 * in nuif_constants.h.
 *****************************************************************/

/** 
 * \brief Returns the first property in a NUI file.
 * \param nuif (in) A valid non-NULL NUIF object pointer.
 * \returns The first property in a NUI file or NULL if no properties.
 *          Having no properties in not and error.
 */
NUI_SDK_API
NUIF_PROPERTY *NUIF_PROPERTY_get_first(NUIF *nuif); 

/** 
 * \brief Returns the next property, after specified one, in a NUI file.
 * \param property (in) A valid non-NULL NUIF_PROPERTY object pointer.
 * \returns The next property in a NUI file after or NULL if end of list.
 */
NUI_SDK_API
NUIF_PROPERTY *NUIF_PROPERTY_get_next(NUIF_PROPERTY *property);

/** 
 * \brief Returns the last property in a NUI file.
 * \param nuif (in) A valid non-NULL NUIF object pointer.
 * \returns The last property in a NUI file or NULL if no properties.
 *          Having no properties is not an error.
 */
NUI_SDK_API
NUIF_PROPERTY *NUIF_PROPERTY_get_last(NUIF *nuif); 
/** 
 * \brief Returns the previous property, before specified one, in a NUI file.
 * \param property (in) A valid non-NULL NUIF_PROPERTY object pointer.
 * \returns The previous property in a NUI file or NULL if end of list.
 */
NUI_SDK_API
NUIF_PROPERTY *NUIF_PROPERTY_get_previous(NUIF_PROPERTY *property);

/** 
 * \brief Gets information about this property.
 * \param property (in) A valid non-NULL NUIF_PROPERTY object pointer.
 * \param id (out) Address to hold the identifier of the property.
 * \param name (out) Address to hold the name of the property.
 * \param secondary_id (out) Address to hold the secondary identifier.
 * \param secondary_name (out) Address to hold the secondary id name.
 * \param type (out) Address to hold property's data type.
 * \param total_values (out) Address to hold total values of specified 
 *        data type.
 * \param total_rows (out) Address to hold the total rows if \b type 
 *        was a matrix.
 * \param total_columns (out) Address to hold the total columns if 
 *        \b type was a matrix.
 * \param total_bytes (out) Address to hold the total bytes allocated to
 *        hold all the values.
 * \note Besides \b property (which can't be NULL) if any other parameter
 *       is NULL, it is ignored.
 * \returns 0 on success, -1 if unable to get property information.
 * \sa NUIF_PROPERTY_set_parameters()
 */
NUI_SDK_API
NUI_INT32 NUIF_PROPERTY_get_parameters(NUIF_PROPERTY *property,
                                NUI_UINT32         *id,
                                NUI_CHAR           *name,
                                NUI_UINT32         *secondary_id,
                                NUI_CHAR           *secondary_name,
                                NUIF_PROPERTY_TYPE *type,
                                NUI_UINT32         *total_values,
                                NUI_UINT32         *total_rows,
                                NUI_UINT32         *total_columns,
                                NUI_UINT32         *total_bytes);

/**
 * \brief Support function. Returns name of a property from its ID
 * \param property_id (in) A valid property ID (one of NUIF_PROPERTY_ID...)
 *                    as specified in nuif_constants.h
 * \returns Name of the property as assigned by the system or NULL
 *          if property ID was not one of NUIF_PROPERTY_ID... 
 *          No error code is set.
 */
NUI_SDK_API
NUI_CHAR *NUIF_PROPERTY_get_name_by_id(NUI_UINT32 property_id);

/**
 * \brief Returns values that the property has.
 * \param property (in) A valid non-NULL NUIF_PROPERTY object pointer.
 * \returns A void pointer to a list of 0, 1 or more values of specified
 *          data type occupying specified number of bytes in memory.
 * \warning Do not free the memory returned by this function.
 * \sa NUIF_PROPERTY_get_parameters()
 */
NUI_SDK_API
void      *NUIF_PROPERTY_get_values(NUIF_PROPERTY *property);

/**
 * \brief Finds a property of specified ID
 *
 * There may be several properties in a NUI file with the same
 * property ID. 
 * - If \b property is NULL, the function traverses the
 * entire list of properties in \b nuif and returns the property
 * which has the first occurance of \b id. 
 * - If \b property is not NULL, the function
 * traverses the entire list of properties in \b nuif until it 
 * encounters the specified \b property. It then starts to search for 
 * the first occurance of \b id in the rest of the property list \b 
 * after the specified \b property and returns the property, if found.
 *
 * \param nuif (in) A valid non-NULL NUIF object pointer with properties.
 * \param property (in) A valid NUIF_PROPERTY object pointer or NULL.
 * \param id (in) Identifier of the properties to find. There may be
 *        several properties in a NUI file with the same property ID,
 * \returns A pointer to the property object just found or NULL if
 *          \b property is not  a part of \b nuif or if \b id was not
 *          found, setting the error code appropriately.
 * \warning The entire property header needs to be loaded in memory.
 */
NUI_SDK_API
NUIF_PROPERTY *NUIF_PROPERTY_find_by_id(NUIF          *nuif, 
                                        NUIF_PROPERTY *property,
                                        NUI_UINT32    id);

/**
 * \brief Prints properties to a file
 * \param filename (in) Output text file name. If NULL, output is redirected
 *        to \b stdout.
 * \param property (in) A valid non-NULL NUIF_PROPERTY object pointer.
 */
NUI_SDK_API
void           NUIF_PROPERTY_print(NUI_CHAR      *filename, 
                                   NUIF_PROPERTY *property);


/**
 * \brief Returns information on the property in a formatted string
 *
 * \param property (in) Pointer to non-NULL valid NUIF_PROPERTY object
 * \param as_html (in) If NUI_TRUE, information is formatted in HTML format
 *        and otherwise it is formatted to be printed on a console screen.
 * \returns Pointer to statically allocated character string that has
 *        information about a NUI 2.2 file property.
 * \warning Do not free return string. Immediately copy it to another 
 *        location in your program.
 */
NUI_SDK_API
NUI_CHAR  *NUIF_PROPERTY_get_formatted_information(NUIF_PROPERTY *property,
                                                   NUI_INT32      as_html);

#ifdef __cplusplus
}
#endif

#endif /* _NUIF_GET_H_HAS_BEEN_INCLUDED_ */

/*
 * nuif_get.h ends
 */
