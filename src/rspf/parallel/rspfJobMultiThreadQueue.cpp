#include <rspf/parallel/rspfJobMultiThreadQueue.h>

rspfJobMultiThreadQueue::rspfJobMultiThreadQueue(rspfJobQueue* q, rspf_uint32 nThreads)
:m_jobQueue(q?q:new rspfJobQueue())
{
   setNumberOfThreads(nThreads);
}
rspfJobQueue* rspfJobMultiThreadQueue::getJobQueue()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return m_jobQueue.get();
}
const rspfJobQueue* rspfJobMultiThreadQueue::getJobQueue()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return m_jobQueue.get();
}
void rspfJobMultiThreadQueue::setQueue(rspfJobQueue* q)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   rspf_uint32 idx = 0;
   m_jobQueue = q;
   for(idx = 0; idx < m_threadQueueList.size(); ++idx)
   {
      m_threadQueueList[idx]->setJobQueue(m_jobQueue.get());
   }
}
void rspfJobMultiThreadQueue::setNumberOfThreads(rspf_uint32 nThreads)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   rspf_uint32 idx = 0;
   rspf_uint32 queueSize = m_threadQueueList.size();
   
   if(nThreads > queueSize)
   {
      for(idx = queueSize; idx < nThreads;++idx)
      {
         rspfRefPtr<rspfJobThreadQueue> threadQueue = new rspfJobThreadQueue();
         threadQueue->setJobQueue(m_jobQueue.get());
         m_threadQueueList.push_back(threadQueue);
      }
   }
   else if(nThreads < queueSize)
   {
      ThreadQueueList::iterator iter = m_threadQueueList.begin()+nThreads;
      while(iter != m_threadQueueList.end())
      {
         (*iter)->cancel();
         iter = m_threadQueueList.erase(iter);
      }
   }
}

rspf_uint32 rspfJobMultiThreadQueue::getNumberOfThreads() const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return static_cast<rspf_uint32>( m_threadQueueList.size() );
}

rspf_uint32 rspfJobMultiThreadQueue::numberOfBusyThreads()const
{
   rspf_uint32 result = 0;
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   rspf_uint32 idx = 0;
   rspf_uint32 queueSize = m_threadQueueList.size();
   for(idx = 0; idx < queueSize;++idx)
   {
      if(m_threadQueueList[idx]->isProcessingJob()) ++result;
   }
   return result;
}

bool rspfJobMultiThreadQueue::areAllThreadsBusy()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   rspf_uint32 idx = 0;
   rspf_uint32 queueSize = m_threadQueueList.size();
   for(idx = 0; idx < queueSize;++idx)
   {
      if(!m_threadQueueList[idx]->isProcessingJob()) return false;
   }
   
   return true;
}

bool rspfJobMultiThreadQueue::hasJobsToProcess()const
{
   bool result = false;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
      rspf_uint32 queueSize = m_threadQueueList.size();
      rspf_uint32 idx = 0;
      for(idx = 0; ((idx<queueSize)&&!result);++idx)
      {
         result = m_threadQueueList[idx]->hasJobsToProcess();
      }
   }
   
   return result;
}

