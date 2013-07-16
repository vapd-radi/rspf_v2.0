//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts 
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfRefreshEvent.h 19819 2011-07-14 17:28:48Z gpotts $
#ifndef rspfRefreshEvent_HEADER
#define rspfRefreshEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfDpt.h>

class RSPFDLLEXPORT rspfRefreshEvent : public rspfEvent
{
public:
   enum RefreshType
   {
      REFRESH_NONE      = 0,
      REFRESH_POSITION  = 1,
      REFRESH_PIXELS    = 2,
      REFRESH_GEOMETRY  = 4,
      REFRESH_ALL       = (REFRESH_POSITION|REFRESH_PIXELS|REFRESH_GEOMETRY)
   };
   enum PositionAnchor
   {
      ANCHOR_UPPER_LEFT = 1,
      ANCHOR_CENTER     = 2
   };
   rspfRefreshEvent(rspfObject* object=0)  // the object associated with the event if any
   :rspfEvent(object, RSPF_EVENT_REFRESH_ID),
    m_refreshType(static_cast<RefreshType>(REFRESH_PIXELS|REFRESH_GEOMETRY)),
    m_anchor(ANCHOR_CENTER)
   {m_position.makeNan();}
   rspfRefreshEvent(RefreshType refreshType, rspfObject* object=0)
   :rspfEvent(object, RSPF_EVENT_REFRESH_ID),
    m_refreshType(refreshType),
    m_anchor(ANCHOR_CENTER)

   {
      m_position.makeNan();
   }
   rspfRefreshEvent(const rspfRefreshEvent& src)
   :rspfEvent(src),
    m_refreshType(src.m_refreshType),
    m_position(src.m_position),
    m_anchor(src.m_anchor)
   {
   }
   virtual rspfObject* dup()const
   {
      return new rspfRefreshEvent(*this);
   }
 
   void setRefreshType(int refreshType, bool on=true);
   RefreshType getRefreshType()const{return m_refreshType;}
   
   void setPosition(const rspfDpt& position)
   {
      m_position = position;
      if(!m_position.hasNans())setRefreshType(REFRESH_POSITION);
   }
   
   const rspfDpt& getPosition()const{return m_position;}
   
                     
protected:
   RefreshType m_refreshType;
   rspfDpt    m_position;
   PositionAnchor m_anchor;
   TYPE_DATA
};

#endif
