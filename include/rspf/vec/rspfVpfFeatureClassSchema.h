#ifndef rspfVpfFeatureClassSchema_HEADER
#define rspfVpfFeatureClassSchema_HEADER
#include <iostream>
#include <map>
#include <vector>
#include <rspf/vec/rspfVpfTable.h>
class rspfVpfLibrary;
class rspfVpfFeatureClassSchemaNode
{
 public:
   friend ostream& operator <<(std::ostream& out,
                               const rspfVpfFeatureClassSchemaNode& data)
      {
         out << "table    :     " << data.theTable << endl
             << "key      :     " << data.theTableKey << endl
             << "primitive:     " << data.thePrimitiveTable << endl
             << "primitive key: " << data.thePrimitiveTableKey << endl;
         return out;
      }
   
   rspfVpfFeatureClassSchemaNode(const rspfFilename& table=rspfFilename(""),
                                  const rspfString& tableKey=rspfString(""),
                                  const rspfFilename& primitiveTable=rspfFilename(""),
                                  const rspfString& primitiveTableKey=rspfString(""))
      :theTable(table),
      theTableKey(tableKey),
      thePrimitiveTable(primitiveTable),
      thePrimitiveTableKey(primitiveTableKey)
   {
   }
   
   rspfFilename theTable;
   rspfString   theTableKey;
   rspfFilename thePrimitiveTable;
   rspfString   thePrimitiveTableKey;
};
class rspfVpfFeatureClassSchema : public rspfVpfTable
{
public:
   rspfVpfFeatureClassSchema();
   bool openSchema(rspfVpfCoverage* coverage);
   virtual void closeTable();
   
   void getFeatureClasses(std::vector<rspfString>& featureClassArray)const;
   bool getFeatureClassNode(const rspfString& featureClass,
                            rspfVpfFeatureClassSchemaNode& featureClassNode)const;
   rspf_int32 getNumberOfFeatureClasses()const;
protected:
   rspfVpfCoverage* theCoverage;
   rspfFilename     theSchemaTableName;
   std::map<rspfString, rspfVpfFeatureClassSchemaNode> theFeatureClassMap;
   
   bool validateColumnNames()const;
   void setFeatureClassMapping();
};
#endif
