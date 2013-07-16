/*
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 */
/**
 * \file nuif_constants.h
 * \section NUIF_CONSTANTS
 *
 * \brief NUIF API needs constants. This file defines those constants.
 *
 * Many of the constants represent property IDs. These are to be treated
 * as carefully as say GeoTIFF tags. If you wish to add tags, please consult
 * Pixia Corp for reserving property tags for you.
 *
 * What is a \b property? A \b property is a record that holds a single 
 * unit of metadata regarding information in the raster section of
 * the file.
 *
 * What are the actual values of these constants? 
 * \code
 * #include <nuiSDK.h>
 * int main(int argc, char *argv[])
 * {
 *      NUI_printf("The value of \"%s\" is 0x%08x or %u or %f", 
 *         "NUIF_MAX_CHANNEL_NAME_LENGTH", 
 *          NUIF_MAX_CHANNEL_NAME_LENGTH, 
 *          NUIF_MAX_CHANNEL_NAME_LENGTH, 
 *          NUIF_MAX_CHANNEL_NAME_LENGTH);
 * }
 * \endcode
 *
 * \warning Do not add new properties without consulting the Pixia Corp.
 * NUI file control committee. 
 *
 * \note  Constants with a MAFC prefix are from the MAFC library. This is
 *        an internal API. 
 *
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * 
 * \authors Rahul Thakkar, Saurabh Sood
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuif_constants.h,v 2.1 2007/06/07 16:47:38 thakkarr Exp $
 * $Log: nuif_constants.h,v $
 * Revision 2.1  2007/06/07 16:47:38  thakkarr
 * Added property ID to identify NMV
 *
 * Revision 2.0  2006/11/16 21:13:04  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.2  2006/11/16 18:26:10  jensenk
 * Bringing files modified since initial break from the old source tree up-to-date in the new tree.
 *
 * Revision 1.3  2006/10/26 21:14:26  thakkarr
 *  - NUIF_GEO_KEY_LONG removed
 *  - GDAL GIS data types added for property
 *  - Warnings fixed for Linux
 *  - Code for benchmarking extended
 *
 * Revision 1.2  2005/01/19 14:58:23  thakkarr
 * nuif.c/h and nuif_get.c/h completed
 *
 * Revision 1.1  2005/01/17 19:05:46  thakkarr
 * Starting implementation of core code from prototype to main body.
 *
 * Revision 1.1  2005/01/13 16:54:33  soods
 * 	Skeletal Header File Only
 *
 *
 *
 */

#ifndef _NUIF_CONSTANTS_H_HAS_BEEN_INCLUDED_
#define _NUIF_CONSTANTS_H_HAS_BEEN_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
 
#define NUIF_MAX_CHANNEL_NAME_LENGTH         MAFC_MAX_CHANNEL_NAME_LENGTH        
/*!< Each channel may be assigned a name that is this long.
 *   \sa NUIF_CHANNEL_set_name(), NUIF_CHANNEL_get_name()
 */
 
#define NUIF_MAX_PROPERTY_NAME_LENGTH        MAFC_MAX_PROPERTY_NAME_LENGTH       
/*!< Each property may be assigned a name that is this long
 *   \sa NUIF_PROPERTY_get_parameters(), NUIF_PROPERTY_set_parameters()
 */
 
#define NUIF_MIN_LEVEL_WIDTH                 MAFC_MIN_LEVEL_WIDTH                
/*!< Miniumum pixel width that a pyramid level may have */
 
#define NUIF_MIN_LEVEL_HEIGHT                MAFC_MIN_LEVEL_HEIGHT               
/*!< Miniumum pixel height that a pyramid level may have */

/**********************************************************************
 * Property ID list
 * Property IDs 
 * - From 0x00000000 to 0x0000FFFF are reserved for Pixia Corp.
 * - Within that, 0x00000000 to 0x00007FFF - for NUI file properties
 * - 0x00008000 to 0x0000FFFF are for NUI file metadata properties.
 * - From 0x00010000 to 0xFFFFFFFF are assigned by Pixia Corp. to
 *   various organizations for their internal (confidential) properties.
 * - 0xFFFFFFFF is reserved and should never be used.
 **********************************************************************/
 
#define NUIF_PROPERTY_ID_PIXIA_MIN           MAFC_PROPERTY_ID_PIXIA_MIN
/*!< Lowest property ID reserved for NUI file internal use */
 
#define NUIF_PROPERTY_ID_NUI_FILE_MIN        MAFC_PROPERTY_ID_NUI_FILE_MIN
/*!< Minimum property ID of NUI 2.2 file specific properties */
 
#define NUIF_PROPERTY_ID_NUI_FILE_MAX        MAFC_PROPERTY_ID_NUI_FILE_MAX
/*!< Maximum property ID of NUI 2.2 file specific properties */
 
/*****************************************************************
 * The following properties do not require a secondary property id 
 * Comments along side property ID are their internal names used to
 * identify the properties within the MAFC API.
 *****************************************************************/
 
#define NUIF_PROPERTY_ID_FILENAME            MAFC_PROPERTY_ID_FILENAME
/*!< FILE_NAME - holds the name of the NUI 2.2 file */
 
#define NUIF_PROPERTY_ID_COMMENT             MAFC_PROPERTY_ID_COMMENT
/*!< GENERAL_COMMENT - holds a general comment regarding the NUI file  */
 
#define NUIF_PROPERTY_ID_GEOCOMMENT          MAFC_PROPERTY_ID_GEOCOMMENT
/*!< GEO_SPATIAL_COMMENT - holds general comment regarding geo-spatial 
 *   information in the NUI file */
 
#define NUIF_PROPERTY_ID_COPYRIGHT           MAFC_PROPERTY_ID_COPYRIGHT
/*!< COPYRIGHT - holds the copyright of the owner of the image data */
 
#define NUIF_PROPERTY_ID_APPLICATION         MAFC_PROPERTY_ID_APPLICATION
/*!< APPLICATION_CREATING_FILE - holds name of the software that created
 *   the NUI file */
 
#define NUIF_PROPERTY_ID_DEVICE              MAFC_PROPERTY_ID_DEVICE
/*!< DEVICE_CREATING_FILE - holds name of the hardware device that created
 *   the NUI file */
 
#define NUIF_PROPERTY_ID_OS                  MAFC_PROPERTY_ID_OS
/*!< OPERATING_SYSTEM_CREATING_FILE - holds name of the operating system that
 *   the was used to run the software that created the NUI file */
 
#define NUIF_PROPERTY_ID_ACQ_SOURCE          MAFC_PROPERTY_ID_ACQ_SOURCE
/*!< ACQUISION_SOURCE - holds name of the source acquring the image */
 
#define NUIF_PROPERTY_ID_ACQ_TIME            MAFC_PROPERTY_ID_ACQ_TIME
/*!< ACQUISION_TIME - holds the time with the image was acquired */
 
/**********************************************************
 * The following properties require a secondary property id 
 * that would be the channel number in the NUI file. There
 * may be several properties of the same ID, one for each
 * channel.
 **********************************************************/
 
#define NUIF_PROPERTY_ID_LUT                 MAFC_PROPERTY_ID_LUT
/*!< COLOR_LOOKUP_TABLE - specifies a color lookup table for the
 *   image. It may be a matrix with number of rows equalling the
 *   number of color code values in the image data and number of
 *   columns equal the number of bands. The matrix cell is the
 *   converted color code value for that band (column), for that 
 *   source image color code value (row). */
 
#define NUIF_PROPERTY_ID_DISPLAY             MAFC_PROPERTY_ID_DISPLAY
/*!< CONVERSION_MATRIX_FOR_DISPLAY - Similar to the color lookup table.
 *   However, the values in the matrix are code values to be used when
 *   displaying the image on a display device. */
 
/**********************************************************
 * The following properties don't require a secondary 
 * property id. These values override geo-spatial reference
 * information in the NUI file by GDAL compatible geo-spatial
 * reference information.
 * MAFC_PROPERTY_ID_GDAL_OGC_WKT - character string
 * MAFC_PROPERTY_ID_GDAL_GEO_TRANSFORM - six doubles
 *  value[0] - top left x (Projected X of top-left corner)
 *  value[1] - w-e pixel resolution (GSD-X)
 *  value[2] - rotation, 0 if image is "north up" (always 0)
 *  value[3] - top left y  (Projected Y of top-left corner)
 *  value[4] - rotation, 0 if image is "north up" (always 0)
 *  value[5] - n-s pixel resolution (GSD-Y)
 **********************************************************/
#define NUIF_PROPERTY_ID_GDAL_OGC_WKT       MAFC_PROPERTY_ID_GDAL_OGC_WKT 
/*!< GDAL_OGC_WKT - overrides Geo-spatial coordinate system info 
 *   if specified. MAFC_GEO coordinate system info is ignored.
 *   OpenGIS well known text. */

#define NUIF_PROPERTY_ID_GDAL_GEO_TRANSFORM MAFC_PROPERTY_ID_GDAL_GEO_TRANSFORM
/*!< GDAL_GEO_TRANSFORM - overrides top-left corner projection
 *   information and GSD. Data is array of 6 doubles.
 *   value[0] - top left x (Projected X of top-left corner)
 *   value[1] - w-e pixel resolution (GSD-X)
 *   value[2] - rotation, 0 if image is "north up" (always 0)
 *   value[3] - top left y  (Projected Y of top-left corner)
 *   value[4] - rotation, 0 if image is "north up" (always 0)
 *   value[5] - n-s pixel resolution (GSD-Y) */

/***********************************************************
 * The ID below identifies if the NUI file is an interim
 * NMV file (NUI Movie file). The NUI file will have several
 * properties with this primary ID. The secondary ID of these
 * properties will provide information necessary to read 
 * data from the NMV file. This secondary information is 
 * published to limited customers only and is controlled
 * by Pixia Corp. This requires secondary IDs. Existance
 * of this property does not guarantee that this file is
 * a valid NMV file. The NMV API is required to read this
 * type of file. The NMV_HDR API is required to read this
 * header.
 ***********************************************************/
#define NUIF_PROPERTY_ID_NMV_1               MAFC_PROPERTY_ID_NMV_1
/*!< NMV_FILE: If the NUI is a NUI MOVIE or NMV file, it must have NMV
     related parameters in several properties whose primary ID is this. The
     secondary ID of these properties identifies members of the NMV header.
     See nmv_hdr.h. This file is visible to members approved by Pixia Corp */


#define NUIF_PROPERTY_ID_METADATA_MIN        MAFC_PROPERTY_ID_METADATA_MIN
/*!< Lowest property ID reserved for describing NUI file metadata */
 
#define NUIF_PROPERTY_ID_METADATA_MAX        MAFC_PROPERTY_ID_METADATA_MAX
/*!< Highest property ID reserved for describing NUI file metadata */
 
#define NUIF_PROPERTY_ID_NITF_METADATA       MAFC_PROPERTY_ID_NITF_METADATA
/*!< Describes metadata from NITF file, secondary IDs are NITF specific */
 
#define NUIF_PROPERTY_ID_TIFF_METADATA       MAFC_PROPERTY_ID_TIFF_METADATA
/*!< Describes metadata from TIFF file, secondary IDs are TIFF specific */
 
#define NUIF_PROPERTY_ID_IMAGINE_METADATA    MAFC_PROPERTY_ID_IMAGINE_METADATA
/*!< Describes metadata from ERDAS IMAGINE file, secondary IDs are 
 ERDAS IMAGINE specific */
 
#define NUIF_PROPERTY_ID_MRSID_METADATA      MAFC_PROPERTY_ID_MRSID_METADATA
/*!< Describes metadata from MrSID file, secondary IDs are 
 MrSID specific */
 
/**********************************************************
 * Add new METADATA property IDs before this comment. New
 * property IDs are added based on the sole discretion
 * of Pixia Corp. 
 **********************************************************/
 
#define NUIF_PROPERTY_ID_PIXIA_MAX           MAFC_PROPERTY_ID_PIXIA_MAX
/*!< Highest property ID reserved for NUI file internal use */
 
/**********************************************************
 * Following property IDs are reserved for exclusive use
 * by the US Government. All other governments are to be
 * treated as commercial enterprises.
 *
 * Any classified US government information will be a
 * secondary ID. All primary IDs are unclassified and
 * published so that no one else uses them.
 **********************************************************/
 
#define NUIF_PROPERTY_ID_US_GOVERNMENT_MIN   MAFC_PROPERTY_ID_US_GOVERNMENT_MIN
/*!< US Govt minimum property ID */
 
#define NUIF_PROPERTY_ID_US_GOVERNMENT_MAX   MAFC_PROPERTY_ID_US_GOVERNMENT_MAX
/*!< US Govt maximum property ID */

/**********************************************************
 * US Government property IDs are defined above.
 **********************************************************/
 
#define NUIF_PROPERTY_ID_RESERVED            MAFC_PROPERTY_ID_RESERVED
/*!< This is the end-of-property-list marker - 0xFFFFFFFF is reserved */
 
    
#ifdef __cplusplus
}
#endif

#endif /* _NUIF_CONSTANTS_H_HAS_BEEN_INCLUDED_ */

/*
 * nuif_constants.h ends
 */
