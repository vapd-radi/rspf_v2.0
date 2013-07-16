#include <rspf/parallel/rspfJobQueue.h>

#include <algorithm> /* for std::find */


rspfJobQueue::rspfJobQueue()
{
}

void rspfJobQueue::add(rspfJob* job, bool guaranteeUniqueFlag)
{
   rspfRefPtr<Callback> cb;
   {
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
         
         if(guaranteeUniqueFlag)
         {
            if(findByPointer(job) != m_jobQueue.end())
            {
               m_block.set(true);
               return;
            }
         }
         cb = m_callback.get();
      }
      if(cb.valid()) cb->adding(this, job);
      
      job->ready();
      m_jobQueueMutex.lock();
      m_jobQueue.push_back(job);
      m_jobQueueMutex.unlock();
   }
   if(cb.valid())
   {
      cb->added(this, job);
   }
   m_block.set(true);
}

rspfRefPtr<rspfJob> rspfJobQueue::removeByName(const rspfString& name)
{
   rspfRefPtr<rspfJob> result;
   rspfRefPtr<Callback> cb;
   if(name.empty()) return result;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
      rspfJob::List::iterator iter = findByName(name);
      if(iter!=m_jobQueue.end())
      {
         result = *iter;
         m_jobQueue.erase(iter);
      }
      cb = m_callback.get();
   }      
   m_block.set(!m_jobQueue.empty());
   
   if(cb.valid()&&result.valid())
   {
      cb->removed(this, result.get());
   }
   return result;
}
rspfRefPtr<rspfJob> rspfJobQueue::removeById(const rspfString& id)
{
   rspfRefPtr<rspfJob> result;
   rspfRefPtr<Callback> cb;
   if(id.empty()) return result;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
      rspfJob::List::iterator iter = findById(id);
      if(iter!=m_jobQueue.end())
      {
         result = *iter;
         m_jobQueue.erase(iter);
      }
      cb = m_callback.get();
      m_block.set(!m_jobQueue.empty());
   }
   if(cb.valid()&&result.valid())
   {
      cb->removed(this, result.get());
   }
   return result;
}

void rspfJobQueue::remove(const rspfJob* Job)
{
   rspfRefPtr<rspfJob> removedJob;
   rspfRefPtr<Callback> cb;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
      rspfJob::List::iterator iter = std::find(m_jobQueue.begin(), m_jobQueue.end(), Job);
      if(iter!=m_jobQueue.end())
      {
         removedJob = (*iter);
         m_jobQueue.erase(iter);
      }
      cb = m_callback.get();
   }
   if(cb.valid()&&removedJob.valid())
   {
      cb->removed(this, removedJob.get());
   }
}

void rspfJobQueue::removeStoppedJobs()
{
   rspfJob::List removedJobs;
   rspfRefPtr<Callback> cb;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
      cb = m_callback.get();
      rspfJob::List::iterator iter = m_jobQueue.begin();
      while(iter!=m_jobQueue.end())
      {
         if((*iter)->isStopped())
         {
            removedJobs.push_back(*iter);
            iter = m_jobQueue.erase(iter);
         }
         else 
         {
            ++iter;
         }
      }
   }
   if(!removedJobs.empty())
   {
      if(cb.valid())
      {
         rspfJob::List::iterator iter = removedJobs.begin();
         while(iter!=removedJobs.end())
         {
            cb->removed(this, (*iter).get());
            ++iter;
         }
      }
      removedJobs.clear();
   }
}

void rspfJobQueue::clear()
{
   rspfJob::List removedJobs(m_jobQueue);
   rspfRefPtr<Callback> cb;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
      m_jobQueue.clear();
      cb = m_callback.get();
   }
   if(cb.valid())
   {
      // rspf_uint32 idx = 0;
      for(rspfJob::List::iterator iter=removedJobs.begin();iter!=removedJobs.end();++iter)
      {
         cb->removed(this, (*iter).get());
      }
   }
}

rspfRefPtr<rspfJob> rspfJobQueue::nextJob(bool blockIfEmptyFlag)
{
   m_jobQueueMutex.lock();
   bool emptyFlag = m_jobQueue.empty();
   m_jobQueueMutex.unlock();
   if (blockIfEmptyFlag && emptyFlag)
   {
      m_block.block();
   }
   
   rspfRefPtr<rspfJob> result;
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
   
   if (m_jobQueue.empty())
   {
      m_block.set(false);
      return result;
   }
   
   rspfJob::List::iterator iter= m_jobQueue.begin();
   while((iter != m_jobQueue.end())&&
         (((*iter)->isCanceled())))
   {
      (*iter)->finished(); // mark the ob as being finished 
      iter = m_jobQueue.erase(iter);
   }
   if(iter != m_jobQueue.end())
   {
      result = *iter;
      m_jobQueue.erase(iter);
   }
   m_block.set(!m_jobQueue.empty());
   return result;
}
void rspfJobQueue::releaseBlock()
{
   m_block.release();
}
bool rspfJobQueue::isEmpty()const
{
   // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
   // return m_jobQueue.empty();
   m_jobQueueMutex.lock();
   bool result =  m_jobQueue.empty();
   m_jobQueueMutex.unlock();
   return result;
}

rspf_uint32 rspfJobQueue::size()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
   return (rspf_uint32) m_jobQueue.size();
}

rspfJob::List::iterator rspfJobQueue::findById(const rspfString& id)
{
   if(id.empty()) return m_jobQueue.end();
   rspfJob::List::iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if(id == (*iter)->id())
      {
         return iter;
      }
      ++iter;
   }  
   return m_jobQueue.end();
}

rspfJob::List::iterator rspfJobQueue::findByName(const rspfString& name)
{
   if(name.empty()) return m_jobQueue.end();
   rspfJob::List::iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if(name == (*iter)->name())
      {
         return iter;
      }
      ++iter;
   }  
   return m_jobQueue.end();
}

rspfJob::List::iterator rspfJobQueue::findByPointer(const rspfJob* job)
{
   return std::find(m_jobQueue.begin(),
                    m_jobQueue.end(),
                    job);
}

rspfJob::List::iterator rspfJobQueue::findByNameOrPointer(const rspfJob* job)
{
   rspfString n = job->name();
   rspfJob::List::iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if((*iter).get() == job)
      {
         return iter;
      }
      else if((!n.empty())&&
              (job->name() == (*iter)->name()))
      {
         return iter;
      }
      ++iter;
   }  
   
   return m_jobQueue.end();
}

bool rspfJobQueue::hasJob(rspfJob* job)
{
   rspfJob::List::const_iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if(job == (*iter).get())
      {
         return true;
      }
      ++iter;
   }
   
   return false;
}

void rspfJobQueue::setCallback(Callback* c)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
   m_callback = c;
}

rspfJobQueue::Callback* rspfJobQueue::callback()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobQueueMutex);
   return m_callback.get();
}
