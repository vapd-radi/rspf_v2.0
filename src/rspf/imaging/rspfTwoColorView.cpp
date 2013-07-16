//-------------------------------------------------------------------
// License:  LGPL.  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//-------------------------------------------------------------------
//  $Id$

#include <rspf/imaging/rspfTwoColorView.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfTwoColorView,
          "rspfTwoColorView" ,
          rspfImageCombiner);

rspfTwoColorView::rspfTwoColorView()
   :
   rspfImageCombiner(0, 2, 0, true, false) ,
   m_byPassFlag(true),
   m_nativeFlag(false),
   m_newInput(0),
   m_oldInput(0),
   m_newInputBandIndex(0),
   m_oldInputBandIndex(0),
   m_redSource(rspfTwoColorView::OLD),
   m_grnSource(rspfTwoColorView::NEW),
   m_bluSource(rspfTwoColorView::NEW)
{
}

rspf_uint32 rspfTwoColorView::getNumberOfOutputBands() const
{
   if(m_byPassFlag||!isSourceEnabled())
   {
      return rspfImageCombiner::getNumberOfOutputBands();
   }
   return 3;
}

rspfScalarType rspfTwoColorView::getOutputScalarType() const
{
   if(m_byPassFlag||!isSourceEnabled())
   {
      return rspfImageCombiner::getOutputScalarType();
   }
   return RSPF_UINT8;
}

void rspfTwoColorView::setBandIndexMapping(
   rspf_uint32 oldInputBandIndex,
   rspf_uint32 newInputBandIndex,
   rspfTwoColorMultiViewOutputSource redOutputSource,
   rspfTwoColorMultiViewOutputSource grnOutputSource,
   rspfTwoColorMultiViewOutputSource bluOutputSource)
{
   m_oldInputBandIndex = oldInputBandIndex;
   m_newInputBandIndex = newInputBandIndex;
   m_redSource = redOutputSource;
   m_grnSource = grnOutputSource;
   m_bluSource = bluOutputSource;
}

double rspfTwoColorView::getNullPixelValue(rspf_uint32 band)const
{
   if(m_byPassFlag||!isSourceEnabled())
   {
      return rspfImageCombiner::getNullPixelValue(band);
   }
   return 0;
}

double rspfTwoColorView::getMinPixelValue(rspf_uint32 band)const
{
   if(m_byPassFlag||!isSourceEnabled())
   {
      return rspfImageCombiner::getMinPixelValue(band);
   }
   return 1;
}

double rspfTwoColorView::getMaxPixelValue(rspf_uint32 band)const
{
   if(m_byPassFlag||!isSourceEnabled())
   {
      return rspfImageCombiner::getMaxPixelValue(band);
   }
   return 256;
}

rspfRefPtr<rspfImageData> rspfTwoColorView::getTile(const rspfIrect& rect,
                                                       rspf_uint32 resLevel)
{
   rspf_uint32 tileIdx = 0;
   if(m_byPassFlag||!isSourceEnabled())
   {
      return getNextTile(tileIdx, 0, rect, resLevel);
   }
   if(!m_twoColorTile.valid())
   {
      allocate();
   }
   if(!m_twoColorTile.valid())
   {
      return m_twoColorTile;
   }
   m_twoColorTile->setImageRectangle(rect);
   m_twoColorTile->makeBlank();
   
   rspfRefPtr<rspfImageData> newData = m_newInput->getTile(rect, resLevel);
   rspfRefPtr<rspfImageData> oldData = m_oldInput->getTile(rect, resLevel);

   runAlgorithm(newData.get(), oldData.get());

   m_twoColorTile->validate();

   return m_twoColorTile;
}
void rspfTwoColorView::runAlgorithm(rspfImageData* newData, rspfImageData* oldData)
{
   if(m_nativeFlag)
   {
      runNative8(newData, oldData);
   }
   else 
   {
      runNorm(newData, oldData);
   }
}

void rspfTwoColorView::runNative8(rspfImageData* newData, rspfImageData* oldData)
{
   if ( newData && oldData && m_twoColorTile.valid() &&
        ( m_twoColorTile->getNumberOfBands() == 3 ) )
   {
      // old, new, red, green, blue buffers...
      rspf_uint8* o = static_cast<rspf_uint8*>( oldData->getBuf(m_oldInputBandIndex) );
      rspf_uint8* n = static_cast<rspf_uint8*>( newData->getBuf(m_newInputBandIndex) );
      rspf_uint8* r = static_cast<rspf_uint8*>( m_twoColorTile->getBuf(0) );
      rspf_uint8* g = static_cast<rspf_uint8*>( m_twoColorTile->getBuf(1) );
      rspf_uint8* b = static_cast<rspf_uint8*>( m_twoColorTile->getBuf(2) );

      if ( o && n && r && g && b )
      {
         // Assuming null pix of 0 for 8 bit.
         const rspf_uint8 MP = 1;
         const rspf_uint8 NP = 0;

         rspf_uint8 newPix = 0;
         rspf_uint8 oldPix = 0;
         
         const rspf_uint32 MAX_IDX = m_twoColorTile->getSizePerBand();
         
         for(rspf_uint32 idx = 0; idx < MAX_IDX; ++idx)
         {
            if( ( *n == NP ) && ( *o == NP ) )
            {
               // Both inputs null, set all outputs null.
               *r = NP;
               *b = NP;
               *g = NP;
            }
            else 
            {
               // At least one input is not null.
               newPix = (*n != NP) ? *n : MP;
               oldPix = (*o != NP) ? *o : MP;

               // Set red, OLD is default so check first:
               if ( m_redSource == rspfTwoColorView::OLD )
               {
                  *r = oldPix;
               }
               else if ( m_redSource == rspfTwoColorView::NEW )
               {
                  *r = newPix;
               }
               else
               {
                  *r = MP;
               }

               // Set green, NEW is default so check first:
               if ( m_grnSource == rspfTwoColorView::NEW )
               {
                  *g = newPix;
               }
               else if ( m_grnSource == rspfTwoColorView::OLD )
               {
                  *g = oldPix;
               }
               else
               {
                  *g = MP;
               }

               // Set blue, NEW is default so check first:
               if ( m_grnSource == rspfTwoColorView::NEW )
               {
                  *b = newPix;
               }
               else if ( m_grnSource == rspfTwoColorView::OLD )
               {
                  *b = oldPix;
               }
               else
               {
                  *b = MP;
               }
            }

            // Next pixel:
            ++n;
            ++o;
            ++r;
            ++g;
            ++b;
         }
      }
   }
}

void rspfTwoColorView::runNorm(rspfImageData* newData, rspfImageData* oldData)
{
   if ( newData && oldData && m_twoColorTile.valid() &&
        ( m_twoColorTile->getNumberOfBands() == 3 ) )
   {
      const rspf_uint32 MAX_IDX = m_twoColorTile->getSizePerBand();
      
      // Buffers for normalized oldData and newData tiles.
      std::vector<rspf_float32> oldDataBuffer(MAX_IDX);
      std::vector<rspf_float32> newDataBuffer(MAX_IDX);

      // old and new input buffers.
      rspf_float32* o = &oldDataBuffer.front();      
      rspf_float32* n = &newDataBuffer.front();

      // Normalize/copy to buffers.
      newData->copyTileBandToNormalizedBuffer(m_newInputBandIndex, n);
      oldData->copyTileBandToNormalizedBuffer(m_oldInputBandIndex, o);
      
      // Get the output buffers.
      rspf_uint8* r = static_cast<rspf_uint8*>( m_twoColorTile->getBuf(0) );
      rspf_uint8* g = static_cast<rspf_uint8*>( m_twoColorTile->getBuf(1) );
      rspf_uint8* b = static_cast<rspf_uint8*>( m_twoColorTile->getBuf(2) );

      if ( o && n && r && g && b )
      {
         // Assuming null pix of 0 for 8 bit.
         const rspf_uint8 MP = 1;
         const rspf_uint8 NP = 0;

         rspf_uint8   newPix = 0;
         rspf_uint8   oldPix = 0;
         rspf_float32 tmpPix = 0.0;
         
         const rspf_uint32 MAX_IDX = m_twoColorTile->getSizePerBand();
         
         for(rspf_uint32 idx = 0; idx < MAX_IDX; ++idx)
         {
            if( ( *n == NP ) && ( *o == NP ) )
            {
               // Both inputs null, set all outputs null.
               *r = NP;
               *b = NP;
               *g = NP;
            }
            else 
            {
               // At least one input is not null.
               
               // Set the newPix:
               if ( *n != NP )
               {
                  // Un-normalize:
                  tmpPix = (*n) * 255.0;

                  // Clamp to min/max.
                  tmpPix = (tmpPix <= 255.0) ? ( (tmpPix >= 1.0) ? tmpPix : 1.0) : 255.0;

                  // Copy
                  newPix = static_cast<rspf_uint8>( tmpPix );
               }
               else
               {
                  newPix = MP;
               }

               // Set the oldPix:
               if ( *o != NP )
               {
                  // Un-normalize:
                  tmpPix = (*o) * 255.0;

                  // Clamp to min/max.
                  tmpPix = (tmpPix <= 255.0) ? ( (tmpPix >= 1.0) ? tmpPix : 1.0) : 255.0;

                  // Copy
                  oldPix = static_cast<rspf_uint8>( tmpPix );
               }
               else
               {
                  oldPix = MP;
               }

               // Set red, OLD is default so check first:
               if ( m_redSource == rspfTwoColorView::OLD )
               {
                  *r = oldPix;
               }
               else if ( m_redSource == rspfTwoColorView::NEW )
               {
                  *r = newPix;
               }
               else
               {
                  *r = MP;
               }

               // Set green, NEW is default so check first:
               if ( m_grnSource == rspfTwoColorView::NEW )
               {
                  *g = newPix;
               }
               else if ( m_grnSource == rspfTwoColorView::OLD )
               {
                  *g = oldPix;
               }
               else
               {
                  *g = MP;
               }

               // Set blue, NEW is default so check first:
               if ( m_grnSource == rspfTwoColorView::NEW )
               {
                  *b = newPix;
               }
               else if ( m_grnSource == rspfTwoColorView::OLD )
               {
                  *b = oldPix;
               }
               else
               {
                  *b = MP;
               }
            }

            // Next pixel:
            ++n;
            ++o;
            ++r;
            ++g;
            ++b;
         }
      }
   }
}

void rspfTwoColorView::allocate()
{
   m_twoColorTile = rspfImageDataFactory::instance()->create(this, this);
   if(m_twoColorTile.valid())
   {
      m_twoColorTile->initialize();
   }
}


void rspfTwoColorView::initialize()
{
   rspfImageCombiner::initialize();
   m_newInput = 0;
   m_oldInput = 0;
   m_twoColorTile = 0;
   m_nativeFlag = false;
   m_byPassFlag = false;

   if(getNumberOfInputs() < 2)
   {
      m_byPassFlag = true;
   }
   else 
   {
      m_oldInput = dynamic_cast<rspfImageSource*>( getInput(0) );
      m_newInput = dynamic_cast<rspfImageSource*>( getInput(1) );

      //---
      // Range check band selection. This can be set from setBandIndexMapping method which
      // does no error checking because inputs may not be set.
      //----
      if ( m_oldInput.valid() )
      {
         if ( m_oldInputBandIndex >= m_oldInput->getNumberOfOutputBands() )
         {
            m_oldInputBandIndex = 0;
         }
      }
      
      if ( m_newInput.valid() )
      {
         if ( m_newInputBandIndex >= m_newInput->getNumberOfOutputBands() )
         {
            m_newInputBandIndex = 0;
         }
      }
         
      if(!m_newInput||!m_oldInput)
      {
         m_byPassFlag = true;
      }
      else
      {
         if((m_newInput->getOutputScalarType() == RSPF_UINT8)&&
            (m_oldInput->getOutputScalarType() == RSPF_UINT8))
         {
            m_nativeFlag = true;
         }
      }
   }
}
