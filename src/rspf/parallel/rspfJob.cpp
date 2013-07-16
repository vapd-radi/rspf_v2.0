#include <rspf/parallel/rspfJob.h>

void rspfJob::setState(int value, bool on)
{
   // we will need to make sure that the state flags are set properly
   // so if you turn on running then you can't have finished or ready turned onturned on
   // but can stil have cancel turned on
   //
   int newState = m_state;
   if(on)
   {
      newState = ((newState | value)&rspfJob_ALL);
   }
   else 
   {
      newState = ((newState & ~value)&rspfJob_ALL);
   }

   int oldState     = 0;
   int currentState = 0;
   rspfRefPtr<rspfJobCallback> callback;

   bool stateChangedFlag = false;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
      
      stateChangedFlag = newState != m_state;
      oldState = m_state;
      m_state = static_cast<State>(newState);
      currentState = m_state;
      callback = m_callback.get();
   }
   
   if(stateChangedFlag&&callback.valid())
   {
      if(callback.valid())
      {
         if(!(oldState&rspfJob_READY)&&
            (currentState&rspfJob_READY))
         {
            callback->ready(this);
         }
         else if(!(oldState&rspfJob_RUNNING)&&
                 (currentState&rspfJob_RUNNING))
         {
            callback->started(this);
         }
         else if(!(oldState&rspfJob_CANCEL)&&
                 (currentState&rspfJob_CANCEL))
         {
            callback->canceled(this);
         }
         else if(!(oldState&rspfJob_FINISHED)&&
                 (currentState&rspfJob_FINISHED))
         {
            callback->finished(this);
         }
      }
   }
}
