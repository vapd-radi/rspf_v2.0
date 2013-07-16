#ifndef rspfJobThreadQueue_HEADER
#define rspfJobThreadQueue_HEADER
#include <rspf/parallel/rspfJobQueue.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/Thread>

class RSPF_DLL rspfJobThreadQueue : public rspfReferenced, 
                                      public OpenThreads::Thread
{
public:
   rspfJobThreadQueue(rspfJobQueue* jqueue=0);
   void setJobQueue(rspfJobQueue* jqueue);
   
   rspfJobQueue* getJobQueue();
   
   const rspfJobQueue* getJobQueue() const; 
   
   rspfRefPtr<rspfJob> currentJob();
   
   void cancelCurrentJob();
   bool isValidQueue()const;
   
   virtual void run();
   
   void setDone(bool done);
   
   bool isDone()const;
   virtual int cancel();
   bool isEmpty()const;
   
   bool isProcessingJob()const;
   
   bool hasJobsToProcess()const;
   
protected:
   virtual ~rspfJobThreadQueue();
   
   void startThreadForQueue();
   virtual rspfRefPtr<rspfJob> nextJob();
   
   bool                       m_doneFlag;
   mutable OpenThreads::Mutex m_threadMutex;
   rspfRefPtr<rspfJobQueue> m_jobQueue;
   rspfRefPtr<rspfJob>      m_currentJob;
   
};

#endif
