//**************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
//**************************************************************************************************
//  $Id$
#ifndef rspfJobQueue_HEADER
#define rspfJobQueue_HEADER

#include <rspf/parallel/rspfJob.h>
#include <OpenThreads/Block>

//*************************************************************************************************
//! Class for maintaining an ordered list of jobs to be processed. As the jobs are completed and
//! the product consumed, the jobs are removed from this list
//*************************************************************************************************
class RSPF_DLL rspfJobQueue : public rspfReferenced
{
public:
   class RSPF_DLL Callback : public rspfReferenced
   {
   public:
      Callback(){}
      virtual void adding(rspfJobQueue* /*q*/, rspfJob* /*job*/){}
      virtual void added(rspfJobQueue* /*q*/, rspfJob* /*job*/){}
      virtual void removed(rspfJobQueue* /*q*/, rspfJob* /*job*/){}
   protected:
   };
   rspfJobQueue();
   
   virtual void add(rspfJob* job, bool guaranteeUniqueFlag=true);
   virtual rspfRefPtr<rspfJob> removeByName(const rspfString& name);
   virtual rspfRefPtr<rspfJob> removeById(const rspfString& id);
   virtual void remove(const rspfJob* Job);
   virtual void removeStoppedJobs();
   virtual void clear();
   virtual rspfRefPtr<rspfJob> nextJob(bool blockIfEmptyFlag=true);
   virtual void releaseBlock();
   bool isEmpty()const;
   rspf_uint32 size();
   void setCallback(Callback* c);
   Callback* callback();
   
protected:
   rspfJob::List::iterator findById(const rspfString& id);
   rspfJob::List::iterator findByName(const rspfString& name);
   rspfJob::List::iterator findByPointer(const rspfJob* job);
   rspfJob::List::iterator findByNameOrPointer(const rspfJob* job);
   bool hasJob(rspfJob* job);
   
   mutable OpenThreads::Mutex m_jobQueueMutex;
   OpenThreads::Block m_block;
   rspfJob::List m_jobQueue;
   rspfRefPtr<Callback> m_callback;
};

#endif
