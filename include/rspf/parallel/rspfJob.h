//**************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
//**************************************************************************************************
//  $Id$
#ifndef rspfJob_HEADER
#define rspfJob_HEADER
#include <rspf/base/rspfObject.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfString.h>
#include <list>

class rspfJob;

//*************************************************************************************************
//! Generic callback class needed by rspfJob
//*************************************************************************************************
class RSPF_DLL rspfJobCallback : public rspfReferenced
{
public:
   rspfJobCallback(rspfJobCallback* nextCallback=0):m_nextCallback(nextCallback){}

   virtual void ready(rspfJob* job)    {if(m_nextCallback.valid()) m_nextCallback->ready(job);   }
   virtual void started(rspfJob* job)  {if(m_nextCallback.valid()) m_nextCallback->started(job); }
   virtual void finished(rspfJob* job) {if(m_nextCallback.valid()) m_nextCallback->finished(job);}
   virtual void canceled(rspfJob* job) {if(m_nextCallback.valid()) m_nextCallback->canceled(job);}

   virtual void nameChanged(const rspfString& name, rspfJob* job)
   {if(m_nextCallback.valid()) m_nextCallback->nameChanged(name, job);}
   
   virtual void descriptionChanged(const rspfString& description, rspfJob* job)
   {if(m_nextCallback.valid()) m_nextCallback->descriptionChanged(description, job);}

   virtual void idChanged(const rspfString& id, rspfJob* job)
   {if(m_nextCallback.valid()) m_nextCallback->idChanged(id, job);}

   virtual void percentCompleteChanged(double percentValue, rspfJob* job)
   {if(m_nextCallback.valid()) m_nextCallback->percentCompleteChanged(percentValue, job);}

   void setCallback(rspfJobCallback* c){m_nextCallback = c;}
   rspfJobCallback* callback(){return m_nextCallback.get();}

protected:
   rspfRefPtr<rspfJobCallback> m_nextCallback;
};


//*************************************************************************************************
//! Pure virtual base class for all job types
//*************************************************************************************************
class RSPF_DLL rspfJob : public rspfObject
{
public:
   typedef std::list<rspfRefPtr<rspfJob> > List;

   /** 
   * This is a Bit vector.  The only value that can be assigned as both active is FINISHED and CANCEL.
   * CANCELED job may not yet be finished.  Once the job is finished the Cancel is complete
   */ 
   enum State
   {
      rspfJob_NONE     = 0,
      rspfJob_READY    = 1,
      rspfJob_RUNNING  = 2,
      rspfJob_CANCEL   = 4,
      rspfJob_FINISHED = 8,
      rspfJob_ALL = (rspfJob_READY|rspfJob_RUNNING|rspfJob_CANCEL|rspfJob_FINISHED)
   };
   
   rspfJob() : m_state(rspfJob_READY),  m_priority(0.0) {}

   virtual void start()=0;
   
   void setPercentComplete(double value)
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      if(m_callback.valid())
      {
         m_callback->percentCompleteChanged(value, this);
      }
   }

   void setPriority(double value)
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      m_priority = value;
   }

   double priority()const
   {
      return m_priority;
   }

   virtual void release(){}

   State state()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return m_state;
   }

   virtual void resetState(int value)
   {
      m_jobMutex.lock();
      if(value != m_state)
      {
         m_state = rspfJob_NONE;
         m_jobMutex.unlock();
         setState(value);
      }
      else 
      {
         m_jobMutex.unlock();
      }

   }

   virtual void setState(int value, bool on=true);

   bool isCanceled()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return (m_state & rspfJob_CANCEL);
   }

   virtual void cancel()
   {
      // append the cancel flag to current state
      setState(rspfJob_CANCEL);
   }

   virtual void ready()
   {
      resetState(rspfJob_READY);
   }

   virtual void running()
   {
      resetState(rspfJob_RUNNING);
   }

   virtual void finished()
   {
      int newState = 0;
      {
         // maintain the cancel flag so we can indicate the job has now finished
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         newState = ((m_state & rspfJob_CANCEL) | 
            (rspfJob_FINISHED));
      }
      // now reset to the new state
      resetState(newState);
   }

   bool isReady()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return m_state & rspfJob_READY;
   }

   bool isStopped()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return (m_state & rspfJob_FINISHED);
   }

   bool isFinished()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return (m_state & rspfJob_FINISHED);
   }

   bool isRunning()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return (m_state & rspfJob_RUNNING);
   }

   void setCallback(rspfJobCallback* callback)
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      m_callback = callback;
   }

   void setName(const rspfString& value)
   {
      bool changed = false;
      rspfRefPtr<rspfJobCallback> callback;
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         changed = value!=m_name;
         m_name = value;
         callback = m_callback;
      }
      if(changed&&callback.valid())
      {
         callback->nameChanged(value, this);
      }
   }

   const rspfString& name()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return m_name;
   }

   void setId(const rspfString& value)
   {
      bool changed = false;
      rspfRefPtr<rspfJobCallback> callback;
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         changed = value!=m_id;
         m_id = value;
         callback = m_callback;
      }
      if(changed&&callback.valid())
      {
         callback->idChanged(value, this);
      }
   }

   const rspfString& id()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return m_id;
   }

   void setDescription(const rspfString& value)
   {
      bool changed = false;
      rspfRefPtr<rspfJobCallback> callback;
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         changed = value!=m_description;
         m_description = value;
         callback = m_callback;
      }
      if(changed&&callback.valid())
      {
         callback->descriptionChanged(value, this);
      }
   }

   const rspfString& description()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      return m_description;
   }
   rspfJobCallback* callback() {return m_callback.get();}

protected:
   mutable OpenThreads::Mutex m_jobMutex;
   rspfString m_name;
   rspfString m_description;
   rspfString m_id;
   State       m_state;
   double      m_priority;
   rspfRefPtr<rspfJobCallback> m_callback;
};

#endif
