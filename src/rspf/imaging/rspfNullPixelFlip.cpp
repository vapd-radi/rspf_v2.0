//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// 
//*******************************************************************
//  $Id$

#include <rspf/imaging/rspfNullPixelFlip.h>
#include <rspf/imaging/rspfImageData.h>

RTTI_DEF1(rspfNullPixelFlip, "rspfNullPixelFlip", rspfImageSourceFilter);
rspfNullPixelFlip::rspfNullPixelFlip()
:m_clipMode(ClipMode_BOUNDING_RECT),
m_replacementType(ReplacementType_MINPIX)
{
}

void rspfNullPixelFlip::initialize()
{
}

rspfRefPtr<rspfImageData> rspfNullPixelFlip::getTile(const rspfIrect& tile_rect,
                                                        rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> result = rspfImageSourceFilter::getTile(tile_rect, resLevel);
   if(!isSourceEnabled()||!result.valid())
   {
      return result.get();
   }
   rspfDataObjectStatus status =  result->getDataObjectStatus();
   
   if((status == RSPF_FULL)||
      (status == RSPF_NULL))
   {
      return result.get();
   }
   // Call the appropriate load method.
   switch (result->getScalarType())
   {
      case RSPF_UCHAR:
      {
         flipPixels(rspf_uint8(0), result.get(), resLevel);
         break;
      }
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         flipPixels(rspf_uint16(0), result.get(), resLevel);
         break;
      }
         
      case RSPF_SSHORT16:
      {
         flipPixels(rspf_sint16(0), result.get(), resLevel);
         break;
      }
      case RSPF_UINT32:
      {
         flipPixels(rspf_uint32(0), result.get(), resLevel);
         break;
      }
      case RSPF_SINT32:
      {
         flipPixels(rspf_sint32(0), result.get(), resLevel);
         break;
      }
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         flipPixels(rspf_float32(0), result.get(), resLevel);
         break;
      }
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         flipPixels(rspf_float64(0), result.get(), resLevel);
         break;
      }
         
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
//         rspfNotify(rspfNotifyLevel_WARN)
//         << "rspfPixelFlipper::getTile Unsupported scalar type!" << endl;
         break;
      }
   }
   
   return result;
}

template <class T>
void rspfNullPixelFlip::flipPixels(T /* dummy */,
                                   rspfImageData* inputTile,
                                   rspf_uint32 resLevel)
{
   if (!inputTile) return;
   rspfDataObjectStatus status =  inputTile->getDataObjectStatus();
   rspfIrect rect = inputTile->getImageRectangle();
   rspfIrect imageBounds = getBoundingRect(resLevel);
   rspfIrect clipRect;
   if(!rect.intersects(imageBounds))
   {
      return;
   }
   clipRect = rect.clipToRect(imageBounds);
   if((status == RSPF_EMPTY)||
      (status == RSPF_PARTIAL))
   {
      if(!rect.completely_within(imageBounds))
      {
         rspf_uint32 bands = inputTile->getNumberOfBands();
         rspfIpt origin = clipRect.ul() - rect.ul();
         rspf_uint32 bandIdx = 0;
         rspf_uint32 inputW = inputTile->getWidth();
         rspf_uint32 originOffset = origin.y*inputW + origin.x;
         rspf_uint32 w = clipRect.width();
         rspf_uint32 h = clipRect.height();
         rspf_uint32 x = 0;
         rspf_uint32 y = 0;
         for(bandIdx = 0; bandIdx < bands; ++bandIdx)
         {
            T* bandPtr = static_cast<T*>(inputTile->getBuf(bandIdx)) + originOffset;
            T  nullValue = static_cast<T>(inputTile->getNullPix(bandIdx));
            T  replaceValue = (m_replacementType==ReplacementType_MINPIX?static_cast<T>(inputTile->getMinPix(bandIdx)):
                               static_cast<T>(inputTile->getMaxPix(bandIdx)));
            for(y = 0; y < h; ++y)
            {
               for(x = 0; x < w; ++x)
               {
                  if(bandPtr[x] == nullValue)
                  {
                     bandPtr[x] = replaceValue;
                  }
               }
               bandPtr += inputW;
            }
         }
      }
      else
      {
         rspf_uint32 bands = inputTile->getNumberOfBands();
         rspf_uint32 bandIdx = 0;
         rspf_uint32 size = inputTile->getWidth()*inputTile->getHeight();
         for(bandIdx = 0; bandIdx < bands; ++bandIdx)
         {
            T* bandPtr = static_cast<T*>(inputTile->getBuf(bandIdx));
            T  nullValue = static_cast<T>(inputTile->getNullPix(bandIdx));
            T  replaceValue = (m_replacementType==ReplacementType_MINPIX?static_cast<T>(inputTile->getMinPix(bandIdx)):
                               static_cast<T>(inputTile->getMaxPix(bandIdx)));
                               
            rspf_uint32 idx = 0;
            for(idx = 0; idx < size;++idx)
            {
               if((*bandPtr) == nullValue)
               {
                  (*bandPtr) = replaceValue;
               }
               ++bandPtr;
            }
         }
         inputTile->setDataObjectStatus(RSPF_FULL);
      }
   }
}
