/*
 * (C) 2003 Pixia Corp All rights reserved
 *
 * Tags are for automatic documentation generator 
 *
 */
/**
 * \file nui_cdf.h
 * \section NUI_CDF
 * \brief This API is for reading, writing and manipulating CDF files. 
 * 
 * This file format was inspired by NASA's CDF file format. In function 
 * it is similar. For Pixia Corp, we decided to follow a simple visual
 * hierarchy in an ASCII text file. A typical CDF file looks like this:
 * \code
 * nui_view_workspace_file = {
 *     date = May 20, 2004 01:20:51 PM
 *     copyright = (C) 2003 Pixia Corp. All rights reserved
 *     software = nuiViewer
 *     user_name = John Doe
 *     machine_type = Intel(x86 Family 15 Model 2 Stepping 7)-2392 MHz
 * }
 * total_projects = 1
 * filename = X:\Testimages\SingleImages.nvp
 * # start of project "NUI and Vector Data"
 * project = {
 *     name = NUI and Vector Data
 *     raster_state = 1
 *     # start of object "Boston 10 Inches"
 *     object = {
 *         filename = X:\Testimages\boston\raster\boston.nui
 *         name = Boston 10 Inches
 *         red_channel_layout = 0
 *         grn_channel_layout = 1
 *         blu_channel_layout = 2
 *         position_x = 20416
 *         position_y = 19585
 *         level = 0
 *     }
 *     # end of object "Boston 10 Inches"
 *     ...
 * }
 * \endcode
 *
 * The organization is quite simple. The file is made up of a name and
 * value pair. This name and value pair is called a \b NODE. The name 
 * and value pair is organized as <b> NAME = VALUE </b>.
 *
 * - A Pixia Corp CDF file is a list of \b NODEs.
 * - A \b NODE has one or more <b>NAME = VALUE</b> pairs.
 * - A \b NAME is an ASCII character string with no white spaces in it.
 * - A \b VALUE can be an ASCII character string or a \b NODE.
 * - If the \b VALUE is a \b NODE, the \b NODE is syntactically encapsulated
 *   within curly braces \b { and \b }.
 * - If a \b NODE 'A' has another \b NODE 'B' as its value, then \b NODE 'B'
 *   is considered to be the child of \b NODE 'A'.
 *
 * \authors Rahul Thakkar
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui_cdf/nui_cdf.h,v 2.0 2006/11/16 21:13:05 jensenk Exp $
 * $Log: nui_cdf.h,v $
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.9  2005/02/10 15:44:15  thakkarr
 * Updated comments and added documentation
 *
 * Revision 1.8  2005/01/17 18:27:47  thakkarr
 * Comments and constants match Pixia Standards
 *
 * Revision 1.7  2005/01/17 18:12:02  soods
 *  Added Empty Doxygen Tags for documentation, Aligned Code for clarity
 *
 * Revision 1.6  2005/01/17 11:31:43  soods
 *  Added NUI_SDK_API Tags
 *
 * Revision 1.5  2004/09/24 18:20:50  thakkarr
 * Update for install
 *
 * Revision 1.4  2004/09/23 19:26:55  thakkarr
 * Cleanup.
 *
 * Revision 1.3  2004/06/16 21:09:06  thakkarr
 * - NUI_CDF_append added
 * - NUI_CDF_print improved
 *
 * Revision 1.2  2003/07/17 21:51:46  thakkarr
 * add functions completed and tested with nui_calc_levels tool
 *
 * Revision 1.1  2003/07/16 22:51:36  thakkarr
 * added to cvs
 * first install
 *
 *
 */

#ifndef _NUI_CDF_H_HAS_BEEN_INCLUDED_
#define _NUI_CDF_H_HAS_BEEN_INCLUDED_

#include <nui_constants.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This is the NUI_CDF object that points to the entire CDF data set
 *        in memory.
 */
typedef struct nui_cdf_struct          NUI_CDF;
/**
 * \brief This object represents a single \b NODE within a NUI_CDF object.
 */
typedef struct nui_cdf_node_struct     NUI_CDF_NODE;

/**
 * \brief Creates an empty NUI_CDF object.
 *
 * \returns A pointer to a valid but empty NUI_CDF object. If out of memory,
 *        the function returns NULL and sets error code appropriately.
 * \sa NUI_CDF_error_string()
 */
NUI_SDK_API 
NUI_CDF *NUI_CDF_create(void);

/**
 * \brief Destroys a NUI_CDF object and frees all nodes.
 *
 * \param cdf (in) Pointer to a non-NULL NUI_CDF object.
 *        
 */
NUI_SDK_API 
void     NUI_CDF_destroy(NUI_CDF *cdf);

/**
 * \brief Loads data from a file into a NUI_CDF object and returns the
 *        newly created NUI_CDF object.
 * \param filename (in) Pointer to a non-NULL valid NUI_CDF file name.
 * 
 * \returns A valid and duly populated NUI_CDF object with all \b NODEs
 *        from the file loaded into memory, ready for traversal and 
 *        processing. Upon error, the function returns NULL and sets the
 *        error code.
 * \sa NUI_CDF_error_string(), NUI_CDF_save().
 * \code
 * // Print NUI_CDF - support function
 * static void print_nui_cdf_node(NUI_CDF_NODE *node, NUI_INT32 spaces,
 *                                NUI_INT32 incr)
 * {
 *      NUI_CHAR         format[64], *name = NULL;
 *      NUI_CDF_NODE    *child = NULL;
 *      if (NUI_CDF_NODE_has_children(node)) {
 *          sprintf(format, "%%%ds %%s = {", spaces);
 *          name = NUI_CDF_NODE_get_name(node);
 *          NUI_printf(format, " ", (name == NULL) ? "root node" : name);
 *          child = NUI_CDF_NODE_get_first_child(node);
 *          while (child != NULL) {
 *              print_nui_cdf_node(child, spaces+incr, incr);
 *              child = NUI_CDF_NODE_get_next_sibling(child);
 *          }
 *          sprintf(format, "%%%ds } # %%s", spaces);
 *          NUI_SUPPORT_printf(format, " ", NUI_CDF_NODE_get_name(node));
 *      } else {
 *          sprintf(format, "%%%ds %%s = %%s", spaces);
 *          NUI_SUPPORT_printf(format, " ", 
 *              NUI_CDF_NODE_get_name(node), NUI_CDF_NODE_get_value(node));
 *      }
 * }
 * // Print NUI_CDF
 * NUI_INT32 print_nui_cdf(char *filename)
 * {
 *     NUI_CDF      *cdf  = NULL;
 *     NUI_CDF_NODE *root = NULL;
 *     cdf = NUI_CDF_load(filename);
 *     // return -1 if cdf is NULL and process error.
 *     root = NUI_CDF_get_head_node(cdf);
 *     // return -1 if root is NULL and process error.
 *     print_nui_cdf_node(root, 0, 4);
 *     return 0;
 * }
 * \endcode
 */
NUI_SDK_API 
NUI_CDF *NUI_CDF_load(NUI_CHAR *filename);

/**
 * \brief Saves data from a NUI_CDF object into a NUI_CDF file
 *
 * \param cdf (in) Pointer to a non-NULL NUI_CDF object.
 * \param filename (in) Pointer to a non-NULL valid output NUI_CDF file name.
 *  
 * \returns 0 on success or -1 on failure. Error code is set upon error.    
 * \code
 * // Create a NUI_CDF file - support function.
 * void add_simple_assignments(NUI_CDF_NODE *parent, NUI_INT32 count)
 * {
 *     NUI_INT32       i;
 *     NUI_CHAR        name[64], value[64];
 *     NUI_DOUBLE      d;
 *     NUI_LONG        l;
 *     NUI_CDF_NODE   *node = NULL;
 *     for (i = 0; i < count; i++) {
 *         sprintf(value, "some value for this node %d", i);
 *         d = (double)rand();
 *         l = rand();
 *         sprintf(name, "new_node_name_%d", i);
 *         NUI_CDF_NODE_add_simple_assignment(parent, name, value);
 *         sprintf(name, "another_new_node_name_%d", i);
 *         NUI_CDF_NODE_add_long_assignment(parent, name, l);
 *         sprintf(name, "yet_another_new_node_name_%d", i);
 *         NUI_CDF_NODE_add_double_assignment(parent, name, d);
 *     }
 * }
 * // Create a NUI_CDF file.
 * NUI_INT32 create_nui_cdf(NUI_CHAR *filename) 
 * {
 *     NUI_CDF      *cdf  = NULL;
 *     NUI_CDF_NODE *root = NULL, *node = NULL;
 *     NUI_INT32     i;
 *     cdf = NUI_CDF_create(); // create object
 *     // return -1 if cdf is NULL and process error.
 *     root = NUI_CDF_NODE_create(); // create and set root node
 *     // return -1 if cdf is NULL and process error.
 *     NUI_CDF_set_head_node(cdf, root);
 *     add_simple_assignments(root, 3);
 *     for (i = 0; i < 4; i++) {
 *         node = NUI_CDF_NODE_create();
 *         // perform error check for node being NULL
 *         NUI_CDF_NODE_append_child(root, node);
 *         add_simple_assignments(node, 3);
 *     }
 *     NUI_CDF_set_info(cdf, "This is a sample CDF file", "1.0");
 *     NUI_CDF_save(cdf, filename); // save CDF
 *     // perform error check for save failure
 *     NUI_CDF_destroy(cdf); // cleanup
 *     return 0;
 * }
 * \endcode
 */
NUI_SDK_API 
NUI_INT32      NUI_CDF_save(NUI_CDF *cdf, NUI_CHAR *filename);

/**
 * \brief Appends one NUI_CDF object to another NUI_CDF object
 *
 * This is a convenience function. The source NUI_CDF object is
 * specified by parameter \b cdf and the destination NUI_CDF object 
 * is specified by parameter \b append_to. If you wish to append
 * the source NUI_CDF to the destination NUI_CDF such that it stands
 * by itself, you may do so by setting parameter \b append_as_node_name
 * to NULL. This means, source NUI_CDF is appended to destination
 * NUI_CDF at the \b root node level. If \b append_as_node_name is
 * a valid character string, a new node is created in \b append_to
 * NUI_CDF and the source NUI_CDF is added as a value to the new
 * node.
 *
 * \param cdf (in) Pointer to non-NULL source NUI_CDF you want to append.
 * \param append_to (in/out) Pointer to non-NULL destination NUI_CDF that you
 *        want parameter \b cdf appended to.
 * \param append_as_node_name (in) If NULL, \b cdf is appended to \b append_to
 *        at the root node level. Otherwise a new node by the name specified
 *        in parameter \b append_as_node_name is created under the root node
 *        and \b cdf is set as a value to this new node.
 */
NUI_SDK_API 
void     NUI_CDF_append(NUI_CDF *cdf, NUI_CDF *append_to, 
                        NUI_CHAR *append_as_node_name);
/**
 * \brief Returns the root node in the NUI_CDF file.
 * \param cdf (in) Pointer to a non-NULL NUI_CDF object.
 * \returns The root node in the NUI_CDF file or NULL if there is 
 *        no root node.
 */
NUI_SDK_API 
NUI_CDF_NODE *NUI_CDF_get_head_node(NUI_CDF *cdf);

/**
 * \brief Sets a node to be the root node.
 * \param cdf (in) Pointer to a non-NULL NUI_CDF object.
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object that will become
 *        the root node to the NUI_CDF object. Root nodes are special. All they
 *        can have are other nodes as children.
 */
NUI_SDK_API 
void          NUI_CDF_set_head_node(NUI_CDF *cdf, NUI_CDF_NODE *node);

/**
 * \brief Creates an empty NUI_CDF_NODE object.
 * \returns A pointer to a new and empty NUI_CDF_NODE object.
 */
NUI_SDK_API 
NUI_CDF_NODE *NUI_CDF_NODE_create(void);

/**
 * \brief Destroys a valid NUI_CDF_NODE object and frees all memory 
 *        allocated by it.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 *        
 */
NUI_SDK_API 
void          NUI_CDF_NODE_destroy(NUI_CDF_NODE *node);

/**
 * \brief Returns NUI_TRUE (1) if the node has 1 or more nodes as children.
 *        Otherwise it returns NUI_FALSE (0).
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 * \returns Returns NUI_TRUE if node has 1 or more children, else NUI_FALSE.
 *        
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_has_children(NUI_CDF_NODE *node);

/**
 * \brief Returns NUI_TRUE (1) if the node is a \b name=value pair.
 *        Otherwise it returns NUI_FALSE (0).
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 * \returns Returns NUI_TRUE if node is a \b name=value pair, else NUI_FALSE.
 *        
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_has_value(NUI_CDF_NODE *node);

/**
 * \brief Returns the number of child \b NODEs that this \b NODE has.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 * \returns The number of child \b NODEs that this \b NODE has.
 *        
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_get_number_of_children(NUI_CDF_NODE *node);

/**
 * \brief Returns the first child in the list of children that 
 *        this \b NODE has.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 * \returns The first child in the list of children that this \b NODE has.
 *        It returns NULL if the node has no children.
 * \sa NUI_CDF_NODE_get_number_of_children(), NUI_CDF_NODE_has_children().
 *        
 */
NUI_SDK_API 
NUI_CDF_NODE *NUI_CDF_NODE_get_first_child(NUI_CDF_NODE *node);

/**
 * \brief Upon getting the first child node from a list of children
 *        for a particular node, you can use the child node to traverse
 *        through the list of all the children.
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 * \returns The next child of the parent of parameter \b node, i.e. it
 *        returns the next sibling of this node. It returns NULL if
 *        there are no more siblings.
 */
NUI_SDK_API 
NUI_CDF_NODE *NUI_CDF_NODE_get_next_sibling(NUI_CDF_NODE *node);

/**
 * \brief Searches for a child node of a specified name, of the 
 *        specified node
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object.
 * \param name (in) Pointer to a non-NULL character string that is the
 *        name of the child-node you are searching for.
 * \returns Returns the first child node of the specified name or NULL.
 *        Error code is set upon NULL return.
 */
NUI_SDK_API 
NUI_CDF_NODE *NUI_CDF_NODE_find_child_by_name(NUI_CDF_NODE *node, 
                                              NUI_CHAR     *name);

/**
 * \brief Adds a child node to the specified parent node.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object that will
 *        be the parent node.
 * \param child_node (in) Pointer to a non-NULL NUI_CDF_NODE object that
 *        will be appended to the parent node's child list.
 *        
 */
NUI_SDK_API 
void          NUI_CDF_NODE_append_child(NUI_CDF_NODE *node, 
                                        NUI_CDF_NODE *child_node);

/**
 * \brief Removed the \b child_node from it's parent \b node object.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object that will
 *        be the parent node.
 * \param child_node (in) Pointer to a non-NULL NUI_CDF_NODE object that
 *        will be removed from the parent node's child list.
 * \node This function does not destroy the \b child_node.
 */
NUI_SDK_API 
void          NUI_CDF_NODE_delete_child(NUI_CDF_NODE *node, 
                                        NUI_CDF_NODE *child_node);

/**
 * \brief Creates a new node of name=value type and appends it as
 *        a child to NUI_CDF_NODE parameter \b node. The value of the
 *        child node is assumed to be a character string.
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object that will
 *        be the parent node.
 * \param name (in) Character string (non-NULL, no spaces) that's the name
 *        of the new child node that's appended to parameter \b node.
 * \param value (in) Character string (non-NULL) that's the value of the
 *        new child node that's appended to parameter \b node.
 * \returns 0 on success or -1 on failure. Sets error code.
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_add_simple_assignment(NUI_CDF_NODE *node, 
                                                 NUI_CHAR     *name, 
                                                 NUI_CHAR     *value);

/**
 * \brief Creates a new node of name=value type and appends it as
 *        a child to NUI_CDF_NODE parameter \b node. The value of the
 *        child node is assumed to be a long integer.
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object that will
 *        be the parent node.
 * \param name (in) Character string (non-NULL, no spaces) that's the name
 *        of the new child node that's appended to parameter \b node.
 * \param value (in) A long integer that's the value of the
 *        new child node that's appended to parameter \b node.
 * \returns 0 on success or -1 on failure. Sets error code.
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_add_long_assignment(NUI_CDF_NODE *node, 
                                               NUI_CHAR     *name, 
                                               NUI_LONG      value);

/**
 * \brief Creates a new node of name=value type and appends it as
 *        a child to NUI_CDF_NODE parameter \b node. The value of the child
 *        node is assumed to be a double-precision floating-point number.
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object that will
 *        be the parent node.
 * \param name (in) Character string (non-NULL, no spaces) that's the name
 *        of the new child node that's appended to parameter \b node.
 * \param value (in) A double-precision floating-point number that's the
 *        value of the new child node that's appended to parameter \b node.
 * \returns 0 on success or -1 on failure. Sets error code.
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_add_double_assignment(NUI_CDF_NODE *node, 
												 NUI_CHAR     *name, 
                                                 NUI_DOUBLE    value);

/**
 * \brief Returns the parent of the specified node or NULL if no parent.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object whose parent
 *        node is desired.
 * \returns The parent of the specified node or NULL if no parent.
 */
NUI_SDK_API 
NUI_CDF_NODE *NUI_CDF_NODE_get_parent(NUI_CDF_NODE *node);

/**
 * \brief Sets the name of a NUI_CDF_NODE object.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object
 * \param name (in) Character string (non-NULL, no spaces) that's to
 *        be the name of the node.
 *        
 * \returns
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_set_name(NUI_CDF_NODE *node, NUI_CHAR *name);

/**
 * \brief Returns the name of the node, or NULL if no name was assigned.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object
 *        
 * \returns The name of the node, or NULL if no name was assigned.
 */
NUI_SDK_API 
NUI_CHAR     *NUI_CDF_NODE_get_name(NUI_CDF_NODE *node);

/**
 * \brief Sets the value of a NUI_CDF_NODE object.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object
 * \param value (in) Character string (non-NULL) that's to
 *        be the value of the node.
 *        
 * \returns
 */
NUI_SDK_API 
NUI_INT32     NUI_CDF_NODE_set_value(NUI_CDF_NODE *node, NUI_CHAR *value);

/**
 * \brief Returns the value of the node, or NULL if no value was assigned.
 *        The value is returned as a character string.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object
 *        
 * \returns The value of the node, or NULL if no value was assigned.
 *        The value is returned as a character string.
 */
NUI_SDK_API 
NUI_CHAR      *NUI_CDF_NODE_get_value(NUI_CDF_NODE *node);

/**
 * \brief Returns the value of the node. The value is returned as a 
 *        long integer.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object
 *        
 * \returns The value of the node. The value is returned as a 
 *        long integer.
 */
NUI_SDK_API 
NUI_LONG       NUI_CDF_NODE_get_value_as_long(NUI_CDF_NODE *node);

/**
 * \brief Returns the value of the node. The value is returned as a 
 *        double.
 *
 * \param node (in) Pointer to a non-NULL NUI_CDF_NODE object
 *        
 * \returns The value of the node. The value is returned as a 
 *        double.
 */
NUI_SDK_API 
NUI_DOUBLE     NUI_CDF_NODE_get_value_as_double(NUI_CDF_NODE *node);

/**
 * \brief Creates a special node in the NUI_CDF object that is the
 *        \b information node. This node has two character strings
 *        as values. The first is some information you wish to 
 *        set for the file. The second is a version string.
 * \param cdf (in) Pointer to a non-NULL NUI_CDF object
 * \param info (in) Character string that has information about the NUI_CDF.
 * \param version (in) Character string that has version of the NUI_CDF.
 */
NUI_SDK_API 
void           NUI_CDF_set_info(NUI_CDF  *cdf, 
                                NUI_CHAR *info, 
                                NUI_CHAR *version);

/* error codes */
#define NUI_CDF_ERR_MEMORY		1 /*!< Out of memory */
#define NUI_CDF_ERR_SYSTEM		2 /*!< Returns strerror(errno) */
#define NUI_CDF_ERR_FORMAT      3 /*!< Bad CDF file format */
#define NUI_CDF_ERR_FIND        4 /*!< Name not found in list */

/* error management and debug functions */
extern NUI_INT32 NUI_CDF_errno; /*!< Error code for NUI_CDF API */

/**
 * \brief Returns a statically allocated error string that corresponds
 *        to the value of NUI_CDF_errno. Do not free this string.
 *        This is a non-reentrant function.
 * \returns A statically allocated error string that corresponds
 *        to the value of NUI_CDF_errno. Do not free this string.
 */
NUI_SDK_API 
NUI_CHAR *NUI_CDF_error_string(void);

/**
 * \brief Prints information about the NUI_CDF object and all its
 *        nodes to \b stdout.
 * \param cdf (in) Pointer to a non-NULL NUI_CDF object.
 */
NUI_SDK_API 
void  NUI_CDF_print(NUI_CDF *cdf); /* debug NUI_printf on stdout */

#ifdef __cplusplus
}
#endif

#endif /* _NUI_CDF_H_HAS_BEEN_INCLUDED_ */

/*
 * nui_cdf.h ends
 */
