//**************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
//**************************************************************************************************
//  $Id$
#ifndef rspfImageHandlerMtAdaptor_HEADER
#define rspfImageHandlerMtAdaptor_HEADER 1

#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfCacheTileSource.h>
#include <OpenThreads/Thread>

//**************************************************************************************************
//! Intended mainly to provide a mechanism for mutex-locking access to a shared resource during
//! a getTile operation on an rspfImageHandler. This is needed for multi-threaded implementation.
//**************************************************************************************************
class RSPFDLLEXPORT rspfImageHandlerMtAdaptor : public rspfImageHandler
{
public:
   static const char* ADAPTEE_ID_KW;

   rspfImageHandlerMtAdaptor(rspfImageHandler* adaptee = 0);

   //! Sets the handler being adapted.
   void setAdaptee(rspfImageHandler* handler);

   //! Only an rspfImageHandler is allowed as input here.
   virtual bool canConnectMyInputTo(rspf_int32 i, const rspfConnectableObject* obj) const;

   //! Intercepts the getTile call intended for the adaptee and sets a mutex lock around the
   //! adaptee's getTile call.
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIpt& origin, rspf_uint32 resLevel=0);
   
   //! Intercepts the getTile call intended for the adaptee and sets a mutex lock around the
   //! adaptee's getTile call.
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect, rspf_uint32 resLevel=0);

   //! Intercepts the getTile call intended for the adaptee and sets a mutex lock around the
   //! adaptee's getTile call.
   virtual bool getTile(rspfImageData* result, rspf_uint32 resLevel=0);
   
   //! Method to save the state of an object to a keyword list.
   //! Return true if ok or false on error.
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   //! Method to the load (recreate) the state of an object from a keyword
   //! list.  Return true if ok or false on error.
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   // The following are virtuals in the base class. Implemented here as pass-through to adaptee
   virtual rspf_uint32    getNumberOfInputBands() const;
   virtual bool            isOpen() const;
   virtual bool            open();
   virtual rspf_uint32    getNumberOfLines(rspf_uint32 resLevel = 0) const;
   virtual rspf_uint32    getNumberOfSamples(rspf_uint32 resLevel = 0) const;
   virtual rspf_uint32    getImageTileWidth() const;
   virtual rspf_uint32    getImageTileHeight() const;
   virtual rspfString     getLongName()  const;
   virtual rspfString     getShortName() const;
   virtual void            close();
   virtual rspf_uint32    getNumberOfOutputBands() const;
   virtual rspf_uint32    getNumberOfDecimationLevels() const;
   virtual rspfScalarType getOutputScalarType() const;
   virtual rspf_uint32    getTileWidth() const;
   virtual rspf_uint32    getTileHeight() const;
   virtual rspf_float64   getMinPixelValue(rspf_uint32 band=0)const;
   virtual rspf_float64   getMaxPixelValue(rspf_uint32 band=0)const;
   virtual rspf_float64   getNullPixelValue(rspf_uint32 band=0)const;

   double       d_getTileT;

protected:
   //! Protected destructor forces using reference pointer for instantiation.
   virtual ~rspfImageHandlerMtAdaptor();

   rspfRefPtr<rspfImageHandler>    m_adaptedHandler;
   rspfRefPtr<rspfCacheTileSource> m_cache;
   mutable OpenThreads::Mutex        m_mutex;   

   bool                        d_useCache;
   bool                        d_useFauxTile;
   rspfRefPtr<rspfImageData> d_fauxTile;
   friend class rspfMtDebug;

TYPE_DATA
};

#endif









