//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id: rspfOrthoIgen.cpp 21962 2012-11-30 15:44:32Z dburken $


#include <rspf/parallel/rspfOrthoIgen.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfException.h>
#include <rspf/init/rspfInit.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/imaging/rspfBandSelector.h>
#include <rspf/imaging/rspfCacheTileSource.h>
#include <rspf/imaging/rspfGeoAnnotationSource.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageRenderer.h>
#include <rspf/imaging/rspfHistogramRemapper.h>
#include <rspf/imaging/rspfImageMosaic.h>
#include <rspf/imaging/rspfBlendMosaic.h>
#include <rspf/imaging/rspfBandMergeSource.h>
#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfOrthoImageMosaic.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfMaskFilter.h>
#include <rspf/imaging/rspfTiffWriter.h>
#include <rspf/imaging/rspfEsriShapeFileInterface.h>
#include <rspf/imaging/rspfTilingRect.h>
#include <rspf/imaging/rspfTilingPoly.h>
#include <rspf/imaging/rspfGeoPolyCutter.h>
#include <rspf/imaging/rspfEastingNorthingCutter.h>
#include <rspf/imaging/rspfHistogramEqualization.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfGeoAnnotationPolyObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiPolyObject.h>
#include <rspf/imaging/rspfPixelFlipper.h>
#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/parallel/rspfIgen.h>
#include <rspf/parallel/rspfMpi.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfEpsgProjectionFactory.h>

#include <sstream>

// In Windows, standard output is ASCII by default. 
// Let's include the following in case we have
// to change it over to binary mode.
#if defined(_WIN32)
#  include <io.h>
#  include <fcntl.h>
#endif


static rspfTrace traceDebug("rspfOrthoIgen:debug");
static rspfTrace traceLog("rspfOrthoIgen:log");

static const char* AUTOGENERATE_HISTOGRAM_KW = "autogenerate_histogram";

using namespace rspf;

//*************************************************************************************************
// Parses the file info as specified in the command line or src file. The file info is a '|'-
// delimited string with filename and additional attributes such as entry and band numbers.
//*************************************************************************************************
bool rspfOrthoIgen::parseFilename(const rspfString& file_spec, bool decodeEntry)
{
   rspfSrcRecord src_record;

   std::vector<rspfString> fileInfos = file_spec.split("|");
   unsigned int num_fields = (unsigned int) fileInfos.size();
   unsigned int field_idx = 0;

   if (num_fields == 0)
      return false;

   // First field is the actual filename:
   src_record.setFilename(fileInfos[field_idx]);
   ++field_idx;

   // Next field depends on whether an entry is being decoded:
   if ((field_idx < num_fields) && decodeEntry)
   {
      src_record.setEntryIndex(fileInfos[field_idx].trim().toInt32());
      ++field_idx;
   }

   // The rest of the fields can appear in any order:
   while (field_idx < num_fields)
   {
      rspfString active_field (fileInfos[field_idx].trim());
      rspfString downcased_field (active_field);
      downcased_field.downcase();
      ++field_idx;

      // Check for overview file spec:
      rspfFilename filename (active_field);
      if (filename.contains(".ovr") || filename.isDir())
      {
         src_record.setSupportDir(filename.path());
      }
      else if (filename.contains(".mask") || filename.isDir())
      {
         src_record.setSupportDir(filename.path());
      }

      // else check for auto-minmax histogram stretch:
      else if ((downcased_field == "auto-minmax") || downcased_field.contains("std-stretch"))
      {
         src_record.setHistogramOp(downcased_field);
      }

      // Otherwise, this must be a band specification. Band numbers begin with 1:
      else
      {
         // multiple bands delimited by comma:
         std::vector<rspfString> bandsStr = active_field.split(",");
         std::vector<rspf_uint32> bands;
         for (unsigned int i = 0; i < bandsStr.size(); i++)
         {
            int band = bandsStr[i].toInt32() - 1;
            if (band >= 0)
               bands.push_back((rspf_uint32)band);
         }
         src_record.setBands(bands);
      }

   } // end of while loop parsing fileInfos spec

   theSrcRecords.push_back(src_record);
   return true;
}

//*************************************************************************************************
// Constructor
//*************************************************************************************************
rspfOrthoIgen::rspfOrthoIgen()
   :
   rspfIgen(),
   theDeltaPerPixelUnit(RSPF_UNIT_UNKNOWN),
   theDeltaPerPixelOverride(rspf::nan(), rspf::nan()),
   theProjectionType(RSPF_UNKNOWN_PROJECTION),
   theProjectionName(""),
   theGeoScalingLatitude(rspf::nan()),
   theCombinerType("rspfImageMosaic"),
   theResamplerType("nearest neighbor"),
   theWriterType(""),
   theTemplateView(""),
   theTilingTemplate(""),
   theTilingFilename(""),
   theChainTemplate(""),
   theCombinerTemplate(""),
   theAnnotationTemplate(""),
   theWriterTemplate(""),
   theSupplementaryDirectory(""),
   theSlaveBuffers("2"),
   theCutOriginType(rspfOrthoIgen::RSPF_CENTER_ORIGIN),
   theCutOrigin(rspf::nan(), rspf::nan()),
   theCutDxDy(rspf::nan(), rspf::nan()),
   theCutOriginUnit(RSPF_UNIT_UNKNOWN),
   theCutDxDyUnit(RSPF_UNIT_UNKNOWN),
   theLowPercentClip(rspf::nan()),
   theHighPercentClip(rspf::nan()),
   theStdDevClip(-1),
   theUseAutoMinMaxFlag(false),
   theClipToValidRectFlag(false),   
   theReaderProperties(),
   theWriterProperties(),   
   theTargetHistoFileName(),
   theProductFilename(),
   theReferenceProj(0),
   theMaskShpFile(""),
   theCacheExcludedFlag(false),
   theOutputRadiometry(""),
   thePixelAlignment(RSPF_PIXEL_IS_AREA) // will revert to "point" upon first occurrence in source list
{
   // Determine default behavior of clip from preferences:
   rspfString flag = rspfPreferences::instance()->findPreference("orthoigen.clip_to_valid_rect");
   if (!flag.empty())
      theClipToValidRectFlag = flag.toBool();

   thePixelReplacementMode = rspfPreferences::instance()->findPreference("orthoigen.flip_null_pixels"); 
   return;
}

//*************************************************************************************************
// Initializes the argument parser
//*************************************************************************************************
void rspfOrthoIgen::addArguments(rspfArgumentParser& argumentParser)
{
   // These are in ALPHABETIC ORDER. Please keep it that way.

   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--annotate", "annotation keyword list");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--chain-template","Specify an external file that contains chain information");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--clamp-pixels <min> <max>","Specify the min and max allowed pixel values. All values "
      "outside of this get mapped to their corresponding clamp value.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--clip-pixels <min> <max>","Causes all pixel values between min and max (inclusive)"
      " to be mapped to the null pixel value. Min and max can be equal for mapping a single value."
      " See also related option \"--replacement-mode\" for additional explanation.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--clip-to-valid-rect <true|false>","When true, any requested cut rect is clipped by the "
      "valid image bounding rect to minimize null border pixels. If false, the output will "
      "correspond to the cut rect as close as possible given the product projection. This option "
      "overrides the rspf_preferences setting. If no cut options are supplied, this option is "
      "ignored.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--combiner-template","Specify an external file that contains combiner information");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--combiner-type","Specify what mosaic to use, rspfImageMosiac or rspfFeatherMosaic or "
      "osimBlendMosaic ... etc");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--cut-bbox-en","Specify the min easting, min northing, max easting, max northing");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--cut-bbox-ll","Specify the min lat and min lon and max lat and maxlon <minLat> <minLon> "
      "<maxLat> <maxLon>");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--cut-center-ll","Specify the center cut in lat lon space.  Takes two argument <lat> <lon>");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--cut-pixel-width-height","Specify cut box's width and height in pixels");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--cut-radius-meters","Specify the cut distance in meters.  A bounding box for the cut will "
      "be produced");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--degrees","Specifies an override for degrees per pixel. Takes either a single value "
      "applied equally to x and y directions, or two values applied correspondingly to x then y.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--geo","Defaults to a geographic image chain with GSD = to the input.  Origin of latitude is"
      "on the equator.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--geo-scaled","Takes a latitude as an argument for purpose of scaling in the longitude "
      "direction so that the pixels will appear nearly square in ground space at specified "
      "latitude. Implies a geographic projection.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--hist-auto-minmax","uses the automatic search for the best min and max clip values."
      " Incompatible with other histogram options.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--hist-match","Takes one image filename argument for target histogram to match."
      " Incompatible with other histogram options.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--hist-std-stretch","Specify histogram stretch as a standard deviation from the mean as"
      " <int>, where <int> is 1, 2, or 3."
      " Incompatible with other histogram options.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--hist-stretch","Specify in normalized percent the low clip and then the high clip value"
      " as <low.dd> <hi.dd>."
      " Incompatible with other histogram options.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--input-proj","Makes the view equal to the input.  If more than one file then the first is "
      "taken");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--mask","Specify the ESRI shape file with polygons to clip the image");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--meters","Specifies an override for the meters per pixel. Takes either a single value "
      "applied equally to x and y directions, or two values applied correspondingly to x then y.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--no-cache","Excludes the cache from the input image chain(s). Necessary as a workaround "
      " for inconsistent cache behavior for certain image types.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--output-radiometry","Specifies the desired product's pixel radiometry type. Possible "
      "values are: U8, U11, U16, S16, F32. Note this overrides the deprecated option \"scale-to"
      "-8-bit\".");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--reader-prop","Passes a name=value pair to the reader(s) for setting it's property.  Any "
      "number of these can appear on the line.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--replacement-mode <mode>","Specify how to treat multi-band imagery when providing "
      "clip-pixels and/or clamp-pixels settings. Possible values are: REPLACE_BAND_IF_TARGET | "
      "REPLACE_BAND_IF_PARTIAL_TARGET | REPLACE_ALL_BANDS_IF_ANY_TARGET | "
      "REPLACE_ONLY_FULL_TARGETS.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--resample-type","Specify what resampler to use, nearest neighbor, bilinear, cubic");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--scale-to-8-bit","Scales the output to unsigned eight bits per band. This option has been"
      " deprecated by the newer \"--output-radiometry\" option.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--slave-buffers","number of slave tile buffers for mpi processing (default = 2)");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--srs","specify an output reference frame/projection. Example: --srs EPSG:4326");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--stdout","Output the image to standard out.  This will return an error if writer does not "
      "support writing to standard out.  Callers should combine this with the --rspf-logfile "
      "option to ensure output image stream does not get corrupted.  You must still pass an output "
      "file so the writer type can be determined like \"dummy.png\".");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--supplementary-directory or --support","Specify the supplementary directory path where "
      "overviews, histograms and external geometries are located");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "-t or --thumbnail", "thumbnail size");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--tiling-template","Specify an external file that contains tiling information");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--threads [n]","Indicates multi-threaded process using optionally-specified number of threads");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--utm","Defaults to a utm image chain with GSD = to the input");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--view-template","Specify an external file that contains view information");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "-w or --writer","Specifies the output writer.  Default uses output file extension to "
      "determine writer.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--wkt","specify an output reference frame/projection that is in a wkt format.  Must have the"
      " rspfgdal_plugin compiled");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--writer-prop","Passes a name=value pair to the writer for setting it's property.  Any "
      "number of these can appear on the line.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
      "--writer-template","Specify an external file that contains tiling information"); 
}

//*************************************************************************************************
// Initializes this objects data members given the command line args
//*************************************************************************************************
void rspfOrthoIgen::initialize(rspfArgumentParser& argumentParser)
{
   if(traceDebug())
   {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfOrthoIgen::initialize DEBUG:"
            << "Entered..... " 
            << std::endl;
   }
   double tempDouble;
   double tempDouble2;
   double tempDouble3;
   double tempDouble4;
   rspfString tempString;
   unsigned int tempUint;
   rspfArgumentParser::rspfParameter stringParam(tempString);
   rspfArgumentParser::rspfParameter doubleParam(tempDouble);
   rspfArgumentParser::rspfParameter doubleParam2(tempDouble2);
   rspfArgumentParser::rspfParameter doubleParam3(tempDouble3);
   rspfArgumentParser::rspfParameter doubleParam4(tempDouble4);
   rspfArgumentParser::rspfParameter uintParam(tempUint);
   theCutOriginType = rspfOrthoIgen::RSPF_CENTER_ORIGIN;
   theCutOrigin.makeNan();
   theCutDxDy.makeNan();
   theCutOriginUnit   = RSPF_UNIT_UNKNOWN;
   theCutDxDyUnit     = RSPF_UNIT_UNKNOWN;
   theLowPercentClip  = rspf::nan();
   theHighPercentClip = rspf::nan();
   double minX=rspf::nan(), minY=rspf::nan(), maxX=rspf::nan(), maxY=rspf::nan();
   theUseAutoMinMaxFlag = false;
   theDeltaPerPixelOverride.makeNan();
   theDeltaPerPixelUnit = RSPF_UNIT_UNKNOWN;
   theCacheExcludedFlag = false;
   theClampPixelMin = rspf::nan();
   theClampPixelMax = rspf::nan();
   theClipPixelMin = rspf::nan();
   theClipPixelMax = rspf::nan();
   
   if(argumentParser.read("--annotate", stringParam))
   {
      theAnnotationTemplate = rspfFilename(tempString);
   }
   if(argumentParser.read("-t", stringParam)   ||
      argumentParser.read("--thumbnail", stringParam))
   {
      rspfString comma (",");
      if (tempString.contains(comma))
      {
         theThumbnailSize.x = tempString.before(comma).toInt();
         theThumbnailSize.y = tempString.after(comma).toInt();
      }
      else
      {
         theThumbnailSize.x = tempString.toInt();
         theThumbnailSize.y = 0;
      }
      theBuildThumbnailFlag = true;
   }

   theReaderProperties.clear();
   while(argumentParser.read("--reader-prop", stringParam))
   {
      std::vector<rspfString> splitArray;
      tempString.split(splitArray, "=");
      if(splitArray.size() == 2)
      {
         theReaderProperties.insert(std::make_pair(splitArray[0], splitArray[1]));
      }
   }

   if(argumentParser.read("-w", stringParam)   ||
      argumentParser.read("--writer", stringParam))
   {
      theWriterType = tempString;
   }

   theWriterProperties.clear();
   
   while(argumentParser.read("--writer-prop", stringParam))
   {
      std::vector<rspfString> splitArray;
      tempString.split(splitArray, "=");
      if(splitArray.size() == 2)
      {
         theWriterProperties.insert(std::make_pair(splitArray[0], splitArray[1]));
      }
   }
         
   if(argumentParser.read("--slave-buffers", stringParam))
   {
      theSlaveBuffers = tempString;
   }
   if(argumentParser.read("--cut-center-ll", doubleParam, doubleParam2))
   {
      theCutOrigin.lat = tempDouble;
      theCutOrigin.lon = tempDouble2;
      theCutOriginUnit = RSPF_DEGREES;
      theCutOriginType = rspfOrthoIgen::RSPF_CENTER_ORIGIN;
   }
   if(argumentParser.read("--cut-radius-meters", doubleParam))
   {
      theCutDxDy.x = tempDouble;
      theCutDxDy.y = tempDouble;
      theCutDxDyUnit = RSPF_METERS;
   }
   if(argumentParser.read("--cut-bbox-ll", doubleParam, doubleParam2, doubleParam3, doubleParam4))
   {
      minY = tempDouble;
      minX = tempDouble2;
      maxY = tempDouble3;
      maxX = tempDouble4;
      theCutOriginUnit = RSPF_DEGREES;
      theCutOriginType = rspfOrthoIgen::RSPF_UPPER_LEFT_ORIGIN;
      theCutOrigin.lat = maxY;
      theCutOrigin.lon = minX;
      theCutDxDy.lat   = (maxY-minY);
      if ( (maxX < 0.0) && (minX >= 0.0) )
      {
         //---
         // Min is eastern hemisphere, max is western. Crossed the international date line.
         // Add 360 to make it positive.
         //
         // Note no check for just max < min here???  Perhaps throw exception.(drb)
         //---
         maxX += 360.0;
      }      
      theCutDxDy.lon   = (maxX-minX);
      theCutDxDyUnit   = RSPF_DEGREES;
   }
   if(argumentParser.read("--cut-bbox-en", doubleParam, doubleParam2, doubleParam3, doubleParam4))
   {
      minX = tempDouble;
      minY = tempDouble2;
      maxX = tempDouble3;
      maxY = tempDouble4;
      theCutOriginUnit = RSPF_METERS;
      theCutOriginType = rspfOrthoIgen::RSPF_UPPER_LEFT_ORIGIN;
      theCutOrigin.x = minX;
      theCutOrigin.y = maxY;
      theCutDxDy.x   = (maxX-minX);
      theCutDxDy.y   = (maxY-minY);
      theCutDxDyUnit   = RSPF_METERS;
   }
   if(argumentParser.read("--cut-pixel-width-height", doubleParam, doubleParam2))
   {
      if((rspf::isnan(minX) == false)&&
         (rspf::isnan(minY) == false)&&
         (rspf::isnan(maxX) == false)&&
         (rspf::isnan(maxY) == false))
      {
         theDeltaPerPixelOverride = rspfDpt(theCutDxDy.x/(tempDouble-1),
                                             theCutDxDy.y/(tempDouble2-1));
         theDeltaPerPixelUnit     = theCutDxDyUnit;
      }
      else
      {
         theCutOrigin.makeNan();
         rspfNotify(rspfNotifyLevel_WARN) << "Can't have option --cut-pixel-width-height without --cut-bbox-ll" << std::endl;
      }
   }
   
   int num_params = argumentParser.numberOfParams("--degrees", doubleParam);
   if (num_params == 1)
   {
      argumentParser.read("--degrees", doubleParam);
      theDeltaPerPixelUnit = RSPF_DEGREES;
      theDeltaPerPixelOverride.x = tempDouble;
      theDeltaPerPixelOverride.y = tempDouble;
   }
   else if (num_params == 2)
   {
      argumentParser.read("--degrees", doubleParam, doubleParam2);
      theDeltaPerPixelUnit = RSPF_DEGREES;
      theDeltaPerPixelOverride.x = tempDouble;
      theDeltaPerPixelOverride.y = tempDouble2;
   }

   // The three histogram options are mutually exclusive:
   bool histo_op_selected = false;
   if(argumentParser.read("--hist-match", stringParam))
   {
      rspfFilename target_image (tempString);
      histo_op_selected = true;
      
      // Check for histogram matching request and initialize for that:
      if (target_image.isReadable())
      {
         // Establish target histogram file:
         theTargetHistoFileName = target_image;
         theTargetHistoFileName.setExtension("his");
         if (!theTargetHistoFileName.isReadable())
         {
            rspfNotify(rspfNotifyLevel_NOTICE)<<"Target histogram file <" << theTargetHistoFileName 
               << "> not found. Cannot perform histogram matching." << std::endl;
            theTargetHistoFileName.clear();
         }
      }
   }
   if(argumentParser.read("--hist-stretch", doubleParam, doubleParam2))
   {
      if (histo_op_selected)
      {
         rspfNotify(rspfNotifyLevel_WARN) << "Cannot specify nore than one histogram operation. "
            " Ignoring --hist-stretch option." << std::endl;
      }
      else
      {
         theLowPercentClip = tempDouble;
         theHighPercentClip = tempDouble2;
         histo_op_selected = true;
      }
   }
   if(argumentParser.read("--hist-std-stretch", stringParam))
   {
      if (histo_op_selected)
      {
         rspfNotify(rspfNotifyLevel_WARN) << "Cannot specify nore than one histogram operation. "
            " Ignoring --hist-stretch option." << std::endl;
      }
      else
      {
         theStdDevClip = tempString.toInt32();
         histo_op_selected = true;
         if ((theStdDevClip < 1) || (theStdDevClip > 3))
         {
            rspfNotify(rspfNotifyLevel_WARN) << "Invalid standard deviation value provided with"
               " --hist-std-stretch option. Only 1,2, or 3 allowed. Ignoring option."<< std::endl;
         }
      }
   }
   if(argumentParser.read("--hist-auto-minmax"))
   {
      if (histo_op_selected)
      {
         rspfNotify(rspfNotifyLevel_WARN) << "Cannot specify nore than one histogram operation. "
            " Ignoring --hist-auto-minmax option." << std::endl;
      }
      else
         theUseAutoMinMaxFlag = true;
   }

   num_params = argumentParser.numberOfParams("--meters", doubleParam);
   if (num_params == 1)
   {
      argumentParser.read("--meters", doubleParam);
      theDeltaPerPixelUnit = RSPF_METERS;
      theDeltaPerPixelOverride.x = tempDouble;
      theDeltaPerPixelOverride.y = tempDouble;
   }
   else if (num_params == 2)
   {
      argumentParser.read("--meters", doubleParam, doubleParam2);
      theDeltaPerPixelUnit = RSPF_METERS;
      theDeltaPerPixelOverride.x = tempDouble;
      theDeltaPerPixelOverride.y = tempDouble2;
   }

   if(argumentParser.read("--no-cache"))
   {
      theCacheExcludedFlag = true;
   }

   if(argumentParser.read("--output-radiometry", stringParam))
   {
      theOutputRadiometry = tempString;
   }

   if(argumentParser.read("--scale-to-8-bit"))
   {
      if (theOutputRadiometry.empty())
         theOutputRadiometry = "U8";
   }

   if (argumentParser.read("--stdout"))
   {
#if defined(_WIN32)
      // In Windows, cout is ASCII by default. 
      // Let's change it over to binary mode.
      int result = _setmode( _fileno(stdout), _O_BINARY );
      if( result == -1 )
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfOrthoIgen::initialize WARNING:"
            << "\nCannot set standard output mode to binary."
            << std::endl;
         return;
      }
#endif

      theStdoutFlag = true;
   }
   
   if(argumentParser.read("--writer-template", stringParam))
   {
      theWriterTemplate = tempString;
   }
   if(argumentParser.read("--tiling-template", stringParam))
   {
      theTilingTemplate = rspfFilename(tempString);
   }
   if(argumentParser.read("--chain-template", stringParam))
   {
      theChainTemplate = rspfFilename(tempString);
   }
   if(argumentParser.read("--combiner-template", stringParam))
   {
      theCombinerTemplate = rspfFilename(tempString);
   }
   
   theGeoScalingLatitude = rspf::nan();

   if (argumentParser.read("--utm"))
   {
      theProjectionType = RSPF_UTM_PROJECTION;
      theProjectionName = "rspfUtmProjection";
   }
   else if(argumentParser.read("--geo"))
   {
      theProjectionType = RSPF_GEO_PROJECTION;
      theProjectionName = "rspfEquDistCylProjection";
      theGeoScalingLatitude = 0.0;
   }
   else if(argumentParser.read("--input-proj"))
   {
      theProjectionType = RSPF_INPUT_PROJECTION;
   }
   else if (argumentParser.read("--srs", stringParam))
   {
      theCrsString=tempString;
      theProjectionType = RSPF_SRS_PROJECTION;
   }

   if(argumentParser.read("--view-template", stringParam))
   {
      theTemplateView = rspfFilename(tempString);
      theProjectionType = RSPF_EXTERNAL_PROJECTION;
   }

   if(argumentParser.read("--geo-scaled", doubleParam))
   {
      theProjectionType = RSPF_GEO_PROJECTION;
      theProjectionName = "rspfEquDistCylProjection";
      if ( (tempDouble < 90.0) && (tempDouble > -90.0) )
      {
         theGeoScalingLatitude = tempDouble;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfOrthoIgen::initialize WARNING:"
            << "\nLatitude out  of range!  Must be between -90 and 90."
            << std::endl;
      }
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfOrthoIgen::initialize DEBUG:"
            << "\ngeographicOriginOfLatitude:  " << theGeoScalingLatitude
            << std::endl;
      }
   }
   if(argumentParser.read("--combiner-type", stringParam))
      theCombinerType = tempString;

   if(argumentParser.read("--resample-type", stringParam))
   {
      theResamplerType = tempString;
   }
   if(argumentParser.read("--supplementary-directory", stringParam) ||
      argumentParser.read("--support", stringParam))
   {
      theSupplementaryDirectory = rspfFilename(tempString);
   }

   if (argumentParser.read("--clip-to-valid-rect", stringParam))
   {
      theClipToValidRectFlag = tempString.toBool();
   }

   if(argumentParser.read("--mask", stringParam))
   {
     theMaskShpFile = tempString;
   }

   // Pixel flipper control options:
   if (argumentParser.read("--clip-pixels", doubleParam, doubleParam2))
   {
      theClipPixelMin = tempDouble;
      theClipPixelMax = tempDouble2;
   }
   if (argumentParser.read("--clamp-pixels", doubleParam, doubleParam2))
   { 
      theClampPixelMin = tempDouble;
      theClampPixelMax = tempDouble2;
   }
   if (argumentParser.read("--replacement-mode", stringParam))
   { 
      thePixelReplacementMode = tempString;
   }

   // Threading:
   num_params = argumentParser.numberOfParams("--threads", uintParam);
   if (num_params == 0)   // No param means system decides optimal thread count
   {
      argumentParser.read("--threads");
      theThreadCount = 0; // Flags system-resolved
   }
   else if (num_params == 1)
   {
      argumentParser.read("--threads", uintParam);
      theThreadCount = (rspf_uint32) tempUint; 
   }

   if(traceDebug())
   {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfOrthoIgen::initialize DEBUG:"
            << "Leaving..... " 
            << std::endl;
   }
}

//*************************************************************************************************
// Adds any file specifications to the files list
//*************************************************************************************************
void rspfOrthoIgen::addFiles(rspfArgumentParser& argumentParser,
                              bool withDecoding,
                              rspf_uint32 startIdx)
{
   rspf_uint32 idx = startIdx;
   rspf_uint32 last_idx = argumentParser.argc()-1;
   while(argumentParser.argv()[idx] && (idx < last_idx))
   {
     rspfString file_spec = argumentParser.argv()[idx];
     if (file_spec.contains(".src"))
     {
        // input file spec provided via src file. Need to parse it:
        addSrcFile(rspfFilename(file_spec));
     }
     else
     {
        // Filename with optional switches explicitly provided on command line:
        parseFilename(file_spec, withDecoding);
     }
     ++idx;
   }

   // The last filename left on the command line should be the product filename:
   theProductFilename = argumentParser.argv()[last_idx];
}

//*************************************************************************************************
// Performs the top-level management of image generation
//*************************************************************************************************
bool rspfOrthoIgen::execute()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfOrthoIgen::execute DEBUG: Entered ..."
         << std::endl;
   }
//   double start=0, stop=0;

   if(theSrcRecords.size() < 1)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfOrthoIgen::execute WARNING: No filenames to process"
         << std::endl;
      return false;
   }

   if (!theCrsString.empty() && !theProductFilename.empty())
   {
     if ((theProductFilename.ext().upcase() == "KMZ" || theProductFilename.ext().upcase() == "KML") 
       && theCrsString.upcase() != "EPSG:4326")
     {
       rspfNotify(rspfNotifyLevel_FATAL)
         << "rspfOrthoIgen::execute ERROR: Unsupported projection for kmz or kml"
         << std::endl;
       return false;
     }
   }

   if(rspfMpi::instance()->getRank() == 0)
   {
      try
      {
         setupIgenChain();
      }
      catch (const rspfException& e)
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << e.what() << std::endl;
         }
         throw; // re-throw exception
      }

      if (traceLog())
      {
         generateLog();
      }
   }

   try
   {
      // theProductProjection->print(cout) << endl;
      outputProduct();
   }
   catch(const rspfException& e)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << e.what() << std::endl;
      }
      throw; // re-throw
   }
   
   return true;
}

//*************************************************************************************************
// METHOD
//*************************************************************************************************
void rspfOrthoIgen::clearFilenameList()
{
   theSrcRecords.clear();
}

//*************************************************************************************************
// Parses the .src file specified in the command line. These contain an alternate specification
// of input file and associated attributes as a KWL.
//*************************************************************************************************
void rspfOrthoIgen::addSrcFile(const rspfFilename& src_file)
{
   if (!src_file.isReadable())
      return;

   rspfKeywordlist src_kwl;
   src_kwl.setExpandEnvVarsFlag(true);
   if ( src_kwl.addFile(src_file) == false ) return;

   unsigned int image_idx = 0;
   // int entry = -1;

   // Loop to read all image file entries:
   double sum_weights = 0;
   while (true)
   {
      rspfSrcRecord src_record(src_kwl, image_idx++);
      if (!src_record.valid()) 
         break;

      // Check for the presence of separate RGB file specs in this SRC record. This indicates 
      // special processing. (comment added OLK 01/11)
      if (src_record.isRgbData())
      {
         for (rspf_uint32 rgb_index = 0; rgb_index < 3; rgb_index++)
         {
            // This call creates another band-specific rspfSrcRecord that is pushed onto 
            // theSrcRecords vector data member. (comment added OLK 01/11)
            if (parseFilename(src_record.getRgbFilename(rgb_index), true))
            {
               // The parseFilename call pushes the R, G, or B band onto the back of theSrcRecords 
               // vector. Set some additional attributes on this last entry. (OLK 01/11)
               theSrcRecords.back().setRgbDataBool(true);
               theSrcRecords.back().setHistogramOp(src_record.getRgbHistogramOp(rgb_index));
               theSrcRecords.back().setHistogram(src_record.getRgbHistogramPath(rgb_index));
               theSrcRecords.back().setOverview(src_record.getRgbOverviewPath(rgb_index));
            }
         }
      }
      else
      {
         // Not RGB data, so treat as conventional image: (comment added OLK 01/11)
         theSrcRecords.push_back(src_record);
         sum_weights += src_record.getWeight();

         //if the vector file exists, set the mosaic combiner type to rspfBlendMosaic
         if (src_record.isVectorData())
            theCombinerType = "rspfBlendMosaic";
      }
   }

   double max_weight = (sum_weights > 100.0 ? sum_weights : 100.0);
   double num_entries = (double)theSrcRecords.size();
   double weight; 
   vector<rspfSrcRecord>::iterator iter = theSrcRecords.begin();
   while (iter != theSrcRecords.end())
   {
      if (sum_weights > 0.0)
      {
         // Somebody declared opacity, so need to share the remaining contributions among
         // other images:
         theCombinerType = "rspfBlendMosaic";
         if (iter->getWeight() == 0.0)
         {
            // No weight has been assigned for this image, so use default remaining partial
            if (num_entries == 1.0)
               weight = 1.0; // This is the only image, so full weight
            else
            {
               // share remaining contributions:
               weight = (1.0 - sum_weights/max_weight)/(num_entries - 1); 
               if (weight < 0.01)
                  weight = 0.01;
            }
         }
         else
         {
            // An opacity value was specified for this
            weight = iter->getWeight()/max_weight;
         }
      }
      else
      {
         // No opacity values were specified, so simply use the default equal share. Note that the
         // mosaic may not even be of type rspfBlendMosaic:
         weight = 100.0/num_entries; // default if no opacity specified
      }

      iter->setWeight(weight);
      iter++;
   }
}

//*************************************************************************************************
// METHOD
//*************************************************************************************************
void rspfOrthoIgen::setDefaultValues()
{
   theBuildThumbnailFlag = false;
   theDeltaPerPixelUnit = RSPF_UNIT_UNKNOWN;
   theDeltaPerPixelOverride.makeNan();
   theTemplateView = "";
   theProjectionType = RSPF_UNKNOWN_PROJECTION;
   theGeoScalingLatitude = rspf::nan();
   theCombinerType = "rspfImageMosaic";
   theResamplerType = "nearest neighbor";
   theTilingTemplate = "";
   theTilingFilename = "";
   theSupplementaryDirectory = "";
   theSlaveBuffers = "2";
   clearFilenameList();
   theLowPercentClip = rspf::nan();
   theHighPercentClip = rspf::nan();
   theCutOrigin.makeNan();
   theCutDxDy.makeNan();
   theCutOriginUnit   = RSPF_UNIT_UNKNOWN;
   theCutDxDyUnit     = RSPF_UNIT_UNKNOWN;

   // PIXEL_IS_AREA HACK -- Set the assumed pixel alignment type to "area". Upon the first occurrence
   // of a pixel-is-point entry, this property will revert to point. THIS NEEDS TO BE
   // REMOVED WHEN THE EW GUI PROVIDES FOR THE USER TO SET THIS PROPERTY (OLK 09/11):
   thePixelAlignment = RSPF_PIXEL_IS_AREA; // not a default, but necessary for later logic
}

//*************************************************************************************************
// Initializes the processing chain from the information on the command line
//*************************************************************************************************
void rspfOrthoIgen::setupIgenChain()
{
   if (traceDebug())
      rspfNotify(rspfNotifyLevel_DEBUG)<< "rspfOrthoIgen::setupIgenChain DEBUG: Entered ..."<< std::endl;

   setupTiling();

   if (theSlaveBuffers == "")
      theNumberOfTilesToBuffer = 2;
   else
      theNumberOfTilesToBuffer = theSlaveBuffers.toLong();

   if(theProductFilename.empty())
      throw(rspfException(std::string("Must supply an output file.")));

   // Create the output mosaic object, to be connected to its inputs later:
   rspfKeywordlist templateKwl;
   templateKwl.clear();
   rspfRefPtr<rspfImageCombiner> mosaicObject = 0;
   rspfRefPtr<rspfImageCombiner> bandMergeObject = 0;
   if(theCombinerTemplate.exists())
   {
      templateKwl.addFile(theCombinerTemplate);
      mosaicObject = PTR_CAST(rspfImageCombiner, 
                              rspfObjectFactoryRegistry::instance()->createObject(templateKwl));
   }
   if (!mosaicObject.valid())
   {
      mosaicObject = PTR_CAST(rspfImageCombiner, 
         rspfObjectFactoryRegistry::instance()->createObject(theCombinerType));
      if(!mosaicObject.valid())
      {
         mosaicObject = PTR_CAST(rspfImageMosaic, rspfObjectFactoryRegistry::instance()->
            createObject(rspfString("rspfImageMosaic")));
      }
   }
   
   // Keep this pointer around for special processing if blend mosaic:
   rspfBlendMosaic* obm = PTR_CAST(rspfBlendMosaic, mosaicObject.get());

   // An orthomosaic implies that all input images are already orthorectified to a common projection
   // so the input chains do not require a renderer:
   bool orthoMosaic = (PTR_CAST(rspfOrthoImageMosaic, mosaicObject.get()) != 0);

   // Establish default individual input chain from template, if any:
   templateKwl.clear();
   rspfRefPtr<rspfImageChain> default_single_image_chain = 0;
   if(theChainTemplate.exists())
   {
      templateKwl.addFile(theChainTemplate);
      rspfObject* obj = 0;
      if(templateKwl.find("type"))
         obj = rspfObjectFactoryRegistry::instance()->createObject(templateKwl);
      else if(templateKwl.find("object1.type"))
         obj = rspfObjectFactoryRegistry::instance()->createObject(templateKwl, "object1.");  
      default_single_image_chain = PTR_CAST(rspfImageChain, obj);
   }
   if(!default_single_image_chain.valid())  // then create a default rendering chain
   {
      default_single_image_chain = new rspfImageChain;
      {
         // Only need a renderer if an output projection or an explicit GSD was specified.
         if(!orthoMosaic)
         {
            rspfImageRenderer* renderer   = new rspfImageRenderer;
            if (renderer->getResampler())
               renderer->getResampler()->setFilterType(theResamplerType);
            default_single_image_chain->addChild(renderer);
         }
      }
   }

   rspf_uint32 num_inputs = (rspf_uint32)theSrcRecords.size();
   rspf_uint32 idx;
   rspfString prefix ("object1.object");
   theReferenceProj = 0;

   // Loop over each input image file to establish a single image chain that will be added to the
   // output mosaic:
   rspfImageSource* current_source = 0;
   for(idx = 0; idx < num_inputs; ++idx)
   {
      // first lets add an input handler to the chain:
      rspfFilename input  = theSrcRecords[idx].getFilename();
      rspfRefPtr<rspfImageHandler> handler = rspfImageHandlerRegistry::instance()->open(input);
      if(!handler.valid())
      {
         rspfNotify(rspfNotifyLevel_WARN) << "Could not open input file <" << input << ">. "
            << "Skipping this entry." << std::endl;
         continue;
      }

      // Pass on any reader properties if there are any.
      rspfPropertyInterface* propInterface = (rspfPropertyInterface*)handler.get();
      PropertyMap::iterator iter = theReaderProperties.begin();
      while(iter != theReaderProperties.end())
      {
         propInterface->setProperty(iter->first, iter->second);
         ++iter;
      }

      // Presently, handler->loadState() is called only on vector data, though in the future we
      // should stuff many of the members in rspfSrcRecord in a KWL (similar to what is currently
      // done with vector properties) so that the handler is initialized via loadState() instead of 
      // individual calls to set methods.  OLK 10/10
      if (theSrcRecords[idx].isVectorData())
         handler->loadState(theSrcRecords[idx].getAttributesKwl());

      std::vector<rspf_uint32> entryList;
      if(theSrcRecords[idx].getEntryIndex() > -1 )
         entryList.push_back(theSrcRecords[idx].getEntryIndex());
      else
         handler->getEntryList(entryList);

      // Input image file may have multiple entries. Loop over each and establish single image
      // chains for each:
      rspf_uint32 entryIdx = 0;
      for(entryIdx = 0; entryIdx < entryList.size(); ++entryIdx)
      {
         // Instantiate the chain for one input image source. Copy existing default chain
         // which may already possess a renderer (so don't do any addFirst()!):
         rspfImageChain* singleImageChain = (rspfImageChain*) default_single_image_chain->dup();

         // Establish the image handler for this particular frame. This may be just
         // the handler already opened in the case of single image per file:
          rspfImageHandler* img_handler = 0;
         if (entryList.size() == 1)
            img_handler = handler.get();
         else
            img_handler = (rspfImageHandler*)handler->dup();

         // The user can specify an external "support" (a.k.a. supplementary directory) several ways
         if ( theSupplementaryDirectory.empty() == false )
         {
            img_handler->setSupplementaryDirectory( theSupplementaryDirectory );
         }
         else if (theSrcRecords[idx].getSupportDir().empty() == false)
         {
            img_handler->setSupplementaryDirectory(theSrcRecords[idx].getSupportDir());
         }
         else if (theSrcRecords[idx].getOverviewPath().empty() == false)
         {
            if (theSrcRecords[idx].getOverviewPath().isDir())
               img_handler->setSupplementaryDirectory(theSrcRecords[idx].getOverviewPath());
            else
               img_handler->setSupplementaryDirectory(theSrcRecords[idx].getOverviewPath().path());
         }
         img_handler->setCurrentEntry(entryList[entryIdx]);
         if ( img_handler->hasOverviews() )
         {
            img_handler->openOverview();
         }
         if (theSrcRecords[idx].isRgbData() && theSrcRecords[idx].getBands().size() > 0 && 
            theSrcRecords[idx].getOverviewPath().empty())
         {
            img_handler->setOutputBandList(theSrcRecords[idx].getBands());
         }

         // Image handler is ready to insert on the input side of the chain:
         singleImageChain->addLast(img_handler);
         current_source = img_handler;

         // PIXEL_IS_AREA HACK -- Scan the pixel alignment to see if all inputs are "area",
         // in which case we override the command-line writer property setting. THIS NEEDS TO BE
         // REMOVED WHEN THE EW GUI PROVIDES FOR THE USER TO SET THIS PROPERTY (OLK 09/11):
         if (img_handler->getPixelType() == RSPF_PIXEL_IS_POINT)
            thePixelAlignment = RSPF_PIXEL_IS_POINT;

         // This call will check for the presence of a raster mask file alongside the image,
         // and insert the mask filter in the chain if present:
         current_source = setupRasterMask(singleImageChain, theSrcRecords[idx]);

         // If this is the first input chain, use it as the reference projection to help with
         // the instantiation of the product projection (the view):
         if (!theReferenceProj.valid())
         {
            rspfRefPtr<rspfImageGeometry> geom = img_handler->getImageGeometry();
            if ( geom.valid() ) 
               theReferenceProj = geom->getProjection();
         }

         // Insert a partial-pixel flipper to remap null-valued pixels to min.  
         // This is set via preference keyword "orthoigen.flip_null_pixels"  
         current_source = setupPixelFlipper(singleImageChain, theSrcRecords[idx]);

         // Install a band selector if needed:
         if (theSrcRecords[idx].getBands().size() && (img_handler->getNumberOfOutputBands() > 1))
         {
            rspf_uint32 bands = img_handler->getNumberOfOutputBands();
            bool validBand = true;
            for (rspf_uint32 i = 0; i < theSrcRecords[idx].getBands().size(); ++i)
            {
               if (theSrcRecords[idx].getBands()[i] >= bands)
               {
                  validBand = false;
                  rspfNotify(rspfNotifyLevel_FATAL) << " ERROR:" << "\nBand list range error!"
                     << "\nHighest available band:  " << bands << std::endl;
               }
            }
            if (validBand)
            {
               rspfRefPtr<rspfBandSelector> bs = new rspfBandSelector();
               singleImageChain->insertRight(bs.get(), current_source);
               bs->setOutputBandList(theSrcRecords[idx].getBands());
               current_source = bs.get();
            }
         }

         // Install a histogram object if needed. This inserts just to the left of the resampler.
         setupHistogram(singleImageChain, theSrcRecords[idx]);

         // Add a cache just to the left of the resampler.
         if (!theCacheExcludedFlag)
            addChainCache(singleImageChain);

         // Add the single image chain to the mosaic and save it to the product spec file:
         singleImageChain->makeUniqueIds();

         if (theSrcRecords[idx].isRgbData())
         {
            if (bandMergeObject == 0)
            {
               bandMergeObject = new rspfBandMergeSource();
            }
            bandMergeObject->connectMyInputTo(singleImageChain);
            singleImageChain->changeOwner(bandMergeObject.get());
         }
         else
         {
            mosaicObject->connectMyInputTo(singleImageChain);
            singleImageChain->changeOwner(mosaicObject.get());
         }
         //theContainer->addChild(singleImageChain);

         // Set the weight for this image when doing a blend mosaic:
         if (obm)
            obm->setWeight(idx, theSrcRecords[idx].getWeight());
      }
   }

   // Finished initializing the inputs to the mosaic. Add the mosaic to the product chain.
   theProductChain = new rspfImageChain;
   if (bandMergeObject != 0)
   {
      theProductChain->addFirst(bandMergeObject.get());
   }
   theProductChain->addFirst(mosaicObject.get());
   
   // Now need to pass the product chain through the histogram setup for possible remapper given 
   // target histogram (used when histo-matching selected):
   setupHistogram();

   // When mosaicking common input projections without rendering each, need to add a renderer to the
   // mosaic for reprojecting to output projection:
   if(orthoMosaic)
   {
      rspfImageRenderer* renderer   = new rspfImageRenderer;
      renderer->getResampler()->setFilterType(theResamplerType);
      theProductChain->addFirst(current_source);
   }

   //---
   // Now that "theProductChain" is initialized we must initialize elevation if needed as it can
   // affect the tie point of the output projection.
   //---
   if ( isAffectedByElevation() )
   {
      rspfInit::instance()->initializeElevation();

      // Chain gsd's affected by elevation so recompute.
      reComputeChainGsds();
   }

   // Set up the output product's projection:
   setupProjection();

   // Annotation setup...
   setupAnnotation();

   // Output rect cutter:
   setupCutter();

   // Output radiometry filter:
   setupOutputRadiometry();

   // After all the connections have been established, add the product chain to the overall 
   // product container. This container will also hold the writer object.
   theContainer->addChild(theProductChain.get());

   // Lastly, set up the write object (object2):
   setupWriter();

}

//*************************************************************************************************
// Initializes the Cut Rect filter to crop the mosaic to specified rectangle.
// This method assumes that the view (theProductProjection) has already been propagated to all 
// the renderers (via call to setView()). This was done by prior call to setupProjection().
//*************************************************************************************************
void rspfOrthoIgen::setupCutter()
{
   // The command line accepts cut rect specification in several formats. Consolidate them to
   // a common form (UL tiepoint <theCutOrigin> and distance to LR pixel center <theCutDxDy>. This
   // method also updates the product projection with new bounds:
   consolidateCutRectSpec();

   rspfImageSource* input_source = theProductChain->getFirstSource();
   if((theCutDxDy.hasNans()&&theMaskShpFile.empty())||!theProductProjection.valid()||!input_source)
      return;

   //user may pass the shape filename with an query (e.g C:/myshp.shp|select * from myshp),
   //parse the name of mask shape file here
   rspfString query = "";
   if (!theMaskShpFile.empty())
   {
     if (theMaskShpFile.contains("|"))
     {
       rspfString fileName = theMaskShpFile;
       std::vector<rspfString> fileList = fileName.split("|");
       if (fileList.size() > 1)
       {
         theMaskShpFile = fileList[0];
         query = fileList[1];
       }
     }
   }

   if (!theMaskShpFile.exists())
   {
     if (theCutOriginUnit == RSPF_METERS)  // projection in meters...
     {
       rspfEastingNorthingCutter* cutter = new rspfEastingNorthingCutter;
       rspfDpt mpp (theProductProjection->getMetersPerPixel());
       rspfDpt lr (theCutOrigin.x + theCutDxDy.x - mpp.x, theCutOrigin.y - theCutDxDy.y + mpp.y);
       cutter->setView(theProductProjection.get());
       cutter->setEastingNorthingRectangle(theCutOrigin, lr);
       theProductChain->addFirst(cutter);
     }
     else // geographic projection, units = decimal degrees.
     {
       rspfGeoPolyCutter* cutter = new rspfGeoPolyCutter;
       std::vector<rspfGpt> polygon;

       rspfDpt dpp (theProductProjection->getDecimalDegreesPerPixel());
       rspfGpt ul(theCutOrigin.lat,                        theCutOrigin.lon               );
       rspfGpt ur(theCutOrigin.lat,                        theCutOrigin.lon + theCutDxDy.x - dpp.x);
       rspfGpt lr(theCutOrigin.lat - theCutDxDy.y + dpp.y, theCutOrigin.lon + theCutDxDy.x - dpp.x);
       rspfGpt ll(theCutOrigin.lat - theCutDxDy.y + dpp.y, theCutOrigin.lon               );

       polygon.push_back(ul);
       polygon.push_back(ur);
       polygon.push_back(lr);
       polygon.push_back(ll);

       cutter->setView(theProductProjection.get());
       cutter->setNumberOfPolygons(1);
       cutter->setPolygon(polygon);
       theProductChain->addFirst(cutter);
     }
   }
   else
   {
     rspfIrect inputRect = input_source->getBoundingRect();

     rspfGeoPolyCutter* exteriorCutter = new rspfGeoPolyCutter;
     exteriorCutter->setView(theProductProjection.get());

     rspfGeoPolyCutter* interiorCutter = NULL;

     rspfRefPtr<rspfImageHandler> shpHandler = rspfImageHandlerRegistry::instance()->open(theMaskShpFile);
     rspfEsriShapeFileInterface* shpInterface = PTR_CAST(rspfEsriShapeFileInterface, shpHandler.get());
     if (shpInterface != NULL)
     {
       if (!query.empty())
       {
         shpInterface->setQuery(query);
       }
       std::multimap<long, rspfAnnotationObject*> features = shpInterface->getFeatureTable();
       if (features.size() > 0)
       {
         std::multimap<long, rspfAnnotationObject*>::iterator it = features.begin();
         while (it != features.end())
         {
           rspfAnnotationObject* anno = it->second;
           if (anno != NULL)
           {
             rspfGeoAnnotationPolyObject* annoPoly = PTR_CAST(rspfGeoAnnotationPolyObject, anno);
             rspfGeoAnnotationMultiPolyObject* annoMultiPoly = NULL;
             if (annoPoly == NULL)
             {
                annoMultiPoly = PTR_CAST(rspfGeoAnnotationMultiPolyObject, anno);
             }
             if (annoPoly != NULL)
             {
               std::vector<rspfGpt> polygon;

               //get the points of a polygon
               std::vector<rspfGpt> points = annoPoly->getPoints();
               for (rspf_uint32 i = 0; i < points.size(); i++)
               {
                 polygon.push_back(points[i]);
               }

               //get polygon type, if it is an internal polygon, initialize the internal cutter
               rspfGeoAnnotationPolyObject::rspfPolyType polyType = annoPoly->getPolyType();
               if (polyType == rspfGeoAnnotationPolyObject::RSPF_POLY_INTERIOR_RING)
               {
                 if (interiorCutter == NULL)
                 {
                   interiorCutter = new rspfGeoPolyCutter;
                   interiorCutter->setView(theProductProjection.get());
                   interiorCutter->setCutType(rspfPolyCutter::RSPF_POLY_NULL_INSIDE);
                 }
                 interiorCutter->addPolygon(polygon);
               }
               else
               {
                 exteriorCutter->addPolygon(polygon);
               }
             }
             else if (annoMultiPoly != NULL)
             {
               std::vector<rspfGeoPolygon> multiPolys = annoMultiPoly->getMultiPolygon();
               for (rspf_uint32 i = 0; i < multiPolys.size(); i++)
               {
                 rspfGeoPolygon geoPoly = multiPolys[i];
                 std::vector<rspfGeoPolygon> holePolys = geoPoly.getHoleList();
                 if (holePolys.size() > 0)
                 {
                   if (interiorCutter == NULL)
                   {
                     interiorCutter = new rspfGeoPolyCutter;
                     interiorCutter->setView(theProductProjection.get());
                     interiorCutter->setCutType(rspfPolyCutter::RSPF_POLY_NULL_INSIDE);
                   }
                   for (rspf_uint32 j = 0; j < holePolys.size(); j++)
                   {
                      interiorCutter->addPolygon(holePolys[j]);
                   }
                 }
                 exteriorCutter->addPolygon(multiPolys[i]);
               }
             }
             else
             {
                throw(rspfException(std::string("The geometry type of the mask shape file is not polygon.")));
             }
           }
           it++;
         }
       }
     }

     //if user define the cut box, add it to the image chain
     rspfGeoPolyCutter* boundCutter = NULL;
     if (!theCutDxDy.hasNans() && !theCutOrigin.hasNans())
     {
       std::vector<rspfGpt> bound;
       if (theCutOriginUnit == RSPF_METERS)
       {
          rspfDpt mpp (theProductProjection->getMetersPerPixel());
         rspfGpt ul = theProductProjection->inverse(rspfDpt(theCutOrigin.x, theCutOrigin.y));
         rspfGpt ur = theProductProjection->inverse(rspfDpt(theCutOrigin.x + theCutDxDy.x - mpp.x, theCutOrigin.y));
         rspfGpt lr = theProductProjection->inverse(rspfDpt(theCutOrigin.x + theCutDxDy.x - mpp.x, theCutOrigin.y - theCutDxDy.y + mpp.y));
         rspfGpt ll = theProductProjection->inverse(rspfDpt(theCutOrigin.x, theCutOrigin.y - theCutDxDy.y + mpp.y));

         bound.push_back(ul);
         bound.push_back(ur);
         bound.push_back(lr);
         bound.push_back(ll);
       }
       else
       {
          rspfDpt dpp (theProductProjection->getDecimalDegreesPerPixel());
          rspfGpt ul(theCutOrigin.lat,                        theCutOrigin.lon               );
          rspfGpt ur(theCutOrigin.lat,                        theCutOrigin.lon + theCutDxDy.x - dpp.x);
          rspfGpt lr(theCutOrigin.lat - theCutDxDy.y + dpp.y, theCutOrigin.lon + theCutDxDy.x - dpp.x);
          rspfGpt ll(theCutOrigin.lat - theCutDxDy.y + dpp.y, theCutOrigin.lon               );

         bound.push_back(ul);
         bound.push_back(ur);
         bound.push_back(lr);
         bound.push_back(ll);
       }
       boundCutter = new rspfGeoPolyCutter;

       boundCutter->setView(theProductProjection.get());
       boundCutter->setNumberOfPolygons(1);
       boundCutter->setPolygon(bound);
     }

     if (boundCutter == NULL)
     {
        rspfIrect shpRect = shpHandler->getBoundingRect();
        if (shpRect.width() > inputRect.width() && shpRect.height() > inputRect.height())
        {
           exteriorCutter->setRectangle(inputRect);
        }
     }
     
     theProductChain->addFirst(exteriorCutter);

     if (interiorCutter != NULL)
     {
       theProductChain->addFirst(interiorCutter);
     }

     if (boundCutter != NULL)
     {
        theProductChain->addFirst(boundCutter);
     }
   }
}

//*************************************************************************************************
// METHOD 
//*************************************************************************************************
void rspfOrthoIgen::setupWriter()
{
   if (!theProductChain.valid())
      return;

   rspfRefPtr<rspfImageFileWriter> writer = 0;
   
   if (theWriterType.size())
   {
       // User selected writer with -w or --writer option.
      writer = rspfImageWriterFactoryRegistry::instance()->createWriter(theWriterType);
   }
   else if ( theWriterTemplate.size() && theWriterTemplate.exists() )
   {
      // User sent us a writer template.
      rspfKeywordlist kwlTemplate;
      kwlTemplate.addFile(theWriterTemplate);

      // Try first with no prefix.
      writer = rspfImageWriterFactoryRegistry::instance()->createWriter(kwlTemplate);
      if ( !writer.valid() )
         writer = rspfImageWriterFactoryRegistry::instance()->createWriter(kwlTemplate, "object2.");
   }
   else if ( theTilingFilename == "%SRTM%")
   {
      rspfKeywordlist kwlWriter;
      kwlWriter.add("type", "rspfGeneralRasterWriter", true);
      kwlWriter.add("byte_order", "big_endian");
      writer = rspfImageWriterFactoryRegistry::instance()->createWriter(kwlWriter);
      theProductFilename = theProductFilename.path();
   }
   else if (!theTilingFilename.empty())
   {
      if (theProductFilename.isDir())
      {
         theProductFilename = theProductFilename + "/" + theTilingFilename;
      }
   }

   try
   {
      //---
      // Set the output file name if not already set.
      // NOTE: Could be outputing to stdout in which case outputFilename does not
      // make sense.  Leaving here though to not break code downstream. (drb)
      //---
      if ( theProductFilename == rspfFilename::NIL )
      {
         throw(rspfException(std::string("Writer output filename not set.")));
      }

      //---
      // Final check for writer.
      //---
      if ( !writer.valid() )
      {
         // Derive writer from the extension.
         rspfFilename ext = theProductFilename.ext();
         if ( ext.size() )
            writer = rspfImageWriterFactoryRegistry::instance()->createWriterFromExtension(ext);

         //---
         // Lastly default to tiff.  Perhaps throw exception here instead. (drb)
         //---
         if( !writer.valid() )
         {
            writer = new rspfTiffWriter;
            theProductFilename.setExtension("tif");
         }
      }

      // PIXEL_IS_AREA HACK: Temporary patch to override command line alignment type with source
      // image's alignment type. TO BE REMOVED ONCE EW GUI PROVIDES FOR USER-SETTING OF THIS 
      // PROPERTY (OLK 09/11):
      if (thePixelAlignment == RSPF_PIXEL_IS_AREA)
      {
          rspfString pixelType ("pixel_type");
          theWriterProperties.erase(pixelType);
          theWriterProperties.insert(std::make_pair(pixelType, rspfString("area")));
      }

      //---
      // Set writer filename, connect and add to writer to keyword list.
      //---
      if ( writer.valid() )
      {
         writer->setFilename(theProductFilename);
         writer->connectMyInputTo(0, theProductChain.get());

         rspfPropertyInterface* propInterface = (rspfPropertyInterface*)writer.get();
         PropertyMap::iterator iter = theWriterProperties.begin();
         while(iter != theWriterProperties.end())
         {
            propInterface->setProperty(iter->first, iter->second);
            ++iter;
         }
         theContainer->addChild(writer.get());
      }
      else
      {
         throw(rspfException(std::string("Unable to create writer.")));
      }
   }
   catch (const rspfException& e)
   {
      if (traceDebug())
         rspfNotify(rspfNotifyLevel_DEBUG) << e.what() << std::endl;
      throw; // re-throw exception
   }
}


//*************************************************************************************************
// This method establishes the output (view) projection of the product.
// NOTE: Completely rewritten to simplify and reduce redundancy. OLK 3/10
//*************************************************************************************************
void rspfOrthoIgen::setupProjection()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<<"Entering rspfOrthoIgen::setupProjection():"<<std::endl;
   }

   theProductProjection = 0;

   // Throw exception if no valid input image projection was established:
   if(!theReferenceProj.valid())
   {
      std::string errMsg = "rspfOrthoIgen::setupProjection() -- Could not establish input image's "
         "projection. Cannot setup output view.";
      throw(rspfException(errMsg));
   }

   // Fetch the reference input projection first. Settings may be copied to the product projection:
   rspfMapProjection* ref_map = PTR_CAST(rspfMapProjection, theReferenceProj.get());

   // Now focus on establishing the output product projection.
   // Consider externally specified geometry first:
   if (theProjectionType == RSPF_EXTERNAL_PROJECTION)
   {
      if (!theTemplateView.isReadable())
      {
         rspfString errMsg = "rspfOrthoIgen::setupProjection() -- Could not read the product "
            "projection template file at <";
         errMsg += theTemplateView;
         errMsg += ">. Cannot establish output projection.";
         throw(rspfException(errMsg));
      }

      // Default template format is no prefix, but consider alternate with prefix if first attempt 
      // fails:
      rspfKeywordlist templateKwl (theTemplateView);
      rspfObjectFactoryRegistry* ofr = rspfObjectFactoryRegistry::instance();
      rspfRefPtr<rspfObject> productObj = ofr->createObject(templateKwl, "product.projection.");
      if(!productObj.valid())
         productObj = ofr->createObject(templateKwl);
      theProductProjection = PTR_CAST(rspfMapProjection, productObj.get());
   }

   // Geographic? (Assuming WGS 84 for now.)
   else if (theProjectionType == RSPF_GEO_PROJECTION)
   {
      theProductProjection = new rspfEquDistCylProjection();
      rspfGpt gpt(0.0, 0.0);
      if (!rspf::isnan(theGeoScalingLatitude))
        gpt = rspfGpt(theGeoScalingLatitude, 0.0);
      theProductProjection->setOrigin(gpt);
   }

   // CRS code specified on the command line
   else if (theProjectionType == RSPF_SRS_PROJECTION)
   {
      rspfProjection* base_proj = 
        rspfEpsgProjectionFactory::instance()->createProjection(theCrsString);

      theProductProjection = PTR_CAST(rspfMapProjection, base_proj);
      if(theProductProjection.valid())
      {
         // Reassign the type for geographic. Now we know
         if (theProductProjection->isGeographic())
         {
            theProjectionType = RSPF_GEO_PROJECTION;
            rspfGpt gpt(0.0, 0.0);
            if (!rspf::isnan(theGeoScalingLatitude))
              gpt = rspfGpt(theGeoScalingLatitude, 0.0);
            theProductProjection->setOrigin(gpt);
         }
      }
      else
      {
         theProjectionType = RSPF_UNKNOWN_PROJECTION;
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfOrthoIgen::setupProjection() WARNING:" << " Unsupported spatial reference system."
            << " Will default to the projection from the input image."
            << std::endl;
      }
   }

   // UTM?
   else if (theProjectionType == RSPF_UTM_PROJECTION)
   {
      rspfUtmProjection* utm = new rspfUtmProjection;
      rspfGpt refGpt;
      theReferenceProj->lineSampleToWorld(rspfDpt(0,0), refGpt);

      utm->setZone(refGpt);
      utm->setHemisphere(refGpt);
      theProductProjection = utm;
   }

   // None of the above?
   else
   {
      // Either RSPF_INPUT_PROJECTION or RSPF_UNKNOWN_PROJECTION. In both cases
      // just use the first image's input projection for the output. Need to make 
      // sure the input_proj is a map projection though:
      if (ref_map)
      {
         theProductProjection = PTR_CAST(rspfMapProjection, ref_map->dup());
         theProjectionType = RSPF_INPUT_PROJECTION; // just in case it was unknown before
      }
      else
      {  
         theProjectionType = RSPF_GEO_PROJECTION;
         theProductProjection = new rspfEquDistCylProjection();

         rspfGpt gpt(0.0, 0.0);
         if (!rspf::isnan(theGeoScalingLatitude))
            gpt = rspfGpt(theGeoScalingLatitude, 0.0);
         theProductProjection->setOrigin(gpt);
      }  
   }

   // At this point there should be a valid output projection defined:
   if (!theProductProjection.valid())
   {
      std::string errMsg = "rspfOrthoIgen::setupProjection() -- Could not establish valid output "
         "projection";
      throw(rspfException(errMsg));
   }

   // HACK (OLK 06/10): The projection may not have had the PCS code initialized even though it
   // is an EPSG projection, so take this opportunity to identify a PCS for output:
   rspf_uint32 pcs_code = theProductProjection->getPcsCode();
   if (pcs_code == 0)
   {
      pcs_code = rspfEpsgProjectionDatabase::instance()->
         findProjectionCode(*(theProductProjection.get()));
      theProductProjection->setPcsCode(pcs_code);
   }

   // Bootstrap the process of establishing the mosaic tiepoint by setting it to the reference proj.
   if (ref_map)
      theProductProjection->setUlGpt(ref_map->getUlGpt());

   // cout << "ref_map->getUlGpt(): " << ref_map->getUlGpt() << endl;

   // Base class makes sure the product view projection is properly wired into the chain. 
   setView();

   // Set the desired image GSD. This is nontrivial due to the many ways GSD can be implied and/or
   // explicitly provided. This method also does a setView before returning:
   setProductGsd();

   theProjectionName = theProductProjection->getProjectionName();

   // At this point, the product projection will not have a tiepoint (UL corner coordinates)
   // defined unless it is the same projection as the input reference. Need to set it now. Note that
   // if a cut-rect is specified, the tie-point will be modified later in setupCutter()
   establishMosaicTiePoint();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfOrthoIgen::setupProjection DEBUG:"
         << "Leaving...." << __LINE__
         << std::endl;
   }
}

//*************************************************************************************************
// METHOD
//*************************************************************************************************
void rspfOrthoIgen::setupAnnotation()
{
   rspfImageSource* input_source = theProductChain->getFirstSource();
   if (!input_source)
      return;

   if(theAnnotationTemplate.exists() == false)
      return;
   
   rspfKeywordlist templateKwl;
   if (templateKwl.addFile(theAnnotationTemplate) == false)
      return;
   
   rspfRefPtr<rspfObject> obj = rspfObjectFactoryRegistry::instance()->
      createObject(templateKwl, "object1.");
   if (obj.valid())
   {
      rspfGeoAnnotationSource* oga = PTR_CAST(rspfGeoAnnotationSource, obj.get());
      if (oga)
      {
         if (theProductProjection.valid())
            oga->setGeometry(new rspfImageGeometry(0, theProductProjection.get())); 
         theProductChain->addFirst(oga);
      }
   }
   return;
}

//*************************************************************************************************
// Set up multi-file tiling if indicated on the command line.
//*************************************************************************************************
bool rspfOrthoIgen::setupTiling()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfOrthoIgen::setupTiling: Entered......" << std::endl;
   }
   rspfKeywordlist templateKwl;
   rspfFilename outputFilename = theProductFilename;
   theTilingEnabled = false;

   if ((theTilingTemplate == "")||(!templateKwl.addFile(theTilingTemplate)))
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfOrthoIgen::setupTiling: Leaving......" << __LINE__ << std::endl;
      }
      return false;
   }

   rspfString prefix ("igen.tiling.");
   while (1)
   {
      if(outputFilename.isDir())
      {
         if(templateKwl.find(prefix.chars(), "type"))
         {
            theTilingFilename = templateKwl.find(prefix.chars(),"tile_name_mask");
            theTilingEnabled = true;
            break;
         }
         else if (templateKwl.find(prefix.chars(), "tile_size") || templateKwl.find(prefix.chars(), "tile_source"))
         {
            theTilingFilename = templateKwl.find(prefix.chars(),"output_file_name");
            theTilingEnabled = true;
            break;
         }
      }
      else
      {
         theTilingFilename = outputFilename.file();
         if (!theTilingFilename.contains("%"))
         {
            rspfString fileNoExt = theTilingFilename.fileNoExtension();
            rspfString ext = theTilingFilename.ext();
            theTilingFilename = fileNoExt + "_%r%_%c%." + ext;
         }
         if(templateKwl.find(prefix.chars(), "type"))
         {
            templateKwl.add(prefix.chars(), "tile_name_mask", theTilingFilename.c_str(), true);
            rspfFilename path (outputFilename.path());
            theProductFilename = path;
            theTilingEnabled = true;
            break;
         }
         else if (templateKwl.find(prefix.chars(), "tile_size") || templateKwl.find(prefix.chars(), "tile_source"))
         {
            templateKwl.add(prefix.chars(), "output_file_name", theTilingFilename.c_str(), true);
            rspfFilename path (outputFilename.path());
            theProductFilename = path;
            theTilingEnabled = true;
            break;
         }
      }

      // If we got here, then no matches were found in the template. Try again but without a prefix:
      if (prefix.empty())
         break;
      prefix.clear();
   }

   // Initialize the tiling object if enabled:
   if (templateKwl.find(prefix.chars(), "tile_size"))
   {
      theTiling = 0;
      theTiling = new rspfTilingRect;
   }

   if (templateKwl.find(prefix.chars(), "tile_source"))
   {
      theTiling = 0;
      theTiling = new rspfTilingPoly;
   }

   if (theTilingEnabled && !theTiling->loadState(templateKwl, prefix))
      theTilingEnabled = false;

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfOrthoIgen::setupTiling: templateKwl = \n" << templateKwl << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfOrthoIgen::setupTiling: Leaving......" << __LINE__ << std::endl;
   }
   
   return true;
}

//*************************************************************************************************
// Consolidates specification of bounding rect given various ways of specifying on the command
// line. This avoids multiple, redundant checks scattered throughout the code. On exit:
// 
//   1. theCutOriginType is converted to RSPF_UPPER_LEFT_ORIGIN
//   2. theCutOrigin is converted to the proper coordinates (lat/lon or easting/northing) and
//      associated theCutOriginUnits is assigned accordingly.
//   3. theCutDxDy reflects the full size of the rect, in the units corresponding to the projection
//      and associated theCutDxDyUnit is set to METERS for UTM, DEGREES for geographic
//   4. The product projection's origin (image center) and tie point are updated to reflect the
//      output rectangle.
//
//*************************************************************************************************
void rspfOrthoIgen::consolidateCutRectSpec()
{
   if (!theProductProjection.valid() || theCutDxDy.hasNans() || theCutOrigin.hasNans())
      return; 

   if ((theCutDxDyUnit != RSPF_METERS) && 
       (theCutDxDyUnit != RSPF_DEGREES) &&
       (theCutDxDyUnit != RSPF_UNIT_UNKNOWN))
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfOrthoIgen::consolidateCutRectSpec: An unhandled"
         " type of units was encountered. The cut rect needs to be specified in either meters or"
         " degrees. The resulting cut rect and origin may be incorrect." << std::endl;
      return;
   }

   rspfGpt originPLH;
   rspfDpt resolution;

   // Geographic Projection (lat/lon cut rect) requested?
   if(theProductProjection->isGeographic()) 
   {
      // geographic projection; units need to be decimal degrees. First check for consistent origin:
      if (theCutOriginUnit == RSPF_METERS)
      {
         originPLH = theProductProjection->inverse(theCutOrigin);
         theCutOrigin.x = originPLH.lon;
         theCutOrigin.y = originPLH.lat;
      }
      else
      {
         originPLH.lat = theCutOrigin.y;
         originPLH.lon = theCutOrigin.x;
      }

      // Check for consistent rect size:
      if (theCutDxDyUnit == RSPF_METERS)
      {
         rspfDpt mtrs_per_deg (originPLH.metersPerDegree());
         theCutDxDy.x = theCutDxDy.x/mtrs_per_deg.x;
         theCutDxDy.y = theCutDxDy.y/mtrs_per_deg.y;
      }

      // Set these to the correct units. May already be correct, but just in case...
      theCutOriginUnit = RSPF_DEGREES; 
      theCutDxDyUnit = RSPF_DEGREES; 

      if (theClipToValidRectFlag)
      {
         // Now we need to clip the cut rect by the valid image footprint for the entire mosaic:
         rspfDrect boundingRect = theProductChain->getBoundingRect(); // in view coordinates

         // The bounding rect is in image space. Since pixel-is-point, the actual valid area on the
         // ground will extend 1/2 pixel beyond the centers, so grow the bounding rect by 1/2 p:
         boundingRect.expand(rspfDpt(0.5, 0.5));
         rspfGpt mosaic_ul, mosaic_lr;
         theProductProjection->lineSampleHeightToWorld(boundingRect.ul(), 0, mosaic_ul);
         theProductProjection->lineSampleHeightToWorld(boundingRect.lr(), 0, mosaic_lr);

         // Establish the LR bound defined by the cut-rect and clip the cut-rect if necessary:
         rspfGpt cutrect_lr (theCutOrigin.lat - theCutDxDy.lat, theCutOrigin.lon + theCutDxDy.lon);
         if (mosaic_ul.lat < theCutOrigin.lat)
            theCutOrigin.lat = mosaic_ul.lat;
         if (mosaic_lr.lat > cutrect_lr.lat)
            theCutDxDy.lat = theCutOrigin.lat - mosaic_lr.lat;
         if (mosaic_ul.lon > theCutOrigin.lon)
            theCutOrigin.lon = mosaic_ul.lon;
         if (mosaic_lr.lon < cutrect_lr.lon)
            theCutDxDy.lon = mosaic_lr.lon - theCutOrigin.lon;
      }

      resolution = theProductProjection->getDecimalDegreesPerPixel();
   }

   // Map Projection (easting, northing cut rect) requested?
   else 
   {
      // Special case code to account for origin and delta being specified in geographic, leading to
      // offset error due to northing difference between UL and UR corners at constant lat:
      if ((theCutOriginType == RSPF_UPPER_LEFT_ORIGIN) &&
          (theCutOriginUnit == RSPF_DEGREES) && (theCutDxDyUnit == RSPF_DEGREES))
      {
         rspfGpt ulgp (theCutOrigin.lat                 , theCutOrigin.lon                 , 0);
         rspfGpt urgp (theCutOrigin.lat                 , theCutOrigin.lon + theCutDxDy.lon, 0);
         rspfGpt llgp (theCutOrigin.lat - theCutDxDy.lat, theCutOrigin.lon                 , 0);
         rspfGpt lrgp (theCutOrigin.lat - theCutDxDy.lat, theCutOrigin.lon + theCutDxDy.lon, 0);

         rspfDpt ulen (theProductProjection->forward(ulgp));
         rspfDpt uren (theProductProjection->forward(urgp));
         rspfDpt llen (theProductProjection->forward(llgp));
         rspfDpt lren (theProductProjection->forward(lrgp));
         
         double n_top    = (ulen.y > uren.y ? ulen.y : uren.y);
         double n_bottom = (llen.y < lren.y ? llen.y : lren.y);
         double e_left   = (ulen.x < llen.x ? ulen.x : llen.x);
         double e_right  = (uren.x > lren.x ? uren.x : lren.x);

         theCutOrigin.x = e_left;
         theCutOrigin.y = n_top;

         theCutDxDy.x = e_right - e_left;
         theCutDxDy.y = n_top - n_bottom;

         if (theClipToValidRectFlag)
         {
            // Now we need to clip the cut rect by the valid image footprint for the entire mosaic:
            rspfDrect boundingRect = theProductChain->getBoundingRect(); // in view coordinates
            boundingRect.expand(rspfDpt(0.5, 0.5));
            rspfDpt mosaic_ul, mosaic_lr;
            theProductProjection->lineSampleToEastingNorthing(boundingRect.ul(), mosaic_ul);
            theProductProjection->lineSampleToEastingNorthing(boundingRect.lr(), mosaic_lr);

            // Establish the LR bound defined by the cut-rect and clip the cut-rect if necessary:
            rspfDpt cutrect_lr (theCutOrigin.x + theCutDxDy.x, theCutOrigin.y - theCutDxDy.y);
            if (mosaic_ul.y < theCutOrigin.y)
               theCutOrigin.y = mosaic_ul.y;
            if (mosaic_lr.y > cutrect_lr.y)
               theCutDxDy.y = theCutOrigin.y - mosaic_lr.y;
            if (mosaic_ul.x > theCutOrigin.x)
               theCutOrigin.x = mosaic_ul.x;
            if (mosaic_lr.x < cutrect_lr.x)
               theCutDxDy.x = mosaic_lr.x - theCutOrigin.x;
         }
      }
      else
      {
         // Just map the geographic coordinates to easting/northing, without regard to corner
         // mismatch:
         if (theCutOriginUnit == RSPF_DEGREES)
         {
            originPLH.lat = theCutOrigin.y;
            originPLH.lon = theCutOrigin.x;
            theCutOrigin = theProductProjection->forward(originPLH);
         }
         else
         {
            // Determine the geographic position that might be needed for scaling below:
            originPLH = theProductProjection->inverse(theCutOrigin);
         }

         // Check for consistent rect size:
         if (theCutDxDyUnit == RSPF_DEGREES)
         {
            // POTENTIAL BUG: conversion from degrees longitude to meters should be a function 
            //                of latitude here. Implemented here but needs testing:
            rspfDpt mtrs_per_deg (originPLH.metersPerDegree());
            theCutDxDy.x = theCutDxDy.x * mtrs_per_deg.x;
            theCutDxDy.y = theCutDxDy.y * mtrs_per_deg.y;
         }
      }

      // Set these to the correct units. May already be correct, but just in case...
      theCutOriginUnit = RSPF_METERS; 
      theCutDxDyUnit = RSPF_METERS; 

      resolution = theProductProjection->getMetersPerPixel();
   }

   // The cut rect corresponds to the edges of the pixel ("edge-to-edge"), while RSPF considers
   // coordinates to correspond to the pixel centers. Need to shift the origin to the SE by 1/2p:
   rspfDpt half_pixel = resolution * 0.5;
   theCutOrigin.y -= half_pixel.y;
   theCutOrigin.x += half_pixel.x;

   // The size of the cutrect needs to be an integral number of pixels in output space:
   theCutDxDy.x = (floor(theCutDxDy.x/resolution.x + 0.5))* resolution.x;
   theCutDxDy.y = (floor(theCutDxDy.y/resolution.y + 0.5))* resolution.y;

   // Adjust a center origin specification to be Upper Left corner:
   if (theCutOriginType == RSPF_CENTER_ORIGIN)
   {
      theCutOrigin.y += theCutDxDy.y;
      theCutOrigin.x -= theCutDxDy.x;

      // theCutDxDy in this case represented a radius. This needs to be converted to 
      // RSPF_UPPER_LEFT_ORIGIN form:
      theCutDxDy.x *= 2.0;
      theCutDxDy.y *= 2.0;
      theCutOriginType = RSPF_UPPER_LEFT_ORIGIN;
   }

   // Finally, update the product projection with new rectangle:
   rspfDpt cutCenter (theCutOrigin.x + theCutDxDy.x/2.0, theCutOrigin.y - theCutDxDy.y/2.0);
   rspfGpt gpt;
   if (theCutDxDyUnit == RSPF_METERS)
   {
      // Set the E/N values for the cut origin as the tie point:
      theProductProjection->setUlTiePoints(theCutOrigin);
   }
   else
   {
      // Set the projection center (origin) latitude at the center of the cut rect:
      gpt.lat = cutCenter.y;
      gpt.lon = 0.0;
      theProductProjection->setOrigin(gpt);

      // Set the lat/lon values for the cut origin as the tie point:
      gpt.lat = theCutOrigin.y;
      gpt.lon = theCutOrigin.x;
      theProductProjection->setUlTiePoints(gpt);
   }

   // cout << "\n**************** proj 2:\n";
   // theProductProjection->print(cout);

   // Propagates changes to the projection to the processing chain:
   setView();
}

//*************************************************************************************************
//! Sets up the histogram operation requested for the image chain passed in.
//*************************************************************************************************
void rspfOrthoIgen::setupHistogram(rspfImageChain* input_chain, const rspfSrcRecord& src_record)
{
   // Check if the source passed in is the output mosaic object, because the target
   // histogram remapper needs to be connected to it (only valid when histo matching is requested):
   if (input_chain == NULL)
   {
      if (!theTargetHistoFileName.isReadable())
         return;

      rspfHistogramRemapper* remapper = new rspfHistogramRemapper;
      remapper->openHistogram(theTargetHistoFileName);
      theProductChain->addFirst(remapper);
      return;
   }

   // Check if any histo operation was requested on individual image:
   if ((rspf::isnan(theHighPercentClip) || rspf::isnan(theLowPercentClip)) &&
      !theUseAutoMinMaxFlag && (theStdDevClip < 0) && src_record.getHistogramOp().empty() &&
      theTargetHistoFileName.empty())
   {
      return; // no histo op requested
   }

   // Remaining operations require a histogram on the input image source:
   rspfImageHandler* handler = PTR_CAST(rspfImageHandler, input_chain->getLastSource());
   if (handler == NULL)
   {
      rspfNotify(rspfNotifyLevel_FATAL)<<"Could not locate an image handler object in the image"
         << "chain provided. This should not happen. Ignoring histogram request." << std::endl;
      return;
   }

   // Establish the ideal filename for this histogram. The following do-block is all for testing
   // different histogram file naming schemes since alternate directory and entry-indexing might be
   // used:
   rspfFilename histoFilename (src_record.getHistogramPath());
   rspfFilename candidateHistoFilename;
   rspfFilename defaultHistoFilename (handler->createDefaultHistogramFilename());
   rspfFilename entryName (handler->getFilenameWithThisExtension(rspfString(".his"), true));

   do
   {
      if (!histoFilename.empty())
      {
         // Try histogram filename based on specified name in the .src file:
         if (histoFilename.isDir())
            histoFilename = histoFilename.dirCat(defaultHistoFilename.file());
         if (histoFilename.exists()) break;

         // Try specified name with entry index:
         if (src_record.getEntryIndex() >= 0)
         {
            histoFilename = histoFilename.path().dirCat(entryName.file());
            if (histoFilename.exists()) break;
         }

         // Not found so set the candidate filename in case we need to generate it:
         candidateHistoFilename = histoFilename;
      }

      // Next try looking for a histogram based on the default name:
      histoFilename = defaultHistoFilename;
      if (histoFilename.exists())  break;

      //---
      // Last possibility is the default name with entry index.  We will test
      // even if there is only one entry, like "file_e0.his".
      //---
      histoFilename = entryName;
      if (histoFilename.exists())  break;

      // If not already set, set the candidate filename in case we need to generate it:
      if (candidateHistoFilename.empty())
         candidateHistoFilename = histoFilename;
   }
   while (false); // only pass through once


   // If the histogram was still not located, look into creating one:
   if (!histoFilename.exists())
   {
      // Check the preferences for histogram autogeneration:
      rspfString lookup = rspfPreferences::instance()->findPreference(AUTOGENERATE_HISTOGRAM_KW);
      if (lookup.toBool())
      {
         // No histogram available for this image, need to create one:
         histoFilename = candidateHistoFilename;
         rspfNotify(rspfNotifyLevel_WARN) <<"Histogram file <" <<  histoFilename
            << "> not found. Creating one now..."  << std::endl;
         bool success = createHistogram(input_chain, histoFilename);
         if (!success)
         {
            rspfNotify(rspfNotifyLevel_WARN) <<"Error encountered creating histogram file <" 
               << histoFilename << ">. Ignoring histogram request."  << std::endl;
            return;
         }
      }
   }

   // Need to insert any histogram object to the left of the renderer in the chain. Search for a 
   // renderer and save for later:
   // rspfConnectableObject* renderer = PTR_CAST(rspfConnectableObject,
   //    input_chain->findFirstObjectOfType(rspfString("rspfImageRenderer")));

   rspfTypeNameVisitor visitor( rspfString("rspfImageRenderer"),
                                 true, // firstofTypeFlag
                                 (rspfVisitor::VISIT_INPUTS|
                                  rspfVisitor::VISIT_CHILDREN) );
   input_chain->accept( visitor );
   rspfRefPtr<rspfImageRenderer> renderer = visitor.getObjectAs<rspfImageRenderer>(0);

   // Histo Match?
   if (theTargetHistoFileName.isReadable())
   {
      // A histogram match was requested. This involves applying a histo equalization to the input
      // chain and then applying an inverted equalization using the target histogram:
      rspfRefPtr<rspfHistogramEqualization> forwardEq = new rspfHistogramEqualization;
      rspfRefPtr<rspfHistogramEqualization> inverseEq = new rspfHistogramEqualization;
      
      // Init equalizers with the source and target histogram files:
      forwardEq->setInverseFlag(false);
      forwardEq->setHistogram(histoFilename);
      inverseEq->setInverseFlag(true);
      inverseEq->setHistogram(theTargetHistoFileName);

      // Need check that source and target histograms are compatible:
      rspfRefPtr<rspfMultiResLevelHistogram> sourceHisto = forwardEq->getHistogram();
      rspfRefPtr<rspfMultiResLevelHistogram> targetHisto = inverseEq->getHistogram();
      bool are_incompatible = false;
      if (!sourceHisto.valid() || !targetHisto.valid())
      {
         are_incompatible = true;
      }
      else
      {
         rspf_uint32 num_source_bands = sourceHisto->getNumberOfBands();
         if (num_source_bands != targetHisto->getNumberOfBands())
         {
            are_incompatible = true;
         }
         else
         {
            for (rspf_uint32 band=0; band<num_source_bands; band++)
            {
               rspfRefPtr<rspfHistogram> sourceBandHisto = sourceHisto->getHistogram(band);
               rspfRefPtr<rspfHistogram> targetBandHisto = targetHisto->getHistogram(band);
               if (!sourceBandHisto.valid() || !targetBandHisto.valid() ||
                   (sourceBandHisto->GetRes() != targetBandHisto->GetRes()))
               {
                  are_incompatible = true;
                  break;
               }
            }
         }
      }
      if (are_incompatible)
      {
         // Error was encountered establishing histograms for match operation:
         rspfNotify(rspfNotifyLevel_WARN)<<"Error encountered setting up histogram match "
            "operation. Check that source and target histograms are compatible. No histogram "
            "operations will be performed on this image." << std::endl;
         return;
      }

      // The source and target histos are compatible, insert to the left of renderer if one exists:
      if ( renderer.valid() )
         input_chain->insertLeft( forwardEq.get(), renderer.get() );
      else
         input_chain->addFirst(forwardEq.get());
      input_chain->insertRight(inverseEq.get(), forwardEq.get());
      
      return;
   }

   // Remaining possibilities (clip or stretch) require a remapper.
   // Insert to the left of renderer if one exists:
   rspfRefPtr<rspfHistogramRemapper> remapper = new rspfHistogramRemapper;
   if ( renderer.valid() )
      input_chain->insertLeft( remapper.get(), renderer.get() );
   else
      input_chain->addFirst(remapper.get());

   // Fetch the input histogram:
   bool histo_read_ok = remapper->openHistogram(histoFilename);
   if (!histo_read_ok)
   {
      // No histogram available for this image, need to create one (TODO):
      rspfNotify(rspfNotifyLevel_WARN)<<"Error encountered loading histogram file <" 
         << histoFilename << ">. No histogram operations will be performed on this image." 
         << std::endl;
      return;
   }

   // Set values to construct remap table:
   if (!rspf::isnan(theHighPercentClip) && !rspf::isnan(theLowPercentClip))
   {
      // Hi/Lo clip requested
      remapper->setHighNormalizedClipPoint(1.0-theHighPercentClip);
      remapper->setLowNormalizedClipPoint(theLowPercentClip);
   }

   else
   {
      // Consider histogram stretch operations. These can be on a per-image basis or global for all
      // input images. Give priority to the img_histo_op (per-image spec) over general flags below:
      rspfHistogramRemapper::StretchMode mode = rspfHistogramRemapper::STRETCH_UNKNOWN;
      rspfString img_histo_op (src_record.getHistogramOp());
      if (img_histo_op=="auto-minmax")
         mode = rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX;
      else if (img_histo_op.contains("std-stretch"))
      {
         if (img_histo_op.contains("1"))
            mode = rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN;
         else if (img_histo_op.contains("2"))
            mode = rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN;
         else if (img_histo_op.contains("3"))
            mode = rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN;
      }
      else if (theUseAutoMinMaxFlag)
         mode = rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX;
      else if (theStdDevClip > 0)
         mode = (rspfHistogramRemapper::StretchMode) theStdDevClip;

      // Finally init the remapper with proper stretch mode:
      if (mode != rspfHistogramRemapper::STRETCH_UNKNOWN)
         remapper->setStretchMode(mode, true);
   }
   
   return;
}


//*************************************************************************************************
//! Utility method for creating a histogram for an input image. Returns TRUE if successful.
//*************************************************************************************************
bool rspfOrthoIgen::createHistogram(rspfImageChain* chain, const rspfFilename& histo_filename)
{
   rspfRefPtr<rspfImageHistogramSource> histoSource = new rspfImageHistogramSource;
   rspfRefPtr<rspfHistogramWriter> writer = new rspfHistogramWriter;

   histoSource->connectMyInputTo(chain);
   histoSource->enableSource();
   histoSource->setComputationMode(RSPF_HISTO_MODE_FAST);

   writer->connectMyInputTo(histoSource.get());
   writer->setFilename(histo_filename);
   writer->addListener(&theStdOutProgress);
   bool success = writer->execute();

   writer=0;
   histoSource=0;

   if (success)
   {
      rspfNotify(rspfNotifyLevel_NOTICE)<<std::endl;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"Error encountered creating Histogram file <" 
         << histo_filename << ">. No histogram operations will be performed on this image." 
         << std::endl;
   }

   return success;
}

//*************************************************************************************************
// METHOD
//*************************************************************************************************
void rspfOrthoIgen::addChainCache(rspfImageChain* chain) const
{
   if (chain)
   {
      //rspfConnectableObject* renderer =
      //   PTR_CAST(rspfConnectableObject,
      //             chain->findFirstObjectOfType(rspfString("rspfImageRenderer")));

      rspfTypeNameVisitor visitor( rspfString("rspfImageRenderer"),
                                    true, // firstofTypeFlag
                                    (rspfVisitor::VISIT_INPUTS|
                                     rspfVisitor::VISIT_CHILDREN) );
      chain->accept( visitor );
      rspfRefPtr<rspfImageRenderer> renderer = visitor.getObjectAs<rspfImageRenderer>(0);
      if ( renderer.valid() )
      {
         rspfCacheTileSource* cache = new rspfCacheTileSource();
         chain->insertLeft( cache, renderer.get() );
      }
   }
}

//*************************************************************************************************
// Generates a log KWL file that could be fed directly to Igen. Used for verifying chain.
//*************************************************************************************************
void rspfOrthoIgen::generateLog()
{
   if (!theSrcRecords.size() || !theProductChain.valid() || theProductFilename.empty())
      return;

   // Establish output filename:
   rspfFilename logFile = theProductFilename;
   logFile.setExtension("log");

   // Fill a KWL with all info:
   rspfKeywordlist kwl; 
   theContainer->saveState(kwl);

   if (theProductProjection.valid())
      theProductProjection->saveState(kwl, "product.projection.");

   kwl.write(logFile.chars());
}

//*************************************************************************************************
//! Determines the UL corner tiepoint of the product projection as the overall UL corner of the
//! mosaic. This may not be the final tiepoint, since a cut origin may have been specified, and the
//*************************************************************************************************
void rspfOrthoIgen::establishMosaicTiePoint()
{
   if (!theProductChain.valid())
      return;

   // Need to find all image handlers to query for their UL ground point:
#if 0
   rspfConnectableObject::ConnectableObjectList clientList;
   theProductChain->findAllInputsOfType(clientList, STATIC_TYPE_INFO(rspfImageHandler), true, true);

   if (clientList.size() == 0)
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"rspfOrthoIgen::establishMosaicTiePoint() WARNING -- "
         "Expected to find image handler in the chain but none was identified."<<std::endl;
      return;
   }
#endif

   rspfTypeNameVisitor visitor( rspfString("rspfImageHandler"),
                                 false, // firstofTypeFlag
                                 (rspfVisitor::VISIT_INPUTS|
                                  rspfVisitor::VISIT_CHILDREN) );
   theProductChain->accept( visitor );

   if ( visitor.getObjects().empty() )
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"rspfOrthoIgen::establishMosaicTiePoint() WARNING -- "
         "Expected to find image handler in the chain but none was identified."<<std::endl;
      return;
   }
   rspfGpt tie_gpt_i, tie_gpt;
   rspfDpt tie_dpt_i, tie_dpt;
   tie_gpt.makeNan();
   tie_gpt.height(0.0);
   tie_dpt.makeNan();


   // Loop over all input handlers and latch the most NW tiepoint as the mosaic TP:
   //   rspfConnectableObject::ConnectableObjectList::iterator iter = clientList.begin();
   // while (iter != clientList.end())
   for( rspf_uint32 i = 0; i < visitor.getObjects().size(); ++i )
   {
      // rspfImageHandler* handler = PTR_CAST(rspfImageHandler, (*iter).get());
      // iter++;

      rspfImageHandler* handler = visitor.getObjectAs<rspfImageHandler>( i );
      if (!handler) break;

      rspfRefPtr<rspfImageGeometry> geom = handler->getImageGeometry();
      if (!geom.valid()) 
         continue; // Skip over any non geometry inputs (e.g., masks)

      if ( theProductProjection->isGeographic() )
      {
         geom->getTiePoint( tie_gpt_i, true ); // True to get edge of tie.
         if ( tie_gpt_i.hasNans() == false )
         {
            if (tie_gpt.hasNans())
               tie_gpt = tie_gpt_i;
            else
            {
               if (tie_gpt_i.lat > tie_gpt.lat) 
                  tie_gpt.lat = tie_gpt_i.lat;
               if (tie_gpt_i.lon < tie_gpt.lon) 
                  tie_gpt.lon = tie_gpt_i.lon;
            }
         }
      }
      else
      {
         geom->getTiePoint( tie_dpt_i, true ); // True to get edge of tie.
         if ( tie_dpt_i.hasNans() == false )
         {
            if (tie_dpt.hasNans())
               tie_dpt = tie_dpt_i;
            else
            {
               if (tie_dpt_i.y > tie_dpt.y) 
                  tie_dpt.y = tie_dpt_i.y;
               if (tie_dpt_i.x < tie_dpt.x) 
                  tie_dpt.x = tie_dpt_i.x;
            }
         }
      }
   }
   
#if 0
      // Establish input image bounding rect, making sure to expand to include the FULL pixel since
      // pixel is point -- i.e. the pixel coordinate corresponds to the center of the pixel area, 
      // not the edge. Therefore shift the image point by 1/2 pixel to correspond to edges:
      // (OLK 09/11)
      rspfDrect boundingRect (handler->getBoundingRect());
      vector<rspfDpt> img_vertices;
      img_vertices.push_back(boundingRect.ul() + rspfDpt(-0.5, -0.5));
      img_vertices.push_back(boundingRect.ur() + rspfDpt( 0.5, -0.5));
      img_vertices.push_back(boundingRect.lr() + rspfDpt( 0.5,  0.5));
      img_vertices.push_back(boundingRect.ll() + rspfDpt(-0.5,  0.5));
      

      // The tie point will be in easting/northing or lat/lon depending on the type of projection 
      // used for the product. Need to consider all image corners since the orientation of the image
      // is not known:
      for (int j=0; j<4; j++)
      {
         geom->localToWorld(img_vertices[j], tie_gpt_i);
         if (theProductProjection->isGeographic())
         {
            tie_gpt.height(0.0);
            if (tie_gpt.hasNans())
               tie_gpt = tie_gpt_i;
            else
            {
               if (tie_gpt_i.lat > tie_gpt.lat) 
                  tie_gpt.lat = tie_gpt_i.lat;
               if (tie_gpt_i.lon < tie_gpt.lon) 
                  tie_gpt.lon = tie_gpt_i.lon;
            }
         }
         else
         {
            tie_dpt_i = theProductProjection->forward(tie_gpt_i);
            if (tie_dpt.hasNans())
               tie_dpt = tie_dpt_i;
            else
            {
               if (tie_dpt_i.y > tie_dpt.y) 
                  tie_dpt.y = tie_dpt_i.y;
               if (tie_dpt_i.x < tie_dpt.x) 
                  tie_dpt.x = tie_dpt_i.x;
            }
         }
      }
   }

#endif

   // The tie point coordinates currently reflect the UL edge of the UL pixel. We'll need to shift
   // the tie point from the edge to the center. (OLK 09/11)
   rspfDpt half_pixel_shift(0,0);
   if (theProductProjection->isGeographic())
   {
      half_pixel_shift = theProductProjection->getDecimalDegreesPerPixel() * 0.5;
      if (!tie_gpt.hasNans())
      {
         tie_gpt.lat -= half_pixel_shift.lat;
         tie_gpt.lon += half_pixel_shift.lon;
         theProductProjection->setUlTiePoints(tie_gpt);
      }
   }
   else
   {
      half_pixel_shift = theProductProjection->getMetersPerPixel() * 0.5;
      tie_dpt.y -= half_pixel_shift.y;
      tie_dpt.x += half_pixel_shift.x;
      theProductProjection->setUlTiePoints(tie_dpt);
   }

   // Propagates changes to the projection to the processing chain:
   setView();
}

//*************************************************************************************************
// Initialize the pixel flipper in the source chain if one is called for
//*************************************************************************************************
rspfImageSource* rspfOrthoIgen::setupPixelFlipper(rspfImageChain* singleImageChain,
                                                    const rspfSrcRecord& src_record)
{
   if (singleImageChain == NULL)
      return NULL;

   // Fetch the image handler that should be the last (left-most) source in the chain:
   rspfImageSource* current_source = singleImageChain->getLastSource();
   if (current_source == NULL)
      return NULL;

   // There are two possibilities for specifying pixel flipping -- either as a command line option
   // that applies to all input imagery, or specified for a particular input via the .src file.
   // The .src file takes precedence:
   const rspfSrcRecord::PixelFlipParams& flipParams = src_record.getPixelFlipParams();

   // The replacement can be specified globally in the preferences if none found in the src record:  
   rspfString replaceModeStr = flipParams.replacementMode;
   if (replaceModeStr.empty())
      replaceModeStr = thePixelReplacementMode; 
   
   // First consider if a range of clipped pixels was specified:
   rspf_float64 clip_min = flipParams.clipMin;
   if (rspf::isnan(clip_min)) 
      clip_min = theClipPixelMin;
   rspf_float64 clip_max = flipParams.clipMax;
   if (rspf::isnan(clip_max))
      clip_max = theClipPixelMax;

   rspfPixelFlipper* flipper = 0;
   if (!rspf::isnan(clip_min) && !rspf::isnan(clip_max))
   {
      // A clip within a range of pixel values was requested. All pixels within the specified range
      // are mapped to NULL. Create the remapper and insert it into the chain just after the handler
      flipper = new rspfPixelFlipper();  
      flipper->setTargetRange(clip_min, clip_max);  
      flipper->setReplacementValue(current_source->getNullPixelValue());
      flipper->setReplacementMode(replaceModeStr);
      singleImageChain->insertRight(flipper, current_source);  
      current_source = flipper;
   }

   // The user can also specify a clamping similar to the pixel clipping above. This would be a
   // second flipper object in the chain:
   rspf_float64 clamp_min = flipParams.clampMin;
   if (rspf::isnan(clamp_min))
      clamp_min = theClampPixelMin;
   rspf_float64 clamp_max = flipParams.clampMax;
   if (rspf::isnan(clamp_max))
      clamp_max = theClampPixelMax;

   flipper = 0; // no leak since chain assumes ownership of prior instance.
   if (!rspf::isnan(clamp_min))
   {
      // A bottom clamping was requested. All pixels below this value are set to this value:
      flipper = new rspfPixelFlipper();  
      flipper->setClampValue(clamp_min, false); // false = clamp bottom
   }
   if (!rspf::isnan(clamp_max))
   {
      // A top clamping was requested. All pixels above this value are set to this value.
      // The same flipper object can be used as the bottom clamp (if created):
      if (!flipper)
         flipper = new rspfPixelFlipper();  
      flipper->setClampValue(clamp_max, true); // true = clamp top
   }
   if (flipper)
   {
      // Common code for top and bottom clamping:
      flipper->setReplacementMode(replaceModeStr);
      singleImageChain->insertRight(flipper, current_source);  
      current_source = flipper;
   }

   return current_source;
}

//*************************************************************************************************
// Checks for the presence of a raster mask file alongside the image, and inserts the mask 
// filter in the chain if mask file exists. Returns pointer to the "current (last added) source 
// in the single image chain. 
//*************************************************************************************************
rspfImageSource* rspfOrthoIgen::setupRasterMask(rspfImageChain* singleImageChain,
                                                  const rspfSrcRecord& src_record)
{
   if (singleImageChain == NULL)
      return NULL;

   // Search for the image handler in the chain:
   rspfImageHandler* img_handler = 
      dynamic_cast<rspfImageHandler*>(singleImageChain->getLastSource());
   if (img_handler == NULL) 
      return NULL;

   // See if a raster mask was specified in the SRC record:
   rspfFilename mask_file = src_record.getMaskPath();
   if (!mask_file.exists())
      return img_handler;

   // Open up the mask file and verify it is good:
   rspfImageHandler* mask_handler =  rspfImageHandlerRegistry::instance()->open(mask_file);
   if (mask_handler == NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"rspfOrthoIgen::setupRasterMask() -- Could not open "
         "raster mask file <"<<mask_file<<">. Maske request will be ignored."<<endl;
      return img_handler;
   }

   // Create the mask filter and give it the image and mask tile sources. Add it to the chain.
   // IMPORTANT NOTE: the mask filter is an image combiner. It is being inserted into a single 
   // image chain. Since it owns its two inputs (the image handler and the mask), it must
   // replace the handler in the chain. Also, see note in rspfMaskFilter::setInputSources().
   //singleImageChain->deleteLast(); // Remove the handler
   // rspfImageSource* nextInChain = singleImageChain->getLastSource();
   rspfRefPtr<rspfMaskFilter> mask_filter = new rspfMaskFilter;
   singleImageChain->insertRight(mask_filter.get(), img_handler); 
   mask_filter->setMaskSource(mask_handler);  // assumes ownership of object

   //---
   // Set the mode to SELECT_CLAMP_MIN.  This clamps data to min pixel value in the valid image
   // area if the input pixel is null(essentially a pixel flip).
   //---
   mask_filter->setMaskType(rspfMaskFilter::RSPF_MASK_TYPE_SELECT_CLAMP_MIN);
   
   return mask_filter.get();
}

//*************************************************************************************************
// Adds a scalar remapper to the extreme right of the chain is specified by the 
// --output-radiometry option.
//*************************************************************************************************
void rspfOrthoIgen::setupOutputRadiometry()
{
   if (theOutputRadiometry.empty())
      return;

   // Map the specified radiometry to a valid type:
   rspfScalarType scalar_type = 
      rspfScalarTypeLut::instance()->getScalarTypeFromString(theOutputRadiometry);
   if (scalar_type == RSPF_SCALAR_UNKNOWN)
      return;

   // Add a scalar remapper to the product chain:
   if(theProductChain->getOutputScalarType() != scalar_type)
   {
      rspfScalarRemapper* remapper = new rspfScalarRemapper;
      remapper->setOutputScalarType(scalar_type);
      theProductChain->addFirst(remapper);
   }
}

//*************************************************************************************************
// Private method to see if any image chain input projections are affected by elevation.
//*************************************************************************************************
bool rspfOrthoIgen::isAffectedByElevation()
{
   bool result = false;
   
   // Get a list of all the image handlers.
   // rspfConnectableObject::ConnectableObjectList clientList;
   // theProductChain->findAllInputsOfType(clientList, STATIC_TYPE_INFO(rspfImageHandler),
   //                                      true, true);

   rspfTypeNameVisitor visitor( rspfString("rspfImageHandler"),
                                 false, // firstofTypeFlag
                                 (rspfVisitor::VISIT_INPUTS|
                                  rspfVisitor::VISIT_CHILDREN) );
   theProductChain->accept( visitor );
   
   // Loop over all input handlers and see if affected by elevation.
   // rspfConnectableObject::ConnectableObjectList::iterator iter = clientList.begin();
   // while (iter != clientList.end())
   for( rspf_uint32 i = 0; i < visitor.getObjects().size(); ++i )
   {
      // rspfRefPtr<rspfImageHandler> handler = PTR_CAST(rspfImageHandler, (*iter).get());
      rspfRefPtr<rspfImageHandler> handler =  visitor.getObjectAs<rspfImageHandler>( i );
      if ( handler.valid() )
      {
         rspfRefPtr<rspfImageGeometry> geom = handler->getImageGeometry();
         if (geom.valid())
         {
            rspfRefPtr<const rspfProjection> proj = geom->getProjection();
            if ( proj.valid() )
            {
               if ( proj->isAffectedByElevation() )
               {
                  result = true;
                  break;
               }
            }
         }
      }
      // ++iter;
   }
   return result;
}

//*************************************************************************************************
// Private method to recompute the gsd on all image handlers that have projections affected by
// elevation.
//*************************************************************************************************
void rspfOrthoIgen::reComputeChainGsds()
{
   // Get a list of all the image handlers.
   // rspfConnectableObject::ConnectableObjectList clientList;
   // theProductChain->findAllInputsOfType(clientList, STATIC_TYPE_INFO(rspfImageHandler),
   //                                      true, true);
   
   // Loop over all input handlers and see if affected by elevation.
   // rspfConnectableObject::ConnectableObjectList::iterator iter = clientList.begin();
   // while (iter != clientList.end())

   rspfTypeNameVisitor visitor( rspfString("rspfImageHandler"),
                                 false, // firstofTypeFlag
                                 (rspfVisitor::VISIT_INPUTS|
                                  rspfVisitor::VISIT_CHILDREN) );
   theProductChain->accept( visitor );

   for( rspf_uint32 i = 0; i < visitor.getObjects().size(); ++i )
   {
      // rspfRefPtr<rspfImageHandler> handler = PTR_CAST(rspfImageHandler, (*iter).get());

      rspfRefPtr<rspfImageHandler> handler =  visitor.getObjectAs<rspfImageHandler>( i );
      if ( handler.valid() )
      {
         rspfRefPtr<rspfImageGeometry> geom = handler->getImageGeometry();
         if (geom.valid())
         {
            rspfRefPtr<rspfProjection> proj = geom->getProjection();
            if ( proj.valid() )
            {
               if ( proj->isAffectedByElevation() )
                  proj->getMetersPerPixel();
            }
         }
      }
      // ++iter;
   }
}

//*************************************************************************************************
// GSD Determination is nontrivial since there are various command-line options that control
// this quantity. This method considers all information before setting the product's GSD.
//*************************************************************************************************
void rspfOrthoIgen::setProductGsd()
{
   if (!theProductChain.valid())
      return;

   // Fetch the reference input projection first. Settings may be copied to the product projection:
   rspfMapProjection* ref_map = PTR_CAST(rspfMapProjection, theReferenceProj.get());
   rspfGpt origin;

   // The geo-scaling latitude effectively specifies the map projection's origin latitude, which
   // may affect the scaling of GSD in x-direction. This is only relevant for geographic projections
   if (theProductProjection->isGeographic())
   {
      rspfGpt origin (0.0, theProductProjection->getOrigin().lon, 0.0);
      if (rspf::isnan(theGeoScalingLatitude))
      {
         // Loop over all input handlers and accumulate the geographic centers. This will allow 
         // computing mosaic center point (approximate) for purposes of establishing reference 
         // latitude for scale:
         origin.lat = 0.0;
         origin.lon = theProductProjection->getOrigin().lon;

         // rspfConnectableObject::ConnectableObjectList clientList;
         // theProductChain->findAllInputsOfType(clientList, STATIC_TYPE_INFO(rspfImageHandler), 1, 1);
         // rspfConnectableObject::ConnectableObjectList::iterator iter = clientList.begin();

         rspfTypeNameVisitor visitor( rspfString("rspfImageHandler"),
                                       false, // firstofTypeFlag
                                       (rspfVisitor::VISIT_INPUTS|
                                        rspfVisitor::VISIT_CHILDREN) );
         theProductChain->accept( visitor );
         
         rspfDpt center_pt;
         rspfGpt geocenter;
         int num_contributors = 0;
         // while (iter != clientList.end())

         for( rspf_uint32 i = 0; i < visitor.getObjects().size(); ++i )
         {
            // rspfImageHandler* handler = PTR_CAST(rspfImageHandler, (*iter).get());
            rspfRefPtr<rspfImageHandler> handler =  visitor.getObjectAs<rspfImageHandler>( i );
            if ( handler.valid() )
            {
               // iter++;
               rspfRefPtr<rspfImageGeometry> geom = handler->getImageGeometry();
               if (!geom.valid()) 
                  continue; // Skip over any non geometry inputs (e.g., masks)
               
               handler->getBoundingRect().getCenter(center_pt);
               if (!geom->localToWorld(center_pt, geocenter))  
                  continue;
               if (num_contributors == 0)
                  origin.lat = geocenter.lat;
               else
                  origin.lat += geocenter.lat;
               ++num_contributors;
            }
            else
            {
               break;
            }
         }

         // Compute average latitude among all contributors:
         if (num_contributors)
            origin.lat /= (double)num_contributors;
      }
      else
      {
         // A geo-scaling reference latitude was provided on the command line:
         origin.lat = theGeoScalingLatitude;
      }

      // Set the latitude of origin to the reference latitude (either specified on command line or
      // computed as scene center):
      theProductProjection->setOrigin(origin); // proj now can handle meters and degrees correctly
   }

   // Establish the resolution based on either command line option or reference proj if no values
   // provided on command line (--degrees or --meters):
   rspfDpt resolution (theDeltaPerPixelOverride);
   rspfUnitType resUnits = theDeltaPerPixelUnit;
   if (resolution.hasNans())
   {
      //  No GSD specified, so copy it from the input projection:
      if (ref_map && ref_map->isGeographic())
      {
         resolution = ref_map->getDecimalDegreesPerPixel();
         resUnits = RSPF_DEGREES;
      }
      else
      {
         resolution = theReferenceProj->getMetersPerPixel();
         resUnits = RSPF_METERS;
      }
   }

   // Set the desired image GSD, accounting for possible mixing of units:
   if (resUnits == RSPF_DEGREES)
   {
      // Need to adjust the resolution in the longitude direction if the user requested geo-scaling:
      if (!rspf::isnan(theGeoScalingLatitude))
         resolution.lon = resolution.lat/rspf::cosd(theGeoScalingLatitude);
      theProductProjection->setDecimalDegreesPerPixel(resolution);
   }
   else 
      theProductProjection->setMetersPerPixel(resolution);

   // Propagates changes to the projection to the processing chain:
   setView();
}
