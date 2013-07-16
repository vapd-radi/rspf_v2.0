#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/vec/rspfVpfCoverage.h>
#include <rspf/vec/rspfVpfFeatureClassSchema.h>
#include <rspf/vec/rspfVpfFeatureClass.h>
rspfVpfFeatureClassSchema::rspfVpfFeatureClassSchema()
   :theCoverage(NULL)
{
}
bool rspfVpfFeatureClassSchema::openSchema(rspfVpfCoverage* coverage)
{
   theCoverage = coverage;
   bool result = false;
   if(theCoverage)
   {
      theSchemaTableName = theCoverage->getPath().dirCat("fcs");
      if(theSchemaTableName.exists())
      {
         result = openTable(theSchemaTableName);
         if(result)
         {
            result = validateColumnNames();
         }
         if(result)
         {
            setFeatureClassMapping();
         }
      }
   }
   
   return result;
}
void rspfVpfFeatureClassSchema::closeTable()
{
   rspfVpfTable::closeTable();
   theFeatureClassMap.clear();
}
void rspfVpfFeatureClassSchema::getFeatureClasses(std::vector<rspfString>& featureClassArray)const
{
   featureClassArray.clear();
   std::map<rspfString, rspfVpfFeatureClassSchemaNode>::const_iterator featureClassIter = theFeatureClassMap.begin();
   while(featureClassIter != theFeatureClassMap.end())
   {
      featureClassArray.push_back((*featureClassIter).first);
      ++featureClassIter;
   }
}
bool rspfVpfFeatureClassSchema::getFeatureClassNode(const rspfString& featureClass,
                                                     rspfVpfFeatureClassSchemaNode& featureClassNode)const
{
   std::map<rspfString, rspfVpfFeatureClassSchemaNode>::const_iterator fIter = theFeatureClassMap.find(featureClass);
   if(fIter != theFeatureClassMap.end())
   {
      featureClassNode = (*fIter).second;
      return true;
   }
   return false;
}
rspf_int32 rspfVpfFeatureClassSchema::getNumberOfFeatureClasses()const
{
   return (rspf_int32)theFeatureClassMap.size();
}
bool rspfVpfFeatureClassSchema::validateColumnNames()const
{
   if(isClosed()) return false;
   return ((getColumnPosition("feature_class")>=0)&&
           (getColumnPosition("table1")>=0)&&
           (getColumnPosition("table1_key")>=0)&&
           (getColumnPosition("table2")>=0)&&
           (getColumnPosition("table2_key")>=0));
}
void rspfVpfFeatureClassSchema::setFeatureClassMapping()
{
   if(!isClosed())
   {
      rspf_int32 featureIdx = getColumnPosition("feature_class");
      rspf_int32 table1Idx = getColumnPosition("table1");
      rspf_int32 table1KeyIdx = getColumnPosition("table1_key");
      rspf_int32 table2Idx = getColumnPosition("table2");
      rspf_int32 table2KeyIdx = getColumnPosition("table2_key");
      reset();
      if(getNumberOfRows() > 0)
      {
         row_type row;
         const int ROWS = getNumberOfRows();
         for(int rowIdx = 1; rowIdx <= ROWS; ++rowIdx)
         {
            if(rowIdx == 1)
            {
               row = read_row(rowIdx,
                              *theTableInformation);
            }
            else
            {
               row = read_next_row(*theTableInformation);
            }
            rspfFilename primitiveTable =  getColumnValueAsString(row,
                                                                   table2Idx);
            if(rspfVpfFeatureClass::isPrimitive(primitiveTable))
            {
               rspfString primitiveTableKey = getColumnValueAsString(row,
                                                                      table2KeyIdx);
               rspfFilename table = getColumnValueAsString(row,
                                                            table1Idx);
               rspfString tableKey = getColumnValueAsString(row,
                                                             table1KeyIdx);
               rspfString featureClass = getColumnValueAsString(row,
                                                                 featureIdx);
               rspfVpfFeatureClassSchemaNode node(table,
                                                   tableKey,
                                                   primitiveTable,
                                                   primitiveTableKey);
               
               theFeatureClassMap.insert(make_pair(featureClass,
                                                   node));
            }
            
            free_row(row, *theTableInformation);
         }
      }
   }
}
