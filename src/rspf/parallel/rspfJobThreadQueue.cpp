#include<rspf/parallel/rspfJobThreadQueue.h>

rspfJobThreadQueue::rspfJobThreadQueue(rspfJobQueue* jqueue)
:m_doneFlag(false)
{
   setJobQueue(jqueue);    
}
void rspfJobThreadQueue::setJobQueue(rspfJobQueue* jqueue)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   
   if (m_jobQueue == jqueue) return;
   
   if(isRunning())
   {
      rspfRefPtr<rspfJobQueue> jobQueueTemp = m_jobQueue;
      m_jobQueue = jqueue;
      if(jobQueueTemp.valid())
      {
         jobQueueTemp->releaseBlock();
      }
   }
   else 
   {
      m_jobQueue = jqueue;
   }
   
   startThreadForQueue();
}

rspfJobQueue* rspfJobThreadQueue::getJobQueue() 
{ 
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_jobQueue.get(); 
}

const rspfJobQueue* rspfJobThreadQueue::getJobQueue() const 
{ 
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_jobQueue.get(); 
}

rspfRefPtr<rspfJob> rspfJobThreadQueue::currentJob() 
{ 
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_currentJob; 
}

void rspfJobThreadQueue::cancelCurrentJob()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   if(m_currentJob.valid())
   {
      m_currentJob->cancel();
   }
}
bool rspfJobThreadQueue::isValidQueue()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_jobQueue.valid();
}

void rspfJobThreadQueue::run()
{
   bool firstTime = true;
   bool validQueue = true;
   rspfRefPtr<rspfJob> job;
   do
   {
      // osg::notify(osg::NOTICE)<<"In thread loop "<<this<<std::endl;
      validQueue = isValidQueue();
      job = nextJob();
      if (job.valid()&&!m_doneFlag)
      {
         {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
            m_currentJob = job;
         }
         
         // if the job is ready to execute
         if(job->isReady())
         {
            job->resetState(rspfJob::rspfJob_RUNNING);
            job->start();
         }
         {            
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
            m_currentJob = 0;
         }
         job->setState(rspfJob::rspfJob_FINISHED);
         job = 0;
      }
      
      if (firstTime)
      {
         // do a yield to get round a peculiar thread hang when testCancel() is called 
         // in certain cirumstances - of which there is no particular pattern.
         YieldCurrentThread();
         firstTime = false;
      }
   } while (!m_doneFlag&&validQueue);
   
   if(job.valid()&&m_doneFlag&&job->isReady())
   {
      {            
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
         m_currentJob = 0;
      }
      job->cancel();
   }
   {            
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
      m_currentJob = 0;
   }
   job = 0;
}

void rspfJobThreadQueue::setDone(bool done)
{
   m_threadMutex.lock();
   if (m_doneFlag==done)
   {
      m_threadMutex.unlock();
      return;
   }
   m_doneFlag = done;
   m_threadMutex.unlock();
   if(done)
   {
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
         if (m_currentJob.valid())
            m_currentJob->release();
      }
      
      if (m_jobQueue.valid())
         m_jobQueue->releaseBlock();
   }
}

bool rspfJobThreadQueue::isDone() const 
{ 
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_doneFlag; 
}

bool rspfJobThreadQueue::isProcessingJob()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_currentJob.valid();
}

int rspfJobThreadQueue::cancel()
{
   
   if( isRunning() )
   {
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
         m_doneFlag = true;
         if (m_currentJob.valid())
         {
            m_currentJob->cancel();
         }
         
         if (m_jobQueue.valid()) 
         {
            m_jobQueue->releaseBlock();
         }
      }
      
      // then wait for the the thread to stop running.
      while(isRunning())
      {
#if 1
         {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
            
            if (m_jobQueue.valid()) 
            {
               m_jobQueue->releaseBlock();
            }
         }
#endif
         OpenThreads::Thread::YieldCurrentThread();
      }
   }
   return OpenThreads::Thread::cancel();
}

bool rspfJobThreadQueue::isEmpty()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
   return m_jobQueue->isEmpty();
}

rspfJobThreadQueue::~rspfJobThreadQueue()
{
   cancel();
}

void rspfJobThreadQueue::startThreadForQueue()
{
   if(m_jobQueue.valid())
   {
      if(!isRunning())
      {
         start();
         while(!isRunning()) // wait for the thread to start running
         {
            OpenThreads::Thread::YieldCurrentThread();
         }
      }
   }
}

bool rspfJobThreadQueue::hasJobsToProcess()const
{
   bool result = false;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_threadMutex);
      result = !m_jobQueue->isEmpty()||m_currentJob.valid();
   }
   
   return result;
}

rspfRefPtr<rspfJob> rspfJobThreadQueue::nextJob()
{
   rspfRefPtr<rspfJob> job;
   m_threadMutex.lock();
   rspfRefPtr<rspfJobQueue> jobQueue = m_jobQueue;
   bool checkIfValid = !m_doneFlag&&jobQueue.valid();
   m_threadMutex.unlock();
   if(checkIfValid)
   {
      return jobQueue->nextJob(true);
   }
   return 0;
}
