//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfCustomEditorWindowRegistry.h 17108 2010-04-15 21:08:06Z dburken $
#ifndef rspfCustomEditorWindowRegistry_HEADER
#define rspfCustomEditorWindowRegistry_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfCustomEditorWindow.h>
#include <rspf/base/rspfCustomEditorWindowFactoryBase.h>

#include <vector>
class rspfCustomEditorWindowFactoryBase;

class RSPFDLLEXPORT rspfCustomEditorWindowRegistry : public rspfCustomEditorWindowFactoryBase
{
public:
   virtual ~rspfCustomEditorWindowRegistry();
   static rspfCustomEditorWindowRegistry* instance();
   bool registerFactory(rspfCustomEditorWindowFactoryBase* factory);

   virtual rspfCustomEditorWindow* createCustomEditor(rspfObject* obj,
                                                       void* parent=NULL)const;
   virtual rspfCustomEditorWindow* createCustomEditor(const rspfString& classType,
                                                       void* parent=NULL)const;

   virtual rspfObject* createObject(const rspfString& typeName)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   /*!
    * Returns a native GUI Editor with popup
    */
   virtual void* createPopupEditor(rspfObject* obj,
                                   void* parent=NULL)const;
   
   /*!
    * Returns a native GUI child window without a popup.
    */
   virtual void* createPanelEditor(rspfObject* obj,
                                   void* parent=NULL)const;
   
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;

protected:
   rspfCustomEditorWindowRegistry(){}
   rspfCustomEditorWindowRegistry(const rspfCustomEditorWindowRegistry&){}
   void operator =(const rspfCustomEditorWindowRegistry& /*rhs*/){}
   
   std::vector<rspfCustomEditorWindowFactoryBase*> theFactoryList;
   
   static rspfCustomEditorWindowRegistry*                 theInstance;

TYPE_DATA
};

extern "C"
{
  RSPFDLLEXPORT void* rspfCustomEditorWindowRegistryGetInstance();
}
#endif
