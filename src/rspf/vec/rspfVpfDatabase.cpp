#include <rspf/vec/rspfVpfDatabase.h>
#include <rspf/vec/rspfVpfDatabaseHeader.h>
#include <rspf/vec/rspfVpfDatabaseHeaderTableValidator.h>
#include <rspf/vec/rspfVpfLibraryAttributeTableValidator.h>
#include <rspf/vec/rspfVpfTable.h>
#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/base/rspfErrorCodes.h>
rspfVpfDatabase::rspfVpfDatabase()
   :theDatabaseHeaderTable(""),
    theLibraryAttributeTable(""),
    theOpenedFlag(false)
{
}
rspfVpfDatabase::~rspfVpfDatabase()
{
   deleteLibraryList();
   
   closeDatabase();
}
bool rspfVpfDatabase::openDatabase(const rspfFilename& filename)
{
   bool result = true;
   rspfFilename tempFilename = filename;
   tempFilename.convertBackToForwardSlashes();
   theDatabaseHeaderTable   = rspfFilename(tempFilename.path()).dirCat("dht");
   theLibraryAttributeTable = rspfFilename(tempFilename.path()).dirCat("lat");
   if(theDatabaseHeaderTable.exists() &&
      theLibraryAttributeTable.exists())
   {
      rspfVpfTable table;
      result = table.openTable(theDatabaseHeaderTable);
      if(result&&
         !rspfVpfDatabaseHeaderTableValidator().isValid(table))
      {
         result =  false;
      }
      
      result = table.openTable(theLibraryAttributeTable);
      if((result) &&!rspfVpfLibraryAttributeTableValidator().isValid(table))
      {
         result = false;
      }
   }
   else
   {
      result = false;
   }
   if(result)
   {
      theOpenedFlag = true;
      initializeLibraryList();
   }
   
   return result;
}
void rspfVpfDatabase::closeDatabase()
{
   theLibraryAttributeTable = "";
   theDatabaseHeaderTable   = "";
   theOpenedFlag = false;
   deleteLibraryList();
}
rspfFilename rspfVpfDatabase::getPath()const
{
   return theLibraryAttributeTable.path();
}
long rspfVpfDatabase::getNumberOfLibraries()const
{
   return (long)theVpfLibraryList.size();
}
rspfVpfLibrary* rspfVpfDatabase::getLibrary(unsigned long libraryNumber)
{
   if(libraryNumber < theVpfLibraryList.size())
   {
      return theVpfLibraryList[libraryNumber];
   }
   
   return NULL;
}
rspfVpfLibrary* rspfVpfDatabase::getLibrary(const rspfString& name)
{
   for(long index = 0; index < (long)theVpfLibraryList.size(); ++index)
   {
      if(theVpfLibraryList[index]->getName() == name)
      {
         return theVpfLibraryList[index];
      }
   }
   
   return NULL;
}
bool rspfVpfDatabase::isOpened()const
{
   return theOpenedFlag;
}
vector<rspfString> rspfVpfDatabase::getLibraryNames()const
{
   rspfVpfTable table;
   if(table.openTable(theLibraryAttributeTable))
   {
      return table.getColumnValues("LIBRARY_NAME");
   }
   
   return vector<rspfString>();
}
vector<rspfString> rspfVpfDatabase::getLibraryNamesFullPath()const
{
   vector<rspfString> result;
   
   rspfVpfTable table;
   if(table.openTable(theLibraryAttributeTable))
   {
      result = table.getColumnValues("LIBRARY_NAME");
   }
   for(long index=0; index < (long)result.size();index++)
   {
      result[index] = rspfString(theLibraryAttributeTable.path()) +
                      rspfString("/") +
                      result[index];
   }
   
   return result;
}
rspfFilename rspfVpfDatabase::getLibraryAttributeTable()const
{
   return theLibraryAttributeTable;
}
void rspfVpfDatabase::initializeLibraryList()
{
   deleteLibraryList();
   vector<rspfString> libraryNames = getLibraryNames();
   for(long index = 0; index < (long)libraryNames.size(); index++)
   {
      rspfVpfLibrary *library = new rspfVpfLibrary;
      library->openLibrary(this,
                           libraryNames[index],
                           getPath().dirCat(libraryNames[index]));
      theVpfLibraryList.push_back(library);
   }
}
void rspfVpfDatabase::deleteLibraryList()
{
   for(long idx = 0; idx <(long)theVpfLibraryList.size(); idx++)
   {
      delete theVpfLibraryList[idx];
      theVpfLibraryList[idx] = 0;
   }
   theVpfLibraryList.clear();
}
