#include <cmath>
#include <gdal_priv.h>
#include <rspfGdalOverviewBuilder.h>
#include <rspfGdalTiledDataset.h>
#include <rspfGdalDataset.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageSourceSequencer.h>
RTTI_DEF1(rspfGdalOverviewBuilder,
          "rspfGdalOverviewBuilder",
          rspfOverviewBuilderBase);
static const char* OVR_TYPE[] = { "unknown",
                                  "gdal_tiff_nearest",
                                  "gdal_tiff_average",
                                  "gdal_hfa_nearest",
                                  "gdal_hfa_average" };
static const rspfTrace traceDebug(
   rspfString("rspfGdalOverviewBuilder:debug"));
rspfGdalOverviewBuilder::rspfGdalOverviewBuilder()
   :
   theDataset(0),
   theOutputFile(),
   theOverviewType(rspfGdalOverviewTiffAverage),
   theLevels(0),
   theGenerateHfaStatsFlag(false)
{
}
rspfGdalOverviewBuilder::~rspfGdalOverviewBuilder()
{
   if ( theDataset )
   {
      delete theDataset;
      theDataset = 0;
   }
}
void rspfGdalOverviewBuilder::setOutputFile(const rspfFilename& file)
{
   theOutputFile = file;
}
rspfFilename rspfGdalOverviewBuilder::getOutputFile() const
{
   if (theOutputFile == rspfFilename::NIL)
   {
      if (theDataset)
      {
         if (theDataset->getImageHandler())
         {
            rspfFilename outputFile =
               theDataset->getImageHandler()->getFilename();
            switch (theOverviewType)
            {
               case rspfGdalOverviewHfaNearest:
               case rspfGdalOverviewHfaAverage:  
                  outputFile.setExtension(getExtensionFromType());
                  break;
               default:
                  outputFile += ".ovr";
                  break;
            }
            return outputFile;
         }
      }
   }
   
   return theOutputFile;
}
bool rspfGdalOverviewBuilder::open(const rspfFilename& file)
{
   if (theDataset)
   {
      delete theDataset;
   }
   theDataset = new rspfGdalDataset;
   return theDataset->open(file);
}
bool rspfGdalOverviewBuilder::setInputSource(rspfImageHandler* imageSource)
{
   if ( !imageSource )
   {
      return false;
   }
   
   if (theDataset)
   {
      delete theDataset;
   }
   theDataset = new rspfGdalDataset();
   theDataset->setImageHandler(imageSource);
   return true;
}
bool rspfGdalOverviewBuilder::setOverviewType(const rspfString& type)
{
   if(type == OVR_TYPE[rspfGdalOverviewTiffNearest])
   {
      theOverviewType = rspfGdalOverviewTiffNearest;
   }
   else if(type == OVR_TYPE[rspfGdalOverviewTiffAverage])
   {
      theOverviewType = rspfGdalOverviewTiffAverage;
   }
   else if(type == OVR_TYPE[rspfGdalOverviewHfaNearest])
   {
      theOverviewType = rspfGdalOverviewHfaNearest;
   }
   else if(type == OVR_TYPE[rspfGdalOverviewHfaAverage])
   {
      theOverviewType = rspfGdalOverviewHfaAverage;
   }
   else
   {
      return false;
   }
   return true;
}
rspfString rspfGdalOverviewBuilder::getOverviewType() const
{
   return rspfString(OVR_TYPE[theOverviewType]);
}
void rspfGdalOverviewBuilder::getTypeNameList(
   std::vector<rspfString>& typeList)const
{
   typeList.push_back(
      rspfString(OVR_TYPE[rspfGdalOverviewTiffNearest]));
   typeList.push_back(
      rspfString(OVR_TYPE[rspfGdalOverviewTiffAverage]));
   typeList.push_back(
      rspfString(OVR_TYPE[rspfGdalOverviewHfaNearest]));
   typeList.push_back(
      rspfString(OVR_TYPE[rspfGdalOverviewHfaAverage]));
}
bool rspfGdalOverviewBuilder::execute()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalOverviewBuilder::execute entered..."
         << endl;
   }
   bool result = false;
   
   if (!theDataset || !theDataset->getImageHandler())
   {
      return result;
   }
   rspfFilename overviewFile = getOutputFile();
   if (theDataset->getImageHandler()->getFilename() == overviewFile)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Source image file and overview file cannot be the same!"
         << std::endl;
      return result;
   }
   if (theGenerateHfaStatsFlag)
   {
      theDataset->setGdalAcces(GA_Update);
      theDataset->initGdalOverviewManager();
      if (generateHfaStats() == false)
      {
         cerr << " generateHfaStats failed..." << endl;
      }
      else
      {
      }
   }
   theDataset->initGdalOverviewManager();
   
   rspfString pszResampling = getGdalResamplingType();
   rspfIrect bounds = theDataset->getImageHandler()->getBoundingRect();
   rspf_uint32 minBound = rspf::min( bounds.width(), bounds.height() );
   rspf_int32 numberOfLevels = 0;
   if (theLevels.size())
   {
      numberOfLevels = theLevels.size();
   }
   else
   {
      rspf_uint32 stopDim = getOverviewStopDimension();
      while (minBound > stopDim)
      {
         minBound = minBound / 2;
         ++numberOfLevels;
      }
      
      if (numberOfLevels == 0)
      {
         return result; // nothing to do.
      }
   }
   
   rspf_int32* levelDecimationFactor = new rspf_int32[numberOfLevels];
   rspf_uint32 idx;
   if (theLevels.size())
   {
      for (idx = 0; idx < theLevels.size(); ++idx)
      {
         levelDecimationFactor[idx] = theLevels[idx];
      }
   }
   else
   {
      levelDecimationFactor[0] = 2;
      for(idx = 1; idx < static_cast<rspf_uint32>(numberOfLevels); ++idx)
      {
         levelDecimationFactor[idx] = levelDecimationFactor[idx-1]*2;
      }
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalOverviewBuilder::execute DEBUG:"
         << "\noverviewFilename:   " << overviewFile
         << "\npszResampling:      " << pszResampling
         << "\nnumberOfLevels:     " << numberOfLevels
         << endl;
      for(idx = 0; idx < static_cast<rspf_uint32>(numberOfLevels); ++idx)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "levelDecimationFactor["
            << idx << "]: " << levelDecimationFactor[idx]
            << endl;
      }
   }
   CPLErr eErr = CE_None;
   if ( (theOverviewType == rspfGdalOverviewHfaAverage) ||
        (theOverviewType == rspfGdalOverviewHfaNearest) )
   {
      CPLSetConfigOption("USE_RRD", "YES");
   }
   if( theDataset->BuildOverviews( pszResampling.c_str(), 
                                   numberOfLevels,
                                   levelDecimationFactor,
                                   0,
                                   0,
                                   GDALTermProgress,
                                   0 ) != CE_None )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Overview building failed." << std::endl;
   }
   if ( levelDecimationFactor )
   {
      delete [] levelDecimationFactor;
      levelDecimationFactor = 0;
   }
   
   if (eErr  == CE_None )
   {
      result = true;
   }
   if (result == true)
   {
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "Wrote file:  " << overviewFile << std::endl;
   }
   return result;
}
void rspfGdalOverviewBuilder::setProperty(rspfRefPtr<rspfProperty> property)
{
   if (property.valid() == false)
   {
      return;
   }
   rspfString s = property->getName();
   s.downcase();
   if ( s == "levels" )
   {
      rspfString value;
      property->valueToString(value);
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfGdalOverviewBuilder::setProperty DEBUG:"
            << std::endl;
      }
      theLevels.clear();
      std::vector<rspfString> v1 = value.split(",");
      for (rspf_uint32 i = 0; i < v1.size(); ++i)
      {
         rspf_int32 level = v1[i].toInt32();
         theLevels.push_back(level);
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "level[" << i << "]:  " << level << std::endl;
         }
      }
   }
   else if ( s == "generate-hfa-stats" )
   {
      theGenerateHfaStatsFlag = true;
   }
}
void rspfGdalOverviewBuilder::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(rspfString("levels"));
   propertyNames.push_back(rspfString("generate-hfa-stats"));
}
std::ostream& rspfGdalOverviewBuilder::print(std::ostream& out) const
{
   out << "rspfGdalOverviewBuilder::print"
       << "\nfilename: " << theOutputFile.c_str()
       << "\noverview_type: "
       << OVR_TYPE[theOverviewType]
       << "\nresampling type: " << getGdalResamplingType()
       << std::endl;
   return out;
}
bool rspfGdalOverviewBuilder::generateHfaStats() const
{
   
   bool result = false;
   if (!theDataset)
   {
      return result;
   }
   if (!theDataset->getImageHandler())
   {
      return result;
   }
   
   rspfFilename sourceImageFile =
      theDataset->getImageHandler()->getFilename();
   if (sourceImageFile.empty())
   {
      return result;
   }
   GDALDriverH hDriver = GDALGetDriverByName( "HFA" );
   if (!hDriver)
   {
      return false;
   }
   GDALDatasetH hDataset = theDataset;
   GDALDatasetH	hOutDS   = 0; 
   int bStrict = true;
   
   char** papszCreateOptions = 0;
   GDALProgressFunc pfnProgress = GDALTermProgress;
   
   rspfString s = "DEPENDENT_FILE=";
   s += sourceImageFile.file(); // Must not have absolute path...
   
   papszCreateOptions = CSLAddString( papszCreateOptions, "AUX=YES");
   papszCreateOptions = CSLAddString( papszCreateOptions, "STATISTICS=YES");
   papszCreateOptions = CSLAddString( papszCreateOptions, s.c_str() );
   hOutDS = GDALCreateCopy( hDriver,
                            getOutputFile().c_str(),
                            hDataset, 
                            bStrict,
                            papszCreateOptions, 
                            pfnProgress,
                            0 );
   CSLDestroy( papszCreateOptions );
   if( hOutDS != 0 )
   {
      GDALClose( hOutDS );
   }
   
   return true;
}
rspfString rspfGdalOverviewBuilder::getGdalResamplingType() const
{
   rspfString result;
   switch (theOverviewType)
   {
      case rspfGdalOverviewTiffNearest:
      case rspfGdalOverviewHfaNearest:
         result = "nearest";
         break;
      case rspfGdalOverviewTiffAverage:
      case rspfGdalOverviewHfaAverage:
         result = "average";
         break;
      case rspfGdalOverviewType_UNKNOWN:
         result = "unknown";
         break;
   }
   return result;
}
rspfString rspfGdalOverviewBuilder::getExtensionFromType() const
{
   rspfString result;
   switch (theOverviewType)
   {
      case rspfGdalOverviewHfaNearest:
      case rspfGdalOverviewHfaAverage:  
         result = "aux";
         break;
      default:
         result = "ovr";
         break;
   }
   return result;
}
rspfObject* rspfGdalOverviewBuilder::getObject()
{
   return this;
}
const rspfObject* rspfGdalOverviewBuilder::getObject() const
{
   return this;
}
bool rspfGdalOverviewBuilder::canConnectMyInputTo(
   rspf_int32 index,
   const rspfConnectableObject* obj) const
{
   if ( (index == 0) &&
        PTR_CAST(rspfImageHandler, obj) )
   {
      return true;
   }
   return false;
}
