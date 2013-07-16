#include <algorithm>
#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/vec/rspfVpfDatabase.h>
#include <rspf/vec/rspfVpfTable.h>
#include <rspf/vec/rspfVpfDatabase.h>
#include <rspf/vec/rspfVpfExtent.h>
#include <rspf/vec/rspfVpfCoverage.h>
#include <rspf/base/rspfErrorCodes.h>
rspfVpfLibrary::rspfVpfLibrary()
   :theDatabase(0),
    theLibraryName(""),
    theLibraryNameFullPath(""),
    theNumberOfCoverages(0)
{
}
rspfVpfLibrary::~rspfVpfLibrary()
{
   theDatabase = 0;
}
bool rspfVpfLibrary::openLibrary(rspfVpfDatabase* database,
                                  const rspfString& name,
                                  const rspfFilename& libraryPath)
{
   bool returnCode = true;
   theNumberOfCoverages = 0;
   theLibraryName = "";
   theLibraryNameFullPath = "";
   
   
   theLibraryName         = name;
   theLibraryNameFullPath =  libraryPath;
   theDatabase            = database;
   
   if(!rspfFilename(theLibraryNameFullPath).exists())
   {
      returnCode = false;
   }
   if(returnCode)
   {
      setCoverageNames();
      
      rspfVpfTable table;
      
      theNumberOfCoverages = (rspf_uint32)theCoverageNames.size();
      returnCode = (theNumberOfCoverages> 0);
   }
   
   return returnCode;
}
bool rspfVpfLibrary::getCoverage(long coverageNumber,
                                  rspfVpfCoverage& coverage)
{
   
   bool result = false;
   
   if((coverageNumber >=0) && (coverageNumber < (int)theCoverageNames.size()))
   {
      result = coverage.openCoverage(this,
                                     theCoverageNames[coverageNumber],
                                     theLibraryNameFullPath.dirCat(theCoverageNames[coverageNumber]));
   }
   
   return result;
}
bool rspfVpfLibrary::getCoverage(const rspfString& name,
                                  rspfVpfCoverage& coverage)
{
  for(int idx = 0; idx < (int)theCoverageNames.size(); ++idx)
    {
      if(theCoverageNames[idx] == name)
	{
	  return getCoverage(idx,
			     coverage);
	}
    }
  return false;
}
bool rspfVpfLibrary::getExtent(rspfVpfExtent& extent)const
{
   bool result = false;
   rspfVpfTable tempTable;
   
    /* Get library extent from Library Attribute Table (LAT) */
/*    char* buf;*/
/*    long int n;*/
/*    double xmin,ymin,xmax,ymax;*/
    int libraryNamePosition, xminPosition, yminPosition;
    int xmaxPosition, ymaxPosition;
    int i;
    bool found;
    row_type row;
    extent_type libextent;
    if(!theDatabase)
    {
       return false;
    }
    rspfFilename file(theDatabase->getLibraryAttributeTable());
    if(!tempTable.openTable(file))
    {
       return result;
    }
    vpf_table_type *table = tempTable.getVpfTableData();
       
    libraryNamePosition = table_pos( "LIBRARY_NAME", *table );
    found = false;
    for (i=1;(i<=tempTable.getNumberOfRows())&&(!found);i++)
    {
       row = read_row( i, *table );
       rspfString libraryName = tempTable.getColumnValueAsString(row, libraryNamePosition);
       libraryName = libraryName.trim();
       if (libraryName == theLibraryName)
       {
          xminPosition = table_pos( "XMIN", *table );
          yminPosition = table_pos( "YMIN", *table );
          xmaxPosition = table_pos( "XMAX", *table );
          ymaxPosition = table_pos( "YMAX", *table );
          libextent.x1 = tempTable.getColumnValueAsString(row, xminPosition).toDouble();
          libextent.y1 = tempTable.getColumnValueAsString(row, yminPosition).toDouble();
          libextent.x2 = tempTable.getColumnValueAsString(row, xmaxPosition).toDouble();
          libextent.y2 = tempTable.getColumnValueAsString(row, ymaxPosition).toDouble();
          found = true;
       }
       else
       {
          result = false;
       }
       free_row( row, *table );
    }
    
    extent =  rspfVpfExtent(libextent);
    return result;
}
rspfFilename rspfVpfLibrary::getPath()const
{
   return theLibraryNameFullPath;
}
rspfString rspfVpfLibrary::getName()const
{
   return theLibraryName;
}
bool rspfVpfLibrary::getCoverageNames(std::vector<rspfString>& coverageNames)const
{
   bool result = true;
   coverageNames = theCoverageNames;
   
   return result;
}
int rspfVpfLibrary::getNumberOfCoverages()const
{
   return theNumberOfCoverages;
}
bool rspfVpfLibrary::hasCoverage(const rspfString& coverageName)const
{
   return (std::find(theCoverageNames.begin(),
                    theCoverageNames.end(),
                    coverageName)!=theCoverageNames.end());
}
rspfString rspfVpfLibrary::getTileName(rspf_int32 id)const
{
   rspfString result;
   if(theLibraryNameFullPath.dirCat("tileref").exists())
   {
      if(theTileNameMap.size() <1)
      {
         setTileNames();
      }
      std::map<rspf_int32, rspfString>::iterator tileNameIter = theTileNameMap.find(id);
      if(tileNameIter != theTileNameMap.end())
      {
         result = (*tileNameIter).second;
      }
   }
   return result;
}
void rspfVpfLibrary::getTileNames(std::vector<rspfString>& tileNames)const
{
   if(theLibraryNameFullPath.dirCat("tileref").exists())
   {
      if(theTileNameMap.size() <1)
      {
         setTileNames();
      }
      std::map<rspf_int32, rspfString>::iterator tileNameIter = theTileNameMap.begin();
      while(tileNameIter != theTileNameMap.end())
      {
         tileNames.push_back((*tileNameIter).second);
         ++tileNameIter;
      }
   }   
}
void rspfVpfLibrary::setCoverageNames()
{
   rspfVpfTable table;
   theCoverageNames.clear();
   if(table.openTable(theLibraryNameFullPath.dirCat("cat")))
   {
      theCoverageNames = table.getColumnValues("COVERAGE_NAME");
   }
}
void rspfVpfLibrary::setTileNames()const
{
   rspfVpfTable table;
   theTileNameMap.clear();
   row_type row;
   if(table.openTable(theLibraryNameFullPath.dirCat("tileref").dirCat("tileref.aft")))
   {
      table.reset();
      const int ROWS = table.getNumberOfRows();
      for (int rowIdx = 1; rowIdx <= ROWS; ++rowIdx)
      {
         row = read_row( rowIdx, *(table.getVpfTableData()) );
         rspf_int32 namePosition   = table.getColumnPosition("TILE_NAME");
         rspf_int32 tileIdPosition = table.getColumnPosition("ID");
         
         rspfString tileName = table.getColumnValueAsString(row,
                                                             namePosition);;
         rspfString tileId   = table.getColumnValueAsString(row,
                                                             tileIdPosition);
         theTileNameMap.insert(make_pair(tileId.toInt32(), tileName.trim()));
         free_row( row, *(table.getVpfTableData()) );
      }
   }
}
