//----------------------------------------------------------------------------
//
// File: rspfChipperUtil.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Utility class definition processing digital elevation
// models(dems).
// 
//----------------------------------------------------------------------------
// $Id: rspfChipperUtil.cpp 22201 2013-03-20 00:53:33Z dburken $

#include <rspf/util/rspfChipperUtil.h>

#include <rspf/base/rspfArgumentParser.h>
#include <rspf/base/rspfApplicationUsage.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfVisitor.h>

#include <rspf/imaging/rspfBumpShadeTileSource.h>
#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageMosaic.h>
#include <rspf/imaging/rspfImageRenderer.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfImageToPlaneNormalFilter.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfIndexToRgbLutFilter.h>
#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/imaging/rspfTwoColorView.h>

#include <rspf/init/rspfInit.h>

#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfImageViewAffineTransform.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfUtmProjection.h>

#include <rspf/support_data/rspfSrcRecord.h>

#include <sstream>
#include <string>

static rspfTrace traceDebug("rspfChipperUtil:debug");
static rspfTrace traceLog("rspfChipperUtil:log");
static rspfTrace traceOptions("rspfChipperUtil:options");

static const char APPLICATION_NAME_KW[]     = "application_name";
static const char COLOR_BLUE_KW[]           = "color_blue";
static const char COLOR_GREEN_KW[]          = "color_green";
static const char COLOR_RED_KW[]            = "color_red";
static const char CUT_CENTER_LAT_KW[]       = "cut_center_latitude";
static const char CUT_CENTER_LON_KW[]       = "cut_center_longitude";
static const char CUT_CENTER_HEIGHT_KW[]    = "cut_center_height";  // pixels
static const char CUT_CENTER_WIDTH_KW[]     = "cut_center_width";   // pixels
static const char CUT_CENTER_RADIUS_KW[]    = "cut_center_radius";  // meters
static const char CUT_MAX_LAT_KW[]          = "cut_maximum_latitude";
static const char CUT_MAX_LON_KW[]          = "cut_maximum_longitude";
static const char CUT_MIN_LAT_KW[]          = "cut_minimum_latitude";
static const char CUT_MIN_LON_KW[]          = "cut_minimum_longitude";
static const char DEM_KW[]                  = "dem";
static const char GAIN_KW[]                 = "gain";
static const char FILE_KW[]                 = "file";
static const char HISTO_OP_KW[]             = "hist-op";
static const char IMG_KW[]                  = "image";
static const char LUT_FILE_KW[]             = "lut_file";
static const char DEGREES_X_KW[]            = "degrees_x";
static const char DEGREES_Y_KW[]            = "degrees_y";
static const char METERS_KW[]               = "meters";
static const char NORTH_UP_KW[]             = "north_up"; // bool
static const char OP_KW[]                   = "operation";
static const char OUTPUT_RADIOMETRY_KW[]    = "output_radiometry";
static const char RESAMPLER_FILTER_KW[]     = "resampler_filter";
static const char SCALE_2_8_BIT_KW[]        = "scale_2_8_bit";
static const char SNAP_TIE_TO_ORIGIN_KW[]   = "snap_tie_to_origin";
static const char SRC_FILE_KW[]             = "src_file";
static const char SRS_KW[]                  = "srs";
static const char THUMBNAIL_RESOLUTION_KW[] = "thumbnail_resolution"; // pixels
static const char TRUE_KW[]                 = "true";
static const char UP_IS_UP_KW[]             = "up_is_up"; // bool
static const char WRITER_KW[]               = "writer";
static const char WRITER_PROPERTY_KW[]      = "writer_property";

static const char TWOCMV_OLD_INPUT_BAND_KW[]      = "2cmv_old_input_band";
static const char TWOCMV_NEW_INPUT_BAND_KW[]      = "2cmv_new_input_band";
static const char TWOCMV_RED_OUTPUT_SOURCE_KW[]   = "2cmv_red_output_source";
static const char TWOCMV_GREEN_OUTPUT_SOURCE_KW[] = "2cmv_green_output_source";
static const char TWOCMV_BLUE_OUTPUT_SOURCE_KW[]  = "2cmv_blue_output_source";

rspfChipperUtil::rspfChipperUtil()
   : rspfReferenced(),
     m_operation(RSPF_CHIPPER_OP_UNKNOWN),
     m_kwl(new rspfKeywordlist()),
     m_srcKwl(0),
     m_geom(0),
     m_ivt(0),
     m_demLayer(0),
     m_imgLayer(0)
{
   m_kwl->setExpandEnvVarsFlag(true);
}

rspfChipperUtil::~rspfChipperUtil()
{
}

void rspfChipperUtil::addArguments(rspfArgumentParser& ap)
{
   rspfString usageString = ap.getApplicationName();
   usageString += " [option]... [input-option]... <input-file(s)> <output-file>\nNote at least one input is required either from one of the input options, e.g. --input-dem <my-dem.hgt> or adding to command line in front of the output file in which case the code will try to ascertain what type of input it is.\n\nAvailable traces:\n-T \"rspfChipperUtil:debug\"   - General debug trace to standard out.\n-T \"rspfChipperUtil:log\"     - Writes a log file to output-file.log.\n-T \"rspfChipperUtil:options\" - Writes the options to output-file-options.kwl.";

   rspfApplicationUsage* au = ap.getApplicationUsage();
   
   au->setCommandLineUsage(usageString);
   
   au->setDescription(ap.getApplicationName()+" Utility application for generating elevation products from dem data.");
   
   au->addCommandLineOption("--azimuth", "<azimuth>\nhillshade option - Light source azimuth angle for bump shade.\nRange: 0 to 360, Default = 180.0");

   au->addCommandLineOption( "-b or --bands <n,n...>", "Use the specified bands in given order: e.g. \"3,2,1\" will select bands 3, 2 and 1 of the input image.\nNote: it is 1 based" );

   au->addCommandLineOption("--central-meridian","<central_meridian_in_decimal_degrees>\nNote if set this will be used for the central meridian of the projection.  This can be used to lock the utm zone.");

   au->addCommandLineOption("--color","<r> <g> <b>\nhillshade option - Set the red, green and blue color values to be used with hillshade.\nThis option can be used with or without an image source for color.\nRange 0 to 255, Defualt r=255, g=255, b=255");

   au->addCommandLineOption("--color-table","<color-table.kwl>\nhillshade or color-relief option - Keyword list containing color table for color-relief option.");

   au->addCommandLineOption("--cut-bbox-ll", "<min_lat> <min_lon> <max_lat> <max_lon>\nSpecify a bounding box with the minimum latitude/longitude and max latitude/longitude in decimal degrees.");
   
   au->addCommandLineOption("--cut-center-llwh","<latitude> <longitude> <width> <height>\nSpecify the center cut in latitude longitude space with width and height in pixels.");

   au->addCommandLineOption("--cut-center-llr","<latitude> <longitude> <radius_in_meters>\nSpecify the center cut in latitude longitude space with radius in meters.");

   au->addCommandLineOption("--degrees","<dpp_xy> | <dpp_x> <dpp_y>\nSpecifies an override for degrees per pixel. Takes either a single value applied equally to x and y directions, or two values applied correspondingly to x then y. This option takes precedence over the \"--meters\" option.");

   au->addCommandLineOption("--elevation", "<elevation>\nhillshade option - Light source elevation angle for bumb shade.\nRange: 0 to 90, Default = 45.0");

   au->addCommandLineOption("-e or --entry", "<entry> For multi image handlers which entry do you wish to extract. For list of entries use: \"rspf-info -i <your_image>\" ");  

   au->addCommandLineOption("--exaggeration", "<factor>\nMultiplier for elevation values when computing surface normals. Has the effect of lengthening shadows for oblique lighting.\nRange: .0001 to 50000, Default = 1.0");
   
   au->addCommandLineOption("-h or --help", "Display this help and exit.");

   au->addCommandLineOption("--histogram-op", "<operation>\nHistogram operation to perform. Valid operations are \"auto-minmax\", \"std-stretch-1\", \"std-stretch-2\" and \"std-stretch-3\".");

   au->addCommandLineOption("--input-dem", "<dem> Input dem to process.");

   au->addCommandLineOption("--input-img", "<image> Input image to process.");
   
   au->addCommandLineOption("--input-src","<file.src> Input source file list keyword list with list of dems or images or both to process.");
   
   au->addCommandLineOption("--meters", "<meters>\nSpecifies an override for the meters per pixel");

   au->addCommandLineOption("-n or --north-up", "Rotates image North up. \"chip\" operation only.");
   
   au->addCommandLineOption( "--op", "<operation>\nOperation to perform. Valid operations are \"chip\", \"color-relief\", \"hillshade\", \"2cmv\"(two color multi view) and \"ortho\".\nchip = input projection = output projection(image space), single image operation only." );

   au->addCommandLineOption("--options-keyword-list","<options.kwl>  This can be all or part of the application options.  To get a template you can turn on trace to the rspfChipperUtil class by adding \"-T rspfChipperUtil\" to your command.");

   au->addCommandLineOption("--origin-latitude","<latidude_in_decimal_degrees>\nNote if set this will be used for the origin latitude of the projection.  Setting this to something other than 0.0 with a geographic projection creates a scaled geographic projection.");

   au->addCommandLineOption("--output-radiometry", "<R>\nSpecifies the desired product's pixel radiometry type. Possible values for <R> are: U8, U11, U16, S16, F32. Note this overrides the deprecated option \"scale-to-8-bit\"."); 

   au->addCommandLineOption("--projection", "<output_projection> Valid projections: geo, geo-scaled, input or utm\ngeo = Equidistant Cylindrical, origin latitude = 0.0\ngeo-scaled = Equidistant Cylindrical, origin latitude = image center\ninput Use first images projection. Must be a map projecion.\nutm = Universal Tranverse Mercator\nIf input and multiple sources the projection of the first image will be used.\nIf utm the zone will be set from the scene center of first image.\nNOTE: --srs takes precedence over this option.");
   
   au->addCommandLineOption("--resample-filter","<type>\nSpecify what resampler filter to use, e.g. nearest neighbor, bilinear, cubic.\nSee rspf-info ----resampler-filters"); 

   au->addCommandLineOption("--scale-to-8-bit", "Scales the output to unsigned eight bits per band. This option has been deprecated by the newer \"--output-radiometry\" option.");

   au->addCommandLineOption("--snap-tie-to-origin",
                                "Snaps tie point to projection origin so that (tie-origin)/gsd come out on an even integer boundary.");   
   
   au->addCommandLineOption("--srs","<src_code>\nSpecify an output reference frame/projection. Example: --srs EPSG:4326");

   au->addCommandLineOption("-t or --thumbnail", "<max_dimension>\nSpecify a thumbnail resolution.\nScale will be adjusted so the maximum dimension = argument given.");

   au->addCommandLineOption("-u or --up-is-up", "Rotates image to up is up. \"chip\" operation only.");

   au->addCommandLineOption("-w or --writer","<writer>\nSpecifies the output writer.  Default uses output file extension to determine writer. For valid output writer types use: \"rspf-info --writers\"\n");
   
   au->addCommandLineOption("--writer-prop", "<writer-property>\nPasses a name=value pair to the writer for setting it's property. Any number of these can appear on the line.");

   au->addCommandLineOption("--2cmv-old-input-band", "<band>\nBand to use for two color multi view old input.\n");   
   au->addCommandLineOption("--2cmv-new-input-band", "<band>\nBand to use for two color multi view new input.\n");
   
   au->addCommandLineOption("--2cmv-red-output-source", "<source>\nTwo color multi view source input for red output.  Either, old, new, or mp(min pix).  Default=old.\n");
   
   au->addCommandLineOption("--2cmv-green-output-source", "<source>\nTwo color multi view source input for green output.  Either, old, new, or mp(min pix).  Default=new.\n");
   
   au->addCommandLineOption("--2cmv-blue-output-source", "<source>\nTwo color multi view source input for blue output.  Either, old, new, or mp(min pix).  Default=new.\n");
   
} // End: rspfChipperUtil::addArguments

bool rspfChipperUtil::initialize(rspfArgumentParser& ap)
{
   static const char MODULE[] = "rspfChipperUtil::initialize(rspfArgumentParser&)";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   

   if( ap.read("-h") || ap.read("--help") || (ap.argc() == 1) )
   {
      usage(ap);
      
      return false; // Indicates process should be terminated to caller.
   }

   // Start with clean options keyword list.
   m_kwl->clear();

   rspfString tempString1;
   rspfArgumentParser::rspfParameter stringParam1(tempString1);
   rspfString tempString2;
   rspfArgumentParser::rspfParameter stringParam2(tempString2);
   rspfString tempString3;
   rspfArgumentParser::rspfParameter stringParam3(tempString3);
   rspfString tempString4;
   rspfArgumentParser::rspfParameter stringParam4(tempString4);
   double tempDouble1;
   rspfArgumentParser::rspfParameter doubleParam1(tempDouble1);
   double tempDouble2;
   rspfArgumentParser::rspfParameter doubleParam2(tempDouble2);

   rspf_uint32 demIdx  = 0;
   rspf_uint32 imgIdx  = 0;
   rspf_uint32 propIdx = 0;
   rspfString  key     = "";
   
   // Extract optional arguments and stuff them in a keyword list.
   if( ap.read("--azimuth", stringParam1) )
   {
      m_kwl->add( rspfKeywordNames::AZIMUTH_ANGLE_KW, tempString1.c_str() );
   }

   if (ap.read("-b", stringParam1) || ap.read("--bands", stringParam1))
   {
      m_kwl->add( rspfKeywordNames::BANDS_KW, tempString1.c_str() );
   }   
   
   if( ap.read("--central-meridian", stringParam1) )
   {
      m_kwl->add( rspfKeywordNames::CENTRAL_MERIDIAN_KW, tempString1.c_str() );
   }

   if( ap.read("--color", stringParam1, stringParam2, stringParam3) )
   {
      m_kwl->add( COLOR_RED_KW,   tempString1.c_str() );
      m_kwl->add( COLOR_GREEN_KW, tempString2.c_str() );
      m_kwl->add( COLOR_BLUE_KW,  tempString3.c_str() );
   }

   if( ap.read("--color-table", stringParam1) )
   {
      m_kwl->add( LUT_FILE_KW, tempString1.c_str() );
   }

   if( ap.read("--cut-bbox-ll", stringParam1, stringParam2, stringParam3, stringParam4) )
   {
      m_kwl->add( CUT_MIN_LAT_KW, tempString1.c_str() );
      m_kwl->add( CUT_MIN_LON_KW, tempString2.c_str() );
      m_kwl->add( CUT_MAX_LAT_KW, tempString3.c_str() );
      m_kwl->add( CUT_MAX_LON_KW, tempString4.c_str() );
   }
   
   if( ap.read("--cut-center-llwh", stringParam1, stringParam2, stringParam3, stringParam4) )
   {
      m_kwl->add( CUT_CENTER_LAT_KW,    tempString1.c_str() );
      m_kwl->add( CUT_CENTER_LON_KW,    tempString2.c_str() );
      m_kwl->add( CUT_CENTER_WIDTH_KW,  tempString3.c_str() );
      m_kwl->add( CUT_CENTER_HEIGHT_KW, tempString4.c_str() );
   }

   if( ap.read("--cut-center-llr", stringParam1, stringParam2, stringParam3) )
   {
      m_kwl->add( CUT_CENTER_LAT_KW,    tempString1.c_str() );
      m_kwl->add( CUT_CENTER_LON_KW,    tempString2.c_str() );
      m_kwl->add( CUT_CENTER_RADIUS_KW, tempString3.c_str() );
   }

   int num_params = ap.numberOfParams("--degrees", doubleParam1);
   if (num_params == 1)
   {
      ap.read("--degrees", doubleParam1);
      m_kwl->add( DEGREES_X_KW, tempDouble1 );
      m_kwl->add( DEGREES_Y_KW, tempDouble1 );
   }
   else if (num_params == 2)
   {
      ap.read("--degrees", doubleParam1, doubleParam2);
      m_kwl->add( DEGREES_X_KW, tempDouble1 );
      m_kwl->add( DEGREES_Y_KW, tempDouble2 );
   }   

   if ( ap.read("--elevation", stringParam1) )
   {
      m_kwl->add( rspfKeywordNames::ELEVATION_ANGLE_KW, tempString1.c_str() );
   }

   if ( ap.read("-e", stringParam1) || ap.read("--entry", stringParam1) )
   {
      m_kwl->add( rspfKeywordNames::ENTRY_KW, tempString1.c_str() );
   }

   if ( ap.read("--exaggeration", stringParam1) )
   {
      m_kwl->add( GAIN_KW, tempString1.c_str() );
   }

   if ( ap.read("--histogram-op", stringParam1) )
   {
      m_kwl->add( HISTO_OP_KW, tempString1.c_str() );
   }

   while( ap.read("--input-dem", stringParam1) )
   {
      key = DEM_KW;
      key += rspfString::toString(demIdx);
      key += ".";
      key += FILE_KW;
      m_kwl->add( key, tempString1.c_str() );
      ++demIdx;
   }
   
   while( ap.read("--input-img", stringParam1) )
   {
      key = IMG_KW;
      key += rspfString::toString(imgIdx);
      key += ".";
      key += FILE_KW;
      m_kwl->add(key, tempString1.c_str() );
      ++imgIdx;
   }

   if( ap.read("--input-src", stringParam1) )
   {
      m_kwl->add( SRC_FILE_KW, tempString1.c_str() );
   }

   if( ap.read("--meters", stringParam1) )
   {
      m_kwl->add( METERS_KW, tempString1.c_str() );
   }

   if ( ap.read("-n") || ap.read("--north-up") )
   {
      m_kwl->add( NORTH_UP_KW, TRUE_KW);
   }

   if( ap.read("--op", stringParam1) )
   {
      m_kwl->add( OP_KW, tempString1.c_str() );
   }

   if( ap.read("--options-keyword-list", stringParam1) )
   {
      rspfFilename optionsKwl = tempString1;
      if ( optionsKwl.exists() )
      {
         if ( m_kwl->addFile(optionsKwl) == false )
         {
            std::string errMsg = "ERROR could not open options keyword list file: ";
            errMsg += optionsKwl.string();
            throw rspfException(errMsg);
         }
      }
      else
      {
         std::string errMsg = "ERROR options keyword list file does not exists: ";
         errMsg += optionsKwl.string();
         throw rspfException(errMsg); 
      }
   }
   
   if( ap.read("--origin-latitude", stringParam1) )
   {
      m_kwl->add( rspfKeywordNames::ORIGIN_LATITUDE_KW, tempString1.c_str() );
   }

   if(ap.read("--output-radiometry", stringParam1))
   {
      m_kwl->add( OUTPUT_RADIOMETRY_KW, tempString1.c_str() );
   }

   if( ap.read("--projection", stringParam1) )
   {
      m_kwl->add( rspfKeywordNames::PROJECTION_KW, tempString1.c_str() );
   }

   if( ap.read("--resample-filter", stringParam1) )
   {
      m_kwl->add( RESAMPLER_FILTER_KW, tempString1.c_str() );
   }

   if ( ap.read("--scale-to-8-bit") )
   {
      m_kwl->add( SCALE_2_8_BIT_KW, TRUE_KW);
   }

   if ( ap.read("--snap-tie-to-origin") )
   {
      m_kwl->add( SNAP_TIE_TO_ORIGIN_KW, TRUE_KW);
   }
   
   if( ap.read("--srs", stringParam1) )
   {
      if ( tempString1.contains("EPSG:") )
      {
         tempString1.gsub( rspfString("EPSG:"), rspfString("") );
      }
      m_kwl->add( SRS_KW, tempString1.c_str() );
   }

   if( ap.read("-t", stringParam1) || ap.read("--thumbnail", stringParam1) )
   {
      m_kwl->add( THUMBNAIL_RESOLUTION_KW, tempString1.c_str() );
   }

   if ( ap.read("-u") || ap.read("--up-is-up") )
   {
      m_kwl->add( UP_IS_UP_KW, TRUE_KW);
   }

   if( ap.read("-w", stringParam1) || ap.read("--writer", stringParam1) )
   {
      m_kwl->add( WRITER_KW, tempString1); 
   }

   while (ap.read("--writer-prop", stringParam1))
   {
      key = WRITER_PROPERTY_KW;
      key += rspfString::toString(propIdx);
      m_kwl->add(key, tempString1.c_str() );
      ++propIdx;
   }
   
   if( ap.read("--2cmv-old-input-band", stringParam1) )
   {
      m_kwl->add( TWOCMV_OLD_INPUT_BAND_KW, tempString1.c_str() );
   }

   if( ap.read("--2cmv-new-input-band", stringParam1) )
   {
      m_kwl->add( TWOCMV_NEW_INPUT_BAND_KW, tempString1.c_str() );
   }
   if( ap.read("--2cmv-red-output-source", stringParam1) )
   {
      m_kwl->add( TWOCMV_RED_OUTPUT_SOURCE_KW, tempString1.c_str() );
   }
   
   if( ap.read("--2cmv-green-output-source", stringParam1) )
   {
      m_kwl->add( TWOCMV_GREEN_OUTPUT_SOURCE_KW, tempString1.c_str() );
   }
   
   if( ap.read("--2cmv-blue-output-source", stringParam1) )
   {
      m_kwl->add( TWOCMV_BLUE_OUTPUT_SOURCE_KW, tempString1.c_str() );
   }

   // End of arg parsing.
   ap.reportRemainingOptionsAsUnrecognized();
   if ( ap.errors() )
   {
      ap.writeErrorMessages(rspfNotify(rspfNotifyLevel_NOTICE));
      std::string errMsg = "Unknown option...";
      throw rspfException(errMsg);
   }

   if ( ap.argc() >= 2 )
   {
      // Output file is last arg:
      m_kwl->add( rspfKeywordNames::OUTPUT_FILE_KW, ap[ap.argc()-1]);
   }
   else
   {
      if (  !m_kwl->find(rspfKeywordNames::OUTPUT_FILE_KW) )
      {
         ap.writeErrorMessages(rspfNotify(rspfNotifyLevel_NOTICE));
         std::string errMsg = "Must supply an output file.";
         throw rspfException(errMsg);
      }
   }

   if ( ap.argc() > 2 ) // User passed inputs in front of output file.
   {
      int pos = 1; // ap.argv[0] is application name. 
      while ( pos < (ap.argc()-1) )
      {
         rspfFilename file = ap[pos];
         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "argv[" << pos << "]: " << file << "\n";
         }
         
         if ( isDemFile(file) )
         {
            key = DEM_KW;
            key += rspfString::toString(demIdx);
            key += ".";
            key += FILE_KW;
            m_kwl->add( key, file.c_str() );
            ++demIdx;
         }
         else if ( isSrcFile(file) ) 
         {
            if ( m_kwl->find( SRC_FILE_KW ) ) // --input-src used also
            {
               std::string errMsg = MODULE;
               errMsg += "ERROR Multiple src files passed in.  Please combine into one.";
               throw rspfException(errMsg);
            }
            
            m_kwl->add( SRC_FILE_KW, file.c_str() );
         }
         else // Add as an input image.
         {
            key = IMG_KW;
            key += rspfString::toString(imgIdx);
            key += ".";
            key += FILE_KW;
            m_kwl->add(key, file.c_str() );
            ++imgIdx;
         }
         
         ++pos; // Go to next arg...
         
      } // End: while ( pos < (ap.argc()-1) )
       
   } // End: if ( ap.argc() > 2 )

   initialize();
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited..." << std::endl;
   }  
   return true;
   
} // End: void rspfChipperUtil::initialize(rspfArgumentParser& ap)

void rspfChipperUtil::initialize()
{
   static const char MODULE[] = "rspfChipperUtil::initialize()";

   if ( traceOptions() )
   {
      rspfFilename optionsFile;
      getOutputFilename(optionsFile);
      optionsFile = optionsFile.noExtension();
      optionsFile += "-options.kwl";
      rspfString comment = " Can be use for --options-keyword-list argument.";
      m_kwl->write( optionsFile.c_str(), comment.c_str() );
   } 

   // Determine the operation to do.
   std::string op = m_kwl->findKey( std::string(OP_KW) );
   if ( op.size() )
   {
      rspfString s = op;
      s.downcase();
      
      if ( s == "chip" )
      {
         m_operation = RSPF_CHIPPER_OP_CHIP;
      }
      else if ( s == "hillshade" )
      {
         m_operation = RSPF_CHIPPER_OP_HILL_SHADE;
      }

      else if ( s == "color-relief" )
      {
         m_operation = RSPF_CHIPPER_OP_COLOR_RELIEF;
      }
      else if ( s == "ortho" )
      {
         m_operation = RSPF_CHIPPER_OP_ORTHO;
      }
      else if ( s == "2cmv" )
      {
         m_operation = RSPF_CHIPPER_OP_2CMV;
      }
      else
      {
         std::string errMsg = "unknown operation: ";
         errMsg += s.string();
         throw rspfException(errMsg);
      }
   }
   else
   {
      std::string errMsg = "keyword not found: ";
      errMsg += OP_KW;
      errMsg += "\nUse --op option to specify operation.\n";
      throw rspfException(errMsg);  
   }

   //---
   // Populate the m_srcKwl if --src option was set.
   // Note do this before creating chains.
   //---
   initializeSrcKwl();
   
   // Check for required inputs. Do this after initializeSrcKwl.
   if ( m_operation == RSPF_CHIPPER_OP_CHIP )
   {
      if ( getNumberOfInputs() != 1 )
      {
         std::ostringstream errMsg;
         errMsg << op << " operation takes one input.";
         throw rspfException( errMsg.str() );
      }
   }
   
   if ( m_operation == RSPF_CHIPPER_OP_2CMV )
   {
      if ( getNumberOfInputs() != 2 )
      {
         std::ostringstream errMsg;
         errMsg << op << " operation requires two inputs.";
         throw rspfException( errMsg.str() );
      }
   }

   if ( upIsUp() || northUp() )
   {
      std::string option = "up_is_up";
      if ( northUp() )
      {
         option = "north_up";
      }
         
      // One input, chip operation only.
      if ( getNumberOfInputs() != 1 )
      {
         std::ostringstream errMsg;
         errMsg << option << " option takes one input.";
         throw rspfException( errMsg.str() );
      }
      if ( m_operation != RSPF_CHIPPER_OP_CHIP )
      {
         std::ostringstream errMsg;
         errMsg << option << " option only valid with \"chip\" operation.";
         throw rspfException( errMsg.str() );
      }
      if ( upIsUp() && northUp() )
      {
         std::ostringstream errMsg;
         errMsg << "Having up_is_up AND north_up turned on doesn't make sense!";
         throw rspfException( errMsg.str() );
      }
   }

   // Create chains for any dem sources.
   addDemSources();

   // Create chains for any image sources.
   addImgSources();

   // Initialize projection and propagate to chains.
   initializeOutputProjection();
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }

} // End: void rspfChipperUtil::initialize()

void rspfChipperUtil::initializeOutputProjection()
{
   if ( isIdentity() )
   {
      createIdentityProjection();
   }
   else
   {
      // Create the output projection.
      createOutputProjection();
      
      // Setup the view in all the chains.
      propagateOutputProjectionToChains();
   }
}

void rspfChipperUtil::execute()
{
   static const char MODULE[] = "rspfChipperUtil::execute";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   const char* lookup = 0;  // used throughout...

   rspfRefPtr<rspfImageSource> source = 0;

   if ( hasBumpShadeArg() )
   {
      // Combine the dems.
      rspfRefPtr<rspfImageSource> demSource = combineLayers( m_demLayer );

      // Set up the normal source.
      rspfRefPtr<rspfImageToPlaneNormalFilter> normSource = new rspfImageToPlaneNormalFilter;

      //---
      // Set the track scale flag to true.  This enables scaling the surface
      // normals by the GSD in order to maintain terrain proportions.
      //---
      normSource->setTrackScaleFlag(true);

      // Connect to dems.
      normSource->connectMyInputTo( demSource.get() );

      // Set the smoothness factor.
      rspf_float64 gain = 1.0;
      lookup = m_kwl->find( GAIN_KW );
      if ( lookup )
      {
         gain = rspfString::toFloat64(lookup);
      }
      normSource->setSmoothnessFactor(gain);

      rspfRefPtr<rspfImageSource> colorSource = 0;
      if ( hasLutFile() )
      {
         if ( m_imgLayer.size() )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " WARNING:"
               << "\nBoth a color table and image(s) have been provided for a color source.\n"
               << "Choosing color table of image(s).\n";
         }

         colorSource = addIndexToRgbLutFilter( demSource );
      }
      else
      {
         // Combine the images and set as color source for bump shade.
         colorSource = combineLayers( m_imgLayer );
      }

      // Create the bump shade.
      rspfRefPtr<rspfBumpShadeTileSource> bumpShade = new rspfBumpShadeTileSource;

      // Set the azimuth angle.
      rspf_float64 azimuthAngle = 180;
      lookup = m_kwl->find( rspfKeywordNames::AZIMUTH_ANGLE_KW );
      if ( lookup )
      {
         rspf_float64 f = rspfString::toFloat64(lookup);
         if ( (f >= 0) && (f <= 360) )
         {
            azimuthAngle = f;
         }
      }
      bumpShade->setAzimuthAngle(azimuthAngle);

      // Set the elevation angle.
      rspf_float64 elevationAngle = 45.0;
      lookup = m_kwl->find( rspfKeywordNames::ELEVATION_ANGLE_KW );
      if ( lookup )
      {
         rspf_float64 f = rspfString::toFloat64(lookup);
         if ( (f >= 0.0) && (f <= 90) )
         {
            elevationAngle = f;
         }
      }
      bumpShade->setElevationAngle(elevationAngle);

      if ( !hasLutFile() )
      {
         // Set the color.
         rspf_uint8 r = 0xff;
         rspf_uint8 g = 0xff;
         rspf_uint8 b = 0xff;
         lookup = m_kwl->find( COLOR_RED_KW );
         if ( lookup )
         {
            r = rspfString(lookup).toUInt8();
         }
         lookup = m_kwl->find( COLOR_GREEN_KW );
         if ( lookup )
         {
            g = rspfString(lookup).toUInt8();
         }
         lookup = m_kwl->find( COLOR_BLUE_KW );
         if ( lookup )
         {
            b = rspfString(lookup).toUInt8();
         }
         bumpShade->setRgbColorSource(r, g, b);
      }

      // Connect the two sources.
      bumpShade->connectMyInputTo(0, normSource.get());
      bumpShade->connectMyInputTo(1, colorSource.get());

      if ( traceDebug() )
      {
         rspf_uint8 r = 0xff;
         rspf_uint8 g = 0xff;
         rspf_uint8 b = 0xff;
         bumpShade->getRgbColorSource(r, g, b);
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "\nazimuthAngle:      " << azimuthAngle
            << "\nelevation angle:   " << elevationAngle
            << "\ngain factor:       " << gain
            << "\nr:                 " << int(r)
            << "\ng:                 " << int(g)
            << "\nb:                 " << int(b)
            << "\n";
      }

      // Capture the pointer to give to the writer.
      source = bumpShade.get();
   }
   else if ( m_operation == RSPF_CHIPPER_OP_COLOR_RELIEF )
   {
      source = combineLayers();
      if ( hasLutFile() )
      {
         source = addIndexToRgbLutFilter( source );
      }
      else
      {
         // No LUT file provided, so doing the default 8-bit linear stretch:
         if ( source->getOutputScalarType() != RSPF_UINT8 )
         {
            source = addScalarRemapper( source, RSPF_UINT8 );
         }
      }
   }
   else if ( ( m_operation == RSPF_CHIPPER_OP_CHIP ) ||
             ( m_operation == RSPF_CHIPPER_OP_ORTHO ) )  
   {
      source = combineLayers();
   }
   else if ( m_operation == RSPF_CHIPPER_OP_2CMV )
   {
      source = combine2CmvLayers(); // Two Color Multiview.
   }

   if ( source.valid() )
   {
      //---
      // This is conditional.  Output radiometry may of may not be set.  This can also be set at
      // the rspfSingleImageChain level.
      //---
      if ( ( getOutputScalarType() != RSPF_SCALAR_UNKNOWN) &&
           ( source->getOutputScalarType() != getOutputScalarType() ) )
      {
         source = addScalarRemapper( source, getOutputScalarType() );
      }
      
      //---
      // Get the area of interest. This will be the scene bounding rect if not
      // explicitly set by user with one of the --cut options.
      //  Need to get this before the thumbnail code.
      //---
      rspfIrect aoi;
      getAreaOfInterest(source.get(), aoi);

      //---
      // Set the image size here.  Note must be set after combineLayers.  This is needed for
      // the rspfImageGeometry::worldToLocal call for a geographic projection to handle wrapping
      // accross the date line.
      //---
      m_geom->setImageSize( aoi.size() );

      if ( hasThumbnailResolution() )
      {
         //---
         // Adjust the projection scale and get the new rect.
         // Note this will resize the rspfImageGeometry::m_imageSize is scale changes.
         //---
         initializeThumbnailProjection( aoi, aoi );

         // Reset the source bounding rect if it changed.
         source->initialize();
      }

      // Set up the writer.
      rspfRefPtr<rspfImageFileWriter> writer = createNewWriter();

      // Connect the writer.
      writer->connectMyInputTo(0, source.get());

      //---
      // Set the area of interest.
      // NOTE: This must be called after the writer->connectMyInputTo as
      // rspfImageFileWriter::initialize incorrectly resets theAreaOfInterest
      // back to the bounding rect.
      //---
      if ( !aoi.hasNans() )
      {
         writer->setAreaOfInterest(aoi);
      }
      
      if (writer->getErrorStatus() == rspfErrorCodes::RSPF_OK)
      {
         // Add a listener to get percent complete.
         rspfStdOutProgress prog(0, true);
         writer->addListener(&prog);

         if ( traceLog() )
         {
            rspfKeywordlist logKwl;
            writer->saveStateOfAllInputs(logKwl);
            
            rspfFilename logFile;
            getOutputFilename(logFile);
            logFile.setExtension("log");
            rspfKeywordlist kwl;
            writer->saveStateOfAllInputs(kwl);
            kwl.write(logFile.c_str() );
         }
         
         // Write the file:
         writer->execute();

         writer->removeListener(&prog);
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
}

void rspfChipperUtil::addDemSources()
{
   static const char MODULE[] = "rspfChipperUtil::addDemSources";
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   // Add the images from the options keyword list.
   rspf_uint32 demCount = m_kwl->numberOf(DEM_KW);
   rspf_uint32 maxIndex = demCount + 1000; // Allow for skippage in numbering.
   rspf_uint32 foundRecords = 0;
   rspf_uint32 i = 0;
   while ( foundRecords < demCount )
   {
      rspfString key = DEM_KW;
      key += rspfString::toString(i);
      key += ".";
      key += FILE_KW;
      const char* lookup = m_kwl->find( key.c_str() );
      if ( lookup )
      {
         rspfFilename f = lookup;
         addDemSource(f);
         ++foundRecords;
      }
      ++i;
      if ( i >= maxIndex ) break;
   }

   if ( m_srcKwl.valid() )
   {
      // Add stuff from src keyword list.
      demCount = m_srcKwl->numberOf(DEM_KW);
      maxIndex = demCount + 1000;
      foundRecords = 0;
      i = 0;
      while ( foundRecords < demCount )
      {
         rspfString prefix = DEM_KW;
         prefix += rspfString::toString(i);
         prefix += ".";
         rspfSrcRecord src;
         if ( src.loadState( *(m_srcKwl.get()), prefix ) )
         {
            addDemSource(src);
            ++foundRecords;
         }
         ++i;
         if ( i >= maxIndex ) break;
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   } 
}

void rspfChipperUtil::addDemSource(const rspfFilename& file)
{
   static const char MODULE[] = "rspfChipperUtil::addDemSource(const rspfFilename&)";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfSingleImageChain> ic = createChain(file);
   if ( ic.valid() )
   {
      m_demLayer.push_back(ic);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exiting...\n";
   }
}

void rspfChipperUtil::addDemSource(const rspfSrcRecord& rec)
{
   static const char MODULE[] = "rspfChipperUtil::addDemSource(const rspfSrcRecord&)";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfSingleImageChain> ic = createChain(rec);
   if ( ic.valid() )
   {
      m_demLayer.push_back(ic);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exiting...\n";
   }
}

void rspfChipperUtil::addImgSources()
{
   static const char MODULE[] = "rspfChipperUtil::addImgSources";
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   
   rspf_uint32 imgCount = m_kwl->numberOf(IMG_KW);
   rspf_uint32 maxIndex = imgCount + 1000; // Allow for skippage in numbering.
   rspf_uint32 foundRecords = 0;
   rspf_uint32 i = 0;
   while ( foundRecords < imgCount )
   {
      rspfString key = IMG_KW;
      key += rspfString::toString(i);
      key += ".";
      key += FILE_KW;
      const char* lookup = m_kwl->find( key.c_str() );
      if ( lookup )
      {
         rspfFilename f = lookup;
         addImgSource(f);
         ++foundRecords;
      }
      ++i;
      if ( i >= maxIndex ) break;
   }

   if ( m_srcKwl.valid() )
   {
      // Add stuff from src keyword list.
      imgCount = m_srcKwl->numberOf(IMG_KW);
      maxIndex = imgCount + 1000;
      foundRecords = 0;
      i = 0;
      while ( foundRecords < imgCount )
      {
         rspfString prefix = IMG_KW;
         prefix += rspfString::toString(i);
         prefix += ".";
         rspfSrcRecord src;
         if ( src.loadState( *(m_srcKwl.get()), prefix ) )
         {
            addImgSource(src);
            ++foundRecords;
         }
         ++i;
         if ( i >= maxIndex ) break;
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::addImgSource(const rspfFilename& file)
{
   static const char MODULE[] = "rspfChipperUtil::addImgSource";
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfSingleImageChain> ic = createChain(file);
   if ( ic.valid() )
   {
      m_imgLayer.push_back(ic);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exiting...\n";
   }
}

void rspfChipperUtil::addImgSource(const rspfSrcRecord& rec)
{
   static const char MODULE[] = "rspfChipperUtil::addImgSource(const rspfSrcRecord&)";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfSingleImageChain> ic = createChain(rec);
   if ( ic.valid() )
   {
      m_imgLayer.push_back(ic);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exiting...\n";
   }
}

rspfRefPtr<rspfSingleImageChain> rspfChipperUtil::createChain(const rspfFilename& file) const
{
   static const char MODULE[] = "rspfChipperUtil::createChain(const rspfFilename&";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..." << "\nfile: " << file << "\n";
   }   
   
   rspfRefPtr<rspfSingleImageChain> ic = 0;

   if ( file.size() )
   {
      if ( file.exists() )
      {
         ic = new rspfSingleImageChain;
         if ( ic->open( file ) )
         {
            // Set the entry if not zero.
            rspf_uint32 entryIndex = getEntryNumber();
            if ( entryIndex )
            {
               if ( setChainEntry( ic, entryIndex ) == false )
               {
                  std::ostringstream errMsg;
                  errMsg << MODULE << " ERROR:\nEntry " << entryIndex << " out of range!"
                         << std::endl;
                  throw rspfException( errMsg.str() );
               }
            }

            // Bands selection: Two pieces here and then after the initialize of chain.
            std::vector<rspf_uint32> bandList(0);
            getBandList( bandList );
            if ( bandList.size() == 3 )
            {
               if ( ( bandList[0] == 0 ) && ( bandList[1] == 1 ) && ( bandList[2] == 2 ) )
               {
                  ic->setThreeBandFlag( true );
                  
                  bandList.clear(); // Clear list as we don't need now.
               }
               else if ( ( bandList[0] == 2 ) && ( bandList[1] == 1 ) && ( bandList[2] == 0 ) )
               {
                  ic->setThreeBandReverseFlag( true );
                  
                  bandList.clear(); // Clear list as we don't need now.
               }
            }
            
            //---
            // If multiple inputs and scaleToEightBit do it at the end of the processing
            // chain to alleviate un-even stretches between inputs.
            //---
            bool scaleFlag = ( scaleToEightBit() && ( getNumberOfInputs() == 1) );
            ic->setRemapToEightBitFlag( scaleFlag );
            
            // Always have resampler cache.
            ic->setAddResamplerCacheFlag(true);

            //---
            // Don't need a chain cache as we're doing a sequential write.  So the same tile
            // should never be visited more than once.
            //---
            ic->setAddChainCacheFlag(false);

            // Histogram.
            ic->setAddHistogramFlag( hasHistogramOperation() );

            // Create the chain.
            ic->createRenderedChain();

            // Band selection: Do this post creation of chain if needed.
            if ( bandList.size() )
            {
               ic->setBandSelection( bandList );
            }

            // Set the filter type if needed.
            const char* lookup = m_kwl->find( RESAMPLER_FILTER_KW );
            if ( lookup )
            {
               ic->getImageRenderer()->getResampler()->setFilterType(rspfString(lookup));
            }

            // Histogram setup.
            if ( hasHistogramOperation() )
            {
               setupChainHistogram( ic );
            }
         }
      }
   }

   if ( ic.valid() == false )
   {
      std::string errMsg = "Could not open: ";
      errMsg += file.string();
      throw rspfException(errMsg); 
   }

   if ( traceDebug() )
   {
      rspfKeywordlist kwl;
      ic->saveState(kwl, 0);

      rspfNotify(rspfNotifyLevel_DEBUG)
         << "chain:\n" << kwl << "\n"
         << MODULE << " exiting...\n";
   }   

   return ic;
}

rspfRefPtr<rspfSingleImageChain> rspfChipperUtil::createChain(const rspfSrcRecord& rec) const
{
   static const char MODULE[] = "rspfChipperUtil::createChain(const rspfSrcRecord&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }   
   
   rspfRefPtr<rspfSingleImageChain> ic = new rspfSingleImageChain;
   if ( ic->open(rec) )
   {
      //---
      // If multiple inputs and scaleToEightBit do it at the end of the processing
      // chain to alleviate un-even strectes between inputs.
      //---
      bool scaleFlag = ( scaleToEightBit() && ( getNumberOfInputs() == 1) );
      ic->setRemapToEightBitFlag( scaleFlag );
      
      // Always have resampler cache.
      ic->setAddResamplerCacheFlag(true);
      
      //---
      // Don't need a chain cache as we're doing a sequential write.  So the same tile
      // should never be visited more than once.
      //---
      ic->setAddChainCacheFlag(false);

      // Histogram.
      ic->setAddHistogramFlag( hasHistogramOperation() );

      //---
      // NOTE: Histogram and band selector can be set in ic->createRenderedChain(rec)
      // if the right keywords are there.
      //---
      ic->createRenderedChain(rec);

      // Set the filter type if needed.
      const char* lookup = m_kwl->find( RESAMPLER_FILTER_KW );
      if ( lookup )
      {
         ic->getImageRenderer()->getResampler()->setFilterType(rspfString(lookup));
      }
   }
   else // Open failed.
   {
      std::string errMsg = "Could not open from src record!";
      throw rspfException(errMsg); 
   }

   if ( traceDebug() )
   {
      rspfKeywordlist kwl;
      ic->saveState(kwl, 0);

      rspfNotify(rspfNotifyLevel_DEBUG)
         << "chain:\n" << kwl << "\n"
         << MODULE << " exiting...\n";
   }   

   return ic;
}
   
void rspfChipperUtil::createOutputProjection()
{
   static const char MODULE[] = "rspfChipperUtil::createOutputProjection";
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   const char* op  = m_kwl->find(rspfKeywordNames::PROJECTION_KW);
   const char* srs = m_kwl->find(SRS_KW);
   
   if ( op && srs )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " WARNING:"
         << "\nBoth " << SRS_KW << " and " << rspfKeywordNames::PROJECTION_KW
         << " keywords are set!"
         << "\nsrs:               " << srs
         << "\noutput_projection: " << op
         << "\nTaking " << srs << " over " << op << "\n";
   }
   
   bool usingInput = false;
   rspfChipperOutputProjection projType = getOutputProjectionType();
   rspfRefPtr<rspfMapProjection> proj = 0;
   
   // If an srs code use that first.
   if (srs)
   {
      proj = getNewProjectionFromSrsCode( rspfString(srs) );
   }
   else if (op)
   {
      switch ( projType )
      {
         case rspfChipperUtil::RSPF_CHIPPER_PROJ_GEO:
         {
            proj = getNewGeoProjection();
            break;
         }
         case rspfChipperUtil::RSPF_CHIPPER_PROJ_GEO_SCALED:
         {
            proj = getNewGeoScaledProjection();
            break;
         }
         case rspfChipperUtil::RSPF_CHIPPER_PROJ_INPUT:
         {
            proj = getFirstInputProjection();
            usingInput = true;
            break;
         }
         case rspfChipperUtil::RSPF_CHIPPER_PROJ_UTM:
         {
            proj = getNewUtmProjection();
            break;
         }
         default:
         {
            break; // Just for un-handled type warning.
         }
      }
   }
   
   // Check for identity projection:
   rspfRefPtr<rspfMapProjection> inputProj = getFirstInputProjection();   
   if ( proj.valid() && inputProj.valid() )
   {
      if ( *(inputProj.get()) == *(proj.get()) )
      {
         if ( projType == RSPF_CHIPPER_PROJ_GEO_SCALED )
         {
            // Get the origin used for scaling. 
            rspfGpt origin = proj->getOrigin();

            // Copy the input projection to our projection.  Has the tie and scale we need.
            proj = inputProj;

            // Set the origin for scaling.
            proj->setOrigin(origin);
         }
         else
         {
            proj = inputProj;
         }
         usingInput = true;
      }
   }
   
   if ( !proj.valid() )
   {
      // Try first input. If map projected use that.
      if ( inputProj.valid() )
      {
         proj = inputProj;
         usingInput = true;
         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "WARNING: No projection set!"
               << "\nDefaulting to first input's projection.\n";
         }
      }
      else
      {
         proj = getNewGeoScaledProjection();
         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "WARNING: No projection set!"
               << "\nDefaulting to scaled geographic at scene center.\n";
         }
      }
   }

   // Create our rspfImageGeometry with projection (no transform).
   m_geom  = new rspfImageGeometry( 0, proj.get() );

   //---
   // If the input is the same as output projection do not modify; else, set
   // the gsd to user selected "METERS_KW" or the best resolution of the inputs,
   // set the tie and then snap it to the projection origin.
   //---
   if ( !usingInput || m_kwl->find(METERS_KW) || m_kwl->find(DEGREES_X_KW))
   {
      // Set the scale.
      initializeProjectionGsd();
   }

   // Set the tie.
   intiailizeProjectionTiePoint();

   if ( snapTieToOrigin() )
   {
      // Adjust the projection tie to the origin.
      proj->snapTiePointToOrigin();
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "using input projection: " << (usingInput?"true":"false")
         << "\noutput image geometry:\n";

      m_geom->print(rspfNotify(rspfNotifyLevel_DEBUG));

      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
   
} // End: rspfChipperUtil::createOutputProjection()

void rspfChipperUtil::createIdentityProjection()
{
   static const char MODULE[] = "rspfChipperUtil::createIdentityProjection";
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   // Get the singe image chain.  Sould be only one.
   rspfRefPtr<rspfSingleImageChain> sic = 0;
   if ( m_demLayer.size() )
   {
      sic = m_demLayer[0];
   }
   else if ( m_imgLayer.size() )
   {
      sic = m_imgLayer[0];
   }

   if ( sic.valid() )
   {
      // Get the image handler.
      rspfRefPtr<rspfImageHandler>  ih = sic->getImageHandler();

      // Resampler:
      rspfRefPtr<rspfImageRenderer> resampler = sic->getImageRenderer();

      if ( ih.valid() )
      {
         //---
         // Get the geometry from the image handler.  Since we're in "identity"
         // mode use the inputs for the outputs.
         //---
         m_geom = ih->getImageGeometry();
         
         if ( m_geom.valid() )
         {
            // Get the image projection.
            rspfRefPtr<rspfProjection> proj = m_geom->getProjection();
            if ( proj.valid() )
            {
               rspf_float64 rotation = 0.0;
               if ( upIsUp() )
               {
                  rotation = m_geom->upIsUpAngle();
               }
               else if ( northUp() )
               {
                  rotation = m_geom->northUpAngle();
               }

               if ( rspf::isnan( rotation ) )
               {
                  rotation = 0.0;
               }     
               
               rspfDrect rect;
               m_geom->getBoundingRect(rect);
               rspfDpt midPt = rect.midPoint();
               
               if ( traceDebug() )
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << MODULE
                     << "\nAffine transform parameters:"
                     << "\nrotation:  " << rotation
                     << "\nmid point: " << midPt << std::endl;
               }
               
               m_ivt = new rspfImageViewAffineTransform(-rotation,
                                                         1.0,1.0, //scale x and y
                                                         0.0, 0.0, // translate x,y
                                                         midPt.x, midPt.y); // pivot point

               if ( m_kwl->find(METERS_KW) || m_kwl->find(DEGREES_X_KW ) )
               {
                  // Set the image view transform scale.
                  initializeIvtScale();
               }
               
               resampler->setImageViewTransform( m_ivt.get() );
            }

         } // Matches: if ( m_geom.valid() )
         
      } // Matches: if ( ih.valid() )
      
   } // Matches: if ( sic.valid() 
   
} // End: createIdentityProjection()

void rspfChipperUtil::initializeIvtScale()
{
   rspfDpt scale;
   scale.makeNan();
   
   if ( isIdentity() && m_ivt.valid() && m_geom.valid() )
   {
      // Check for GSD spec. Degrees/pixel takes priority over meters/pixel:
      const char* lookup = m_kwl->find(DEGREES_X_KW);
      if ( lookup )
      {
         rspfDpt outputDpp;
         outputDpp.makeNan();

         outputDpp.x = rspfString::toFloat64(lookup);

         lookup = m_kwl->find(DEGREES_Y_KW);
         if ( lookup )
         {
            outputDpp.y = rspfString::toFloat64(lookup);
         }
         
         if ( !outputDpp.hasNans() )
         {
            // Input degress per pixel.  Consider this a scale of 1.0.
            rspfDpt inputDpp;
            m_geom->getDegreesPerPixel( inputDpp );

            if ( !inputDpp.hasNans() )
            {
               scale.x = outputDpp.x/inputDpp.x;
               scale.y = outputDpp.y/inputDpp.y;
            }
         }
      }
      else
      {
         lookup = m_kwl->find(METERS_KW);
         if ( lookup )
         {
            rspfDpt outputMpp;
            outputMpp.makeNan();
            outputMpp.x = rspfString::toFloat64(lookup);
            outputMpp.y = outputMpp.x;

            if ( !outputMpp.hasNans() )
            {
               // Input meters per pixel.  Consider this a scale of 1.0.
               rspfDpt inputMpp;
               m_geom->getMetersPerPixel( inputMpp );

               if ( !inputMpp.hasNans() )
               {
                  scale.x = outputMpp.x/inputMpp.x;
                  scale.y = outputMpp.y/inputMpp.y;
               }
            }
         }
      }
   }

   if ( !scale.hasNans() )
   {
      m_ivt->scale( scale.x, scale.y );
   }
   else
   {
      std::string errMsg = "rspfChipperUtil::initializeIvtScale failed!";
      throw rspfException(errMsg);
   }
   
} // End: rspfChipperUtil::initializeIvtScale()

void rspfChipperUtil::intiailizeProjectionTiePoint()
{
   static const char MODULE[] = "rspfChipperUtil::initializeProjectionTiePoint()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   // Get the map projection from the output geometry:
   rspfRefPtr<rspfMapProjection> mapProj = getMapProjection();

   if ( mapProj.valid() )
   {
      //---
      // If the output is geographic of there are sensor model inputs, get the tie
      // using the ground point.
      //---
      if ( mapProj->isGeographic() || hasSensorModelInput() )
      {
         rspfGpt tiePoint;
         tiePoint.makeNan();
         getTiePoint(tiePoint);
         
         if ( !tiePoint.hasNans() )
         {
            //---
            // The tie point coordinates currently reflect the UL edge of the UL pixel.
            // We'll need to shift the tie point bac from the edge to the center base on the
            // output gsd.
            //---
            rspfDpt half_pixel_shift = m_geom->getDegreesPerPixel() * 0.5;
            tiePoint.lat -= half_pixel_shift.lat;
            tiePoint.lon += half_pixel_shift.lon;
            mapProj->setUlTiePoints(tiePoint);
         }
         else
         {
            std::string errMsg = MODULE;
            errMsg += " tie point has nans!";
            throw( rspfException(errMsg) );
         }

         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "projection tie point: " << tiePoint << "\n" << MODULE << " exited...\n";
         }
      }
      else
      {
         //---
         // TODO: Add test for like input projections and use above geographic tie
         // code if not.
         //---
         rspfDpt tiePoint;
         tiePoint.makeNan();
         getTiePoint(tiePoint);

         if ( !tiePoint.hasNans() )
         {
            //---
            // The tie point coordinates currently reflect the UL edge of the UL pixel.
            // We'll need to shift the tie point bac from the edge to the center base on the
            // output gsd.
            //---
            rspfDpt half_pixel_shift = m_geom->getMetersPerPixel() * 0.5;
            tiePoint.y -= half_pixel_shift.y;
            tiePoint.x += half_pixel_shift.x;
            mapProj->setUlTiePoints(tiePoint);
         }
         else
         {
            std::string errMsg = MODULE;
            errMsg += " tie point has nans!";
            throw( rspfException(errMsg) );
         }

         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "projection tie point: " << tiePoint << "\n" << MODULE << " exited...\n";
         }
      }
      
   } // Matches: if ( mapProj.valid() )
   else
   {
      std::string errMsg = MODULE;
      errMsg += "m_projection is null!";
      throw( rspfException(errMsg) ); 
   }
}

void rspfChipperUtil::initializeProjectionGsd()
{
   static const char MODULE[] = "rspfChipperUtil::initializeProjectionGsd()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfMapProjection> mapProj = getMapProjection();

   if ( !mapProj.valid() )
   {
      std::string errMsg = MODULE;
      errMsg += "projection is null!";
      throw( rspfException(errMsg) ); 
   }
   
   rspfDpt gsd;
   gsd.makeNan();

   // Check for GSD spec. Degrees/pixel takes priority over meters/pixel:
   const char* lookup = m_kwl->find(DEGREES_X_KW);
   if ( lookup )
   {
      gsd.x = rspfString::toFloat64(lookup);

      lookup = m_kwl->find(DEGREES_Y_KW);
      if ( lookup )
      {
         gsd.y = rspfString::toFloat64(lookup);
      }
      if ( !gsd.hasNans() )
      {
         mapProj->setDecimalDegreesPerPixel(gsd);
      }
   }
   else
   {
      lookup = m_kwl->find(METERS_KW);
      if ( lookup )
      {
         gsd.x = rspfString::toFloat64(lookup);
         gsd.y = gsd.x;
         if ( !gsd.hasNans() )
         {
            mapProj->setMetersPerPixel(gsd);
         }
      }
   }

   if ( gsd.hasNans() )
   {
      // Get the best resolution from the inputs.
      getMetersPerPixel(gsd);

      // See if the output projection is geo-scaled; if so, make the pixels square in meters.
      if ( getOutputProjectionType() == rspfChipperUtil::RSPF_CHIPPER_PROJ_GEO_SCALED )
      {
         // Pick the best resolution and make them both the same.
         gsd.x = rspf::min<rspf_float64>(gsd.x, gsd.y);
         gsd.y = gsd.x;
      }

      // Set to input gsd.
      mapProj->setMetersPerPixel(gsd);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "projection gsd: " << gsd << "\n" << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getTiePoint(rspfGpt& tie)
{
   static const char MODULE[] = "rspfChipperUtil::getTiePoint(rspfGpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx;

   tie.lat = rspf::nan();
   tie.lon = rspf::nan();
   tie.hgt = 0.0;
   
   // Loop through dem layers.
   rspfGpt chainTiePoint;
   chainIdx = m_demLayer.begin();
   while ( chainIdx != m_demLayer.end() )
   {
      getTiePoint( (*chainIdx).get(), chainTiePoint );
      if ( tie.hasNans() )
      {
         tie = chainTiePoint;
      }
      else
      {
         if ( chainTiePoint.lat > tie.lat )
         {
            tie.lat = chainTiePoint.lat;
         }
         if ( chainTiePoint.lon < tie.lon )
         {
            tie.lon = chainTiePoint.lon;
         }
      }
      ++chainIdx;
   }

   // Loop through image layers.
   chainIdx = m_imgLayer.begin();
   while ( chainIdx != m_imgLayer.end() )
   {
      getTiePoint( (*chainIdx).get(), chainTiePoint );
      if ( tie.hasNans() )
      {
         tie = chainTiePoint;
      }
      else
      {
         if ( chainTiePoint.lat > tie.lat )
         {
            tie.lat = chainTiePoint.lat;
         }
         if ( chainTiePoint.lon < tie.lon )
         {
            tie.lon = chainTiePoint.lon;
         }
      }
      ++chainIdx;
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "tie point: " << tie << "\n" << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getTiePoint(rspfSingleImageChain* chain, rspfGpt& tie)
{
   static const char MODULE[] = "rspfChipperUtil::getTiePoint(rspfSingleImageChain*,rspfGpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }   

   if (chain && m_geom.valid() )
   {
      //---
      // The view is not set yet in the chain so we get the tie point from the
      // image handler geometry not from the chain which will come from the
      // rspfImageRenderer.
      //---
      rspfRefPtr<rspfImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() )
         {
            geom->getTiePoint( tie, true );
         }
         

         // Set height to 0.0 even though it's not used so hasNans test works.
         tie.hgt = 0.0;
         
         if ( tie.hasNans() )
         {
            std::string errMsg = MODULE;
            errMsg += "\ngeom->localToWorld returned nan for chain.";
            errMsg += "\nChain: ";
            errMsg += chain->getFilename().string();
            throw rspfException(errMsg);
         }
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += "\nNo geometry for chain: ";
         errMsg += chain->getFilename().string();
         throw rspfException(errMsg);
      }
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null chain passed to method!";
      throw rspfException(errMsg);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "chain name: " << chain->getFilename()
         << "\ntie point:  " << tie << "\n"
         << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getTiePoint(rspfDpt& tie)
{
   static const char MODULE[] = "rspfChipperUtil::getTiePoint(rspfDpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx;

   tie.makeNan();
   
   // Loop through dem layers.
   rspfDpt chainTiePoint;
   chainIdx = m_demLayer.begin();
   while ( chainIdx != m_demLayer.end() )
   {
      getTiePoint( (*chainIdx).get(), chainTiePoint );
      if ( tie.hasNans() )
      {
         tie = chainTiePoint;
      }
      else
      {
         if ( chainTiePoint.y > tie.y )
         {
            tie.y = chainTiePoint.y;
         }
         if ( chainTiePoint.x < tie.x )
         {
            tie.x = chainTiePoint.x;
         }
      }
      ++chainIdx;
   }

   // Loop through image layers.
   chainIdx = m_imgLayer.begin();
   while ( chainIdx != m_imgLayer.end() )
   {
      getTiePoint( (*chainIdx).get(), chainTiePoint );
      if ( tie.hasNans() )
      {
         tie = chainTiePoint;
      }
      else
      {
         if ( chainTiePoint.y > tie.y )
         {
            tie.y = chainTiePoint.y;
         }
         if ( chainTiePoint.x < tie.x )
         {
            tie.x = chainTiePoint.x;
         }
      }
      ++chainIdx;
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "tie point: " << tie << "\n" << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getTiePoint(rspfSingleImageChain* chain, rspfDpt& tie)
{
   static const char MODULE[] = "rspfChipperUtil::getTiePoint(rspfSingleImageChain*,rspfDpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }   

   if (chain && m_geom.valid() )
   {
      //---
      // The view is not set yet in the chain so we get the tie point from the
      // image handler geometry not from the chain which will come from the
      // rspfImageRenderer.
      //---
      rspfRefPtr<rspfImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() )
         {
            geom->getTiePoint( tie, true );
         }
         
         if ( tie.hasNans() )
         {
            std::string errMsg = MODULE;
            errMsg += "\ngeom->localToWorld returned nan for chain.";
            errMsg += "\nChain: ";
            errMsg += chain->getFilename().string();
            throw rspfException(errMsg);
         }
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += "\nNo geometry for chain: ";
         errMsg += chain->getFilename().string();
         throw rspfException(errMsg);
      }
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null chain passed to method!";
      throw rspfException(errMsg);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "chain name: " << chain->getFilename()
         << "\ntie point:  " << tie << "\n"
         << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getMetersPerPixel(rspfDpt& gsd)
{
   static const char MODULE[] = "rspfChipperUtil::getMetersPerPixel(rspfDpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   gsd.makeNan();
   
   rspfDpt chainGsd;
   std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx;

   // Loop through dem layers.
   chainIdx = m_demLayer.begin();
   while ( chainIdx != m_demLayer.end() )
   {
      getMetersPerPixel( (*chainIdx).get(), chainGsd);
      if ( gsd.hasNans() || ( chainGsd.x < gsd.x ) ) 
      {
         gsd = chainGsd;
      }
      ++chainIdx;
   }

   // Loop through image layers.
   chainIdx = m_imgLayer.begin();
   while ( chainIdx != m_imgLayer.end() )
   {
      getMetersPerPixel( (*chainIdx).get(), chainGsd);
      if ( gsd.hasNans() || ( chainGsd.x < gsd.x ) )
      {
         gsd = chainGsd;
      }
      ++chainIdx;
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "gsd: " << gsd << "\n" << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getMetersPerPixel(rspfSingleImageChain* chain, rspfDpt& gsd)
{
   static const char MODULE[] = "rspfChipperUtil::getMetersPerPixel(rspfSingleImageChain*,rspfDpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   if (chain)
   {
      rspfRefPtr<rspfImageGeometry> geom = chain->getImageGeometry();
      if ( geom.valid() )
      {
         geom->getMetersPerPixel( gsd );
         if ( gsd.hasNans() )
         {
            std::string errMsg = MODULE;
            errMsg += "\ngeom->getMetersPerPixel returned nan for chain.";
            errMsg += "\nChain: ";
            errMsg += chain->getFilename().string();
            throw rspfException(errMsg);
         }
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += "\nNo geometry for chain: ";
         errMsg += chain->getFilename().string();
         throw rspfException(errMsg);
      }
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null chain passed to method!";
      throw rspfException(errMsg);
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "chain name: " << chain->getFilename()
         << "\nmeters per pixel: " << gsd << "\n" << MODULE << " exited...\n";
   }
}

rspf_float64 rspfChipperUtil::getCentralMeridian() const
{
   rspf_float64 result = rspf::nan();
   const char* lookup = m_kwl->find(rspfKeywordNames::CENTRAL_MERIDIAN_KW);
   if ( lookup )
   {
      result = rspfString::toFloat64(lookup);
      if ( (result < -180.0) || (result > 180.0) )
      {
         std::string errMsg = "central meridian range error!";
         errMsg += " Valid range: -180 to 180";
         throw rspfException(errMsg);
      }
   }
   return result;
}

rspf_float64 rspfChipperUtil::getOriginLatitude() const
{
   rspf_float64 result = rspf::nan();
   const char* lookup = m_kwl->find(rspfKeywordNames::ORIGIN_LATITUDE_KW);
   if ( lookup )
   {
      result = rspfString::toFloat64(lookup);
      if ( (result < -90) || (result > 90.0) )
      {
         std::string errMsg = "origin latitude range error!";
         errMsg += " Valid range: -90 to 90";
         throw rspfException(errMsg);
      }
   }
   return result;
}

void rspfChipperUtil::getSceneCenter(rspfGpt& gpt)
{
   static const char MODULE[] = "rspfChipperUtil::getSceneCenter(rspfGpt&)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   std::vector<rspfGpt> centerGptArray;
   rspfGpt centerGpt;
   
   std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx;

   // Loop through dem layers.
   chainIdx = m_demLayer.begin();
   while ( chainIdx != m_demLayer.end() )
   {
      getSceneCenter( (*chainIdx).get(), centerGpt);
      if ( !centerGpt.hasNans() )
      {
         centerGptArray.push_back( centerGpt );
      }
      ++chainIdx;
   }

   // Loop through image layers.
   chainIdx = m_imgLayer.begin();
   while ( chainIdx != m_imgLayer.end() )
   {
      getSceneCenter( (*chainIdx).get(), centerGpt);
      if ( !centerGpt.hasNans() )
      {
         centerGptArray.push_back( centerGpt );
      }
      ++chainIdx;
   }

   rspf_float64 lat = 0.0;
   rspf_float64 lon = 0.0;
   
   std::vector<rspfGpt>::const_iterator pointIdx = centerGptArray.begin();
   while ( pointIdx != centerGptArray.end() )
   {
      lat += (*pointIdx).lat;
      lon += (*pointIdx).lon;
      ++pointIdx;
   }

   lat /= centerGptArray.size();
   lon /= centerGptArray.size();

   if ( (lat >= -90.0) && (lat <= 90.0) && (lon >= -180.0) && (lon <= 180.0) )
   {
      gpt.lat = lat;
      gpt.lon = lon;
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " range error!";
      errMsg += "\nlatitude = ";
      errMsg += rspfString::toString(lat).string();
      errMsg += "\nlongitude = ";
      errMsg += rspfString::toString(lon).string();
      throw rspfException(errMsg);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "scene center: " << gpt << "\n" << MODULE << " exited...\n";
   }
}

void rspfChipperUtil::getSceneCenter(rspfSingleImageChain* chain, rspfGpt& gpt)
{
   static const char MODULE[] =
      "rspfChipperUtil::getSceneCenter(const rspfSingleImageChain*,rspfGpt&)";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }   
   
   if (chain)
   {
      rspfRefPtr<rspfImageGeometry> geom = chain->getImageGeometry();
      if ( geom.valid() )
      {
         rspfIrect boundingRect = chain->getBoundingRect();
         rspfDpt midPoint = boundingRect.midPoint();
         geom->localToWorld(midPoint, gpt);
         gpt.hgt = 0.0;
         
         if ( gpt.hasNans() )
         {
            std::string errMsg = MODULE;
            errMsg += "\ngeom->localToWorld returned nan for chain.";
            errMsg += "\nChain: ";
            errMsg += chain->getFilename().string();
            throw rspfException(errMsg);
         }
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += "\nNo geometry for chain: ";
         errMsg += chain->getFilename().string();
         throw rspfException(errMsg);
      }
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null chain passed to method!";
      throw rspfException(errMsg);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "chain name: " << chain->getFilename()
         << "\nscene center: " << gpt << "\n"
         << MODULE << " exited...\n";
   }
}

rspfRefPtr<rspfMapProjection> rspfChipperUtil::getFirstInputProjection()
{
   static const char MODULE[] = "rspfChipperUtil::getFirstInputProjection";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfImageHandler>  ih     = 0;
   rspfRefPtr<rspfMapProjection> result = 0;

   // Get the first image handler.
   if ( m_demLayer.size() )
   {
      ih = m_demLayer[0]->getImageHandler();
   }
   else if ( m_imgLayer.size() )
   {
      ih = m_imgLayer[0]->getImageHandler();
   }
   
   if ( ih.valid() )
   {
      // Get the geometry from the first image handler.      
      rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
      if ( geom.valid() )
      {
         // Get the image projection.
         rspfRefPtr<rspfProjection> proj = geom->getProjection();
         if ( proj.valid() )
         {
            // Cast and assign to result.
            rspfMapProjection* mapProj = PTR_CAST( rspfMapProjection, proj.get() );
            if (mapProj)
            {
               // Must duplicate in case the output projection gets modified.
               result = (rspfMapProjection*) mapProj->dup();
            }
            if ( !result.valid() && traceDebug() )
            {
               rspfNotify(rspfNotifyLevel_WARN) << "Could not cast to map projection.\n";
            }
         }
         else if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_WARN) << "No projection in first chain...\n";
         }
      }
   }
   else if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_WARN) << "No image handler in first chain...\n";
   }
   
   if ( traceDebug() )
   {
      if ( result.valid() )
      {
         result->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
   
   return result;
}

rspfRefPtr<rspfMapProjection> rspfChipperUtil::getNewGeoProjection()
{
   return rspfRefPtr<rspfMapProjection>(new rspfEquDistCylProjection());
}

rspfRefPtr<rspfMapProjection> rspfChipperUtil::getNewGeoScaledProjection()
{
   // Make projection:
   rspfRefPtr<rspfMapProjection> result = getNewGeoProjection();

   // Set the origin for scaling:
   
   // First check for user set "central_meridian" and "origin_latitude":
   rspfGpt origin;
   origin.lat = getOriginLatitude();
   origin.lon = getCentralMeridian();
   origin.hgt = 0.0;
   
   if ( origin.hasNans() )
   {
      // Use the scene center from the input.
      getSceneCenter( origin );

      //---
      // Note only latitude used for scaling, origin kept at 0.0.
      // This is a fix/hack for rspfEquDistCylProjection wrapping issues.
      //---
      origin.lon = 0.0;
   }

   if ( !origin.hasNans() )
   {
      result->setOrigin(origin);
   }
   else
   {
      std::string errMsg = "rspfChipperUtil::getNewGeoScaledProjection ERROR";
      errMsg += "\nOrigin has nans!";
      throw rspfException(errMsg);
   }
   
   return result;
}

rspfRefPtr<rspfMapProjection> rspfChipperUtil::getNewProjectionFromSrsCode(
   const rspfString& code)
{
   rspfRefPtr<rspfMapProjection> result = 0;

   if (code == "4326")  // Avoid factory call for this.
   {
      result = new rspfEquDistCylProjection();
   }
   else
   {
      rspfRefPtr<rspfProjection> proj = rspfProjectionFactoryRegistry::instance()->
         createProjection(code);
      if ( proj.valid() )
      {
         result = PTR_CAST( rspfMapProjection, proj.get() );
      }
   }
   return result;
}

rspfRefPtr<rspfMapProjection> rspfChipperUtil::getNewUtmProjection()
{
   // Make projection:
   rspfRefPtr<rspfUtmProjection> utm = new rspfUtmProjection;

   // First check for user set "central_meridian" and "origin_latitude":
   rspfGpt origin;
   origin.lat = getOriginLatitude();
   origin.lon = getCentralMeridian();
   origin.hgt = 0.0;
   
   if ( origin.hasNans() )
   {
      // Use the scene center from the input.
      getSceneCenter( origin );
   }

   if ( !origin.hasNans() )
   {
      utm->setZone(origin);
      utm->setHemisphere(origin);
   }
   else
   {
      std::string errMsg = "rspfChipperUtil::getNewUtmProjection ERROR";
      errMsg += "\nOrigin has nans!";
      throw rspfException(errMsg);
   }

   return rspfRefPtr<rspfMapProjection>(utm.get());
}

rspfRefPtr<rspfMapProjection> rspfChipperUtil::getMapProjection()
{
   rspfRefPtr<rspfMapProjection> mp = 0;
   if ( m_geom.valid() )
   {
      mp = dynamic_cast<rspfMapProjection*>( m_geom->getProjection() );
   }
   return mp;
}

rspfRefPtr<rspfImageFileWriter> rspfChipperUtil::createNewWriter() const
{
   static const char MODULE[] = "rspfChipperUtil::createNewWriter()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfFilename outputFile;
   getOutputFilename(outputFile);

   if ( outputFile == rspfFilename::NIL)
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR no output file name!";
      throw rspfException(errMsg);
   }

   rspfRefPtr<rspfImageFileWriter> writer = 0;
   
   const char* lookup = m_kwl->find( WRITER_KW );
   if ( lookup ) // User defined.
   {
      writer = rspfImageWriterFactoryRegistry::instance()->createWriter(rspfString(lookup));

      if ( !writer.valid() )
      {
         std::string errMsg = MODULE;
         errMsg += " ERROR creating writer: ";
         errMsg += lookup;
         throw rspfException(errMsg);
      }
   }
   else // Create from output file extension.
   {
      writer = rspfImageWriterFactoryRegistry::instance()->
         createWriterFromExtension( outputFile.ext() );

      if ( !writer.valid() )
      {
         std::string errMsg = MODULE;
         errMsg += " ERROR creating writer from extension: ";
         errMsg += outputFile.ext().string();
         throw rspfException(errMsg);
      }
   }

   // Set the output name.
   writer->setFilename( outputFile );

   // Add any writer props.
   rspf_uint32 count = m_kwl->numberOf(WRITER_PROPERTY_KW);
   for (rspf_uint32 i = 0; i < count; ++i)
   {
      rspfString key = WRITER_PROPERTY_KW;
      key += rspfString::toString(i);
      lookup = m_kwl->find( key.c_str() );
      if ( lookup )
      {
         rspfString s = lookup;
         std::vector<rspfString> splitArray;
         s.split(splitArray, "=");
         if(splitArray.size() == 2)
         {
            rspfRefPtr<rspfProperty> prop =
               new rspfStringProperty(splitArray[0], splitArray[1]);

            if ( traceDebug() )
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Setting writer prop: " << splitArray[0] << "=" << splitArray[1] << "\n";
            }
            
            writer->setProperty( prop );
         }
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "writer name: " << writer->getClassName() << "\n"
         << MODULE << " exited...\n";
   }

   return writer;
}

void rspfChipperUtil::propagateOutputProjectionToChains()
{
   static const char MODULE[] = "rspfChipperUtil::propagateOutputProjectionToChains()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx;

   // Loop through dem layers.
   chainIdx = m_demLayer.begin();
   while ( chainIdx != m_demLayer.end() )
   {
      rspfRefPtr<rspfImageRenderer> resampler = (*chainIdx)->getImageRenderer();
      if ( resampler.valid() )
      {
         resampler->setView( m_geom.get() );
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += " chain has no resampler!";
         throw( rspfException(errMsg) );
      }
      ++chainIdx;
   }

   // Loop through image layers.
   chainIdx = m_imgLayer.begin();
   while ( chainIdx != m_imgLayer.end() )
   {
      rspfRefPtr<rspfImageRenderer> resampler = (*chainIdx)->getImageRenderer();
      if ( resampler.valid() )
      {
         resampler->setView( m_geom.get() );
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += " chain has no resampler!";
         throw( rspfException(errMsg) );
      }
      ++chainIdx;
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
}

rspfRefPtr<rspfImageSource> rspfChipperUtil::combineLayers(
   std::vector< rspfRefPtr<rspfSingleImageChain> >& layers) const
{
   static const char MODULE[] = "rspfChipperUtil::combineLayers(layers)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfImageSource> result = 0;

   if ( layers.size() == 1 )
   {
      result = layers[0].get();
   }
   else if ( layers.size() > 1 )
   {
      result = new rspfImageMosaic;

      std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx = layers.begin();
      while ( chainIdx != layers.end() )
      {
         result->connectMyInputTo( (*chainIdx).get() );
         ++chainIdx;
      }
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }

   return result;
}

rspfRefPtr<rspfImageSource> rspfChipperUtil::combineLayers()
{
   static const char MODULE[] = "rspfChipperUtil::combineLayers()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfImageSource> result = 0;

   rspf_uint32 layerCount = (rspf_uint32) (m_demLayer.size() + m_imgLayer.size());

   if ( layerCount )
   {
      if ( layerCount == 1 )
      {
         if ( m_imgLayer.size() )
         {
            result = m_imgLayer[0].get();
         }
         else
         {
            result = m_demLayer[0].get();
         }
      }
      else
      {
         result = new rspfImageMosaic;
         
         // Combine the images.  Note we'll put the images on top of the dems.
         if ( m_imgLayer.size() )
         {
            std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx =
               m_imgLayer.begin();
            while ( chainIdx !=  m_imgLayer.end() )
            {
               result->connectMyInputTo( (*chainIdx).get() );
               ++chainIdx;
            }
         }
         if ( m_demLayer.size() ) // Combine any dem layers.
         {
            std::vector< rspfRefPtr<rspfSingleImageChain> >::iterator chainIdx =
               m_demLayer.begin();
            while ( chainIdx != m_demLayer.end() )
            {
               result->connectMyInputTo( (*chainIdx).get() );
               ++chainIdx;
            }
         }
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
   
   return result;
}

rspfRefPtr<rspfImageSource> rspfChipperUtil::combine2CmvLayers()
{
   static const char MODULE[] = "rspfChipperUtil::combine2CmvLayers()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfImageSource> result = 0;

   rspf_uint32 layerCount = (rspf_uint32) (m_demLayer.size() + m_imgLayer.size());

   // Must have two and only two inputs.
   if ( layerCount == 2 )
   {     
      rspfRefPtr<rspfConnectableObject> oldImg = 0;
      rspfRefPtr<rspfConnectableObject> newImg = 0;

      //---
      // Expecting two image layers.  We'll code it for flexabilty though...
      // - Take old and new from m_imgLayer if present.
      // - Use m_demLayer only if missing.
      // - Using first image found as old, second new.
      //---

      // Most likely case, two image layers.
      if ( m_imgLayer.size() )
      {
         oldImg = m_imgLayer[0].get();
         
         if ( m_imgLayer.size() == 2 )
         {
            newImg = m_imgLayer[1].get();
         }
      }

      if ( m_demLayer.size() )
      {
         if ( !oldImg.valid() )
         {
            oldImg = m_demLayer[0].get();
         }

         if ( !newImg.valid() )
         {
            if ( m_demLayer.size() == 1 )
            {
               newImg = m_demLayer[0].get();
            }
            else if ( m_demLayer.size() == 2 )
            {
               newImg = m_demLayer[1].get();
            }
         }
      }

      if ( newImg.valid() && oldImg.valid() )
      {
         // Input 0 is old, 1 is new.
         rspfRefPtr<rspfTwoColorView> tcmv = new rspfTwoColorView;
         tcmv->connectMyInputTo( 0, oldImg.get() );
         tcmv->connectMyInputTo( 1, newImg.get() );

         // Look for 2cmv options.
         rspf_uint32 oldInputBandIndex = 0;
         rspf_uint32 newInputBandIndex = 0;
         rspfTwoColorView::rspfTwoColorMultiViewOutputSource redOutputSource = rspfTwoColorView::OLD;
         rspfTwoColorView::rspfTwoColorMultiViewOutputSource grnOutputSource = rspfTwoColorView::NEW;
         rspfTwoColorView::rspfTwoColorMultiViewOutputSource bluOutputSource = rspfTwoColorView::NEW;

         rspfString os;
         std::string key = TWOCMV_OLD_INPUT_BAND_KW;
         std::string val = m_kwl->findKey( key );
         
         if ( val.size() )
         {
            os = val;
            oldInputBandIndex = os.toUInt32();
         }
         
         key = TWOCMV_NEW_INPUT_BAND_KW;
         val = m_kwl->findKey( key );
         if ( val.size() )
         {
            os = val;
            newInputBandIndex = os.toUInt32();
         }

         key = TWOCMV_RED_OUTPUT_SOURCE_KW;
         val = m_kwl->findKey( key );
         if ( val.size() )
         {
            os = val;
            os.downcase();
            
            if ( os == "new" )
            {
               redOutputSource = rspfTwoColorView::NEW;
            }
            else if ( os == "MIN" )
            {
               redOutputSource = rspfTwoColorView::MIN;
            }
         }

         key = TWOCMV_GREEN_OUTPUT_SOURCE_KW;
         val = m_kwl->findKey( key );
         if ( val.size() )
         {
            os = val;
            os.downcase();
            
            if ( os == "old" )
            {
               grnOutputSource = rspfTwoColorView::OLD;
            }
            else if ( os == "MIN" )
            {
               grnOutputSource = rspfTwoColorView::MIN;
            }
         }

         key = TWOCMV_BLUE_OUTPUT_SOURCE_KW;
         val = m_kwl->findKey( key );
         if ( val.size() )
         {
            os = val;
            os.downcase();
            
            if ( os == "old" )
            {
               bluOutputSource = rspfTwoColorView::OLD;
            }
            else if ( os == "MIN" )
            {
               bluOutputSource = rspfTwoColorView::MIN;
            }
         }

         // Set options.
         tcmv->setBandIndexMapping( oldInputBandIndex,
                                    newInputBandIndex,
                                    redOutputSource,
                                    grnOutputSource,
                                    bluOutputSource );
         tcmv->initialize();

         result = tcmv.get();
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
   
   return result;
}

rspfRefPtr<rspfImageSource> rspfChipperUtil::addIndexToRgbLutFilter(
   rspfRefPtr<rspfImageSource> &source) const
{
   static const char MODULE[] = "rspfChipperUtil::addIndexToRgbLutFilter(source)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfImageSource> result = 0;

   if ( source.valid() )
   {
      rspfRefPtr<rspfIndexToRgbLutFilter> lut = new rspfIndexToRgbLutFilter();
      rspfFilename lutFile = m_kwl->find( LUT_FILE_KW );
      if ( lutFile.exists() )
      {
         lut->setLut(lutFile);
         
         // Connect to dems:
         lut->connectMyInputTo( source.get() );

         // Note sure about this.  Make option maybe? (drb)
         lut->setInterpolationType(rspfIndexToRgbLutFilter::rspfIndexToRgbLutFilter_LINEAR);
         
         // Set as color source for bump shade.
         result = lut.get();
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += " color table does not exists: ";
         errMsg += lutFile.string();
         throw rspfException(errMsg);
      }
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null source passed to method!";
      throw rspfException(errMsg);
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }

   return result;
}

rspfRefPtr<rspfImageSource> rspfChipperUtil::addScalarRemapper(
   rspfRefPtr<rspfImageSource> &source, rspfScalarType scalar) const
{
   static const char MODULE[] = "rspfChipperUtil::addScalarRemapper(source)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   rspfRefPtr<rspfImageSource> result = 0;
   
   if ( source.valid() )
   {
      if ( ( scalar != RSPF_SCALAR_UNKNOWN ) && ( source->getOutputScalarType() != scalar ) )
      {
         rspfRefPtr<rspfScalarRemapper> remapper = new rspfScalarRemapper();
         remapper->setOutputScalarType(scalar);
         remapper->connectMyInputTo( source.get() );
         result = remapper.get();
         
         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "\nOutput remapped to: "
               << rspfScalarTypeLut::instance()->getEntryString(scalar) << "\n";
         }
      }
      else
      {
         result = source;
      }
   }
   else
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null source passed to method!";
      throw rspfException(errMsg);
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " exited...\n";
   }
   
   return result;
}

bool rspfChipperUtil::setupChainHistogram( rspfRefPtr<rspfSingleImageChain>& chain) const
{
   static const char MODULE[] = "rspfChipperUtil::setupChainHistogram(chain)";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   } 
   
   bool result = false;

   if ( chain.valid() )
   {
      rspfRefPtr<rspfHistogramRemapper> remapper = chain->getHistogramRemapper();

      if ( remapper.valid() )
      {
         if ( remapper->getHistogramFile() == rspfFilename::NIL )
         {
            rspfRefPtr<rspfImageHandler> ih = chain->getImageHandler();
            if ( ih.valid() )
            {
               rspfFilename f = ih->getFilenameWithThisExtension( rspfString("his") );

               if ( f.empty() || (f.exists() == false) )
               {
                  // For backward compatibility check if single entry and _e0.his
                  f = ih->getFilenameWithThisExtension( rspfString("his"), true );
               }

               if ( remapper->openHistogram( f ) == false )
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                     << MODULE << " WARNING:"
                     << "\nCould not open:  " << f << "\n";
               }
            }
         }

         if ( remapper->getHistogramFile() != rspfFilename::NIL )
         {
            const char* lookup = m_kwl->find(HISTO_OP_KW);
            if (lookup)
            {
               result = true;
               
               // Enable.
               remapper->setEnableFlag(true);
               
               // Set the histo mode:
               rspfString op = lookup;
               op.downcase();
               if ( op == "auto-minmax" )
               {
                  remapper->setStretchMode( rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX );
               }
               else if ( (op == "std-stretch-1") || (op == "std-stretch 1") )
               {
                  remapper->setStretchMode( rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN );
               } 
               else if ( (op == "std-stretch-2") || (op == "std-stretch 2") )
               {
                  remapper->setStretchMode( rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN );
               } 
               else if ( (op == "std-stretch-3") || (op == "std-stretch 3") )
               {
                  remapper->setStretchMode( rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN );
               }
               else
               {
                  result = false;
                  remapper->setEnableFlag(false);
                  rspfNotify(rspfNotifyLevel_WARN)
                     << MODULE << "\nUnhandled operation: " << op << "\n";
               }
            }
         }
      }
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
   
   return result;
}

bool rspfChipperUtil::setChainEntry(
   rspfRefPtr<rspfSingleImageChain>& chain, rspf_uint32 entryIndex ) const
{
   bool result = false;
   if ( chain.valid() )
   {
      rspfRefPtr<rspfImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         result = ih->setCurrentEntry( entryIndex );
      }
   }
   return result;
}

void rspfChipperUtil::getOutputFilename(rspfFilename& f) const
{
   static const char MODULE[] = "rspfChipperUtil::getOutputFilename()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   } 

   const char* lookup =  m_kwl->find( rspfKeywordNames::OUTPUT_FILE_KW );
   if ( lookup )
   {
      f = lookup;
   }
   else
   {
      f = rspfFilename::NIL;
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
}

void rspfChipperUtil::getAreaOfInterest(rspfImageSource* source, rspfIrect& rect) const
{
   static const char MODULE[] = "rspfChipperUtil::getAreaOfInterest()";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   // Nan rect for starters.
   rect.makeNan();
   
   if ( source )
   {
      if ( m_geom.valid() )
      {
         if ( m_kwl->find( CUT_CENTER_LAT_KW ) ) 
         {
            // "Cut Center" with: --cut-center-llwh or --cut-center-llr:

            const char* latStr = m_kwl->find( CUT_CENTER_LAT_KW );
            const char* lonStr = m_kwl->find( CUT_CENTER_LON_KW );
            if ( latStr && lonStr )
            {
               rspfGpt centerGpt;

               //---
               // Want the height nan going into worldToLocal call so it gets picked
               // up by the elevation manager.
               //---
               centerGpt.makeNan(); 

               centerGpt.lat = rspfString(latStr).toFloat64();
               centerGpt.lon = rspfString(lonStr).toFloat64();

               if ( !centerGpt.isLatNan() && !centerGpt.isLonNan() )
               {
                  // Ground "cut center" to view:
                  rspfDpt centerDpt(0.0, 0.0);
                  m_geom->worldToLocal(centerGpt, centerDpt);

                  if ( !centerDpt.hasNans() )
                  {
                     if ( isIdentity() && m_ivt.valid() ) // Chipping in image space.
                     {
                        // Tranform image center point to view:
                        rspfDpt ipt = centerDpt;
                        m_ivt->imageToView( ipt, centerDpt );
                     }
                     
                     // --cut-center-llwh:
                     const char* widthStr  = m_kwl->find( CUT_CENTER_WIDTH_KW );
                     const char* heightStr = m_kwl->find( CUT_CENTER_HEIGHT_KW );
                     if ( widthStr && heightStr )
                     {
                        rspf_int32 width  = rspfString(widthStr).toInt32();
                        rspf_int32 height = rspfString(heightStr).toInt32();
                        if ( width && height )
                        {
                           rspfIpt ul( rspf::round<int>(centerDpt.x - (width/2)),
                                        rspf::round<int>(centerDpt.y - (height/2)) );
                           rspfIpt lr( (ul.x + width - 1), ul.y + height - 1);
                           rect = rspfIrect(ul, lr);
                        }
                     }
                     else // --cut-center-llr: 
                     {
                        const char* radiusStr = m_kwl->find( CUT_CENTER_RADIUS_KW );
                        if ( radiusStr )
                        {
                           rspf_float64 radius = rspfString(radiusStr).toFloat64();
                           if ( radius )
                           {
                              rspfDpt mpp;
                              m_geom->getMetersPerPixel( mpp );

                              if ( !mpp.hasNans() )
                              {
                                 rspf_float64 rx = radius/mpp.x;
                                 rspf_float64 ry = radius/mpp.y;
                                 
                                 rspfIpt ul( rspf::round<int>( centerDpt.x - rx ),
                                              rspf::round<int>( centerDpt.y - ry ) );
                                 rspfIpt lr( rspf::round<int>( centerDpt.x + rx ),
                                              rspf::round<int>( centerDpt.y + ry ) );
                                 rect = rspfIrect(ul, lr);
                              }
                           }
                        }
                     }
                  }
                  
               } // Matches: if ( !centerGpt.hasNans() )
               
            } // Matches: if ( latStr && lonStr )
            
         } // Matches: if ( m_kwl->find( CUT_CENTER_LAT_KW ) )
         
         else if ( m_kwl->find( CUT_MAX_LAT_KW ) ) // --cut-bbox-ll
         {
            const char* maxLat = m_kwl->find( CUT_MAX_LAT_KW );
            const char* maxLon = m_kwl->find( CUT_MAX_LON_KW );
            const char* minLat = m_kwl->find( CUT_MIN_LAT_KW );
            const char* minLon = m_kwl->find( CUT_MIN_LON_KW );
         
            if ( maxLat && maxLon && minLat && minLon )
            {
               rspf_float64 minLatF = rspfString(minLat).toFloat64();
               rspf_float64 maxLatF = rspfString(maxLat).toFloat64();

               //---
               // Check for swap so we don't get a negative height.
               // Note no swap check for longitude as box could cross date line.
               //---
               if ( minLatF > maxLatF )
               {
                  rspf_float64 tmpF = minLatF;
                  minLatF = maxLatF;
                  maxLatF = tmpF;
               }
            
               rspfGpt gpt(0.0, 0.0, 0.0);
               rspfDpt ulPt;
               rspfDpt lrPt;
            
               // Upper left:
               gpt.lat = maxLatF;
               gpt.lon = rspfString(minLon).toFloat64();
               m_geom->worldToLocal(gpt, ulPt);
            
               // Lower right:
               gpt.lat = minLatF;
               gpt.lon = rspfString(maxLon).toFloat64();
               m_geom->worldToLocal(gpt, lrPt);

               if ( isIdentity() && m_ivt.valid() )
               {
                  // Chipping in image space:
                  
                  // Tranform image ul point to view:
                  rspfDpt ipt = ulPt;
                  m_ivt->imageToView( ipt, ulPt );
                  
                  // Tranform image lr point to view:
                  ipt = lrPt;
                  m_ivt->imageToView( ipt, lrPt );
               }
            
               rect = rspfIrect( rspfIpt(ulPt), rspfIpt(lrPt) );
            }
         }

         // If no user defined rect set to scene bounding rect.
         if ( rect.hasNans() ) 
         {
            rect = source->getBoundingRect(0);
         }
      
      } // if ( m_getOuputGeometry.valid() )
      else
      {
         // Should never happer...
         std::string errMsg = MODULE;
         if ( !source )
         {
            errMsg += " image source null!";
         }
         else
         {
            errMsg += " output projection null!";
         }
         throw( rspfException(errMsg) );
      }

   } // if ( source )
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "aoi: " << rect << "\n"
         << MODULE << " exited...\n";
   }
   
} // End: rspfChipperUtil::getAreaOfInterest

void rspfChipperUtil::initializeThumbnailProjection(const rspfIrect& originalRect,
                                                  rspfIrect& adjustedRect)
{
   static const char MODULE[] = "rspfChipperUtil::initializeThumbnailProjection";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered...\n"
         << "origial rect:  " << originalRect << "\n";

      if (m_geom.valid())
      {
         m_geom->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
   }
   
   if ( !originalRect.hasNans() && m_geom.valid() )
   {
      //---
      // Thumbnail setup:
      //---
      const char* thumbRes = m_kwl->find(THUMBNAIL_RESOLUTION_KW);
      if ( thumbRes )
      {
         rspf_float64 thumbSize = rspfString(thumbRes).toFloat64();
         rspf_float64 maxRectDimension =
            rspf::max( originalRect.width(), originalRect.height() );
         
         if ( maxRectDimension > thumbSize )
         {
            // Need to adjust scale:
            
            // Get the corners:
            rspfGpt ulGpt;
            rspfGpt lrGpt;
            rspfDpt dpt;
            
            m_geom->localToWorld(rspfDpt(originalRect.ul()), ulGpt);
            m_geom->localToWorld(rspfDpt(originalRect.lr()), lrGpt);         
            
            rspf_float64 scale = maxRectDimension / thumbSize;

            //---
            // Adjust the projection scale.  Note the "true" is to recenter
            // the tie point so it falls relative to the projection origin.
            //
            // This call also scales: rspfImageGeometry::m_imageSize
            //---
            m_geom->applyScale(rspfDpt(scale, scale), true);

            // Must call to reset the rspfImageRenderer's bounding rect for each input.
            propagateOutputProjectionToChains();
            
            // Get the new upper left in view space.
            m_geom->worldToLocal(ulGpt, dpt);
            rspfIpt ul(dpt);
            
            // Get the new lower right in view space.
            m_geom->worldToLocal(lrGpt, dpt);
            rspfIpt lr(dpt);

            // Clamp to thumbnail bounds.
            rspf_int32 ts = thumbSize;

            if ( (lr.x - ul.x + 1) > ts )
            {
               lr.x = ul.x + ts - 1;
            }
            if ( (lr.y - ul.y + 1) > ts )
            {
               lr.y = ul.y + ts - 1;
            }
            
            adjustedRect = rspfIrect(ul, lr);
         }
      }
      
   } // if ( !originalRect.hasNans() && m_geom.valid() )
   else
   {
      // Should never happer...
      std::string errMsg = MODULE;
      if ( originalRect.hasNans() )
      {
         errMsg += " passed in rect has nans!";
      }
      else
      {
         errMsg += " output projection null!";
      }
      throw( rspfException(errMsg) );
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "\nadjusted rect: " << adjustedRect << "\n";
      if (m_geom.valid())
      {
         m_geom->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
}

bool rspfChipperUtil::hasBandSelection() const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      result = ( m_kwl->find( rspfKeywordNames::BANDS_KW ) != 0 );
   }
   return result;
}

void rspfChipperUtil::getBandList( std::vector<rspf_uint32>& bandList ) const
{
   bandList.clear();
   if ( m_kwl.valid() )
   {
      std::string s = m_kwl->findKey( std::string( rspfKeywordNames::BANDS_KW ) );
      if ( s.size() )
      {
         rspfString os = s;
         std::vector<rspfString> band_list(0);
         os.split( band_list, rspfString(","), false );
         if ( band_list.size() )
         {
            std::vector<rspfString>::const_iterator i = band_list.begin();
            while ( i != band_list.end() )
            {
               rspf_uint32 band = (*i).toUInt32();
               if ( band ) // One based so we need to subtract.
               {
                  bandList.push_back( band - 1 );
               }
               ++i;
            }
         }
      }
   }
   
} // End: rspfChipperUtil::getBandList

bool rspfChipperUtil::hasLutFile() const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      result = ( m_kwl->find( LUT_FILE_KW ) != 0 );
   }
   return result;
}

bool rspfChipperUtil::hasBumpShadeArg() const
{
   bool result = ( m_operation == RSPF_CHIPPER_OP_HILL_SHADE );
   if ( !result && m_kwl.valid() )
   {
      result = ( m_kwl->find( rspfKeywordNames::AZIMUTH_ANGLE_KW ) ||
                 m_kwl->find( COLOR_RED_KW ) ||
                 m_kwl->find( COLOR_GREEN_KW ) ||
                 m_kwl->find( COLOR_BLUE_KW ) ||
                 m_kwl->find( rspfKeywordNames::ELEVATION_ANGLE_KW ) ||
                 m_kwl->find( GAIN_KW ) );
   }
   return result;
}

bool rspfChipperUtil::hasThumbnailResolution() const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      result = ( m_kwl->find(THUMBNAIL_RESOLUTION_KW) != 0 );
   }
   return result;
}

bool rspfChipperUtil::hasHistogramOperation() const
{
   bool result = false;
   
   if ( m_kwl.valid() )
   {
      result = ( m_kwl->find(HISTO_OP_KW) != 0 );
   }
   // No option for this right now.  Only through src file.
   return result;
}

bool rspfChipperUtil::isDemFile(const rspfFilename& file) const
{
   bool result = false;
   rspfString ext = file.ext();
   if ( ext.size() >= 2 )
   {
      ext.downcase();
      if ( ( ext == "hgt" ) ||
           ( ext == "dem" ) ||
          ( ( (*ext.begin()) == 'd' ) && ( (*(ext.begin()+1)) == 't' ) ) )
      {
         result = true;
      }
   }
   return result;
}

bool rspfChipperUtil::isSrcFile(const rspfFilename& file) const
{
   bool result = false;
   rspfString ext = file.ext();
   ext.downcase();
   if ( ext == "src" )
   {
      result = true;
   }
   return result;
}

rspfScalarType rspfChipperUtil::getOutputScalarType() const
{
   rspfScalarType scalar = RSPF_SCALAR_UNKNOWN;
   const char* lookup = m_kwl->find(OUTPUT_RADIOMETRY_KW);
   if ( lookup )
   {
      scalar = rspfScalarTypeLut::instance()->getScalarTypeFromString( rspfString(lookup) );
   }
   if ( scalar == RSPF_SCALAR_UNKNOWN )
   {
      // deprecated keyword...
      lookup = m_kwl->find(SCALE_2_8_BIT_KW);
      if ( lookup )
      {
         if ( rspfString(lookup).toBool() == true )
         {
            scalar = RSPF_UINT8;
         }
      }
   }
   return scalar;
}

bool rspfChipperUtil::scaleToEightBit() const
{
   bool result = false;
   if ( ( m_operation == RSPF_CHIPPER_OP_COLOR_RELIEF ) || // Always 8 bit...
        ( getOutputScalarType() == RSPF_UINT8 ) )
   {
      result = true;
   }
   return result;
}

bool rspfChipperUtil::snapTieToOrigin() const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      const char* lookup = m_kwl->find(SNAP_TIE_TO_ORIGIN_KW);
      if ( lookup )
      {
         result = rspfString(lookup).toBool();
      }
   }
   return result;
}

bool rspfChipperUtil::upIsUp() const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      const char* lookup = m_kwl->find(UP_IS_UP_KW);
      if ( lookup )
      {
         result = rspfString(lookup).toBool();
      }
   }
   return result;
}

bool rspfChipperUtil::northUp() const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      const char* lookup = m_kwl->find(NORTH_UP_KW);
      if ( lookup )
      {
         result = rspfString(lookup).toBool();
      }
   }
   return result;
}

bool rspfChipperUtil::isIdentity() const
{
   return (m_operation == RSPF_CHIPPER_OP_CHIP);
}

rspf_uint32 rspfChipperUtil::getEntryNumber() const
{
   rspf_uint32 result = 0;
   if ( m_kwl.valid() )
   {
      std::string lookup = m_kwl->findKey( std::string( rspfKeywordNames::ENTRY_KW ) );
      if ( lookup.size() )
      {
         result = rspfString(lookup).toUInt32();
      }
   }
   return result;
}

bool rspfChipperUtil::hasSensorModelInput() const
{
   bool result = false;

   // Test image layers.
   std::vector< rspfRefPtr<rspfSingleImageChain> >::const_iterator chainIdx = m_imgLayer.begin();
   while ( chainIdx != m_imgLayer.end() )
   {
      // Get the image handler:
      rspfRefPtr<rspfImageHandler>  ih = (*chainIdx)->getImageHandler();
      if ( ih.valid() )
      {
         // Get the geometry from the first image handler.      
         rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() )
         {
            // Get the image projection.
            rspfRefPtr<rspfProjection> proj = geom->getProjection();
            if ( proj.valid() )
            {
               // Cast and assign to result.
               rspfMapProjection* mapProj = PTR_CAST( rspfMapProjection, proj.get() );
               if ( !mapProj )
               {
                  result = true;
                  break;
               }
            }
         }
      }   
      ++chainIdx;
   }

   if ( !result )
   {
      // Test dem layers.
      chainIdx = m_demLayer.begin();
      while ( chainIdx != m_demLayer.end() )
      {
         // Get the image handler:
         rspfRefPtr<rspfImageHandler>  ih = (*chainIdx)->getImageHandler();
         if ( ih.valid() )
         {
            // Get the geometry from the first image handler.      
            rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
            if ( geom.valid() )
            {
               // Get the image projection.
               rspfRefPtr<rspfProjection> proj = geom->getProjection();
               if ( proj.valid() )
               {
                  // Cast and assign to result.
                  rspfMapProjection* mapProj = PTR_CAST( rspfMapProjection, proj.get() );
                  if ( !mapProj )
                  {
                     result = true;
                     break;
                  }
               }
            }
         }   
         ++chainIdx;
      }
   }
   
   return result;
}

void  rspfChipperUtil::initializeSrcKwl()
{
   static const char MODULE[] = "rspfChipperUtil::initializeSrcKwl";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered...\n";
   }

   const char* lookup = m_kwl->find(SRC_FILE_KW);
   if ( lookup )
   {
      m_srcKwl = new rspfKeywordlist();
      m_srcKwl->setExpandEnvVarsFlag(true);
      if ( m_srcKwl->addFile(lookup) == false )
      {
         m_srcKwl = 0;
      }
   }
   else
   {
      m_srcKwl = 0; 
   }

   if ( traceDebug() )
   {
      if ( m_srcKwl.valid() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "src keyword list:\n" << *(m_srcKwl.get()) << "\n";
      }
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " exited...\n";
   }
}

rspf_uint32 rspfChipperUtil::getNumberOfInputs() const
{
   rspf_uint32 result = 0;
   if ( m_kwl.valid() )
   {
      result += m_kwl->numberOf(DEM_KW);
      result += m_kwl->numberOf(IMG_KW);
   }
   if ( m_srcKwl.valid() )
   {
      result += m_srcKwl->numberOf(DEM_KW);
      result += m_srcKwl->numberOf(IMG_KW);
   }
   return result;
}

rspfChipperUtil::rspfChipperOutputProjection rspfChipperUtil::getOutputProjectionType() const
{
   rspfChipperOutputProjection result = rspfChipperUtil::RSPF_CHIPPER_PROJ_UNKNOWN;
   const char* op  = m_kwl->find(rspfKeywordNames::PROJECTION_KW);
   if ( op )
   {
      rspfString os = op;
      os.downcase();
      if (os == "geo")
      {
         result = rspfChipperUtil::RSPF_CHIPPER_PROJ_GEO;
      }
      else if (os == "geo-scaled")
      {
         result = rspfChipperUtil::RSPF_CHIPPER_PROJ_GEO_SCALED;
      }
      else if ( os == "input" )
      {
         result = rspfChipperUtil::RSPF_CHIPPER_PROJ_INPUT;
      }
      else if ( (os == "utm") || (os == "rspfutmprojection") )
      {
         result = rspfChipperUtil::RSPF_CHIPPER_PROJ_UTM;
      }
   }
   return result;
}

void rspfChipperUtil::usage(rspfArgumentParser& ap)
{
   // Add global usage options.
   rspfInit::instance()->addOptions(ap);
   
   // Set app name.
   std::string appName = ap.getApplicationName();
   ap.getApplicationUsage()->setApplicationName( rspfString( appName ) );

   // Add options.
   addArguments(ap);
   
   // Write usage.
   ap.getApplicationUsage()->write(rspfNotify(rspfNotifyLevel_INFO));

   // Keeping single line in tact for examples for cut and paste purposes.
   rspfNotify(rspfNotifyLevel_INFO)
      << "\nExample commands:\n"

      << "\n// A two color multi view with cut box.  First image is old, second image is new:\n"
      << appName << " --cut-bbox-ll 28.092885092033352 -80.664539599998633 28.109128691071547 -80.626914963229325 --op 2cmv oldMLB.tif newMLB.tif outputs/2cmv-test1.tif\n"

      << "\n// Ortho about point, 512x512, with histogram stretch, and 3,2,1 band order:\n"
      << appName << " --op ortho -b 3,2,1 --histogram-op auto-minmax --cut-center-llwh -42.819784401784275 147.265811350983 512 512 5V090205M0001912264B220000100072M_001508507.ntf orth.tif\n"

      << "\n// Chip, in image space, about point, 512x512, with histogram stretch, and 3,2,1 band order:\n"
      << appName << " --op chip -b 3,2,1 --histogram-op auto-minmax --cut-center-llwh -42.819784401784275 147.265811350983 512 512 5V090205M0001912264B220000100072M_001508507.ntf chip.tif\n"

      << "\n// Chip in image space=, rotate \"up is up\"(-u option) about point, 512x512 with histogram stretch and 3,2,1 band order:\n"
      << appName << " --op chip -u -b 3,2,1 --histogram-op auto-minmax --cut-center-llwh -42.819784401784275 147.265811350983 512 512 5V090205M0001912264B220000100072M_001508507.ntf up-is-up-chip.tif\n"
      << std::endl;
}
