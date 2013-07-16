//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfViewInterface.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfViewInterface_HEADER
#define rspfViewInterface_HEADER
#include <rspf/base/rspfRtti.h>

class rspfObject;

class RSPFDLLEXPORT rspfViewInterface
{
public:
   rspfViewInterface();
   rspfViewInterface(rspfObject* base);
   virtual ~rspfViewInterface();

   /*!
    * The derived classes should overrid this method and
    * return true or false whether it was able to cast the
    * past in view to a view that it can interface to.
    * So if you are a projective view you might
    * cast to an rspfProjection.  The ownsTheView specifies
    * whether or not it owns the passed in view pointer.
    * This will specifiy whether or not the derived classes
    * need to re-allocate the pointer or just delete its old
    * pointer and set it to the passed in pointer.
    */
   virtual bool setView(rspfObject* baseObject)=0;
   virtual rspfObject* getView()=0;
   virtual const rspfObject* getView()const=0;
   virtual void refreshView();
   rspfObject* getBaseObject();

protected:
   rspfObject* theObject;
TYPE_DATA
   
};

#endif
