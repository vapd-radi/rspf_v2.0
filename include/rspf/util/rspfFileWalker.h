//----------------------------------------------------------------------------
//
// File: rspfFileWalker.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  See description for class below.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfFileWalker_HEADER
#define rspfFileWalker_HEADER 1

#include <rspf/base/rspfCallback1.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/parallel/rspfJob.h>
#include <rspf/parallel/rspfJobMultiThreadQueue.h>
#include <OpenThreads/Mutex>
#include <string>
#include <vector>

class rspfFilename;

/**
 * @class rspfFileWalker
 *
 * Utility class to walk through directories and get a list of files to
 * process. For each file found the callback is excecuted.  Mechanisms are in
 * place to filter files and to call back  to a processFile method.  This
 * requires a callback method to process files from the user.  The callback is
 * set via registerProcessFileCallback method.  Internally the processFile
 * calls are placed in a job queue.
 *
 * Typical usage (snip from rspfTiledElevationDatabase):
 *
 * rspfFileWalker* fw = new rspfFileWalker();
 * fw->initializeDefaultFilterList();
 * rspfCallback1<const rspfFilename&, bool&, bool>* cb =
 * new ProcessFileCB(this, &rspfTiledElevationDatabase::processFile);
 * fw->registerProcessFileCallback(cb);
 * fw->walk(f);
 */
class RSPF_DLL rspfFileWalker
{
public:
   
   /** default constructor */
   rspfFileWalker();

   /** destructor */
   ~rspfFileWalker();

   /**
    * @brief Takes an array of files.
    *
    * For each file in array:  If files is a directory, it will walk it. Files
    * found in walk or files(not directories) in the array will be processed
    * via a job queue.
    *
    * Files are filter prior to the callback execution.  The filtering is to
    * eliminate sending unwanted files to the callback.  There is a default
    * filter table.  This can be edited by calling the non-const
    * getFilteredExtensions method.
    *
    * Each callback is placed in a threaded job queue.  So users should ensure
    * their callback is thread safe.
    */
   void walk(const std::vector<rspfFilename>& files);

   /**
    * @brief This will walk "root" and execute a callback for each file found.
    *
    * Files are filter prior to the callback execution.  The filtering is to
    * eliminate sending unwanted files to the callback.  There is a default
    * filter table.  This can be edited by calling the non-const
    * getFilteredExtensions method.
    *
    * Each callback is placed in a threaded job queue.  So users should ensure
    * their callback is thread safe.
    */
   void walk(const rspfFilename& root);

   // void walk(const rspfFilename& root);

   /**
    * @brief Registers callback method to process a file.
    *
    * @param cb Callback to register.
    *
    * @see m_processFileCallBackPtr for documentation on template interface.
    */   
   void registerProcessFileCallback(rspfCallback1<const rspfFilename&>* cb);

   /** @return The list of filtered out files. */
   const std::vector<std::string>& getFilteredExtensions() const;

   /**
    * @brief Non const method to allow access for
    * adding or deleting extensions from the list.
    *
    * The list is used by the private isFiltered method to avoid trying to
    * process unwanted files.
    */
   std::vector<std::string>& getFilteredExtensions();

   /**
    * @brief Initializes the filter list with a default set of filtered out
    * file names.
    */
   void initializeDefaultFilterList();

   /**
    * @brief Sets recurse flag.
    *
    * If set to true this stops recursing of the
    * current directory.  Defaulted to true in the constructor.
    * Typically used to indicate the directory being processed holds a
    * directory based image, e.g. RPF data.
    *
    * @param flag True to recurse, false to stop recursion of current
    * directory.
    */
   void setRecurseFlag(bool flag);

   /**
    * @brief Sets waitOnDir flag.
    *
    * If set to true each directory is processed in entirety before sub
    * directories are recursed.  This allows callers of setRecurseFlag
    * to disable directory walking.  If your code is calling setRecurseFlag
    * this flag should be set to true.
    *
    * @param flag true to wait, false to not wait. Defaulted to false in the constructor. 
    */   
   void setWaitOnDirFlag(bool flag);

   /**
    * @brief If set to true this stops files walking (aborts).
    * @param flag True to abort current "walk".
    */
   void setAbortFlag(bool flag);

   /** @brief Sets the max number of threads(jobs) to run at one time. */
   void setNumberOfThreads(rspf_uint32 nThreads);
   
private:

   /** @brief Private rspfJob class. */
   class rspfFileWalkerJob : public rspfJob
   {
   public:
      /**
       * @brief Constructor that takes callback and file.
       * @param cb Callback to method to process a file.
       * @param file The file to process.
       */
      rspfFileWalkerJob(rspfCallback1<const rspfFilename&>* cb,
                         const rspfFilename& file);
      /**
       * @brief Defines pure virtual rspfJob::start.
       *
       * This executes the call to m_processFileCallBackPtr.
       */
      virtual void start();
      
   private:
      rspfCallback1<const rspfFilename&>* m_processFileCallBackPtr;
      rspfFilename                         m_file;
      
   }; // End: class rspfFileWalkerJob

   /** @brief Private rspfJobCallback class. */
   class rspfFileWalkerJobCallback : public rspfJobCallback
   {
   public:
      rspfFileWalkerJobCallback();
      virtual void started(rspfJob* job);
      virtual void finished(rspfJob* job);
      virtual void canceled(rspfJob* job);
   };

   /**
    * @brief Processes files in directory.
    *
    * If a file in the directory is itself a directory this will do a recursive
    * call to itself.  Individual files are processed in a job queue...
    */
   void walkDir(const rspfFilename& dir);
   
   /**
    * @brief Convenience method for file walker code to check file to see is
    * it should be processed.
    *
    * @param f File to check.
    * 
    * @return true f is stagagble, false if not.
    */
   bool isFiltered(const rspfFilename& f) const;
   
   /**
    * @brief isDotFile method.
    * @param f File/directory to check.
    * @return true if file is a dot file.
    */   
   bool isDotFile(const rspfFilename& f) const;
   
   /**
    * @brief Callback to method to process a file.
    *
    * @param const rspfFilename& First parameter(argument) file to process.
    */
   rspfCallback1<const rspfFilename&>* m_processFileCallBackPtr;
   rspfRefPtr<rspfJobMultiThreadQueue> m_jobQueue;
   std::vector<std::string>              m_filteredExtensions;
   bool                                  m_recurseFlag;
   bool                                  m_waitOnDirFlag;
   bool                                  m_abortFlag;
   OpenThreads::Mutex                    m_mutex;
};

#endif /* #ifndef rspfFileWalker_HEADER */
