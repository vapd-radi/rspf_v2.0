/*
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 *
 */
/**
 * \file nui_arg.h
 * \section NUI_ARG
 * \brief Pixia Corp's standard command line argument processing API
 *
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * This API is a standard argument parsing API for Pixia Corp. For
 * every C or C++ application, Pixia Corp. programming standards
 * recommend having the ability to run the program with command line
 * arguments. This enables launching the program as a stand alone tool
 * that runs in a batch process on servers, or if it has a GUI, it
 * may optionally have a variable argument list that enables the GUI
 * to start up based on user needs.
 * 
 * \author
 * Rahul Thakkar, Saurabh Sood.
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui_arg/nui_arg.h,v 2.0 2006/11/16 21:13:05 jensenk Exp $
 * $Log: nui_arg.h,v $
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.6  2005/02/09 13:46:45  thakkarr
 * NUI_ARG comments had incorrect information due to copy/paste error. Fixed.
 *
 * Revision 1.5  2005/02/03 12:57:30  thakkarr
 * Added DSP and DSW files
 *
 * Revision 1.4  2005/01/17 18:04:50  thakkarr
 * Updated authors list
 *
 * Revision 1.3  2005/01/17 18:03:18  thakkarr
 * Fixed comments and variable names to match Pixia Corp standards
 *
 * Revision 1.2  2005/01/17 14:22:16  soods
 *  Corrected the lines exceeding 80 column count
 *
 * Revision 1.1  2005/01/14 19:07:52  soods
 *  Added new library as a successor to ARG library
 *
 *
 *
 *
 */

#ifndef _NUI_ARG_H_HAS_BEEN_INCLUDED_
#define _NUI_ARG_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h> /*!< For NUI_ constants */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Type of command line argument for NUI_ARG_parse_arguments() 
 */
typedef enum {
	NUI_ARG_ARGUMENT = 1, /*!< For mandatory arguments */
	NUI_ARG_OPTION   = 2, /*!< For options with parameters */
	NUI_ARG_FLAG     = 3, /*!< For optional flags with no parameters */
	NUI_ARG_END      = 4 /*!< Indicates end of arguments */
} NUI_ARG_TYPE; 

/**
 * \brief Data type of parameter to option or argument
 */
typedef enum {
	NUI_ARG_CHAR   = 110, /*!< Argument or option is a single character */
	NUI_ARG_STRING = 111, /*!< Argument or option is an ASCII string */
	NUI_ARG_INT    = 112, /*!< Argument or option is a signed 32-bit number */
	NUI_ARG_SHORT  = 113, /*!< Argument or option is a signed 16-bit number */
	NUI_ARG_LONG   = 114, /*!< Argument or option is a signed 32-bit number */
	NUI_ARG_FLOAT  = 115, /*!< Argument or option is a 32-bit float */
	NUI_ARG_DOUBLE = 116  /*!< Argument or option is a 64-bit float */
} NUI_ARG_DATA_TYPE; 

/**
 * NUI_ARG_parse_arguments:
 *
 * \brief
 * Function parses arguments from the command line and returns
 * desired values into passed variables.
 *
 * This function parses arguments in a meaningful way. This
 * reduces amount of parsing work necessary for every program.
 * This also enforces a very simple interface on the command-line.
 * Since the argument parsed by this function do not leave room
 * for variable arguments, there is a post processor function
 * that can be used by the programmer to process variable arguments.
 * The argument post processor is passed only those arguments or
 * options that the software was unable to parse or understand.
 * Argument post processor function is set using
 * NUI_ARG_set_post_parse_arguments_handler().
 *
 * Arguments are mandatory, options and flags are advisory and
 * the user must assign a default value to all flags and options 
 * before passing them to NUI_ARG_parse_arguments.
 *
 * --help and --usage are reserved flags, they will print API
 * generated usage message on the console and exit.
 *
 * All options and flags MUST start with a '-' character, 
 * otherwise they are assumed to be mandatory arguments.
 *
 * \param argc is same as argc in main(int argc, char *argv[]) 
 *
 * \param argv is same as argv in main(int argc, char *argv[]) 
 *
 * \param ...  are variable arguments that combine to form the guidelines
 *        that are able to process the command line. They are:
 *
 *        - First variable parameter in set describes argument type: 
 *           - NUI_ARG_ARGUMENT: means a mandatory argument list follows
 *           - NUI_ARG_OPTION: means an option follows
 *           - NUI_ARG_FLAG: means a flag follows
 *        - Next parameter is name of flag or option or NULL for argument
 *           E.g. -viewport or -verbose
 *        - Next parameter specifies number of parameters for argument or
 *          option. E.g. -viewport x0 y0 x1 y1 will have 4 as the parameter,
 *          for a flag this is -1.
 *        - If number of parameters is N, the next N * 2 parameters
 *          to the function specify the what values are read in and where
 *          to load correct values for the option or argument. Each param
 *          to the argument or option has 2 parameter in the function:
 *            - Data type of the parameter followed by
 *            - Valid address where value of this option/argument is placed. 
 *              - Supported data types are all  values of NUI_ARG_DATA_TYPE.
 *              - Flags, we know have no additional parameters. The:
 *                - Data type of the parameter is NUI_ARG_INT
 *                - Valid address where the flag's value is placed
 *        - Next parameter to the function is a 10/12 word usage message
 *          of the option, argument or flag. It cannot be NULL.
 *        - The very last parameter must be NUI_ARG_END to indicate end of list.
 *
 * \return
 * Function returns 0 on success, -1 on failure and sets error code
 * If there is an error, internal error code is set and functions
 * NUI_ARG_usage_string(), NUI_ARG_error_string() become meaningful.
 *
 * Sample code:
 * \code 
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include <string.h>
 * #include <errno.h>
 * #include <stdarg.h>
 * #include <nui_arg.h>
 * int 
 * main(int nui_argc, char *argv[]) 
 * {
 *      char *nui_file_name = NULL, *bim_file_name = NULL;
 *      int   x0 = -1, y0 = -1, x1 = -1, y1 = -1;
 *      short levels = -1;
 *      int   info_flag = 0, debug_flag = 0, return_value = 0;
 *      return_value = NUI_ARG_parse_arguments(argc, argv, 
 *          NUI_ARG_ARGUMENT, NULL, 2, 
 *              NUI_ARG_STRING, &nui_file_name, 
 *              NUI_ARG_STRING, &bim_file_name, 
 *                 "input_file.nui output_file.bim",
 *          NUI_ARG_OPTION, "-viewport", 4,
 *              NUI_ARG_LONG, &x0,
 *              NUI_ARG_LONG, &y0,
 *              NUI_ARG_LONG, &x1,
 *              NUI_ARG_LONG, &y1,
 *                  "x0 y0 x1 y1 viewport to extract",
 *          NUI_ARG_OPTION, "-level", 1,
 *              NUI_ARG_SHORT, &level,
 *                  "specifies input pyramid level",
 *          NUI_ARG_FLAG, "-info",  -1, 
 *              NUI_ARG_INT, &info_flag,  
 *                  "enables verbose mode",
 *          NUI_ARG_FLAG, "-debug", -1, 
 *              NUI_ARG_INT, &debug_flag,   
 *                  "enables debugging mode",
 *          NUI_ARG_END);
 *      if (return_value < 0) {
 *          printf("%s error: Invalid command-line arguments (%s)\n",
 *              argv[0], NUI_ARG_error_string());
 *          printf("%s\n", NUI_ARG_usage_string());
 *          return EXIT_FAILURE;
 *      }
 *      // continue normally with your application's code
 *      return EXIT_SUCCESS;
 * }
 * \endcode
 *
 * Sample output to "nui_arg_test --help"
 * \code
 * Usage: nui_arg_test arguments [options][flags]
 * Where:
 * Arguments to the program are:
 * STR STR                       input_file.nui output_file.bim
 *                               total arguments: 2
 *                               argument 1 (STR) - string
 *                               argument 2 (STR) - string
 * Options to the program are:
 * -viewport LNG LNG LNG LNG     x0 y0 x1 y1 viewport to extract
 *                               total parameters to option: 4
 *                               parameter 1 (LNG) - long
 *                               parameter 2 (LNG) - long
 *                               parameter 3 (LNG) - long
 *                               parameter 4 (LNG) - long
 * -level SHT                    specifies scale factor
 *                               total parameters to option: 1
 *                               parameter 1 (SHT) - short
 * Flags to the program are:
 * -info                         enables verbose mode
 * -debug                        enables debugging mode
 * 
 * Standard flags:
 * --usage         prints usage and exits
 * --help          prints usage and exits
 * \endcode
 */
NUI_SDK_API 
NUI_INT32 NUI_ARG_parse_arguments(NUI_INT32 argc, NUI_CHAR **argv, ...);

/**
 * NUI_ARG_usage_string:
 *
 * \brief returns program usage string based on input parameters.
 *
 * If NUI_ARG_parse_arguments() returns -1, this function becomes
 * valid. It generates a valid usage string based on parameters
 * passed to NUI_ARG_parse_arguments(). If NUI_ARG_set_custom_usage_message()
 * was called prior to calling NUI_ARG_parse_arguments() with a non-NULL
 * valid custom usage message, that string is returned instead of
 * automatically generated usage message.
 *
 * \return
 * Formatted ASCII character string for printing on console or GUI.
 *
 */
NUI_SDK_API 
NUI_CHAR *NUI_ARG_usage_string(void);

/**
 * NUI_ARG_set_custom_usage_message:
 *
 * \brief Sets an alternate usage message instead of standard
 *        one created for the user. Programmer is advised to
 *        follow whatever standards are set for the organization.
 * \param usage_message (in) specifies programmer defined usage message
 */
NUI_SDK_API 
void  NUI_ARG_set_custom_usage_message(NUI_CHAR *usage_message);

/**
 * NUI_ARG_error_string:
 *
 * \brief returns error string if NUI_ARG_parse_arguments() returns -1.
 *
 * If NUI_ARG_parse_arguments() returns -1, this function becomes
 * valid. It returns a valid error string based on parameters
 * passed to NUI_ARG_parse_arguments() and resultant error. If an 
 * alternate error string was set using NUI_ARG_set_custom_error_string()
 * prior to this function being called, it returns that custom
 * error string.
 *
 * \return
 * Formatted ASCII error string for printing on console or GUI.
 *
 */
NUI_SDK_API 
NUI_CHAR *NUI_ARG_error_string(void);

/**
 * NUI_ARG_set_post_parse_arguments_handler:
 *
 * \brief
 * Sets the function that is called within NUI_ARG_parse_arguments()
 * \b after NUI_ARG_parse_arguments() has completed parsing.
 *
 * If the user wishes to extend capabilities of
 * NUI_ARG_parse_arguments(), after NUI_ARG_parse_arguments() is done
 * with its work, NUI_ARG_set_post_parse_arguments_handler() will
 * set a valid handler function declared in your program as:
 *
 * \code
 * int your_function_post_processor_for_arguments(int argc, 
 *                                       char *argv[], 
 *                                       void *data);
 * \endcode
 *
 * \param callback is declared as above. Its 3 parameters:
 *    - argc
 *    - argv
 *    - data
 * \param argc specifies number of arguments in \b argv. 
 * \param argv points to the list of arguments that were 
 *    left-over after NUI_ARG_parse_arguments() completed processing 
 *    parameters it was supposed to process. It can be NULL. 
 * \param data is used by the programmer to pass custom data. 
 *    It can be NULL.
 *
 * The post processor for arguments handler must return 0 for
 * success and -1 for failure. Upon failure, programmer must
 * use the function NUI_ARG_set_custom_error_string() to set the
 * error message.
 *
 * Set callback function before calling NUI_ARG_parse_arguments().
 */
NUI_SDK_API 
void  NUI_ARG_set_post_parse_arguments_handler(
       NUI_INT32 (*callback)(NUI_INT32 argc, NUI_CHAR *argv[], void *data),
       void       *callback_data);

/**
 * NUI_ARG_set_custom_error_string:
 *
 * \brief 
 * Sets a custom error string to be returned by
 * NUI_ARG_error_string() instead of standard error strings.
 *
 * If NUI_ARG_parse_arguments() returns -1, this function becomes
 * valid. It is recommended that you used this function in the
 * post parse arguments handler. 
 *
 * If programmer's post parse arguments handler returns -1, 
 * the error string, which the programmer wants from 
 * NUI_ARG_error_string(), can be set using this function.
 *
 * \sa NUI_ARG_error_string, NUI_ARG_parse_arguments, 
 *     NUI_ARG_set_post_parse_arguments_handler
 * 
 */
NUI_SDK_API 
void  NUI_ARG_set_custom_error_string(NUI_CHAR *custom_error_string);


#ifdef __cplusplus
}
#endif

#endif /* _NUI_ARG_H_HAS_BEEN_INCLUDED_ */

/*
 * nui_arg.h ends
 */
