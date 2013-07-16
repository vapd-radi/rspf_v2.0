//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:  Interface class for overview builders.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderBase.cpp 21745 2012-09-16 15:21:53Z dburken $

#include <rspf/imaging/rspfOverviewBuilderBase.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/imaging/rspfImageHandler.h>

RTTI_DEF3(rspfOverviewBuilderBase,
          "rspfOverviewBuilderBase",
          rspfSource,
          rspfProcessInterface,
          rspfConnectableObjectListener)

static const std::string SCAN_FLOAT_DATA_KW = "overview_builder.scan_for_min_max_null_if_float";
   
rspfOverviewBuilderBase::rspfOverviewBuilderBase()
   : m_overviewStopDimension(0),
     m_histoMode(RSPF_HISTO_MODE_UNKNOWN),
     m_bitMaskSpec(),
     m_imageHandler(0),
     m_originalBandList(0),
     m_maskWriter(0),
     m_maskFilter(0),
     m_outputFile(rspfFilename::NIL),
     m_scanForMinMax(false),
     m_scanForMinMaxNull(false),
     m_scanFloatData(false)
{
   initializePreferenceSettings();
}

rspfOverviewBuilderBase::~rspfOverviewBuilderBase()
{
   m_imageHandler = 0;
   m_maskWriter = 0;
   m_maskFilter = 0;
}

bool rspfOverviewBuilderBase::setOutputWriter(rspfImageFileWriter* /* outputWriter */)
{
   return false;
}

bool rspfOverviewBuilderBase::setInputSource(rspfImageHandler* imageSource)
{
   bool result = false;
   if ( imageSource )
   {
      if ( imageSource->getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         m_imageHandler = imageSource;

         // Check handler to see if it's filtering bands.
         if ( m_imageHandler->isBandSelector() )
         { 
            // Capture for finalize method.
            m_imageHandler->getOutputBandList( m_originalBandList );
            
            // Set output list to input.
            m_imageHandler->setOutputToInputBandList();
         }

         // This will set the flag to scan for min, max, nulls if needed.
         initializeScanOptions();
         
         result = true;
      }
   }
   return result;
}

bool rspfOverviewBuilderBase::hasOverviewType(const rspfString& type) const
{
   std::vector<rspfString> typeList;
   getTypeNameList(typeList);
   std::vector<rspfString>::const_iterator i = typeList.begin();
   while ( i != typeList.end() )
   {
      if ( (*i) == type )
      {
         return true;
      }
      ++i;
   }
   return false;
}

rspf_uint32 rspfOverviewBuilderBase::getRequiredResLevels(
   const rspfImageHandler* ih) const
{
   rspf_uint32 result = 0;
   if (ih)
   {
      // Note we always have one rset
      result = 1;

      rspf_uint32 largestImageDimension =
         ih->getNumberOfSamples(0) >
         ih->getNumberOfLines(0) ?
         ih->getNumberOfSamples(0) :
         ih->getNumberOfLines(0);

      while(largestImageDimension > m_overviewStopDimension)
      {
         largestImageDimension /= 2;
         ++result;
      }
   }
   return result;
}

rspf_uint32 rspfOverviewBuilderBase::getOverviewStopDimension() const
{
   return m_overviewStopDimension;
}

void rspfOverviewBuilderBase::setOverviewStopDimension(rspf_uint32 dim)
{
   m_overviewStopDimension = dim;
}

rspfHistogramMode rspfOverviewBuilderBase::getHistogramMode() const
{
   return m_histoMode;
}

void rspfOverviewBuilderBase::setHistogramMode(rspfHistogramMode mode)
{
   m_histoMode = mode;
}

void rspfOverviewBuilderBase::finalize()
{
   if ( m_imageHandler.valid() && m_imageHandler->isBandSelector() && m_originalBandList.size() )
   {
      m_imageHandler->setOutputBandList( m_originalBandList );
   }
}

void rspfOverviewBuilderBase::setBitMaskSpec(const rspfKeywordlist& bit_mask_spec)
{
   m_bitMaskSpec = bit_mask_spec;
}

void rspfOverviewBuilderBase::setScanForMinMax(bool flag)
{
   m_scanForMinMax  = flag;
}

bool rspfOverviewBuilderBase::getScanForMinMax() const
{
   return m_scanForMinMax;
}

void rspfOverviewBuilderBase::setScanForMinMaxNull(bool flag)
{
   m_scanForMinMaxNull = flag;
}

bool rspfOverviewBuilderBase::getScanForMinMaxNull() const
{
   return m_scanForMinMaxNull;
}

void rspfOverviewBuilderBase::initializePreferenceSettings()
{
   // Get the stop dimension from rspf preferences.
   const char* lookup = rspfPreferences::instance()->
      findPreference(rspfKeywordNames::OVERVIEW_STOP_DIMENSION_KW);
   if (lookup)
   {
     m_overviewStopDimension = rspfString(lookup).toUInt32();
   }

   if ( m_overviewStopDimension == 0 )
   {
      // Use the smallest default tile size.
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      m_overviewStopDimension = tileSize.x < tileSize.y ? tileSize.x : tileSize.y;
   }

   // Look for overview_builder.scan_for_min_max_null_if_float:
   lookup = rspfPreferences::instance()->findPreference(SCAN_FLOAT_DATA_KW.c_str());
   if ( lookup )
   {
      m_scanFloatData = rspfString(lookup).toBool();
   }
}

void rspfOverviewBuilderBase::initializeScanOptions()
{
   if ( m_scanFloatData && m_imageHandler.valid())
   {
      rspfScalarType scalar = m_imageHandler->getOutputScalarType();
      if ( ( scalar == RSPF_FLOAT32 ) || ( scalar == RSPF_FLOAT64 ) )
      {
         setScanForMinMaxNull(true);
      }
   }
}
