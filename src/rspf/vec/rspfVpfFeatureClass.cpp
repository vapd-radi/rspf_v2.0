#include <algorithm>
using namespace std;
#include <rspf/vec/rspfVpfFeatureClass.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/vec/rspfVpfCoverage.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/vec/rspfVpfTable.h>
#include <string.h> 
#ifndef NULL
#include <stddef.h>
#endif
rspfVpfFeatureClass::rspfVpfFeatureClass()
{
}
bool rspfVpfFeatureClass::openFeatureClass(const rspfString& featureClassName,
                                            const rspfFilename& featureClassFullPath,
                                            const rspfFilename& table1,
                                            const rspfString&   columnNameIdTable1,
                                            const rspfFilename& table2,
                                            const rspfString&   columnNameIdTable2)
{
   theFeatureClassFullPath = featureClassFullPath;
   theFeatureClassName     = featureClassName;
   theTableName1           = table1;
   theColumnNameId1        = columnNameIdTable1;
   theTableName2           = table2;
   theColumnNameId2        = columnNameIdTable2;
   
   rspfVpfTable t1;
   rspfVpfTable t2;
   rspfFilename t1Name = getTable1();
   rspfFilename t2Name = getTable2();
   if((!t1.openTable(t1Name))||
      (!t2.openTable(t2Name)))
     {
       
       return false;
     }
   
   return true;
}
const rspfString& rspfVpfFeatureClass::getName()const
{
  return theFeatureClassName;
  
}
bool rspfVpfFeatureClass::isPrimitive(const rspfString& featureName)
{
   rspfFilename filename(featureName.trim());   
   filename.convertBackToForwardSlashes();
   
   rspfString copyOfFeature = filename;
   const char* temp = strrchr(copyOfFeature.c_str(), '/');
   rspfString value;
   if(temp)
   {
      ++temp;
      value = temp;
   }
   else
   {
      
      value = copyOfFeature;
   }
   value = value.downcase();
   return( (value == "end") ||
           (value == "cnd") ||
           (value == "edg") ||
           (value == "fac") ||
           (value == "txt") );
}
bool rspfVpfFeatureClass::isSimpleFeature(const rspfString& featureName)
{
   rspfString value = strrchr(featureName.trim().c_str(), '.');
   if(value == "")
   {
      return false;
   }
   value = value.downcase();
   return ( (value == ".pft")||
            (value == ".lft")||
            (value == ".aft")||
            (value == ".tft"));
}
bool rspfVpfFeatureClass::isComplexFeature(const rspfString& featureName)
{
   const char* suffix = strrchr(featureName.trim(), '.');
   
   if (!suffix)
   {
      return false;
   }
   rspfString value = rspfString(suffix).downcase();
   
   return (value == ".cft");
}
bool rspfVpfFeatureClass::isFeature(const rspfString& featureName)
{
   return (isSimpleFeature(featureName) || isComplexFeature(featureName));
}
bool rspfVpfFeatureClass::isJoin(const rspfString& featureName)
{
   rspfString value = strrchr(featureName.c_str(), '.');
   if(value == "")
   {
      return false;
   }
   
   value = value.downcase();
   
   return ( (value == ".cjt")||
            (value == ".pjt")||
            (value == ".ljt")||
            (value == ".ajt")||
            (value == ".tjt"));
}
rspfFilename rspfVpfFeatureClass::getTable1()const
{
  return theFeatureClassFullPath.dirCat(theTableName1);
}
rspfFilename rspfVpfFeatureClass::getTable2()const
{
  return theFeatureClassFullPath.dirCat(theTableName2);
}
