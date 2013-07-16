#ifndef rspfMtDebug_HEADER
#define rspfMtDebug_HEADER

//*************************************************************************************************
//! For debugging purposes. To be removed with final release:
//*************************************************************************************************
class RSPFDLLEXPORT rspfMtDebug
{
   friend class rspfMultiThreadSequencer;
   friend class rspfImageChainMtAdaptor;
   friend class rspfImageHandlerMtAdaptor;

public:
   rspfMtDebug() 
      : handlerCacheEnabled (false), 
      handlerUseFauxTile (false),
      chainDebugEnabled (false),
      chainSharedHandlers (false),
      seqDebugEnabled (false),
      seqTimedBlocksDt (0),
      seqMetricsEnabled (false),
      maxTileCacheSize (0) { m_instance = this; }

   static rspfMtDebug* instance()
   { 
      if (m_instance == NULL) 
         m_instance = new rspfMtDebug; 
      return m_instance; 
   }
   
   bool handlerCacheEnabled;
   bool handlerUseFauxTile;
   bool chainDebugEnabled;
   bool chainSharedHandlers;
   bool seqDebugEnabled;
   rspf_uint32 seqTimedBlocksDt;
   bool seqMetricsEnabled;
   rspf_uint32 maxTileCacheSize;

private:
   static rspfMtDebug* m_instance;
};

#endif
