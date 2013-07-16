//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfId.h 20314 2011-12-01 21:46:37Z oscarkramer $
#ifndef rspfId_HEADER
#define rspfId_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>

class RSPF_DLL rspfId
{
public:
   friend RSPF_DLL std::ostream& operator <<(std::ostream &out, const rspfId& data);
   
   explicit rspfId(rspf_int64 id = 0)
      :theId(id)
      {}
   virtual ~rspfId(){}

   virtual void print(std::ostream& out)const;

   void setId(rspf_int64 id){theId = id;}
   rspf_int64 getId()const{return theId;}

   bool operator ==(const rspfId& rhs)const
   {
      return (theId == rhs.theId);
   }
   bool operator !=(const rspfId& rhs)const
   {
      return (theId != rhs.theId);
   }
  
   bool operator == (rspf_int64 rhs) const
   {
     return (theId == rhs);
   }
   bool operator != (rspf_int64 rhs) const
   {
     return (theId != rhs);
   }

   bool operator <(const rspfId& rhs)const
   {
      return (theId < rhs.theId);
   }
   bool operator <=(const rspfId& rhs)const
   {
      return (theId <= rhs.theId);
   }
   bool operator >(const rspfId& rhs)const
   {
      return (theId > rhs.theId);
   }
   bool operator >=(const rspfId& rhs)const
   {
      return (theId >= rhs.theId);
   }

   bool operator <(rspf_int64 rhs)const
   {
      return (theId < rhs);
   }
   bool operator >(rspf_int64 rhs)const
   {
      return (theId > rhs);
   }
   bool operator <=(rspf_int64 rhs)const
   {
      return (theId <= rhs);
   }
   bool operator >=(rspf_int64 rhs)const
   {
      return (theId >= rhs);
   }

   static const rspf_int64 INVALID_ID;
protected:
   rspf_int64 theId;
};

#endif
