//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfEventIds.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfEventIds_HEADER
#define rspfEventIds_HEADER

#define RSPF_EVENT_NULL_ID  -1


// destruction event
#define RSPF_EVENT_OBJECT_DESTRUCTING_ID     1

// connection events
#define RSPF_EVENT_CONNECTION_DISCONNECT_ID        3
#define RSPF_EVENT_CONNECTION_CONNECT_ID           4

#define RSPF_EVENT_REFRESH_ID  5
#define RSPF_EVENT_PROCESS_PROGRESS_ID  20

#define RSPF_EVENT_ADD_OBJECT_ID     40
#define RSPF_EVENT_REMOVE_OBJECT_ID  41


#define RSPF_EVENT_PROPERTY_ID       80
#define RSPF_EVENT_STATE_CHANGED_ID  90


#define RSPF_EVENT_AOI_RECTANGLE_ID  200
#define RSPF_EVENT_AOI_POLYGON_ID    201


#define RSPF_MOUSE_EVENT_ID               400
#define RSPF_MOUSE_EVENT_LEAVE_WINDOW_ID  401
#define RSPF_MOUSE_EVENT_MOTION_ID        402
#define RSPF_MOUSE_EVENT_LEFT_DOWN_ID     403
#define RSPF_MOUSE_EVENT_RIGHT_DOWN_ID    404
#define RSPF_MOUSE_EVENT_MIDDLE_DOWN_ID   405
#define RSPF_MOUSE_EVENT_LEFT_UP_ID       406
#define RSPF_MOUSE_EVENT_RIGHT_UP_ID      407
#define RSPF_MOUSE_EVENT_MIDDLE_UP_ID     408

#define RSPF_MOUSE_EVENT_LEFT_DCLICK_ID       409
#define RSPF_MOUSE_EVENT_RIGHT_DCLICK_ID      410
#define RSPF_MOUSE_EVENT_MIDDLE_DCLICK_ID     411
#define RSPF_MOUSE_EVENT_ENTER_WINDOW_ID  412

#define RSPF

#define RSPF_KEYBOARD_EVENT_ID  500

#define RSPF_EVENT_VIEW_ID      600


#define RSPF_EVENT_DISPLAY_LIST_ID           700
#define RSPF_EVENT_DISPLAY_REFRESH_ID        701

#define RSPF_EVENT_IMAGE_GEOMETRY_ID         800

// this is the first user defined id.  If users want to add
// additional id's they need to start at this id.  That way
// we have no conflicts.
//
#define RSPF_FIRST_USER_DEFINED_EVENT_ID     1000000

#endif
