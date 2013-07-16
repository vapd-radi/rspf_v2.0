//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfCustomEditorWindowFactoryBase.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfCustomEditorWindowFactoryBase_HEADER
#define rspfCustomEditorWindowFactoryBase_HEADER
#include <rspf/base/rspfObjectFactory.h>

class rspfCustomEditorWindow;

class RSPFDLLEXPORT rspfCustomEditorWindowFactoryBase : public rspfObjectFactory
{
public:
   virtual rspfCustomEditorWindow* createCustomEditor(rspfObject* obj,
                                                       void* parent=NULL)const=0;
   virtual rspfCustomEditorWindow* createCustomEditor(const rspfString& classType,
                                                       void* parent=NULL)const=0;

   /*!
    * Returns a native GUI Editor with popup
    */
   virtual void* createPopupEditor(rspfObject* obj,
                                   void* parent=NULL)const=0;
   
   /*!
    * Returns a native GUI child window without a popup.
    */
   virtual void* createPanelEditor(rspfObject* obj,
                                   void* parent=NULL)const=0;
TYPE_DATA
};

#endif
