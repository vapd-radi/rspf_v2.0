//*****************************************************************************
// FILE: rspfElevManager.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of class rspfElevManager. This object provides a
//   single interface to an imaging chain for accessing multiple elevation
//   sources. This object owns one or more elevation sources in an ordered
//   list. When queried for an elevation at a particular point, it searches
//   the available sources for the best result, instantiating new sources if
//   necessary.
//
// SOFTWARE HISTORY:
//>
//   13Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
#ifndef rspfElevManager_HEADER
#define rspfElevManager_HEADER
#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/elevation/rspfElevSource.h>
#include <rspf/elevation/rspfElevationDatabase.h>
#include <OpenThreads/ReadWriteMutex>
class RSPF_DLL rspfElevManager : public rspfElevSource
{
public: 
   typedef std::vector<rspfRefPtr<rspfElevationDatabase> > ElevationDatabaseListType;
   
   class RSPF_DLL ConnectionStringVisitor : public rspfVisitor
   {
   public:
      ConnectionStringVisitor(const rspfString& value):m_connectionString(value){}
      virtual rspfRefPtr<rspfVisitor> dup()const{return new ConnectionStringVisitor(*this);}
      const rspfString& getConnectionString()const{return m_connectionString;}
      virtual void visit(rspfObject* obj);
      rspfElevationDatabase* getElevationDatabase(){return m_database.get();} 
      
   protected:
      rspfString m_connectionString;
      rspfRefPtr<rspfElevationDatabase> m_database;
   };
   
   virtual ~rspfElevManager();
   
   /**
    * METHOD: instance()
    * Implements singelton pattern
    */
   static rspfElevManager* instance();
   
   virtual double getHeightAboveEllipsoid(const rspfGpt& gpt);
   virtual double getHeightAboveMSL(const rspfGpt& gpt);


   virtual bool pointHasCoverage(const rspfGpt& /*gpt*/) const
   {
      std::cout << "rspfElevManager::pointHasCoverage(): NOT IMPLEMENTED!!!\n";
      return false;
   }
   virtual double getMeanSpacingMeters() const
   {
      std::cout << "rspfElevManager::pointHasCoverage(): NOT IMPLEMENTED AND SHOULD NOT BE USED AT THIS LEVEL!!!\n";
      return 1.0;
   }
   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const;
  
   rspf_uint32 getNumberOfElevationDatabases()const
   {
      return (rspf_uint32)m_elevationDatabaseList.size();
   }
   rspfElevationDatabase* getElevationDatabase(rspf_uint32 idx)
   {
      return m_elevationDatabaseList[idx].get();
   }
   const rspfElevationDatabase* getElevationDatabase(rspf_uint32 idx)const
   {
      return m_elevationDatabaseList[idx].get();
   }
   ElevationDatabaseListType& getElevationDatabaseList()
   {
      return m_elevationDatabaseList;
   }
   const ElevationDatabaseListType& getElevationDatabaseList()const
   {
      return m_elevationDatabaseList;
   }
   void addDatabase(rspfElevationDatabase* database);
   bool loadElevationPath(const rspfFilename& path);
   
   void setDefaultHeightAboveEllipsoid(double meters) {m_defaultHeightAboveEllipsoid=meters;}
   void setElevationOffset(double meters) {m_elevationOffset=meters;}
   double getElevationOffset() const { return m_elevationOffset; }
   
   void getOpenCellList(std::vector<rspfFilename>& list) const;

   void setUseGeoidIfNullFlag(bool flag)
   {
      m_useGeoidIfNullFlag = flag;
   }
   
   bool getUseGeoidIfNullFlag()const
   {
      return m_useGeoidIfNullFlag;
   }
   void clear();
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual void accept(rspfVisitor& visitor);
   
protected:
   rspfElevManager();
   void loadStandardElevationPaths();
   
   static rspfElevManager* m_instance;
   ElevationDatabaseListType m_elevationDatabaseList;
   rspf_float64 m_defaultHeightAboveEllipsoid;
   rspf_float64 m_elevationOffset;
   
   // if an elevation is returned that's null for ellipsoid then use the geoid manager to calculate a shift
   //
   bool          m_useGeoidIfNullFlag; 
   
   
   /**
    * I have tried the readwrite lock interfaces but have found it unstable.  I am using the standard Mutex
    * and it seems to be much more stable across all platforms.  More testing needs to occur for the ReadWriteMutex.
    * For now we will use Mutex.
    */
   OpenThreads::Mutex m_mutex;
};

#endif
