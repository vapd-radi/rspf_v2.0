/*
 * Copyright (C) 2000 Pixia Corp, All Rights Reserved.
 */
/**
 * \file  nui_yuv.h
 * \brief Color Space conversion between RGB and YUV.
 *        For the scope of this API, YUV is treated exactly same as
 *        YCrCb as per the ITU 601 Standard.
 *        
 *        Supported Formats YUV444, YUV422 and YUV411
 * 
 * \note  Even though this library may seem to be agnostic to the 
 *        image width and image height the following is considered 
 *        implicit: \n
 *        \t YUV444 format: No assumption on image width and height\n
 *        \t YUV422 format: Image width should be even\n
 *        \t YUV411 format: Both image width should be multiple of 4 and  
 *                          height assumed even\n
 *           
 * \authors Rahul Thakkar, Alan Black, Saurabh Sood
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui_yuv/nui_yuv.h,v 2.0 2006/11/16 21:13:05 jensenk Exp $
 * $Log: nui_yuv.h,v $
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.2  2005/03/08 19:02:24  soods
 * Corrected a roundoff bug and implemented clipping for YUV
 *
 * Revision 1.1  2005/03/07 22:17:43  soods
 * Added to CVS
 *
 *
 */

#ifndef _NUI_YUV_H_HAS_BEEN_INCLUDED_
#define _NUI_YUV_H_HAS_BEEN_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nui_constants.h>

/* Do clipping operation before the YUV to RGB conversion */ 
#define _NUI_YUV_DO_CLIPPING_
// #undef _NUI_YUV_DO_CLIPPING_

/**
 * \brief Used to specify YUV format
 *        
 */
typedef enum {
    NUI_YUV444_PLANAR = 1,
    NUI_YUV422_PLANAR = 2,
    NUI_YUV411_PLANAR = 3,
} NUI_YUV_PLANAR;

typedef enum {
	NUI_YUV444_PACKED = 11,	      /*!< Encodes YUV444 as YUV YUV YUV.. */

	NUI_YUV422_PACKED_UYVY = 12,  /*!< Encodes YUV422 as U0Y0V0Y1 U2Y2V2Y3.. */
    NUI_YUV422_PACKED_YUYV = 13,  /*!< Encodes YUV422 as Y0U0Y1V0 Y2U2Y3V2.. */

    NUI_YUV411_PACKED_UYVY = 14,  /*!< Encodes YUV411 as U0Y0V0Y1 U4Y2V4Y3 Y4Y5Y6Y7.. */
    NUI_YUV411_PACKED_YUYV = 15   /*!< Encodes YUV411 as Y0U0Y1V0 Y2U4Y3V4 Y4Y5Y6Y7.. */
} NUI_YUV_PACKED;


/**
 * \brief This function is needed to be called to initialize internal arrays
 *		  for fast RGB to YUV conversion and vice versa. This should be called
 *		  only "once" and then multiple RGB to YUV and YUV to RGB conversions 
 *		  could be performed.
 */
void NUI_YUV_initialize(void);

/**
 * \brief Converts from RGB, 8-bits per band, 24-bits per pixel packed to
 *        YUV packed. 
 *
 * \param rgb (in) Non-null pointer to preallocated source RGB image buffer.
 *        The image buffer has total pixels = image_width * image_height.
 *        Size of this buffer is (pixels * 3) bytes, 1 byte each for R,
 *        G and B. Data is stored packed, as RGB RGB RGB...
 *        Pointer may not be NULL.
 * \param yuv (in/out) Non-null pointer to preallocated destination YUV image 
 *        buffer. The image buffer has total pixels= image_width * image_height
 *        Size of this buffer is (pixels * 3) bytes for YUV444, 
 *        (pixels * 2) bytes for YUV422 and (pixels * 3 / 2) bytes for YUV411. 
 *        Data storage format depends on parameter \b encoding. This is where   
 *        the converted data is placed. Pointer may not be NULL.
 * \param image_width (in) Specifies image width in pixels.
 * \param image_height (in) Specifies image heigth in pixels.
 *        The size of the source RGB buffer will be image_width * image_height 
 *        * 3 bytes.
 * \param encoding (in) Can be one of \b NUI_YUV_PACKED . 
 *
 * \returns NUI_SUCCESS if successful or NUI_ERROR if failed
 */
NUI_INT32 NUI_YUV_packed_from_rgb_packed(NUI_UCHAR      *rgb, 
					                     NUI_UCHAR      *yuv,
					                     NUI_UINT32      image_width,
					                     NUI_UINT32      image_height,
					                     NUI_YUV_PACKED  encoding);

/**
 * \brief Converts from YUV packed to RGB, 8-bits per band, 
 *        24-bits per pixel packed.
 *        .
 * \param yuv (in) Non-null pointer to preallocated destination YUV image  
 *        buffer. The image buffer has total pixels= image_width * image_height
 *        Size of this buffer is (pixels * 3) bytes for YUV444, 
 *        (pixels * 2) bytes for YUV422 and (pixels * 3 / 2) bytes  
 *		  for YUV411. Data storage format depends on parameter \b encoding.  
 *        This is where the converted data is placed.  
 *        Pointer may not be NULL.
 * \param rgb (in/out) Non-null pointer to preallocated destination RGB image.
 *        The image buffer has total pixels = image_width * image_height.
 *        Size of this buffer is (pixels * 3) bytes, 1 byte each for R,
 *        G and B. Data is stored packed, as RGBRGBRGB... This is where the 
 *        converted data is placed.
 *        Pointer may not be NULL.
 * \param image_width (in) Specifies image width in pixels.
 * \param image_height (in) Specifies image heigth in pixels.
 *        The size of the source RGB buffer will be image_width * image_height 
 *        * 3 bytes.
 * \param encoding (in) Can be one of \b NUI_YUV_PACKED. 
 *
 * \returns NUI_SUCCESS if successful or NUI_ERROR if failed
 */
NUI_INT32 NUI_YUV_packed_to_rgb_packed(NUI_UCHAR      *yuv,
                                       NUI_UCHAR      *rgb,
 				                       NUI_UINT32      image_width,
					                   NUI_UINT32      image_height,
                                       NUI_YUV_PACKED  encoding);

/**
 * \brief Converts from RGB, 8-bits per channel, 24-bits per pixel packed
 *        into YUV planar such that the destination is saved into 3
 *        separate image buffers, one each for Y, U and V.
 *
 * \param rgb (in) Non-null pointer to preallocated source RGB image buffer.
 *        The image buffer has total pixels = image_width * image_height.
 *        Size of this buffer is pixels * 3 bytes, 1 byte each for R,
 *        G and B. Data is expected to be packed, as RGBRGBRGB...
 *        Pointer may not be NULL.
 * \param y (in/out) Non-null pointer to preallocated destination Y-component
 *        of YUV output. The image buffer has total pixels = image_width * 
 *        image_height. Size of this buffer is (pixels * 1) bytes, 
 *        1 byte each for the Y-component. Data is stored as Y0Y1Y2Y3...
 *        Pointer may not be NULL.
 * \param u (in/out) Non-null pointer to preallocated destination U-component of
 *        YUV output. The image buffer has total pixels = image_width * 
 *        image_height. Size of this buffer is (pixels * 1) bytes for YUV444,
 *        (pixels / 2) bytes for YUV422 or (pixels / 4) in case of YUV411 
 *          1 byte each for every other U-component. Data is stored as 
 *        U0U1U2.. for YUV444, U0U2U4U6U8.. for YUV422 and U0U4U8... for YUV411 
 *        Pointer may not be NULL.
 * \param v (in/out) Non-null pointer to preallocated destination V-component of
 *        YUV 4:2:2 output. The image buffer has total pixels = image_width * 
 *        image_height. Size of this buffer is either (pixels / 2) 
 *        bytes in case of YUV422 or (pixels / 4) in case of YUV411 
 *        1 byte each for every other V-component. Data is stored as
 *        V0V1V2.. for YUV444, V0V2V4V6V8... for YUV422 and V0V4V8... for YUV411
 *        Pointer may not be NULL.
 * \param image_width (in) Specifies image width in pixels.
 * \param image_height (in) Specifies image heigth in pixels.
 *        The size of the source RGB buffer will be image_width * image_height * 3
 *        bytes.
 * \param encoding (in) Can be one of \b NUI_YUV_PLANAR 
 *
 * \returns NUI_SUCCESS if successful or NUI_ERROR if failed
 */
NUI_INT32 NUI_YUV_planar_from_rgb_packed(NUI_UCHAR      *rgb, 
 			      		                 NUI_UCHAR	    *y,
 					                     NUI_UCHAR      *u,
					                     NUI_UCHAR	    *v,
					                     NUI_UINT32      image_width,
					                     NUI_UINT32      image_height,
                                         NUI_YUV_PLANAR  encoding);

/**
 * \brief Converts to RGB, 8-bits per channel, 24-bits per pixel packed
 *        from YUV planar such that the source is available in 3
 *        separate image buffers, one each for Y, U and V.
 *
 * \param y (in) Non-null pointer to preallocated destination Y-component of
 *        YUV output. The image buffer has total pixels = image_width 
 *        * image_height. Size of this buffer is (pixels * 1) bytes, 
 *        1 byte each for the Y-component.  Data is expected as Y0Y1Y2Y3...
 *        Pointer may not be NULL.
 * \param u (in) Non-null pointer to preallocated destination U-component of
 *        YUV output. The image buffer has total pixels = image_width * 
 *        image_height. Size of this buffer is either (pixels / 2) 
 *        bytes in case of YUV422 or (pixels / 4) in case of YUV411 
 *        1 byte each for every other U-component. Data is expected as
 *        U0U1U2.. for YUV444, U0U2U4U6U8 for YUV422 and U0U4U8.. for YUV411 
 *        Pointer may not be NULL.
 * \param v (in) Non-null pointer to preallocated destination V-component of
 *        YUV 4:2:2 output. The image buffer has total pixels as specified
 *        in parameter \b pixels. Size of this buffer is either (pixels / 2) 
 *        bytes in case of YUV422 or (pixels / 4) in case of YUV411 
 *        1 byte each for every other V-component. Data is expected as
 *        V0V1V2.. for YUV444, V0V2V4V6V8.. for YUV422 and V0V4V8.. for YUV411
 *        Pointer may not be NULL.
 * \param rgb (in/out) Non-null pointer to preallocated destination RGB image
 *        buffer.The image buffer has total pixels = image_width * image_height
 *        Size of this buffer is pixels * 3 bytes, 1 byte each 
 *        for R, G and B. Data is stored packed, as RGBRGBRGB...
 *        Pointer may not be NULL.
 * \param image_width (in) Specifies image width in pixels.
 * \param image_height (in) Specifies image heigth in pixels.
 *        The size of the source RGB buffer will be image_width * 
 *        image_height * 3 bytes.
 * \param encoding (in) Can be one of \b NUI_YUV_PLANAR. 
 *
 * \returns NUI_SUCCESS if successful or NUI_ERROR if failed
 */
NUI_INT32 NUI_YUV_planar_to_rgb_packed(NUI_UCHAR      *y,
                                       NUI_UCHAR      *u,
                                       NUI_UCHAR      *v,
                                       NUI_UCHAR      *rgb,
				                       NUI_UINT32      image_width,
					                   NUI_UINT32      image_height,
                                       NUI_YUV_PLANAR  encoding);

/**
 * \brief Converts a Y U V value to its corresponding R G B value
 * \param y (in)  Input Y value
 * \param u (in)  Input U value
 * \param v (in)  Input V value
 * \param r (out) Pointer to the output R value (could be NULL)
 * \param g (out) Pointer to the output G value (could be NULL)
 * \param b (out) Pointer to the output B value (could be NULL)
 * 
 * \note Only a convenience function. 
 */
void NUI_YUV_to_rgb(NUI_INT32 y, NUI_INT32 u, NUI_INT32 v, 
                    NUI_INT32 *r, NUI_INT32 *g, NUI_INT32 *b);

/**
 * \brief Converts a Y U V value to its corresponding R G B value
 * \param r (in)  Input R value
 * \param g (in)  Input G value
 * \param b (in)  Input B value
 * \param y (out) Pointer to the output Y value (could be NULL)
 * \param u (out) Pointer to the output U value (could be NULL)
 * \param v (out) Pointer to the output V value (could be NULL)
 * 
 * \note Only a convenience function.
 */
void NUI_YUV_from_rgb(NUI_INT32 r, NUI_INT32 g, NUI_INT32 b, 
                      NUI_INT32 *y, NUI_INT32 *u, NUI_INT32 *v);


/*
 * Error management
 */
extern int NUI_YUV_errno;

#define NUI_YUV_ERR_FORMAT		    1  /* Invalid YUV format */
#define NUI_YUV_ERR_PIXELS_YUV422	2  /* Pixel specified is odd */
#define NUI_YUV_ERR_PIXELS_YUV411	3  /* Pixel specified not multiple of 4 */

/*
 * Based on error code, this function returns corresponding error string.
 * Do not free this error string.
 */
char *NUI_YUV_error_string(void);

#ifdef __cplusplus
}
#endif

#endif /* _NUI_YUV_H_HAS_BEEN_INCLUDED_ */


/*
 * nui_yuv.h ends
 */
