#ifndef rspfJobMultiThreadQueue_HEADER
#define rspfJobMultiThreadQueue_HEADER
#include <rspf/parallel/rspfJobThreadQueue.h>

class RSPF_DLL rspfJobMultiThreadQueue : public rspfReferenced
{
public:
   typedef std::vector<rspfRefPtr<rspfJobThreadQueue> > ThreadQueueList;
   
   rspfJobMultiThreadQueue(rspfJobQueue* q=0, rspf_uint32 nThreads=0);
   rspfJobQueue* getJobQueue();
   const rspfJobQueue* getJobQueue()const;
   void setQueue(rspfJobQueue* q);
   void setNumberOfThreads(rspf_uint32 nThreads);
   rspf_uint32 getNumberOfThreads() const;
   rspf_uint32 numberOfBusyThreads()const;
   bool areAllThreadsBusy()const;
   
   bool hasJobsToProcess()const;

protected:
   mutable OpenThreads::Mutex  m_mutex;
   rspfRefPtr<rspfJobQueue> m_jobQueue;
   ThreadQueueList m_threadQueueList;
};

#endif
