/*
 * Copyright (C) 2000 Pixia Corp., All Rights Reserved.
 *
 * Tags are for automatic documentation generator
 */
/**
 * \file nui_eta.h
 * \section NUI_ETA
 * \brief Computes elapsed and remaining time for a repetitive task where
 *        each task takes nearly the same time.
 *
 * \htmlonly Copyright &copy Pixia Corp, All rights reserved \endhtmlonly
 *
 * This library provides basic interface to measure time
 * elapsed in the completion of a sequential task that updates
 * periodically at a constant rate. This library also provides
 * ability to estimate time of completion of the same tasks.
 *
 * \warning As you can see below, most functions take NUI_ETA * as
 *          the lead parameter with dealing with the NUI_ETA object.
 *          If this pointer is NULL, in case of the NUI_ETA library,
 *          the function ignores the operation you want to perform
 *          if it's supposed to return any data, the function will return 
 *          a bogus value. This is contrary to most libraries where the
 *          application would generate an assertion and exit. This is 
 *          because this library is a high-performance library and
 *          sophisticated error management may render it a tad-bit slow.
 *
 * One way of using NUI_ETA:
 * \code
 *  NUI_ETA *eta = NULL;
 *  NUI_CHAR elapsed[256];
 *  NUI_CHAR remains[256];
 *  ...
 *  your_stuff();
 *  ...
 *  eta = NUI_ETA_create();
 *  NUI_ETA_begin(eta, start_point, end_point); // start < end 
 *  for (current_point = start_point; current_point <= end_point;
 *          current_point += some_increment) {
 *      ... // performs a time consuming task...
 *      ... // time consuming task completed...
 *      NUI_ETA_update(eta, current_point);
 *      s_elap = NUI_ETA_get_time_elapsed(eta, NULL, NULL, NULL);
 *      s_eta = NUI_ETA_get_time_remaining(eta, NULL, NULL, NULL);
 *      sprintf(elapsed, "%s elapsed", 
 *          NUI_ETA_time_format_extended(eta, s_elap, NULL, NULL, NULL));
 *      sprintf(remains, "%s remaining", 
 *          NUI_ETA_time_format_extended(eta, s_eta, NULL, NULL, NULL));
 *      printf("%s, %s\n", elapsed, remains);
 *  } 
 *  NUI_ETA_end(eta);
 *  NUI_ETA_destroy(eta);
 * \endcode
 *
 * \authors Rahul Thakkar, Saurabh Sood.
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui_eta/nui_eta.h,v 2.0 2006/11/16 21:13:05 jensenk Exp $
 * $Log: nui_eta.h,v $
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.15  2005/06/15 19:37:56  thakkarr
 * Changed the name of eta_info to _nui_eta_info_ for consistency
 *
 * Revision 1.14  2005/06/15 19:30:58  thakkarr
 * Better comment for NUI_ETA_time()
 *
 * Revision 1.13  2005/06/15 17:51:22  thakkarr
 * 1. NUI_ETA_time() uses CPU timer functions for Linux, IRIX and Windows.
 *    This function uses gettimeofday() on MAC OS X
 * 2. gettimeofday() no longer used in NUI_ETA_begin and _update functions
 *    for Linux, IRIX and Windows. Using CPU timer functions instead
 * 3. gettimeofday() only used on MAC OS X
 * 4. CPU timer functions on MAC OS X return -1
 * 5. Comments updated
 *
 * Revision 1.12  2005/02/24 14:33:31  thakkarr
 * Ugly looking comment in cvs log has been fixed
 *
 * Revision 1.11  2005/02/16 09:06:37  thakkarr
 * Comment beautified
 *
 * Revision 1.10  2005/02/16 08:52:13  thakkarr
 * Merged nui_eta_cpu_time.c into nui_eta.c 
 *
 * Revision 1.9  2005/02/16 07:10:56  thakkarr
 * Added code to get timer-ticks since reference point for IRIX
 *
 * Revision 1.8  2005/02/16 04:57:51  thakkarr
 * Added Linux and IRIX functions
 *
 * Revision 1.7  2005/02/11 16:12:05  thakkarr
 * Improved comments
 *
 * Revision 1.6  2005/01/21 17:03:54  thakkarr
 * Added Microsoft specific time measurement functions
 *
 * Revision 1.5  2005/01/17 18:04:59  thakkarr
 * Updated authors list
 *
 * Revision 1.4  2005/01/17 18:02:56  thakkarr
 * Fixed comments and variable names to match Pixia Corp requirements
 *
 * Revision 1.3  2005/01/17 14:26:26  soods
 *  Corrected lines exceeding 80 column limit
 *
 * Revision 1.2  2005/01/17 11:32:15  soods
 *  Removed one incorrect NUI_SDK_API Tag
 *
 * Revision 1.1  2005/01/14 19:08:52  soods
 *  Added new library as a successor to ETA library
 *
 *
 */

#ifndef _NUI_ETA_HAS_BEEN_INCLUDED_
#define _NUI_ETA_HAS_BEEN_INCLUDED_

#include <nui_constants.h> /*!< For NUI_ constants */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Specifies NUI_ETA object. 
 *
 * NUI_ETA object holds data for computing time elapsed and remaining to
 * complete a sequence of evenly spaced tasks.
 */
typedef struct nui_eta_data NUI_ETA;                                             
                                                                            
/**
 * \brief Creates an empty NUI_ETA object
 * \returns A valid pointer to an empty NUI_ETA object or NULL on error
 */
NUI_SDK_API 
NUI_ETA	*NUI_ETA_create(void);

/**
 * \brief Destroys a valid NUI_ETA object and frees allocated memory
 * \param eta - (in) A valid, non-NULL pointer to NUI_ETA object
 * \sa NUI_ETA_create()
 */
NUI_SDK_API 
void	NUI_ETA_destroy(NUI_ETA *eta);

/**
 * \brief Initializes the NUI_ETA object to get prepared for measuring timings
 *        for a sequence of repetitive tasks that run at a constant rate.
 *
 * If you are in a sequence of repetitive tasks that start at point A 
 * (a counter) and end at point B (end of that counter), this function
 * allows you to specify the start and end points. The starting point
 * must always be a lower number than the ending point. The start and end
 * point indication prepares the NUI_ETA object to subsequently start
 * the measurement. 
 * 
 * If you are measuring the time taken for each iteration of a loop, this
 * function would be called prior to starting the loop.
 *
 * \note Use NUI_ETA_update() after this function, in a loop, to update
 *       state of tasks completed.
 * \param eta - (in) A valid, non-NULL pointer to NUI_ETA object
 * \param task_begin - (in) Number where a sequence of tasks begins. This
 *        number is starting point of the sequence of tasks. The starting
 *        point is to be smaller than the ending point.
 * \param task_end - (in) Number where a sequence of tasks ends.
 *        It must be such that \b task_end > \b task_begin.
 * \sa NUI_ETA_create(), NUI_ETA_end(), NUI_ETA_update()
 */
NUI_SDK_API 
void	NUI_ETA_begin(NUI_ETA *eta, 
                      NUI_DOUBLE task_begin, NUI_DOUBLE task_end);

/**
 * \brief Indicates the end of a sequence of repetitive tasks and resets 
 *        its internal clocks.
 * \param eta - (in) A valid, non-NULL pointer to NUI_ETA object
 * \sa NUI_ETA_begin()
 */
NUI_SDK_API 
void	NUI_ETA_end(NUI_ETA *eta);

/**
 * \brief Updates a sequence of repetitive tasks to the current task
 * 
 * Function registers that the specified task number was completed
 * and adjusts internal measurement meters accordingly. The current
 * task number is such that \c task_begin <= \c task_current <= \c task_end,
 * where \c task_begin and \c task_end were specified using a prior call
 * to NUI_ETA_begin(). Before calling NUI_ETA_update(), you may call
 * NUI_ETA_set_update_callback() to execute a callback function 
 * everytime NUI_ETA_update() is executed.
 *
 * \param eta - (in) A non-NULL pointer to NUI_ETA object
 * \param task_current - (in) Current task number completed (\c task_begin <= 
 *        \c task_current <= \c task_end)
 * \sa NUI_ETA_begin(), NUI_ETA_set_update_callback()
 */
NUI_SDK_API 
NUI_DOUBLE NUI_ETA_update(NUI_ETA *eta, NUI_DOUBLE task_current);

/**
 * \brief Used to provide information to the callback function set by
 *        NUI_ETA_set_update_callback().
 *
 * When programmer sets a callback function using NUI_ETA_set_update_callback(),
 * Each time the NUI_ETA_update() function is called, the last thing it does
 * is update an instance of NUI_ETA_INFO, pass it to and call this callback.
 * Within the callback, the application programmer can then use the data
 * to update the user interface.
 * \sa NUI_ETA_set_update_callback()
 */
typedef struct _nui_eta_info_ {
	NUI_DOUBLE total_tasks; 
           /*!< 0 to (task_end - task_begin) */
	NUI_DOUBLE tasks_elapsed; 
           /*!< Total tasks completed, so far */
	NUI_DOUBLE tasks_remaining; 
           /*!< Remaining tasks */
	NUI_DOUBLE seconds_elapsed; 
           /*!< Seconds elapsed for this task, so far */
	NUI_DOUBLE seconds_remaining; 
           /*!< Seconds remaining for this task to end */
	NUI_DOUBLE percent_elapsed; 
           /*!< Percentage tasks complete 0..100% */
	NUI_DOUBLE percent_remaining; 
           /*!< Percentage tasks remaining */
	NUI_CHAR   comment[80]; 
           /*!< Comment for callback from NUI_ETA_set_update_comment() */
	void      *user_data; 
           /*!< \c callback_data from NUI_ETA_set_update_callback() */
} NUI_ETA_INFO;

/**
 * \brief Sets a callback that is executed whenever NUI_ETA_update() is called.
 *
 * In applications with GUI, NUI_ETA_update() may be called by code that
 * may not have access to GUI elements. In that case, programmer can
 * install a callback function. In the loop where tasks are being
 * counted and measured, whenever NUI_ETA_update() is called, it will do
 * its internal updates, initialize NUI_ETA_INFO structure, call the
 * callback function with a pointer to NUI_ETA_INFO. 
 *
 * Declare callback function as:
 * \code
 * void programmer_eta_update_callback(NUI_ETA_INFO *info);
 * \endcode
 *
 * Example usage of function:
 * \code
 * ...
 * NUI_ETA_set_update_callback(eta, programmer_eta_update_callback, 
 *       (void *)gui_progress_bar);
 * ...
 * \endcode
 * 
 * Example implementation of callback:
 * \code
 * void
 * programmer_eta_update_callback(NUI_ETA_INFO *info)
 * {
 *      char message[1024], elapsed[256], remains[256];
 *      SOME_PROGRESS_BAR_OBJECT *pbar = NULL;
 *
 *      pbar = (SOME_PROGRESS_BAR_OBJECT *)info->user_data;
 *      sprintf(elapsed, "%s elapsed", 
 *          NUI_ETA_time_format_extended(eta, info->seconds_elapsed, 
 *          NULL, NULL, NULL));
 *      sprintf(remains, "%s remaining", 
 *          NUI_ETA_time_format_extended(eta, info->seconds_remaining, 
 *          NULL, NULL, NULL));
 *      sprintf(message, "Processing file %.0f of %.0f\n"
 *          "%s elapsed, %s remaining, %.1f%% complete",
 *          info->tasks_elapsed, info->total_tasks, elapsed, remains,
 *          info->percent_elapsed);
 *      pbar->someMethodThatUpdatesMessageBox(message);
 *      // ... check for Cancel being pressed or some such thing...
 * }
 * \endcode
 *
 * \note This function is called before any calls to NUI_ETA_update() and
 *       after NUI_ETA_begin().
 *
 * \param eta - (in) A non-NULL pointer to NUI_ETA object
 * \param callback - (in) Callback declared as 
 *                   void callback(NUI_ETA_INFO *info);
 * \param callback_data - (in) User data that will be passed to callback in
 *        \c NUI_ETA_INFO::user_data.
 * \sa NUI_ETA_begin(), NUI_ETA_update()
 */
NUI_SDK_API 
void    NUI_ETA_set_update_callback(NUI_ETA *eta, 
								void (*callback)(NUI_ETA_INFO *info), 
								void  *callback_data);

/**
 * \brief Sets a short comment (80 characters max) passed to update callback
 *
 * If a callback function was set using NUI_ETA_set_update_callback(), then
 * just before NUI_ETA_update(), call NUI_ETA_set_update_comment() to set a
 * very short message (no more than 80 characters). This message
 * is passed to the callback function via \c NUI_ETA_INFO::comment.
 *
 * \param eta - (in) A non-NULL pointer to NUI_ETA object
 * \param comment - (in) < 80 character comment forwarded to update callback
 * \sa NUI_ETA_update(), NUI_ETA_set_update_callback()
 */
NUI_SDK_API 
void    NUI_ETA_set_update_comment(NUI_ETA *eta, NUI_CHAR *comment);

/**
 * \brief Returns approximate elapsed time when NUI_ETA_update() was last called
 * \param eta - (in) A non-NULL pointer to valid NUI_ETA object
 * \param hh - (out) Location of elapsed hours
 * \param mm - (out) Location of elapsed minutes
 * \param ss - (out) Location of elapsed seconds
 * \returns Elapsed time in seconds as a double precision number
 *          to preserve milli-second granularity
 * \sa NUI_ETA_update()
 */
NUI_SDK_API 
NUI_DOUBLE NUI_ETA_get_time_elapsed(NUI_ETA   *eta, 
                                    NUI_INT32 *hh, 
                                    NUI_INT32 *mm, 
                                    NUI_INT32 *ss);

/**
 * \brief Returns approximate remaining time when NUI_ETA_update() was 
 *        last called
 * \param eta - (in) A non-NULL pointer to valid NUI_ETA object
 * \param hh - (out) Location of remaining hours; ignored if NULL.
 * \param mm - (out) Location of remaining minutes; ignored if NULL.
 * \param ss - (out) Location of remaining seconds; ignored if NULL.
 * \returns Remaining time in seconds as a double precision number
 *          to preserve milli-second granularity
 * \sa NUI_ETA_update()
 */
NUI_SDK_API 
NUI_DOUBLE NUI_ETA_get_time_remaining(NUI_ETA   *eta, 
                                      NUI_INT32 *hh, 
                                      NUI_INT32 *mm, 
                                      NUI_INT32 *ss);


/**
 * \brief Convenience function that resets NUI_ETA_time().
 *
 * \note
 * For all platforms other than MAC OS X, this function internally
 * uses the CPU-based timers provided below.
 *
 * \sa NUI_ETA_time()
 */
NUI_SDK_API 
void    NUI_ETA_time_reset(void);

/**
 * \brief Convenience function that returns seconds elapsed
 *        since the first call to itself or since most recent
 *        call to NUI_ETA_time_reset(). 
 *
 * First time NUI_ETA_time() is called in a program, the time counter
 * is reset and 0 is returned. Next time NUI_ETA_time() is called,
 * it returns time elapsed since the first call to it. If at any
 * point NUI_ETA_time_reset() is called, NUI_ETA_time() is reset to the
 * point where it was first called.
 *
 * \note
 * For all platforms other than MAC OS X, this function internally
 * uses the CPU-based timers provided below.
 *
 * \returns Seconds as a double precision number to preserve
 *          granularity in terms of milliseconds (Mac) or
 *          potentially nanoseconds (other platforms).
 * \sa NUI_ETA_time_reset()
 */
NUI_SDK_API 
NUI_DOUBLE  NUI_ETA_time(void); 

/**
 * \brief Formats \c seconds to an ASCII string of the form "HH:MM:SS"
 * \note Parameters \c hh, \c mm, \c ss are ignored if NULL. 
 *       Do not free return value. Copy it into a temporary string.
 * \param seconds - (in) Specifies seconds as a double
 * \param hh - (out) Location of hours extracted from \c seconds
 * \param mm - (out) Location of minutes extracted from \c seconds
 * \param ss - (out) Location of seconds extracted from \c seconds
 * \returns Character string that formats seconds to the form "HH:MM:SS"
 */
NUI_SDK_API 
NUI_CHAR   *NUI_ETA_time_format(NUI_DOUBLE seconds, 
			    NUI_INT32 *hh, NUI_INT32 *mm, NUI_INT32 *ss); 

/**
 * \brief Formats \c seconds to an ASCII string of the form 
 *        "HH hours MM minutes SS seconds"
 * \note Parameters \c hh, \c mm, \c ss are ignored if NULL. 
 *       Do not free return value. Copy it into a temporary string.
 * \param seconds - (in) Specifies seconds as a double
 * \param hh - (out) Location of hours extracted from \c seconds
 * \param mm - (out) Location of minutes extracted from \c seconds
 * \param ss - (out) Location of seconds extracted from \c seconds
 * \returns Character string that formats seconds to the form 
 *          "HH hours MM minutes SS seconds"
 */
NUI_SDK_API 
NUI_CHAR   *NUI_ETA_time_format_extended(NUI_DOUBLE seconds, 
			    NUI_INT32 *hh, NUI_INT32 *mm, NUI_INT32 *ss); 

/*************************************************************************
 * The following functions deal with CPU-based timers. These are platform-
 * specific but are highly accurate. At the time of writing this section 
 * of the library, the CPU-based timers on Windows have the granularity of
 * the CPU clock ticks; on IRIX and Linux, based on availablily, the
 * CPU-based timer resolution is about 800 microseconds to 1 nanosecond.
 * At present these functions are not available on MAC OS X platform
 * because this platform does not have a clean real-time clock interface
 * with sufficient granularity.
 *************************************************************************/
/**
 * \brief Returns the frequency of the CPU-based timer in Hertz.
 *
 * The CPU-based timer is platform specific.
 * - Windows has the ability to return the frequency of the CPU
 * - On Linux and IRIX, the CPU-based timer frequency is abtained
 *   from the most accurate timer available in the POSIX4 standard.
 *
 * \returns The CPU timer frequency in Hertz or -1 if there is
 *          no timer available.
 */
NUI_SDK_API
NUI_INT64   NUI_ETA_get_cpu_timer_frequency_in_hertz(void);

/**
 * \brief Returns the number of CPU-based timer ticks since startup.
 *
 * The term "startup" has different meaning on each platform.
 * - On Windows it means the number of CPU clock ticks since the
 *   computer was switched on. We have computed that on a 3GHz computer,
 *   this number will not reset for nearly a hundred years. This means,
 *   you need to keep your computer running for that many years and
 *   use this function to fill up the bytes of the counter.
 * - On Linux and IRIX, this number is the number of high resolution
 *   CPU-timer ticks since an arbitrary reference point which could be
 *   the time when the thread calling this function started, or the
 *   time when the process calling this function started, or the time
 *   since the computer started, or any arbitrary system reference.
 *
 * \returns The number of CPU-based timer ticks since startup or -1
 *          if a CPU-based timer is not available.
 */
NUI_SDK_API
NUI_INT64   NUI_ETA_get_cpu_timer_ticks_since_startup(void);

/**
 * \brief Returns the number of seconds elapsed since CPU-based timer startup.
 *
 * This function uses NUI_ETA_get_cpu_timer_ticks_since_startup() to determine
 * seconds elapsed. The constraints remain the same.
 *
 * \returns The number of seconds elapsed since CPU-based timer startup or -1
 *          if a CPU-based timer is not available.
 */
NUI_SDK_API
NUI_DOUBLE  NUI_ETA_get_seconds_elapsed_since_timer_startup(void);

/**
 * \brief Resets time counter for the NUI_ETA_time_using_cpu_timer() function.
 *
 * Use it carefully in a multi-threaded environment as it uses static 
 * variables internally.
 *
 * \sa NUI_ETA_time_using_cpu_timer()
 */
NUI_SDK_API
void        NUI_ETA_reset_time_using_cpu_timer(void);

/**
 * \brief Returns the time elapsed (in seconds) since the first call to
 *        NUI_ETA_time_using_cpu_timer() in your application.
 *
 * This function may be used to accurately measure time elapsed for tasks
 * that take less than a second. If you call this function for the first time
 * in your application, it returns 0. All subsequent calls to this function
 * return the seconds elapsed since the first time the function was called.
 *
 * Use it carefully in a multi-threaded environment as it uses static 
 * variables internally.
 *
 * If at any point you call NUI_ETA_reset_time_using_cpu_timer(), the
 * "time since NUI_ETA_time_using_cpu_timer() was first called" is
 * reset.
 *
 * \returns The time elapsed (in seconds) since the first call to
 *        NUI_ETA_time_using_cpu_timer() in your application or -1
 *        if timer is not available.
 * \sa NUI_ETA_reset_time_using_cpu_timer()
 */
NUI_SDK_API
NUI_DOUBLE  NUI_ETA_time_using_cpu_timer(void);

#ifdef __cplusplus
}
#endif

#endif /* _NUI_ETA_H_HAS_BEEN_INCLUDED_ */

/*
 * nui_eta.h ends
 */
