//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfFilenameProperty.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfFilenameProperty_HEADER
#define rspfFilenameProperty_HEADER
#include <vector>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfFilenameProperty : public rspfProperty
{
public:

   enum rspfFilenamePropertyIoType
   {
      rspfFilenamePropertyIoType_NOT_SPECIFIED = 0,
      rspfFilenamePropertyIoType_INPUT         = 1,
      rspfFilenamePropertyIoType_OUTPUT        = 2
   };
   
   rspfFilenameProperty(const rspfString& name = rspfString(""),
                         const rspfFilename& value = rspfFilename(""),
                         const std::vector<rspfString>& filterList = std::vector<rspfString>());

   rspfFilenameProperty(const rspfFilenameProperty& rhs);
   virtual ~rspfFilenameProperty();
   rspfObject* dup()const;
   virtual void valueToString(rspfString& valueResult)const;
  virtual bool setValue(const rspfString& value);
   const rspfFilenameProperty& operator = (rspfFilenameProperty& rhs);
   virtual const rspfProperty& assign(const rspfProperty& rhs);

   void clearFilterList();
   rspf_uint32 getNumberOfFilters()const;
   void setFilter(rspf_uint32 idx,
                  const rspfString& filterValue);
   rspfString getFilter(rspf_uint32 idx)const;
   rspfString getFilterListAsString(const rspfString& separator=rspfString("\n"))const;
   const std::vector<rspfString>& getFilterList()const;
   void addFilter(const rspfString& filter);

   void setIoType(rspfFilenamePropertyIoType ioType);
   rspfFilenamePropertyIoType getIoType()const;
   bool isIoTypeInput()const;
   bool isIoTypeOutput()const;
   
protected:
   rspfFilename            theValue;
   std::vector<rspfString> theFilterList;
   rspfFilenamePropertyIoType theIoType;
   
TYPE_DATA
};

#endif
