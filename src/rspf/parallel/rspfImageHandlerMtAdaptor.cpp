//**************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
//! Intended mainly to provide a mechanism for mutex-locking access to a shared resource during
//! a getTile operation on an rspfImageHandler. This is needed for multi-threaded implementation.
// 
//**************************************************************************************************
//  $Id$
#include <rspf/parallel/rspfImageHandlerMtAdaptor.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/parallel/rspfMtDebug.h>
#include <rspf/base/rspfTimer.h>

RTTI_DEF1(rspfImageHandlerMtAdaptor, "rspfImageHandlerMtAdaptor", rspfImageHandler);

const char* rspfImageHandlerMtAdaptor::ADAPTEE_ID_KW = "adaptee_id";

//**************************************************************************************************
// Constructor
//**************************************************************************************************
rspfImageHandlerMtAdaptor::rspfImageHandlerMtAdaptor(rspfImageHandler* adaptee)
   :  d_getTileT (0),
      m_adaptedHandler (0),
      m_cache (0),
      d_useCache (false),
      d_useFauxTile (false)
{
   //###### DEBUG ############
   rspfMtDebug* mt_debug = rspfMtDebug::instance();
   d_useCache = mt_debug->handlerCacheEnabled;
   d_useFauxTile = mt_debug->handlerUseFauxTile;
   //###### END DEBUG ############

   setAdaptee(adaptee);
}

//**************************************************************************************************
// Destructor
//**************************************************************************************************
rspfImageHandlerMtAdaptor::~rspfImageHandlerMtAdaptor()
{
   m_adaptedHandler = 0;
   m_cache = 0;
}

//**************************************************************************************************
//! Sets the handler being adapted.
//**************************************************************************************************
void rspfImageHandlerMtAdaptor::setAdaptee(rspfImageHandler* handler)
{
   m_adaptedHandler = handler;
   if (handler == NULL)
      return;

   // Fetch the adaptee's output list and make it our own:
   ConnectableObjectList output_list = handler->getOutputList();

   if (d_useCache)
   {
      // Create the cache and connect this adaptor as its output:
      m_cache = new rspfCacheTileSource;
      m_cache->connectMyOutputTo(this, true, false);
      m_cache->changeOwner(this);
      m_cache->connectMyOutputTo(this, true, false);
      handler->disconnectMyOutputs(output_list, true, false);
      handler->connectMyOutputTo(m_cache.get(), true, true);
   }
   else
   {
      handler->disconnectMyOutputs(output_list, true, false);
      handler->connectMyOutputTo(this, true, false);
   }

   // Finally connect the adaptee's outputs to this and fire connection events:
   connectMyOutputTo(output_list, true, true);
   handler->changeOwner(this);

   if (d_useFauxTile)
   {
      d_fauxTile = (rspfImageData*) handler->getTile(rspfIpt(0,0), 0)->dup();
      //d_fauxTile = new rspfImageData(this, 
      //                                handler->getOutputScalarType(), 
      //                                handler->getNumberOfOutputBands(),
      //                                handler->getTileWidth(),
      //                                handler->getTileHeight());
      //d_fauxTile->fill(128.0);
   }
}

//**************************************************************************************************
//! Only an rspfImageHandler is allowed as input here.
//**************************************************************************************************
bool rspfImageHandlerMtAdaptor::canConnectMyInputTo(rspf_int32 inputIndex,
                                                     const rspfConnectableObject* obj) const
{
   const rspfImageHandler* h = dynamic_cast<const rspfImageHandler*>(obj);
   if ((inputIndex == 0) && (h != NULL))
      return true;
   return false;
}


//**************************************************************************************************
//! Intercepts the getTile call intended for the adaptee and sets a mutex lock around the
//! adaptee's getTile call.
//**************************************************************************************************
rspfRefPtr<rspfImageData> 
   rspfImageHandlerMtAdaptor::getTile(const rspfIpt& origin, rspf_uint32 rLevel)
{
   if (!m_adaptedHandler.valid())
      return NULL;

   // Establish tile rect to call overloaded getTile(tile_rect):
   rspf_uint32 h = m_adaptedHandler->getTileHeight();
   rspf_uint32 w = m_adaptedHandler->getTileWidth();
   rspfIpt lr (origin.x + w - 1, origin.y + h - 1);
   rspfIrect tile_rect (origin, lr);

   // Need to unlock to prevent freezing in the called getTile():
   return getTile(tile_rect, rLevel);
}

//**************************************************************************************************
//! Intercepts the getTile call intended for the adaptee and sets a mutex lock around the
//! adaptee's getTile call.
//**************************************************************************************************
rspfRefPtr<rspfImageData>  
   rspfImageHandlerMtAdaptor::getTile(const rspfIrect& tile_rect, rspf_uint32 rLevel)
{
   if (d_useFauxTile)
   {
      rspfRefPtr<rspfImageData> ftile = new rspfImageData(*(d_fauxTile.get()));
      ftile->setOrigin(tile_rect.ul());
      return ftile;
   }

   if (!m_adaptedHandler.valid())
      return NULL;

   // The sole purpose of the adapter is this mutex lock around the actual handler getTile:
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);

   rspfRefPtr<rspfImageData> tile = new rspfImageData();
   rspfRefPtr<rspfImageData> temp_tile = 0;
   double dt = rspfTimer::instance()->time_s();
   if (d_useCache)
      temp_tile = m_cache->getTile(tile_rect, rLevel);
   else
      temp_tile = m_adaptedHandler->getTile(tile_rect, rLevel);
   d_getTileT += rspfTimer::instance()->time_s() - dt;

   // We make our own instance of a tile and copy the adaptee's returned tile to it. This avoids
   // the product tile from changing while being processed up the chain. The adaptee's tile can
   // change as soon as the mutex lock is released:
   
   if (temp_tile.valid())
      *tile = *(temp_tile.get());
   else
      tile = NULL;
   return tile;
}

//**************************************************************************************************
//! Intercepts the getTile call intended for the adaptee and sets a mutex lock around the
//! adaptee's getTile call.
//**************************************************************************************************
bool rspfImageHandlerMtAdaptor::getTile(rspfImageData* tile, rspf_uint32 rLevel)
{
   if ((!m_adaptedHandler.valid()) || (tile == NULL))
      return false;

   // The sole purpose of the adapter is this mutex lock around the actual handler getTile:
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);

   // This is effectively a copy of rspfImageSource::getTile(rspfImageData*). It is reimplemented 
   // here to save two additional function calls:
   tile->ref();
   bool status = true;
   rspfIrect tile_rect = tile->getImageRectangle();

   rspfRefPtr<rspfImageData> temp_tile = 0;
   if (d_useCache)
      temp_tile = m_cache->getTile(tile_rect, rLevel);
   else
      temp_tile = m_adaptedHandler->getTile(tile_rect, rLevel);
   
   if (temp_tile.valid())
      *tile = *(temp_tile.get());
   else
      status = false;
   tile->unref();
   
   return status;
}

//**************************************************************************************************
//! Method to save the state of an object to a keyword list.
//! Return true if ok or false on error.
//**************************************************************************************************
bool rspfImageHandlerMtAdaptor::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   if (!m_adaptedHandler.valid())
      return false;
   
   // Skip the rspfImageHandler::saveState() since it is not necessary here:
   rspfImageSource::saveState(kwl, prefix);

   kwl.add(prefix, ADAPTEE_ID_KW, m_adaptedHandler->getId().getId());

   return true;
}

//**************************************************************************************************
//! Method to the load (recreate) the state of an object from a keyword
//! list.  Return true if ok or false on error.
//**************************************************************************************************
bool rspfImageHandlerMtAdaptor::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   m_adaptedHandler = 0;

   // Skip the rspfImageHandler::loadState() since it is not necessary here:
   if (!rspfImageSource::loadState(kwl, prefix))
      return false;

   // The adaptee's ID at least will be in the KWL:
   rspfString value = kwl.find(prefix, ADAPTEE_ID_KW);
   if (value.empty())
      return false;
   
   return true;
}

//**************************************************************************************************
// The following are virtuals in the base class. Implemented here as pass-through to adaptee
//**************************************************************************************************
rspf_uint32 rspfImageHandlerMtAdaptor::getNumberOfInputBands() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getNumberOfInputBands();
   return 0;
}

bool rspfImageHandlerMtAdaptor::isOpen() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->isOpen();
   return false;
}

bool rspfImageHandlerMtAdaptor::open()
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->open();
   return false;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getNumberOfLines(rspf_uint32 resLevel) const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getNumberOfLines(resLevel);
   return 0;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getNumberOfSamples(rspf_uint32 resLevel) const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getNumberOfSamples(resLevel);
   return 0;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getImageTileWidth() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getImageTileWidth();
   return 0;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getImageTileHeight() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getImageTileHeight();
   return 0;
}

rspfString rspfImageHandlerMtAdaptor::getLongName() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getLongName();
   return rspfString();
}

rspfString rspfImageHandlerMtAdaptor::getShortName() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getShortName();
   return rspfString();
}

void rspfImageHandlerMtAdaptor::close()
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->close();
}

rspf_uint32 rspfImageHandlerMtAdaptor::getNumberOfOutputBands() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getNumberOfOutputBands();
   return 0;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getNumberOfDecimationLevels() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getNumberOfDecimationLevels();
   return 0;
}

rspfScalarType rspfImageHandlerMtAdaptor::getOutputScalarType() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getOutputScalarType();
   return RSPF_SCALAR_UNKNOWN;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getTileWidth() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getTileWidth();
   return 0;
}

rspf_uint32 rspfImageHandlerMtAdaptor::getTileHeight() const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getTileHeight();
   return 0;
}

rspf_float64 rspfImageHandlerMtAdaptor::getMinPixelValue(rspf_uint32 band) const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getMinPixelValue(band);
   return 0.0;
}

rspf_float64 rspfImageHandlerMtAdaptor::getMaxPixelValue(rspf_uint32 band) const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getMaxPixelValue(band);
   return 0.0;
}

rspf_float64 rspfImageHandlerMtAdaptor::getNullPixelValue(rspf_uint32 band) const
{
   if (m_adaptedHandler.valid())
      return m_adaptedHandler->getNullPixelValue(band);
   return 0.0;
}

