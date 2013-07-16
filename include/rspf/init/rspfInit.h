#ifndef rspfInit_HEADER
#define rspfInit_HEADER 1
#include <rspf/base/rspfFilename.h>
class rspfPreferences;
class rspfArgumentParser;
class RSPFDLLEXPORT rspfInit
{
public:
   ~rspfInit();
   /*!
    * Returns the static instance of an rspfInit object. This is of no use
    * until non-static methods are implemented.
    */
   static rspfInit* instance();
   void addOptions(rspfArgumentParser& parser);
   /*!
    * METHOD: initialize()
    * This method shall be called from the application's main module with
    * the command-line arguments. Every RSPF application should have the
    * following line early in the main module:
    *
    *   rspfInit::initialize(argc, argv);
    *
    * OR
    *
    *   rspfInit::instance()->initialize(argc, argv);
    *
    * The two forms are functionally identical. Pick the latter form if you
    * like to type. The argv command line options are parsed and may be
    * stripped. the value of argc will be adjusted to account for stripped
    * options.
    */
   void initialize(int& argc, char** argv);
   void initialize(rspfArgumentParser& parser);
   
   void initialize();
   void finalize();
   /*!
    * METHOD: usage()
    * Prints to stdout the list of command line options that this object parses
    */
   void usage();
   /** @return theElevEnabledFlag */
   bool getElevEnabledFlag() const;
   /**
    * @brief Sets theElevEnabledFlag.
    * @param flag If true rspfElevManager will be initialized. Set to false
    * to NOT initialize the rspfElevManager from preferences.
    * Default in class is true.
    */
   void setElevEnabledFlag(bool flag);
   void setPluginLoaderEnabledFlag(bool flag);
   /**
    * Can take a file or a directory.  If a directory is givien then it will check all files in
    * the directory and add each file that is detected to be a valid plugin.
    *
    * @param plugin Is the filename of the plugin to load
    * @param options Is a keywordlist of plugin specific options
    
    */ 
   void loadPlugins(const rspfFilename& plugin, const char* options=0);
   
   void initializePlugins();
   void initializeDefaultFactories();
   void initializeElevation();
   /**
    * @brief Initializes log file from preferences keyword lookup
    * of "rspf.log.file" if log file has not already been set and
    * keyword is set in preferences.
    *
    * So this should be called after parse options as the --rspf-logfile
    * should override any preferences setting.
    */
   void initializeLogFile();
   /**
    * @return The version in the form of:
    * "version major.minor.release (yyyymmdd)"
    * where
    * yyyymmdd is the build date.
    *
    * e.g. "version 1.7.0 (20071003)"
    */
   rspfString version() const;
   rspfFilename appName()const;
protected:
   /** protected default constructor. */
   rspfInit();
   /** Hidden from use copy constructor. */
   rspfInit(const rspfInit& obj);
   
   /** Hidden from use assignment operator. */
   void operator=(const rspfInit& rhs) const;
   
   void parseOptions(rspfArgumentParser& parser);
   void parseNotifyOption(rspfArgumentParser& parser);
   /*!
    * METHOD: removeOptions()
    * Utility for stripping from argv all characters associated with a
    * particular option:
    */
   void removeOption(int&   argc, 
                     char** argv,
                     int    argToRemove);
   
   static rspfInit*  theInstance;
   bool               theInitializedFlag;  
   rspfFilename      theAppName;
   rspfPreferences*  thePreferences;
   bool               theElevEnabledFlag;
   bool               thePluginLoaderEnabledFlag;
};
#endif
