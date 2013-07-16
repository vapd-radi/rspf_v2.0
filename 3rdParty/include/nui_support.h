/**
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator
 *
 * \file nui_support.h
 * \section NUI_SUPPORT
 * \brief API to manipulate filenames, get platform specific data etc.
 * Convenience API
 *
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * \authors Rahul Thakkar, Saurabh Sood
 */
/*
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nuiSDK/nui_support.h,v 2.2 2007/08/18 03:50:37 jensenk Exp $
 * $Log: nui_support.h,v $
 * Revision 2.2  2007/08/18 03:50:37  jensenk
 * Added new basic memory allocation functions (NUI_malloc, NUI_malloc_aligned,
 * NUI_strdup) which detect memory leaks.
 *
 * Revision 2.1  2007/03/02 15:24:30  jensenk
 * Fixed description of NUI_SUPPORT_get_specified_file_extension which said
 * the extension list was 0-based indexing, but actually it is 1-based indexing.
 *
 * Revision 2.0  2006/11/16 21:13:04  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:56:17  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.9  2006/07/13 21:06:18  jensenk
 * Fixed English mnistakes a few function descriptions
 *
 * Revision 1.8  2005/04/04 18:02:30  thakkarr
 *  - Added NUI_SUPPORT_strdup()
 *  - Changed project file in MSVC++ for standalone and mass compile.
 *
 * Revision 1.7  2005/03/24 01:39:02  thakkarr
 * nuif_get_i.c function to get rows and columns from center position
 * was implemented and tested using nuiSDK_private sample program.
 *
 * Revision 1.6  2005/03/15 22:53:02  thakkarr
 * Added a suite of NUI_SUPPORT functions to match ALLOC_WIN library
 * functions and NUI_ library functions that we would like to
 * export to the world to use as well as for Pixia developers to use.
 *
 * Revision 1.5  2005/02/10 15:45:02  thakkarr
 * Updated comments
 *
 * Revision 1.4  2005/01/31 15:01:27  thakkarr
 * Added - printing information to console - ability in MAFC
 * and updated in nuiSDK
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
 * Revision 1.1  2005/01/13 16:54:33  soods
 * 	Skeletal Header File Only
 */

#ifndef _NUI_SUPPORT_H_HAS_BEEN_INCLUDED_
#define _NUI_SUPPORT_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h>  /* Pixia Corp. global constants */
#include <nui_alloc.h>      /* Pixia Corp. memory allocation functions. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Memory allocation function
 *
 * This function uses platform specific memory allocation
 * functions to allocate memory that is memory page aligned.
 * This function also clears the memory to specified value if
 * desired.
 *
 * \deprecated NUI_SUPPORT_malloc() will be deprecated in the near future.
 *        Code should be changed to use NUI_malloc_aligned() and
 *        NUI_free_aligned() to allocate page-aligned buffers for reading
 *        tiles from a NUI file.  For smaller buffers, applications 
 *        should use NUI_malloc() and NUI_free().
 *
 * \param bytes (in) Number of bytes to allocate. This parameter
 *        must be a number > 0.
 * \param value (in) A number between 0 and 255 to which all the
 *        bytes in the memory buffer will be initialized. If a
 *        value < 0 is specified then no initialization is
 *        performed.
 *
 * \returns A void pointer to memory block that is aligned to the
 *        memory page of that system or NULL on error.
 * \warning To free memory allocate using this function ONLY use
 *        NUI_SUPPORT_free().
 * \sa NUI_SUPPORT_free()
 */
#define NUI_SUPPORT_malloc(bytes, value) NUI_malloc_aligned((bytes), (value))

/**
 * \brief This function uses platform specific functions to free
 *        memory that was allocated using NUI_SUPPORT_malloc().
 * \deprecated NUI_SUPPORT_free() will be deprecated in the near future.
 *        Code should be changed to use NUI_malloc_aligned() and
 *        NUI_free_aligned() to create page-aligned buffers for reading
 *        tiles from a NUI file.  For smaller buffers, applications 
 *        should use NUI_malloc() and NUI_free().
 *
 * \param pointer (in) Pointer to memory area to be set free. This
 *        pointer must have been allocated using NUI_SUPPORT_malloc().
 */
#define NUI_SUPPORT_free(pointer) NUI_free_aligned((pointer))

/**
 * \brief Same as strdup(). Uses NUI_SUPPORT_malloc() to allocate memory.
 * \deprecated NUI_SUPPORT_strdup() will be deprecated in the near future.
 *        Code should be changed to use NUI_strdup() and NUI_free() 
 *        (\b NOT NUI_aligned_free()) instead.
 * \param source (in) Pointer to a non-NULL NULL-terminated ASCII character
 *        string that will be duplicated.
 * \returns Pointer to newly allocated character string that is identical
 *        to parameter \b source.
 * \warning If you are using this function, in order to free the returned
 *        value, you \b must use NUI_SUPPORT_free(). Do not use free() or
 *        any other function. Due to strange behaviour of strdup() on
 *        some platforms and unavailability of strdup() on some other
 *        platforms, we have provided this convenience function. Use it
 *        wisely.
 */
 NUI_SDK_API
 NUI_CHAR *NUI_SUPPORT_strdup(const char *source);
/**
 * \brief This function is identical to printf() except one detail.
 *        The function automatically adds a '\\n' at the end of each
 *        call. This eliminates the need to remember putting a '\\n'.
 * \param format,... (in) See documentation on the printf() command on your
 *        platform. \b format is expected to be the same way.
 */
NUI_SDK_API
void NUI_SUPPORT_printf(NUI_CHAR *format, ...);

/**
 * \brief This function is identical to printf(). It does not add
 *        any additional characters.
 * \param format,... (in) See documentation on the printf() command on your
 *        platform. \b format is expected to be the same way.
 */
NUI_SDK_API
void
NUI_SUPPORT_print_line(NUI_CHAR *format, ...);

/**
 * \brief This function is identical to NUI_SUPPORT_printf(). However, if
 *        parameter \b filename is not NULL, then the output is appended to
 *        an ASCII file by that name. If the file does not exist, it is
 *        created.
 * \param filename (in) Specifies name of an output file where the function
 *        will print the information to. The output file, if it exists, is
 *        assumed to be an ASCII file and is appended to. If it does not exist,
 *        a file of that name is created and information printed to it. If
 *        \b format is NULL, this file is cleared if it exists or an empty
 *        file is created if it does not exist. If \b filename is NULL, output
 *        is sent to \b stdout.
 * \param format,... (in) See documentation on the printf() command on your
 *        platform. \b format is expected to be the same way. If \b format
 *        is NULL, and a valid \b filename was specified, a file of that name
 *        is cleared if it exists or created with no contents if it does not
 *        exist.
 * \returns 0 on success and -1 on failure. Error code is set accordingly
 *        and nuiSDK_error_string() may be used to access the error message.
 *        The function returns failure if it failed to create or append
 *        to the file specified by \b filename. In that case, at the very
 *        least, output is redirected to \b stdout.
 */
NUI_SDK_API
NUI_INT32 NUI_SUPPORT_file_printf(NUI_CHAR *filename, NUI_CHAR *format, ...);

/**
 * \brief Sets the name of the application for eventual retrieval
 *
 * This is a support function. If you are writing an application in which
 * you need to save the name of the program explicitly, this function
 * \b sets the name of the program.
 *
 * \param program_name (in) Pointer to a non-NULL character string that
 *        specifies the name of the program.
 * \sa NUI_SUPPORT_get_program_name()
 */
NUI_SDK_API
void      NUI_SUPPORT_set_program_name(NUI_CHAR *program_name);

/**
 * \brief Returns the value set using the function
 *        NUI_SUPPORT_set_program_name()
 *
 * This is a support function. If you are writing an application in which
 * you need to save the name of the program explicitly, this function
 * \b gets the name of the program that was set using
 * NUI_SUPPORT_set_program_name()
 *
 * \returns Pointer to a character string that was set using
 *          NUI_SUPPORT_set_program_name()
 * \sa NUI_SUPPORT_set_program_name()
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_program_name(void);

/**
 * \brief Returns standard Pixia Corp. copyright message
 * \returns Standard Pixia Corp. copyright message
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_copyright_message(void);

/**
 * \brief Returns today's date and time as a character string
 * \returns Today's date and time as a character string
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_date_and_time(void);

/**
 * \brief Removes path and extension and returns the name of file
 *
 * If the name of the file on Windows was X:/Pixia/Doc/some_file.number.html,
 * then this function returns some_file.number.
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 *
 * \param filename (in) Input path and file name with extension
 * \returns File name without path and extension. If no file name, returns "".
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_file_name(NUI_CHAR *filename);

/**
 * \brief Removes path and filename and returns extension (without .)
 *
 * If the name of the file on Windows was X:/Pixia/Doc/some_file.number.html,
 * then this function returns html.
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \note The leading extension delimiter '.' is dropped
 * \param filename (in) Input path and file name with extension
 * \returns Extension without path and file name. If no extension, returns "".
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_file_extension(NUI_CHAR *filename);

/**
 * \brief Creates new file name based on filename and ext.
 *
 * If filename has the desired extension already as the last extension,
 * the function returns filename other wise it attaches ext to filename
 * and returns that new name.
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param filename (in) Non-NULL input name of the file
 * \param ext (in) Non-NULL input file extension
 * \returns String composed as 'filename'.'ext'
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_create_file_name(NUI_CHAR *filename, NUI_CHAR *ext);

/**
 * \brief Creates temporary file name using pseudo random number
 *        generator
 *
 * Using a pseudo-random number generator, this function creates a
 * temporary file name. It will attach specified extension to
 * the file name, if ext has been specified. If it is NULL, only
 * the base name is returned.
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param ext (in) Desired temporary file name extension
 * \returns Temporary file name with optional extension generated
 *          using a pseudo-random number generator.
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_create_temporary_file_name(NUI_CHAR *ext);

/**
 * \brief Returns file extension from specified number.
 *
 * For example, if the input file name is
 * /pixia/data/images/birmingam.25meters.100.nui, this file has 3
 * extensions. Extension 1 would be nui, extension 2 would be 100,
 * extension 3 would be 25meters, extension 4 or more returns NULL,
 * and NUI_errno would be set to NUI_INVALID
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param filename (in) specifies input file name to parse
 * \param extension_number (in) specifies extension number to parse
 *        and return from input file
 * \returns Character string that is the extension of specified index
 *          or NULL if index is out of range.
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_specified_file_extension(NUI_CHAR *filename,
                                           NUI_INT32 extension_number);

/**
 * \brief Removes filename and extension and returns only the path
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param path_and_filename (in) Input full path and file name
 * \returns Path section of path and file name or "" if not available
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_file_path(NUI_CHAR *path_and_filename);

/**
 * \brief  Removes path and returns filename and extension
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param path_and_filename (in) Input full path and file name
 * \returns File name and extension from full path and file name or ""
 * \warning Function not reentrant.
 */

NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_file_name_and_extension(NUI_CHAR *path_and_filename);

/**
 * \brief  Removes last extension; returns path and filename (removes .)
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param path_and_filename (in) Input full path and file name
 * \returns Strips last extension and returns path and file name or ""
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_path_and_file_name(NUI_CHAR *path_and_filename);

/**
 * \brief  Removes all extensions; returns path and filename
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param path_and_filename (in) Input full path and file name
 * \returns Strips all extensions and returns path and file name.
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_strip_extensions(NUI_CHAR *path_and_filename);

/**
 * \brief Returns full path of file name from root, removes file name though
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param path_and_filename (in) Input full path and file name
 * \returns Full file path from root (removes file name and extension).
 * \warning Function not reentrant.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_get_full_path(NUI_CHAR *path_and_filename);

/**
 * \brief Returns sequence number embedded in filename, if any.
 * \warning Do not use -1 to number a file.
 * \param filename (in) Input file name with some sequence number.
 *        Example x:/pixia/foo.100.nui returns 100, x:/pixia/foo.bar.100.nui
 *        returns 100
 * \returns Number embedded in a file name or -1 if none.
 */
NUI_SDK_API
NUI_INT32 NUI_SUPPORT_get_file_sequence_number(NUI_CHAR *filename);

/**
 * \brief Attaches extension to filename
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param filename (in) Non NULL input file name
 * \param ext (in) Non NULL input file name extension to attach to file name
 * \returns String of the form 'filename'.'ext'
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_attach_extension_to_file_name(NUI_CHAR *filename,
                                                    NUI_CHAR *extension);

/**
 * \brief Attaches path to filename
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param filename (in) Non-NULL input file name and extension
 * \param path (in) Non-NULL input path to attach filename to.
 * \returns character string of type 'path'\\'filename'
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_attach_path_to_file_name(NUI_CHAR *filename,
                                               NUI_CHAR *path);


/*************************
 * File search functions
 *************************/
/**
 * \brief File search function
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 *
 * Search order:
 * - Searches for specified filename in ".", if not found,
 * - Searches for filename.extension, if not found
 * - Returns NULL and sets NUI_errno to NUI_ERR_SYSTEM.
 * - Otherwise returns the name of the file that was found.
 * - If extension is NULL, the only check performed is
 *   for existance of filename.
 *
 * \param filename (in) Non-NULL input file name to look for
 * \param extension (in) Optional extension to attach to file
 *        name while searching for it
 * \returns The file name found by this function or NULL.
 * \code
 * char *found_file = NUI_find_file("foo.100", NUI_EXT_NUI);
 * \endcode
 * Searches for a file \c foo.100. If not found, it searches
 * for \c foo.100.nui. If not found, returns NULL else it returns what
 * it found.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_find_file(NUI_CHAR *filename,
                                NUI_CHAR *extension);

/**
 * \brief Searches for specified file in a ; delimited string in specified
 *        environment variable.
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 * \param filename (in) Filename (with extension) to search for.
 * \param env_variable (in) Environment variable that holds a ; delimited
 *        character string which is a list of directories to search in.
 * \returns Full path and file name of the found file or NULL.
 * \code
 * char *name = NUI_search_file_in_environment("foo.txt", "PATH");
 * // searches for foo.txt in the PATH environment variable
 * \endcode
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_search_file_in_environment(NUI_CHAR *filename,
                                                 NUI_CHAR *env_variable);

/**
 * \brief Recursively search for a file in a search path
 * \warning Do not free return value, copy it immediately to another
 *          internal character string.
 *
 * Performs recursive search starting from specified path.
 * If file not found or the user cancels the search using callback
 * set by NUI_set_search_recursive_callback() and the function returns NULL.
 * If the file was indeed found, it returns full path name. If
 * skip_this_path_flag is NUI_TRUE, then the search will skip current
 * path and continue searching in all the directories inside this one.
 * This will help you do the "find next file of the same name" part well.
 *
 * \param filename (in) File name to search for.
 * \param search_path (in) Path to start recursively searching in.
 * \param skip_this_path_flag (in) if NUI_FALSE, the search function
 *        looks for the file within search_path. If NUI_TRUE, the search
 *        function only looks within all sub-directories in search_path.
 * \returns The full path and file name of file that was found or NULL.
 */
NUI_SDK_API
NUI_CHAR *NUI_SUPPORT_search_file_recursively(NUI_CHAR *filename,
                                              NUI_CHAR *search_path,
                                              NUI_INT32 skip_this_path_flag);

/**
 * \brief Data structure passed to callback function set by
 *        NUI_set_search_recursive_callback()
 */
typedef struct nui_support_search_file_data_struct {
	NUI_CHAR *current_file_found;
    /*!< At file in NUI_SUPPORT_search_file_recursively() */
	NUI_CHAR *path_being_searched;
    /*!< Current path being searched    */
	NUI_INT32 file_attribute;
    /*!< Helps for knowing info on file */
	void     *data;
    /*!< user_data in NUI_SUPPORT_set_search_recursive_callback() */
} NUI_SUPPORT_SEARCH_FILE_DATA;

/**
 * \brief Sets callback function that gets called every time
 *        NUI_search_file_recursively() goes through a file.
 *
 * When performing recursive search, this function provides information
 * of the search. If you return -1 in the callback, the recursive search
 * stops and NUI_search_file_recursively returns NULL.
 *
 * \note For each call to NUI_search_file_recursively(), you have to
 * preceed it with a call to NUI_set_search_recursive_callback() if you
 * want to have the callback called. Callback is reset to NULL after
 * NUI_search_file_recursively() returns.
 * \param callback (in) Callback function to called everytime
 *        NUI_search_file_recursively() looks at a file.
 * \param data Passed to callback function
 * \param user_data (in) Pointer to user's data (such as a GUI pointer
 *        for processing progress bar) for use within callback function.
 */
NUI_SDK_API
void  NUI_SUPPORT_set_search_recursive_callback(
		NUI_INT32 (*callback)(NUI_SUPPORT_SEARCH_FILE_DATA *data),
		void *user_data);

/**********************************
 * Computer system query functions
 **********************************/
/**
 * \brief Returns total physical memory in the computer
 * \returns Total physical memory in the computer in bytes.
 */
NUI_SDK_API
NUI_DOUBLE NUI_SUPPORT_get_total_memory(void);

/**
 * \brief Returns total physical memory free in the computer
 * \returns Total physical memory free in the computer
 */
NUI_SDK_API
NUI_DOUBLE NUI_SUPPORT_get_total_free_memory(void);

/**
 * \brief Returns total physical memory used by applications in the computer
 * \returns Total physical memory used by the computer in bytes.
 */
NUI_SDK_API
NUI_DOUBLE NUI_SUPPORT_get_total_used_memory(void);

/**
 * \brief Returns total swap memory in the computer
 * \returns Total swap memory in the computer
 */
NUI_SDK_API
NUI_DOUBLE NUI_SUPPORT_get_total_swap_memory(void);

/**
 * \brief Returns total swap memory free in the computer
 * \returns Total swap memory free in the computer in bytes.
 */
NUI_SDK_API
NUI_DOUBLE NUI_SUPPORT_get_total_swap_free_memory(void);

/**
 * \brief Returns total swap memory used by applications in the computer
 * \returns Total swap memory used by applications in the computer in bytes.
 */
NUI_SDK_API
NUI_DOUBLE NUI_SUPPORT_get_total_swap_used_memory(void);

/**
 * \brief Returns total disk space on volume in bytes.
 *
 * Returns total disk space on the volume that contains
 * specified path and file name or current path, if path
 * and file name were not specified.
 *
 * \param full_path_name (in) If not NULL, the system returns total
 *        disk space on the volume that contains specified path and
 *        file. If NULL, the system assumes that the user wants total
 *        disk space of the volume that contains the current path.
 *
 * \returns Total disk space on current volume in bytes or -1 on
 *        failure.
 */
NUI_SDK_API
NUI_INT64 NUI_SUPPORT_get_total_disk_space(NUI_CHAR *full_path_name);

/**
 * \brief Returns total disk space free on volume in bytes.
 *
 * Returns total disk space free on the volume that contains
 * specified path and file name or current path, if path
 * and file name were not specified.
 *
 * \param full_path_name (in) If not NULL, the system returns total
 *        disk space free on the volume that contains specified path and
 *        file. If NULL, the system assumes that the user wants total
 *        disk space of the volume that contains the current path.
 *
 * \returns Total disk space free on volume in bytes or -1 on
 *        failure.
 */
NUI_SDK_API
NUI_INT64 NUI_SUPPORT_get_total_free_disk_space(NUI_CHAR *full_path_name);

/***********************
 * System information
 ***********************/
#define NUI_SUPPORT_SYSTEM_STRLEN    1024 /*!< for NUI_SUPPORT_PROCESSOR */
#define NUI_SUPPORT_SYSTEM_NAMELEN   128  /*!< for NUI_PROCESSOR */

/**
 * \brief Used by NUI_SUPPORT_get_system_info() to query system information
 */
typedef struct nui_system_struct {
	NUI_INT32 page_size;
    /*!< Memory page size in bytes */
	NUI_INT32 num_processors;
    /*!< Total CPUs in system */
	NUI_CHAR  processor[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< Name of the CPU */
	NUI_CHAR  architecture[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< CPU architecture */
	NUI_CHAR  level[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< CPU level - valid for some processors only */
	NUI_CHAR  speed[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< CPU speed in MHz */
	NUI_CHAR  manufacturer[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< CPU manufacturer */
	NUI_CHAR  username[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< Current user name */
	NUI_CHAR  computername[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< Host name of the computer */
	NUI_CHAR  info[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< Formatted CPU info string */
	NUI_CHAR  unused[NUI_SUPPORT_SYSTEM_STRLEN];
    /*!< Not used any more */
} NUI_SUPPORT_SYSTEM_INFO;

/**
 * \brief Returns information about the computer.
 *
 * Returns processor information in a nicely formatted character string
 * and number of processors in the system. Upon error, it returns NULL
 * and sets error code.
 *
 * \returns Pointer to valid NUI_SUPPORT_SYSTEM_INFO structure or
 *          NULL on error
 */
NUI_SDK_API
NUI_SUPPORT_SYSTEM_INFO *NUI_SUPPORT_get_system_info(void);

#ifdef __cplusplus
}
#endif

#endif /* _NUI_SUPPORT_H_HAS_BEEN_INCLUDED_ */

/*
 * nui_support.h ends
 */
