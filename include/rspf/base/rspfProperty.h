//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfProperty.h 19917 2011-08-09 11:12:24Z gpotts $
#ifndef rspfProperty_HEADER
#define rspfProperty_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfXmlNode.h>

class rspfContainerProperty;
class RSPF_DLL rspfProperty : public rspfObject
{
public:
   /*!
    * This are bitwise and can be ored together
    * The change type is very important and
    * it determines how things are refreshed.
    */
   enum rspfPropertyChangeType
   {
      rspfPropertyChangeType_NOTSPECIFIED   = 0,
      rspfPropertyChangeType_CACHE_REFRESH  = 1,
      rspfPropertyChangeType_FULL_REFRESH   = 2,
      rspfPropertyChangeType_AFFECTS_OTHERS = 4, // used to identify if this property modification affects the values of other properties once set
      rspfPropertyChangeType_ALL = (rspfPropertyChangeType_CACHE_REFRESH|rspfPropertyChangeType_FULL_REFRESH|rspfPropertyChangeType_AFFECTS_OTHERS)
   };
   rspfProperty(const rspfString& name=rspfString(""));
   rspfProperty(const rspfProperty& rhs);
   
   const  rspfProperty& operator = (const rspfProperty& rhs);
   virtual const rspfProperty& assign(const rspfProperty& rhs);

   const rspfString& getName()const;
   void setName(const rspfString& name);

   virtual bool setValue(const rspfString& value)=0;
   virtual void valueToString(rspfString& valueResult)const=0;
   virtual rspfString valueToString()const;

   virtual const rspfContainerProperty* asContainer()const;
   virtual rspfContainerProperty* asContainer();
   bool isChangeTypeSet(int type)const;
   void clearChangeType();
   void setChangeType(int type, bool on=true);
   void setFullRefreshBit();
   void setCacheRefreshBit();
   
   rspfPropertyChangeType getChangeType()const;
   bool isFullRefresh()const;
   bool isCacheRefresh()const;
   bool isChangeTypeSpecified()const;
   bool affectsOthers()const;
   
   virtual void setReadOnlyFlag(bool flag);
   bool getReadOnlyFlag()const;
   bool isReadOnly()const;

   void setModifiedFlag(bool flag);
   bool getModifiedFlag()const;
   bool isModified()const;
   void clearModifiedFlag();

   void setDescription(const rspfString& description);
   
   virtual rspfString getDescription()const;

   virtual rspfRefPtr<rspfXmlNode> toXml()const;
   
   virtual void saveState(rspfKeywordlist& kwl, const rspfString& prefix = "")const;
   virtual void accept(rspfVisitor& visitor);
   
protected:
   virtual ~rspfProperty();
   rspfString             theName;
   rspfString             theDescription;
   bool                    theReadOnlyFlag;
   rspfPropertyChangeType theChangeType;
   bool                    theModifiedFlag;
   
TYPE_DATA
};

#endif
