#include <algorithm>
using namespace std;
#include <rspf/vec/rspfVpfCoverage.h>
#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/vec/rspfVpfFeatureClassSchema.h>
#include <rspf/vec/rspfVpfBoundingRecordTable.h>
#include <rspf/base/rspfErrorCodes.h>
rspfVpfCoverage::rspfVpfCoverage()
   :theLibrary(NULL),
    theCoverageName(""),
    theCoverageNameFullPath("")
{
}
rspfVpfCoverage::~rspfVpfCoverage()
{
   theLibrary = NULL;
}
bool rspfVpfCoverage::openCoverage(rspfVpfLibrary* library,
                                    const rspfString& name,
                                    const rspfFilename& fullPath)
{
   bool result = true;
   theCoverageName         = name;
   theCoverageNameFullPath = fullPath;
   theLibrary = library;
   if(!theCoverageNameFullPath.dirCat("fcs").exists())
   {
      result = false;
   }
   
   return result;
}
const rspfFilename& rspfVpfCoverage::getPath()const
{
   return theCoverageNameFullPath;
}
const rspfString& rspfVpfCoverage::getName()const
{
   return theCoverageName;
}
void rspfVpfCoverage::print(ostream& out)const
{
  rspfVpfTable table;
  if(table.openTable(theCoverageNameFullPath.dirCat("fcs")))
  {
     table.print(out);
  }
}
bool rspfVpfCoverage::getExtent(rspfVpfExtent& extent)const
{
   bool result = false;
   bool firstOneSetFlag = false;
   rspfVpfExtent resultExtent;
   rspfVpfExtent tempExtent;
   if(theLibrary)
   {
      if(theCoverageNameFullPath.dirCat("ebr").exists())
      {
         rspfVpfBoundingRecordTable boundingRecord;
         
         if(boundingRecord.openTable(theCoverageNameFullPath.dirCat("ebr")))
         {
            boundingRecord.getExtent(tempExtent);
            if(!firstOneSetFlag)
            {
               resultExtent = tempExtent;
               firstOneSetFlag = true;
            }
            else
            {
               resultExtent = resultExtent + tempExtent;
            }
            
            result = true;
         }
      }
      if(theCoverageNameFullPath.dirCat("fbr").exists())
      {
         rspfVpfBoundingRecordTable boundingRecord;
         
         if(boundingRecord.openTable(theCoverageNameFullPath.dirCat("fbr")))
         {
            boundingRecord.getExtent(tempExtent);
            
            if(!firstOneSetFlag)
            {
               resultExtent = tempExtent;
               firstOneSetFlag = true;
            }
            else
            {
               resultExtent = resultExtent + tempExtent;
            }
            
            result = true;
         }
      }
      std::vector<rspfString> tileNames;
      
      theLibrary->getTileNames(tileNames);
      
      for(int i = 0; i < (int)tileNames.size(); ++i)
      {
         rspfFilename tileName = tileNames[0];
         rspfFilename tileDir;
         if(theCoverageNameFullPath.dirCat(tileName).exists())
         {
            tileDir = theCoverageNameFullPath.dirCat(tileName);
         }
         else if(theCoverageNameFullPath.dirCat(tileName.downcase()).exists())
         {
            tileDir = theCoverageNameFullPath.dirCat(tileName.downcase());
         }
         if(tileDir != "")
         {
            if(tileDir.dirCat("ebr").exists())
            {
               rspfVpfBoundingRecordTable boundingRecord;
               
               if(boundingRecord.openTable(tileDir.dirCat("ebr")))
               {
                  boundingRecord.getExtent(tempExtent);
                  
                  if(!firstOneSetFlag)
                  {
                     resultExtent = tempExtent;
                     firstOneSetFlag = true;
                  }
                  else
                  {
                     resultExtent = resultExtent + tempExtent;
                  }
                  
                  result = true;
               }
            }
            if(tileDir.dirCat("fbr").exists())
            {
               rspfVpfBoundingRecordTable boundingRecord;
               
               if(boundingRecord.openTable(tileDir.dirCat("fbr")))
               {
                  boundingRecord.getExtent(tempExtent);
                  
                  if(!firstOneSetFlag)
                  {
                     resultExtent = tempExtent;
                     firstOneSetFlag = true;
                  }
                  else
                  {
                     resultExtent = resultExtent + tempExtent;
                  }
                  
                  result = true;
               }
            }
         }
      }
   }
   extent = resultExtent;
   
   return result;
}
rspfVpfLibrary* rspfVpfCoverage::getLibrary()
{
   return theLibrary;
}
bool rspfVpfCoverage::openFeatureClassSchema(rspfVpfFeatureClassSchema& schema)
{
   if(!schema.isClosed()) schema.closeTable();
   return schema.openSchema(this);
}
