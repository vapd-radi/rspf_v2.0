#ifndef rspfVpfDatabase_HEADER
#define rspfVpfDatabase_HEADER
#include <vector>
using namespace std;
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>
class rspfVpfLibrary;
class rspfVpfDatabase
{
public:
   /*!
    * Initializes everything to null;
    */
   rspfVpfDatabase();
   /*!
    * Destroys any allocated space.
    */
   virtual ~rspfVpfDatabase();
   /*!
    * This will open a database.  This will either
    * be a dht(database header table) table or a
    * library attribut table (lat) file.  Both of these
    * files should exist in a vpf database.
    */
   virtual bool openDatabase(const rspfFilename& filename);
   void closeDatabase();
   rspfFilename getPath()const;
   long getNumberOfLibraries()const;
   /*!
    *
    */
   rspfVpfLibrary* getLibrary(unsigned long libraryNumber);
   /*!
    *
    */
   rspfVpfLibrary* getLibrary(const rspfString& name);
   bool isOpened()const;
   
   vector<rspfString> getLibraryNames()const;
   vector<rspfString> getLibraryNamesFullPath()const;
   rspfFilename getLibraryAttributeTable()const;
protected:
   void initializeLibraryList();
   void deleteLibraryList();
   
   /*!
    * This is the complete path to the Database Header Table (dht file).
    */
   rspfFilename theDatabaseHeaderTable;
   /*!
    * This is the complete path to the library attribute table (lat file).
    */
   rspfFilename theLibraryAttributeTable;
   /*!
    * Will hold a constructed set of libraries.
    */
   vector<rspfVpfLibrary*> theVpfLibraryList;
   /*!
    * This jhust specifies whether the database has been opened or not.
    */
   bool  theOpenedFlag;
};
#endif
