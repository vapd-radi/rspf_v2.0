//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfViewEvent.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfViewEvent_HEADER
#define rspfViewEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfGpt.h>
class RSPFDLLEXPORT rspfViewEvent : public rspfEvent
{
public:
   enum rspfViewEventType
   {
      RSPF_VIEW_EVENT_TYPE_GENERIC     = 0,
      RSPF_VIEW_EVENT_SCALE_CHANGE,
      RSPF_VIEW_EVENT_TRANSFORM_CHANGE,
      RSPF_VIEW_EVENT_VIEW_TYPE_CHANGE 
   };
   enum rspfViewPropagateType
   {
      RSPF_VIEW_EVENT_PROPAGATE_NONE         = 0,
      RSPF_VIEW_EVENT_PROPAGATE_ALL_DISPLAYS = 1,
      RSPF_VIEW_EVENT_PROPAGATE_ALL_CHAINS   = 2,
   };
   
   rspfViewEvent(rspfObject* view,
                  rspfGpt     centerPt,
                  rspfViewPropagateType propagateType = RSPF_VIEW_EVENT_PROPAGATE_NONE,
                  rspfViewEventType eventType = RSPF_VIEW_EVENT_TYPE_GENERIC,
                  rspfObject* object=NULL)
      :rspfEvent(object, RSPF_EVENT_VIEW_ID),
       theView(view),
       theCenterPoint(centerPt),
       theEventType(eventType),
       thePropagateType(propagateType),
       theUpdateInputViewFlag(true),
       theRefreshDisplayFlag(true)
      {
      }
   rspfViewEvent(const rspfViewEvent& rhs)
      :rspfEvent(rhs),
       theView(rhs.theView),
       theCenterPoint(rhs.theCenterPoint),
       theEventType(rhs.theEventType),
       thePropagateType(rhs.thePropagateType),
       theUpdateInputViewFlag(rhs.theUpdateInputViewFlag),
       theRefreshDisplayFlag(rhs.theRefreshDisplayFlag)
      {
      }
   virtual rspfObject* dup()const
      {
         return new rspfViewEvent(*this);
      }
   virtual void setView(rspfObject* view)
      {
         theView = view;
      }
   virtual rspfObject* getView()
      {
         return theView;
      }
   void disablePropagation()
      {
         thePropagateType = RSPF_VIEW_EVENT_PROPAGATE_NONE;
      }
   void setPropagateType(rspfViewPropagateType type)
      {
         thePropagateType = type;         
      }
   void setViewEventType(rspfViewEventType eventType)
      {
         theEventType = eventType;
      }
   void setEventTypeViewTypeChange()
      {
         theEventType = RSPF_VIEW_EVENT_VIEW_TYPE_CHANGE;
      }
   void setEventTypeTransformChange()
      {
         theEventType = RSPF_VIEW_EVENT_TRANSFORM_CHANGE;
      }
   void setEventTypeGeneric()
      {
         theEventType = RSPF_VIEW_EVENT_TYPE_GENERIC;
      }
   void setCenterGroundPoint(const rspfGpt& gpt)
      {
         theCenterPoint = gpt;
      }
   const rspfGpt& getCenterGroundPoint()const
      {
         return theCenterPoint;
      }
   rspfViewEventType getViewEventType()const
      {
         return theEventType;
      }
   bool isViewChange()const
      {
         return (theEventType == RSPF_VIEW_EVENT_VIEW_TYPE_CHANGE);
      }
   bool isTransformChange()const
      {
         return (theEventType == RSPF_VIEW_EVENT_TRANSFORM_CHANGE);
      }
   bool isScaleChange()const
      {
         return (theEventType == RSPF_VIEW_EVENT_SCALE_CHANGE);
         
      }
   bool isPropagateEnabled()const
      {
         return (thePropagateType != RSPF_VIEW_EVENT_PROPAGATE_NONE);
      }
   bool isPropagateToDisplays()const
      {
         return (thePropagateType & RSPF_VIEW_EVENT_PROPAGATE_ALL_DISPLAYS);
      }
   bool isPropagateToChains()const
      {
         return (thePropagateType & RSPF_VIEW_EVENT_PROPAGATE_ALL_CHAINS);
      }
   void setUpdateInputViewFlag(bool flag)
      {
         theUpdateInputViewFlag = flag;
      }
   bool getUpdateInputViewFlag()const
      {
         return theUpdateInputViewFlag;
      }
   void setRefreshDisplayFlag(bool flag)
      {
         theRefreshDisplayFlag = flag;
      }
   bool getRefreshDisplayFlag()const
      {
         return theRefreshDisplayFlag;
      }
protected:
   rspfObject*           theView;
   rspfGpt               theCenterPoint;
   rspfViewEventType     theEventType;
   rspfViewPropagateType thePropagateType;
   bool                   theUpdateInputViewFlag;
   bool                   theRefreshDisplayFlag;
TYPE_DATA
};
#endif
