//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
// Base class for any object that can be a source.  Derives from
// rspfConnectableObject since all source's should be connectable
//
//*************************************************************************
// $Id: rspfSource.h 15798 2009-10-23 19:15:20Z gpotts $

#ifndef rspfSource_HEADER
#define rspfSource_HEADER

#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfConstants.h>

class rspfDataObject;

class RSPFDLLEXPORT rspfSource : public rspfConnectableObject,
				   public rspfErrorStatusInterface
{
public:
   rspfSource(rspfObject* owner=0);
   rspfSource(rspfObject* owner,
               rspf_uint32 inputListSize,
               rspf_uint32 outputListSize,
               bool inputListIsFixedFlag=true,
               bool outputListIsFixedFlag=true);
   virtual ~rspfSource();
   
   virtual bool isSourceEnabled()const;
   virtual void enableSource();
   virtual void disableSource();
   virtual bool getEnableFlag() const;
   virtual void setEnableFlag(bool flag);

   virtual bool isInitialized() const;
   virtual void setInitializedFlag(bool flag);
   virtual void initialize();

   /*---------------------- PROPERTY INTERFACE ---------------------------*/
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /*--------------------- PROPERTY INTERFACE END ------------------------*/

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual std::ostream& print(std::ostream& out) const;

protected:
   rspfSource(const rspfSource &rhs);
   const rspfSource& operator=(const rspfSource &rhs);

   bool   theEnableFlag;
   bool   theInitializedFlag;

TYPE_DATA
};

#endif
