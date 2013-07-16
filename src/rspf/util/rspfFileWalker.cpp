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
// Description:
//
// Utility class to walk through directories and get a list of files to
// process.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/util/rspfFileWalker.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/parallel/rspfJobQueue.h>
#include <OpenThreads/Thread>

static rspfTrace traceDebug(rspfString("rspfFileWalker:debug"));

rspfFileWalker::rspfFileWalker()
   : m_processFileCallBackPtr(0),
     m_jobQueue(new rspfJobMultiThreadQueue(new rspfJobQueue(), 1)),     
     m_filteredExtensions(0),
     m_recurseFlag(true),
     m_waitOnDirFlag(false),
     m_abortFlag(false),
     m_mutex()
{
}

rspfFileWalker::~rspfFileWalker()
{
   m_jobQueue = 0; // Not a leak, ref pointer.
}

void rspfFileWalker::walk(const std::vector<rspfFilename>& files)
{
   static const char M[] = "rspfFileWalker::walk";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered\n";
   }

   if ( files.size() )
   {
      std::vector<rspfFilename>::const_iterator i = files.begin();
      while ( i != files.end() )
      {
         // Must have call back set at this point.
         if ( !m_abortFlag && m_processFileCallBackPtr )
         {
            rspfFilename file = (*i).expand();
            if ( file.size() && file.exists() )
            {
               if ( file.isDir() ) // Directory:
               {
                  walkDir(file);
               }  
               else // File:
               {
                  if ( isFiltered(file) == false )
                  {
                     if(traceDebug())
                     {
                        rspfNotify(rspfNotifyLevel_DEBUG)
                           << "Making the job for: " << (*i) << std::endl;
                     }
                     
                     // Make the job:
                     rspfRefPtr<rspfFileWalkerJob> job =
                        new rspfFileWalkerJob( m_processFileCallBackPtr, file );
                     
                     job->setName( rspfString( file.string() ) );
                     
                     job->setCallback( new rspfFileWalkerJobCallback() );
                     
                     // Set the state to ready:
                     job->ready();
                     
                     // Add job to the queue:
                     m_jobQueue->getJobQueue()->add( job.get() );
                     
                     m_mutex.lock();
                     if ( m_abortFlag )
                     {
                        // Clear out the queue.
                        m_jobQueue->getJobQueue()->clear();
                        
                        break; // Callee set our abort flag so break out of loop.
                     }
                     m_mutex.unlock();
                  }
               }
            }
         }

         ++i;
      
      } // while ( i != files.end() )

      // FOREVER loop until all jobs are completed.
      while (1)
      {
         if ( OpenThreads::Thread::microSleep(250) == 0 )
         {
            if ( m_jobQueue->hasJobsToProcess() == false )
            {
               break;
            }
         }
      }

   } // if ( files.size() )

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exiting...\n";
   }  
}

void rspfFileWalker::walk(const rspfFilename& root)
{
   static const char M[] = "rspfFileWalker::walk";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " entered root=" << root << "\n";
   }

   // Must have call back set at this point.
   if ( !m_abortFlag && m_processFileCallBackPtr )
   {
      rspfFilename rootFile = root.expand();
      if ( rootFile.size() && rootFile.exists() )
      {
         if ( rootFile.isDir() )
         {
            walkDir(rootFile);

            // FOREVER loop until all jobs are completed.
            while (1)
            {
               if ( OpenThreads::Thread::microSleep(250) == 0 )
               {
                  if ( m_jobQueue->hasJobsToProcess() == false )
                  {
                     break;
                  }
               }
            }
         }
         else
         {
            // Single file no job queue needed.
            if ( isFiltered(rootFile) == false )
            {
               m_processFileCallBackPtr->operator()(rootFile);
            }
         }
      }
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exiting...\n";
   }  
}

void rspfFileWalker::walkDir(const rspfFilename& dir)
{
   static const char M[] = "rspfFileWalker::walkDir";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n" << "processing dir: " << dir << "\n";
   }

   // List of directories in this directory...
   std::vector<rspfFilename> dirs;
   
   // List of files in this directory...
   std::vector<rspfFilename> files;

   m_mutex.lock();
   rspfDirectory d;
   bool rspfDirectoryStatus = d.open(dir);
   m_mutex.unlock();

   if ( rspfDirectoryStatus )
   {
      // Loop to get the list of files and directories in this directory.
      m_mutex.lock();
      rspfFilename f;
      bool valid_file = d.getFirst(f);
      while ( valid_file )
      {
         if ( isFiltered(f) == false )
         {
            if (f.isDir())
            {
               dirs.push_back(f);
            }
            else
            {
               files.push_back(f);
            }
         }
         valid_file = d.getNext(f);
      }
      m_mutex.unlock();

      //---
      // Process files first before recursing directories.  If a file is a directory base image,
      // e.g. RPF, then the callee should call rspfFileWalker::setRecurseFlag to false to
      // stop us from going into sub directories.
      //---
      std::vector<rspfFilename>::const_iterator i = files.begin();
      while (i != files.end())
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "Making the job for: " << (*i) << std::endl;
         }
         
         // Make the job:
         rspfRefPtr<rspfFileWalkerJob> job =
            new rspfFileWalkerJob( m_processFileCallBackPtr, (*i) );

         job->setName( rspfString( (*i).string() ) );

         job->setCallback( new rspfFileWalkerJobCallback() );

         // Set the state to ready:
         job->ready();

         // Add job to the queue:
         m_jobQueue->getJobQueue()->add( job.get() );

         m_mutex.lock();
         if ( m_abortFlag )
         {
            // Clear out the queue.
            m_jobQueue->getJobQueue()->clear();
            
            break; // Callee set our abort flag so break out of loop.
         }
         m_mutex.unlock();

         ++i;
      }

      if ( m_waitOnDirFlag )
      {
         // FOREVER loop until all jobs are completed.
         while (1)
         {
            if ( OpenThreads::Thread::microSleep(250) == 0 )
            {
               if ( m_jobQueue->hasJobsToProcess() == false )
               {
                  break;
               }
            }
         }
      }

      m_mutex.lock();
      if ( !m_abortFlag && m_recurseFlag )
      {
         // Process sub directories...
         i = dirs.begin();
         while (i != dirs.end())
         {
            m_mutex.unlock();
            walkDir( (*i) );
            m_mutex.lock();

            if ( m_abortFlag )
            {
               break; // Callee set our abort flag so break out of loop.
            }
            ++i;
         }
      }
      m_mutex.unlock();
      
   } // if ( rspfDirectoryOpenStatus )

   // Reset the m_recurseFlag.
   m_mutex.lock();
   m_recurseFlag = true;
   m_mutex.unlock();
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

bool rspfFileWalker::isFiltered(const rspfFilename& file) const
{
   bool result = false;
   if ( file.size() )
   {
      if ( isDotFile(file) )
      {
         result = true;
      }
      else if ( file[file.size()-1] == '~' )
      {
         result = true;
      }
      else 
      {
         std::string ext = file.ext().downcase().c_str();
         if ( ext.size() )
         {
            std::vector<std::string>::const_iterator i = m_filteredExtensions.begin();
            while ( i != m_filteredExtensions.end() )
            {
               if ( ext == (*i) )
               {
                  result = true;
                  break;
               }
               ++i;
            }
         }
      }
   }
#if 0 /* Please leave for debug. (drb) */
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfFileWalker::isFiltered file " << (result?"filtered: ":"not filtered: ")
         << file << "\n";
   }
#endif
   
   return result;
}

bool rspfFileWalker::isDotFile(const rspfFilename& f) const
{
   bool result = false;
   
   // Get the size in bytes.
   if ( f.size() )
   {
      std::string::size_type firstDotPos = f.find('.');
      if ( firstDotPos == 0 )
      {
         result = true;
      }
      else if ( firstDotPos != std::string::npos ) // Dot in file.
      {
         // Get the position of first dot from the end.
         std::string::size_type lastDotPos = f.find_last_of('.');
         if ( lastDotPos != std::string::npos )
         {
            // Make copy.
            rspfFilename f2 = f;
            
            // Convert an '\'s to '/'s. 
            f2.convertBackToForwardSlashes();
            
            // Find the first slash from end.
            std::string::size_type lastSlashPos = f2.find_last_of('/');
            
            if (lastSlashPos != std::string::npos) // Found a slash.
            {
               if ( (lastSlashPos+1) == lastDotPos )
               {
                  // dot in front of slash like /home/foo/.xemacs
                  result = true;
               }
            }
         }
      }
   }

   return result;
}

const std::vector<std::string>& rspfFileWalker::getFilteredExtensions() const
{
   return m_filteredExtensions;
}

std::vector<std::string>& rspfFileWalker::getFilteredExtensions()
{
   return m_filteredExtensions;
}

void rspfFileWalker::initializeDefaultFilterList()
{
   m_mutex.lock();
   
   // Common extensions to filter out, most common first.
   m_filteredExtensions.push_back(std::string("ovr"));
   m_filteredExtensions.push_back(std::string("omd"));
   m_filteredExtensions.push_back(std::string("his"));
   m_filteredExtensions.push_back(std::string("geom"));
   
   // The rest alphabetical.
   m_filteredExtensions.push_back(std::string("aux"));
   m_filteredExtensions.push_back(std::string("bin"));
   m_filteredExtensions.push_back(std::string("dbf"));
   m_filteredExtensions.push_back(std::string("hdr"));
   m_filteredExtensions.push_back(std::string("jpw"));
   m_filteredExtensions.push_back(std::string("kwl"));
   m_filteredExtensions.push_back(std::string("log"));
   m_filteredExtensions.push_back(std::string("out"));
   m_filteredExtensions.push_back(std::string("prj"));
   m_filteredExtensions.push_back(std::string("save"));
   m_filteredExtensions.push_back(std::string("sdw"));
   m_filteredExtensions.push_back(std::string("shx"));
   m_filteredExtensions.push_back(std::string("spec"));
   m_filteredExtensions.push_back(std::string("statistics"));
   m_filteredExtensions.push_back(std::string("tfw"));
   m_filteredExtensions.push_back(std::string("tmp"));
   m_filteredExtensions.push_back(std::string("txt"));

   m_mutex.unlock();
}

void rspfFileWalker::setRecurseFlag(bool flag)
{
   m_mutex.lock();
   m_recurseFlag = flag;
   m_mutex.unlock();
}

void rspfFileWalker::setWaitOnDirFlag(bool flag)
{
   m_mutex.lock();
   m_waitOnDirFlag = flag;
   m_mutex.unlock();
}

void rspfFileWalker::setAbortFlag(bool flag)
{
   m_mutex.lock();
   m_abortFlag = flag;
   m_mutex.unlock();
}

void rspfFileWalker::setNumberOfThreads(rspf_uint32 nThreads)
{
   m_mutex.lock();
   m_jobQueue->setNumberOfThreads(nThreads);
   m_mutex.unlock();
}

void rspfFileWalker::registerProcessFileCallback(
   rspfCallback1<const rspfFilename&>* cb)
{
   m_mutex.lock();
   m_processFileCallBackPtr = cb;
   m_mutex.unlock();
}

rspfFileWalker::rspfFileWalkerJob::rspfFileWalkerJob(
   rspfCallback1<const rspfFilename&>* cb,
   const rspfFilename& file)
   : m_processFileCallBackPtr(cb),
     m_file(file)
{
}

void rspfFileWalker::rspfFileWalkerJob::start()
{
   if ( m_processFileCallBackPtr && m_file.size() )
   {
      m_processFileCallBackPtr->operator()(m_file);
   }
}

rspfFileWalker::rspfFileWalkerJobCallback::rspfFileWalkerJobCallback()
   : rspfJobCallback()
{
}

void rspfFileWalker::rspfFileWalkerJobCallback::started(rspfJob* job)
{
   rspfJobCallback::started(job);
}

void rspfFileWalker::rspfFileWalkerJobCallback::finished(rspfJob* job)
{
   rspfJobCallback::finished(job);
}

void rspfFileWalker::rspfFileWalkerJobCallback::canceled(rspfJob* job)
{
   rspfJobCallback::canceled(job);
}
