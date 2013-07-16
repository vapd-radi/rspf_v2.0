//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//   Contains declaration of class rspfPreferences. This class provides
//   a static keywordlist for global preferences. Objects needing access to
//   application-wide global parameters shall do so through this class.
//
// SOFTWARE HISTORY:
//>
//   23Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************

#ifndef rspfPreferences_HEADER
#define rspfPreferences_HEADER

#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>

/*!****************************************************************************
 *
 * CLASS:  rspfPreferences
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfPreferences
{
public:
   // This is only here so we can swig wrap this.  The destructor should never be called directly.
   ~rspfPreferences();
   /*!
    * METHOD: instance()
    * The static singleton instance of this object is accessed via this method:
    */
   static rspfPreferences* instance();

   /*!
    * METHOD: loadPreferences()
    * These methods clear the current preferences and load either the default
    * preferences file or the specified file. Returns TRUE if loaded properly:
    */
   bool loadPreferences();
   bool loadPreferences(const rspfFilename& pathname);

   /*!
    * METHOD: savePrefences()
    * This method permits saving the preferences file to the default location
    * or to a specified location:
    */
   bool savePreferences() const;
   bool savePreferences(const rspfFilename& pathname);

   /*!
    * METHOD: findPreference()
    * Performs a lookup for the specified keyword in the preferences KWL:
    */
   const char* findPreference(const char* key) const {return theKWL.find(key);}


   /*!
    * METHOD: addPreference()
    * Inserts keyword/value pair into the in-memory preferences KWL. It does
    * save to disk. App must do a savePreferences() for changes to be saved.
    */
   void addPreference(const char* key,
                      const char* value);

   /*!
    * METHOD: preferencesKWL()
    * An alternative to utilizing findPreference(), for objects derived from
    * rspfObject, is to access the preferences KWL with this method
    * (typically during construction) and provide it to the object's
    * loadState() method.
    */
   const rspfKeywordlist& preferencesKWL() const { return theKWL; }
   
   /*!
    * METHOD: preferencesKWL()
    * An alternative to utilizing findPreference(), for objects derived from
    * rspfObject, is to access the preferences KWL with this method
    * (typically during construction) and provide it to the object's
    * loadState() method.
    */
   rspfKeywordlist& preferencesKWL()  { return theKWL; }
   
   void addPreferences(const rspfKeywordlist& kwl,
                       const char* prefix=0,
                       bool stripPrefix=true);

   /** @return The preference filename. */ 
   rspfFilename getPreferencesFilename() const;
   
protected:
   /*!
    * Override the compiler default constructors:
    */
   rspfPreferences();
   rspfPreferences(const rspfPreferences&) {}

   void operator = (const rspfPreferences&) const {}

   static rspfPreferences* theInstance;
   rspfKeywordlist         theKWL;
   rspfFilename            thePrefFilename;
   mutable bool             theInstanceIsModified;
};

#endif
