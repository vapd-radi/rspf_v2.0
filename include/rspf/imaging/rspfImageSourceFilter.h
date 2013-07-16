//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: rspfImageSourceFilter.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfImageSourceFilter_HEADER
#define rspfImageSourceFilter_HEADER
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfConnectionEvent.h>

class RSPFDLLEXPORT rspfImageSourceFilter : public rspfImageSource,
     public rspfConnectableObjectListener
{
public:
   rspfImageSourceFilter(rspfObject* owner=NULL);
   rspfImageSourceFilter(rspfImageSource* inputSource);
   rspfImageSourceFilter(rspfObject* owner,
                          rspfImageSource* inputSource);

   /*!
    * Returns the input connection's output band list.
    * If not connected calls rspfImageSource::getOutputBandList.
    */
   virtual void getOutputBandList(std::vector<rspf_uint32>& bandList) const;
   
   /*!
    * Returns the number of bands available from the input.
    */
   virtual rspf_uint32 getNumberOfInputBands()const;   

   virtual void initialize();

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object)const;
   virtual void connectInputEvent(rspfConnectionEvent& event);
   virtual void disconnectInputEvent(rspfConnectionEvent& event);
   virtual void propertyEvent(rspfPropertyEvent& event);
   virtual void refreshEvent(rspfRefreshEvent& event);
   

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
protected:
   virtual ~rspfImageSourceFilter();
   rspfImageSource* theInputConnection;
TYPE_DATA
};

#endif /* #ifndef rspfImageSourceFilter_HEADER */
