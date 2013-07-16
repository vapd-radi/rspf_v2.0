/*
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 */
/**
 * \file nuif_types.h
 * \section NUIF_TYPES
 *
 * \brief NUIF API needs enumerated data types and other type definitions
 *        All of those are provided in this file.
 *
 * \note  Constants with a MAFC prefix are from the MAFC library. This is
 *        an internal API.
 *
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * 
 * \authors Rahul Thakkar, Saurabh Sood
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuif_types.h,v 2.1 2006/12/04 22:46:40 thakkarr Exp $
 * $Log: nuif_types.h,v $
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
 * Revision 1.2  2005/06/13 23:02:32  thakkarr
 * Added functions and support constants for getting geo-spatial information
 * like coordinate system WKT, name, units name, and bounding box.
 *
 * Revision 1.1  2005/01/19 15:00:59  thakkarr
 * File is now added to CVS
 *
 *
 */

#ifndef _NUIF_TYPES_H_HAS_BEEN_INCLUDED_
#define _NUIF_TYPES_H_HAS_BEEN_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
 * NUIF - The code file object. All NUI 2.2 file operations
 * happen using this object
 ***********************************************************/
/**
 * \brief NUIF - This object represents a NUI image file
 *
 * All operations on a NUI file are performed through this object. All
 * read and write operations on image data are performed on a per-tile
 * basis. A tile is a single logical image unit. 
 *
 * It consists of: NUIF_CHANNEL, NUIF_PROPERTY, NUIF_LEVEL and NUIF_GEO objects.
 *
 * \sa NUIF_CHANNEL, NUIF_PROPERTY, NUIF_LEVEL, NUIF_GEO
 */
typedef struct mafc_struct                NUIF;

/***********************************************************
 * NUIF_LEVEL - This object holds information about what
 * bands are stored in the NUI 2.2 file and how.
 ***********************************************************/
/**
 * \brief Pyramid level object. 
 *
 * A pyramid level is a reduced resolution version of the original image.
 * It is stored as a part of the NUI 2.2 file. If the original image
 * changes, it must be updated in all pyramid levels. 
 *
 * The user can determine the size of each pyramid level, as long as each
 * subsequent pyramid level is smaller than the previous pyramid level.
 *
 * It is a doubly linked list. The first pyramid level in an image is
 * \b always the \b original image. The first pyramid level is level 0.
 * For an image with N pyramid levels, they are numbered from 0 (original
 * image) to N-1 (smallest reduced resolution version of the level 0).
 */
typedef struct mafc_pyramid_level_struct  NUIF_LEVEL;

/***********************************************************
 * NUIF_CHANNEL - This object holds information about what
 * bands are stored in the NUI 2.2 file and how.
 ***********************************************************/
/**
 * \brief Channel (or Band) object. 
 *
 * It is a doubly linked list. Holds properties of each channel (or band) in 
 * the NUI 2.2 file. All operations to a channel happen using this object.
 * The NUIF_CHANNEL API has get/set functions to manipulate information 
 * in this object. 
 */
typedef struct mafc_channel_struct        NUIF_CHANNEL;

/***********************************************************
 * NUIF_PROPERTY - This object holds information about all 
 * the metadata elements that are stored in the NUI 2.2 file.
 ***********************************************************/
/**
 * \brief A property object provides information about one 
 *        particular feature about a NUI image file.
 *
 * For example: File name, citations, comments, color space conversion
 * matrices, statistics, histogram are all properties. Properties are
 * stored in file as a list at the end of file, and in memory as a 
 * linked list. All metadata forms NUI file properties.
 * Each NUIF_PROPERTY object has an entry in the NUI 2.2 file header.
 *
 * Properties are of several types.
 * - NUI 2.2 specific properties
 * - Metadata associated with the source image
 * - Custom user defined properties
 *
 * See NUIF_PROPERTY API to manage properties.
 */
typedef struct mafc_property_struct       NUIF_PROPERTY;

/***********************************************************
 * NUIF_GEO, NUIF_GEO_KEY, NUIF_GEO_KEY_VALUE - These objects 
 * hold information about geo-spatial information stored in 
 * the NUI 2.2 file.
 ***********************************************************/
/**
 * \brief Specifies a geo-key value inside a NUIF_GEO_KEY
 * \sa NUIF_GEO_KEY
 */
typedef union  mafc_geo_key_value_union   NUIF_GEO_KEY_VALUE;

/**
 * \brief A geo-key data is stored in memory as this object
 *
 * Each NUIF_GEO_KEY object has a corresponding entry in the NUI 2.2 file
 * describing the type of geo-key and the data it contains.
 * \sa NUIF_GEO
 */
typedef struct mafc_geo_key_struct        NUIF_GEO_KEY;

/**
 * \brief Defines all geo-spatial data, in memory, from a NUI 2.2 file
 *
 * See NUIF_GEO get/set functions for more information on how to manage
 * this object.
 */
typedef struct mafc_geo_struct            NUIF_GEO;

/************************************************************
 * The NUI file format describes a lot of its parameters 
 * using enumerated data types. The following are all of those
 * data types.
 ************************************************************/

/**
 * \brief Specifies the codec used for compressing/decompressing a NUI file.
 * \sa NUIF_get_image_parameters(), NUIF_set_image_parameters()
 */
typedef enum {
    NUIF_CODEC_J2K  = 1, /*!< Compressed using JPEG2000 codec */
    NUIF_CODEC_NONE = 0  /*!< Image is uncompressed */
} NUIF_CODEC;

/**
 * \brief A developer can create a NUI file, read from a NUI file, or
 *        edit data in a NUI file. All operations can be performed on
 *        a per-tile basis. 
 * \sa NUIF_open_file()
 */
typedef enum {
    NUIF_MODE_INVALID   = 0, /*!< Internal use. Never pass to function */
    NUIF_MODE_READONLY  = 1, /*!< Open NUI file for read-only */
    NUIF_MODE_CREATE    = 2, /*!< Create NUI file */
    NUIF_MODE_READWRITE = 3  /*!< Open NUI file for read-and-write */
} NUIF_MODE;

/**
 * \brief If number of channels is > 1, pixel data may be laid out in file
 *        as packed or planar based on NUIF_DATA_LAYOUT.
 * \sa NUIF_get_image_parameters(), NUIF_set_image_parameters()
 */
typedef enum {
    NUIF_DATA_LAYOUT_PACKED = 1, /*!< Packed layout (RGBRGBRGB...) */
    NUIF_DATA_LAYOUT_PLANAR = 2, /*!< Planar layout (RRR... GGG... BBB...) */
    NUIF_DATA_LAYOUT_TEXTURE = 200 /*!< Use NUIF_TEXTURE_TYPE for layout */
} NUIF_DATA_LAYOUT;

/**
 * \brief This data type is for information only. It informs the developer
 *        about how uni-channel or multi-channel data is laid out in a tile.
 * \sa NUIF_get_image_parameters(), NUIF_set_image_parameters()
 */
typedef enum {
    NUIF_LUMINANCE = 1, /*!< 1-channel NUI, luminance (mono) */
    NUIF_RGB       = 2, /*!< 3-channel NUI, R,G,B  */
    NUIF_BGR       = 3, /*!< 3-channel NUI, B,G,R  */
    NUIF_RGBA      = 4, /*!< 4-channel NUI, R,G,B,Alpha */
    NUIF_ABGR      = 5, /*!< 4-channel NUI, Alpha,B,G,R */
    NUIF_RGBX      = 6, /*!< 4-channel NUI, R,G,B,Empty */
    NUIF_XBGR      = 7, /*!< 4-channel NUI, Empty,B,G,R */
    NUIF_CMYK      = 8, /*!< 4-channel NUI, C,M,Y,K */
    NUIF_COMPLEX   = 9, /*!< 2-channel NUI, X + iY (X=Real, Y=imaginary) */
    NUIF_IR        = 10, /*!< 1-channel NUI, Infra-red (mono) */
    NUIF_RGBIR     = 11, /*!< 4-channel NUI, R,G,B,Infra-red */
    NUIF_IRBGR     = 12, /*!< 4-channel NUI, Infra-red,B,G,R */
    NUIF_CUSTOM    = 100, /*!< N-channel NUI, custom, uses channel name */
    NUIF_TEXTURE   = 200  /*!< use NUIF_TEXTURE_TYPE for data type */
} NUIF_DATA_TYPE;

/**
 * \brief Specifies bits/channel (the term channel means a spectral-band)
 *        in a NUI file.
 *
 * \sa NUIF_get_image_parameters(), NUIF_set_image_parameters(),
 *     NUIF_get_total_bits_per_channel(), NUIF_get_total_bytes_per_channel(),
 *     NUIF_get_total_bytes_per_pixel()
 */
typedef enum {
    NUIF_BITS_8  = 8,  /*!< Stored as unsigned 8-bits/channel */
    NUIF_BITS_10 = 10, /*!< Stored as signed 16-bits/channel, using 10 LSB */
    NUIF_BITS_11 = 11, /*!< Stored as signed 16-bits/channel, using 11 LSB */
    NUIF_BITS_12 = 12, /*!< Stored as signed 16-bits/channel, using 12 LSB */
    NUIF_BITS_16 = 16, /*!< Stored as signed 16-bits/channel */
    NUIF_BITS_32 = 32, /*!< Stored as signed 32-bits/channel */
    NUIF_BITS_64 = 64, /*!< Stored as signed 64-bits/channel */
    NUIF_BITS_S8  = 508,  /*!< Stored as signed 8-bits/channel */
    NUIF_BITS_S16 = 16, /*!< Stored as signed 16-bits/channel */
    NUIF_BITS_S32 = 32, /*!< Stored as signed 32-bits/channel */
    NUIF_BITS_S64 = 64, /*!< Stored as signed 64-bits/channel */
    NUIF_BITS_U8  = 8,  /*!< Stored as unsigned 8-bits/channel */
    NUIF_BITS_U16 = 516, /*!< Stored as unsigned 16-bits/channel */
    NUIF_BITS_U32 = 532, /*!< Stored as unsigned 32-bits/channel */
    NUIF_BITS_U64 = 564, /*!< Stored as unsigned 64-bits/channel */
    NUIF_BITS_FLOAT  = 132, /*!< Stored as 32-bits/channel, floating point */
    NUIF_BITS_DOUBLE = 164, /*!< Stored as 64-bits/channel, double */
    NUIF_BITS_TEXTURE = 200 /*!< use NUIF_TEXTURE_TYPE for bits/pixel */
} NUIF_BITS_PER_CHANNEL;

/**
 * \brief Specifies texture_type in NUIF_get/set_texture_type().
 * 
 * In general, the layout of a pixel is defined using data layout, data type,
 * bits/channel and number of channels. There is a special case: Image
 * data in a NUI 2.2 file needs be used as a texture in an OpenGL texture
 * server engine. If so, use NUIF_set_texture_type() to set the texture 
 * as an OpenGL texture with either RGBA data stored in 16-bits/pixel as 
 * 5 bits red, 5 bits green, 5 bits blue, 1 bit alpha or in 32-bits/pixel as 
 * 8 bits each for red, green, blue and alpha. By default, the texture type
 * is NUIF_TEXTURE_TYPE_NONE indicating that the image is NOT to be used as
 * an OpenGL texture map.
 * 
 * If set as a texture map of either 5551 or 8888 type, NUIF_BITS_PER_CHANNEL,
 * NUIF_DATA_TYPE and NUIF_DATA_LAYOUT are all set to NUIF_...TEXTURE.
 *
 * \sa NUIF_set_texture_type(), NUIF_get_texture_type()
 */
typedef enum {
    NUIF_TEXTURE_TYPE_NONE = 0, 
    /*!< Normal image as set by other parameters */
    NUIF_TEXTURE_TYPE_OPEN_GL_RGBA_5551 = 2, 
    /*!< OpenGL texture, RGBA layout as 5:5:5:1 bits with 16 bits/pixel */
    NUIF_TEXTURE_TYPE_OPEN_GL_RGBA_8888 = 3
    /*!< OpenGL texture, RGBA layout as 8:8:8:8 bits with 32 bits/pixel */
} NUIF_TEXTURE_TYPE;

/**
 * \brief A geo-spatial key's value can be one of three data types.
 * \sa NUIF_GEO_KEY_VALUE
 */
typedef enum {
    NUIF_GEO_KEY_SHORT = 2,
    NUIF_GEO_KEY_ASCII = 5,
    NUIF_GEO_KEY_DOUBLE = 7
} NUIF_GEO_KEY_TYPE;

/**
 * \brief For use with NUIF_GEO_get_coordinate_system_as_OpenGIS_WKT()
 * 
 * OpenGIS Well Known Text is a standard character string in BNR form.
 * This string is understood by most GIS applications including ESRI
 * tools. For interoperability between NUI file format and 3rd party
 * software. It is essential to have the ability in the nuiSDK to 
 * query geo-spatial coordinate system information in this format. 
 * Several string forms have been provided to work with various 
 * 3rd party systems.
 *
 * \sa NUIF_GEO_get_coordinate_system_as_OpenGIS_WKT()
 * \warning This type may be obsoleted in the near future as a new
 *          version of geo-spatial engine is implemented.
 */
typedef enum {
    NUI_OPENGIS_WKT_STANDARD = 1,
    /*!< Standard format of the OpenGIS Well Known Text */
    NUI_OPENGIS_WKT_FORMATTED = 2,
    /*!< Standard format of the OpenGIS Well Known Text 
         which is nicely formatted for printing on the 
         console with carriage returns */
    NUI_OPENGIS_WKT_FOR_ESRI = 3, 
    /*!< ESRI compatible format of the OpenGIS Well Known Text */
    NUI_OPENGIS_WKT_FOR_PROJ4 = 4 
    /*!< PROJ API compatible format of the OpenGIS Well Known Text */
} NUIF_OPENGIS_WKT_FORMAT;

/**
 * \brief Specifies what type of data does a property represent
 * \sa NUIF_PROPERTY
 */
typedef enum {
    NUIF_PROPERTY_TYPE_UCHAR = 1,   /*!< Unsigned char value(s) */
    NUIF_PROPERTY_TYPE_DOUBLE = 2,  /*!< Double precision float value(s) */
    NUIF_PROPERTY_TYPE_COMPLEX = 3, /*!< Double precision value pair(s) */
    NUIF_PROPERTY_TYPE_UINT16 = 4,  /*!< Unsigned 16 bit value(s) */
    NUIF_PROPERTY_TYPE_UINT32 = 5,  /*!< Unsigned 32 bit value(s) */
    NUIF_PROPERTY_TYPE_UINT64 = 6,  /*!< Unsigned 64 bit value(s) */
    NUIF_PROPERTY_TYPE_INT16 = 7,   /*!< Signed 16 bit value(s) */
    NUIF_PROPERTY_TYPE_INT32 = 8,   /*!< Signed 32 bit value(s) */
    NUIF_PROPERTY_TYPE_INT64 = 9,   /*!< Signed 64 bit value(s) */
    NUIF_PROPERTY_TYPE_STRING = 10, /*!< NULL terminated string */
    NUIF_PROPERTY_TYPE_MATRIX = 11  /*!< Matrix of doubles */
} NUIF_PROPERTY_TYPE;


#ifdef __cplusplus
}
#endif

#endif /* _NUIF_TYPES_H_HAS_BEEN_INCLUDED_ */

/*
 * nuif_types.h ends
 */
