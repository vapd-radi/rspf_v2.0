//*******************************************************************
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Histogram remapper class declaration.  Derived from rspfTableRemapper.
//
// Remaps a tile based on mode and histogram clip points.
//
//*************************************************************************
// $Id: rspfHistogramRemapper.cpp 22187 2013-03-07 20:29:00Z dburken $

#include <rspf/imaging/rspfHistogramRemapper.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfHistogramSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <cstdlib>
#include <iomanip>

RTTI_DEF1(rspfHistogramRemapper, "rspfHistogramRemapper", rspfTableRemapper)

static rspfTrace traceDebug("rspfHistogramRemapper:debug");

static const char* STRETCH_MODE[] = { "linear_one_piece",
                                      "linear_1std_from_mean",
                                      "linear_2std_from_mean",
                                      "linear_3std_from_mean",
                                      "linear_auto_min_max",
                                      "stretch_unknown" };

static const char NORMALIZED_LOW_CLIP_POINT_KW[] = "normalized_low_clip_point";
static const char NORMALIZED_HIGH_CLIP_POINT_KW[]="normalized_high_clip_point";
static const char MID_POINT_KW[] = "mid_point";
static const char MIN_OUTPUT_VALUE_KW[] = "min_output_value";
static const char MAX_OUTPUT_VALUE_KW[] = "max_output_value";
static const char STRETCH_MODE_KW[] = "stretch_mode";
static const char HISTOGRAM_FILENAME_KW[] = "histogram_filename";

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfHistogramRemapper.cpp 22187 2013-03-07 20:29:00Z dburken $";
#endif

rspfHistogramRemapper::rspfHistogramRemapper()
   :
   rspfTableRemapper(),  // base class
   theStretchMode(rspfHistogramRemapper::LINEAR_ONE_PIECE),
   theDirtyFlag(false),
   theHistogram(0),
   theNormalizedLowClipPoint(),
   theNormalizedHighClipPoint(),
   theMidPoint(),
   theMinOutputValue(),
   theMaxOutputValue(),
   theBandList(),
   theBypassFlag(true),
   theResetBandIndicesFlag(false)
{
   setNumberOfInputs(2);
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHistogramRemapper::rspfHistogramRemapper entered...\n";
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << "\n";
#endif      
   }
}

rspfHistogramRemapper::~rspfHistogramRemapper()
{
}

void rspfHistogramRemapper::initialize()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHistogramRemapper::initialize entered..." << endl;
   }
	

   theResetBandIndicesFlag=(dynamic_cast<const rspfHistogramSource*>(getInput(1))!=0);
   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   rspfTableRemapper::initialize();
	
   // Check the band list to see if it's changes.
   vector<rspf_uint32> bandList;
   getOutputBandList(bandList);
   rspf_uint32 idx = 0;
   // bool emptyOutput = theBandList.empty();

   if(theResetBandIndicesFlag)
   {
      for(idx = 0; idx < bandList.size();++idx)
      {
         bandList[idx] = idx;
      }
   }
   if (theBandList.size() != 0 && bandList.size())
   {
      if (theBandList.size() != bandList.size())
      {
         // Somethings changed.  Set everthing back.
         initializeClips();
         setNullCount();
         theTable.clear();
         theDirtyFlag = true;
      }
      else
      {
         // Sizes same check the order.
         for(rspf_uint32 band = 0; band < theBandList.size(); ++band)
         {
            if (theBandList[band] != bandList[band])
            {
               //initializeClips();
               //setNullCount();
               theDirtyFlag = true;
               break;
            }            
         }
      }
   }
   else
   {
      initializeMinMaxOutput();
   }

   // Update the band list with the latest.
   theBandList = bandList;
	
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHistogramRemapper::initialize exited..." << endl;
   }
   verifyEnabled();

}

void rspfHistogramRemapper::reset()
{
   // We could delete theTable to free up memory???
   setStretchMode(LINEAR_ONE_PIECE, false);
   initializeClips();
   verifyEnabled();
}

void
rspfHistogramRemapper::setHistogram(rspfRefPtr<rspfMultiResLevelHistogram> histogram)
{
   theHistogram = histogram;
   setNullCount();
	
   // Note: initializeClips before setNullCount since it relies on clips.
   initializeClips();
   theTable.clear();
   theDirtyFlag = true;
}

bool rspfHistogramRemapper::openHistogram(const rspfFilename& histogram_file)
{
   rspfRefPtr<rspfMultiResLevelHistogram> h = new rspfMultiResLevelHistogram();
   if (h->importHistogram(histogram_file))
   {
      setHistogram(h);
      return true;
   }
	
   // Import failed if here!
   return false;
}

rspfRefPtr<rspfImageData> rspfHistogramRemapper::getTile(
   const rspfIrect& tile_rect,
   rspf_uint32 resLevel)
{
   
#if 0 /* Please leave for serious debug. (drb) */
   cout << "\ntheEnableFlag: " << theEnableFlag
        << "\ntheBypassFlag: " << theBypassFlag
        << "\ntheDirtyFlag:  " << theDirtyFlag
        << endl;
#endif
   rspfRefPtr<rspfImageData> result = 0;

   makeClean();
   
   if (!theBypassFlag||!theEnableFlag) // Not bypassed...
   {
      // Base handles the rest...
      result = rspfTableRemapper::getTile(tile_rect, resLevel);
   }
   else if (theInputConnection)
   {
      // Fetch tile from pointer from the input source.
      result = theInputConnection->getTile(tile_rect, resLevel);
   }

   return result;
}

void rspfHistogramRemapper::setLowNormalizedClipPoint(const rspf_float64& clip)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setLowNormalizedClipPoint(clip, band);
   }
}

void rspfHistogramRemapper::makeClean()
{
   if (theEnableFlag && theDirtyFlag) // Enabled and dirty.
   {
      // Always rebuild table if dirty.
      buildTable();
      
      // Check for internal bypass.
      verifyEnabled();

      // Always clear the dirty flag.
      theDirtyFlag = false;
   }
}

void rspfHistogramRemapper::setLowNormalizedClipPoint(const rspf_float64& clip,
                                                  rspf_uint32 zero_based_band)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
	
   if (zero_based_band >= BANDS)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::setLowNormalizedClipPoint ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (BANDS-1) << endl;
   }
	
   if (theNormalizedLowClipPoint.size() != BANDS)
   {
      initializeClips();
   }
	
   if ( clip != theNormalizedLowClipPoint[zero_based_band] &&
        clip < theNormalizedHighClipPoint[zero_based_band] )
   {
      theDirtyFlag = true;
      theNormalizedLowClipPoint[zero_based_band] = clip;
   }
}

void
rspfHistogramRemapper::setHighNormalizedClipPoint(const rspf_float64& clip)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setHighNormalizedClipPoint(clip, band);
   }
}

void rspfHistogramRemapper::setHighNormalizedClipPoint(
   const rspf_float64& clip, rspf_uint32 zero_based_band)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
	
   if (zero_based_band >= BANDS)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::setHighNormalizedClipPoint ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (BANDS-1) << endl;
   }
	
   if (theNormalizedHighClipPoint.size() != BANDS)
   {
      initializeClips();
   }
	
   if ( clip != theNormalizedHighClipPoint[zero_based_band] &&
        clip > theNormalizedLowClipPoint[zero_based_band] )
   {
      theDirtyFlag = true;
      theNormalizedHighClipPoint[zero_based_band] = clip;
   }
}

void rspfHistogramRemapper::setLowClipPoint(const rspf_float64& clip)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setLowClipPoint(clip, band);
   }
}

void rspfHistogramRemapper::setLowClipPoint(const rspf_float64& clip,
                                             rspf_uint32 zero_based_band)
{
   // allow the call to getHistogram to happen this way we can calculate a histogram if 
   // a histosource is connected to this object
   //
   //   if (!theHistogram) return;
	
   rspfRefPtr<rspfHistogram> h = getHistogram(zero_based_band);
   if (h.valid())
   {
      rspf_float64 d = h->getLowFractionFromValue(clip);
      if (rspf::isnan(d) == false)
      {
         setLowNormalizedClipPoint(d, zero_based_band);
      }
   }
}

void rspfHistogramRemapper::setHighClipPoint(const rspf_float64& clip)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setHighClipPoint(clip, band);
   }
}

void rspfHistogramRemapper::setHighClipPoint(const rspf_float64& clip,
                                              rspf_uint32 zero_based_band)
{
   // allow the call to getHistogram to happen this way we can calculate a histogram if 
   // a histosource is connected to this object
   //
   //   if (!theHistogram) return;
	
   rspfRefPtr<rspfHistogram> h = getHistogram(zero_based_band);
   if (h.valid())
   {
      rspf_float64 d = h->getHighFractionFromValue(clip);
      if (rspf::isnan(d) == false)
      {
         d = 1.0-d;
         setHighNormalizedClipPoint(d, zero_based_band);
      }
   }
}

void rspfHistogramRemapper::setMidPoint(const rspf_float64& value)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setMidPoint(value, band);
   }
}

void rspfHistogramRemapper::setMidPoint(const rspf_float64& value,
                                         rspf_uint32 zero_based_band)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
	
   if (zero_based_band >= BANDS)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::setMidPoint ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (BANDS-1) << endl;
   }
	
   if (theMidPoint.size() != BANDS)
   {
      initializeClips();
   }
	
   if (theMidPoint[zero_based_band] != value)
   {
      theDirtyFlag = true;
      theMidPoint[zero_based_band] = value;
   }
}

void rspfHistogramRemapper::setMinOutputValue(const rspf_float64& value)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setMinOutputValue(value, band);
   }
}

void rspfHistogramRemapper::setMinOutputValue(const rspf_float64& value,
                                               rspf_uint32 zero_based_band)
{
   if (theInputConnection)
   {
      const rspf_uint32 BANDS = getNumberOfInputBands();
      
      if (zero_based_band >= BANDS)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramRemapper::setMinOutputValue ERROR:"
            << "\nband " << zero_based_band << " is out of range!"
            << "\nhighest band:  " << (BANDS-1) << endl;
      }
      
      if (theMinOutputValue.size() != BANDS)
      {
         initializeClips();
      }
      
      if ( value != theMinOutputValue[zero_based_band] &&
           value >= theInputConnection->getMinPixelValue(zero_based_band) &&
           value < theMaxOutputValue[zero_based_band] )
      {
         theDirtyFlag = true;
         theMinOutputValue[zero_based_band] = value;
      }
   }
}

void rspfHistogramRemapper::setMaxOutputValue(const rspf_float64& value)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      setMaxOutputValue(value, band);
   }
}

void rspfHistogramRemapper::setMaxOutputValue(const rspf_float64& value,
                                               rspf_uint32 zero_based_band)
{
   if (theInputConnection)
   {
      const rspf_uint32 BANDS = getNumberOfInputBands();
      
      if (zero_based_band >= BANDS)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramRemapper::setMidPoint ERROR:"
            << "\nband " << zero_based_band << " is out of range!"
            << "\nhighest band:  " << (BANDS-1) << endl;
      }
      
      if (theMaxOutputValue.size() != BANDS)
      {
         initializeClips();
      }
      
      if ( value != theMaxOutputValue[zero_based_band] &&
           value <= theInputConnection->getMaxPixelValue(zero_based_band) &&
           value >  theMinOutputValue[zero_based_band] )
      {
         theDirtyFlag = true;
         theMaxOutputValue[zero_based_band] = value;
      }
   }
}

rspf_float64 rspfHistogramRemapper::getLowNormalizedClipPoint(rspf_uint32 zero_based_band) const
{
   if (theNormalizedLowClipPoint.size() == 0 ||
       zero_based_band >= getNumberOfInputBands())
   {
      return rspf::nan();
   }
   
   if (zero_based_band >= theNormalizedLowClipPoint.size())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::getLowNormalizedClipPoint ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (theNormalizedLowClipPoint.size()-1)
         << endl;
      return rspf::nan();
   }
	
   return theNormalizedLowClipPoint[zero_based_band];
}

rspf_float64 rspfHistogramRemapper::getLowNormalizedClipPoint() const
{
   if (theNormalizedLowClipPoint.size() == 0)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = (rspf_uint32)theNormalizedLowClipPoint.size();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getLowNormalizedClipPoint(band);
   }
   
   return (d / BANDS);
}

rspf_float64 rspfHistogramRemapper::getHighNormalizedClipPoint(rspf_uint32 zero_based_band) const
{
   if (theNormalizedHighClipPoint.size() == 0 ||
       zero_based_band >= getNumberOfInputBands())
   {
      return rspf::nan();
   }
   
   if (zero_based_band >= theNormalizedHighClipPoint.size())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::getHighNormalizedClipPoint ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (theNormalizedHighClipPoint.size()-1)
         << endl;
      return rspf::nan();
   }
	
   return theNormalizedHighClipPoint[zero_based_band];
}

rspf_float64 rspfHistogramRemapper::getHighNormalizedClipPoint() const
{
   if (theNormalizedHighClipPoint.size() == 0)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = (rspf_uint32)theNormalizedHighClipPoint.size();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getHighNormalizedClipPoint(band);
   }
   
   return (d / BANDS);
}

rspf_float64 rspfHistogramRemapper::getLowClipPoint(rspf_uint32 zero_based_band) const
{
   if(zero_based_band >= getNumberOfInputBands() ||
      !theHistogram || zero_based_band >= theNormalizedLowClipPoint.size())
   {
      return rspf::nan();
   }
	
   if (theNormalizedLowClipPoint[zero_based_band] == 0.0)
   {
      return getMinPixelValue(zero_based_band);
   }   
	
   rspfRefPtr<rspfHistogram> h = getHistogram(zero_based_band);
   if (h.valid())
   {
      rspf_float64 d =
         h->LowClipVal(theNormalizedLowClipPoint[zero_based_band]);
		
      return floor(d);
   }
	
   return rspf::nan();
}

rspf_float64 rspfHistogramRemapper::getLowClipPoint() const
{
   if (theNormalizedLowClipPoint.size() == 0 || !theHistogram)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getLowClipPoint(band);
   }
   
   return (d / BANDS);
}

rspf_float64 rspfHistogramRemapper::getHighClipPoint(rspf_uint32 zero_based_band) const
{
   if(zero_based_band >= getNumberOfInputBands() ||
      !theHistogram || zero_based_band >= theNormalizedHighClipPoint.size())
   {
      return rspf::nan();
   }
	
   if (theNormalizedHighClipPoint[zero_based_band] == 1.0)
   {
      return getMaxPixelValue(zero_based_band);
   }
	
   rspfRefPtr<rspfHistogram> h = getHistogram(zero_based_band);
   if (h.valid())
   {
      rspf_float64 d =
         h->HighClipVal(1.0-theNormalizedHighClipPoint[zero_based_band]);
      return ceil(d);
   }
   
   return rspf::nan();
}

rspf_float64 rspfHistogramRemapper::getHighClipPoint() const
{
   if (theNormalizedHighClipPoint.size() == 0 || !theHistogram)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = getNumberOfInputBands();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getHighClipPoint(band);
   }
   
   return (d / BANDS);
}

rspf_float64 rspfHistogramRemapper::getMidPoint(rspf_uint32 zero_based_band) const
{
   if (theMidPoint.size() == 0 ||
       zero_based_band >= getNumberOfInputBands())
   {
      return rspf::nan();
   }
   
   if (zero_based_band >= theMidPoint.size())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::getMidPoint ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (theMidPoint.size()-1)
         << endl;
      return rspf::nan();
   }
	
   return theMidPoint[zero_based_band];
}

rspf_float64 rspfHistogramRemapper::getMidPoint() const
{
   if (theMidPoint.size() == 0)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = (rspf_uint32)theMidPoint.size();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getMidPoint(band);
   }
   
   return (d / BANDS);
}

rspf_float64 rspfHistogramRemapper::getMinOutputValue(rspf_uint32 zero_based_band) const
{
   if (theMinOutputValue.size() == 0 ||
       zero_based_band >= getNumberOfInputBands())
   {
      return rspf::nan();
   }
   
   if (zero_based_band >= theMinOutputValue.size())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::getMinOutputValue ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (theMinOutputValue.size()-1)
         << endl;
      return rspf::nan();
   }
	
   return theMinOutputValue[zero_based_band];
}

rspf_float64 rspfHistogramRemapper::getMinOutputValue() const
{
   if (theMinOutputValue.size() == 0)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = (rspf_uint32)theMinOutputValue.size();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getMinOutputValue(band);
   }
   
   return (d / BANDS);
}

rspf_float64 rspfHistogramRemapper::getMaxOutputValue(rspf_uint32 zero_based_band) const
{
   if (theMaxOutputValue.size() == 0 ||
       zero_based_band >= getNumberOfInputBands())
   {
      return rspf::nan();
   }
   
   if (zero_based_band >= theMaxOutputValue.size())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfHistogramRemapper::getMaxOutputValue ERROR:"
         << "\nband " << zero_based_band << " is out of range!"
         << "\nhighest band:  " << (theMaxOutputValue.size()-1)
         << endl;
      return rspf::nan();
   }
	
   return theMaxOutputValue[zero_based_band];
}

rspf_float64 rspfHistogramRemapper::getMaxOutputValue() const
{
   if (theMaxOutputValue.size() == 0)
   {
      return rspf::nan();
   }
   
   rspf_float64 d = 0.0;
   const rspf_uint32 BANDS = (rspf_uint32)theMaxOutputValue.size();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      d += getMaxOutputValue(band);
   }
   
   return (d / BANDS); 
}

bool rspfHistogramRemapper::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   static const char MODULE[] = "rspfHistogramRemapper::loadState";
   if (traceDebug())
   {
      CLOG << " Entered..."
           << "\nprefix:  " << prefix << endl;
   }
   // Load the base class states...
   bool status = rspfTableRemapper::loadState(kwl, prefix);
   if (status)
   {
      const char* lookup = 0;
      rspfString keyword;
      
      lookup = kwl.find(prefix, HISTOGRAM_FILENAME_KW);
      if (lookup)
      {
         if ( !openHistogram(rspfFilename(lookup)) )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfHistogramRemapper::loadState ERROR!"
               << "\nCould not open file:  " << lookup
               << "\nReturning..." << endl;
            return false;
         }
      }
     
      //---
      // Get the band specific keywords.
      // NOTES:
      // -  This loadState assumes the all keywords will have the same number
      //    of bands.
      // -  The set methods cannot be used here as there may not be a connection
      //    yet that they need.
      //---
      rspf_uint32 bands = 0;
      lookup = kwl.find(prefix, rspfKeywordNames::NUMBER_BANDS_KW);
      if (lookup)
      {
         bands = rspfString::toUInt32(lookup);
      }
      else  // For backwards compatibility.
      {
         // Use the low clip to find number of bands...
         keyword = NORMALIZED_LOW_CLIP_POINT_KW;
         bands = kwl.numberOf(prefix, keyword);
      }
      //if(!bands&&theHistogram.valid())
     // {
     //    bands = theHistogram->getNumberOfBands();
     // }

      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfHistogramRemapper::loadState DEBUG:"
            << "\nBands:  " << bands
            << endl;
      }
      if (bands)
      {
         // Start with fresh clips.
         initializeClips(bands);
        for (rspf_uint32 band = 0; band < bands; ++band)
         {
            // Get the low clip.
            keyword = NORMALIZED_LOW_CLIP_POINT_KW;
            keyword += ".";
            keyword += rspfKeywordNames::BAND_KW;
            keyword += rspfString::toString(band+1);
            lookup = kwl.find(prefix, keyword);
            if(lookup)
            {
               theNormalizedLowClipPoint[band] = atof(lookup);
            }
            
            // Get the high clip.
            keyword = NORMALIZED_HIGH_CLIP_POINT_KW;
            keyword += ".";
            keyword += rspfKeywordNames::BAND_KW;
            keyword += rspfString::toString(band+1);
            lookup = kwl.find(prefix, keyword);
            if(lookup)
            {
               theNormalizedHighClipPoint[band] = atof(lookup);
            }
            
            // Get the mid point.
            keyword = MID_POINT_KW;
            keyword += ".";
            keyword += rspfKeywordNames::BAND_KW;
            keyword += rspfString::toString(band+1);
            lookup = kwl.find(prefix, keyword);
            if(lookup)
            {
               theMidPoint[band] = atof(lookup);
            }
            
            // Get the min output value.
            keyword = MIN_OUTPUT_VALUE_KW;
            keyword += ".";
            keyword += rspfKeywordNames::BAND_KW;
            keyword += rspfString::toString(band+1);
            lookup = kwl.find(prefix, keyword);
            if(lookup)
            {
               theMinOutputValue[band] = atof(lookup);
            }
            
            // Get the max output value.
            keyword = MAX_OUTPUT_VALUE_KW;
            keyword += ".";
            keyword += rspfKeywordNames::BAND_KW;
            keyword += rspfString::toString(band+1);
            lookup = kwl.find(prefix, keyword);
            if(lookup)
            {
               theMaxOutputValue[band] = atof(lookup);
            }      
         }
     }
     
      lookup = kwl.find(prefix, STRETCH_MODE_KW);
      if (lookup)
      {
         rspfString s = lookup;
         s.downcase();
         if (s == STRETCH_MODE[rspfHistogramRemapper::LINEAR_ONE_PIECE])
         {
            theStretchMode = rspfHistogramRemapper::LINEAR_ONE_PIECE;
         }
         else if (s == STRETCH_MODE[rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN])
         {
            theStretchMode = rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN;
         }
         else if (s == STRETCH_MODE[rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN])
         {
            theStretchMode = rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN;
         }
         else if (s == STRETCH_MODE[rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN])
         {
            theStretchMode = rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN;
         }
         else if (s == STRETCH_MODE[rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX])
         {
            theStretchMode = rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX;
         }
         else
         {
            theStretchMode = rspfHistogramRemapper::STRETCH_UNKNOWN;
         }
      }

      // Always set the dirty flag.
      theDirtyFlag = true;
   }
   verifyEnabled();

   if (traceDebug())
   {
      CLOG << "rspfHistogramRemapper::loadState DEBUG:"
           << *this
           << "\nExited..." << endl;
   }
   return status;
}

bool rspfHistogramRemapper::saveState(rspfKeywordlist& kwl,
                                       const char* prefix) const
{
   if (theHistogram.valid())
   {
      kwl.add(prefix,
              HISTOGRAM_FILENAME_KW,
              theHistogram->getHistogramFile().c_str(),
              true);
   }
   kwl.add(prefix,
           STRETCH_MODE_KW,
           getStretchModeString().c_str(),
           true);
	
   const rspf_uint32 BANDS = (rspf_uint32)theNormalizedLowClipPoint.size();
   
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_BANDS_KW,
           BANDS,
           true);
	
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      rspfString keyword;
		
      // Save the low clip.
      keyword = NORMALIZED_LOW_CLIP_POINT_KW;
      keyword += ".";
      keyword += rspfKeywordNames::BAND_KW;
      keyword += rspfString::toString(band+1);
      kwl.add(prefix,
              keyword,
              theNormalizedLowClipPoint[band],
              true);
		
      // Save the high clip.
      keyword = NORMALIZED_HIGH_CLIP_POINT_KW;
      keyword += ".";
      keyword += rspfKeywordNames::BAND_KW;
      keyword += rspfString::toString(band+1);
      kwl.add(prefix,
              keyword,
              theNormalizedHighClipPoint[band],
              true);
		
      // Save the mid point.
      keyword = MID_POINT_KW;
      keyword += ".";
      keyword += rspfKeywordNames::BAND_KW;
      keyword += rspfString::toString(band+1);
      kwl.add(prefix,
              keyword,
              theMidPoint[band],
              true);
		
      // Save the min output value.

      keyword = MIN_OUTPUT_VALUE_KW;
      keyword += ".";
      keyword += rspfKeywordNames::BAND_KW;
      keyword += rspfString::toString(band+1);
      kwl.add(prefix,
              keyword,
              theMinOutputValue[band],
              true);
		
      // Save the max output value.
      keyword = MAX_OUTPUT_VALUE_KW;
      keyword += ".";
      keyword += rspfKeywordNames::BAND_KW;
      keyword += rspfString::toString(band+1);

      kwl.add(prefix,
              keyword,
              theMaxOutputValue[band],
              true);
   }

   return rspfTableRemapper::saveState(kwl, prefix);
}

void rspfHistogramRemapper::setStretchMode(StretchMode mode,
                                            bool rebuildTable)
{
   if (theStretchMode != mode)
   {
      theStretchMode = mode;

      if (rebuildTable)
      {
         buildTable();

         // Check for internal bypass.
         verifyEnabled();

         // Clear the dirty flag.
         theDirtyFlag = false;
      }
      else
      {
         theDirtyFlag = true;
      }
   }
}

void rspfHistogramRemapper::setStretchModeAsString(const rspfString& mode,
                                                    bool rebuildTable)
{
   if( mode == "linear_one_piece")
   {
      setStretchMode(LINEAR_ONE_PIECE, rebuildTable);
   }
   else if(mode == "linear_1std_from_mean")
   {
      setStretchMode(LINEAR_1STD_FROM_MEAN, rebuildTable);
   }
   else if(mode == "linear_2std_from_mean")
   {
      setStretchMode(LINEAR_2STD_FROM_MEAN, rebuildTable);
   }
   else if(mode == "linear_3std_from_mean")
   {
      setStretchMode(LINEAR_3STD_FROM_MEAN, rebuildTable);
   }
   else if(mode == "linear_auto_min_max")
   {
      setStretchMode(LINEAR_AUTO_MIN_MAX, rebuildTable);
   }
   else
   {
      setStretchMode(STRETCH_UNKNOWN, false);
   }   
}

void rspfHistogramRemapper::buildTable()
{
   setupTable();
   switch(theStretchMode)
   {
      case rspfHistogramRemapper::LINEAR_ONE_PIECE:
      {
         buildLinearTable();
         break;
      }
      case rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX:
      {
         buildAutoLinearMinMaxTable();
         break;
      }
      case rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN:
      case rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN:
      case rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN:
         buildLinearTableStdFromMean();
         break;
         
      default:
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfHistogramRemapper::buildTable ERROR!"
               << "\nUnknown stretch type!" << endl;
         }
         break;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHistogramRemapper::buildTable DEBUG:\n" << endl;
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
}

void rspfHistogramRemapper::buildLinearTable()
{
   switch (getOutputScalarType())
   {
      case RSPF_UINT8:
      {
         buildLinearTable(rspf_uint8(0));
         break;
      }
			
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         buildLinearTable(rspf_uint16(0));
         break;
      }
			
      case RSPF_SINT16:
      {
         buildLinearTable(rspf_sint16(0));
         break;
      }

      case RSPF_UINT32:
      {
         buildLinearTable(rspf_uint32(0));
         break;
      }
			
      case RSPF_SINT32:
      {
         buildLinearTable(rspf_sint32(0));
         break;
      }
			
      case RSPF_NORMALIZED_FLOAT:      
      case RSPF_FLOAT:
      {
         buildLinearTable(rspf_float32(0));
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_DOUBLE:
      {
         buildLinearTable(rspf_float64(0));
         break;
      }
			
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
            // Shouldn't hit this.
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfHistogramRemapper::buildLinearTable RSPF_SCALAR_UNKNOWN!" << endl;
         }
         break;
      }
			
   } // End of "switch (theTableType)"
}

void rspfHistogramRemapper::buildAutoLinearMinMaxTable()
{
   switch (getOutputScalarType())
   {
      case RSPF_UINT8:
      {
         buildAutoLinearMinMaxTableTemplate(rspf_uint8(0));
         break;
      }
			
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         buildAutoLinearMinMaxTableTemplate(rspf_uint16(0));
         break;
      }
			
      case RSPF_SINT16:
      {
         buildAutoLinearMinMaxTableTemplate(rspf_sint16(0));
         break;
      }
			
      case RSPF_NORMALIZED_FLOAT:      
      case RSPF_FLOAT:
      {
         buildAutoLinearMinMaxTableTemplate(rspf_float32(0));
         break;
      }
			
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_DOUBLE:
      {
         buildAutoLinearMinMaxTableTemplate(rspf_float64(0));
         break;
      }
			
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
         // Shouldn't hit this.
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfTableRemapper::buildTable RSPF_SCALAR_UNKNOWN!"
               << endl;
         }
         break;
      }
			
   } // End of "switch (theTableType)"
}

template <class T> void rspfHistogramRemapper::buildLinearTable(T /* dummy */)
{
   // This builds a native table.
   theTableType = rspfTableRemapper::NATIVE;
   
   const rspf_uint32 BANDS = getNumberOfInputBands();

   // Sanity check.
   if (theNormalizedLowClipPoint.size() != BANDS || !getHistogram(0).valid())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramRemapper::buildTable ERROR!"
            << " Line:  " << __LINE__ << endl;
      }
      return;
   }
   
   T* table = reinterpret_cast<T*>(&theTable.front());
   rspf_uint32 index = 0;
   
   // Finally, build the table...
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      rspfRefPtr<rspfHistogram> h  = getHistogram(band);
      if (h.get())
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfHistogramRemapper::buildLinearTable DEBUG:"
               << "\nband:  " << band
               << "\nmean:  " << h->GetMean()
               << "\nstddev:  " << h->GetStandardDev()
               << endl;
         }
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramRemapper::buildTable ERROR!"
            << " Line:  " << __LINE__ << endl;
         }
         return; 
      }
		//std::cout << "MIN OUTPUT ==== " << theMinOutputValue[band] << std::endl;
      const T NULL_PIX = static_cast<T>(getNullPixelValue(band));
      const T MIN_PIX  = static_cast<T>(theMinOutputValue[band]);
      const T MAX_PIX  = static_cast<T>(theMaxOutputValue[band]);

      rspf_float64 min_clip_value =
         h->LowClipVal(theNormalizedLowClipPoint[band]);
      rspf_float64 max_clip_value =
         h->HighClipVal(1.0-theNormalizedHighClipPoint[band]);
      min_clip_value = floor(min_clip_value);
      max_clip_value = ceil(max_clip_value);
      rspf_float64 gain = (MAX_PIX-MIN_PIX+1)/(max_clip_value-min_clip_value);
      
      table[index] = NULL_PIX;
      ++index;

      for (rspf_uint32 pix = 1; pix < theTableBinCount; ++pix)
      {
         rspf_float64 p = pix;
         if (p <= min_clip_value)
         {
            p = MIN_PIX;
         }
         else if (p >= max_clip_value)
         {
            p = MAX_PIX;
         }
         else
         {
            p = ((p - min_clip_value) * gain) + MIN_PIX - 1.0;
         }

         if(p == NULL_PIX)
         {
            p = MIN_PIX;
         }
         table[index] = static_cast<T>(p+0.5);
			
         ++index;
      }
   } // End of band loop.

   // Clear the dirty flag so the table's not rebuilt on the next getTile.
   theDirtyFlag = false;
}

template <class T> void rspfHistogramRemapper::buildAutoLinearMinMaxTableTemplate(T /* dummy */)
{
   const rspf_uint32 BANDS = getNumberOfInputBands();

   // Sanity check.
   if (theNormalizedLowClipPoint.size() != BANDS || !getHistogram(0).valid())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramRemapper::buildAutoLinearMinMaxTableTemplate ERROR!"
            << " Line:  " << __LINE__ << endl;
      }
      return;
   }
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      rspfRefPtr<rspfHistogram> h      = getHistogram(band);
      T nullPix = static_cast<T>(getNullPixelValue(band));
      if(h.valid())
      {
         rspf_uint32 n     = h->GetRes();
         rspf_float64 low  = h->GetMinVal();
         rspf_float64 high = h->GetMaxVal();

 // std::cout << "LOW ========= " << low << std::endl;
  //std::cout << "HIGH ========= " << high << std::endl;

         double percentChange = .006;
         if(n > 0)
         {
            double newCount       = 0.0;
            double nextPercentage = 0.0;
            double percentage = 0.0;
            int idx = 0;
            float * counts = h->GetCounts();
            double count   = h->ComputeArea();
				
            for(idx = 0; idx < (rspf_int32)(n-1); ++idx)
            {
               if(nullPix != idx)
               {
                  newCount += counts[idx];
               }
               percentage = newCount / count;
               nextPercentage =
                  (newCount + counts[idx+1]) / count;
//               if (std::fabs(percentage - 0.006) <
//                   std::fabs(nextPercentage - 0.006))
                  if (std::fabs(percentage - percentChange) <
                      std::fabs(nextPercentage - percentChange))
               {
                  low = idx+1;
                  break;
               }
               
            }
            newCount = 0.0;
            for (idx = n-1; idx > 0; idx--)
            {
               newCount += counts[idx];
               percentage = newCount / count;
               nextPercentage =
                  (newCount + counts[idx-1]) / count;
               if (std::fabs(percentage - percentChange) <
                   std::fabs(nextPercentage - percentChange))
               {
                  high=idx-1;
                  break;
               }
            }
            if(low > high)
            {
               low = 0;
               high = n - 1;
            }
            //theMinOutputValue[band] = 1;
            //theMaxOutputValue[band] = 65535;

            //std::cout << "LOW ===== " << low << std::endl;
            //std::cout << "HIGH ===== " << high << std::endl;
            setLowClipPoint(low, band);
            setHighClipPoint(high, band);
         }
      }
   }
   
   buildLinearTable();
}

void rspfHistogramRemapper::buildLinearTableStdFromMean()
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
	
   // Sanity check.
   if (theNormalizedLowClipPoint.size() != BANDS || !getHistogram(0).valid())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramRemapper::buildTable ERROR!"
            << " Line:  " << __LINE__ << endl;
      }
      return;
   }
   
   rspf_float64 multiplier = 1.0;
   if (theStretchMode == LINEAR_2STD_FROM_MEAN)
   {
      multiplier =  2.0;
   }
   else if (theStretchMode == LINEAR_3STD_FROM_MEAN)
   {
      multiplier =  3.0;
   }
	
   // Finally, build the table...
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      rspfRefPtr<rspfHistogram> h      = getHistogram(band);
      rspf_float64 mean = 0.0;
      rspf_float64 stdDev = 0.0;
      if(h.valid())
      {
         mean     = h->GetMean();
         stdDev   = h->GetStandardDev();
      }
      rspf_float64 lowClip  = mean - (stdDev * multiplier);
      rspf_float64 highClip = mean + (stdDev * multiplier);
		
      // Clamp to min/max.
      if (lowClip < theMinOutputValue[band])
      {
         lowClip = theMinOutputValue[band];
      }
      if (highClip > theMaxOutputValue[band])
      {
         highClip = theMaxOutputValue[band];
      }
      
      setLowClipPoint(lowClip, band);
      setMidPoint(mean, band);
      setHighClipPoint(highClip, band);
      
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfHistogramRemapper::buildLinearStdFromMean DEBUG:"
            << "\nband:       " << band
            << "\nmean:       " << mean
            << "\nstddev:     " << stdDev
            << "\nlow clip:   " << lowClip
            << "\nhigh clip:  " << highClip
            << endl;
      }
   }
   
   buildLinearTable();
}

void rspfHistogramRemapper::initializeClips()
{
   initializeClips(getNumberOfInputBands());
}

void rspfHistogramRemapper::initializeClips(rspf_uint32 bands)
{
   //---
   // NOTE: This method deoes not set theDirtyFlag by design.
   //---
   if (bands)
   {
      theNormalizedLowClipPoint.resize(bands);
      theNormalizedHighClipPoint.resize(bands);
      theMidPoint.resize(bands);
      theMinOutputValue.resize(bands);
      theMaxOutputValue.resize(bands);
      
      for (rspf_uint32 band = 0; band < bands; ++band)
      {
         theNormalizedLowClipPoint[band]  = 0.0;
         theNormalizedHighClipPoint[band] = 1.0;
         theMidPoint[band] = 0.0;
         
         // Must have an output scalar type for getMin/Max call.
         if (theOutputScalarType != RSPF_SCALAR_UNKNOWN)
         {
            theMinOutputValue[band] = rspf::defaultMin(theOutputScalarType);//getMinPixelValue(band);
            theMaxOutputValue[band] = rspf::defaultMax(theOutputScalarType);//getMaxPixelValue(band);
         }
         else
         {
            theMinOutputValue[band] = 0.0;
            theMaxOutputValue[band] = 0.0;
         }
      }
   }
}

void rspfHistogramRemapper::initializeMinMaxOutput()
{
   rspf_uint32 nBands = getNumberOfInputBands();
   theMinOutputValue.resize(nBands);
   theMaxOutputValue.resize(nBands);
   for (rspf_uint32 band = 0; band < nBands; ++band)
   {
         // Must have an output scalar type for getMin/Max call.
         if (theOutputScalarType != RSPF_SCALAR_UNKNOWN)
         {
            theMinOutputValue[band] = rspf::defaultMin(theOutputScalarType);//getMinPixelValue(band);
            theMaxOutputValue[band] = rspf::defaultMax(theOutputScalarType);//getMaxPixelValue(band);
         }
         else
         {
            theMinOutputValue[band] = 0.0;
            theMaxOutputValue[band] = 0.0;
         }
  }

}

void rspfHistogramRemapper::setNullCount()
{
   // This will set the null bucket for all res zero bands to zero!
   if (getHistogram(0).valid())
   {
      const rspf_uint32 BANDS = theHistogram->getNumberOfBands();
      for (rspf_uint32 band = 0; band < BANDS; ++band)
      {
         rspfRefPtr<rspfHistogram> h = theHistogram->getHistogram(band);
         if (h.valid())
         {
            h->SetCount(0, 0);
         }
      }
   }
}

rspfHistogramRemapper::StretchMode rspfHistogramRemapper::getStretchMode() const
{
   return theStretchMode;
}

rspfString rspfHistogramRemapper::getStretchModeString() const
{
   return rspfString(STRETCH_MODE[theStretchMode]);
}

rspf_uint32 rspfHistogramRemapper::getNumberOfStretchModes() const
{
   return rspfHistogramRemapper::STRETCH_UNKNOWN; // last index
}

rspfString
rspfHistogramRemapper::getStretchModeString(rspf_uint32 index) const
{
   if (index < rspfHistogramRemapper::STRETCH_UNKNOWN)
   {
      return rspfString(STRETCH_MODE[index]);
   }
	
   return rspfString(STRETCH_MODE[rspfHistogramRemapper::STRETCH_UNKNOWN]);
   
}

ostream& rspfHistogramRemapper::print(ostream& os) const
{
   os << setiosflags(ios::fixed) << setprecision(15)
      << "\nrspfHistogramRemapper::print:"
      << "\ntheStretchMode:         " << getStretchModeString().c_str()
      << "\ntheDirtyFlag:           " << (theDirtyFlag?"true":"false")
      << "\ntheBypassFlag:          " << (theBypassFlag?"true":"false")
      
      << "\ntheHistogram:           " << (theHistogram.valid()?"initialized":"null")
      << "\ntheTableSizeInBytes:    " << theTable.size()
      << endl;
	
   const rspf_uint32 BANDS = (rspf_uint32)theNormalizedLowClipPoint.size();
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      os << "\ntheNormalizedLowClipPoint[" << band << "]:   "
         << theNormalizedLowClipPoint[band]
         << "\ntheNormalizedHighClipPoint[" << band << "]:  "
         << theNormalizedHighClipPoint[band]
         << "\ntheMidPoint[" << band << "]:                 "
         << theMidPoint[band]
         << "\ntheMinOutputValue[" << band << "]:           "
         << theMinOutputValue[band]
         << "\ntheMaxOutputValue[" << band << "]:           "
         << theMaxOutputValue[band]
         << endl;
   }
	
   for (rspf_uint32 i = 0; i < theBandList.size(); ++i)
   {
      os << "theBandList[" << i << "]:  " << theBandList[i] << endl;
   }

   // Note: To set table call rspfTableRemapper::print or turn trace on
   // for "rspfTableRemapper:debug"
   
   return os;
}

void rspfHistogramRemapper::setupTable()
{
   const rspf_uint32 BANDS = getNumberOfInputBands();
   
   if (theNormalizedLowClipPoint.size() == 0)
   {
      initializeClips(BANDS);
   }
   
   rspf_uint32 values_per_band = 0;
   rspf_uint32 bytes_per_pixel = 0;
   
   switch (theOutputScalarType)
   {
      case RSPF_UINT8:
         values_per_band = 256;  // 2 ^ 8
         bytes_per_pixel = 1;
         theTableType = rspfTableRemapper::NATIVE;
         break;
         
      case RSPF_USHORT11:
         values_per_band = 2048; // 2 ^ 11
         bytes_per_pixel = 2;
         theTableType = rspfTableRemapper::NATIVE;
         break;
         
      case RSPF_UINT16:
      case RSPF_SINT16:
         values_per_band = 65536; // 2 ^ 16
         bytes_per_pixel = 2;
         theTableType = rspfTableRemapper::NATIVE;
         break;
         
      case RSPF_UINT32:
      case RSPF_SINT32:
         values_per_band = 65536; // 2 ^ 16
         bytes_per_pixel = 4;
         theTableType = rspfTableRemapper::NATIVE;
         break;
			
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT:
         bytes_per_pixel = 4;
         break;
			
      case RSPF_NORMALIZED_DOUBLE:         
      case RSPF_DOUBLE:
         bytes_per_pixel = 8;
         theTableType = rspfTableRemapper::NORMALIZED;
         break;
         
      default:
         break;
   }
	
   if ( theOutputScalarType == RSPF_FLOAT ||
        theOutputScalarType == RSPF_DOUBLE ||
        theOutputScalarType == RSPF_NORMALIZED_FLOAT ||
        theOutputScalarType == RSPF_NORMALIZED_DOUBLE )
   {
      for (rspf_uint32 band = 0; band < BANDS; ++band)
      {
         rspfRefPtr<rspfHistogram> h  = getHistogram(band);
			
         if (h.valid())
         {
            if (h->GetRes() > static_cast<rspf_int32>(values_per_band))
            {
               values_per_band = h->GetRes();
            }
         }
      }
   }      
   
   theTableBinCount  = values_per_band;
   theTableBandCount = BANDS;
   // Check the size of the table prior to deletion and making a new one.
   rspf_uint32 size_in_bytes = values_per_band * BANDS * bytes_per_pixel;
   if(theTable.empty() || (theTable.size() != size_in_bytes))
   {
      theTable.resize(size_in_bytes);
   }

   rspfImageSource* input = dynamic_cast<rspfImageSource*>(getInput());
   double minPix = rspf::defaultMin(getOutputScalarType());
   double maxPix = rspf::defaultMax(getOutputScalarType());
   
   if(input)
   {
      //---
      // Last check for NaNs in key data members and set to some default if so.
      // This could occur if someone stripped a keyword list down to a minimal
      // set of keywords.
      //---
      for (rspf_uint32 band = 0; band < BANDS; ++band)
      {
         minPix = input->getMinPixelValue(band);
         maxPix = input->getMaxPixelValue(band);
         if ( rspf::isnan(theMinOutputValue[band]) )
         {
            theMinOutputValue[band] = minPix;
         }
         if ( rspf::isnan(theMaxOutputValue[band]) )
         {
            theMaxOutputValue[band] = maxPix;
         }
      }   
   }
   else
   {
      //---
      // Last check for NaNs in key data members and set to some default if so.
      // This could occur if someone stripped a keyword list down to a minimal
      // set of keywords.
      //---
      for (rspf_uint32 band = 0; band < BANDS; ++band)
      {
         if ( rspf::isnan(theMinOutputValue[band]) )
         {
            theMinOutputValue[band] = minPix;
         }
         if ( rspf::isnan(theMaxOutputValue[band]) )
         {
            theMaxOutputValue[band] = maxPix;
         }
      }   
   }
}

void rspfHistogramRemapper::verifyEnabled()
{
   //---
   // Since this filter can be constructed with no input connection do not
   // output and error, simply return.
   //---	
      setBypassFlag(true);
   //if (theInputConnection)
   {

#if 1     
      // Start off bypassed.

      if (theStretchMode != STRETCH_UNKNOWN)
      {
         if(theHistogram.valid())
         {
            setBypassFlag(false);
         }
/*         
         const rspf_uint32 BANDS =
            (rspf_uint32)theNormalizedLowClipPoint.size();
         for (rspf_uint32 band = 0; band < BANDS; ++band)
         {
            //const double MIN = rspfTableRemapper::getMinPixelValue(band);
            //const double MAX = rspfTableRemapper::getMaxPixelValue(band);
            if ( theNormalizedLowClipPoint[band]  != 0.0   ||
                 theNormalizedHighClipPoint[band] != 1.0 )
                // theMinOutputValue[band] != MIN ||
                 //theMaxOutputValue[band] != MAX )
            {
               // Need to turn filter on.
               setBypassFlag(false);
               break;
            }
         }
         */
      }
#endif
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfHistogramRemapper::verifyEnabled theBypassFlag: "
         << (theBypassFlag ? "true" : "false") << endl;
   }
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfHistogramRemapper::getHistogram()
{
   return rspfRefPtr<rspfMultiResLevelHistogram>(theHistogram.get());
}

rspfRefPtr<const rspfMultiResLevelHistogram> rspfHistogramRemapper::getHistogram()const
{
   return rspfRefPtr<const rspfMultiResLevelHistogram>(theHistogram.get());
}

rspfRefPtr<rspfHistogram>
rspfHistogramRemapper::getHistogram(rspf_uint32 zero_based_band,
                                     rspf_uint32 res_level) const
{
   rspf_uint32 histogram_band = getHistogramBand(zero_based_band);
   
   if(!theHistogram.valid())
   {
      const rspfHistogramSource* source = dynamic_cast<const rspfHistogramSource*>(getInput(1));
      if(source)
      {
         theHistogram = const_cast<rspfHistogramSource*>(source)->getHistogram();
      }      
   }
   
   if (theHistogram.valid())
   {
      if (histogram_band < theHistogram->getNumberOfBands())
      {
         return theHistogram->getHistogram(histogram_band, res_level);
      }
   }
	
   return 0;
}

rspfFilename rspfHistogramRemapper::getHistogramFile() const
{
   if (theHistogram.valid())
   {
      return theHistogram->getHistogramFile();
   }
	
   return rspfFilename::NIL;
}

rspf_uint32
rspfHistogramRemapper::getHistogramBand(rspf_uint32 input_band) const
{
   if(theResetBandIndicesFlag) return input_band;
   vector<rspf_uint32> bandList;
   getOutputBandList(bandList);
   
   const rspf_uint32 BANDS = (rspf_uint32)bandList.size();
	
   if (BANDS == 0)
   {
      return 0;
   }
	
   return bandList[input_band];
}

rspfString rspfHistogramRemapper::getLongName()const
{
   return rspfString("Histogram Remapper, filter with controls for histogram stretching.");
}

rspfString rspfHistogramRemapper::getShortName()const
{
   return rspfString("Histogram Remapper");
}

ostream& operator<<(ostream& os, const rspfHistogramRemapper& hr)
{
   return hr.print(os);
}


// Private to disallow use...
rspfHistogramRemapper::rspfHistogramRemapper(const rspfHistogramRemapper&)
{
}

// Private to disallow use...
rspfHistogramRemapper& rspfHistogramRemapper::operator=(const rspfHistogramRemapper&)
{
   return *this;
}

void rspfHistogramRemapper::setBypassFlag(bool flag)
{
   if (theBypassFlag != flag)
   {
      //---
      // If switching from bypassed to enabled set
      // the dirty flag.  We don't care about going the other way.
      //---
      if ( theBypassFlag && !flag )
      {
         theDirtyFlag = true;
      }
      theBypassFlag = flag;
   }
}

double rspfHistogramRemapper::getMinPixelValue(rspf_uint32 band)const
{
   double result = rspfTableRemapper::getMinPixelValue(band);
   if(theEnableFlag&&!theBypassFlag &&(band < theMinOutputValue.size()))
   {
      result = theMinOutputValue[band];
   }
  // if(theOutputScalarType != RSPF_SCALAR_UNKNOWN)
  // {
  //    return rspf::defaultMin(theOutputScalarType);
  // }

   return result;
}

double rspfHistogramRemapper::getMaxPixelValue(rspf_uint32 band)const
{
   double result = rspfTableRemapper::getMaxPixelValue(band);

   if(theEnableFlag&&!theBypassFlag &&(band < theMaxOutputValue.size()))
   {
      result = theMaxOutputValue[band];
   }
   //if(theOutputScalarType != RSPF_SCALAR_UNKNOWN)
  // {
  //    result = rspf::defaultMax(theOutputScalarType);
  // }

   return result;
}

bool rspfHistogramRemapper::canConnectMyInputTo(rspf_int32 inputIndex,
                                                 const rspfConnectableObject* object)const
{
   // we will alloe the histogram source only to be connected to input slot 2 and
   // the first slot will be reserved for the rspfImageSource.
   //
   bool result = false;
   if(inputIndex == 0) result = dynamic_cast<const rspfImageSource*>(object)!=0;
   if(inputIndex == 1) result = dynamic_cast<const rspfHistogramSource*>(object)!=0;
   
   return result;
}

