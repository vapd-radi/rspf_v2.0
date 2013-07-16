//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfCustomEditorWindow.h 17108 2010-04-15 21:08:06Z dburken $
#ifndef rspfCustomEditorWindow_HEADER
#define rspfCustomEditorWindow_HEADER
#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfCustomEditorWindow : public rspfObject
{
public:
   /*!
    * Returns the system dependent window.  If you are in wx it
    * will probalby be some type of wxWindow*.
    */
   virtual void* getWindow()=0;

   /*!
    * This is the parent window.  If the parent can be
    * set by the derived classes it will return true.
    */
   virtual void  setParent(void* parent)=0;
   
   /*!
    * Will return the parent of the managed wxWindow
    */
   virtual void* getParent()=0;
   
   /*!
    * WIll set the object to edit.  If this editor does not know
    * anything about the passed in obj it will return false.
    */
   virtual bool setObjectToEdit(rspfObject* obj)=0;

   /*!
    * Just asks if I can edit the passed in object.
    */
   virtual bool canEdit(rspfObject* obj)const=0;

   /*!
    * Returns the object we are editing.
    */
   virtual rspfObject* getObject()=0;

protected:
   rspfCustomEditorWindow(){}
   rspfCustomEditorWindow(const rspfCustomEditorWindow&){}
   void operator =(const rspfCustomEditorWindow& /*rhs*/){}

TYPE_DATA
};
#endif
