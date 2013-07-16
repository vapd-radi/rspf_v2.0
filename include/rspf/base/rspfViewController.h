//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfViewController.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfViewController_HEADER
#define rspfViewController_HEADER
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfViewInterface.h>

/*!
 * All view sources will derive from this class.  For example
 * we can have a mapped view or Perspective view of the scene
 * being rendered.  All sources within the containers are expected
 * to derive from the view interface.
 */
class RSPFDLLEXPORT rspfViewController : public rspfSource
{
public:

   rspfViewController();
   
   rspfViewController(rspfObject* owner,
                       rspf_uint32 inputListSize,
                       rspf_uint32 outputListSize,
                       bool   inputListFixedFlag=true,
                       bool   outputListFixedFlag=false);

   virtual ~rspfViewController();
 
   virtual rspfString getShortName()const;
   
   virtual rspfString getLongName()const;

   virtual bool propagateView();
   virtual bool setView(rspfObject* object);
   virtual rspfObject* getView();
   virtual const rspfObject* getView()const;
   virtual bool canConnectMyInputTo(rspf_int32 index,
                                    const rspfConnectableObject* obj)const;
   virtual const rspfObject* findFirstViewOfType(RTTItypeid typeId)const;
protected:
   rspfRefPtr<rspfObject> theView;
   
TYPE_DATA
};

#endif
