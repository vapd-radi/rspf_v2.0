/* -*-c++-*- rspf - Copyright (C) since 2004 Garrett Potts 
 *
 * This was taken directly from OpenSceneGraph and will retain OSGGPL license.
 * This is basically an LGPL.
 * 
*/
#ifndef rspfRefPtr_HEADER
#define rspfRefPtr_HEADER
#include <rspf/base/rspfConstants.h>

template<class T> class rspfRefPtr
{
 public:
   typedef T element_type;
   
   rspfRefPtr() :m_ptr(0) {}
   rspfRefPtr(T* t):m_ptr(t)              { if (m_ptr) m_ptr->ref(); }
   rspfRefPtr(const rspfRefPtr& rp):m_ptr(rp.m_ptr)  { if (m_ptr) m_ptr->ref(); }
   ~rspfRefPtr()                           { if (m_ptr) m_ptr->unref(); m_ptr=0; }
   
   inline rspfRefPtr& operator = (const rspfRefPtr& rp)
      {
         if (m_ptr==rp.m_ptr) return *this;
         T* tmpPtr = m_ptr;
         m_ptr = rp.m_ptr;
         if (m_ptr) m_ptr->ref();
         // unref second to prevent any deletion of any object which might
         // be referenced by the other object. i.e rp is child of the
         // original _ptr.
         if (tmpPtr) tmpPtr->unref();
         return *this;
      }
   
   inline rspfRefPtr& operator = (T* ptr)
      {
         if (m_ptr==ptr) return *this;
         T* tmpPtr = m_ptr;
         m_ptr = ptr;
         if (m_ptr) m_ptr->ref();
         // unref second to prevent any deletion of any object which might
         // be referenced by the other object. i.e rp is child of the
         // original m_ptr.
         if (tmpPtr) tmpPtr->unref();
         return *this;
      }
   
   // comparison operators for rspfRefPtr.
   inline bool operator == (const rspfRefPtr& rp) const { return (m_ptr==rp.m_ptr); }
   inline bool operator != (const rspfRefPtr& rp) const { return (m_ptr!=rp.m_ptr); }
   inline bool operator < (const rspfRefPtr& rp) const { return (m_ptr<rp.m_ptr); }
   inline bool operator > (const rspfRefPtr& rp) const { return (m_ptr>rp.m_ptr); }
   
   // comparion operator for const T*.
   inline bool operator == (const T* ptr) const { return (m_ptr==ptr); }
   inline bool operator != (const T* ptr) const { return (m_ptr!=ptr); }
   inline bool operator < (const T* ptr) const { return (m_ptr<ptr); }
   inline bool operator > (const T* ptr) const { return (m_ptr>ptr); }
   
   inline T& operator*()  { return *m_ptr; }
   
   inline const T& operator*() const { return *m_ptr; }
   
   inline T* operator->() { return m_ptr; }
   
   inline const T* operator->() const   { return m_ptr; }
   
   inline bool operator!() const	{ return m_ptr==0L; }
   
   inline bool valid() const	{ return m_ptr!=0L; }
   
   inline T* get() { return m_ptr; }
   
   inline const T* get() const { return m_ptr; }
   
   /** take control over the object pointed to by ref_ptr, unreference but do not delete even if ref count goes to 0,
    * return the pointer to the object.
    * Note, do not use this unless you are 100% sure your code handles the deletion of the object correctly, and
    * only use when absolutely required.*/
   inline T* take() { return release();}
   
   inline T* release() { T* tmp=m_ptr; if (m_ptr) m_ptr->unref_nodelete(); m_ptr=0; return tmp;}
   
 private:
   T* m_ptr;
};

#endif
