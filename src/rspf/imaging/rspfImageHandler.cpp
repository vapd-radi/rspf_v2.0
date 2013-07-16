//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// This class defines an abstract image handler which all loaders should
// derive from.
//
//*******************************************************************
//  $Id: rspfImageHandler.cpp 22228 2013-04-12 14:11:45Z dburken $

#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfContainerEvent.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHistogramSource.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfTiffOverviewBuilder.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <algorithm>

RTTI_DEF1(rspfImageHandler, "rspfImageHandler", rspfImageSource)

// Static trace for debugging
static rspfTrace traceDebug("rspfImageHandler:debug");

// Property keywords.
static const char HAS_LUT_KW[]                 = "has_lut";
static const char OPEN_OVERVIEW_FLAG_KW[]      = "open_overview_flag";
static const char START_RES_LEVEL_KW[]         = "start_res_level";
static const char SUPPLEMENTARY_DIRECTORY_KW[] = "supplementary_directory";
static const char VALID_VERTICES_FILE_KW[]     = "valid_vertices_file";

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfImageHandler.cpp 22228 2013-04-12 14:11:45Z dburken $";
#endif

// GARRETT! All of the decimation factors are scattered throughout. We want to fold that into 
// the image geometry object. The code is ready in rspfImageGeometry for this (more or less), but
// the rspfImageGeometry::m_decimationFactors is not being set nor referenced. Can you do this?
// I'm a little foggy on how we want to incorporate R-level scaling into the geometry object.
   
rspfImageHandler::rspfImageHandler()
:
rspfImageSource(0, 0, 0, true, false /* output list is not fixed */ ),
theImageFile(rspfFilename::NIL),
theOverviewFile(rspfFilename::NIL),
theSupplementaryDirectory(rspfFilename::NIL),
theOverview(0),
theValidImageVertices(0),
theMetaData(),
theGeometry(),
theLut(0),
theDecimationFactors(0),
theImageID(""),
theStartingResLevel(0),
theOpenOverviewFlag(true),
thePixelType(RSPF_PIXEL_IS_POINT)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageHandler::rspfImageHandler() DEBUG:" << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  "
         << RSPF_ID
         << std::endl;
#endif      
   }
}

rspfImageHandler::~rspfImageHandler()
{
   theOverview = 0;
}

void rspfImageHandler::initialize()
{
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfImageHandler::saveState(rspfKeywordlist& kwl,
                                  const char* prefix) const
{
   static const char MODULE[] = "rspfImageHandler::saveState";

   rspfImageSource::saveState(kwl, prefix);
   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE
         << " ERROR detected in keyword list!  State not saved."
         << std::endl;
      return false;
   }

   kwl.add(prefix, rspfKeywordNames::FILENAME_KW, theImageFile.c_str(), true);
   kwl.add(prefix, HAS_LUT_KW, (theLut.valid()?"true":"false"), true);
   kwl.add(prefix, rspfKeywordNames::IMAGE_ID_KW, theImageID, true);
   kwl.add(prefix, rspfKeywordNames::OVERVIEW_FILE_KW, theOverviewFile.c_str(), true);
   kwl.add(prefix, SUPPLEMENTARY_DIRECTORY_KW, theSupplementaryDirectory.c_str(), true);
   kwl.add(prefix, START_RES_LEVEL_KW, theStartingResLevel, true);
   kwl.add(prefix, OPEN_OVERVIEW_FLAG_KW, (theOpenOverviewFlag?"1":"0"), true);
   kwl.add(prefix, rspfKeywordNames::PIXEL_TYPE_KW, (rspf_uint16) thePixelType, true);

   return true;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfImageHandler::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   static const char MODULE[] = "rspfImageHandler::loadState(kwl, prefix)";
   theDecimationFactors.clear();
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: entered ..."
         << std::endl;
   }
   
   rspfImageSource::loadState(kwl, prefix);
   
   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE
         << "WARNING: error detected in keyword list!  State not load."
         << std::endl;
      
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG: "
            << "leaving with error in kwl ..." << std::endl;
      }
      return false;
   }
   
   const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if (!lookup)
   {
      // Deprecated...
      lookup = kwl.find(prefix, rspfKeywordNames::IMAGE_FILE_KW);
   }
   
   if (lookup)
   {
      theImageFile = lookup;
   }
   
   lookup = kwl.find(prefix, rspfKeywordNames::OVERVIEW_FILE_KW);
   if(lookup)
   {
      theOverviewFile = rspfFilename(lookup);
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:\n"
         << "filename = " << theImageFile << std::endl;
   }
   
   // Check for an external geometry file to initialize our image geometry object:
   lookup = kwl.find(prefix, rspfKeywordNames::GEOM_FILE_KW);
   if (lookup)
   {
      rspfKeywordlist geomKwl;
      if (geomKwl.addFile(lookup) == true)
      {
         theGeometry = new rspfImageGeometry;
         theGeometry->loadState(geomKwl);
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << MODULE << " DEBUG:"
               << "Loaded external geometry file:  "
               << lookup
               << "\n" << geomKwl << std::endl;
         }
      }
   }
   
   // Check for an valid image vetices file.
   lookup = kwl.find(prefix, VALID_VERTICES_FILE_KW);
   if (lookup)
   {
      initVertices(lookup);
   }

   // Starting resolution level.
   lookup = kwl.find(prefix, START_RES_LEVEL_KW);
   if (lookup)
   {
      theStartingResLevel = rspfString(lookup).toUInt32();
   }

   // Open overview flag.
   lookup = kwl.find(prefix, OPEN_OVERVIEW_FLAG_KW);
   if (lookup)
   {
      setOpenOverviewFlag( rspfString(lookup).toBool() );
   }

   // The supplementary directory for finding the overview
   lookup = kwl.find(prefix, SUPPLEMENTARY_DIRECTORY_KW);
   if (lookup)
   {
      theSupplementaryDirectory = rspfFilename(lookup);
   }

   if(getNumberOfInputs())
   {
      theInputObjectList.clear();
   }
   theInputListIsFixedFlag = true;
   
   // Read image id if present:
   theImageID = kwl.find(prefix, rspfKeywordNames::IMAGE_ID_KW);

   lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_TYPE_KW);
   if (lookup)
   {
      thePixelType = (rspfPixelType) atoi(lookup);
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: Leaving Ok ..."
         << std::endl;
   }
   return true;
}

bool rspfImageHandler::initVertices(const char* file)
{
   static const char MODULE[] = "rspfImageHandler::initVertices";

   rspfFilename f = file;
   if (!f.exists()) return false;

   rspfKeywordlist kwl(file);
   
   if (kwl.getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfImageHandler::initVertices NOTICE:"
            << "\nError reading valid vertice file:  " << file << std::endl;
      }
      return false;
   }

   // Clean out any old vertices...
   theValidImageVertices.clear();

   rspf_uint32 number_of_points = kwl.numberOf("point", "x");

   for (rspf_uint32 i=0; i<number_of_points; ++i)
   {
      rspfIpt pt;
      const char* lookup;
      rspfString p = "point";
      p += rspfString::toString(i);
      
      rspfString px = p + ".x";
      lookup = kwl.find(px.c_str());
      if (lookup)
      {
         pt.x = atoi(lookup);
      }
      else
      {
         if (traceDebug())
         {
            CLOG << " ERROR:"
                 << "\nlookup failed for:  " << px.c_str()
                 << "\nReturning..."
                 << std::endl;
         }
         return false;
      }
         
      rspfString py = p + ".y";
      lookup = kwl.find(py.c_str());
      if (lookup)
      {
         pt.y = atoi(lookup);
      }
      else
      {
         if (traceDebug())
         {
            CLOG << " ERROR:"
                 << "\nLookup failed for:  " << py.c_str()
                 << "\nReturning..."
                 << std::endl;
         }
         return false;
      }

      theValidImageVertices.push_back(pt);
   }

   
   if (traceDebug())
   {
      CLOG << " DEBUG:"
           << "\nVertices file:  " << f
           << "\nValid image vertices:"
           << std::endl;
      for (rspf_uint32 i=0; i<theValidImageVertices.size(); ++i)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "\npoint[" << i << "]:  " << theValidImageVertices[i];
      }
      rspfNotify(rspfNotifyLevel_DEBUG) << std::endl;
   }
   
   return true;
}

rspf_uint32 rspfImageHandler::getNumberOfDecimationLevels() const
{
   rspf_uint32 result = 1; // Add r0
   if (theOverview.valid())
   {
      result += theOverview->getNumberOfDecimationLevels();
   }
   return result;
}

rspf_uint32 rspfImageHandler::getNumberOfReducedResSets() const
{
   return getNumberOfDecimationLevels();
}

rspfIrect rspfImageHandler::getImageRectangle(rspf_uint32 resLevel) const
{
   rspfIrect result;
   
   if( isOpen() && isValidRLevel(resLevel) )
   {
      rspf_int32 lines   = getNumberOfLines(resLevel);
      rspf_int32 samples = getNumberOfSamples(resLevel);
      if( !lines || !samples )
      {
         result.makeNan();
      }
      else
      {
         result = rspfIrect(0, 0, samples-1, lines-1);
      }
   }
   else
   {
      result.makeNan();
   }

   return result;
}

rspfIrect rspfImageHandler::getBoundingRect(rspf_uint32 resLevel) const
{
   return getImageRectangle(resLevel);
}


void rspfImageHandler::getDecimationFactor(rspf_uint32 resLevel, rspfDpt& result) const
{
   if (resLevel < theDecimationFactors.size())
      result = theDecimationFactors[resLevel];
   else
      result.makeNan();
}

void rspfImageHandler::getDecimationFactors(vector<rspfDpt>& decimations) const
{
   decimations = theDecimationFactors;
}

//*************************************************************************************************
// Method determines the decimation factors at each resolution level. This
// base class implementation computes the decimation by considering the ratios in image size
// between resolution levels, with fuzzy logic for rounding ratios to the nearest power of 2
// if possible. Derived classes need to override this method if the decimations are provided
// as part of the image metadata. In some cases (cf. rspfNitfTileSource), the derived class can
// set the first R-level (R0) decimation in the case that it is not unity, and then invoke this
// base class implementation to compute remaining R-levels, respecting the R0 decimation previously
// set by derived class.
//*************************************************************************************************
void rspfImageHandler::establishDecimationFactors() 
{
   double line_decimation, samp_decimation, decimation, ratio;
   rspf_uint32 num_lines, num_samps;
   
   // The error margin here is effectively the percent error tolerated between ideal number of 
   // pixels for given power-of-2 decimation, and the actual number of pixels found at this 
   // decimation level. Typically, the last level will have fewer pixels than expected, hence the 
   // need for this logic...
   static const double ERROR_MARGIN = 1.1;  // 10% allowance

   // NOTE -- Until the end of this method, all decimation values are actually inverse quantities,
   // i.e., a decimation of 0.5 typical for rlevel 1 is represented here as 2. This facilitates the
   // fuzzy logic for recognizing powers of 2 (as integers)

   // Default implementation assumes R0 is not decimated. Check for R0 defined by derived class
   // however, in case this is not the case:
   rspfDpt decimation_r0 (1.0, 1.0);
   if (theDecimationFactors.size() > 0)
   {
      decimation_r0.x = 1.0/theDecimationFactors[0].x; // note use of inverse decimation
      decimation_r0.y = 1.0/theDecimationFactors[0].y; 
   }
   else
      theDecimationFactors.push_back(decimation_r0);

   // Variables used in loop below:
   rspf_uint32 num_lines_r0 = getNumberOfLines(0);
   rspf_uint32 num_samps_r0 = getNumberOfSamples(0);
   rspf_uint32 power_of_2_decimation = (rspf_uint32) decimation_r0.x;

   // Remaining res levels are computed as a ratio of image size at R0 to image size at Rn:
   rspf_uint32 nRlevels = getNumberOfDecimationLevels();
   for(rspf_uint32 res_level = 1; res_level < nRlevels; ++res_level)
   {
      num_lines = getNumberOfLines(res_level);
      num_samps = getNumberOfSamples(res_level);
      
      if ((num_lines < 2) || (num_samps < 2)) 
         break;

      line_decimation = decimation_r0.y * num_lines_r0 / (double)num_lines;
      samp_decimation = decimation_r0.x * num_samps_r0 / (double)num_samps;
      decimation = line_decimation<samp_decimation ? line_decimation:samp_decimation;

      // Check for possible error due to inexact ratios.
      // Loop until reasonable effort was made to establish the corresponding power-of-2 decimation.
      // If close match is found, the exact integer value is assigned:
      do 
      {
         power_of_2_decimation *= 2;
         ratio = (double)power_of_2_decimation / decimation;
         if (ratio < 1.0)
            ratio = 1.0/ratio;
         if (ratio < ERROR_MARGIN)
            decimation = (double) power_of_2_decimation;

      } while ((double) power_of_2_decimation < decimation);

      // Convert the computed decimation back to fractional form before saving in the factors list:
      decimation = 1.0/decimation;
      theDecimationFactors.push_back(rspfDpt(decimation, decimation));
   }
}


bool rspfImageHandler::buildHistogram(int numberOfRLevels)
{
   if(isOpen())
   {
      rspfFilename file =
         getFilenameWithThisExtension(rspfString(".his"));
      
      rspfRefPtr<rspfImageHistogramSource> histoSource = new rspfImageHistogramSource;
      rspfRefPtr<rspfHistogramWriter> writer = new rspfHistogramWriter;
      
      if(numberOfRLevels)
      {
         histoSource->setMaxNumberOfRLevels(numberOfRLevels);
      }
      else
      {
         histoSource->setMaxNumberOfRLevels(getNumberOfDecimationLevels());
      }
      histoSource->connectMyInputTo(0, this);
      histoSource->enableSource();
      writer->connectMyInputTo(0, histoSource.get());
      writer->setFilename(file);
      writer->addListener(&theStdOutProgress);
      writer->execute();
      histoSource->disconnect();
      writer->disconnect();
      histoSource = 0;
      writer = 0;
   }
   else
   {
      return false;
   }
   
   return true;
}

bool rspfImageHandler::buildAllHistograms(int numberOfRLevels)
{
   rspf_uint32 currentEntry = getCurrentEntry();
   std::vector<rspf_uint32> entryList;
   getEntryList(entryList);
   rspf_uint32 idx = 0;
   for(idx = 0; idx < entryList.size(); ++idx)
   {
      setCurrentEntry(entryList[idx]);
      if(!buildHistogram(numberOfRLevels))
      {
         setCurrentEntry(currentEntry);
         return false;
      }
   }
   
   setCurrentEntry(currentEntry);
   return true;
}

bool rspfImageHandler::buildOverview(rspfImageHandlerOverviewCompressionType compressionType,
                                      rspf_uint32 quality,
                                      rspfFilterResampler::rspfFilterResamplerType resampleType,
                                      bool includeFullResFlag)
{
   rspfFilename file = getFilenameWithThisExtension(rspfString(".ovr"));

   theOverviewFile = file;
   if(buildOverview(file, compressionType, quality, resampleType, includeFullResFlag))
   {
      openOverview();
   }

   return true;
}

bool rspfImageHandler::buildAllOverviews(rspfImageHandlerOverviewCompressionType compressionType,
                                          rspf_uint32 quality,
                                          rspfFilterResampler::rspfFilterResamplerType resampleType,
                                          bool includeFullResFlag)
{
   rspf_uint32 currentEntry = getCurrentEntry();
   std::vector<rspf_uint32> entryList;
   getEntryList(entryList);
   rspf_uint32 idx = 0;
   for(idx = 0; idx < entryList.size(); ++idx)
   {
      setCurrentEntry(entryList[idx]);
      if(!buildOverview(compressionType, quality, resampleType, includeFullResFlag))
      {
         setCurrentEntry(currentEntry);
         return false;
      }
   }
   
   setCurrentEntry(currentEntry);
   return true;
}


bool rspfImageHandler::buildOverview(const rspfFilename& filename,
                                      rspfImageHandlerOverviewCompressionType compressionType,
                                      rspf_uint32 quality,
                                      rspfFilterResampler::rspfFilterResamplerType resampleType,
                                      bool includeFullResFlag)
{
   closeOverview();
   if(!isOpen())
   {
      return false;
   }

   theOverviewFile = filename;
   rspfTiffOverviewBuilder tiffBuilder;
   if ( tiffBuilder.setInputSource(this) == false )
   {
      return false;
   }
   
   rspf_uint16 cType = COMPRESSION_NONE;
   switch(compressionType)
   {
      case RSPF_OVERVIEW_COMPRESSION_NONE:
      {
         cType = COMPRESSION_NONE;
         break;
      }
      case RSPF_OVERVIEW_COMPRESSION_JPEG:
      {
         cType = COMPRESSION_JPEG;
         break;
      }
      case RSPF_OVERVIEW_COMPRESSION_LZW:
      {
         cType = COMPRESSION_LZW;
         break;
      }
      case RSPF_OVERVIEW_COMPRESSION_DEFLATE:
      {

         cType = COMPRESSION_DEFLATE;
         break;
      }
      case RSPF_OVERVIEW_COMPRESSION_PACKBITS:
      {
         cType = COMPRESSION_PACKBITS;
         break;
      }
   }
   tiffBuilder.setJpegCompressionQuality(quality);
   tiffBuilder.setCompressionType(cType);
   tiffBuilder.setResampleType(resampleType);
   tiffBuilder.buildOverview(filename, includeFullResFlag);

   return true;
}

//*****************************************************************************
//! Returns the image geometry object associated with this tile source or
//! NULL if non defined.
//! The geometry contains full-to-local image transform as well as projection
//! (image-to-world).
//*****************************************************************************
rspfRefPtr<rspfImageGeometry> rspfImageHandler::getImageGeometry()
{
   if ( !theGeometry )
   {
      //---
      // Check factory for external geom:
      //---
      theGeometry = getExternalImageGeometry();

      if ( !theGeometry )
      {
         //---
         // WARNING:
         // Must create/set the geometry at this point or the next call to
         // rspfImageGeometryRegistry::extendGeometry will put us in an infinite loop
         // as it does a recursive call back to rspfImageHandler::getImageGeometry().
         //---
         theGeometry = new rspfImageGeometry();

         //---
         // And finally allow factories to extend the internal geometry.
         // This allows plugins for tagged formats with tags not know in the base
         // to extend the internal geometry.
         //
         // Plugins can do handler->getImageGeometry() then modify/extend.
         //---
         if(!rspfImageGeometryRegistry::instance()->extendGeometry( this ))
         {
            //---
            // Check for internal, for geotiff, nitf and so on as last resort for getting some
            // kind of geometry loaded
            //---
            theGeometry = getInternalImageGeometry();
         }
      }

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   return theGeometry;
}

rspfRefPtr<rspfImageGeometry> rspfImageHandler::getExternalImageGeometry() const
{
   rspfRefPtr<rspfImageGeometry> geom = 0;

   // No geometry object has been set up yet. Check for external geometry file.
   // Try "foo.geom" if image is "foo.tif":
   rspfFilename filename = getFilenameWithThisExtension(rspfString(".geom"), false);
   if(!filename.exists())
   {
      // Try "foo_e0.tif" if image is "foo.tif" where "e0" is entry index.
      filename = getFilenameWithThisExtension(rspfString(".geom"), true);
   }
   if(!filename.exists())
   {
      // Try supplementary data directory for remote geometry:
      filename = getFilenameWithThisExtension(rspfString(".geom"), false);
      filename = theSupplementaryDirectory.dirCat(filename.file());
   }
   if(!filename.exists())
   {
      // Try supplementary data directory for remote geometry with entry index:
      filename = getFilenameWithThisExtension(rspfString(".geom"), true);
      filename = theSupplementaryDirectory.dirCat(filename.file());
   }

   if(filename.exists())
   {
      // Open the geom file as a KWL and initialize our geometry object:
      filename = filename.expand();
      rspfKeywordlist geomKwl(filename);

      // Try loadState with no prefix.
      std::string prefix = "";
      std::string key = rspfKeywordNames::TYPE_KW;
      std::string value = geomKwl.findKey(prefix, key);

      if ( value.empty() || (value != "rspfImageGeometry") )
      {
         // Try with "image0." type prefix.
         prefix += std::string("image") + rspfString::toString(getCurrentEntry()).string() +
            std::string(".");
         value = geomKwl.findKey(prefix, key);
         
         if ( value.empty() || (value != "rspfImageGeometry") )
         {
            // Try with "image0.geometry." prefix.
            prefix += std::string( "geometry." );
            value = geomKwl.findKey(prefix, key);
            
            if ( value.empty() || (value != "rspfImageGeometry") )
            {
               // Try with "geometry." prefix.
               prefix = std::string( "geometry." );
               value = geomKwl.findKey(prefix, key);
            }
         }
      }
      
      if ( value == "rspfImageGeometry" )
      {
         geom = new rspfImageGeometry;
         if( !geom->loadState(geomKwl, prefix.c_str()) )
         {
            geom = 0;
         }
      }
   }
   
   return geom;
}

rspfRefPtr<rspfImageGeometry> rspfImageHandler::getInternalImageGeometry() const
{
   // Default, derived classes should override.
   rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();
   return geom;
}

void rspfImageHandler::setImageGeometry( rspfImageGeometry* geom)
{
   theGeometry = geom;
}

void rspfImageHandler::saveImageGeometry() const
{
   rspfFilename geometryFile = getFilenameWithThisExtension(rspfString(".geom"));
   saveImageGeometry(geometryFile);
}

void rspfImageHandler::saveImageGeometry(const rspfFilename& geometry_file) const
{
   if (theGeometry.valid())
   {
      rspfKeywordlist kwl;
      theGeometry->saveState(kwl);
      kwl.write(geometry_file.chars());
   }
}

void rspfImageHandler::closeOverview()
{
   theOverview = 0;
}

const rspfImageHandler* rspfImageHandler::getOverview() const
{
   return theOverview.get();
}

bool rspfImageHandler::hasOverviews() const
{
   return (getNumberOfDecimationLevels() > 1);
}

bool rspfImageHandler::openOverview(const rspfFilename& overview_file)
{
   bool result = false;
   
   closeOverview();

   if (overview_file != theImageFile) // Make sure we don't open ourselves.
   {
      //---
      // Get the number of level before the call to opening the overview so
      // the overview can be told what it's starting res level is.
      //---
      rspf_uint32 overviewStartingResLevel = getNumberOfDecimationLevels();


      // Try to open:
      theOverview = rspfImageHandlerRegistry::instance()->openOverview( overview_file );

      if (theOverview.valid())
      {
         result = true;
         
         //---
         // Set the owner in case the overview reader needs to get something
         // from the it like min/max/null.
         //---
         theOverview->changeOwner(this);
         
         // Set the starting res level of the overview.
         theOverview->setStartingResLevel(overviewStartingResLevel);
         
         // Capture the file name.
         theOverviewFile = overview_file;

         //---
         // Some overview handlers cannot store what the null is.  Like dted
         // null is -32767 not default -32768 so this allows passing this to the
         // overview reader provided it overrides setMin/Max/NullPixel value
         // methods. (drb)
         //---
         const rspf_uint32 BANDS = getNumberOfOutputBands();
         for (rspf_uint32 band = 0; band < BANDS; ++band)
         {
            theOverview->setMinPixelValue(band, getMinPixelValue(band));
            theOverview->setMaxPixelValue(band, getMaxPixelValue(band));
            theOverview->setNullPixelValue(band, getNullPixelValue(band));
         }

         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "overview starting res level: " << overviewStartingResLevel
               << "\noverview levels: "
               << theOverview->getNumberOfDecimationLevels()
               << "\nlevels: " << getNumberOfDecimationLevels()
               << endl;
         }
         
         //---
         // This is not really a container event; however, using for now.
         //---
         rspfContainerEvent event(this,
                                   RSPF_EVENT_ADD_OBJECT_ID);
         event.setObjectList(theOverview.get());
         fireEvent(event);
      }
   }
   
   return result;
}

bool rspfImageHandler::openOverview()
{
   closeOverview();
   
   // 1) ESH 03/2009 -- Use the overview file set e.g. using a .spec file.
   rspfFilename overviewFilename = getOverviewFile();
   
   if (overviewFilename.empty() || (overviewFilename.exists() == false) )
   {
      // 2) Generate the name from image name.
      overviewFilename = createDefaultOverviewFilename();
      
      if (overviewFilename.empty() || (overviewFilename.exists() == false) )
      {  
         // 3) For backward compatibility check if single entry and _e0.ovr
         overviewFilename = getFilenameWithThisExtension(rspfString(".ovr"), true);
         if (overviewFilename.empty() || (overviewFilename.exists() == false) )
         {
            // 4) For overviews built with gdal look for foo.tif.ovr
            overviewFilename = getFilename();
            overviewFilename += ".ovr";
         }
      }
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "Looking for " << overviewFilename
         << " overview file..." << std::endl;
   }

   bool status = false;
   
   if ( overviewFilename.exists() )
   {
      status = openOverview( overviewFilename );
   }

   if ( !status  && traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageHandler::openOverview NOTICE:"
         << "\nCould not find an overview." << std::endl;
   }

   return status;
}


bool rspfImageHandler::writeValidImageVertices(const std::vector<rspfIpt>& vertices, const rspfFilename& file)
{
   rspfFilename tempFile = file;
   if(tempFile == "")
   {
      tempFile = createDefaultValidVerticesFilename();
   }
   rspf_uint32 i = 0;
   rspfKeywordlist tempKwl;
   rspfPolygon poly(vertices);
   
   poly.checkOrdering();

   // Capture the vertices.
   theValidImageVertices = vertices;
   
   if(poly.getOrdering() == RSPF_COUNTERCLOCKWISE_ORDER)
   {
      std::reverse(theValidImageVertices.begin(), theValidImageVertices.end());
   }
   
   if(theValidImageVertices.size())
   {
      for(i = 0; i < theValidImageVertices.size(); ++i)
      {
         rspfString prefix = rspfString("point") + rspfString::toString(i) + ".";
         tempKwl.add(prefix.c_str(),
                     "x",
                     theValidImageVertices[i].x,
                     true);
	 tempKwl.add(prefix.c_str(),
                     "y",
                     theValidImageVertices[i].y,
                     true);
      }
      
      tempKwl.write(tempFile.c_str());
   }

   if ( tempFile.exists() )
   {
      return true;
   }

   return false; // Write of file failed.
}

bool rspfImageHandler::openValidVertices(const rspfFilename& vertices_file)
{
   return initVertices(vertices_file);
}

bool rspfImageHandler::openValidVertices()
{
   rspfFilename vertices_file;
   vertices_file = theImageFile.path();
   vertices_file = vertices_file.dirCat(theImageFile.fileNoExtension());
   if(getNumberOfEntries() > 1)
   {
      vertices_file += rspfString("_vertices") + "_e" + rspfString::toString(getCurrentEntry()) + ".kwl";
   }
   else
   {
      vertices_file += "_vertices.kwl";
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "Looking for " << vertices_file
         << " vertices file..." << std::endl;
   }

   return openValidVertices(vertices_file);
}

bool rspfImageHandler::open(const rspfFilename& imageFile)
{
   if(isOpen())
   {
      close();
   }

   setFilename(imageFile);
   
   return open();
}

bool rspfImageHandler::open(const rspfFilename& imageFile,
                             rspf_uint32 entryIndex)
{
   if(isOpen())
   {
      close();
   }
   setFilename(imageFile);
   
   bool result = open();

   if(result)
   {
      return setCurrentEntry(entryIndex);
   }

   return result;
}

bool rspfImageHandler::isValidRLevel(rspf_uint32 resLevel) const
{
   bool result = false;
   
   const rspf_uint32 LEVELS = getNumberOfDecimationLevels();
   
   if ( !theStartingResLevel) // Not an overview.
   {
      result = (resLevel < LEVELS);
   }
   else  // Used as overview.
   {
      if (resLevel >= theStartingResLevel)
      {
         //---
         // Adjust the res level to be zero based for this overview before
         // checking.
         //---
         result = ( (resLevel - theStartingResLevel) < LEVELS);
      }
   }
   
   return result;
}

void rspfImageHandler::getValidImageVertices(vector<rspfIpt>& validVertices,
                                              rspfVertexOrdering ordering,
                                              rspf_uint32 resLevel) const
{
   rspfDpt decimation;
   getDecimationFactor(resLevel, decimation);
   if(decimation.hasNans())
   {
      return;
   }
   if (theValidImageVertices.size() != 0)
   {
      validVertices.clear();
      for (rspf_uint32 i=0; i<theValidImageVertices.size(); ++i)
      {
         rspfIpt pt(rspf::round<int>(theValidImageVertices[i].x*decimation.x),
                     rspf::round<int>(theValidImageVertices[i].y*decimation.y));
         validVertices.push_back(pt);
      }

      if(ordering == RSPF_COUNTERCLOCKWISE_ORDER)
      {
         std::reverse(validVertices.begin()+1, validVertices.end());
      }
   }
   else
   {
      // Call the base class method which will set to bounding rectangle.
      rspfImageSource::getValidImageVertices(validVertices, ordering, resLevel);
   }
}

void rspfImageHandler::close()
{
   theOverview = 0;
   theGeometry = 0;
   theValidImageVertices.clear();
   theDecimationFactors.clear();
}

bool rspfImageHandler::isBandSelector() const
{
   return false;
}

bool rspfImageHandler::setOutputBandList(const std::vector<rspf_uint32>& /* band_list */)
{
   return false;
}

bool rspfImageHandler::setOutputToInputBandList()
{
   bool result = false;
   if ( isBandSelector() )
   {
      std::vector<rspf_uint32> bandList;
      rspfImageSource::getOutputBandList( bandList );
      result = setOutputBandList( bandList );
   }
   return result;
}

bool rspfImageHandler::isIdentityBandList( const std::vector<rspf_uint32>& bandList ) const
{
   bool result = false;
   const rspf_uint32 BANDS = bandList.size();
   if ( BANDS )
   {
      std::vector<rspf_uint32> inputList;
      rspfImageSource::getOutputBandList( inputList ); // This populates an identity band list.

      if ( inputList.size() == BANDS )
      {
         rspf_uint32 band = 0;
         while ( band < BANDS )
         {
            if ( inputList[band] != bandList[band] )
            {
               break;
            }
            ++band;
         }
         if ( band == BANDS )
         {
            result = true;
         }
      }
   }
   return result;
}

// Protected convenience method:
bool rspfImageHandler::setOutputBandList(const std::vector<rspf_uint32>& inBandList,
                                          std::vector<rspf_uint32>& outBandList)
{
   bool result = false;

   const rspf_uint32 INPUT_BANDS  = getNumberOfInputBands();
   const rspf_uint32 OUTPUT_BANDS = inBandList.size();

   if ( INPUT_BANDS && OUTPUT_BANDS )
   {
      result = true;
      outBandList.resize( OUTPUT_BANDS );
      for ( rspf_uint32 band = 0; band < OUTPUT_BANDS; ++band )
      {
         if ( inBandList[band] < INPUT_BANDS )
         {
            outBandList[band] = inBandList[band];
         }
         else // Out of range...
         {
            result = false;
            break;
         }
      }
      if ( result && theOverview.valid() )
      {
         result = theOverview->setOutputBandList( inBandList );
      }
   }

   if ( result == false )
   {
      rspfImageSource::getOutputBandList( outBandList ); // Set to identity.
   }

   return result;
}

bool rspfImageHandler::isImageTiled() const
{
   return (getImageTileWidth() && getImageTileHeight());
}

void rspfImageHandler::loadMetaData()
{
  theMetaData.clear();

  rspfFilename filename = getFilenameWithThisExtension(rspfString(".omd"), false);
  if ( filename.exists() == false )
  {
     filename = getFilenameWithThisExtension(rspfString(".omd"), true);
  }
  if(filename.exists())
  {
     rspfKeywordlist kwl;
     
     kwl.addFile(filename.c_str());
     
     theMetaData.loadState(kwl);
  }
  else
  {
     theMetaData.setScalarType(getOutputScalarType());
  }
}

double rspfImageHandler::getMinPixelValue(rspf_uint32 band)const
{
   if(theMetaData.getNumberOfBands())
   {
      return theMetaData.getMinPix(band);
   }
   return rspf::defaultMin(getOutputScalarType());
}

double rspfImageHandler::getMaxPixelValue(rspf_uint32 band)const
{
   if(theMetaData.getNumberOfBands())
   {
      return theMetaData.getMaxPix(band);
   }
   return rspf::defaultMax(getOutputScalarType());
}

double rspfImageHandler::getNullPixelValue(rspf_uint32 band)const
{
   if(theMetaData.getNumberOfBands())
   {
      return theMetaData.getNullPix(band);
   }
   
   return rspf::defaultNull(getOutputScalarType());
}

void rspfImageHandler::setMinPixelValue(rspf_uint32 band,
                                         const rspf_float64& pix)
{
   theMetaData.setMinPix(band, pix);
}

void rspfImageHandler::setMaxPixelValue(rspf_uint32 band,
                                         const rspf_float64& pix)
{
   theMetaData.setMaxPix(band, pix); 
}

void rspfImageHandler::setNullPixelValue(rspf_uint32 band,
                                          const rspf_float64& pix)
{
   theMetaData.setNullPix(band, pix);  
}

rspf_uint32 rspfImageHandler::getCurrentEntry()const
{
   return 0;
}

bool rspfImageHandler::setCurrentEntry(rspf_uint32 /* entryIdx */)
{
   return true;
}

rspfFilename rspfImageHandler::getOverviewFile()const
{
   return theOverviewFile;
}

void rspfImageHandler::getEntryList(std::vector<rspf_uint32>& entryList)const
{
   entryList.push_back(0);
}

void rspfImageHandler::getEntryStringList(
   std::vector<rspfString>& entryStringList) const
{
   std::vector<rspf_uint32> entryList;
   getEntryList(entryList);

   if (entryList.size())
   {
      entryStringList.resize(entryList.size());
      for (rspf_uint32 i = 0; i < entryList.size(); ++i)
      {
         entryStringList[i] = rspfString::toString(entryList[i]);
      }
   }
}

bool rspfImageHandler::useEntryIndex() const
{
   bool result = false;
   if ( isOpen() )
   {
      std::vector<rspf_uint32> entryList;
      getEntryList(entryList);

      if ( entryList.size() )
      {
         if ( (entryList.size() > 1) || (entryList[0] != 0) ) result = true;
      }
   }
   return result;
}

rspf_uint32 rspfImageHandler::getNumberOfEntries()const
{
   std::vector<rspf_uint32> tempList;
   getEntryList(tempList);
   
   return (rspf_uint32)tempList.size();
}

void rspfImageHandler::getEntryName( rspf_uint32 /* entryIdx */, std::string& name ) const
{
   //---
   // This implementation does nothing. Interface for multi entry image readers that have
   // named entries, e.g. HDF5 data.
   //---
   name.clear();
}

void rspfImageHandler::completeOpen()
{
   loadMetaData();
   if ( theOpenOverviewFlag )
   {
      openOverview();
   }
   establishDecimationFactors();
   openValidVertices();
}

bool rspfImageHandler::canConnectMyInputTo(rspf_int32 /* inputIndex */,
                                            const rspfConnectableObject* /* object */)const
{      
   return false;
}

bool rspfImageHandler::hasMetaData()const
{
   return theMetaData.isValid();
}

void rspfImageHandler::setFilename(const rspfFilename& filename)
{
   theImageFile = filename;
}

const rspfFilename& rspfImageHandler::getFilename()const
{
   return theImageFile;
}

void rspfImageHandler::setSupplementaryDirectory(const rspfFilename& dir)
{
   if (dir.isDir())
      theSupplementaryDirectory = dir;
   else
      theSupplementaryDirectory = dir.path();

   // A change in supplementary directory presents an opportunity to find the OVR that could not be
   // opened previously, as well as other support data items:
   if (!theOverview.valid())
   {
      if (theDecimationFactors.size() > 0)
      {
        theDecimationFactors.clear();
      }
      completeOpen();
   }
}

const rspfFilename& rspfImageHandler::getSupplementaryDirectory()const
{
   return theSupplementaryDirectory;
}

void rspfImageHandler::setProperty(rspfRefPtr<rspfProperty> property)
{
   if ( property.valid() )
   {
      if(property->getName() == rspfKeywordNames::ENTRY_KW)
      {
         setCurrentEntry(property->valueToString().toUInt32());
      }
      else if ( property->getName() == OPEN_OVERVIEW_FLAG_KW )
      {
         setOpenOverviewFlag( property->valueToString().toBool() );
      }
      else
      {
         rspfImageSource::setProperty(property);
      }
   }
}

rspfRefPtr<rspfProperty> rspfImageHandler::getProperty(const rspfString& name)const
{

   if(name == rspfKeywordNames::ENTRY_KW)
   {
      std::vector<rspf_uint32> entryList;
      getEntryList(entryList);
      
      rspfStringProperty* stringProperty = new rspfStringProperty(name,
                                                                    rspfString::toString(getCurrentEntry()));
      rspf_uint32 idx = 0;
      for(idx = 0; idx < entryList.size(); ++idx)
      {
         stringProperty->addConstraint(rspfString::toString(entryList[idx]));
      }
      stringProperty->setFullRefreshBit();
      stringProperty->setEditableFlag(false);

      return stringProperty;
   }
   else if(name == rspfKeywordNames::NUMBER_SAMPLES_KW)
   {
      rspfNumericProperty* prop = new rspfNumericProperty(name, rspfString::toString(getNumberOfSamples()));
      prop->setReadOnlyFlag(true);
      
      return prop;
   }
   else if(name == rspfKeywordNames::NUMBER_LINES_KW)
   {
      rspfNumericProperty* prop = new rspfNumericProperty(name, rspfString::toString(getNumberOfLines()));
      prop->setReadOnlyFlag(true);
      
      return prop;
   }
   else if(name == rspfKeywordNames::NUMBER_BANDS_KW)
   {
      rspfNumericProperty* prop = new rspfNumericProperty(name, rspfString::toString(getNumberOfInputBands()));
      prop->setReadOnlyFlag(true);
      
      return prop;
   }
   else if(name == rspfKeywordNames::NUMBER_REDUCED_RES_SETS_KW)
   {
      rspfNumericProperty* prop = new rspfNumericProperty(name, rspfString::toString(getNumberOfDecimationLevels()));
      prop->setReadOnlyFlag(true);
      
      return prop;
   }
   else if(name == rspfKeywordNames::SCALAR_TYPE_KW)
   {
      rspfStringProperty* prop = new rspfStringProperty(name, rspfScalarTypeLut::instance()->getEntryString(getOutputScalarType()));
      prop->setReadOnlyFlag(true);
      
      return prop;
   }
   else if(name == "histogram_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, createDefaultHistogramFilename());
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();
      
      return filenameProp;
      
   }
   else if(name == "geometry_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, createDefaultGeometryFilename());
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();
      
      return filenameProp;
      
   }
   else if(name == "valid_vertices_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, createDefaultValidVerticesFilename());
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();
      
      return filenameProp;
   }
   else if(name == "metadata_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, createDefaultMetadataFilename());
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();
      
      return filenameProp;
   }
   else if(name == "overview_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, createDefaultOverviewFilename());
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();
      
      return filenameProp;
   }
   else if(name == "filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, getFilename());
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setFullRefreshBit();
      
      return filenameProp;
   }
   else if ( name == OPEN_OVERVIEW_FLAG_KW)
   {
      rspfRefPtr<rspfProperty> result =
         new rspfBooleanProperty(rspfString(OPEN_OVERVIEW_FLAG_KW), theOpenOverviewFlag); 
      return result;
   }
   
   return rspfImageSource::getProperty(name);
}

void rspfImageHandler::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSource::getPropertyNames(propertyNames);
   propertyNames.push_back(rspfKeywordNames::ENTRY_KW);
   propertyNames.push_back(rspfKeywordNames::NUMBER_SAMPLES_KW);
   propertyNames.push_back(rspfKeywordNames::NUMBER_LINES_KW);
   propertyNames.push_back(rspfKeywordNames::NUMBER_BANDS_KW);
   propertyNames.push_back(rspfKeywordNames::NUMBER_REDUCED_RES_SETS_KW);
   propertyNames.push_back(rspfKeywordNames::SCALAR_TYPE_KW);
   propertyNames.push_back(OPEN_OVERVIEW_FLAG_KW);
}

rspfFilename rspfImageHandler::getFilenameWithThisExtension(
   const rspfString& ext,
   bool set_e0_prefix) const
{
   // Get the image file.
   rspfFilename f = getFilename();

   // If the supplementary directory is set, find the extension
   // at that location instead of at the default.
   if ( theSupplementaryDirectory.size() )
   {
      
      rspfString drivePart;
      rspfString pathPart;
      rspfString filePart;
      rspfString extPart;

      f.split(drivePart,
              pathPart,
              filePart,
              extPart);

      rspfFilename newDrivePart = theSupplementaryDirectory.drive();
      rspfFilename newPathPart  = theSupplementaryDirectory.after(newDrivePart);

      f.merge( newDrivePart, newPathPart, filePart, extPart );
   }

   // Wipe out the extension.
   f.setExtension("");

   if (set_e0_prefix || (getNumberOfEntries() > 1))
   {
      f += "_e";
      f += rspfString::toString(getCurrentEntry());
   }
   if (ext.size())
   {
      if (ext[static_cast<std::string::size_type>(0)] != '.')
      {
         f += ".";
      }
      f += ext;
   }
   return f;
}

bool rspfImageHandler::getOverviewTile(rspf_uint32 resLevel,
                                        rspfImageData* result)
{
   bool status = false;
   
   if (theOverview.valid())
   {
      if (theOverview->isValidRLevel(resLevel) && result &&
          (result->getNumberOfBands() == getNumberOfOutputBands()) )
      {
         status = theOverview->getTile(result, resLevel);
      }
   }
   
   return status;
}

rspfRefPtr<rspfNBandLutDataObject> rspfImageHandler::getLut()const
{
   return theLut;
}

bool rspfImageHandler::hasLut() const
{
   return theLut.valid();
}

rspfFilename rspfImageHandler::createDefaultOverviewFilename() const
{
   return getFilenameWithThisExtension("ovr");
}
   
rspfFilename rspfImageHandler::createDefaultGeometryFilename() const
{
   return getFilenameWithThisExtension("geom");
}

rspfFilename rspfImageHandler::createDefaultMetadataFilename() const
{
   return getFilenameWithThisExtension("omd");
}

rspfFilename rspfImageHandler::createDefaultHistogramFilename() const
{
   return getFilenameWithThisExtension("his");
}

rspfFilename rspfImageHandler::createDefaultValidVerticesFilename() const
{
   rspfFilename result;
   
   result = theImageFile.path();
   result = result.dirCat(theImageFile.fileNoExtension());
   result += "_vertices";
   if(getNumberOfEntries() > 1)
   {
      result += "_e";
      result += rspfString::toString(getCurrentEntry());
   }
   result += ".kwl";
   
   return result;
}

rspf_uint32 rspfImageHandler::getStartingResLevel() const
{
   return theStartingResLevel;
}

void rspfImageHandler::setStartingResLevel(rspf_uint32 level)
{
   theStartingResLevel = level;
}

bool rspfImageHandler::getOpenOverviewFlag() const
{
   return theOpenOverviewFlag;
}
   
void rspfImageHandler::setOpenOverviewFlag(bool flag)
{
   theOpenOverviewFlag = flag;

   // If false close overview if open??? (drb)
}

void rspfImageHandler::initImageParameters(rspfImageGeometry* geom) const
{
   if ( geom )
   {
      // Set decimation levels
      rspf_uint32 num_levels = getNumberOfDecimationLevels();
      if ((num_levels > 0) && (num_levels != geom->getNumberOfDecimations()))
      {
         geom->setDiscreteDecimation(theDecimationFactors);
      }

      // Set image size.
      if(geom->getImageSize().hasNans())
      {
         rspfIrect rect = getBoundingRect();
         geom->setImageSize(rspfIpt(rect.width(), rect.height()));
      } 
   }
}
