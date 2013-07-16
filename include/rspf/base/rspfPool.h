//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
//**********************************************************************
// $Id: rspfPool.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfPool_HEADER
#define rspfPool_HEADER
#include <iostream>
#include <vector>
#include <exception>
#include <stdexcept>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfPoolObject.h>
#include <rspf/base/rspfNotifyContext.h>

#define RSPF_DEFAULT_POOL_SIZE (rspf_uint32)10

/**
 * Container pool to be used for things like threads.
 */
template <class T>
class rspfPool
{
 public:
   /**
    * Contructor
    * @param n Number of pooled objects to create.
    */
   rspfPool(rspf_uint32 n=RSPF_DEFAULT_POOL_SIZE) throw(std::bad_alloc)
      : thePool(n)
   {
      for(rspf_uint32 idx = 0; idx < n; ++idx)
      {
         try
         {
               rspfPoolObject<T>* poolObject = new rspfPoolObject<T>();
               poolObject->theAvailableFlag = true;
               poolObject->theObjectPtr = new T();
               thePool[idx] = poolObject;
         }
         catch(std::bad_alloc)
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "FATAL rspfPool::rspfPool(): Bad allocation" << endl;
         }
      }
   }

   /**
    * Destructor
    */
   ~rspfPool()
   {
      thePool.clear();
   }

   /**
    * @return A pooled object of type T*.  Will return null if the pool is
    * used up or pool size is zero.
    */
   T* checkout()
   {
      T* t = NULL;
      
      theCriticalSectionMutex.lock();
      for (rspf_uint32 idx = 0; idx < thePool.size(); ++idx)
      {
         if (thePool[idx]->theAvailableFlag == true)
         {
            thePool[idx]->theAvailableFlag = false;
            t = thePool[idx]->theObjectPtr;
            break;
         }
      }
      theCriticalSectionMutex.unlock();
      return t;
   }

   /**
    * Resets and eturns object to the pool.
    * @param obj Object to be returned to the pool.
    * 
    * @note The object is required to have a "reset()" method.
    *
    * @return Returns void unless object was not from pool then throws
    * a std::out_of_range error.
    */
   void checkin(T* obj) throw(std::out_of_range)
   {
      theCriticalSectionMutex.lock();
      for (rspf_uint32 idx = 0; idx < thePool.size(); ++idx)
      {
         if (obj == thePool[idx]->theObjectPtr)
         {
            obj->reset();
            thePool[idx]->theAvailableFlag = true;
            theCriticalSectionMutex.unlock();
            return;
         }
      }
      
      theCriticalSectionMutex.unlock();
      throw std::out_of_range(string("rspfPool<T>::checkin Error Object not managed by this pool!"));
   }

   /**
    * Outputs object addresses and theAvailableFlag.
    */
   std::ostream& print(std::ostream& out) const
   {
      for (rspf_uint32 idx = 0; idx < thePool.size(); ++idx)
      {
         out << "obj address:  " << (hex) << thePool[idx]->theObjectPtr
             << " is "
             << (thePool[idx]->theAvailableFlag ? "available":"unavailable")
             << endl;
      } 
      return out;
   }
   
   template <class C> friend std::ostream& operator<<(std::ostream& out,
                                                      const rspfPool<C>& obj)
   {
      return obj.print(out);
   }

protected:
   
   std::vector<rspfPoolObject<T>*> thePool;
};

#endif /* #ifndef rspfPool_HEADER */

