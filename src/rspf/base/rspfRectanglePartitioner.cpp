//----------------------------------------------------------------------------
// Copyright (C) 2004 David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// Utility class to partition up a rectangle.
//
// $Id: rspfRectanglePartitioner.cpp 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------

#include <rspf/base/rspfRectanglePartitioner.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
using namespace std;

static rspfTrace traceDebug("rspfRectanglePartitioner:degug");

rspfRectanglePartitioner::rspfRectanglePartitioner()
{
}

rspfRectanglePartitioner::~rspfRectanglePartitioner()
{
}

void rspfRectanglePartitioner::binaryPartition(
   const rspfIrect& inputRectangle,
   vector<rspfIrect>& result,
   rspf_uint64 maxSizeInBytes,
   rspf_uint32 bands,
   rspf_uint32 bytesPerPixel,
   rspf_uint32 internalOverlapPixels) const
{
   // Clear the result rect for starters.
   result.clear();

   // Some sanity checks.
   if (maxSizeInBytes == 0)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Waning:  0 passed for max size in bytes.  Returning..."
         << endl;
      return;
   }
   if (bands == 0)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Waning:  0 passed for number of bands.  Returning..."
         << endl;
      return;
   }
   if (bytesPerPixel == 0)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Waning:  0 passed for bytes per pixel.  Returning..."
         << endl;
      return;
   }
   if (inputRectangle.hasNans())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Waning:  Input rectangle has nans in it!  Returning..."
         << endl;
      return;
   }
   if ( (maxSizeInBytes / (bands*bytesPerPixel)) < 4)
   {
      // Come on now you have to have at least four pixels.
      rspfNotify(rspfNotifyLevel_WARN)
         << "Waning:  Max size in bytes too small.  Returning..."
         << endl;
      return;
   }
   // End of sanity checks...

   // Check the size... We could already be there.
   if (getSize(inputRectangle, bands, bytesPerPixel) <= maxSizeInBytes)
   {
      result.push_back(inputRectangle);

      if (traceDebug())
      {
         trace(inputRectangle, result, maxSizeInBytes, bands, bytesPerPixel);
      }
      
      return;
   }   

   // OK, find the rectangle size that gets that will fit the max size.
   rspfIrect tileRect = inputRectangle;
   
   splitUntilLessThanMax(tileRect,
                         maxSizeInBytes,
                         bands,
                         bytesPerPixel,
                         internalOverlapPixels);

   rspf_int32 input_width  = static_cast<rspf_int32>(inputRectangle.width());
   rspf_int32 input_height = static_cast<rspf_int32>(inputRectangle.height());

   rspf_int32 tile_width   = static_cast<rspf_int32>(tileRect.width());
   rspf_int32 tile_height  = static_cast<rspf_int32>(tileRect.height());

   rspf_int32 tiles_wide   = (input_width % tile_width) ?
      ( (input_width/tile_width)+1) : (input_width/tile_width);
   rspf_int32 tiles_high   = (input_height % tile_height) ?
      ( (input_height/tile_height)+1) : (input_height/tile_height);

   rspf_int32 y_start = inputRectangle.ul().y;
   rspf_int32 y_stop  = y_start + tile_height - 1 + internalOverlapPixels;

   for (rspf_int32 y = 0; y < tiles_high; ++y)
   {
      // Clip to bottom if needed.
      if (y_stop > inputRectangle.lr().y)
      {
         y_stop = inputRectangle.lr().y;
      }
      
      rspf_int32 x_start = inputRectangle.ul().x;
      rspf_int32 x_stop  = x_start + tile_width - 1 + internalOverlapPixels;
      
      for (rspf_int32 x = 0; x < tiles_wide; ++x)
      {
         // Clip to right edge is needed.
         if (x_stop > inputRectangle.lr().x)
         {
            x_stop = inputRectangle.lr().x;
         }

         rspfIrect r(x_start, y_start, x_stop, y_stop);

         // Add it to the result.
         result.push_back(r);
         
         if( 0 == x )
         {
            x_start += tile_width - internalOverlapPixels;
         }
         else
         {
            x_start += tile_width;
         }
         x_stop  += tile_width;
         
      } // End of tiles_wide loop.
      
      if( 0 == y )
      {
         y_start += tile_height - internalOverlapPixels;
      }
      else
      {
         y_start += tile_height;
      }
      y_stop  += tile_height;
      
   } // End of tiles_high loop.
   
   if (traceDebug())
   {
      trace(inputRectangle, result, maxSizeInBytes, bands, bytesPerPixel);
   }
}

void rspfRectanglePartitioner::splitUntilLessThanMax(
   rspfIrect& rect,
   rspf_uint64 maxSizeInBytes,
   rspf_uint32 bands,
   rspf_uint32 bytesPerPixel,
   rspf_uint32 internalOverlapPixels) const
{
   do
   {
      splitRect(rect);

   } while ( getSize(rect,
                     bands,
                     bytesPerPixel,
                     internalOverlapPixels) > maxSizeInBytes );
}

void rspfRectanglePartitioner::splitRect(rspfIrect& rect) const
{
   rspf_int32 width  = static_cast<rspf_int32>(rect.width());
   rspf_int32 height = static_cast<rspf_int32>(rect.height());
   rspf_int32 new_width;
   rspf_int32 new_height;
   
   if (height > width)
   {
      new_width  = width;
      new_height = (height % 2) ? ( (height/2) + 1 ) : (height/2);
   }
   else
   {
      new_width  = (width % 2) ? ( (width/2) + 1) : (width/2);
      new_height = height;
   }
   
   rect = rspfIrect(0, 0, new_width-1, new_height-1);
}

rspf_uint64 rspfRectanglePartitioner::getSize(
   const rspfIrect& rect,
   rspf_uint32 bands,
   rspf_uint32 bytesPerPixel,
   rspf_uint32 internalOverlapPixels) const
{
   return( ( rect.width()  + 2 * internalOverlapPixels ) *
           ( rect.height() + 2 * internalOverlapPixels ) *
           bands * bytesPerPixel );
}

void rspfRectanglePartitioner::trace(const rspfIrect& r,
                                      const std::vector<rspfIrect>& v,
                                      rspf_uint64 maxSizeInBytes,
                                      rspf_uint32 bands,
                                      rspf_uint32 bytesPerPixel) const
{
   rspfNotify(rspfNotifyLevel_DEBUG)
      << "rspfRectanglePartitioner DEBUG:"
      << "\nInput rectangle:            " << r
      << "\nInput rectangle byte size:  " << getSize(r, bands, bytesPerPixel)
      << "\nTile max size in bytes:     " << maxSizeInBytes
      << "\nbands:                      " << bands
      << "\nbytesPerPixel:              " << bytesPerPixel
      << "\nNumber of output tiles:     " << v.size()
      << "\nTiled rectangles:\n";

   int index = 0;
   vector<rspfIrect>::const_iterator i = v.begin();
   while(i != v.end())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "tile[" << index << "]:  " << *i
         << "\nsize in bytes:  " << getSize(*i, bands, bytesPerPixel)
         << endl;
      ++i;
      ++index;
   }
}
