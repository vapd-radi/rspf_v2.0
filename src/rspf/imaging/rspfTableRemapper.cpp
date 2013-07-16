//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Table remapper class definition.
//
// Takes input tile, remaps it through a table, then output tile in the desired
// scalar type.
//
// Two modes one that works on native remap tables, that being of the same
// scalar type (like rspf_uint8) of the input connection, and another that
// uses a normalized remap table (more scalar independent).
//
//*************************************************************************
// $Id: rspfTableRemapper.cpp 21787 2012-09-30 21:27:09Z gpotts $

#include <rspf/imaging/rspfTableRemapper.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfTableRemapper, "rspfTableRemapper", rspfImageSourceFilter)

static rspfTrace traceDebug("rspfTableRemapper:debug");

static const char* TABLE_TYPE[] = { "UNKNOWN", "NATIVE", "NORMALIZED" };

rspfTableRemapper::rspfTableRemapper()
   :
      rspfImageSourceFilter(),  // base class
      theTile(0),
      theTmpTile(0),
      theNormBuf(0),
      theTableBinCount(0),
      theTableBandCount(0),
      theTableType(rspfTableRemapper::UKNOWN),
      theInputScalarType(RSPF_SCALAR_UNKNOWN),
      theOutputScalarType(RSPF_SCALAR_UNKNOWN)
{
   //***
   // Set the base class "theEnableFlag" to off since no adjustments have been
   // made yet.
   //***
   //disableSource();
}

rspfTableRemapper::~rspfTableRemapper()
{
   destroy();
}

void rspfTableRemapper::destroy()
{
   if (theNormBuf)
   {
      delete [] theNormBuf;
      theNormBuf = 0;
   }
   theTmpTile = 0;
   theTile    = 0;
}

void rspfTableRemapper::initialize()
{
   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   rspfImageSourceFilter::initialize();
   destroy();
   if (theInputConnection)
   {
      // Since we override "getOutputScalarType" make sure something's set.
      if (theOutputScalarType == RSPF_SCALAR_UNKNOWN)
      {
         theOutputScalarType = theInputConnection->getOutputScalarType();
      }

      if (theTile.valid())
      {
         //if ( theTile->getNumberOfBands() !=
         //     theInputConnection->getNumberOfOutputBands() )
        // {
            // Wipe everything slick. The next getTile will call allocate.
        //    destroy();
        // }
      }
   }

   // Nothing else to do for this...
}

void rspfTableRemapper::allocate(const rspfIrect& rect)
{
   //---
   // It's either first getTile or we've had either a resize, so allocate
   // needed stuff...
   //---
   destroy();

   if (theInputConnection)
   {
      rspf_uint32 width  = rect.width();
      rspf_uint32 height = rect.height();
      theInputScalarType  = theInputConnection->getOutputScalarType();
    
      theTile =  rspfImageDataFactory::instance()->create(this,this);
      theTile->setImageRectangle(rect); 
     // theTile =
     //    rspfImageDataFactory::instance()->create(this,
     //                                              getOutputScalarType(),
     //                                              getNumberOfOutputBands(),
     //                                              width,x
     //                                              height);
      theTile->initialize();


      if (theInputScalarType !=  getOutputScalarType() &&
          theTableType == rspfTableRemapper::NATIVE)
      {
         //---
         // We need a temp tile so we can do two remaps
         // inputTile->remapTable->tmpTile then
         // tmpTile->normalize->theTile(unnormalize to new scalar)...
         //---
         theTmpTile 
         = rspfImageDataFactory::instance()->create(this,
                                                     theInputScalarType,
                                                     getNumberOfOutputBands(),
                                                     width,
                                                     height);
         theTmpTile->setMinPix(theTile->getMinPix(), theTile->getNumberOfBands());
         theTmpTile->initialize();

      }

      if (theTableType == rspfTableRemapper::NORMALIZED ||
          theInputScalarType != theOutputScalarType)
      {
         theNormBuf = new rspf_float64[theTile->getSize()];
      }
   }
}

rspfRefPtr<rspfImageData> rspfTableRemapper::getTile(
   const rspfIrect& tile_rect, rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> result = 0;
   
   if(theInputConnection)
   {
      // Fetch tile from pointer from the input source.
      result = theInputConnection->getTile(tile_rect, resLevel);
      if (theEnableFlag&&result.valid())
      {  
         // Get its status of the input tile.
         rspfDataObjectStatus tile_status = result->getDataObjectStatus();
         
         // Check for remap bypass:
         if ( (tile_status != RSPF_NULL) &&
              (tile_status != RSPF_EMPTY) && theTable.size() )
         {

            // OK we have an input tile... and it's not null or empty.
            if(!theTile)
            {
               allocate(tile_rect);
            }
            if (theTile.valid())
            {
               theTile->setImageRectangle(tile_rect);
               if(theTmpTile.valid()) // not mandatory for all modes.
               {
                  theTmpTile->setImageRectangle(tile_rect);
               }   
               // Think things are good.  Do the real work...
               if (theTableType == rspfTableRemapper::NATIVE)
               {
                  // Most efficient case...
                  remapFromNativeTable(result);
               }
               else
               {
                  remapFromNormalizedTable(result);
               }
              
               theTile->validate();
               result = theTile;
            }
         }
      }
   }
   return result;
}

void rspfTableRemapper::remapFromNativeTable(
   rspfRefPtr<rspfImageData>& inputTile)
{
   switch (theOutputScalarType)
   {
      case RSPF_UINT8:
      {
         remapFromNativeTable(rspf_uint8(0), inputTile);
         break;
      }
      
      case RSPF_USHORT11:
      case RSPF_USHORT16:
      {
         remapFromNativeTable(rspf_uint16(0), inputTile);
         break;
      }
      
      case RSPF_SSHORT16:
      {
         remapFromNativeTable(rspf_sint16(0), inputTile);
         break;
      }
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT:
      {
         remapFromNativeTable(rspf_float32(0), inputTile);
         break;
      }
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_DOUBLE:
      {
         remapFromNativeTable(rspf_float64(0), inputTile);
         break;
      }
      
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
         // Shouldn't hit this.
            rspfNotify(rspfNotifyLevel_WARN)
            << "rspfTableRemapper::remapFromNativeTable"
            << "\nOutput scalar type is RSPF_SCALAR_UNKNOWN!\n";
         }
         break;
      }

   } // End of "switch (theTableType)"
}

template <class T> void rspfTableRemapper::remapFromNativeTable(
   T /* dummy */,
   rspfRefPtr<rspfImageData>& inputTile)
{
   
   const rspf_uint32 BANDS = theTile->getNumberOfBands();
   const rspf_uint32 PPB   = theTile->getSizePerBand(); // pixels per band

   //---
   // NOTE:
   // If (theTableBandCount == one) that means we only have one band; or,
   // the user wanted to remap all bands using the same table...
   //---
   const rspf_uint32 BAND_OFFSET =
      (theTableBandCount != 1) ? theTableBinCount: 0;

   const T* rt = reinterpret_cast<T*>(&theTable.front()); // remap table (rt)
   rspfRefPtr<rspfImageData> id;
   if (theInputScalarType == theOutputScalarType)
   {
      id = theTile;
   }
   else
   {
      id = theTmpTile;
   }
   
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const T NULL_PIX = static_cast<T>(id->getNullPix(band));
      //const T MIN_PIX  = static_cast<T>(id->getMinPix(band));

      const T* s = static_cast<T*>(inputTile->getBuf(band)); // source (s)
      T*       d = static_cast<T*>(id->getBuf(band));   // destination (d)

      for (rspf_uint32 pixel = 0; pixel < PPB; ++pixel)
      {
         //---
         // Get the source pixel and use it as an index to the table.
         // Note Null is always the "0" index, min always the "1".
         //---
         T p = s[pixel];
         if (p == NULL_PIX)  // Null is not always zero (dted).
         {
            d[pixel] = NULL_PIX;
           // p = 0;
         }
         else
         {
//         else
//         {
//            p = (p+1-MIN_PIX);  // If min == 1 we want that to map to index 1.
//         }

            rspf_uint32 table_index = static_cast<rspf_uint32>(p);
            
            //---
            // If within range use to index the remap table; else, null.
            // Note:
            // There is no min, max range checking on value retrieved from table.
            // Range checking should be performed when the table is built.
            //---
            if(table_index < theTableBinCount)
            {
               d[pixel] = rt[table_index];
            }
//            else if(table_index < 0)
//            {
//               if(theTableBinCount > 1)
//               {
//                  d[pixel] = rt[1];
//               }
//               else
//               {
//                  d[pixel] = table_index;
 //              }
 //           }
            else
            {
               if(theTableBinCount>0)
               {
                  d[pixel] = rt[theTableBinCount-1];
               }
               else
               {
                  d[pixel] = table_index;
              }
            }
         }
      }

      rt += BAND_OFFSET; // Go to next band in the table.
   }

   if (theInputScalarType != theOutputScalarType)
   {
      // Normalize and copy the temp tile to a buffer.
      id->copyTileToNormalizedBuffer(theNormBuf);

      // Un-normalize and copy the buffer to the destination tile.
      theTile->copyNormalizedBufferToTile(theNormBuf);
   }
}

void rspfTableRemapper::remapFromNormalizedTable(
   rspfRefPtr<rspfImageData>& inputTile)
{
   if (!theNormBuf) // First time...
   {
      theNormBuf = new rspf_float64[theTile->getSize()];
   }
   
   // Normalize and copy the source tile to a buffer.
   inputTile->copyTileToNormalizedBuffer(theNormBuf);

   const rspf_uint32 BANDS = theTile->getNumberOfBands();
   const rspf_uint32 PPB   = theTile->getSizePerBand(); // pixels per band

   //---
   // NOTE:
   // If (theTableBandCount == one) that means we only have one band; or,
   // the user wanted to remap all bands using the same table...
   //---
   const rspf_uint32 BAND_OFFSET = (theTableBandCount == 1) ? 0 : PPB;

   // remap table (rt)
   rspf_float64* rt = reinterpret_cast<rspf_float64*>(&theTable.front()); 

   rspf_float64* buf = theNormBuf;
   rspf_float64 p = 0.0;
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      for (rspf_uint32 pixel = 0; pixel < PPB; ++pixel)
      {
         // Get the source pixel...
         rspf_uint32 idx
            = static_cast<rspf_uint32>(buf[pixel]*theTableBinCount+0.5);

         if((idx < theTableBinCount))
         {
            // If within range use to index the remap table; else, null.
            p = (idx < theTableBinCount) ? rt[idx] : 0.0;
         }
//         else if(idx < 0)
//         {
//            p = 0.0;
//         }
         else 
         {
            p = 1.0;
         }
         // Range check (in case table bad) and assign to destination pixel.
         buf[pixel] = (p >= 0.0) ? ( (p <=1.0) ? p : 1) : 0.0;
      }

      // Go to next band.
      rt += BAND_OFFSET;
      buf += PPB;
   }
   
   // Un-normalize and copy the buffer to the destination tile.
   theTile->copyNormalizedBufferToTile(theNormBuf);
}

rspfScalarType rspfTableRemapper::getOutputScalarType() const
{
   if (theOutputScalarType != RSPF_SCALAR_UNKNOWN)
   {
      return theOutputScalarType;
   }

   return rspfImageSourceFilter::getOutputScalarType();
}

void rspfTableRemapper::setTable(const std::vector<rspf_uint8>& table,
                                  rspf_uint32 table_bin_count,
                                  rspf_uint32 table_band_count,
                                  RemapTableType table_type,
                                  rspfScalarType output_scalar_type)
{
   // Start with a clean slate...
   destroy();
   
   theTable            = table;
   theTableBinCount    = table_bin_count;
   theTableBandCount   = table_band_count;
   theTableType        = table_type;
   theOutputScalarType = output_scalar_type;
}

bool rspfTableRemapper::loadState(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   // Look for scalar type keyword.
   rspf_int32 st = rspfScalarTypeLut::instance()->
      getEntryNumber(kwl, prefix, true);

   // Lookup table returns -1 if not found so check return...
   if ( (st != -1) && (st != RSPF_SCALAR_UNKNOWN) )
   {
      theOutputScalarType = static_cast<rspfScalarType>(st);
   }

   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfTableRemapper::saveState(rspfKeywordlist& kwl,
                                   const char* prefix) const
{
   // Save the output scalar type.
   kwl.add(prefix,
           rspfKeywordNames::SCALAR_TYPE_KW,
           rspfScalarTypeLut::instance()->
           getEntryString(theOutputScalarType).c_str(),
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

ostream& rspfTableRemapper::print(ostream& os) const
{
   rspfScalarTypeLut* sl = rspfScalarTypeLut::instance();

   os << "\nrspfTableRemapper::print:"
      << "\ntheTableBinCount:    " << theTableBinCount
      << "\ntheTableBandCount:   " << theTableBandCount
      << "\ntheTableType:        " << TABLE_TYPE[theTableType]
      << "\ntheInputScalarType:  " << (*sl)[theInputScalarType]
      << "\ntheOutputScalarType: " << (*sl)[theOutputScalarType]
      << endl;
   
   if (theTile.valid())
   {
      os << "theTile:\n" << *theTile << endl;
   }
   if (theTmpTile.valid())
   {
      os << "theTmpTile:\n" << *theTmpTile << endl;
   }

   switch (theOutputScalarType)
   {
      case RSPF_UCHAR:
      {
         dumpTable(rspf_uint8(0), os);
         break;
      }
      
      case RSPF_USHORT11:
      case RSPF_USHORT16:
      {
         dumpTable(rspf_uint16(0), os);
         break;
      }
      
      case RSPF_SSHORT16:
      {
         dumpTable(rspf_sint16(0), os);
         break;
      }
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT:
      {
         dumpTable(rspf_float32(0), os);
         break;
      }
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_DOUBLE:
      {
         dumpTable(rspf_float64(0), os);
         break;
      }
      
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
         // Shouldn't hit this.
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfTableRemapper::print RSPF_SCALAR_UNKNOWN!\n";
         }
         break;
      }

   } // End of "switch (theTableType)"

   return os;
}

template <class T> void rspfTableRemapper::dumpTable(T /*dummy*/,
                                                      ostream& os) const
{
   if (theTable.empty() || !theTableBinCount || !theTableBandCount)
   {
      return;
   }

   const T* table = reinterpret_cast<const T*>(&theTable.front());

   rspf_uint32 table_index = 0;
   for (rspf_uint32 band = 0; band < theTableBandCount; ++band)
   {
      for (rspf_uint32 index = 0; index < theTableBinCount; ++index)
      {
         os << "tabel[" << band << "][" << index << "]:  "
            << static_cast<int>(table[table_index]) << endl;
         ++table_index;
      }
   }
}


ostream& operator<<(ostream& os, const rspfTableRemapper& tr)
{
   return tr.print(os);
}

// Private to disallow use...
rspfTableRemapper::rspfTableRemapper(const rspfTableRemapper&)
{
}

// Private to disallow use...
rspfTableRemapper& rspfTableRemapper::operator=(const rspfTableRemapper&)
{
   return *this;
}
