/*
 * Copyright (C) 2007 Pixia Corp., All Rights Reserved.
 */
/**
 * \file nui_alloc.h
 * \section NUI_ALLOC
 * \author Kirk Jensen 
 *
 * \brief Wrapper for basic memory allocation.
 *
 * \htmlonly Copyright &copy Pixia Corp., All rights reserved.\endhtmlonly
 * 
 * \note NUI_SUPPORT_malloc(), NUI_SUPPORT_free(), and NUI_SUPPORT_strdup() 
 *       will be deprecated in the near future.  They are replaced by 
 *       NUI_malloc_aligned(), NUI_free_aligned(), and NUI_strdup(),
 *       respectively.
 */

#ifndef NUI_ALLOC_H_HAS_BEEN_INCLUDED
#define NUI_ALLOC_H_HAS_BEEN_INCLUDED
#pragma once

#include <stdlib.h>
#include <nui_constants.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief Disables tracking of memory allocations in debug builds when the 
 * nuiSDK when released to customers.  If this is not defined here, customers'
 * programs will not link to nuiSDK properly if _DEBUG is defined at
 * compile time (which is default behavior of Visual C++).
 */
#define NUI_NO_DEBUG_MALLOC
#undef  NUI_DEBUG_MALLOC


/**
 * \brief Internal helper function for NUI_malloc(). Do not call this function
 *        in your code.
 * \internal NUI_malloc() will expand to this function at compile-time in 
 *        non-debug mode.  Always use nui_free_ndebug() to free pointers
 *        allocated by this function. 
 */
NUI_SDK_API
void *nui_malloc_ndebug(size_t size);  


/**
 * \brief Internal helper function for NUI_free(). Do not call this function
 *        in your code.
 * \internal NUI_free() will expand to this function at compile-time in 
 *        non-debug mode. Use this function to deallocate pointers returned 
 *        by nui_malloc_ndebug(). 
 */
NUI_SDK_API
void  nui_free_ndebug(void* p);


/**
 * \brief Internal helper function for NUI_malloc_aligned(). Do not call this
 *        function in your code.
 * \internal NUI_malloc_aligned() will expand to this function at compile-time
 *        in non-debug mode. Always use nui_free_aligned_ndebug() to free 
 *        pointers allocated by this function.
 */
NUI_SDK_API
void *nui_malloc_aligned_ndebug(size_t size, int value);


/**
 * \brief Internal helper function for NUI_free_aligned(). Do not call this 
 *        function in your code.
 * \internal NUI_free_aligned() will expand to this function at compile-time
 *        in non-debug mode. Use this function to deallocate pointers returned 
 *        by nui_malloc_aligned_ndebug(). 
 */
NUI_SDK_API
void  nui_free_aligned_ndebug(void* p);


/** 
 * \brief Allocates a small memory block.
 * \param bytes (in) - number of bytes to allocate.
 * \returns A valid void pointer to allocated memory, or NULL if out of memory
 *       or if bytes is 0.
 * \note Use NUI_free() to free any non-NULL pointer returned by 
 *       NUI_malloc().  
 * \sa NUI_free, NUI_malloc_aligned, NUI_free_aligned.
 */
#define NUI_malloc(bytes) nui_malloc_ndebug((bytes))


/** 
 * \brief Frees memory allocated by NUI_malloc.
 * \param pointer (in) - non-NULL memory block allocated by NUI_malloc.
 * \sa NUI_malloc, NUI_strdup.
 */
#define NUI_free(pointer)  nui_free_ndebug((pointer))


/** 
 * \brief Allocates a page-aligned memory block, and also clears the new
 *        memory to specified value if desired.
 *
 * \note Use NUI_free() to free any non-NULL pointer returned by NUI_malloc().
 * \param bytes (in) - number of bytes to allocate.
 * \param value (in) - a number between 0 and 255 to which all the
 *        bytes in the memory buffer will be initialized. If a negative
 *        number is specified, then no initialization is performed.
 *
 * \returns A valid void pointer to allocated memory, or NULL if out of memory
 *       or if \c bytes is 0.
 * \sa NUI_free_aligned, NUI_malloc, NUI_free.
 */
#define NUI_malloc_aligned(bytes, value)  nui_malloc_aligned_ndebug((bytes), (value)) 


/**
 * \brief Frees memory that was allocated using NUI_malloc_aligned().
 * \param pointer (in) non-NULL memory block allocated by NUI_malloc_aligned.
 */
#define NUI_free_aligned(pointer) nui_free_aligned_ndebug((pointer))


/** 
 * \brief Returns the duplicate of a NULL-terminated character string.
 * \param string (in) - a NULL terminated character string to be duplicated.
 * \note Does not use strdup(). Code is written from scratch to perfrom
 *       duplication. Uses NUI_malloc() to allocate memory for the
 *       duplicate string. Use NUI_free() to free returned duplicate.
 * \returns Pointer to a NULL-terminated character string that is a 
 *          duplicate of \c string.
 */
NUI_SDK_API
char *NUI_strdup(const char *string); 


#ifdef __cplusplus
}
#endif

#endif //NUI_ALLOC_H_HAS_BEEN_INCLUDED

/*
 * End of nui_alloc.h
 */

