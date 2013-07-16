//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfMouseEvent.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfMouseEvent_HEADER
#define rspfMouseEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfIpt.h>

class RSPFDLLEXPORT rspfMouseEvent : public rspfEvent
{
public:
   rspfMouseEvent(const rspfDpt& scenePoint=rspfDpt(0,0),
                   const rspfDpt& relViewPoint=rspfDpt(0,0),
                   rspfObject* object = NULL,
                   long id = RSPF_MOUSE_EVENT_ID,
                   long mouseType = 0)
      :rspfEvent(object, id),
       theScenePoint(scenePoint),
       theRelViewPoint(relViewPoint),
       theMouseType(mouseType)
      {
      }
   
   virtual rspfObject* dup()const{return new rspfMouseEvent(*this);}
   
   bool controlDown() const { return theControlKeyDown; }
   bool altDown() const { return theAltKeyDown; }
   bool shiftDown() const { return theShiftKeyDown; }
   
   // Find which event was just generated
   bool leftDown() const{   return (theMouseType == RSPF_MOUSE_EVENT_LEFT_DOWN_ID); }
   bool middleDown() const{ return (theMouseType == RSPF_MOUSE_EVENT_MIDDLE_DOWN_ID); }
   bool rightDown() const{  return (theMouseType == RSPF_MOUSE_EVENT_RIGHT_DOWN_ID); }
   bool leftUp() const{   return (theMouseType == RSPF_MOUSE_EVENT_LEFT_UP_ID); }
   bool middleUp() const{ return (theMouseType == RSPF_MOUSE_EVENT_MIDDLE_UP_ID); }
   bool rightUp() const{  return (theMouseType == RSPF_MOUSE_EVENT_RIGHT_UP_ID); }
   
   bool leftDClick() const { return (theMouseType == RSPF_MOUSE_EVENT_LEFT_DCLICK_ID); }
   bool middleDClick() const { return (theMouseType == RSPF_MOUSE_EVENT_MIDDLE_DCLICK_ID); }
   bool rightDClick() const { return (theMouseType == RSPF_MOUSE_EVENT_RIGHT_DCLICK_ID); }
   
   // Find the current state of the mouse buttons (regardless
   // of current event type)
   bool leftIsDown() const   { return theLeftDown; }
   bool middleIsDown() const { return theMiddleDown; }
   bool rightIsDown() const  { return theRightDown; }
   
   // True if a button is down and the mouse is moving
   bool dragging() const
      {
         return (moving() &&
                 (leftIsDown() || middleIsDown() || rightIsDown()));
      }

   // True if the mouse is moving, and no button is down
   bool moving() const { return (theMouseType == RSPF_MOUSE_EVENT_MOTION_ID); }
   
   // True if the mouse is just entering the window
   bool entering() const { return (theMouseType == RSPF_MOUSE_EVENT_ENTER_WINDOW_ID); }
   
   // True if the mouse is just leaving the window
   bool leaving() const { return (theMouseType   == RSPF_MOUSE_EVENT_LEAVE_WINDOW_ID); }
   
   rspfDpt theScenePoint;
   rspfDpt theRelViewPoint;
   int     theMouseType;
   bool     theAltKeyDown;
   bool     theControlKeyDown;
   bool     theShiftKeyDown;
   bool     theLeftDown;
   bool     theRightDown;
   bool     theMiddleDown;
   
TYPE_DATA
};

#endif
