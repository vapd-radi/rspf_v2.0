/*
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 */
/**
 * \file nuif.h
 * \section NUIF
 * \brief API provides functions to interface with a nui file. This API will
 * also have functions to create/destroy anui object, open (read or
 * read+write)/create/close a nui file. 
 * 
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * 
 * \authors Rahul Thakkar, Saurabh Sood
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nuif.h,v 2.0 2006/11/16 21:13:04 jensenk Exp $
 * $Log: nuif.h,v $
 * Revision 2.0  2006/11/16 21:13:04  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:56:17  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.5  2005/02/08 15:14:34  thakkarr
 * Added NUIF_get_swapped_header_state()
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
 * Revision 1.2  2005/01/17 19:48:56  thakkarr
 * - New files added to CVS for Windows setup
 * - Tested the fact that functions do indeed get exported with NUI_SDK_API
 *
 * Revision 1.1  2005/01/13 16:54:33  soods
 * 	Skeletal Header File Only
 *
 *
 *
 */

#ifndef _NUIF_H_HAS_BEEN_INCLUDED_
#define _NUIF_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h>  /*!< Pixia Corp global constants */
#include "nuif_constants.h" /*!< Constants for NUIF */
#include "nuif_types.h"     /*!< Data types for NUIF */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief Creates an abstract, empty NUIF object
 * \returns A valid, empty, abstract NUIF object upon success or NULL on 
 *          failure, setting the error code appropriately.
 */
NUI_SDK_API 
NUIF *NUIF_create(void);

/**
 * \brief Destroys a valid NUIF object and frees all resources used by it.
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 */
NUI_SDK_API 
void  NUIF_destroy(NUIF *nuif);

/**
 * \brief Creates or opens a NUI file for reading or writing.
 *
 * If the file exists and it is opened, the function loads both the
 * static and variable sections of the header in memory.
 * 
 * \param nui_filename (in) Pointer to non-NULL name of a NUI file.
 * \param mode (in) Manner in which the file in \b nui_filename is opened.
 *   Possible values of \b mode are:
 *   - NUIF_MODE_READONLY implies that the NUI file in \b nui_filename  
 *     exists and should be opened for reading only. It may not be modified.
 *   - NUIF_MODE_CREATE implies that a new NUI file named as in stated in
 *     \b nui_filename should be created if it does not exist. If a file by
 *     that name exists, it is destroyed and a new file created. It may
 *     be modified.
 *   - NUIF_MODE_READWRITE implies that the NUI file in \b nui_filename  
 *     exists and should be opened for both reading and writing image data.
 *
 * \warning If the file of specified name exists and you are opening a NUI
 *     file in NUIF_MODE_CREATE, the file is destroyed and a new, empty file
 *     is created in its place. For example, it may be necessary for the 
 *     nuiSDK user to confirm the presence of the file and interactively ask 
 *     the application user if they want to overwrite the file.
 *
 * \returns An abstract NUIF object representing the existing or new NUI file.
 *     If the file was just created, different parameters such as those
 *     available in NUIF_get_image_parameters() are not yet set and need to
 *     be set using NUIF_set_image_parameters() before being able to use the
 *     file in any way.
 */
NUI_SDK_API
NUIF     *NUIF_open_file(char *nui_filename, NUIF_MODE mode);

/**
 * \brief Closes an open NUI file.
 *
 * - If the file was opened as NUIF_MODE_READONLY, it is closed.
 * - It the file was opened as NUIF_MODE_CREATE or NUIF_MODE_READWRITE, 
 *   - All unwritten buffers are flushed and updated in the file, 
 *   - The variable and fixed header are saved to the file, 
 *   - The file is closed.
 *
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \returns 0 on success and -1 on failure, setting NUIF_errno.
 */
NUI_SDK_API
NUI_INT32 NUIF_close_file(NUIF *nuif); 

/**
 * \brief Creates a duplicate copy of a NUIF object.
 * \param nuif (in) Pointer to a valid, non-NULL NUIF object.
 * \param filename (in) Pointer to name of a new NUI 2.2 file,
 *        other than name of file referred by \b nuif. \b filename
 *        Cannot be NULL.
 * \returns a pointer to an exact duplicate copy of the NUIF object.
 *        If \b filename was specified, a corresponding file with
 *        those specifications is also created.
 * \warning If the file size is very large, the function does
 *        create an empty file with NO image data in it. The API
 *        user needs to explicity state that there is image data
 *        in the file and then re-save the header and image data.
 */
NUI_SDK_API
NUIF     *NUIF_duplicate_file(NUIF *nuif, char *filename);

/**
 * \brief Returns NUI_BOOLEAN_TRUE if the file was created on a 
 *        platform that has "endianness" different from current platform.
 *
 *        If the file was created on a platform such as IRIX or Macintosh
 *        and it needs to be viewed on Windows or Linux (Intel), then
 *        the header is automatically byte-swapped for your convenience.
 *        However, the user must know whether byte-swapping was indeed
 *        performed on this file when opening it. This is because it
 *        reflects that the NUI file is a non-native platform file.
 * \param mafc (in) Pointer to a valid, non-NULL NUIF object that points
 *        to an open NUI 2.2 file.
 * \returns NUI_BOOLEAN_TRUE if the file was created on a platform
 *        that has "endianness" different from current platform. Otherwise
 *        it returns NUI_BOOLEAN_FALSE.
 */
NUI_SDK_API
NUI_BOOLEAN NUIF_get_swapped_header_state(NUIF *nuif);


/**
 * \brief Returns a short error string representing the last error.
 * \returns Returns a short error string representing the value assigned 
 *          to the internal NUIF error code when the last error occurred.
 */
NUI_SDK_API 
NUI_CHAR *NUIF_error_string(void);

/**
 * \brief Returns a detailed error string representing the last error. 
 * \note  Use it when debugging your application only.
 * \returns Returns a detailed string describing the last error in the .
 *        NUIF API. May include data on state of NUIF, with C source file
 *        name and approximate line number in the file where error occurred.
 */
NUI_SDK_API 
NUI_CHAR *NUIF_error_string_detailed(void);

/**
 * \brief Returns formatted file information in a character string
 * \param nuif (in) Pointer to a valid NUIF object (non-NULL)
 * \param return_string (out) Address of pointer to a character string
 *        Formatted data about the file is returned in this parameter.
 *        To free memory allocated for this returned value, use 
 *        NUIF_free_formatted_file_information_return_string() (not free()).
 * \param as_html (in) If NUI_TRUE, returned character string is
 *        formatted in HTML for display in a web-browser or saving as an
 *        HTML file. If NUI_FALSE, information is formatted for display
 *        in a standard terminal.
 * \returns 0 on success, -1 on failure and sets error code.
 * \sa NUIF_free_formatted_file_information_return_string()
 * \code
 * NUI_CHAR *info_str = NULL;
 * if (NUIF_get_formatted_file_information(nuif, &info_str, NUI_TRUE) < 0) {
 *      NUI_printf("error: Cannot get formatted file information (%s)",
 *          NUIF_error_string());
 *      return -1; // some application specific error code or exit
 * }
 * // display info_str in, say a web-browser
 * NUIF_free_formatted_file_information_return_string(info_str);
 * info_str = NULL;
 * ...
 * \endcode
 */
NUI_SDK_API
NUI_INT32 NUIF_get_formatted_file_information(NUIF      *nuif, 
                                              NUI_CHAR **return_string, 
                                              NUI_INT32  as_html);
/**
 * \brief Frees the memory allocated by \b return_string parameter in the
 *        function NUIF_get_formatted_file_information().
 * \param string (in) Pointer to a non-NULL character string that was
 *        returned from a prior call to NUIF_get_formatted_file_information()
 */
NUI_SDK_API
void NUIF_free_formatted_file_information_return_string(NUI_CHAR *string);

#ifdef __cplusplus
}
#endif

#endif /* _NUIF_H_HAS_BEEN_INCLUDED_ */

/*
 * nuif.h ends
 */
