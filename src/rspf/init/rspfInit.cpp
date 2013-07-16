#include <rspf/init/rspfInit.h>
#include <rspf/rspfVersion.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfArgumentParser.h>
#include <rspf/base/rspfApplicationUsage.h>
#include <rspf/base/rspfTraceManager.h>
#include <algorithm>
#include <rspf/base/rspfEnvironmentUtility.h>
#include <rspf/base/rspfGeoidEgm96.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/imaging/rspfImageSourceFactoryRegistry.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspf2dTo2dTransformRegistry.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryRegistry.h>
#include <rspf/imaging/rspfOverviewBuilderFactory.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfImageMetaDataWriterRegistry.h>
#include <rspf/projection/rspfProjectionViewControllerFactory.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfBaseObjectFactory.h>
#include <rspf/base/rspfCustomEditorWindowRegistry.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/plugin/rspfSharedPluginRegistry.h>
#include <rspf/plugin/rspfDynamicLibrary.h>
#include <rspf/font/rspfFontFactoryRegistry.h>
#include <rspf/base/rspfNotifyContext.h>
static rspfTrace traceExec = rspfTrace("rspfInit:exec");
static rspfTrace traceDebug = rspfTrace("rspfInit:debug");
rspfInit* rspfInit::theInstance = 0;
rspfInit::~rspfInit()
{
   theInstance = 0;
}
rspfInit::rspfInit()
    :
       theInitializedFlag(false),
       theAppName(),
       thePreferences(rspfPreferences::instance()),
       theElevEnabledFlag(true),
       thePluginLoaderEnabledFlag(true)
{
}
rspfInit* rspfInit::instance()
{
   if (!theInstance)
   {
      theInstance = new rspfInit();
   }
   return theInstance;
}
void rspfInit::addOptions(rspfArgumentParser& parser)
{
   parser.getApplicationUsage()->addCommandLineOption("-P", "specify a preference file to load");
   parser.getApplicationUsage()->addCommandLineOption("-K", "specify individual keywords to add to the preferences keyword list: name=value");
   parser.getApplicationUsage()->addCommandLineOption("-T", "specify the classes to trace, ex: rspfInit|rspfImage.* \nwill trace rspfInit and all rspfImage classes");
   parser.getApplicationUsage()->addCommandLineOption("--disable-elev", "Will disable the elevation");
   parser.getApplicationUsage()->addCommandLineOption("--disable-plugin", "Will disable the plugin loader");
   
   parser.getApplicationUsage()->addCommandLineOption("--rspf-logfile", "takes a logfile as an argument.  All output messages are redirected to the specified log file.  By default there is no log file and all messages are enabled.");
   parser.getApplicationUsage()->addCommandLineOption("--disable-notify", "Takes an argument. Arguments are ALL, WARN, NOTICE, INFO, FATAL, DEBUG.  If you want multiple disables then just do multiple --disable-notify on the command line.  All argument are case insensitive.  Default is all are enabled.");
   parser.getApplicationUsage()->addCommandLineOption("-V or --version", "Display version information.");
}
/*!****************************************************************************
 * METHOD: rspfInit::initialize()
 *  
 *  Method called from the RSPF application main.
 *  
 *****************************************************************************/
void rspfInit::initialize(int& argc, char** argv)
{
   if( !theInitializedFlag )
   {
      rspfArgumentParser argumentParser(&argc, argv);
      theInstance->initialize(argumentParser);
   }
}
void rspfInit::initialize(rspfArgumentParser& parser)
{
   if(theInitializedFlag)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfInit::initialize(parser):"
            << " Already initialized, returning......"
            << std::endl;
      }
      return;
   }
   theInstance->theAppName  = parser.getApplicationUsage()->getApplicationName();
   theInstance->parseNotifyOption(parser);
   theInstance->thePreferences = rspfPreferences::instance();
      
   theInstance->parseOptions(parser);
   theInstance->initializeDefaultFactories();
   
   if ( theElevEnabledFlag )
   {
      theInstance->initializeElevation();
   }
   theInstance->initializeLogFile();
   
   if(thePluginLoaderEnabledFlag)
   {
      theInstance->initializePlugins();
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspf preferences file: "
         << theInstance->thePreferences->getPreferencesFilename()
         << "\nVersion: " << version()
         << "\nrspfInit::initialize(parser): leaving..." << std::endl;
   }
   
   theInitializedFlag = true;
}
void rspfInit::initialize()
{
   if(theInitializedFlag)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfInit::initialize(): Already initialized, returning......" << std::endl;
      }
      return;
   }
   theInstance->theAppName  = "";
   theInstance->thePreferences = rspfPreferences::instance();
   theInstance->initializeDefaultFactories();
   
   if ( theElevEnabledFlag )
   {
      theInstance->initializeElevation();
   }
   theInstance->initializeLogFile();
   if(thePluginLoaderEnabledFlag)
   {
      theInstance->initializePlugins();
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspf preferences file: "
         << theInstance->thePreferences->getPreferencesFilename()
         << "\nVersion: " << version()
         << "\nrspfInit::initialize() leaving..."
         << std::endl;
   } 
   
   theInitializedFlag = true;
}
void rspfInit::finalize()
{
   
}
/*!****************************************************************************
 *  Prints to stdout the list of command line options that this object parses.
 *****************************************************************************/
void rspfInit::usage()
{
   rspfNotify(rspfNotifyLevel_INFO)
      << "INFORMATION rspfInit::usage():\n"
      << "Additional command-line options available are as follows: "
      << "\n"
      << "\n  -P<pref_filename> -- Allows the user to override the loading "
      << "\n    of the default preferences with their own pref file."
      << "\n"
      << "\n  -K<keyword>[=<value>] -- Allows the user to specify additional"
      << "\n    keyword/value pairs that are added onto the preferences "
      << "\n    previously loaded. Keywords specified here override those in"
      << "\n    the preferences file."
      << "\n"
      << "\n  -T<trace_tag> -- Lets user turn on specific trace flags."
      << "\n"
      << "\n  -S<session_filename> -- Allows user to specify a session file"
      << "\n    to load."
      << "\n  -V or --version -- Outputs version information."
      << std::endl;
   return;
}
bool rspfInit::getElevEnabledFlag() const
{
   return theElevEnabledFlag;
}
void rspfInit::setElevEnabledFlag(bool flag)
{
   theElevEnabledFlag = flag; 
}
void rspfInit::setPluginLoaderEnabledFlag(bool flag)
{
   thePluginLoaderEnabledFlag = flag;  
}
void rspfInit::loadPlugins(const rspfFilename& plugin, const char* options)
{
   if(!thePluginLoaderEnabledFlag) return;
   if(plugin.exists())
   {
      if(plugin.isDir())
      {
         rspfDirectory dir;
         if(dir.open(plugin))
         {
            rspfFilename file;
            
            if(dir.getFirst(file,
                            rspfDirectory::RSPF_DIR_FILES))
            {
               do
               { 
                  rspfSharedPluginRegistry::instance()->registerPlugin(file, options);
               }
               while(dir.getNext(file));
            }
         }
      }
      else
      {
         rspfSharedPluginRegistry::instance()->registerPlugin(plugin, options);
      }
   }
}
void rspfInit::parseOptions(rspfArgumentParser& parser)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfInit::parseOptions: entering..." << std::endl;
   
   std::string tempString;
   rspfArgumentParser::rspfParameter stringParameter(tempString);
   while(parser.read("-P", stringParameter));
   if(tempString != "")
   {
      thePreferences->loadPreferences(rspfFilename(tempString));
   }
   while(parser.read("-K", stringParameter))
   {
      rspfString option = tempString;
      if (option.contains("=") )
      {
         rspfString delimiter = "=";
         rspfString key (option.before(delimiter));
         rspfString value = option.after(delimiter);
         thePreferences->addPreference(key.c_str(), value.c_str());
      }
      else
      {
         rspfString key (option);
         thePreferences->addPreference(key, "");
      }
   }
   while(parser.read("-T", stringParameter))
   {
      rspfTraceManager::instance()->setTracePattern(rspfString(tempString));
   }
   while(parser.read("--rspf-logfile", stringParameter))
   {
      rspfSetLogFilename(rspfFilename(tempString));
   }
   while(parser.read("--disable-notify", stringParameter))
   {
      rspfString tempDownCase = tempString;
      tempDownCase = tempDownCase.downcase();
      if(tempDownCase == "warn")
      {
         rspfDisableNotify(rspfNotifyFlags_WARN);
      }
      else if(tempDownCase == "fatal")
      {
         rspfDisableNotify(rspfNotifyFlags_FATAL);
      }
      else if(tempDownCase == "debug")
      {
         rspfDisableNotify(rspfNotifyFlags_DEBUG);
      }
      else if(tempDownCase == "info")
      {
         rspfDisableNotify(rspfNotifyFlags_INFO);
      }
      else if(tempDownCase == "notice")
      {
         rspfDisableNotify(rspfNotifyFlags_NOTICE);
      }
      else if(tempDownCase == "all")
      {
         rspfDisableNotify(rspfNotifyFlags_ALL);
      }
   }
   if(parser.read("--disable-elev"))
   {
      theElevEnabledFlag = false;
   }
   if(parser.read("--disable-plugin"))
   {
      thePluginLoaderEnabledFlag = false;
   }
   if (parser.read("--version") || parser.read("-V")) 
   {
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "\n" << parser.getApplicationName().c_str() << " " << version() << std::endl;
   }
}
void rspfInit::parseNotifyOption(rspfArgumentParser& parser)
{
   std::string tempString;
   rspfArgumentParser::rspfParameter stringParameter(tempString);
   while(parser.read("--disable-notify", stringParameter))
   {
      rspfString tempDownCase = tempString;
      tempDownCase = tempDownCase.downcase();
      if(tempDownCase == "warn")
      {
         rspfDisableNotify(rspfNotifyFlags_WARN);
      }
      else if(tempDownCase == "fatal")
      {
         rspfDisableNotify(rspfNotifyFlags_FATAL);
      }
      else if(tempDownCase == "debug")
      {
         rspfDisableNotify(rspfNotifyFlags_DEBUG);
      }
      else if(tempDownCase == "info")
      {
         rspfDisableNotify(rspfNotifyFlags_INFO);
      }
      else if(tempDownCase == "notice")
      {
         rspfDisableNotify(rspfNotifyFlags_NOTICE);
      }
      else if(tempDownCase == "all")
      {
         rspfDisableNotify(rspfNotifyFlags_ALL);
      }
   }
}
/*!****************************************************************************
 * METHOD: rspfInit::removeOption()
 *  
 * Removes all characters associated with the indicated option from the
 * command line string.
 *  
 *****************************************************************************/
void rspfInit::removeOption(int& argc,
                             char** argv,
                             int argToRemove)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfInit::removeOption(argc, argv, argToRemove): entering..."
      << std::endl;
   
   for (int i=argToRemove+1; i<argc;  i++)
   {
      argv[i - 1] = argv[i];
   }
   
   argc--;
   argv[argc] = 0;
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfInit::removeOption(argc, argv, argToRemove): leaving..."
      << std::endl;
   return;
}
void rspfInit::initializeDefaultFactories()
{
   rspfObjectFactoryRegistry::instance()->registerFactory(rspfImageSourceFactoryRegistry::instance());
   rspfImageWriterFactoryRegistry::instance();
   rspfDatumFactoryRegistry::instance();
   rspfImageMetaDataWriterRegistry::instance();
   rspfImageHandlerRegistry::instance();
   rspf2dTo2dTransformRegistry::instance();
   rspfImageGeometryRegistry::instance();
   rspfOverviewBuilderFactoryRegistry::instance()->
      registerFactory(rspfOverviewBuilderFactory::instance(), true);
   
   rspfObjectFactoryRegistry::instance()->addFactory(rspfBaseObjectFactory::instance());
   
   rspfProjectionFactoryRegistry::instance();
   rspfObjectFactoryRegistry::instance()->registerFactory(rspfProjectionViewControllerFactory::instance());
   rspfFontFactoryRegistry::instance();
}
void rspfInit::initializePlugins()
{      
   rspfString regExpressionDir =  rspfString("^(") + "plugin.dir[0-9]+)";
   rspfString regExpressionFile =  rspfString("^(") + "plugin.file[0-9]+)";
   const rspfKeywordlist& kwl = thePreferences->preferencesKWL();
   vector<rspfString> keys = kwl.getSubstringKeyList( regExpressionDir );
   rspf_uint32 numberOfDirs = (rspf_uint32)keys.size();
   rspf_uint32 offset = (rspf_uint32)rspfString("plugin.dir").size();
   int idx = 0;
   
   std::vector<int> numberList(numberOfDirs);
   
   rspfFilename userPluginDir = rspfEnvironmentUtility::instance()->getUserOssimPluginDir();
   loadPlugins(userPluginDir);
   if(numberList.size()>0)
   {
      for(idx = 0; idx < (int)numberList.size();++idx)
      {
         rspfString numberStr(keys[idx].begin() + offset,
                               keys[idx].end());
         numberList[idx] = numberStr.toInt();
      }
      
      std::sort(numberList.begin(), numberList.end());
      for(idx=0;idx < (int)numberList.size();++idx)
      {
         rspfString newPrefix = "plugin.dir";
         newPrefix += rspfString::toString(numberList[idx]);
         const char* directory = kwl.find(newPrefix.c_str());
         
         if(directory)
         {
            loadPlugins(rspfFilename(directory));
         }
      }
   }
   keys = kwl.getSubstringKeyList( regExpressionFile );
   
   rspf_uint32 numberOfFiles = (rspf_uint32)keys.size();
   offset = (rspf_uint32)rspfString("plugin.file").size();
   numberList.resize(numberOfFiles);
   if(numberList.size()>0)
   {
      for(idx = 0; idx < (int)numberList.size();++idx)
      {
         rspfString numberStr(keys[idx].begin() + offset,
                               keys[idx].end());
         numberList[idx] = numberStr.toInt();
      }
      
      std::sort(numberList.begin(), numberList.end());   
      for(idx=0;idx < (int)numberList.size();++idx)
      {
         rspfString newPrefix="plugin.file";
         newPrefix += rspfString::toString(numberList[idx]);
         const char* file = kwl.find(newPrefix.c_str());
         
         if(file&&rspfFilename(file).exists())
         {
            loadPlugins(file);
         }
      }
   }
   
   regExpressionFile =  rspfString("^(") + "plugin[0-9]+\\.file)";
   keys = kwl.getSubstringKeyList( regExpressionFile );
   
   numberOfFiles = (rspf_uint32)keys.size();
   offset = (rspf_uint32)rspfString("plugin").size();
   numberList.resize(numberOfFiles);
   
   if(numberList.size()>0)
   {
      for(idx = 0; idx < (int)numberList.size();++idx)
      {
         std::vector<rspfString> splitArray;
         keys[idx].split(splitArray, ".");
         if(splitArray.size())
         {
            keys[idx] = rspfString(splitArray[0].begin(), splitArray[0].begin()+offset);
         }
         rspfString numberStr(splitArray[0].begin() + offset,
                               splitArray[0].end());
         numberList[idx] = numberStr.toInt();
      }
      
      std::sort(numberList.begin(), numberList.end());   
      for(idx=0;idx < (int)numberList.size();++idx)
      {
         rspfString newPrefix = rspfString("plugin")+rspfString::toString(numberList[idx]) + ".";
         const char* file    = kwl.find((newPrefix+"file").c_str());
         const char* options = kwl.find((newPrefix+"options").c_str());
         if(file&&rspfFilename(file).exists())
         {
            loadPlugins(file, options);
         }
      }
   }
   rspfString auto_load_plugins(rspfPreferences::instance()->findPreference("rspf_init.auto_load_plugins"));
   
   if(auto_load_plugins.empty()) auto_load_plugins = "true";
   if(auto_load_plugins.toBool())
   {
      rspfDirectory currentDir(theAppName.path());
      std::vector<rspfFilename> result;
      currentDir.findAllFilesThatMatch(result, "rspf.*plugin.*", rspfDirectory::RSPF_DIR_FILES);
      
      if(result.size())
      {
         rspf_uint32 idx = 0;
         for(idx = 0; idx < result.size(); ++idx)
         {
            rspfSharedPluginRegistry::instance()->registerPlugin(result[idx]);
         }
      }
   }
}
void rspfInit::initializeElevation()
{
   if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfInit::initializeElevation(): Entered..." << std::endl;
   
   const rspfKeywordlist& KWL = thePreferences->preferencesKWL();
   rspfFilename appPath = theAppName.path();
   {
	   rspfFilename geoid = appPath.dirCat("geoids");
	   geoid = geoid.dirCat("geoid1996");
	   geoid = geoid.dirCat("egm96.grd");
	   if(geoid.exists())
	   {
         rspfGeoid* geoidPtr = new rspfGeoidEgm96(geoid);
         if (geoidPtr->getErrorStatus() == rspfErrorCodes::RSPF_OK)
         {
		    rspfGeoidManager::instance()->addGeoid(geoidPtr);
		 }
	   }
   }
   rspfGeoidManager::instance()->loadState(KWL);
   
   rspfFilename elevation = appPath.dirCat("elevation");
   if(elevation.exists())
   {
      rspfElevManager::instance()->loadElevationPath(elevation);
   }
   rspfString regExpression =  rspfString("^(") + "elevation_source[0-9]+.)";
   vector<rspfString> keys =
   KWL.getSubstringKeyList( regExpression );
   if(!keys.empty())
   {
      rspfNotify(rspfNotifyLevel_WARN) << "Please specify elevation_source keywords with the new prefix\n"
                                         << "of elevation_manager.elevation_source....\n";
      thePreferences->preferencesKWL().addPrefixToKeysThatMatch("elevation_manager.", regExpression);
   }
   rspfElevManager::instance()->loadState(KWL, "elevation_manager.");
   
   if (traceDebug()) rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfInit::initializeElevation(): leaving..." << std::endl;
}
void rspfInit::initializeLogFile()
{
   rspfFilename logFile;
   rspfGetLogFilename(logFile);
   if ( (logFile.size() == 0) && thePreferences )
   {
      const char* lookup =
         thePreferences->preferencesKWL().find("rspf.log.file");
      if (lookup)
      {
         logFile = lookup;
         rspfSetLogFilename(logFile);
      }
   }
}
rspfString rspfInit::version() const
{
   rspfString versionString;
#ifdef RSPF_VERSION
   versionString += RSPF_VERSION;
#else
   versionString += "Version ?.?.?";
#endif
   
   versionString += " ";
#ifdef RSPF_BUILD_DATE
   versionString += RSPF_BUILD_DATE;
#else
   versionString += "(yyyymmdd)";
#endif
   return versionString;
}
rspfFilename rspfInit::appName()const
{
   return theAppName;
}
rspfInit::rspfInit(const rspfInit& /* obj */ )
{}       
void rspfInit::operator=(const rspfInit& /* rhs */) const
{}
