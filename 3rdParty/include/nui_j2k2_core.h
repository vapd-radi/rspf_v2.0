/*
 * Confidential Intellectual Property of Pixia Corp.
 * Copyright (C) 2004 Pixia Corp. All rights reserved.
 *
 * Tags beginning with a backslash are for automatic documentation generation.
 */
/**
 * \file nui_j2k2_core.h
 * \brief The NUI_J2K library provides JPEG2000 codec support.
 *
 * The NUI_J2K2 core provides basic compression and decompression
 * functionality. This is supplemented by functions that help the user to
 * create and manage input and output buffers.
 *
 * Typical workflows for comression and decompression are shown below. Some
 * function definitions include code examples.
 *
 * A typical image compression workflow migtht include the following steps:
 *  - Load raw (input) image into user supplied buffer.
 *  - Create NUI_J2K2_ENCODE object.
 *  - Set input image information (width, height, etc.) including the pointer
 *    to and size of input buffer in encoder object.
 *  - Set desired compression ratio in encoder object.
 *  - Create output buffer using the size recommended by API 
 *  - Set output buffer and its size in encoder object.
 *  - Perform compression by passing the encoder object to the compress
 *    function.
 *  - Save compressed JPEG2000 codestream
 *
 * A typical image decompression workflow migtht include the following steps:
 *  - Load j2c (input) file into user supllied buffer.
 *  - Create NUI_J2K2_DECODE object.
 *  - Set the input buffer and its size in decoder object.
 *  - Create output buffer using size query function 
 *  - Set output buffer and its size in decoder object.
 *  - Perform decompression by passing the decoder object to the decompress
 *    function.
 *  - Manage the decompressed raw image
 *
 * \authors Saurabh Sood, Alan Black
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui_j2k2/nui_j2k2_core.h,v 2.3 2007/04/17 13:59:03 soods Exp $
 * $Log: nui_j2k2_core.h,v $
 * Revision 2.3  2007/04/17 13:59:03  soods
 * Enabled progression with Kakadu 5.2 by adding nui_synthesis
 *
 * Revision 2.2  2007/03/09 20:04:02  soods
 * Added support for JP2 decompression
 *
 * Revision 2.1  2007/03/01 15:12:16  jensenk
 * Disabled progressive decoding for VS2005 and Kakadu 2005 at compile time.
 *
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.14  2005/08/19 19:49:31  soods
 * Fixed initialization of j2c_format in NUI_J2K2_ENCODE. comments updated for
 * default parameters.
 *
 * Revision 1.13  2005/08/03 19:50:45  thakkarr
 * Exported all functons
 *
 * Revision 1.12  2005/06/07 19:06:48  soods
 *  Fixed comment on return value in NUI_J2K2_DECODE_query_raw_image_size
 *
 * Revision 1.11  2005/05/09 20:22:46  soods
 *  Added more data members for progression callback
 *
 * Revision 1.10  2005/05/04 19:49:19  soods
 *  Added compression using reversible wavelet and fixed small bug in callback
 *
 * Revision 1.9  2005/05/03 18:28:05  soods
 * Added the resolution progression while decompressing
 *
 * Revision 1.8  2005/03/16 16:18:34  soods
 * some coding standard improvements
 *
 * Revision 1.7  2005/03/15 23:12:06  soods
 *  fixed small bug in YUV 422 buffer free() , eliminated warnings
 *
 * Revision 1.6  2005/03/15 21:35:47  soods
 * Misc Small Improvements (J2C codestream explicit check, coding standards,
 * ADV202 Disabled, ADV202 SDK enable/disabled streamlined)
 *
 * Revision 1.5  2005/03/09 16:12:57  blacka
 * Added ability to create both YUV422 and YUV444 (standard), upgraded to
 * NUI_YUV library.
 *
 * Revision 1.4  2005/03/01 19:15:44  blacka
 * Timing functions added
 *
 * Revision 1.3  2005/02/25 15:11:30  wangj
 * Minor changes
 *
 * Revision 1.2  2005/02/24 16:19:28  blacka
 * Added hardware codec support for Analog Devices ADV202 (Windows ONLY!)
 *
 * Revision 1.1  2005/01/31 12:02:24  blacka
 * Added to CVS.
 *
 */
#ifndef _NUI_J2K2_CORE_H_HAS_BEEN_INCLUDED_
#define _NUI_J2K2_CORE_H_HAS_BEEN_INCLUDED_

#include <stdlib.h>  /* Included for size_t */

#include <nui_constants.h>

#define _NUI_J2K2_ENABLE_ADV202_SUPPORT_ /* Enable the ADV202 harware SDK */
#undef _NUI_J2K2_ENABLE_ADV202_SUPPORT_  /* Disable the ADV202 harware SDK */

/*
 * Constants
 */
#define NUI_J2K2_MIN_COMPRESSION_RATIO       2.0
    /*!< Minimum allowed compression ratio (always greater than 1.0) */  
#define NUI_J2K2_MAX_COMPRESSION_RATIO       200.0
    /*!< Maximum allowed compression ratio */

/**
 * \brief Specifies the compression/decompression SDK
 *
 * The NUI_J2K2 library provides support for the JPEG2000 image compression
 * standard via third-party SDK's (software development kits). The
 * NUI_J2K2_SDK data type definition enumerates the SDK's supported by
 * the NUI_J2K2 library.
 *
 * 1 = Kakadu
 * 2 = ADV202
 */
typedef enum {
	NUI_J2K2_SDK_KAKADU = 1,
    NUI_J2K2_SDK_ADV202 = 2
} NUI_J2K2_SDK;

typedef enum {
    NUI_J2K2_FORMAT_STANDARD = 1, /* SDK manages color space conversion */
    NUI_J2K2_FORMAT_YUV411   = 2, /* YUV 4:1:1 done external to SDK */
    NUI_J2K2_FORMAT_YUV422   = 3, /* YUV 4:2:2 done external to SDK */
    NUI_J2K2_FORMAT_YUV444   = 4, /* YUV 4:4:4 done external to SDK */
    NUI_J2K2_FORMAT_JP2,          /* JP2 stream */
    NUI_J2K2_FORMAT_END
} NUI_J2K2_FORMAT;

typedef enum {
    NUI_J2K2_PROGRESSION_NONE       = 1, /* No progression */
    NUI_J2K2_PROGRESSION_RESOLUTION = 2  /* Progression based on resolution */
} NUI_J2K2_PROGRESSION_TYPE;

typedef enum {
    NUI_J2K2_PROGRESSION_SUCCESS = 1,   /* Progression successful  */
    NUI_J2K2_PROGRESSION_FAILURE = 2,   /* Progression failure     */
    NUI_J2K2_PROGRESSION_INTERRUPT = 3  /* Progression interrupted */
} NUI_J2K2_PROGRESSION_STATE;

/**
 * \brief Specifies NUI_J2K2_ENCODE structure used to compress raw images into
 * JPEG2000 code streams.
 *
 * The NUI_J2K2_ENCODE structure provides pointers to the (input) uncompressed
 * image and the (output) resultant JPEG2000 code stream buffers. The
 * structure also contains compression parameters and input image
 * information.
 *
 * \warning The JPEG2000 code stream (output) buffer MUST be allocated by the
 * user. In order to allocate output buffer memory before calling the encode
 * function, use the NUI_J2K2_ENCODE_recommend_code_stream_mem_size function.
 *
 * \sa NUI_J2K2_ENCODE_recommend_code_stream_mem_size
 * \sa NUI_J2K2_ENCODE_set_code_stream_mem_size
 */
typedef struct nui_j2k2_encode NUI_J2K2_ENCODE;

/**
 * \brief Specifies NUI_J2K2_DECODE structure used to decompress JPEG2000 code
 * streams into raw images.
 *
 * The NUI_J2K2_DECODE structure provides pointers to the (input) JPEG2000 code
 * stream and the (output) raw image buffers. The structure also contains
 * output image information.
 *
 * \warning The raw image (output) buffer MUST be allocated by the
 *          user. In order to allocate output buffer memory before 
 *          calling the decode function, use the 
 *          NUI_J2K2_DECODE_query_raw_image_mem_size function.
 * 
 * \sa NUI_J2K2_DECODE_query_raw_image_size
 * \sa NUI_J2K2_DECODE_set_raw_image_mem_size
 */
typedef struct nui_j2k2_decode NUI_J2K2_DECODE;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates an empty NUI_J2K2_ENCODE object
 *  The default j2c_format assumed is NUI_J2K2_FORMAT_STANDARD 
 *  The default compression ratio assumed is NUI_J2K2_MIN_COMPRESSION_RATIO
 *
 * \param sdk (in) - enumerated constant that determines which SDK will be 
 *                   used for encoding (compression)
 * \returns A valid, empty abstract NUI_J2K2_ENCODE object or NULL on failure,
 *          setting the error code appropriately
 *
 * \sa NUI_J2K2_SDK
 * \sa NUI_J2K2_ENCODE_set_compression_ratio()
 * \sa NUI_J2K2_ENCODE_set_j2c_format()
 */
NUI_SDK_API
NUI_J2K2_ENCODE *
NUI_J2K2_ENCODE_create(NUI_J2K2_SDK sdk);

/**
 * \brief Destroys a valid NUI_J2K2_ENCODE object and frees all 
 *        resources used by it.
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL 
 *                            NUI_J2K2_ENCODE object
 */
NUI_SDK_API
void
NUI_J2K2_ENCODE_destroy(NUI_J2K2_ENCODE *j2k2_encoder);

/**
 * \brief Sets compression ratio for JPEG2000 encoding
 *
 * This function sets the target compression ratio 
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param ratio (in) - Target compression ratio 
 *                     (Final size is 1/ratio times the input size)
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to
 *          NUI_J2K2_ERR_RATIO
 *
 * \note This library does not support 1:1 (i.e. lossless) compression.
 * \warning The compression ratio value must be greater than or equal to
 *          NUI_J2K2_MIN_COMPRESSION_RATIO and less than or equal to
 *          NUI_J2K2_MIN_COMPRESSION_RATIO
 *
 * \sa NUI_J2K2_MIN_COMPRESSION_RATIO
 * \sa NUI_J2K2_MAX_COMPRESSION_RATIO
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_ENCODE_set_compression_ratio(NUI_J2K2_ENCODE *j2k2_encoder,
                                      NUI_DOUBLE       ratio);

/**
 * \brief Gets compression ratio for JPEG2000 encoding
 *
 * This function gets the target compression ratio parameter in a
 * NUI_J2K2_ENCODE object.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * 
 * \returns Target compression ratio as a double on success or -1 on
 *          failure with NUI_J2K2_errno set to NUI_J2K2_ERR_INCOMPLETE
 */
NUI_SDK_API
NUI_DOUBLE
NUI_J2K2_ENCODE_get_compression_ratio(NUI_J2K2_ENCODE *j2k2_encoder);

/**
 * \brief Sets JPEG2000 code stream (j2c) format
 *
 * This function sets the j2c format which determines how color conversions
 * and image component downsampling is handled.
 * 
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param j2c_format (in) - a NUI_J2K2_FORMAT indicating the desired format
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to 
 *          NUI_J2K2_ERR_FORMAT in the case of an unusable format
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_ENCODE_set_j2c_format(NUI_J2K2_ENCODE *j2k2_encoder,
                               NUI_J2K2_FORMAT  j2c_format);

/**
 * \brief Gets JPEG2000 code stream (j2c) format
 * 
 * This function returns the j2c format which determines how color conversions
 * and image component downsampling is handled.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 *
 * \param j2c_format (out) - a NUI_J2K2_FORMAT indicating the desired format
 *                           or 0 on error with NUI_J2K2_errno set to 
 *                           NUI_J2K2_ERR_FORMAT 
 * \returns Format of the color space to be used
 * \sa NUI_J2K2_FORMAT
 */
NUI_SDK_API
NUI_J2K2_FORMAT
NUI_J2K2_ENCODE_get_j2c_format(NUI_J2K2_ENCODE *j2k2_encoder);

/**
 * \brief Sets raw (input) image parameters for compression engine
 *
 * This function sets the parameters that describe the raw image to the
 * compression engine in terms of width, height, number of channels and bits
 * per channel.
 * Single channel images will be treated as grayscale. 3 channel images will
 * be treated as packed RGB (in that order).
 * Images may have a bit depth from 8 to 16 bits per channel.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param width (in) - raw (input) image width in pixels
 * \param height (in) - raw (input) image height in pixels
 * \param channels (in) - number of channels in the raw (input) image
 * \param bits_per_channel (in) - number of bits in each raw (input) image
 *                                channel
 * \param raw_image (in) - A valid, non-NULL pointer to raw (input) image 
 * \param raw_image_mem_size (in) - size of raw (input) image buffer
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to 
 *          NUI_J2K2_ERR_SUPPORT in the case of missing parameters and
 *          NUI_J2K2_ERR_PARAMS in the case of invalid parameter values.
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_ENCODE_set_raw_image(NUI_J2K2_ENCODE *j2k2_encoder,
                              NUI_UINT32       width,
                              NUI_UINT32       height,
                              NUI_UINT32       channels,
                              NUI_UINT32       bits_per_channel,
                              NUI_UCHAR       *raw_image,
                              size_t           raw_image_mem_size);

/**
 * \brief Gets raw (input) image parameters used by the compression engine
 *
 * This function gets the raw (input) image buffer pointer and other
 * parameters from an encoder object 
 * 
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param       width (out) - raw (input) image width in pixels
 * \param      height (out) - raw (input) image height in pixels
 * \param    channels (out) - number of channels in the raw (input) image
 * \param bits_per_channel (out) - number of bits in each raw (input) image
 *                                 channel
 * \param raw_image_mem_size (out) - size of raw (input) image buffer
 *
 * \returns Pointer to the raw (input) image buffer on success and NULL on
 *          failure with NUI_J2K2_errno set to NUI_J2K2_ERR_INCOMPLETE.
 */
NUI_SDK_API
NUI_UCHAR *
NUI_J2K2_ENCODE_get_raw_image(NUI_J2K2_ENCODE *j2k2_encoder,
                              NUI_UINT32      *width,
                              NUI_UINT32      *height,
                              NUI_UINT32      *channels,
                              NUI_UINT32      *bits_per_channel,
                              size_t          *raw_image_mem_size);

/**
 * \brief Get recommended code stream (output) buffer size
 *
 * This function returns a recommendation for the size of code stream (output)
 * buffer. This recommendation is based upon the raw (input) image dimensions 
 * and the target compression ratio value. A buffer AT LEAST this size must be
 * allocated by the user. The buffer's size must be loaded into the
 * NUI_J2K2_ENCODE structure using NUI_J2K2_ENCODE_set_code_stream_mem_size
 * before calling NUI_J2K2_ENCODE_compress_raw_image.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 *
 * \returns Recommended code stream (output) buffer size in bytes. On
 * failure, NUI_J2K2_errno is set to NUI_J2K2_ERR_PARAMS
 *
 * \sa NUI_J2K2_ENCODE_set_code_stream_mem_size
 * \sa NUI_J2K2_ENCODE_compress_raw_image
 */
NUI_SDK_API
size_t
NUI_J2K2_ENCODE_recommend_code_stream_mem_size(NUI_J2K2_ENCODE *j2k2_encoder);

/**
 * \brief Set code stream (output) buffer size
 *
 * This function sets the size of the buffer that will receive the compressed
 * JPEG2000 code stream from the compression engine.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param code_stream_buffer_size (in) - the size of the user allocated code
 *                                       stream (output) buffer
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to
 *          NUI_J2K2_ERR_PARAMS
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_ENCODE_set_code_stream_mem_size(NUI_J2K2_ENCODE *j2k2_encoder,
                                         size_t          code_stream_buffer_size);

/**
 * \brief Get code stream (output) buffer size
 *
 * This function returns the size of the buffer that will receive the
 * compressed JPEG2000 code stream from the compression engine.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 *
 * \returns Code stream (output) buffer size in bytes. On
 *          failure, NUI_J2K2_errno is set to NUI_J2K2_ERR_PARAMS
 */
NUI_SDK_API
size_t
NUI_J2K2_ENCODE_get_code_stream_mem_size(NUI_J2K2_ENCODE *j2k2_encoder);

/**
 * \brief Compress raw (input) image into a JPEG2000 code stream
 *
 * This function compresses and encodes a raw image producing a JPEG2000 code
 * stream. The output code stream may be written directly to disk as a .j2c
 * file without additional processing.
 *
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param code_stream (in/out) - pointer to user allocated code stream buffer
 * \param code_stream_size (out) - returns the actual length of the JPEG2000
 *                                 code stream within the code stream buffer
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to
 *          NUI_J2K2_ERR_ENCODE_FAIL. After an encode operation failure,
 *          a call to NUI_J2K2_error_string() will provide more information 
 *          from the encoder engine.
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_ENCODE_compress_raw_image(NUI_J2K2_ENCODE *j2k2_encoder,
                                   NUI_UCHAR       *code_stream,
                                   size_t          *code_stream_size);

/**
 * \brief Returns timing information on encoding process.
 *        Data not available in non debug mode
 * 
 * \param j2k2_encoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_ENCODE object
 * \param time_yuv_conversion (out) time taken in YUV conversion
 * \param time_api_compress (out) time taken in compression
 *
 * \returns NUI_SUCCESS
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_ENCODE_get_timing_information(NUI_J2K2_ENCODE *j2k2_encoder,
                                       NUI_INT64       *time_yuv_conversion,
                                       NUI_INT64       *time_api_compress);

/**
 * \brief Creates an empty NUI_J2K2_DECODE object
 *  The default progression type assumed is NUI_J2K2_PROGRESSION_NONE
 *
 * \param sdk (in) - enumerated constant that determines which SDK will be 
 *                   used for decoding (decompression)
 *
 * \returns Valid, empty abstract NUI_J2K2_DECODE object or NULL on failure,
 *          setting the error code appropriately
 *
 * \sa NUI_J2K2_SDK
 * \sa NUI_J2K2_DECODE_set_progression_type()
 */
NUI_SDK_API
NUI_J2K2_DECODE *
NUI_J2K2_DECODE_create(NUI_J2K2_SDK sdk);

/**
 * \brief Destroys a NUI_J2K2_DECODE object and frees all 
 *        resources used by it.
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 */
NUI_SDK_API
void
NUI_J2K2_DECODE_destroy(NUI_J2K2_DECODE *j2k2_decoder);

/**
 * \brief Sets JPEG2000 code stream to be decompressed
 *
 * The input buffer must contain a valid JPEG2000 code stream.
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param code_stream_buffer (in) - Valid, non-NULL pointer to JPEG2000 
 *                                  code stream size
 * \param code_stream_buffer_size (in) - size of code stream buffer
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to
 *          NUI_J2K2_ERR_PARAMS
 * \note Only J2C code stream is supported. If another stream is used
 *       for example JP2 it will return with failure.
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_DECODE_set_code_stream(NUI_J2K2_DECODE *j2k2_decoder,
                                NUI_UCHAR       *code_stream_buffer,
                                size_t           code_stream_buffer_size);

/**
 * \brief Gets JPEG2000 code stream to be decompressed
 *
 * This function gets the JPEG2000 code stream input buffer pointer 
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param code_stream_buffer_size (out) - size of code stream buffer
 *
 * \returns Pointer to the JPEG2000 code stream buffer
 */
NUI_SDK_API
NUI_UCHAR *
NUI_J2K2_DECODE_get_code_stream(NUI_J2K2_DECODE *j2k2_decoder,
                                size_t          *code_stream_buffer_size);

/**
 * \brief Checks to see if the selected SDK is compatible with a code stream
 *
 * This function reads the JPEG2000 code stream header and determines whether
 * or not the selected SDK can decode the given j2c code stream.
 * 
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \returns NUI_TRUE if the code stream and SDK are compatible, else NUI_FALSE
 *          with NUI_J2K2_errno set to NUI_J2K2_ERR_PARAMS if the function fails.
 * \note The pointer to the code stream must be set using
 *       NUI_J2K2_DECODE_set_code_stream before calling this function
 */
NUI_SDK_API
NUI_BOOLEAN
NUI_J2K2_DECODE_check_compatibility(NUI_J2K2_DECODE *j2k2_decoder);

/**
 * \brief Queries the JPEG2000 code stream for the raw (output) image size
 *
 * This function returns the size of the decompressed raw (output) image. This
 * allows the user to allocate output buffer memory using
 * NUI_J2K2_DECODE_set_raw_image_mem_size BEFORE the call to
 * NUI_J2K2_DECODE_decompress_code_stream if the output image size is unknown.
 * This function returns the size of the original (raw) uncompressed image by
 * reading and analyzing the JPEG2000 code stream header. The buffer's size
 * must be loaded into the NUI_J2K2_DECODE structure using
 * NUI_J2K2_DECODE_set_raw_image_mem_size before calling
 * NUI_J2K2_DECODE_decompress_code_stream.
 *
 * \note If the user knows the size of the raw (output) image contained within
 * the JPEG2000 code stream, he is free to create an output buffer without
 * using this function to extract size information from the code stream before
 * decoding.
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param width (out) - raw (output) image width in pixels
 * \param height (out) - raw (output) image height in pixels
 * \param channels (out) - number of channels in the raw (output) image
 * \param bits_per_channel (out) - number of bits in each raw (output) image
 *                                 channel
 *
 * \returns 0 on success, NUI_J2K2_errno is set to NUI_J2K2_ERR_PARAMS
 *
 * \note Calling this function will not decompress the image. Only header
 *       information is read and analyzed.
 *
 * \sa NUI_J2K2_DECODE_set_raw_image_mem_size
 * \sa NUI_J2K2_DECODE_decompress_code_stream
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_DECODE_query_raw_image_size(NUI_J2K2_DECODE *j2k2_decoder,
                                     NUI_INT32       *width,
                                     NUI_INT32       *height,
                                     NUI_INT32       *channels,
                                     NUI_INT32       *bits_per_channel,
                                     size_t          *raw_image_size,
                                     NUI_J2K2_FORMAT *j2c_format);

/**
 * \brief Set raw (output) image memory size
 *
 * This function sets the size of the raw (output) image buffer.
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param raw_image_mem_size (in) - size of the raw (output) image buffer
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to
 *          NUI_J2K2_ERR_PARAMS
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_DECODE_set_raw_image_mem_size(NUI_J2K2_DECODE *j2k2_decoder,
                                       size_t           raw_image_mem_size);
/**
 * \brief Get raw (output) image memory size
 *
 * This function returns the size of the raw (output) image buffer. 
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 *
 * \returns Size of the raw (output) image in bytes. On
 *          failure, NUI_J2K2_errno is set to NUI_J2K2_ERR_PARAMS
 *
 * \sa NUI_J2K2_DECODE_decompress_code_stream
 */
NUI_SDK_API
size_t
NUI_J2K2_DECODE_get_raw_image_mem_size(NUI_J2K2_DECODE *j2k2_decoder);

/**
 * \brief Sets the type of the progression
 *
 *  JPEG2000 has ability to generate intermediate lesser quality 
 *  images while decoding. So instead of waiting for full image to 
 *  decode, progression helps in reducing the response time or turn
 *  around time. The decoding process can also be prempted in the 
 *  middle. 
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param type (in) - type of progression
 *
 * \returns 0 on success and -1 if progression type is not supported.
 *          Progression type is set to NUI_J2K2_PROGRESSION_NONE
 *          on failure.
 *
 * \note For all progression types (except NUI_J2K2_PROGRESSION_NONE) a 
 *       callback is also required to be set by the application, otherwise 
 *       the progression defaults to NUI_J2K2_PROGRESSION_NONE.
 * \note This function, if used, is called before any calls to 
 *       NUI_J2K2_DECODE_decompress_code_stream(). If this is not called 
 *       decompression defaults to NUI_J2K2_PROGRESSION_NONE.
 *
 * \sa NUI_J2K2_DECODE_set_decompress_callback()
 * \sa NUI_J2K2_PROGRESSION_TYPE
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_DECODE_set_progression_type(NUI_J2K2_DECODE            *j2k2_decoder,
                                     NUI_J2K2_PROGRESSION_TYPE  type);

/**
 * \brief Gets the type of the progression
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 *
 * \returns Type of progression 
 * \sa NUI_J2K2_PROGRESSION_TYPE
 */
NUI_SDK_API
NUI_J2K2_PROGRESSION_TYPE
NUI_J2K2_DECODE_get_progression_type(NUI_J2K2_DECODE *j2k2_decoder);

/**
 * \brief Used to provide information to the callback function set by
 *        NUI_J2K2_DECODE_set_decompress_callback().
 *
 * When programmer sets a callback function using 
 * NUI_J2K2_DECODE_set_decompress_callback(), Each time the 
 * NUI_J2K2_DECODE_decompress_code_stream() function is called, 
 * the last thing it does is update an instance of NUI_J2K2_DECODE_INFO, 
 * pass it to and call this callback. Within the callback, the application 
 * programmer can then use the data to update the user interface.
 *
 * \sa NUI_J2K2_DECODE_set_decompress_callback()
 */
typedef struct decode_info {
    NUI_J2K2_DECODE *j2k2_decoder;
        /*!<  NUI_J2K2_DECODE object pointer for callback */
    NUI_UINT32       current_level; 
        /*!<  Current progression level (ranges from 1 to total_levels) */
    NUI_UINT32       total_levels; 
        /*!<  Total Progression Levels */
    NUI_UINT32       num_channels; 
        /*!<  Number of channels of raw image */
    NUI_UINT32       bits_per_channel; 
        /*!<  Bits per channel of raw image */
    NUI_UINT32       current_image_width; 
        /*!<  Width of decoded image at current level */
    NUI_UINT32       current_image_height; 
        /*!<  Height of decoded image at current level */
    NUI_UINT32       final_image_width; 
        /*!<  Width of final (best quality) image */
    NUI_UINT32       final_image_height; 
        /*!<  Height of final (best quality) image */
    NUI_UCHAR       *current_raw_image_buffer;
        /*!< Image buffer containing decoded image at current level */
    size_t           current_raw_image_buffer_size;
        /*!< Size of image buffer at current level */
	NUI_DOUBLE       percent_remaining; 
        /*!< Percentage tasks remaining */
	NUI_DOUBLE       total_decompression_time; 
        /*!< Decompression time in msecs (accumulative) */
	NUI_DOUBLE       decompression_time; 
        /*!< Decompression time in msecs (in last step) */
	size_t           total_code_stream_bytes_used; 
        /*!< Size of code stream used (accumulative) */
	size_t           code_stream_bytes_used; 
        /*!< Size of code stream used (in last step) */
	NUI_DOUBLE       compression_ratio; 
        /*!< Effective compression ratio */
	NUI_CHAR         comment[128]; 
        /*!< Comment for callback */
	void            *user_data;     
    /*!< \c callback_data from NUI_J2K2_DECODE_set_decompress_callback() */
} NUI_J2K2_DECODE_INFO;

/**
 * \brief Sets a decompression callback that is executed 
 *        NUI_J2K2_DECODE_INFO::total_levels times inside
 *        NUI_J2K2_DECODE_decompress_code_stream() whenver the function
 *        has an image (at any progression level) available.
 *  
 *  The callback returns NUI_J2K2_PROGRESSION_STATE so following cases
 *  arise : 
 *  1. When callback returns NUI_J2K2_PROGRESSION_SUCCESS the API continues
 *  with the progression normally.
 *  2. When callback returns NUI_J2K2_PROGRESSION_FAILURE the API sets the 
 *  error string and stops futher decoding.
 *  3. When callback returns NUI_J2K2_PROGRESSION_INTERRUPT the API prempts
 *  the decoding process. Raw output image buffer points to the image 
 *  decoded at the last quality level.
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param callback (in) - non-NULL callback declared as 
 *                        NUI_J2K2_PROGRESSION_STATE 
 *                          callback(NUI_J2K2_DECODE_INFO *info);
 * \param callback_data (in) - User data that will be passed to callback in
 *                             \c NUI_J2K2_DECODE_INFO::user_data.
 *
 * \note If you wish to have a callback called during the decompression
 *       process, call this function prior to 
 *       NUI_J2K2_DECODE_decompress_code_stream(). 
 * \note If NUI_J2K2_PROGRESSION_NONE is used then the callback is called
 *       only once by NUI_J2K2_DECODE_decompress_code_stream() after 
 *       decoding final best quality image. 
 * 
 * Declare callback function as:
 * \code
 * NUI_J2K2_PROGRESSION_STATE
 * programmer_get_progressive_images_callback(NUI_J2K2_DECODE_INFO *info);
 * \endcode
 *
 * Example usage of function:
 * \code 
 * ...
 * NUI_J2K2_DECODE_set_decompress_code_stream_callback(
 *       j2k2_decoder, programmer_get_progressive_images_callback, 
 *       (void *) some_progress_bar);
 * ...
 * \endcode
 *
 * Example implementation of callback:
 * \code
 * NUI_J2K2_PROGRESSION_STATE
 * programmer_get_progressive_images_callback(NUI_J2K2_DECODE_INFO *info)
 * {
 *     MAIN_CODEC *main_codec = (MAIN_CODEC *) info->user_data;
 *     NUI_INT32        result = NUI_ERROR;
 *     NUI_CHAR   output_filename[128];
 * 
 *     NUI_printf("Decoded Image %d of %d", 
 *         info->current_level, info->total_levels);
 *     NUI_printf("Current Size : %d x %d", 
 *         info->current_image_width, info->current_image_height);
 *     NUI_printf("Final Size : %d x %d", 
 *         info->final_image_width, info->final_image_height);
 *     NUI_printf("Decompression time : %.02f msec., "
 *                "Elapsed time %.02f secs \n", 
 *                info->decompression_time, info->seconds_elapsed);
 * 
 *     // Save Image
 *     sprintf(output_filename, "%d_%s", 
 *         info->current_level, main_codec->out_filename);
 *     NUI_printf("Writing %s\n", output_filename);
 * 
 *     result = GDAL_DEMO_save_image(output_filename,
 *         info->current_raw_image_buffer, 
 *         info->current_image_width, 
 *         info->current_image_height, 
 *         info->num_channels, info->bits_per_channel);
 * 
 *     // Display Image
 *     // Display code could be added here
 *
 *     return NUI_J2K2_PROGRESSION_SUCCESS;
 * }
 * \endcode
 *
 * \sa NUI_J2K2_DECODE_decompress_code_stream() 
 * \sa NUI_J2K2_DECODE_set_progression_type()
 * \sa NUI_J2K2_DECODE_INFO
 */
NUI_SDK_API
void
NUI_J2K2_DECODE_set_decompress_code_stream_callback(
                                NUI_J2K2_DECODE *j2k2_decoder,
                                NUI_J2K2_PROGRESSION_STATE 
                                    (*callback)(NUI_J2K2_DECODE_INFO *info), 
                                void  *callback_data);

/**
 * \brief Returns raw image generated by the decompression engine
 *
 * This function returns a the raw (output) image buffer as well
 * as output image parameters. If a non-NULL callback is set by the user
 * then image is decoded according to the progression type 
 * NUI_J2K2_PROGRESSION_TYPE. Default progression type is 
 * NUI_J2K2_PROGRESSION_NONE in which case callback is called once with 
 * best quality decoded image. For other progression types the callback 
 * is called every time a progression level is decoded from lowest 
 * quality image to best quality image. 
 *
 * If callback is set to NULL (default) then the progression type is 
 * ignored and the function returns with final best quality image and
 * output image parameters.
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param width (out) - raw (output) image width in pixels
 * \param height (out) - raw (output) image height in pixels
 * \param channels (out) - number of channels in the raw (output) image
 * \param bits_per_channel (out) - number of bits in each raw (output) image
 *                                 channel
 * \param raw_image (in/out) - pointer to user allocated raw image buffer
 *
 * \returns 0 on success and -1 on failure with NUI_J2K2_errno set to
 * NUI_J2K2_ERR_DECODE_FAIL.
 *
 * \sa NUI_J2K2_DECODE_set_progression_type()
 * \sa NUI_J2K2_DECODE_set_decompress_callback()
 * \sa NUI_J2K2_DECODE_INFO
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_DECODE_decompress_code_stream(NUI_J2K2_DECODE *j2k2_decoder,
                                       NUI_INT32       *width,
                                       NUI_INT32       *height,
                                       NUI_INT32       *channels,
                                       NUI_INT32       *bits_per_channel,
                                       NUI_UCHAR       *raw_image);

/**
 * \brief Get decoder timing intervals
 *
 * \note Data from this function is ONLY available when compiled 
 *       with _NUI_J2K2_DEBUG_ENABLED_
 *
 * This function retrieves timing intervals (measured in seconds) from the
 * decode function.
 *
 * \param j2k2_decoder (in) - Pointer to a valid, non-NULL
 *                            NUI_J2K2_DECODE object
 * \param time_yuv_conversion (out) - returns time consumed in YUV->RGB conv.
 * \param time_api_compress (out) - returns time for entire decode process
 *
 */
NUI_SDK_API
NUI_INT32
NUI_J2K2_DECODE_get_timing_information(NUI_J2K2_DECODE *j2k2_decoder,
                                       NUI_INT64       *time_yuv_conversion,
                                       NUI_INT64       *time_api_compress);

/* Error handling */

#define NUI_J2K2_ERR_MEMORY                     1
            /*!< Out of memory                       */
#define NUI_J2K2_ERR_RATIO                      2
            /*!< Invalid compression ratio was set   */
#define NUI_J2K2_NO_SDK_SUPPORT                 3
            /*!< SDK is not supported / enabled      */
#define NUI_J2K2_ERR_SUPPORT                    4
            /*!< Input image format is not supported */
#define NUI_J2K2_ERR_PARAMS                     5
            /*!< Input image parameters invalid      */
#define NUI_J2K2_ERR_INCOMPLETE                 6
            /*!< Input information was not complete  */
#define NUI_J2K2_ERR_SOURCE                     7
            /*!< Encoder missing source image        */
#define NUI_J2K2_ERR_DECODE                     8
            /*!< Decoder missing compressed image    */
#define NUI_J2K2_ERR_SYSTEM                     9
            /*!< strerror(errno); OS error           */
#define NUI_J2K2_ERR_FORMAT                     10
            /*!< Invalid SDK/encoding format combination    */
#define NUI_J2K2_ERR_KAKADU_ENCODE_FAIL         11
            /*! Failure during encoding using Kakadu        */
#define NUI_J2K2_ERR_KAKADU_DECODE_FAIL         12
            /*! Failure during decoding using Kakadu        */
#define NUI_J2K2_ERR_KAKADU_QUERY_FAIL          13
            /*! Failure during query using Kakadu           */
#define NUI_J2K2_ERR_ADV202_DECODE_FAIL         14
            /*! Failure during decoding using ADV202        */
#define NUI_J2K2_ERR_INVALID_INPUT_CODE_STREAM  15
            /*! Input Codestream for decompression invalid  */

extern NUI_INT32 NUI_J2K2_errno; /*!< NUI_J2K2 library error code */

/**
 * \brief Returns last NUI_J2K2 error message
 *
 * This function returns a character string that contains additional
 * information about the last error condition.
 *
 * \note The function returns "No error" if no error has occurred
 *
 * \returns an ASCII string with error information
 */
NUI_SDK_API
NUI_CHAR *
NUI_J2K2_error_string(void);

#ifdef __cplusplus
}
#endif

#endif /* _NUI_J2K2_CORE_H_HAS_BEEN_INCLUDED_ */

/*
 * nui_j2k2_core.h ends
 */
