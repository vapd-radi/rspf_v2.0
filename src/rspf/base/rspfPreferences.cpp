//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//   Contains implementation of class rspfPreferences. This class provides
//   a static keywordlist for global preferences. Objects needing access to
//   application-wide global parameters shall do so through this class.
//
// SOFTWARE HISTORY:
//>
//   23Apr2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************

#include <cstdlib> /* for getenv() */

#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfNotify.h>

//RTTI_DEF1(rspfPreferences, "rspfPreferences" , rspfObject)

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfPreferences:exec");
static rspfTrace traceDebug ("rspfPreferences:debug");

static const char* PREF_FILE_ENV_VAR_NAME = "RSPF_PREFS_FILE";

rspfPreferences* rspfPreferences::theInstance = NULL;

rspfPreferences::rspfPreferences()
{
   /*!
    * If $(env_var_name) is found in the preferences file, 
    * expand it in place.
    */
   theKWL.setExpandEnvVarsFlag( true );

   loadPreferences();
}

rspfPreferences::~rspfPreferences()
{
        theInstance = NULL;
}

/*!****************************************************************************
 * METHOD: rspfPreferences::instance()
 *  
 *  This is the method by which run-time objects access this singleton instance
 *  
 *****************************************************************************/
rspfPreferences* rspfPreferences::instance()
{
   /*!
    * Simply return the instance if already created:
    */
   if (theInstance)
      return theInstance;

   /*!
    * Create the static instance of this class:
    */
   theInstance = new rspfPreferences();

   return theInstance;
}

/*!****************************************************************************
 * METHOD: loadPreferences()
 *  
 *  Loads the preferences file specified in the runtime environment.
 *  
 *****************************************************************************/
bool rspfPreferences::loadPreferences()
{
   static const char MODULE[] = "rspfPreferences::loadPreferences()";
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: " << MODULE << " entering...\n";
   }

   bool parsed_ok = false;
   
   /*!
    * Fetch preferences file name from environment:
    */
   char* pref_filename = getenv(PREF_FILE_ENV_VAR_NAME);

   if (pref_filename)
   {
      /*!
       * Load the preferences file into the static keywordlist object:
       */
      thePrefFilename = pref_filename;
      parsed_ok = theKWL.addFile(pref_filename);

      /*!
       * Check for error opening KWL:
       */
      if (!parsed_ok)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "WARNING: " << MODULE
            << ", an error was encountered loading the prefererences "
            << "file at \"" << thePrefFilename << "\" as specified by the "
            << "environment variable \"" << PREF_FILE_ENV_VAR_NAME << "\"."
            << "Preferences were not loaded.\n";
      }
   }

//   else
//   {
//       if (traceDebug())
//       {
//          // No ENV var found. Print warning:
//          CLOG << "WARNING: the preferences file environment variable \""
//               << PREF_FILE_ENV_VAR_NAME << "\" is not defined. No preferences "
//               << "were loaded. The environment variable should be set to "
//               << "the full path to the preferences keywordlist file desired."
//               << endl;
//       }
//   }
   
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: " << MODULE << "returning...\n";
   }
   return parsed_ok;
}

/*!****************************************************************************
 * METHOD: loadPreferences(filename)
 *  
 *  Loads the preferences file specified in the arg.
 *  
 *****************************************************************************/
bool rspfPreferences::loadPreferences(const rspfFilename& pathname)
{
   static const char MODULE[] = "rspfPreferences::loadPreferences(filename)";
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: " << MODULE << ", entering...\n";
   }

   bool parsed_ok;

   /*!
    * First clear the existing KWL:
    */
   theKWL.clear();
   theInstanceIsModified = true;
   
   /*!
    * Load the preferences file into the static keywordlist object:
    */
   thePrefFilename = pathname;
   parsed_ok = theKWL.addFile(pathname);

   /*!
    * Check for error opening KWL:
    */
   if (!parsed_ok)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "WARNING: " << MODULE
         << ", an error was encountered loading the prefererences "
         << "file at \"" << pathname << "\". Preferences were not "
         << "loaded.\n";
   }

   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: " << MODULE<< ", returning...\n";
   }
   
   return parsed_ok;
}

/*!****************************************************************************
 * METHOD: rspfPreferences::savePreferences()
 *  
 *  Saves KWL to the current filename.
 *  
 *****************************************************************************/
bool rspfPreferences::savePreferences() const
{
   static const char MODULE[] = "rspfPreferences::savePreferences()";
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: " << MODULE << ", entering...\n";
   }
   
   bool success = true;
   
   /*!
    * Save the file to current preferences filename:
    */
   if (theInstanceIsModified)
   {
      theKWL.write(thePrefFilename);
      theInstanceIsModified = false;
   }

   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG:" << MODULE << ", returning...\n";
   }
   
   return success;
}

/*!****************************************************************************
 * METHOD: rspfPreferences::savePreferences(filename)
 *  
 *  Saves KWL to the specified filename.
 *  
 *****************************************************************************/
bool rspfPreferences::savePreferences(const rspfFilename& pathname)
{
   static const char MODULE[] = "rspfPreferences::savePreferences()";
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: "<< MODULE << ", entering...\n";
   }
   
   bool success = true;
   
   /*!
    * Save the file to the specified preferences filename:
    */
   theKWL.write(pathname);

   thePrefFilename = pathname;
   theInstanceIsModified = false;
   
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: " << MODULE << ", returning...\n";
   }
   
   return success;
}

void rspfPreferences::addPreference(const char* key, const char* value)
{
   theKWL.add(key, value, true);
   theInstanceIsModified = true;
}

void rspfPreferences::addPreferences(const rspfKeywordlist& kwl,
                                      const char* prefix,
                                      bool stripPrefix)
{
   theKWL.add(kwl, prefix, stripPrefix);
   theInstanceIsModified = true;
}

rspfFilename rspfPreferences::getPreferencesFilename() const
{
   return thePrefFilename;
}
