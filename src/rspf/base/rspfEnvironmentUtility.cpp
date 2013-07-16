#include <rspf/base/rspfEnvironmentUtility.h>
#include <cstdlib>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  define RSPF_ENVIRONMENT_UTILITY_UNIX 0
#  include <direct.h>
#else
#  define RSPF_ENVIRONMENT_UTILITY_UNIX 1
#endif

rspfEnvironmentUtility* rspfEnvironmentUtility::theInstance=0;

rspfEnvironmentUtility::rspfEnvironmentUtility()
{
   rspfFilename dir = getUserOssimPluginDir();
   
   if(!dir.empty())
   {
      thePluginSearchPath.push_back(dir);
   }
   
   dir = getInstalledOssimPluginDir();
   if(!dir.empty())
   {   
      thePluginSearchPath.push_back(dir);
   }

   
}

rspfEnvironmentUtility* rspfEnvironmentUtility::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfEnvironmentUtility;
   }

   return theInstance;
}

rspfString rspfEnvironmentUtility::getEnvironmentVariable(const rspfString& variable)const
{
   rspfString result;
   char* lookup = std::getenv(variable.c_str());
   // getenv returns NULL if not found.
   if (lookup)
   {
      result = (const char*)lookup;
   }
   return result;
}

rspfFilename rspfEnvironmentUtility::getUserOssimSupportDir()const
{
   rspfFilename result = getUserDir();
   
#if RSPF_ENVIRONMENT_UTILITY_UNIX
#   ifdef __APPLE__
   result = result.dirCat("Library/Application Support/rspf");
   
#   else
   result = result.dirCat(".rspf");
#   endif
#else
   result = result.dirCat("Application Data\\rspf");
#endif
   
   return result;
}

rspfString   rspfEnvironmentUtility::getUserName()const
{
#if RSPF_ENVIRONMENT_UTILITY_UNIX
   return getEnvironmentVariable("USER");
#else
   return getEnvironmentVariable("USERNAME");
#endif
}

rspfFilename rspfEnvironmentUtility::getUserDir()const
{
   rspfFilename result;

#if RSPF_ENVIRONMENT_UTILITY_UNIX
   result = rspfFilename(getEnvironmentVariable("HOME"));
#else
   result =rspfFilename(getEnvironmentVariable("USERPROFILE"));
#endif

   return result;
}

rspfFilename rspfEnvironmentUtility::getUserOssimPreferences()const
{
   rspfFilename result = getUserOssimSupportDir();

   if(result == "")
   {
      return "";
   }

   result = result.dirCat("preferences");
   
   return result;
}

rspfFilename rspfEnvironmentUtility::getUserOssimPluginDir()const
{
   rspfFilename result = getUserOssimSupportDir();

   if(result != "")
   {
      result = result.dirCat("plugins");
   }
   
   return result;
}

rspfFilename rspfEnvironmentUtility::getInstalledOssimSupportDir()const
{
   rspfFilename result;
#if RSPF_ENVIRONMENT_UTILITY_UNIX
#   ifdef __APPLE__
   result = "/Library/Application Support/rspf";
#   else
   result = "/usr/share/rspf";
   if(!result.exists())
   {
      result = "/usr/local/share/rspf";
   }
#   endif
#else
   // NEED the windows test here.
#endif
   if(result != "")
   {
      if(!result.exists())
      {
         result = "";
      }
   }

   return result;
}

rspfFilename rspfEnvironmentUtility::getInstalledOssimPluginDir()const
{
   rspfFilename result = getInstalledOssimSupportDir();

   //Need generic unix plugin location
#if RSPF_ENVIRONMENT_UTILITY_UNIX
#   ifndef __APPLE__
   return "";
#   endif
#endif
   if((result!="")&&result.exists())
   {
      result = result.dirCat("plugins");
   }
   else
   {
      result = "";
   }

   return result;

}

rspfFilename rspfEnvironmentUtility::getInstalledOssimPreferences()const
{
   rspfFilename result = getInstalledOssimSupportDir();

   
   if((result!="")&&result.exists())
   {
      result = result.dirCat("preferences");
   }
   else
   {
      result = "";
   }

   if(!result.exists())
   {
      result = "";
   }
   
   return result;
   
}

rspfFilename rspfEnvironmentUtility::getCurrentWorkingDir()const
{
   rspfFilename result;

#if RSPF_ENVIRONMENT_UTILITY_UNIX
   result = getEnvironmentVariable("PWD");
#else
   char buf[512];
   _getcwd(buf, 512);
   result = buf;
#endif

   return result;
}

rspfFilename rspfEnvironmentUtility::searchAllPaths(const rspfFilename& file)const
{
   rspfFilename result;

   result = findPlugin(file);
   if(!result.empty()) return result;

   result = findData(file);


   return result;
}

void rspfEnvironmentUtility::addDataSearchPath(const rspfFilename& path)
{
   theDataSearchPath.push_back(path);
}

void rspfEnvironmentUtility::addDataSearchPathToFront(const rspfFilename& path)
{
   theDataSearchPath.insert(theDataSearchPath.begin(), path);
}

void rspfEnvironmentUtility::addPluginSearchPath(const rspfFilename& path)
{
   thePluginSearchPath.push_back(path);
}

void rspfEnvironmentUtility::addPluginSearchPathToFront(const rspfFilename& path)
{
   thePluginSearchPath.insert(thePluginSearchPath.begin(), path);
}

rspfFilename rspfEnvironmentUtility::findPlugin(const rspfFilename& plugin)const
{
   for(rspfEnvironmentUtility::FilenameListType::const_iterator iter = thePluginSearchPath.begin();
       iter != thePluginSearchPath.end();
       ++iter)
   {
      rspfFilename temp = iter->dirCat(plugin);
      if(temp.exists())
      {
         return temp;
      }
   }

   return "";
}

rspfFilename rspfEnvironmentUtility::findData(const rspfFilename& data)const
{
   for(rspfEnvironmentUtility::FilenameListType::const_iterator iter = theDataSearchPath.begin();
       iter != theDataSearchPath.end();
       ++iter)
   {
      rspfFilename temp = iter->dirCat(data);
      if(temp.exists())
      {
         return temp;
      }
   }

   return "";
}


rspfEnvironmentUtility::FilenameListType& rspfEnvironmentUtility::getPluginSearchPath()
{
   return thePluginSearchPath;
}

const rspfEnvironmentUtility::FilenameListType& rspfEnvironmentUtility::getPluginSearchPath()const
{
   return thePluginSearchPath;
}

rspfEnvironmentUtility::FilenameListType& rspfEnvironmentUtility::getDataSearchPath()
{
   return theDataSearchPath;
}

const rspfEnvironmentUtility::FilenameListType& rspfEnvironmentUtility::getDataSearchPath()const
{
   return theDataSearchPath;
}

// Hidden copy constructor.
rspfEnvironmentUtility::rspfEnvironmentUtility(
   const rspfEnvironmentUtility& /* obj */)
{
}

// Hidden operator=
const rspfEnvironmentUtility& rspfEnvironmentUtility::operator=(
   const rspfEnvironmentUtility& /* rhs */)
{
   return *this;
}
