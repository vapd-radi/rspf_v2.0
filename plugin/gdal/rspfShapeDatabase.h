#ifndef rspfShapeDatabase_HEADER
#define rspfShapeDatabase_HEADER 1
#include <iostream>
#include <shapefil.h>
#include <rspfPluginConstants.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>
class rspfShapeDatabaseField
{
public:
   rspfString fieldTypeAsString()const;
   
   rspfString  theName;
   int         theWidth;
   int         theDecimals;
   DBFFieldType theFieldType;
   rspfString  theValue;
};
class RSPF_PLUGINS_DLL rspfShapeDatabaseRecord
{
   
public:
   bool getField(rspfShapeDatabaseField& result,
                 rspf_uint32 i);
   bool setField(const rspfShapeDatabaseField& field,
                 rspf_uint32 i);
   int getNumberOfFields()const;
   void setNumberOfFields(int n);
   rspf_int32 getFieldIdx(const rspfString& name,
                           bool caseInsensitive=true)const;
protected:
	std::vector<rspfShapeDatabaseField> theFieldArray;
};
class RSPF_PLUGINS_DLL rspfShapeDatabase : public rspfObject
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfShapeDatabase& rhs);
   
   rspfShapeDatabase();
   virtual ~rspfShapeDatabase();
   
   virtual bool open(const rspfFilename& file,
                     const rspfString& flags=rspfString("rb"));
   
   virtual void close();
   
   bool getRecord(rspfShapeDatabaseRecord& result);
   
   bool getRecord(rspfShapeDatabaseRecord& result,
                  int recordNumber);
   bool getNextRecord(rspfShapeDatabaseRecord& result);
   int getNumberOfRecords()const;
   
   bool isOpen()const;
   virtual DBFHandle getHandle();
   virtual const DBFHandle& getHandle()const;
   
   virtual std::ostream& print(std::ostream& out)const;
protected:
   DBFHandle theHandle;
   rspfFilename theFilename;
   int theRecordNumber;
TYPE_DATA
};
#endif
