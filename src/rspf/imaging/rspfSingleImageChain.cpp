//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Utility class definition for a single image chain.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/imaging/rspfSingleImageChain.h>

#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/support_data/rspfSrcRecord.h>

rspfSingleImageChain::rspfSingleImageChain()
   :
   rspfImageChain(),
   m_handler(0),
   m_bandSelector(0),
   m_histogramRemapper(0),
   m_resamplerCache(0),
   m_resampler(0),
   m_scalarRemapper(0),
   m_chainCache(0),
   m_addHistogramFlag(false),
   m_addResamplerCacheFlag(false),
   m_addChainCacheFlag(false),
   m_remapToEightBitFlag(false),
   m_threeBandFlag(false),
   m_threeBandReverseFlag(false)
{
}

rspfSingleImageChain::rspfSingleImageChain(bool addHistogramFlag,
                                             bool addResamplerCacheFlag,
                                             bool addChainCacheFlag,
                                             bool remapToEightBitFlag,
                                             bool threeBandFlag,
                                             bool threeBandReverseFlag)
   :
   rspfImageChain(),
   m_handler(0),
   m_bandSelector(0),
   m_histogramRemapper(0),
   m_resamplerCache(0),
   m_resampler(0),
   m_scalarRemapper(0),
   m_chainCache(0),
   m_addHistogramFlag(addHistogramFlag),
   m_addResamplerCacheFlag(addResamplerCacheFlag),
   m_addChainCacheFlag(addChainCacheFlag),
   m_remapToEightBitFlag(remapToEightBitFlag),
   m_threeBandFlag(threeBandFlag),
   m_threeBandReverseFlag(threeBandReverseFlag)   
{
}

rspfSingleImageChain::~rspfSingleImageChain()
{
}

void rspfSingleImageChain::reset()
{
   bool result = true;
   do
   {
      result = deleteLast();
   } while (result);

   m_handler               = 0;
   m_bandSelector          = 0;
   m_histogramRemapper     = 0;
   m_resamplerCache        = 0;
   m_resampler             = 0;
   m_scalarRemapper        = 0;
   m_chainCache            = 0;
   m_addHistogramFlag      = false;
   m_addResamplerCacheFlag = false;
   m_addChainCacheFlag     = false;
   m_remapToEightBitFlag   = false;
   m_threeBandFlag         = false;
   m_threeBandReverseFlag  = false;
}

void rspfSingleImageChain::close()
{
   if ( m_handler.valid() )
   {
      if ( removeChild(m_handler.get()) )
      {
         m_handler = 0;
      }
   }
}

rspfFilename rspfSingleImageChain::getFilename() const
{
   rspfFilename result;
   if ( m_handler.valid() )
   {
      result = m_handler->getFilename();
   }
   return result;
}

bool rspfSingleImageChain::open(const rspfFilename& file, bool openOverview)
{
   return addImageHandler(file, openOverview);
}

bool rspfSingleImageChain::isOpen() const
{
   return m_handler.valid();
}

bool rspfSingleImageChain::open(const rspfSrcRecord& src)
{
   return addImageHandler(src);
}

void rspfSingleImageChain::createRenderedChain()
{
   // Band selector after image handler only if needed.
   if ( m_handler.valid() )
   {
      // Only add if multiple bands.
      if ( m_handler->getNumberOfOutputBands() != 1 )
      {
         addBandSelector();
         if ( m_threeBandReverseFlag )
         {
            setToThreeBandsReverse();
         }
      }
   }
   else // No image handler so just add it.
   {
      addBandSelector();
   }
   
   // histogram
   if ( m_addHistogramFlag )
   {
      addHistogramRemapper();
   }

   // resampler cache
   if ( m_addResamplerCacheFlag )
   {
      m_resamplerCache = addCache();
   }

   // scalar remapper
   if ( m_remapToEightBitFlag )
   {
      if ( m_handler.valid() )
      {
         // See if it's eight bit.
         if (m_handler->getOutputScalarType() != RSPF_UINT8)
         {
            addScalarRemapper();
         }
      }
      else
      {
         // Just add...
         addScalarRemapper(); 
      }
   }
   
   // resampler
   addResampler();

   //---
   // Do this here so that if a band selector is added to the end of the
   // chain it will go in before the end of chain cache.
   //---
   if (m_threeBandFlag)
   {
      if (!m_bandSelector)
      {
         addBandSelector();
      }
      setToThreeBands();
   }

   //---
   // Ditto...
   //---
   if ( m_threeBandReverseFlag && !m_bandSelector )
   {
      addBandSelector();
      setToThreeBandsReverse();
   }

   // End of chain cache.
   if ( m_addChainCacheFlag )
   {
      m_chainCache = addCache();
   }

   initialize();

}

void rspfSingleImageChain::createRenderedChain(const rspfSrcRecord& src)
{
   // Band selector after image handler only if needed.
   if ( m_handler.valid() )
   {
      // Only add if needed.
      if ( ( m_handler->getNumberOfOutputBands() != 1 ) || src.getBands().size() )
      {
         addBandSelector(src);
         if ( m_threeBandReverseFlag )
         {
            setToThreeBandsReverse();
         }
      }
   }
   else // No image handler so just add it.
   {
      addBandSelector(src);
   }
   
   // histogram
   if ( m_addHistogramFlag || src.getHistogramOp().size() )
   {
      addHistogramRemapper(src);
   }

   // resampler cache
   if ( m_addResamplerCacheFlag )
   {
      m_resamplerCache = addCache();
   }

   // scalar remapper
   if ( m_remapToEightBitFlag )
   {
      if ( m_handler.valid() )
      {
         // See if it's eight bit.
         if (m_handler->getOutputScalarType() != RSPF_UINT8)
         {
            addScalarRemapper();
         }
      }
      else
      {
         // Just add...
         addScalarRemapper(); 
      }
   }
   
   // resampler
   addResampler();

   //---
   // Do this here so that if a band selector is added to the end of the
   // chain it will go in before the end of chain cache.
   //---
   if (m_threeBandFlag)
   {
      if (!m_bandSelector) // Input must be one band.
      {
         addBandSelector(src);
      }
      setToThreeBands();
   }

   //---
   // Ditto...
   //---
   if ( m_threeBandReverseFlag && !m_bandSelector )
   {
      addBandSelector();
      setToThreeBandsReverse();
   }

   // End of chain cache.
   if ( m_addChainCacheFlag )
   {
      m_chainCache = addCache();
   }

   initialize();

}

bool rspfSingleImageChain::addImageHandler(const rspfFilename& file, bool openOverview)
{
   bool result = false;

   close();
   
   m_handler = rspfImageHandlerRegistry::instance()->open(file, true, openOverview);
   
   if ( m_handler.valid() )
   {
      // Add to the chain.  Note: last is really first.
      addLast( m_handler.get() );
      
      result = true;
   }

   return result;
}

bool rspfSingleImageChain::addImageHandler(const rspfSrcRecord& src)
{
   bool result = addImageHandler( src.getFilename() );
   if (result)
   {
      //---
      // When loading from rspfSrcRecord typically the overview/histograms are
      // not in the same directory and the "support" keyword is not set.  For
      // the rspfImageHandler::getFilenameWithThisExtension to work correctly
      // the rspfImageHandler::theSupplementaryDirectory must be set.
      // So if the rspfSrcRecord::getSupportDir() is empty and the overview
      // or histogram is not co-located with the image we will set it here.
      //---
      rspfFilename supportDir = src.getSupportDir();
      if ( supportDir.empty() )
      {
         if ( src.getOverviewPath().size() )
         {
            if ( src.getOverviewPath().isDir() )
            {
               supportDir = src.getOverviewPath();
            }
            else
            {
               supportDir = src.getOverviewPath().path();
            }
         }
         else if ( src.getHistogramPath().size() )
         {
            if ( src.getHistogramPath().isDir() )
            {
               supportDir = src.getHistogramPath();
            }
            else
            {
               supportDir = src.getHistogramPath().path();
            }
         }
         else if ( src.getMaskPath().size() )
         {
            if ( src.getMaskPath().isDir() )
            {
               supportDir = src.getMaskPath();
            }
            else
            {
               supportDir = src.getMaskPath().path();
            }
         }
      }
      if ( supportDir.size() && (src.getFilename().path() != supportDir) )
      {
         m_handler->setSupplementaryDirectory( supportDir );
      }
      if ( src.getEntryIndex() > 0 ) // defaulted to -1.
      {
         m_handler->setCurrentEntry( static_cast<rspf_uint32>( src.getEntryIndex() ) );
      }
      if ( m_handler->getOverview() == 0 )
      {
         if ( src.getOverviewPath().size() )
         {
            m_handler->openOverview( src.getOverviewPath() );
         }
         else
         {
            rspfFilename ovrFile = m_handler->getFilenameWithThisExtension(rspfString(".ovr"));
            m_handler->openOverview( ovrFile ); 
         }
      }
   }
   return result;
}

void rspfSingleImageChain::addBandSelector()
{
   if (!m_bandSelector)
   {
      m_bandSelector = new rspfBandSelector();

      // Add to the end of the chain.
      addFirst(m_bandSelector.get());
   }
}

void rspfSingleImageChain::addBandSelector(const rspfSrcRecord& src)
{
   if (!m_bandSelector)
   {
      m_bandSelector = new rspfBandSelector();
      
      // Add to the end of the chain.
      addFirst(m_bandSelector.get());
   }
   if ( src.getBands().size() )
   {
      m_bandSelector->setOutputBandList( src.getBands() );
   }
}

void rspfSingleImageChain::addHistogramRemapper()
{
   if (!m_histogramRemapper)
   {
      m_histogramRemapper = new rspfHistogramRemapper();
      
      m_histogramRemapper->setEnableFlag(false);

      // Add to the end of the chain.
      addFirst(m_histogramRemapper.get());
   } 
}

void rspfSingleImageChain::addHistogramRemapper(const rspfSrcRecord& src)
{
   static const char MODULE[] =
      "rspfSingleImageChain::addHistogramRemapper(const rspfSrcRecord&)";
   
   if (!m_histogramRemapper)
   {
      m_histogramRemapper = new rspfHistogramRemapper();
      
      m_histogramRemapper->setEnableFlag(false);

      // Add to the end of the chain.
      addFirst(m_histogramRemapper.get());
   }

   if ( src.getHistogramOp().size() && m_handler.valid() )
   {
      // Create histogram code here???
      
      // Open the histogram if needed.
      if ( m_histogramRemapper->getHistogramFile() == rspfFilename::NIL )
      {
         rspfFilename f;
         if ( src.getSupportDir().size() )
         {
            f = src.getSupportDir();
            f.dirCat( m_handler->getFilename().fileNoExtension() );
            f.setExtension(".his");
         }
         else
         {
            f = m_handler->getFilenameWithThisExtension( rspfString("his") );
         }
         if ( m_histogramRemapper->openHistogram( f ) == false )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << "\nCould not open:  " << f << "\n";
         }
      }

      // Set the histogram strech mode.
      if ( src.getHistogramOp().size() )
      {
         // Enable.
         m_histogramRemapper->setEnableFlag(true);
         
         // Set the histo mode:
         rspfString op = src.getHistogramOp();
         op.downcase();
         if ( op == "auto-minmax" )
         {
            m_histogramRemapper->setStretchMode( rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX );
         }
         else if ( (op == "std-stretch-1") || (op == "std-stretch 1") )
         {
            m_histogramRemapper->setStretchMode( rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN );
         } 
         else if ( (op == "std-stretch-2") || (op == "std-stretch 2") )
         {
            m_histogramRemapper->setStretchMode( rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN );
         } 
         else if ( (op == "std-stretch-3") || (op == "std-stretch 3") )
         {
            m_histogramRemapper->setStretchMode( rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN );
         }
         else
         {
            m_histogramRemapper->setEnableFlag(false);
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << "\nUnhandled operation: " << op << "\n";
         }
      }
      
   } // End: if ( src.setHistogramOp().size() && m_handler.valid() )
}

rspfRefPtr<rspfCacheTileSource> rspfSingleImageChain::addCache()
{
   rspfRefPtr<rspfCacheTileSource> cache = new rspfCacheTileSource();

   // Add to the end of the chain.
   addFirst(cache.get());

   return cache;
}

void rspfSingleImageChain::addResampler()
{
   if ( !m_resampler )
   {
      m_resampler = new rspfImageRenderer();

      // Add to the end of the chain.
      addFirst(m_resampler.get());
   }
}

void rspfSingleImageChain::addScalarRemapper()
{
   if ( !m_scalarRemapper )
   {
      m_scalarRemapper = new rspfScalarRemapper();

      if ( m_resamplerCache.valid() )
      {
         // Add to the left of the resampler cache.
         insertLeft(m_scalarRemapper.get(), m_resamplerCache.get());
      }
      else
      {
         // Add to the end of the chain.
         addFirst(m_scalarRemapper.get());
      }
   }
}

rspfRefPtr<rspfImageHandler> rspfSingleImageChain::getImageHandler() const
{
   return m_handler;
}

rspfRefPtr<rspfImageHandler> rspfSingleImageChain::getImageHandler()
{
   return m_handler;
}

rspfRefPtr<rspfBandSelector> rspfSingleImageChain::getBandSelector() const
{
   return m_bandSelector;
}

rspfRefPtr<rspfBandSelector> rspfSingleImageChain::getBandSelector()
{
   return m_bandSelector;
}

rspfRefPtr<rspfHistogramRemapper> rspfSingleImageChain::getHistogramRemapper() const
{
   return m_histogramRemapper;
}

rspfRefPtr<rspfHistogramRemapper> rspfSingleImageChain::getHistogramRemapper()
{
   return m_histogramRemapper;
}

rspfRefPtr<rspfCacheTileSource> rspfSingleImageChain::getResamplerCache() const
{
   return m_resamplerCache;
}

rspfRefPtr<rspfCacheTileSource> rspfSingleImageChain::getResamplerCache()
{
   return m_resamplerCache;
}

rspfRefPtr<rspfImageRenderer> rspfSingleImageChain::getImageRenderer() const
{
   return m_resampler;
}

rspfRefPtr<rspfImageRenderer> rspfSingleImageChain::getImageRenderer()
{
   return m_resampler;
}

rspfRefPtr<rspfScalarRemapper> rspfSingleImageChain::getScalarRemapper() const
{
   return m_scalarRemapper;
}

rspfRefPtr<rspfScalarRemapper> rspfSingleImageChain::getScalarRemapper()
{
   return m_scalarRemapper;
}

rspfRefPtr<rspfCacheTileSource> rspfSingleImageChain::getChainCache() const
{
   return m_chainCache;
}

rspfRefPtr<rspfCacheTileSource> rspfSingleImageChain::getChainCache()
{
   return m_chainCache;
}

void rspfSingleImageChain::setAddHistogramFlag(bool flag)
{
   m_addHistogramFlag = flag;
}

bool rspfSingleImageChain::getAddHistogramFlag() const
{
   return m_addHistogramFlag;
}

void rspfSingleImageChain::setAddResamplerCacheFlag(bool flag)
{
   m_addResamplerCacheFlag = flag;
}

bool rspfSingleImageChain::getAddResamplerCacheFlag() const
{
   return m_addResamplerCacheFlag;
}

void rspfSingleImageChain::setAddChainCacheFlag(bool flag)
{
   m_addChainCacheFlag = flag;
}

bool rspfSingleImageChain::getAddChainCacheFlag() const
{
   return m_addChainCacheFlag;
}

void rspfSingleImageChain::setRemapToEightBitFlag(bool flag)
{
   m_remapToEightBitFlag = flag;
}

bool rspfSingleImageChain::getRemapToEightBitFlag() const
{
   return m_remapToEightBitFlag;
}

void rspfSingleImageChain::setThreeBandFlag(bool flag)
{
   m_threeBandFlag = flag;
}

bool rspfSingleImageChain::getThreeBandFlag() const
{
   return m_threeBandFlag;
}
   
void rspfSingleImageChain::setThreeBandReverseFlag(bool flag)
{
   m_threeBandReverseFlag = flag;
}

bool rspfSingleImageChain::getThreeBandReverseFlag() const
{
   return m_threeBandReverseFlag;
}

void rspfSingleImageChain::setToThreeBands()
{
   if ( m_handler.valid() )
   {
      std::vector<rspf_uint32> bandList(3);
      const rspf_uint32 BANDS = m_handler->getNumberOfInputBands();
      if(BANDS >= 3)
      {
         bandList[0] = 0;
         bandList[1] = 1;
         bandList[2] = 2;
      }
      else
      {
         bandList[0] = 0;
         bandList[1] = 0;
         bandList[2] = 0;
      }
      setBandSelection(bandList);
   }
}
   
void rspfSingleImageChain::setToThreeBandsReverse()
{
   if ( m_handler.valid() )
   {
      std::vector<rspf_uint32> bandList(3);
      const rspf_uint32 BANDS = m_handler->getNumberOfInputBands();
      if(BANDS >= 3)
      {
         bandList[0] = 2;
         bandList[1] = 1;
         bandList[2] = 0;
      }
      else
      {
         bandList[0] = 0;
         bandList[1] = 0;
         bandList[2] = 0;
      }
      setBandSelection(bandList);
   }
}

void rspfSingleImageChain::setBandSelection(
   const std::vector<rspf_uint32>& bandList)
{
   if (!m_bandSelector)
   {
      addBandSelector();
   }
   m_bandSelector->setEnableFlag(true);
   m_bandSelector->setOutputBandList(bandList);
   if ( m_histogramRemapper.valid() )
   {
      m_histogramRemapper->initialize();
   }
}
rspfScalarType rspfSingleImageChain::getImageHandlerScalarType() const
{
   rspfScalarType result = RSPF_SCALAR_UNKNOWN;
   if ( m_handler.valid() )
   {
      result = m_handler->getOutputScalarType();
   }
   return result;
}

bool rspfSingleImageChain::openHistogram( rspfHistogramRemapper::StretchMode mode )
{
   bool result = false;
   rspfRefPtr<rspfImageHandler> ih = getImageHandler();
   if ( ih.valid() )
   {
      rspfRefPtr<rspfHistogramRemapper> hr = getHistogramRemapper();
      if ( hr.valid() )
      {
         rspfFilename f = ih->getFilenameWithThisExtension( rspfString("his") );
         if ( hr->openHistogram( f ) == true )
         {
            // Enable:
            hr->setEnableFlag(true);

            // Set the mode:
            hr->setStretchMode( mode );

            result = true;
         }
      }
   }
   return result;
}
