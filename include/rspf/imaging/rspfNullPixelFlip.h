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
#ifndef rspfNullPixelFlip_HEADER
#define rspfNullPixelFlip_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPF_DLL rspfNullPixelFlip : public rspfImageSourceFilter
{
public:
   enum ClipMode
   {
      ClipMode_NONE = 0,
      ClipMode_BOUNDING_RECT  = 1
   };
   enum ReplacementType
   {
      ReplacementType_MINPIX = 0,
      ReplacementType_MAXPIX = 1
   };
   
   rspfNullPixelFlip();
   virtual void initialize();
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tile_rect,
                                               rspf_uint32 resLevel=0);
   
protected:
   template <class T> void flipPixels(T dummy,
                                      rspfImageData* inpuTile,
                                      rspf_uint32 resLevel);
   
   template <class T> rspfImageData* clipTile(T dummy,
                                    rspfImageData* inpuTile,
                                    rspf_uint32 resLevel);
   
   ClipMode m_clipMode;
   ReplacementType m_replacementType;
TYPE_DATA   
};
#endif
